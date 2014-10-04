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


#include "preconditioner_ai.hpp"
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
AIChebyshev<OperatorType, VectorType, ValueType>::AIChebyshev() {

  LOG_DEBUG(this, "AIChebyshev::AIChebyshev()",
            "default constructor");


  this->p_ = 0;
  this->lambda_min_ = 0;
  this->lambda_max_ = 0;

}

template <class OperatorType, class VectorType, typename ValueType>
AIChebyshev<OperatorType, VectorType, ValueType>::~AIChebyshev() {

  LOG_DEBUG(this, "AIChebyshev::~AIChebyshev()",
            "destructor");


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
void AIChebyshev<OperatorType, VectorType, ValueType>::Set(const int p, const ValueType lambda_min, const ValueType lambda_max) {

  LOG_DEBUG(this, "AIChebyshev::Set()",
            "p=" << p <<
            " lambda_min=" << lambda_min <<
            " lambda_max=" << lambda_max);

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

  LOG_DEBUG(this, "AIChebyshev::Build()",
            this->build_ <<
            " #*# begin");


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

  LOG_DEBUG(this, "AIChebyshev::Build()",
            this->build_ <<
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "AIChebyshev::Clear()",
            this->build_);

  this->AIChebyshev_.Clear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "AIChebyshev::MoveToHostLocalData_()",
            this->build_);  


  this->AIChebyshev_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "AIChebyshev::MoveToAcceleratorLocalData_()",
            this->build_);


  this->AIChebyshev_.MoveToAccelerator();

}

template <class OperatorType, class VectorType, typename ValueType>
void AIChebyshev<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                    VectorType *x) {


  LOG_DEBUG(this, "AIChebyshev::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);


  this->AIChebyshev_.Apply(rhs, x);

  LOG_DEBUG(this, "AIChebyshev::Solve()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
FSAI<OperatorType, VectorType, ValueType>::FSAI() {

  LOG_DEBUG(this, "FSAI::FSAI()",
            "default constructor");


  this->op_mat_format_ = false;
  this->precond_mat_format_ = CSR;

  this->matrix_power_ = 1;
  this->external_pattern_ = false;
  this->matrix_pattern_ = NULL;

}

template <class OperatorType, class VectorType, typename ValueType>
FSAI<OperatorType, VectorType, ValueType>::~FSAI() {
  
  LOG_DEBUG(this, "FSAI::~FSAI()",
            "destructor");

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
void FSAI<OperatorType, VectorType, ValueType>::Set(const int power) {

  LOG_DEBUG(this, "FSAI::Set()",
            power);


  assert(this->build_ == false);
  assert(power > 0);

  this->matrix_power_ = power;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Set(const OperatorType &pattern) {

  LOG_DEBUG(this, "FSAI::Set()",
            "");

  assert(this->build_ == false);
  assert(&pattern != NULL);

  this->matrix_pattern_ = &pattern;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "FSAI::Build()",
            this->build_ <<
            " #*# begin");


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

  LOG_DEBUG(this, "FSAI::Clear()",
            this->build_);


  if (this->build_ == true) {

    this->FSAI_L_.Clear();
    this->FSAI_LT_.Clear();

    this->t_.Clear();

    this->op_mat_format_ = false;
    this->precond_mat_format_ = CSR;

    this->build_ = false;

  }

  LOG_DEBUG(this, "FSAI::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::SetPrecondMatrixFormat(const unsigned int mat_format) {

  LOG_DEBUG(this, "FSAI::SetPrecondMatrixFormat()",
            mat_format);


  this->op_mat_format_ = true;
  this->precond_mat_format_ = mat_format;

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "FSAI::MoveToHostLocalData_()",
            this->build_);  


  this->FSAI_L_.MoveToHost();
  this->FSAI_LT_.MoveToHost();

  this->t_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "FSAI::MoveToAcceleratorLocalData_()",
            this->build_);

  this->FSAI_L_.MoveToAccelerator();
  this->FSAI_LT_.MoveToAccelerator();

  this->t_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void FSAI<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs, VectorType *x) {

  LOG_DEBUG(this, "FSAI::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->FSAI_L_.Apply(rhs, &this->t_);
  this->FSAI_LT_.Apply(this->t_, x);

  LOG_DEBUG(this, "FSAI::Solve()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
SPAI<OperatorType, VectorType, ValueType>::SPAI() {

  LOG_DEBUG(this, "SPAI::SPAI()",
            "default constructor");

  this->op_mat_format_ = false;
  this->precond_mat_format_ = CSR;

}

template <class OperatorType, class VectorType, typename ValueType>
SPAI<OperatorType, VectorType, ValueType>::~SPAI() {

  LOG_DEBUG(this, "SPAI::~SPAI()",
            "destructor");


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

  LOG_DEBUG(this, "SPAI::Build()",
            this->build_ <<
            " #*# begin");


  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->SPAI_.CloneFrom(*this->op_);
  this->SPAI_.SPAI();

  if (this->op_mat_format_ == true)
    this->SPAI_.ConvertTo(this->precond_mat_format_);

  LOG_DEBUG(this, "SPAI::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "SPAI::Clear()",
            this->build_);

  if (this->build_ == true) {

    this->SPAI_.Clear();

    this->op_mat_format_ = false;
    this->precond_mat_format_ = CSR;

    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::SetPrecondMatrixFormat(const unsigned int mat_format) {

  LOG_DEBUG(this, "SPAI::SetPrecondMatrixFormat()",
            mat_format);

  this->op_mat_format_ = true;
  this->precond_mat_format_ = mat_format;

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "SPAI::MoveToHostLocalData_()",
            this->build_);  

  this->SPAI_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "SPAI::MoveToAcceleratorLocalData_()",
            this->build_);

  this->SPAI_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void SPAI<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs, VectorType *x) {

  LOG_DEBUG(this, "SPAI::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->SPAI_.Apply(rhs, x);

  LOG_DEBUG(this, "SPAI::Solve()",
            " #*# end");

}


template class AIChebyshev< LocalMatrix<double>, LocalVector<double>, double >;
template class AIChebyshev< LocalMatrix<float>,  LocalVector<float>, float >;

template class FSAI< LocalMatrix<double>, LocalVector<double>, double >;
template class FSAI< LocalMatrix<float>,  LocalVector<float>, float >;

template class SPAI< LocalMatrix<double>, LocalVector<double>, double >;
template class SPAI< LocalMatrix<float>,  LocalVector<float>, float >;

}

