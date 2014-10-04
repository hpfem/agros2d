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


#include "minres.hpp"
#include "../iter_ctrl.hpp"

#include "../../base/global_matrix.hpp"
#include "../../base/local_matrix.hpp"

#include "../../base/global_stencil.hpp"
#include "../../base/local_stencil.hpp"

#include "../../base/global_vector.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"
#include "../../utils/math_functions.hpp"

#include <assert.h>
#include <math.h>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
MINRES<OperatorType, VectorType, ValueType>::MINRES() {

  LOG_DEBUG(this, "MINRES::MINRES()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
MINRES<OperatorType, VectorType, ValueType>::~MINRES() {

  LOG_DEBUG(this, "MINRES::~MINRES()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::Print(void) const {
  
  if (this->precond_ == NULL) { 
    
    LOG_INFO("MINRES solver");
    
  } else {
    
    LOG_INFO("PMINRES solver, with preconditioner:");
    this->precond_->Print();

  }

  
}


template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("MINRES (non-precond) linear solver starts");

  } else {

    LOG_INFO("PMINRES solver starts, with preconditioner:");
    this->precond_->Print();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("MINRES (non-precond) ends");

  } else {

    LOG_INFO("PMINRES ends");

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "MINRES::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());
  assert(this->op_->get_nrow() > 0);


  if (this->precond_ != NULL) {
    
    this->precond_->SetOperator(*this->op_);

    this->precond_->Build();
    
    this->z_.CloneBackend(*this->op_);
    this->z_.Allocate("z", this->op_->get_nrow());
    
  } 

  this->v_.CloneBackend(*this->op_);
  this->v_.Allocate("v_k", this->op_->get_nrow());

  this->v_old_.CloneBackend(*this->op_);
  this->v_old_.Allocate("v_k-1", this->op_->get_nrow());

  this->v_new_.CloneBackend(*this->op_);
  this->v_new_.Allocate("v_k+1", this->op_->get_nrow());

  this->d_.CloneBackend(*this->op_);
  this->d_.Allocate("d_k", this->op_->get_nrow());

  this->d_old_.CloneBackend(*this->op_);
  this->d_old_.Allocate("d_k-1", this->op_->get_nrow());

  LOG_DEBUG(this, "MINRES::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "MINRES::Clear()",
            this->build_);

  if (this->build_ == true) {

    if (this->precond_ != NULL) {
      this->precond_->Clear();
      this->precond_   = NULL;
    }
    
    this->v_.Clear();
    this->v_old_.Clear();
    this->v_new_.Clear();
    this->d_.Clear();
    this->d_old_.Clear();
    this->z_.Clear();
    
    this->iter_ctrl_.Clear();
    
    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "MINRES::MoveToHostLocalData_()",
            this->build_);  

  if (this->build_ == true) {

    this->v_.MoveToHost();
    this->v_old_.MoveToHost();
    this->v_new_.MoveToHost();
    this->d_.MoveToHost();
    this->d_old_.MoveToHost();

    if (this->precond_ != NULL) {
      this->z_.MoveToHost();
      this->precond_->MoveToHost();
    }
    
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "MINRES::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->v_.MoveToAccelerator();
    this->v_old_.MoveToAccelerator();
    this->v_new_.MoveToAccelerator();
    this->d_.MoveToAccelerator();
    this->d_old_.MoveToAccelerator();

    if (this->precond_ != NULL) {
      this->z_.MoveToAccelerator();
      this->precond_->MoveToAccelerator();
    }
    
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::SolveNonPrecond_(const VectorType &rhs,
                                                              VectorType *x) {

  LOG_DEBUG(this, "MINRES::SolveNonPrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_  == NULL);
  assert(this->build_ == true);

  const OperatorType *op = this->op_;

  VectorType *v     = &this->v_;
  VectorType *v_old = &this->v_old_;
  VectorType *v_new = &this->v_new_;
  VectorType *d     = &this->d_;
  VectorType *d_old = &this->d_old_;

  ValueType alpha, beta;
  ValueType gamma1, gamma2;
  ValueType delta1, delta2;
  ValueType eps, eps_old;
  ValueType ck, sk;

  // initial residual v = b - Ax
  op->Apply(*x, v);
  v->ScaleAdd(ValueType(-1.0), rhs);

  ValueType res_norm = v->Norm();
  this->iter_ctrl_.InitResidual(res_norm);

  // Lanczos step

  // Normalize v_k
  v->Scale(ValueType(1.0)/res_norm);

  // v_k+1 = Av_k
  op->Apply(*v, v_new);

  // alpha = (v_k+1, v_k)
  alpha = v_new->Dot(*v);

  // v_k+1 = v_k+1 - alpha * v_k
  v_new->AddScale(*v, ValueType(-1.0)*alpha);

  // beta = ||v_k+1||_2
  beta = v_new->Norm();

  // v_k-1 = v_k
  v_old->CopyFrom(*v);

  // v_k = v_k+1
  v->CopyFrom(*v_new);

  // Previous left orthogonalization on middle two entries in last column of T_k
  gamma1 = ValueType(-1.0) * alpha;

  // Previous left orthogonalization to produce first two entries of T_k+1 e_k+1
  delta1 = beta;
  eps    = ValueType(0.0);

  // Current left orthogonalization to zero out beta_k+1
  gamma2 = ValueType(1.0) / sqrt(gamma1 * gamma1 + beta * beta);
  ck     = gamma1 * gamma2;
  sk     = beta * gamma2;

  // Residual norm
  res_norm *= sk;

  // Update solution
  d->CopyFrom(*v_old);
  d->Scale(gamma2);
  d_old->CopyFrom(*d);

  x->AddScale(*d, ck*res_norm);

  while (!this->iter_ctrl_.CheckResidual(res_norm)) {

    // Lanczos step

    // Normalize v_k
    v->Scale(ValueType(1.0)/beta);

    // v_k+1 = Av_k
    op->Apply(*v, v_new);

    // v_k+1 = v_k+1 - beta * v_k-1
    v_new->AddScale(*v_old, ValueType(-1.0)*beta);

    // alpha = (v_k+1, v_k)
    alpha = v_new->Dot(*v);

    // v_k+1 = v_k+1 - alpha * v_k
    v_new->AddScale(*v, ValueType(-1.0)*alpha);

    // beta = ||v_k+1||_2
    beta = v_new->Norm();

    // v_k-1 = v_k
    v_old->CopyFrom(*v);

    // v_k = v_k+1
    v->CopyFrom(*v_new);

    // Previous left orthogonalization on middle two entries in last column of T_k
    delta2 = ck * delta1 + sk * alpha;
    gamma1 = sk * delta1 - ck * alpha;

    // Previous left orthogonalization to produce first two entries of T_k+1 e_k+1
    eps_old = ValueType(-1.0) * eps;
    eps     = sk * beta;
    delta1  = ValueType(-1.0) * ck * beta;

    // Current left orthogonalization to zero out beta_k+1
    gamma2 = ValueType(1.0) / sqrt(gamma1 * gamma1 + beta * beta);
    ck     = gamma1 * gamma2;
    sk     = beta * gamma2;

    // Residual norm
    res_norm *= sk;

    // Update solution
    d->ScaleAdd2(ValueType(-1.0)*delta2, *v_old, 1.0, *d_old, eps_old);
    d->Scale(gamma2);
    d_old->CopyFrom(*d);

    x->AddScale(*d, ck*res_norm);

  }

  LOG_DEBUG(this, "MINRES::SolveNonPrecond_()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void MINRES<OperatorType, VectorType, ValueType>::SolvePrecond_(const VectorType &rhs,
                                                            VectorType *x) {

  LOG_DEBUG(this, "MINRES::SolvePrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_ != NULL);
  assert(this->build_ == true);

  LOG_INFO("Preconditioned MINRES not yet implemented");
  FATAL_ERROR(__FILE__, __LINE__);

  LOG_DEBUG(this, "MINRES::SolvePrecond_()",
            " #*# end");

}


template class MINRES< LocalMatrix<double>, LocalVector<double>, double >;
template class MINRES< LocalMatrix<float>,  LocalVector<float>, float >;

template class MINRES< LocalStencil<double>, LocalVector<double>, double >;
template class MINRES< LocalStencil<float>,  LocalVector<float>, float >;

template class MINRES< GlobalMatrix<double>, GlobalVector<double>, double >;
template class MINRES< GlobalMatrix<float>,  GlobalVector<float>, float >;

template class MINRES< GlobalStencil<double>, GlobalVector<double>, double >;
template class MINRES< GlobalStencil<float>,  GlobalVector<float>, float >;

}

