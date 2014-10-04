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


#include "multigrid.hpp"
#include "../iter_ctrl.hpp"

#include "../../base/global_matrix.hpp"
#include "../../base/local_matrix.hpp"

#include "../../base/global_stencil.hpp"
#include "../../base/local_stencil.hpp"

#include "../../base/global_vector.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"

#include <assert.h>
#include <math.h>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
MultiGrid<OperatorType, VectorType, ValueType>::MultiGrid() {

  LOG_DEBUG(this, "MultiGrid::MultiGrid()",
            "default constructor");


  this->levels_ = -1;
  this->current_level_ = 0;

  this->iter_pre_smooth_ = 1;
  this->iter_post_smooth_ = 2;

  this->operator_type_ = true;
  this->scaling_ = true;

  this->op_level_ = NULL;

  this->restrict_op_level_ = NULL;
  this->prolong_op_level_ = NULL;
  this->restrict_vector_level_ = NULL;

  this->d_level_ = NULL;
  this->r_level_ = NULL;
  this->t_level_ = NULL;
  this->s_level_ = NULL;
  this->p_level_ = NULL;
  this->q_level_ = NULL;
  this->k_level_ = NULL;
  this->l_level_ = NULL;

  this->solver_coarse_ = NULL;
  this->smoother_level_ = NULL;

  this->scaling_ = true;

  this->cycle_ = 0;

}

template <class OperatorType, class VectorType, typename ValueType>
MultiGrid<OperatorType, VectorType, ValueType>::~MultiGrid() {

  LOG_DEBUG(this, "MultiGrid::~MultiGrid()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::InitLevels(const int levels) {

  LOG_DEBUG(this, "MultiGrid::InitLevels()",
            levels);

  assert(this->build_ == false);
  assert(levels > 0);

  this->levels_ = levels;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::InitOperator(const bool type) {

  LOG_DEBUG(this, "MultiGrid::InitOperator()",
            type);

  assert(this->build_ == false);
  this->operator_type_ = type;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetPreconditioner(Solver<OperatorType, VectorType, ValueType> &precond) {

  LOG_INFO("MultiGrid:SetPreconditioner() Perhaps you want to set the smoothers on all levels? use SetSmootherLevel() instead of SetPreconditioner!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetOperatorHierarchy(OperatorType **op) {

  LOG_DEBUG(this, "MultiGrid::SetOperatorHierarchy()",
            "");
  
  assert(this->build_ == false);
  assert(op != NULL );

  this->op_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSmoother(IterativeLinearSolver<OperatorType, VectorType, ValueType> **smoother) {

  LOG_DEBUG(this, "MultiGrid::SetSmoother()",
            "");

//  assert(this->build_ == false); not possible due to AMG
  assert(smoother != NULL);

  this->smoother_level_ = smoother;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSmootherPreIter(const int iter) {

  LOG_DEBUG(this, "MultiGrid::SetSmootherPreIter()",
            iter);

  this->iter_pre_smooth_ = iter;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSmootherPostIter(const int iter) {

  LOG_DEBUG(this, "MultiGrid::SetSmootherPostIter()",
            iter);

  this->iter_post_smooth_ = iter;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSolver(Solver<OperatorType, VectorType, ValueType> &solver) {

  LOG_DEBUG(this, "MultiGrid::SetSolver()",
            "");

//  assert(this->build_ == false); not possible due to AMG
  assert(&solver != NULL);

  this->solver_coarse_ = &solver;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetRestrictOperator(OperatorType **op) {

  LOG_DEBUG(this, "MultiGrid::SetRestrictOperator()",
            "");

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->operator_type_ == true);

  this->restrict_op_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetRestrictVector(LocalVector<int> **op) {

  LOG_DEBUG(this, "MultiGrid::SetRestrictVector()",
            "");

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->operator_type_ == false);

  this->restrict_vector_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetProlongOperator(OperatorType **op) {

  LOG_DEBUG(this, "MultiGrid::SetProlongOperator()",
            "");

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->operator_type_ == true);

  this->prolong_op_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetScaling(const bool scaling) {

  LOG_DEBUG(this, "MultiGrid::SetScaling()",
            scaling);

  this->scaling_ = scaling;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetCycle(const _cycle cycle) {

  LOG_DEBUG(this, "MultiGrid::SetCycle()",
            cycle);

  this->cycle_ = cycle;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Print(void) const {
  
  LOG_INFO("MultiGrid solver");
  
}


template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  assert(this->levels_ > 0);

  LOG_INFO("MultiGrid solver starts");
  LOG_INFO("MultiGrid Number of levels " << this->levels_);
  LOG_INFO("MultiGrid with smoother:");
  this->smoother_level_[0]->Print();

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

    LOG_INFO("MultiGrid ends");

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "MultiGrid::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  for (int i=0; i<this->levels_-1; ++i) {
    assert(this->op_level_[i] != NULL);
    assert(this->smoother_level_[i] != NULL);
    assert(this->restrict_op_level_[i] != NULL);
    assert(this->prolong_op_level_[i] != NULL);
  }
  assert(this->op_ != NULL);
  assert(this->solver_coarse_ != NULL);
  assert(this->levels_ > 0);

  LOG_DEBUG(this, "MultiGrid::Build()",
            "#*# setup finest level 0");

  // Setup finest level 0
  this->smoother_level_[0]->SetOperator(*this->op_);
  this->smoother_level_[0]->Build();


  LOG_DEBUG(this, "MultiGrid::Build()",
            "#*# setup coarser levels");

  // Setup coarser levels
  for (int i=1; i<this->levels_-1; ++i) {
    this->smoother_level_[i]->SetOperator(*this->op_level_[i-1]);
    this->smoother_level_[i]->Build();
  }

  LOG_DEBUG(this, "MultiGrid::Build()",
            "#*# setup coarse grid solver");
  // Setup coarse grid solver
  this->solver_coarse_->SetOperator(*op_level_[this->levels_-2]);
  this->solver_coarse_->Build();

  LOG_DEBUG(this, "MultiGrid::Build()",
            "#*# setup all tmp vectors");

  // Setup all temporary vectors for the cycles - needed on all levels
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

  for (int i=1; i<this->levels_; ++i) {

    // On finest level, we need to get the size from this->op_ instead
    this->d_level_[i] = new VectorType;
    this->d_level_[i]->CloneBackend(*this->op_level_[i-1]);
    this->d_level_[i]->Allocate("defect correction", this->op_level_[i-1]->get_nrow());

    this->r_level_[i] = new VectorType;
    this->r_level_[i]->CloneBackend(*this->op_level_[i-1]);
    this->r_level_[i]->Allocate("residual", this->op_level_[i-1]->get_nrow());

    this->t_level_[i] = new VectorType;
    this->t_level_[i]->CloneBackend(*this->op_level_[i-1]);
    this->t_level_[i]->Allocate("temporary", this->op_level_[i-1]->get_nrow());

    this->s_level_[i] = new VectorType;
    this->s_level_[i]->CloneBackend(*this->op_level_[i-1]);
    this->s_level_[i]->Allocate("temporary", this->op_level_[i-1]->get_nrow());

  }

  this->r_level_[0] = new VectorType;
  this->r_level_[0]->CloneBackend(*this->op_);
  this->r_level_[0]->Allocate("residual", this->op_->get_nrow());

  this->t_level_[0] = new VectorType;
  this->t_level_[0]->CloneBackend(*this->op_);
  this->t_level_[0]->Allocate("temporary", this->op_->get_nrow());

  this->s_level_[0] = new VectorType;
  this->s_level_[0]->CloneBackend(*this->op_);
  this->s_level_[0]->Allocate("temporary", this->op_->get_nrow());

  LOG_DEBUG(this, "MultiGrid::Build()",
            this->build_ <<
            " #*# end");
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "MultiGrid::Clear()",
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

    // Clear smoothers - we built it
    for (int i=0; i<this->levels_-1; ++i)
      this->smoother_level_[i]->Clear();

    // Clear coarse grid solver - we built it
    this->solver_coarse_->Clear();

    this->levels_ = -1;
    
    this->iter_ctrl_.Clear();

    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "MultiGrid::MoveToHostLocalData_()",
            this->build_);  

  if (this->build_ == true) {

    this->r_level_[this->levels_-1]->MoveToHost();
    this->d_level_[this->levels_-1]->MoveToHost();
    this->t_level_[this->levels_-1]->MoveToHost();
    this->s_level_[this->levels_-1]->MoveToHost();
    this->solver_coarse_->MoveToHost();

    for (int i=0; i<this->levels_-1; ++i) {
      this->op_level_[i]->MoveToHost();
      this->smoother_level_[i]->MoveToHost();
      this->r_level_[i]->MoveToHost();
      if (i > 0) this->d_level_[i]->MoveToHost();
      this->t_level_[i]->MoveToHost();
      this->s_level_[i]->MoveToHost();

      if (this->operator_type_) {
        this->restrict_op_level_[i]->MoveToHost();
        this->prolong_op_level_[i]->MoveToHost();
      } else
        this->restrict_vector_level_[i]->MoveToHost();
    }

    // Extra structure for K-cycle
    if (this->cycle_ == 2) {
      for (int i=0; i<this->levels_-2; ++i) {
        this->p_level_[i]->MoveToHost();
        this->q_level_[i]->MoveToHost();
        this->k_level_[i]->MoveToHost();
        this->l_level_[i]->MoveToHost();
      }
    }

    if (this->precond_ != NULL) {
      this->precond_->MoveToHost();
    }

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "MultiGrid::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->r_level_[this->levels_-1]->MoveToAccelerator();
    this->d_level_[this->levels_-1]->MoveToAccelerator();
    this->t_level_[this->levels_-1]->MoveToAccelerator();
    this->s_level_[this->levels_-1]->MoveToAccelerator();
    this->solver_coarse_->MoveToAccelerator();

    for (int i=0; i<this->levels_-1; ++i) {
      this->op_level_[i]->MoveToAccelerator();
      this->smoother_level_[i]->MoveToAccelerator();
      this->r_level_[i]->MoveToAccelerator();
      if (i > 0) this->d_level_[i]->MoveToAccelerator();
      this->t_level_[i]->MoveToAccelerator();
      this->s_level_[i]->MoveToAccelerator();

      if (this->operator_type_) {
        this->restrict_op_level_[i]->MoveToAccelerator();
        this->prolong_op_level_[i]->MoveToAccelerator();
      } else
        this->restrict_vector_level_[i]->MoveToAccelerator();
    }

    // Extra structure for K-cycle
    if (this->cycle_ == 2) {
      for (int i=0; i<this->levels_-2; ++i) {
        this->p_level_[i]->MoveToAccelerator();
        this->q_level_[i]->MoveToAccelerator();
        this->k_level_[i]->MoveToAccelerator();
        this->l_level_[i]->MoveToAccelerator();
      }
    }

    if (this->precond_ != NULL) {
      this->precond_->MoveToAccelerator();
    }

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                                 VectorType *x) {

  LOG_DEBUG(this, "MultiGrid::Solve()",
            " #*# begin");

  assert(this->levels_ > 1);
  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_ != NULL);
  assert(this->build_ == true);
  assert(this->precond_  == NULL);
  assert(this->solver_coarse_ != NULL);

  for (int i=0; i<this->levels_; ++i) {
    if (i > 0) assert(this->d_level_[i] != NULL);
    assert(this->r_level_[i] != NULL);
    assert(this->t_level_[i] != NULL);
    assert(this->s_level_[i] != NULL);
  }

  if (this->cycle_ == 2) {
    for (int i=0; i<this->levels_-2; ++i) {
      assert(this->k_level_[i] != NULL);
      assert(this->l_level_[i] != NULL);
      assert(this->p_level_[i] != NULL);
      assert(this->q_level_[i] != NULL);
    }
  }

  for (int i=0; i<this->levels_-1; ++i) {
    if (i > 0) assert(this->op_level_[i] != NULL);
    assert(this->smoother_level_[i] != NULL);

    if (this->operator_type_) {
      assert(this->restrict_op_level_[i] != NULL);
      assert(this->prolong_op_level_[i] != NULL);
    } else
      assert(this->restrict_vector_level_[i] != NULL);
  }

  if (this->verb_ > 0) {
    this->PrintStart_();
    this->iter_ctrl_.PrintInit();
  }

  // initial residual = b - Ax
  this->op_->Apply(*x, this->r_level_[0]);
  this->r_level_[0]->ScaleAdd(ValueType(-1.0), rhs);

  this->res_norm_ = this->Norm(*this->r_level_[0]);
  this->iter_ctrl_.InitResidual(this->res_norm_);

  while (!this->iter_ctrl_.CheckResidual(this->res_norm_, this->index_))
    this->Vcycle_(rhs, x);

  if (this->verb_ > 0) {
    this->iter_ctrl_.PrintStatus();
    this->PrintEnd_();
  }

  LOG_DEBUG(this, "MultiGrid::Solve()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Restrict_(const VectorType &fine,
                                                                     VectorType *coarse,
                                                               const int level) {

  LOG_DEBUG(this, "MultiGrid::Restrict_()",
            level);

  if (this->operator_type_)
    this->restrict_op_level_[level]->Apply(fine, coarse);
  else
    coarse->Restriction(fine, *this->restrict_vector_level_[level]);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Prolong_(const VectorType &coarse,
                                                                    VectorType *fine,
                                                              const int level) {
  LOG_DEBUG(this, "MultiGrid::Prolong_()",
            level);

  if (this->operator_type_)
    this->prolong_op_level_[level]->Apply(coarse, fine);
  else
    fine->Prolongation(coarse, *this->restrict_vector_level_[level]);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Vcycle_(const VectorType &rhs,
                                                                   VectorType *x) {

  LOG_DEBUG(this, "MultiGrid::Vcycle_()",
            " #*# begin");

  // Perform cycle
  if (this->current_level_ < this->levels_-1) {

    ValueType factor;

    // Pre-smoothing on finest level
    this->smoother_level_[this->current_level_]->InitMaxIter(this->iter_pre_smooth_);
    this->smoother_level_[this->current_level_]->Solve(rhs, x);

    if (this->scaling_ == true) {
      if (this->current_level_ > 0 && this->current_level_ < this->levels_ - 2 && this->iter_pre_smooth_ > 0) {

        this->r_level_[this->current_level_]->PointWiseMult(rhs, *x);
        factor = this->r_level_[this->current_level_]->Reduce();
        this->op_level_[this->current_level_-1]->Apply(*x, this->r_level_[this->current_level_]);
        this->r_level_[this->current_level_]->PointWiseMult(*x);
        factor /= this->r_level_[this->current_level_]->Reduce();
        x->Scale(factor);
      }
    }

    // Update residual
    if (this->current_level_ == 0)
      this->op_->Apply(*x, this->s_level_[this->current_level_]);
    else
      this->op_level_[this->current_level_-1]->Apply(*x, this->s_level_[this->current_level_]);
    this->s_level_[this->current_level_]->ScaleAdd(ValueType(-1.0), rhs);

    // Restrict residual vector on finest level
    this->Restrict_(*this->s_level_[this->current_level_], this->t_level_[this->current_level_+1], this->current_level_);
    ++this->current_level_;

    // Set new solution for recursion to zero
    this->d_level_[this->current_level_]->Zeros();

    // Recursive call dependent on the cycle
    switch (this->cycle_) {
      // V-cycle
      case 0:   this->Vcycle_(*this->t_level_[this->current_level_], d_level_[this->current_level_]);
                break;

      // W-cycle
      case 1:   this->Wcycle_(*this->t_level_[this->current_level_], d_level_[this->current_level_]);
                break;

      // K-cycle
      case 2:   this->Kcycle_(*this->t_level_[this->current_level_], d_level_[this->current_level_]);
                break;

      // F-cycle
      case 3:   this->Fcycle_(*this->t_level_[this->current_level_], d_level_[this->current_level_]);
                break;

      default:  FATAL_ERROR(__FILE__, __LINE__);
                break;
    }

    // Prolong solution vector on finest level
    this->Prolong_(*this->d_level_[this->current_level_], this->r_level_[this->current_level_-1], this->current_level_-1);
    --this->current_level_;

    // Scaling
    if (this->scaling_ == true && this->current_level_ < this->levels_ - 2) {
      if (this->current_level_ == 0)
        this->s_level_[this->current_level_]->PointWiseMult(*this->r_level_[this->current_level_]);
      else
        this->s_level_[this->current_level_]->PointWiseMult(*this->r_level_[this->current_level_], *this->t_level_[this->current_level_]);
      factor = this->s_level_[this->current_level_]->Reduce();
      if (this->current_level_ == 0)
        this->op_->Apply(*this->r_level_[this->current_level_], this->s_level_[this->current_level_]);
      else
        this->op_level_[this->current_level_-1]->Apply(*this->r_level_[this->current_level_], this->s_level_[this->current_level_]);
      this->s_level_[this->current_level_]->PointWiseMult(*this->r_level_[this->current_level_]);
      factor /= this->s_level_[this->current_level_]->Reduce();

      // Defect correction
      x->AddScale(*this->r_level_[this->current_level_], factor);
    } else
      // Defect correction
      x->AddScale(*this->r_level_[this->current_level_], ValueType(1.0));

    // Post-smoothing on finest level
    this->smoother_level_[this->current_level_]->InitMaxIter(this->iter_post_smooth_);
    this->smoother_level_[this->current_level_]->Solve(rhs, x);

    if (this->current_level_ == 0) {
      // Update residual
      this->op_->Apply(*x, this->r_level_[this->current_level_]);
      this->r_level_[this->current_level_]->ScaleAdd(ValueType(-1.0), rhs);

      this->res_norm_ = this->Norm(*this->r_level_[this->current_level_]);
    }

  } else
    // Coarse grid solver
    this->solver_coarse_->SolveZeroSol(rhs, x);

  LOG_DEBUG(this, "MultiGrid::Vcycle_()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Wcycle_(const VectorType &rhs, VectorType *x) {

  LOG_INFO("MultiGrid:Wcycle_() not implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Fcycle_(const VectorType &rhs, VectorType *x) {

  LOG_INFO("MultiGrid:Fcycle_() not implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Kcycle_(const VectorType &rhs, VectorType *x) {

  LOG_INFO("MultiGrid:Kcycle_() not implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}

// do nothing
template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SolveNonPrecond_(const VectorType &rhs,
                                                              VectorType *x) {

  LOG_INFO("MultiGrid:SolveNonPrecond_() this function is disabled - something is very wrong if you are calling it ...");
  FATAL_ERROR(__FILE__, __LINE__);


}

// do nothing
template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SolvePrecond_(const VectorType &rhs,
                                                            VectorType *x) {

  LOG_INFO("MultiGrid:SolvePrecond_() this function is disabled - something is very wrong if you are calling it ...");
  FATAL_ERROR(__FILE__, __LINE__);


}


template class MultiGrid< LocalMatrix<double>, LocalVector<double>, double >;
template class MultiGrid< LocalMatrix<float>,  LocalVector<float>, float >;

}

