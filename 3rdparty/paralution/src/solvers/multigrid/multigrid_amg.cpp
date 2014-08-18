// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2014 Dimitar Lukarski
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************



// PARALUTION version 0.7.0 


#include "multigrid_amg.hpp"

#include "../../base/global_matrix.hpp"
#include "../../base/local_matrix.hpp"

#include "../../base/global_stencil.hpp"
#include "../../base/local_stencil.hpp"

#include "../../base/global_vector.hpp"
#include "../../base/local_vector.hpp"

#include "../krylov/cg.hpp"
#include "../preconditioners/preconditioner_multielimination.hpp"
#include "../preconditioners/preconditioner_multicolored_ilu.hpp"
#include "../preconditioners/preconditioner_multicolored_gs.hpp"
#include "../preconditioners/preconditioner_multicolored.hpp"


#include "../../utils/log.hpp"

#include <assert.h>
#include <math.h>
#include <list>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
AMG<OperatorType, VectorType, ValueType>::AMG() {

  LOG_DEBUG(this, "AMG::AMG()",
            "default constructor");

  // TODO also assuming 1 dof per node

  // parameter for strong couplings in smoothed aggregation
  this->eps_   = ValueType(0.01);
  this->relax_ = ValueType(2.0/3.0);
  this->over_interp_ = 1.5;
  this->coarse_size_ = 300;
  this->interp_type_ = SmoothedAggregation;

  // manual smoothers and coarse solver
  this->set_sm_ = false;
  this->set_s_  = false;

  // default smoother format
  this->sm_format_ = ELL;
  // default operator format
  this->op_format_ = CSR;

  // since hierarchy has not been built yet
  this->hierarchy_ = false;

  // initialize temp default smoother pointer
  this->sm_default_ = NULL;

}

template <class OperatorType, class VectorType, typename ValueType>
AMG<OperatorType, VectorType, ValueType>::~AMG() {

  LOG_DEBUG(this, "AMG::AMG()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("AMG solver");

  LOG_INFO("AMG number of levels " << this->levels_);

  switch(this->interp_type_) {
    case Aggregation:
      LOG_INFO("AMG using aggregation interpolation");
      break;
    case SmoothedAggregation:
      LOG_INFO("AMG using smoothed aggregation interpolation");
      break;
  }

  LOG_INFO("AMG coarsest operator size = " << this->op_level_[this->levels_-2]->get_nrow());
  LOG_INFO("AMG coarsest level nnz = " <<this->op_level_[this->levels_-2]->get_nnz());
  LOG_INFO("AMG with smoother:");
  this->smoother_level_[0]->Print();
  
}


template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  assert(this->levels_ > 0);

  LOG_INFO("AMG solver starts");
  LOG_INFO("AMG number of levels " << this->levels_);

  switch(this->interp_type_) {
    case Aggregation:
      LOG_INFO("AMG using aggregation interpolation");
      break;
    case SmoothedAggregation:
      LOG_INFO("AMG using smoothed aggregation interpolation");
      break;
  }

  LOG_INFO("AMG coarsest operator size = " << this->op_level_[this->levels_-2]->get_nrow());
  LOG_INFO("AMG coarsest level nnz = " <<this->op_level_[this->levels_-2]->get_nnz());
  LOG_INFO("AMG with smoother:");
  this->smoother_level_[0]->Print();

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

    LOG_INFO("AMG ends");

}


template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetCoarsestLevel(const int coarseSize) {

  LOG_DEBUG(this, "AMG::SetCoarsestLevel()",
            coarseSize);


  assert(this->build_ == false);
  assert(this->hierarchy_ == false);

  this->coarse_size_ = coarseSize;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetManualSmoothers(const bool sm_manual) {

  LOG_DEBUG(this, "AMG::SetManualSmoothers()",
            sm_manual);

  assert(this->build_ == false);

  this->set_sm_ = sm_manual;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetManualSolver(const bool s_manual) {

  LOG_DEBUG(this, "AMG::SetManualSolver()",
            s_manual);

  assert(this->build_ == false);

  this->set_s_ = s_manual;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetDefaultSmootherFormat(const unsigned int op_format) {

  LOG_DEBUG(this, "AMG::SetDefaultSmootherFormat()",
            op_format);

  assert(this->build_ == false);

  this->sm_format_ = op_format;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetOperatorFormat(const unsigned int op_format) {

  LOG_DEBUG(this, "AMG::SetOperatorFormat()",
            op_format);

  this->op_format_ = op_format;

}

template <class OperatorType, class VectorType, typename ValueType>
int AMG<OperatorType, VectorType, ValueType>::GetNumLevels() {

  assert(this->hierarchy_ != false);

  return this->levels_;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetInterpolation(_interp interpType) {

  this->interp_type_ = interpType;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetInterpRelax(const ValueType relax) {

  LOG_DEBUG(this, "AMG::SetInterpRelax()",
            relax);

  this->relax_ = relax;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetOverInterp(const ValueType overInterp) {

  LOG_DEBUG(this, "AMG::SetOverInterp()",
            overInterp);

  this->over_interp_ = overInterp;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "AMG::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);

  this->BuildHierarchy();

  this->build_ = true;

  LOG_DEBUG(this, "AMG::Build()",
            "#*# allocate data");

  this->d_level_ = new VectorType*[this->levels_];
  this->r_level_ = new VectorType*[this->levels_];
  this->t_level_ = new VectorType*[this->levels_];
  this->s_level_ = new VectorType*[this->levels_];

  // Extra structure for K-cycle
  if (this->cycle_ == 2) {
    this->p_level_ = new VectorType*[this->levels_-2];
    this->q_level_ = new VectorType*[this->levels_-2];
    this->k_level_ = new VectorType*[this->levels_-2];
    this->l_level_ = new VectorType*[this->levels_-2];

    for (int i=0; i<this->levels_-2; ++i) {
      this->p_level_[i] = new VectorType;
      this->p_level_[i]->CloneBackend(*this->op_level_[i]);
      this->p_level_[i]->Allocate("p", this->op_level_[i]->get_nrow());

      this->q_level_[i] = new VectorType;
      this->q_level_[i]->CloneBackend(*this->op_level_[i]);
      this->q_level_[i]->Allocate("q", this->op_level_[i]->get_nrow());

      this->k_level_[i] = new VectorType;
      this->k_level_[i]->CloneBackend(*this->op_level_[i]);
      this->k_level_[i]->Allocate("k", this->op_level_[i]->get_nrow());

      this->l_level_[i] = new VectorType;
      this->l_level_[i]->CloneBackend(*this->op_level_[i]);
      this->l_level_[i]->Allocate("l", this->op_level_[i]->get_nrow());
    }
  }

  for (int i=0; i<this->levels_; ++i) {

    this->r_level_[i] = new VectorType;
    this->t_level_[i] = new VectorType;
    this->s_level_[i] = new VectorType;
    if (i > 0) this->d_level_[i] = new VectorType;

    this->r_level_[i]->CloneBackend(*this->op_);
    this->t_level_[i]->CloneBackend(*this->op_);
    this->s_level_[i]->CloneBackend(*this->op_);
    if (i > 0) this->d_level_[i]->CloneBackend(*this->op_);

  }

  // Allocate temporary vectors for cycles
  for (int level=0; level<this->levels_; ++level) {

    if (level > 0) {
      this->d_level_[level]->Allocate("defect correction", this->op_level_[level-1]->get_nrow());
      this->r_level_[level]->Allocate("residual", this->op_level_[level-1]->get_nrow());
      this->t_level_[level]->Allocate("temporary", this->op_level_[level-1]->get_nrow());
      this->s_level_[level]->Allocate("temporary", this->op_level_[level-1]->get_nrow());
    } else {
      this->r_level_[level]->Allocate("residual", this->op_->get_nrow());
      this->t_level_[level]->Allocate("temporary", this->op_->get_nrow());
      this->s_level_[level]->Allocate("temporary", this->op_->get_nrow());
    }
  }

  LOG_DEBUG(this, "AMG::Build()",
            "#*# setup smoothers");

  // Setup and build smoothers
  if (this->set_sm_ == false) {

    // Smoother for each level
    FixedPoint<OperatorType, VectorType, ValueType > **sm = NULL;
    sm = new FixedPoint<OperatorType, VectorType, ValueType >* [this->levels_-1];
    MultiColoredGS<OperatorType, VectorType, ValueType > **gs = NULL;
    gs = new MultiColoredGS<OperatorType, VectorType, ValueType >* [this->levels_-1];

    this->smoother_level_ = new IterativeLinearSolver<OperatorType, VectorType, ValueType>*[this->levels_-1];
    this->sm_default_ = new Solver<OperatorType, VectorType, ValueType>*[this->levels_-1];

    for (int i=0; i<this->levels_-1; ++i) {
      sm[i] = new FixedPoint<OperatorType, VectorType, ValueType >;
      gs[i] = new MultiColoredGS<OperatorType, VectorType, ValueType >;

      gs[i]->SetPrecondMatrixFormat(this->sm_format_);

      // relxation
      sm[i]->SetRelaxation(ValueType(1.3));
      sm[i]->SetPreconditioner(*gs[i]);

      // be quite
      sm[i]->Verbose(0);
      this->smoother_level_[i] = sm[i];
      // pass pointer to class so we can free it when clearing
      this->sm_default_[i] = gs[i];

    }

    delete[] sm;
    delete[] gs;

  }

  for (int i=0; i<this->levels_-1; ++i) {
    if (i > 0)
      this->smoother_level_[i]->SetOperator(*this->op_level_[i-1]);
    else
      this->smoother_level_[i]->SetOperator(*this->op_);

    this->smoother_level_[i]->Build();
  }

  LOG_DEBUG(this, "AMG::Build()",
            "#*# setup coarse solver");

  // Setup and build coarse grid solver
  if (this->set_s_ == false) {
    
    // Coarse Grid Solver
    CG<OperatorType, VectorType, ValueType > *cgs
      = new CG<OperatorType, VectorType, ValueType >;

    // be quite
    cgs->Verbose(0);

    this->solver_coarse_ = cgs;
  }

  this->solver_coarse_->SetOperator(*this->op_level_[this->levels_-2]);
  this->solver_coarse_->Build();

  LOG_DEBUG(this, "AMG::Build()",
            "#*# convert operators");

  // Convert operator to op_format
  if (this->op_format_ != CSR)
    for (int i=0; i<this->levels_-1;++i)
      this->op_level_[i]->ConvertTo(this->op_format_);

  LOG_DEBUG(this, "AMG::Build()",
            this->build_ <<
            " #*# end");

}


template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "AMG::Clear()",
            this->build_);

  if (this->build_ == true) {

    for (int i=0; i<this->levels_; ++i) {

      // Clear temporary VectorTypes
      if (i > 0) delete this->d_level_[i];
      delete this->r_level_[i];
      delete this->t_level_[i];
      delete this->s_level_[i];
    }

    delete[] this->d_level_;
    delete[] this->r_level_;
    delete[] this->t_level_;
    delete[] this->s_level_;

    // Extra structure for K-cycle
    if (this->cycle_ == 2) {

      for (int i=0; i<this->levels_-2; ++i) {
        delete this->p_level_[i];
        delete this->q_level_[i];
        delete this->k_level_[i];
        delete this->l_level_[i];
      }

      delete[] this->p_level_;
      delete[] this->q_level_;
      delete[] this->k_level_;
      delete[] this->l_level_;

    }

    for (int i=0; i<this->levels_-1; ++i) {

      // Clear operator data structure
      delete this->op_level_[i];
      delete this->restrict_op_level_[i];
      delete this->prolong_op_level_[i];

      if (this->set_sm_ == false) {
        delete this->smoother_level_[i];
        delete this->sm_default_[i];
      } else
        this->smoother_level_[i]->Clear();

    }

    // Clear coarse grid solver - we built it
    if (this->set_s_ == false)
      delete this->solver_coarse_;
    else
      this->solver_coarse_->Clear();

    delete[] this->op_level_;
    delete[] this->restrict_op_level_;
    delete[] this->prolong_op_level_;

    if (this->set_sm_ == false) {
      delete[] this->smoother_level_;
      delete[] this->sm_default_;
    }

    this->levels_ = -1;
    
    this->iter_ctrl_.Clear();

    this->build_ = false;
    this->hierarchy_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::BuildHierarchy(void) {

  LOG_DEBUG(this, "AMG::BuildHierarchy()",
            " #*# begin");

  if (this->hierarchy_ == false) {

    assert(this->build_ == false);
    this->hierarchy_ = true;

    ValueType eps = this->eps_;

    // AMG will use operators for inter grid transfers
    this->InitOperator(true);
    assert(this->op_ != NULL);
    assert(this->coarse_size_ > 0);

    // Lists for the building procedure
    std::list<OperatorType*> op_list_;
    std::list<OperatorType*> restrict_list_;
    std::list<OperatorType*> prolong_list_;

    this->levels_ = 1;

    // Build finest hierarchy
    LocalVector<int> *connections_type_ = new LocalVector<int>;
    LocalVector<int> *aggregates_type_  = new LocalVector<int>;
    connections_type_->CloneBackend(*this->op_);
    aggregates_type_->CloneBackend(*this->op_);

    op_list_.push_back(new OperatorType);
    restrict_list_.push_back(new OperatorType);
    prolong_list_.push_back(new OperatorType);

    switch(this->interp_type_) {

      case Aggregation:
        this->Connect(*this->op_, eps, connections_type_);
        this->Aggregate(*this->op_, *connections_type_, aggregates_type_);
        this->Aggr(*this->op_, *aggregates_type_, prolong_list_.back(), restrict_list_.back());
        break;

      case SmoothedAggregation:
        this->Connect(*this->op_, eps, connections_type_);
        this->Aggregate(*this->op_, *connections_type_, aggregates_type_);
        eps *= 0.5;
        this->SmoothedAggr(*this->op_, *aggregates_type_, *connections_type_, 
                           prolong_list_.back(), restrict_list_.back());
        break;

      default:
        LOG_INFO("Aggregation not supported");
        FATAL_ERROR(__FILE__, __LINE__);

    }

    // Free unused vectors
    connections_type_->Clear();
    aggregates_type_->Clear();

    // Create coarse operator
    this->CoarsenOperator(*restrict_list_.back(), *prolong_list_.back(), *this->op_, op_list_.back());

    if (this->interp_type_ == Aggregation && this->over_interp_ > 1.0)
      op_list_.back()->Scale(ValueType(1.0)/this->over_interp_);

    ++this->levels_;

    while(op_list_.back()->get_nrow() > this->coarse_size_) {

      // Add new list elements
      restrict_list_.push_back(new OperatorType);
      prolong_list_.push_back(new OperatorType);

      switch(this->interp_type_) {

        case Aggregation:
          this->Connect(*op_list_.back(), eps, connections_type_);
          this->Aggregate(*op_list_.back(), *connections_type_, aggregates_type_);
          this->Aggr(*op_list_.back(), *aggregates_type_, prolong_list_.back(), restrict_list_.back());
          break;

        case SmoothedAggregation:
          this->Connect(*op_list_.back(), eps, connections_type_);
          this->Aggregate(*op_list_.back(), *connections_type_, aggregates_type_);
          eps *= 0.5;
          this->SmoothedAggr(*op_list_.back(), *aggregates_type_, *connections_type_, 
                             prolong_list_.back(), restrict_list_.back());
          break;

      }

      // Free unused vectors
      connections_type_->Clear();
      aggregates_type_->Clear();

      // Add new list element
      OperatorType *prev_op_ = op_list_.back();
      op_list_.push_back(new OperatorType);

      // Create coarse operator
      this->CoarsenOperator(*restrict_list_.back(), *prolong_list_.back(), *prev_op_, op_list_.back());

      if (this->interp_type_ == Aggregation && this->over_interp_ > 1.0)
        op_list_.back()->Scale(ValueType(1.0)/this->over_interp_);

      ++this->levels_;

    }

    delete connections_type_;
    delete aggregates_type_;

    if (this->levels_ < 2) {
      LOG_INFO("Problem size too small for AMG, use Krylov solver instead");
      FATAL_ERROR(__FILE__, __LINE__);
    }

    // Allocate data structures
    this->op_level_ = new OperatorType*[this->levels_-1];
    this->restrict_op_level_ = new OperatorType*[this->levels_-1];
    this->prolong_op_level_ = new OperatorType*[this->levels_-1];

    typename std::list<OperatorType*>::iterator op_it  = op_list_.begin();
    typename std::list<OperatorType*>::iterator pro_it = prolong_list_.begin();
    typename std::list<OperatorType*>::iterator res_it = restrict_list_.begin();

    for (int i=0; i<this->levels_-1; ++i) {

      this->op_level_[i] = *op_it;
      ++op_it;

      this->restrict_op_level_[i] = *res_it;
      ++res_it;

      this->prolong_op_level_[i] = *pro_it;
      ++pro_it;

    }

  }

  LOG_DEBUG(this, "AMG::BuildHierarchy()",
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetCouplingStrength(const ValueType eps) {

  LOG_DEBUG(this, "AMG::SetCouplingStrength()",
            eps);


  this->eps_ = eps;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SmoothedAggr(const OperatorType &op,
                                                            const LocalVector<int> &aggregates,
                                                            const LocalVector<int> &connections,
                                                            OperatorType *prolong,
                                                            OperatorType *restrict) const {

  LOG_DEBUG(this, "AMG::SmoothedAggr()",
            this->build_);

  assert(&op != NULL);
  assert(&aggregates != NULL);
  assert(&connections != NULL);

  op.AMGSmoothedAggregation(this->relax_, aggregates, connections, prolong, restrict);

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Aggr(const OperatorType &op,
                                                    const LocalVector<int> &aggregates,
                                                          OperatorType *prolong,
                                                          OperatorType *restrict) const {
  LOG_DEBUG(this, "AMG::Aggr()",
            this->build_);

 
  assert(&op != NULL);
  assert(&aggregates != NULL);

  op.AMGAggregation(aggregates, prolong, restrict);

}


template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Connect(const OperatorType &op,
                                                       const ValueType eps,
                                                             LocalVector<int> *connections) const {

  LOG_DEBUG(this, "AMG::Connect()",
            this->build_);

  assert(&op != NULL);

  op.AMGConnect(eps, connections);

}


template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Aggregate(const OperatorType &op,
                                                         const LocalVector<int> &connections,
                                                               LocalVector<int> *aggregates) const {

  LOG_DEBUG(this, "AMG::Aggregate()",
            this->build_);

  assert(&op != NULL);
  assert(&connections != NULL);

  op.AMGAggregate(connections, aggregates);

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::CoarsenOperator(const OperatorType &restrict,
                                                               const OperatorType &prolong,
                                                               const OperatorType &op_fine,
                                                                     OperatorType *op_coarse) {

  LOG_DEBUG(this, "AMG::CoarsenOperator()",
            this->build_);

  assert(this->op_ != NULL);
  assert(&op_fine  != NULL);
  assert(&restrict != NULL);
  assert(&prolong  != NULL);

  OperatorType tmp;
  tmp.CloneBackend(*this->op_);
  op_coarse->CloneBackend(*this->op_);

  tmp.MatrixMult(restrict, op_fine);
  op_coarse->MatrixMult(tmp, prolong);

}


template class AMG< LocalMatrix<double>, LocalVector<double>, double >;
template class AMG< LocalMatrix<float>,  LocalVector<float>, float >;

}

