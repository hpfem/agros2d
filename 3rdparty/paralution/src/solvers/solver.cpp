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

#include "solver.hpp"
#include "../base/global_matrix.hpp"
#include "../base/local_matrix.hpp"

#include "../base/global_stencil.hpp"
#include "../base/local_stencil.hpp"

#include "../base/global_vector.hpp"
#include "../base/local_vector.hpp"

#include "../utils/log.hpp"

#include <assert.h>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
Solver<OperatorType, VectorType, ValueType>::Solver() {

  this->op_  = NULL;
  this->precond_   = NULL;

  this->build_   = false;

}

template <class OperatorType, class VectorType, typename ValueType>
Solver<OperatorType, VectorType, ValueType>::~Solver() {

  // the preconditioner is defined outsite
  this->op_  = NULL;
  this->precond_   = NULL;

  this->build_   = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::SetOperator(const OperatorType &op) {

  assert(this->build_ == false);

  this->op_  = &op;

}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::ResetOperator(const OperatorType &op) {

  // TODO
  //  assert(this->build_ != false);

  this->op_  = &op;

}


template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::SolveZeroSol(const VectorType &rhs,
                                                               VectorType *x) {

  x->Zeros();
  this->Solve(rhs, x);

}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::Build(void) {

  // by default - nothing to build

  if (this->build_ == true)
    this->Clear();

  this->build_ = true;

}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::ReBuildNumeric(void) {

  // by default - just reBuild everything
  this->Clear();
  this->Build();


}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::Clear(void) {

  if (this->precond_ != NULL)
    delete this->precond_;

  this->op_  = NULL;
  this->precond_   = NULL;

  this->build_   = false;

}


template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::MoveToHost(void) {

  if ( this->permutation_.get_size() > 0)
    this->permutation_.MoveToHost();

  if (this->precond_ != NULL)
    this->precond_->MoveToHost();

  // move all local data too
  this->MoveToHostLocalData_();

}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::MoveToAccelerator(void) {

  if ( this->permutation_.get_size() > 0)
    this->permutation_.MoveToAccelerator();

  if (this->precond_ != NULL)
    this->precond_->MoveToAccelerator();

  // move all local data too
  this->MoveToAcceleratorLocalData_();

}

template <class OperatorType, class VectorType, typename ValueType>
void Solver<OperatorType, VectorType, ValueType>::Verbose(const int verb) {

  this->verb_ = verb; 

}







template <class OperatorType, class VectorType, typename ValueType>
IterativeLinearSolver<OperatorType, VectorType, ValueType>::IterativeLinearSolver() {

  this->verb_ = 1 ;

}

template <class OperatorType, class VectorType, typename ValueType>
IterativeLinearSolver<OperatorType, VectorType, ValueType>::~IterativeLinearSolver() {
}


template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::Init(const double abs_tol,
                                                             const double rel_tol,
                                                             const double div_tol,
                                                             const int max_iter) {

  this->iter_ctrl_.Init(abs_tol, rel_tol, div_tol, max_iter);

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::InitMaxIter(const int max_iter) {

  this->iter_ctrl_.InitMaximumIterations(max_iter);

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::InitTol(const ValueType abs,
                                                                const ValueType rel,
                                                                const ValueType div) {
  this->iter_ctrl_.InitTolerance(abs, rel, div);
}

template <class OperatorType, class VectorType, typename ValueType>
int IterativeLinearSolver<OperatorType, VectorType, ValueType>::GetIterationCount(void) {

  return this->iter_ctrl_.GetIterationCount();

}

template <class OperatorType, class VectorType, typename ValueType>
ValueType IterativeLinearSolver<OperatorType, VectorType, ValueType>::GetCurrentResidual(void) {

  return this->iter_ctrl_.GetCurrentResidual();

}

template <class OperatorType, class VectorType, typename ValueType>
int IterativeLinearSolver<OperatorType, VectorType, ValueType>::GetSolverStatus(void) {

  return this->iter_ctrl_.GetSolverStatus();

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::RecordResidualHistory(void) {

  this->iter_ctrl_.RecordHistory();

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::RecordHistory(std::string filename) const {

  this->iter_ctrl_.WriteHistoryToFile(filename);

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::Verbose(const int verb) {

  this->verb_ = verb; 
  this->iter_ctrl_.Verbose(verb);

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                              VectorType *x) {

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_ != NULL);
  assert(this->build_ == true);

  if (this->verb_ > 0) {
    this->PrintStart_();
    this->iter_ctrl_.PrintInit();
  }

  if (this->precond_ == NULL) {

    this->SolveNonPrecond_(rhs, x);

  } else {

    this->SolvePrecond_(rhs, x);

  }

  if (this->verb_ > 0) {
    this->iter_ctrl_.PrintStatus();
    this->PrintEnd_();
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void IterativeLinearSolver<OperatorType, VectorType, ValueType>::SetPreconditioner(Solver<OperatorType, VectorType, ValueType> &precond){

  assert(this != &precond);
  this->precond_ = &precond;

}










template <class OperatorType, class VectorType, typename ValueType>
FixedPoint<OperatorType, VectorType, ValueType>::FixedPoint() {

  this->omega_ = 1.0;

}

template <class OperatorType, class VectorType, typename ValueType>
FixedPoint<OperatorType, VectorType, ValueType>::~FixedPoint() {
  this->Clear();
}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::SetRelaxation(const ValueType omega) {

  this->omega_ = omega;

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::Print(void) const {
  
  if (this->precond_ == NULL) { 
    
    LOG_INFO("Fixed Point Iteration solver");
    
  } else {
    
    LOG_INFO("Fixed Point Iteration solver, with preconditioner:");
    this->precond_->Print();

  }

  
}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  assert(this->precond_ != NULL);
  LOG_INFO("Fixed Point Iteration solver starts with");
  this->precond_->Print();

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  LOG_INFO("Fixed Point Iteration solver ends");

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::ReBuildNumeric(void) {

  if (this->build_ == true) {

    this->x_old_.Zeros();
    this->x_res_.Zeros();
    
    this->iter_ctrl_.Clear();

    this->precond_->ReBuildNumeric();

  } else {

    this->Clear();
    this->Build();

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::Clear(void) {

  if (this->build_ == true) {

    if (this->precond_ != NULL) {
      this->precond_->Clear();
      this->precond_   = NULL;    
    }

    this->x_old_.Clear();
    this->x_res_.Clear();
    
    this->iter_ctrl_.Clear();

    this->build_ = false;
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  assert(this->precond_ != NULL);
  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());

  this->build_ = true;

  this->x_old_.CloneBackend(*this->op_);
  this->x_old_.Allocate("x_old", this->op_->get_nrow());

  this->x_res_.CloneBackend(*this->op_);
  this->x_res_.Allocate("x_res", this->op_->get_nrow());

  this->precond_->SetOperator(*this->op_);
  
  this->precond_->Build();

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::SolveNonPrecond_(const VectorType &rhs,
                                                                       VectorType *x) {
  LOG_INFO("Preconditioner for the Fixed Point method is required");
  FATAL_ERROR(__FILE__, __LINE__);
}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::SolvePrecond_(const VectorType &rhs,
                                                                    VectorType *x) {

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_ != NULL);
  assert(this->build_ == true);

  if (this->iter_ctrl_.GetMaximumIterations() > 0) {

    // inital residual x_res = b - Ax
    this->op_->Apply(*x, &this->x_res_);
    this->x_res_.ScaleAdd(ValueType(-1.0), rhs);
    
    this->iter_ctrl_.InitResidual(this->x_res_.Norm());
    
    // Solve M x_old = x_res
    this->precond_->SolveZeroSol(this->x_res_, &this->x_old_);
    
    // x = x + x_old
    x->ScaleAdd(ValueType(1.0), this->x_old_);
    
    // x_res = b - Ax
    this->op_->Apply(*x, &this->x_res_);
    this->x_res_.ScaleAdd(ValueType(-1.0), rhs); 
    
    while (!this->iter_ctrl_.CheckResidual(this->x_res_.Norm())) {
      
      // Solve M x_old = x_res
      this->precond_->SolveZeroSol(this->x_res_, &this->x_old_);
      
      // x = x + omega*x_old
      x->AddScale(this->x_old_, this->omega_);
      
      // x_res = b - Ax
      this->op_->Apply(*x, &this->x_res_);
      this->x_res_.ScaleAdd(ValueType(-1.0), rhs); 
      
    }
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  if (this->build_ == true) {

    this->x_old_.MoveToHost();
    this->x_res_.MoveToHost();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void FixedPoint<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  if (this->build_ == true) {

    this->x_old_.MoveToAccelerator();
    this->x_res_.MoveToAccelerator();

  }

}


template class Solver< LocalMatrix<double>, LocalVector<double>, double >;
template class Solver< LocalMatrix<float>,  LocalVector<float>, float >;

template class Solver< LocalStencil<double>, LocalVector<double>, double >;
template class Solver< LocalStencil<float>,  LocalVector<float>, float >;

template class Solver< GlobalMatrix<double>, GlobalVector<double>, double >;
template class Solver< GlobalMatrix<float>,  GlobalVector<float>, float >;

template class Solver< GlobalStencil<double>, GlobalVector<double>, double >;
template class Solver< GlobalStencil<float>,  GlobalVector<float>, float >;


template class IterativeLinearSolver< LocalMatrix<double>, LocalVector<double>, double >;
template class IterativeLinearSolver< LocalMatrix<float>,  LocalVector<float>, float >;

template class IterativeLinearSolver< LocalStencil<double>, LocalVector<double>, double >;
template class IterativeLinearSolver< LocalStencil<float>,  LocalVector<float>, float >;

template class IterativeLinearSolver< GlobalMatrix<double>, GlobalVector<double>, double >;
template class IterativeLinearSolver< GlobalMatrix<float>,  GlobalVector<float>, float >;

template class IterativeLinearSolver< GlobalStencil<double>, GlobalVector<double>, double >;
template class IterativeLinearSolver< GlobalStencil<float>,  GlobalVector<float>, float >;


template class FixedPoint< LocalMatrix<double>, LocalVector<double>, double >;
template class FixedPoint< LocalMatrix<float>,  LocalVector<float>, float >;

template class FixedPoint< LocalStencil<double>, LocalVector<double>, double >;
template class FixedPoint< LocalStencil<float>,  LocalVector<float>, float >;

template class FixedPoint< GlobalMatrix<double>, GlobalVector<double>, double >;
template class FixedPoint< GlobalMatrix<float>,  GlobalVector<float>, float >;

template class FixedPoint< GlobalStencil<double>, GlobalVector<double>, double >;
template class FixedPoint< GlobalStencil<float>,  GlobalVector<float>, float >;


};
