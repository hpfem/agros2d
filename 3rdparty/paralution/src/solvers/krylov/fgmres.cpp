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


#include "fgmres.hpp"
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
FGMRES<OperatorType, VectorType, ValueType>::FGMRES() {

  LOG_DEBUG(this, "FGMRES::FGMRES()",
            "default constructor");

  this->size_basis_ = 30;

}


template <class OperatorType, class VectorType, typename ValueType>
FGMRES<OperatorType, VectorType, ValueType>::~FGMRES() {

  LOG_DEBUG(this, "FGMRES::~FGMRES()",
            "destructor");

  this->Clear();

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::Print(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("FGMRES solver");

  } else {

    LOG_INFO("FGMRES solver, with preconditioner:");
    this->precond_->Print();

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("FGMRES(" <<this->size_basis_ <<") (non-precond) linear solver starts");

  } else {

    LOG_INFO("FGMRES(" <<this->size_basis_ <<") solver starts, with preconditioner:");
    this->precond_->Print();

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("FGMRES(" <<this->size_basis_ <<") (non-precond) ends");

  } else {

    LOG_INFO("FGMRES(" <<this->size_basis_ <<") ends");

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "FGMRES::Build()",
            this->build_ <<
            " #*# begin");


  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  if (this->precond_ != NULL) {

    this->precond_->SetOperator(*this->op_);

    this->precond_->Build();

    this->z_ = new VectorType*[this->size_basis_+1];
    for (int i=0; i<this->size_basis_+1; ++i) {
      this->z_[i] = new VectorType;
      this->z_[i]->CloneBackend(*this->op_);
      this->z_[i]->Allocate("z", this->op_->get_nrow());
    }

  }

  this->w_.CloneBackend(*this->op_);
  this->w_.Allocate("w", this->op_->get_nrow());

  this->v_ = new VectorType*[this->size_basis_+1];
  for (int i = 0; i < this->size_basis_+1; ++i) {
    this->v_[i] = new VectorType;
    this->v_[i]->CloneBackend(*this->op_);
    this->v_[i]->Allocate("v", this->op_->get_nrow());
  }

  LOG_DEBUG(this, "FGMRES::Build()",
            this->build_ <<
            " #*# end");


}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "FGMRES::Clear()",
            this->build_);

  if (this->build_ == true) {

    if (this->precond_ != NULL) {
        this->precond_->Clear();
        this->precond_   = NULL;

        for (int i=0; i<this->size_basis_+1; ++i)
          delete this->z_[i];
        delete[] this->z_;
    }

    this->w_.Clear();

    for (int i = 0; i < this->size_basis_+1; ++i)
      delete this->v_[i];
    delete[] this->v_;

    this->iter_ctrl_.Clear();

    this->build_ = false;

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "FGMRES::MoveToHostLocalData_()",
            this->build_);  


  if (this->build_ == true) {

    this->w_.MoveToHost();

    for (int i = 0; i < this->size_basis_+1; ++i)
      this->v_[i]->MoveToHost();

    if (this->precond_ != NULL) {
      for (int i=0; i<this->size_basis_+1; ++i)
        this->z_[i]->MoveToHost();
      this->precond_->MoveToHost();
    }

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "FGMRES::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->w_.MoveToAccelerator();

    for (int i = 0; i < this->size_basis_+1; ++i)
      this->v_[i]->MoveToAccelerator();


    if (this->precond_ != NULL) {
      for (int i=0; i<this->size_basis_+1; ++i)
        this->z_[i]->MoveToAccelerator();
      this->precond_->MoveToAccelerator();
    }

  }

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::SetBasisSize(const int size_basis) {

  LOG_DEBUG(this, "FGMRES:SetBasisSize()",
            size_basis);

  assert(size_basis > 0);
  this->size_basis_ = size_basis;

}

template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::SolveNonPrecond_(const VectorType &rhs,
                                                                        VectorType *x) {

  LOG_DEBUG(this, "FGMRES::SolveNonPrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_  == NULL);
  assert(this->build_ == true);
  assert(this->size_basis_ > 0);

  const OperatorType *op = this->op_;

  VectorType *w  = &this->w_;
  VectorType **v = this->v_;

  int size_basis = this->size_basis_;

  std::vector<ValueType> c(size_basis);
  std::vector<ValueType> s(size_basis);
  std::vector<ValueType> H((size_basis+1)*size_basis, ValueType(0.0));
  std::vector<ValueType> sq(size_basis+1, ValueType(0.0));

  ValueType res_norm;


  // Compute residual V[0] = b - Ax
  op->Apply(*x, v[0]);
  v[0]->ScaleAdd(ValueType(-1.0), rhs);

  // res_norm = (v[0],v[0])
  res_norm = v[0]->Dot(*v[0]);
  res_norm = sqrt(res_norm);

  this->iter_ctrl_.InitResidual(res_norm);

  while (!this->iter_ctrl_.CheckResidual(res_norm)) {

    H.assign(H.size(), ValueType(0.0));
    sq.assign(sq.size(), ValueType(0.0));

    // Residual normalization
    // v = r / ||r||
    v[0]->Scale(ValueType(1.0)/res_norm);
    sq[0] = res_norm;
    /*
    for (int i = 0; i < (size_basis+1)*size_basis; i++)
      H[i] = ValueType(0.0);

    for (int i = 1; i < size_basis+1; i++)
      sq[i] = ValueType(0.0);
    */
    for (int i = 0; i < size_basis; ++i) {
      // w = A*v(i)
      op->Apply(*v[i], w);

      // Build Hessenberg matrix H
      for (int j = 0; j <= i; ++j) {
        // H(j,i) = <w,v(j)>
        H[j+i*(size_basis+1)] = w->Dot(*v[j]);

        // w = w - H(j,i) * v(j)
        w->AddScale( *v[j], ValueType(-1.0) * H[j+i*(size_basis+1)] );
      }

      // H(i+1,i) = ||w||
      H[i+1+i*(size_basis+1)] = w->Norm();

      // v(i+1) = w / H(i+1,i)
      w->Scale(ValueType(1.0) / H[i+1+i*(size_basis+1)]);
      v[i+1]->CopyFrom(*w);

      // Apply J(0),...,J(j-1) on ( H(0,i),...,H(i,i) )
      for (int k = 0; k < i; ++k)
        this->ApplyGivensRotation_( c[k], s[k], H[k+i*(size_basis+1)], H[k+1+i*(size_basis+1)] );

      // Construct J(i) (Givens rotation taken from wikipedia pseudo code)
      // TODO optimize me
      ValueType Hx = H[i+i*(size_basis+1)];
      ValueType Hy = H[i+1+i*(size_basis+1)];
      ValueType r = Hy;
      ValueType Hxy = Hx + Hy;

      if (fabs(Hx) > fabs(Hy))
        r = Hx;

      if (Hxy != 0.0) {
        ValueType rsq = r * r;
        if (r < 0.0)
          r *= sqrt(Hx * Hx / rsq + Hy * Hy / rsq);
        else
          r *= -sqrt(Hx * Hx / rsq + Hy * Hy / rsq);

        c[i] = Hx / r;
        s[i] = Hy / r;
      } else {
        c[i] = 1;
        s[i] = 0;
      }

      // Apply J(i) to H(i,i) and H(i,i+1) such that H(i,i+1) = 0
      this->ApplyGivensRotation_(c[i], s[i], H[i+i*(size_basis+1)], H[i+1+i*(size_basis+1)]);

      // Apply J(i) to the norm of the residual sg[i]
      this->ApplyGivensRotation_(c[i], s[i], sq[i], sq[i+1]);

      // Get current residual
      res_norm = fabs(sq[i+1]);

      if (this->iter_ctrl_.CheckResidual(res_norm)) {
        this->BackSubstitute_(sq, H, i);
        // Compute solution of least square problem
        for (int j = 0; j <= i; ++j)
          x->AddScale(*v[j], sq[j]);
        break;
      }

    }

    // If convergence has not been reached, RESTART
    if (!this->iter_ctrl_.CheckResidual(res_norm)) {
      this->BackSubstitute_(sq, H, size_basis-1);

      // Compute solution of least square problem
      for (int j = 0; j <= size_basis-1; ++j)
        x->AddScale(*v[j], sq[j]);

      // Compute residual with previous solution vector
      op->Apply(*x, v[0]);
      v[0]->ScaleAdd(ValueType(-1.0), rhs);

    }
  }

  LOG_DEBUG(this, "FGMRES::SolveNonPrecond_()",
            " #*# end");

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::SolvePrecond_(const VectorType &rhs,
                                                                     VectorType *x) {

  LOG_DEBUG(this, "FGMRES::SolvePrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_ != NULL);
  assert(this->build_ == true);
  assert(this->size_basis_ > 0);

  const OperatorType *op = this->op_;

  VectorType **z = this->z_;
  VectorType *w  = &this->w_;
  VectorType **v = this->v_;

  int size_basis = this->size_basis_;

  std::vector<ValueType> c(size_basis);
  std::vector<ValueType> s(size_basis);
  std::vector<ValueType> H((size_basis+1)*size_basis, ValueType(0.0));
  std::vector<ValueType> sq(size_basis+1, ValueType(0.0));

  ValueType res_norm;

  // Compute residual V[0] = b - Ax
  op->Apply(*x, v[0]);
  v[0]->ScaleAdd(ValueType(-1.0), rhs);

  // res_norm = (v[0],v[0])
  res_norm = v[0]->Dot(*v[0]);
  res_norm = sqrt(res_norm);

  this->iter_ctrl_.InitResidual(res_norm);

  while (!this->iter_ctrl_.CheckResidual(res_norm)) {

    H.assign(H.size(), ValueType(0.0));
    sq.assign(sq.size(), ValueType(0.0));

    // Residual normalization
    // v = r / ||r||
    v[0]->Scale(ValueType(1.0)/res_norm);
    sq[0] = res_norm;

    for (int i = 0; i < size_basis; ++i) {
      // Mz = v(j)
      this->precond_->SolveZeroSol(*v[i], z[i]);

      // w = A*z
      op->Apply(*z[i], w);

      // Build Hessenberg matrix H
      for (int j = 0; j <= i; ++j) {
        // H(j,i) = <w,v(j)>
        H[j+i*(size_basis+1)] = w->Dot(*v[j]);

        // w = w - H(j,i) * v(j)
        w->AddScale(*v[j], ValueType(-1.0) * H[j+i*(size_basis+1)]);
      }

      // H(i+1,i) = ||w||
      H[i+1+i*(size_basis+1)] = w->Norm();

      // v(i+1) = w / H(i+1,i)
      w->Scale(ValueType(1.0) / H[i+1+i*(size_basis+1)]);
      v[i+1]->CopyFrom(*w);

      // Apply J(0),...,J(j-1) on ( H(0,i),...,H(i,i) )
      for (int k = 0; k < i; ++k)
        this->ApplyGivensRotation_(c[k], s[k], H[k+i*(size_basis+1)], H[k+1+i*(size_basis+1)]);

      // Construct J(i) (Givens rotation taken from wikipedia pseudo code)
      ValueType Hx = H[i+i*(size_basis+1)];
      ValueType Hy = H[i+1+i*(size_basis+1)];
      ValueType r = Hy;
      ValueType Hxy = Hx + Hy;

      if (fabs(Hx) > fabs(Hy))
        r = Hx;

      if (Hxy != 0.0) {
        ValueType rsq = r * r;
        if (r < 0.0)
          r *= sqrt(Hx * Hx / rsq + Hy * Hy / rsq);
        else
          r *= -sqrt(Hx * Hx / rsq + Hy * Hy / rsq);

        c[i] = Hx / r;
        s[i] = Hy / r;
      } else {
        c[i] = 1;
        s[i] = 0;
      }

      // Apply J(i) to H(i,i) and H(i,i+1) such that H(i,i+1) = 0
      this->ApplyGivensRotation_(c[i], s[i], H[i+i*(size_basis+1)], H[i+1+i*(size_basis+1)]);

      // Apply J(i) to the norm of the residual sg[i]
      this->ApplyGivensRotation_(c[i], s[i], sq[i], sq[i+1]);

      // Get current residual
      res_norm = fabs(sq[i+1]);

      if (this->iter_ctrl_.CheckResidual(res_norm)) {
        this->BackSubstitute_(sq, H, i);

        // Compute solution of least square problem
        for (int j = 0; j <= i; ++j)
          x->AddScale(*z[j], sq[j]);

        break;
      }

    }

    // If convergence has not been reached, RESTART
    if (!this->iter_ctrl_.CheckResidual(res_norm)) {
      this->BackSubstitute_(sq, H, size_basis-1);

      // Compute solution of least square problem
      for (int j = 0; j <= size_basis-1; ++j)
        x->AddScale(*z[j], sq[j]);

      // Compute residual with previous solution vector
      op->Apply(*x, v[0]);
      v[0]->ScaleAdd(ValueType(-1.0), rhs);

    }

  }

  LOG_DEBUG(this, "FGMRES::SolvePrecond_()",
            " #*# end");

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::ApplyGivensRotation_(const ValueType &c, const ValueType &s,
                                                                            ValueType &x,       ValueType &y) const {

  ValueType temp = x;
  x =  c * x + s * y;
  y = -s * temp + c * y;

}


template <class OperatorType, class VectorType, typename ValueType>
void FGMRES<OperatorType, VectorType, ValueType>::BackSubstitute_(std::vector<ValueType> &sq,
                                                                  const std::vector<ValueType> &H,
                                                                  int k) const {

  for (int i = k; i >= 0; --i) {
    sq[i] = sq[i] / H[i+i*(this->size_basis_+1)];
    for (int j = i-1; j >= 0; --j)
      sq[j] = sq[j] - H[j+i*(this->size_basis_+1)] * sq[i];

  }

}


template class FGMRES< LocalMatrix<double>, LocalVector<double>, double >;
template class FGMRES< LocalMatrix<float>,  LocalVector<float>, float >;

template class FGMRES< LocalStencil<double>, LocalVector<double>, double >;
template class FGMRES< LocalStencil<float>,  LocalVector<float>, float >;

template class FGMRES< GlobalMatrix<double>, GlobalVector<double>, double >;
template class FGMRES< GlobalMatrix<float>,  GlobalVector<float>, float >;

template class FGMRES< GlobalStencil<double>, GlobalVector<double>, double >;
template class FGMRES< GlobalStencil<float>,  GlobalVector<float>, float >;

};
