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

#include "preconditioner.hpp"
#include "../solver.hpp"
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
Preconditioner<OperatorType, VectorType, ValueType>::Preconditioner() {
}

template <class OperatorType, class VectorType, typename ValueType>
Preconditioner<OperatorType, VectorType, ValueType>::~Preconditioner() {
}

template <class OperatorType, class VectorType, typename ValueType>
void Preconditioner<OperatorType, VectorType, ValueType>::PrintStart_(void) const {
  // do nothing
}

template <class OperatorType, class VectorType, typename ValueType>
void Preconditioner<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {
  // do nothing
}

template <class OperatorType, class VectorType, typename ValueType>
void Preconditioner<OperatorType, VectorType, ValueType>::SolveZeroSol(const VectorType &rhs,
                                                                       VectorType *x) {

  this->Solve(rhs, x);

}


template <class OperatorType, class VectorType, typename ValueType>
Jacobi<OperatorType, VectorType, ValueType>::Jacobi() {
}

template <class OperatorType, class VectorType, typename ValueType>
Jacobi<OperatorType, VectorType, ValueType>::~Jacobi() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Jacobi preconditioner");

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->inv_diag_entries_.CloneBackend(*this->op_);
  this->op_->ExtractInverseDiagonal(&this->inv_diag_entries_);

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::ResetOperator(const OperatorType &op) {

  assert(this->op_ != NULL);

  this->inv_diag_entries_.Clear();
  this->inv_diag_entries_.CloneBackend(*this->op_);
  this->op_->ExtractInverseDiagonal(&this->inv_diag_entries_);

}


template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Clear(void) {

  this->inv_diag_entries_.Clear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                        VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);

  if (x != &rhs) {

    x->PointWiseMult(this->inv_diag_entries_,
                     rhs);

  } else {

    x->PointWiseMult(this->inv_diag_entries_);

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->inv_diag_entries_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->inv_diag_entries_.MoveToAccelerator();

}









template <class OperatorType, class VectorType, typename ValueType>
ILU<OperatorType, VectorType, ValueType>::ILU() {

  this->p_ = 0;
  this->level_ = true;

}

template <class OperatorType, class VectorType, typename ValueType>
ILU<OperatorType, VectorType, ValueType>::~ILU() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("ILU(" << this->p_ <<") preconditioner");

  if (this->build_ == true) {
    LOG_INFO("ILU nnz = " << this->ILU_.get_nnz());
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Init(const int p, const bool level) {

  assert(p >= 0);
  assert(this->build_ == false);

  this->p_     = p;
  this->level_ = level;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->ILU_.CloneFrom(*this->op_);

  if (this->p_ == 0 ) {
    // ILU0
    this->ILU_.ILU0Factorize();

  } else {
    // ILUp
    this->ILU_.ILUpFactorize(this->p_, this->level_);
  }

  this->ILU_.LUAnalyse();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Clear(void) {

  this->ILU_.Clear();
  this->ILU_.LUAnalyseClear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->ILU_.MoveToHost();
  this->ILU_.LUAnalyse();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->ILU_.MoveToAccelerator();
  this->ILU_.LUAnalyse();

}


template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                     VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->ILU_.LUSolve(rhs, x);

}







template <class OperatorType, class VectorType, typename ValueType>
ILUT<OperatorType, VectorType, ValueType>::ILUT() {

  this->t_ = 0.05;
  this->max_row_ = 100;

}

template <class OperatorType, class VectorType, typename ValueType>
ILUT<OperatorType, VectorType, ValueType>::~ILUT() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("ILUT(" << this->t_ <<"," << this->max_row_ <<") preconditioner");

  if (this->build_ == true) {
    LOG_INFO("ILUT nnz = " << this->ILUT_.get_nnz());
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Init(const ValueType t) {

  assert(t >= 0);
  assert(this->build_ == false);

  this->t_ = t;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Init(const ValueType t, const int maxrow) {

  assert(t >= 0);
  assert(this->build_ == false);

  this->t_ = t;
  this->max_row_ = maxrow;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->ILUT_.CloneFrom(*this->op_);
  this->ILUT_.ILUTFactorize(this->t_, this->max_row_);
  this->ILUT_.LUAnalyse();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Clear(void) {

  this->ILUT_.Clear();
  this->ILUT_.LUAnalyseClear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->ILUT_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->ILUT_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                     VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->ILUT_.LUSolve(rhs, x);

}







template <class OperatorType, class VectorType, typename ValueType>
IC<OperatorType, VectorType, ValueType>::IC() {

}

template <class OperatorType, class VectorType, typename ValueType>
IC<OperatorType, VectorType, ValueType>::~IC() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("IC(0) preconditioner");

  if (this->build_ == true) {
    LOG_INFO("IC nnz = " << this->IC_.get_nnz());
  }

}


template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->IC_.CloneFrom(*this->op_);
  this->IC_.IC0Factorize();
  this->IC_.LLAnalyse();

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Clear(void) {

  this->IC_.Clear();
  this->IC_.LLAnalyseClear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->IC_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->IC_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                    VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->IC_.LLSolve(rhs, x);

}





template <class OperatorType, class VectorType, typename ValueType>
AIChebyshev<OperatorType, VectorType, ValueType>::AIChebyshev() {

  this->p_ = 0;
  this->lambda_min_ = 0;
  this->lambda_max_ = 0;
}

template <class OperatorType, class VectorType, typename ValueType>
AIChebyshev<OperatorType, VectorType, ValueType>::~AIChebyshev() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Approximate Inverse Chebyshev(" << this->p_ <<") preconditioner");

  if (this->build_ == true) {
    LOG_INFO("AI matrix nnz = " << this->AIChebyshev_.get_nnz());
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Init(const int p, const ValueType lambda_min, const ValueType lambda_max) {

  assert(p > 0);
  assert(lambda_min != ValueType(0.0));
  assert(lambda_max != ValueType(0.0));
  assert(this->build_ == false);

  this->p_ = p;
  this->lambda_min_ = lambda_min;
  this->lambda_max_ = lambda_max;

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->AIChebyshev_.CloneFrom(*this->op_);

  const ValueType q = (1 - sqrt(this->lambda_min_/this->lambda_max_))/(1 + sqrt(this->lambda_min_/this->lambda_max_));
  ValueType c = 1 / sqrt(this->lambda_min_ * this->lambda_max_);

  // Shifting
  // Z = 2/(beta-alpha) [A-(beta+alpha)/2]
  OperatorType Z;
  Z.CloneFrom(*this->op_);

  Z.AddScalarDiagonal(ValueType(-1.0)*(this->lambda_max_ + this->lambda_min_)/(ValueType(2.0)));
  Z.ScaleDiagonal(ValueType(2.0)/(this->lambda_max_ - this->lambda_min_));


  // Chebyshev formula/series
  // ai = I c_0 / 2 + sum c_k T_k
  // Tk = 2 Z T_k-1 - T_k-2

  // 1st term
  // T_0 = I
  // ai = I c_0 / 2
  this->AIChebyshev_.AddScalarDiagonal(c/ValueType(2.0));

  OperatorType Tkm2;
  Tkm2.CloneFrom(Z);
  // 2nd term
  // T_1 = Z
  // ai = ai + c_1 Z
  c = c * ValueType(-1.0)*q;
  this->AIChebyshev_.MatrixAdd(Tkm2, ValueType(1.0),
                               c, true);

  // T_2 = 2*Z*Z - I
  // + c (2*Z*Z - I)
  OperatorType Tkm1;
  Tkm1.CloneBackend(*this->op_);
  Tkm1.MatrixMult(Z, Z);
  Tkm1.Scale(2.0);
  Tkm1.AddScalarDiagonal(ValueType(-1.0));

  c = c * ValueType(-1.0)*q;
  this->AIChebyshev_.MatrixAdd(Tkm1, ValueType(1.0),
                               c, true);


  // T_k = 2 Z T_k-1 - T_k-2     
  OperatorType Tk;
  Tk.CloneBackend(*this->op_);

  for (int i=2; i<=this->p_; ++i){

    Tk.MatrixMult(Z, Tkm1);
    Tk.MatrixAdd(Tkm2, ValueType(2.0),
                 ValueType(-1.0), true);
    
    c = c * ValueType(-1.0)*q;
    this->AIChebyshev_.MatrixAdd(Tk, ValueType(1.0),
                                 c, true);
    
    if (i+1 <= this->p_) {
      Tkm2.CloneFrom(Tkm1);
      Tkm1.CloneFrom(Tk);
    }

  }



}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Clear(void) {

  this->AIChebyshev_.Clear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->AIChebyshev_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->AIChebyshev_.MoveToAccelerator();

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                    VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);


  this->AIChebyshev_.Apply(rhs, x);

}

template <class OperatorType, class VectorType, typename ValueType>
FSAI<OperatorType, VectorType, ValueType>::FSAI() {

  this->op_mat_format_ = false;
  this->precond_mat_format_ = CSR;

  this->matrix_power_ = 1;
  this->external_pattern_ = false;
  this->matrix_pattern_ = NULL;

}

template <class OperatorType, class VectorType, typename ValueType>
FSAI<OperatorType, VectorType, ValueType>::~FSAI() {

  this->Clear();
  this->matrix_pattern_ = NULL;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Factorized Sparse Approximate Inverse preconditioner");

  if (this->build_ == true) {
    LOG_INFO("FSAI matrix nnz = " << this->FSAI_L_.get_nnz()
                                   + this->FSAI_LT_.get_nnz()
                                   - this->FSAI_L_.get_nrow());
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Init(const int power) {

  assert(this->build_ == false);
  assert(power > 0);

  this->matrix_power_ = power;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Init(const OperatorType &pattern) {

  assert(this->build_ == false);
  assert(&pattern != NULL);

  this->matrix_pattern_ = &pattern;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->FSAI_L_.CloneFrom(*this->op_);
  this->FSAI_L_.FSAI(this->matrix_power_, this->matrix_pattern_);

  this->FSAI_LT_.CloneFrom(this->FSAI_L_);
  this->FSAI_LT_.Transpose();

  this->t_.CloneBackend(*this->op_);
  this->t_.Allocate("temporary", this->op_->get_nrow());

  if (this->op_mat_format_ == true) {
    this->FSAI_L_.ConvertTo(this->precond_mat_format_);
    this->FSAI_LT_.ConvertTo(this->precond_mat_format_);
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Clear(void) {

  if (this->build_ == true) {

    this->FSAI_L_.Clear();
    this->FSAI_LT_.Clear();

    this->t_.Clear();

    this->op_mat_format_ = false;
    this->precond_mat_format_ = CSR;

    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::SetPrecondMatrixFormat(const unsigned int mat_format) {

  this->op_mat_format_ = true;
  this->precond_mat_format_ = mat_format;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->FSAI_L_.MoveToHost();
  this->FSAI_LT_.MoveToHost();

  this->t_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->FSAI_L_.MoveToAccelerator();
  this->FSAI_LT_.MoveToAccelerator();

  this->t_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs, VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->FSAI_L_.Apply(rhs, &this->t_);
  this->FSAI_LT_.Apply(this->t_, x);

}


template <class OperatorType, class VectorType, typename ValueType>
SPAI<OperatorType, VectorType, ValueType>::SPAI() {

  this->op_mat_format_ = false;
  this->precond_mat_format_ = CSR;

}

template <class OperatorType, class VectorType, typename ValueType>
SPAI<OperatorType, VectorType, ValueType>::~SPAI() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("SParse Approximate Inverse preconditioner");

  if (this->build_ == true) {
    LOG_INFO("SPAI matrix nnz = " << this->SPAI_.get_nnz());
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::Build(void) {

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->SPAI_.CloneFrom(*this->op_);
  this->SPAI_.SPAI();

  if (this->op_mat_format_ == true)
    this->SPAI_.ConvertTo(this->precond_mat_format_);

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::Clear(void) {

  if (this->build_ == true) {

    this->SPAI_.Clear();

    this->op_mat_format_ = false;
    this->precond_mat_format_ = CSR;

    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::SetPrecondMatrixFormat(const unsigned int mat_format) {

  this->op_mat_format_ = true;
  this->precond_mat_format_ = mat_format;

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  this->SPAI_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  this->SPAI_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs, VectorType *x) {

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->SPAI_.Apply(rhs, x);

}


template class Preconditioner< LocalMatrix<double>, LocalVector<double>, double >;
template class Preconditioner< LocalMatrix<float>,  LocalVector<float>, float >;

template class Preconditioner< LocalStencil<double>, LocalVector<double>, double >;
template class Preconditioner< LocalStencil<float>,  LocalVector<float>, float >;

template class Preconditioner< GlobalMatrix<double>, GlobalVector<double>, double >;
template class Preconditioner< GlobalMatrix<float>,  GlobalVector<float>, float >;

template class Preconditioner< GlobalStencil<double>, GlobalVector<double>, double >;
template class Preconditioner< GlobalStencil<float>,  GlobalVector<float>, float >;


template class Jacobi< LocalMatrix<double>, LocalVector<double>, double >;
template class Jacobi< LocalMatrix<float>,  LocalVector<float>, float >;

template class ILU< LocalMatrix<double>, LocalVector<double>, double >;
template class ILU< LocalMatrix<float>,  LocalVector<float>, float >;

template class ILUT< LocalMatrix<double>, LocalVector<double>, double >;
template class ILUT< LocalMatrix<float>,  LocalVector<float>, float >;

template class IC< LocalMatrix<double>, LocalVector<double>, double >;
template class IC< LocalMatrix<float>,  LocalVector<float>, float >;

template class AIChebyshev< LocalMatrix<double>, LocalVector<double>, double >;
template class AIChebyshev< LocalMatrix<float>,  LocalVector<float>, float >;

template class FSAI< LocalMatrix<double>, LocalVector<double>, double >;
template class FSAI< LocalMatrix<float>,  LocalVector<float>, float >;

template class SPAI< LocalMatrix<double>, LocalVector<double>, double >;
template class SPAI< LocalMatrix<float>,  LocalVector<float>, float >;

}

