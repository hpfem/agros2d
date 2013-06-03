// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2013 Dimitar Lukarski
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

  this->levels_ = -1;
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

  this->solver_coarse_ = NULL;
  this->smoother_level_ = NULL;

  this->scaling_ = true;

}

template <class OperatorType, class VectorType, typename ValueType>
MultiGrid<OperatorType, VectorType, ValueType>::~MultiGrid() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::InitLevels(const int levels) {

  assert(this->build_ == false);
  assert(levels > 0);

  this->levels_ = levels;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::InitOperator(const bool type) {

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

  assert(this->build_ == false);
  assert(op != NULL );

  this->op_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSmoother(IterativeLinearSolver<OperatorType, VectorType, ValueType> **smoother) {

//  assert(this->build_ == false); not possible due to AMG
  assert(smoother != NULL);

  this->smoother_level_ = smoother;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSmootherPreIter(const int iter) {

  this->iter_pre_smooth_ = iter;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSmootherPostIter(const int iter) {

  this->iter_post_smooth_ = iter;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetSolver(Solver<OperatorType, VectorType, ValueType> &solver) {

//  assert(this->build_ == false); not possible due to AMG
  assert(&solver != NULL);

  this->solver_coarse_ = &solver;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetRestrictOperator(OperatorType **op) {

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->operator_type_ == true);

  this->restrict_op_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetRestrictVector(LocalVector<int> **op) {

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->operator_type_ == false);

  this->restrict_vector_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetProlongOperator(OperatorType **op) {

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->operator_type_ == true);

  this->prolong_op_level_ = op;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetScaling(const bool scaling) {

  this->scaling_ = scaling;

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

  // Setup finest level 0
  this->smoother_level_[0]->SetOperator(*this->op_);
  this->smoother_level_[0]->Build();

  // Setup coarser levels
  for (int i=1; i<this->levels_-1; ++i) {
    this->smoother_level_[i]->SetOperator(*this->op_level_[i-1]);
    this->smoother_level_[i]->Build();
  }

  // Setup coarse grid solver
  this->solver_coarse_->SetOperator(*op_level_[this->levels_-2]);
  this->solver_coarse_->Build();

  // Setup all temporary vectors for the cycles - needed on all levels
  this->d_level_ = new VectorType*[this->levels_];
  this->r_level_ = new VectorType*[this->levels_];
  this->t_level_ = new VectorType*[this->levels_];
  this->s_level_ = new VectorType*[this->levels_];

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

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Clear(void) {

  if (this->build_ == true) {

    for (int i=0; i<this->levels_; ++i) {

      // Leave solution VectorType (d_level_[0])
      if (i > 0)
        delete this->d_level_[i];

      // Clear temporary VectorTypes
      delete this->r_level_[i];
      delete this->t_level_[i];
      delete this->s_level_[i];
    }

    delete[] this->d_level_;
    delete[] this->r_level_;
    delete[] this->t_level_;
    delete[] this->s_level_;

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
      if (i > 0)
        this->d_level_[i]->MoveToHost();
      this->t_level_[i]->MoveToHost();
      this->s_level_[i]->MoveToHost();

      if (this->operator_type_) {
        this->restrict_op_level_[i]->MoveToHost();
        this->prolong_op_level_[i]->MoveToHost();
      } else
        this->restrict_vector_level_[i]->MoveToHost();
    }

    if (this->precond_ != NULL) {
      this->precond_->MoveToHost();
    }

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

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
      if (i > 0)
        this->d_level_[i]->MoveToAccelerator();
      this->t_level_[i]->MoveToAccelerator();
      this->s_level_[i]->MoveToAccelerator();

      if (this->operator_type_) {
        this->restrict_op_level_[i]->MoveToAccelerator();
        this->prolong_op_level_[i]->MoveToAccelerator();
      } else
        this->restrict_vector_level_[i]->MoveToAccelerator();
    }

    if (this->precond_ != NULL) {
      this->precond_->MoveToAccelerator();
    }

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                                 VectorType *x) {

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_ != NULL);
  assert(this->build_ == true);

  if (this->verb_ > 0) {
    this->PrintStart_();
    this->iter_ctrl_.PrintInit();
  }

  this->SolveV(rhs, x);

  if (this->verb_ > 0) {
    this->iter_ctrl_.PrintStatus();
    this->PrintEnd_();
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SolveV(const VectorType &rhs,
                                                                  VectorType *x) {

  int levels = this->levels_;

  assert(this->levels_ > 1);
  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->r_level_[levels-1] != NULL);
  assert(this->d_level_[levels-1] != NULL);
  assert(this->t_level_[levels-1] != NULL);
  assert(this->s_level_[levels-1] != NULL);
  assert(this->precond_  == NULL);
  assert(this->build_ == true);
  assert(this->solver_coarse_ != NULL);

  for (int i=0; i<levels-1; ++i) {
    if (i>0)
      assert(this->op_level_[i] != NULL);
    assert(this->smoother_level_[i] != NULL);
    assert(this->r_level_[i] != NULL);
    if (i > 0)
      assert(this->d_level_[i] != NULL);
    assert(this->t_level_[i] != NULL);
    assert(this->s_level_[i] != NULL);

    if (this->operator_type_) {
      assert(this->restrict_op_level_[i] != NULL);
      assert(this->prolong_op_level_[i] != NULL);
    } else
      assert(this->restrict_vector_level_[i] != NULL);
  }

  // initial residual = b - Ax
  this->op_->Apply(*x, this->t_level_[0]);
  this->t_level_[0]->ScaleAdd(ValueType(-1.0), rhs);

  ValueType res_norm;
  res_norm = this->t_level_[0]->Norm();

  this->iter_ctrl_.InitResidual(res_norm);

  this->d_level_[0] = x;
  ValueType scale1, scale2;
  bool scaling = this->scaling_;

  while (!this->iter_ctrl_.CheckResidual(res_norm)) {

    // Set vectors to zero
    for (int i=1; i<levels; ++i)
      this->d_level_[i]->Zeros();

    // Restrict residual vector on finest level
    this->Restrict_(*this->t_level_[0], this->r_level_[1], 0);

    // Loop over levels until coarsest level is reached
    for (int i=1; i<levels-1; ++i) {

      // Pre-smoothing
      this->smoother_level_[i]->InitMaxIter(this->iter_pre_smooth_);
      this->smoother_level_[i]->Solve(*r_level_[i], d_level_[i]);

      if (scaling) {
        // Scaling
        if (i < levels-2 && this->iter_pre_smooth_ > 0) {
          t_level_[i]->PointWiseMult(*r_level_[i], *d_level_[i]);
          scale1 = t_level_[i]->Reduce();
          this->op_level_[i-1]->Apply(*d_level_[i],t_level_[i]);
          t_level_[i]->PointWiseMult(*d_level_[i]);
          scale2 = t_level_[i]->Reduce();
          d_level_[i]->Scale(scale1/scale2);
        }
      }

      // Update residual
      this->op_level_[i-1]->Apply(*this->d_level_[i], this->t_level_[i]);
      this->t_level_[i]->ScaleAdd(ValueType(-1.0), *this->r_level_[i]);

      // Restrict residual vector
      this->Restrict_(*this->t_level_[i], this->r_level_[i+1], i);

    }

    this->solver_coarse_->Solve(*this->r_level_[levels-1], this->d_level_[levels-1]);

    for (int i=levels-2; i>=1; --i) {

      // Prolong solution vector
      this->Prolong_(*this->d_level_[i+1], this->t_level_[i], i);

      if (scaling && (i < levels-2)) {
        // Scaling factor + defect correction
        s_level_[i]->PointWiseMult(*r_level_[i], *t_level_[i]);
        scale1 = s_level_[i]->Reduce();
        this->op_level_[i-1]->Apply(*t_level_[i], s_level_[i]);
        s_level_[i]->PointWiseMult(*t_level_[i]);
        scale2 = s_level_[i]->Reduce();
      
        // Defect correction
        this->d_level_[i]->AddScale(*this->t_level_[i], scale1/scale2);
      } else
        this->d_level_[i]->AddScale(*this->t_level_[i], ValueType(1.0));

      // Post-smoothing
      this->smoother_level_[i]->InitMaxIter(this->iter_post_smooth_);
      this->smoother_level_[i]->Solve(*this->r_level_[i], this->d_level_[i]);

    }

    // Prolong solution vector on finest level
    this->Prolong_(*this->d_level_[1], this->r_level_[0], 0);

    if (scaling) {
      // Scaling factor
      s_level_[0]->PointWiseMult(*r_level_[0], *t_level_[0]);
      scale1 = s_level_[0]->Reduce();
      this->op_->Apply(*r_level_[0], s_level_[0]);
      s_level_[0]->PointWiseMult(*r_level_[0]);
      scale2 = s_level_[0]->Reduce();

      // Defect correction + scaling factor
      this->d_level_[0]->AddScale(*this->r_level_[0], scale1/scale2);
    } else
      // Defect correction + scaling factor
      this->d_level_[0]->AddScale(*this->r_level_[0], ValueType(1.0));

    // Post-smoothing on finest level
    this->smoother_level_[0]->InitMaxIter(this->iter_post_smooth_);
    this->smoother_level_[0]->Solve(rhs, this->d_level_[0]);

    // Update residual
    this->op_->Apply(*this->d_level_[0], this->t_level_[0]); 
    this->t_level_[0]->ScaleAdd(ValueType(-1.0), rhs);

    res_norm = this->t_level_[0]->Norm();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SolveW(const VectorType &rhs,
                                                                  VectorType *x) {

  LOG_INFO("MultiGrid:SolveW() not implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SolveF(const VectorType &rhs,
                                                                  VectorType *x) {

  LOG_INFO("MultiGrid:SolveW() not implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Restrict_(const VectorType &fine,
                                                                     VectorType *coarse,
                                                               const int level) {

  if (this->operator_type_)
    this->restrict_op_level_[level]->Apply(fine, coarse);
  else
    coarse->Restriction(fine, *this->restrict_vector_level_[level]);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Prolong_(const VectorType &coarse,
                                                                    VectorType *fine,
                                                              const int level) {

  if (this->operator_type_)
    this->prolong_op_level_[level]->Apply(coarse, fine);
  else
    fine->Prolongation(coarse, *this->restrict_vector_level_[level]);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Vcycle_() {



}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Wcycle_() {



}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::Fcycle_() {



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

};
