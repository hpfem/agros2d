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

  LOG_DEBUG(this, "Preconditioner::Preconditioner()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
Preconditioner<OperatorType, VectorType, ValueType>::~Preconditioner() {

  LOG_DEBUG(this, "Preconditioner::~Preconditioner()",
            "destructor");

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

  LOG_DEBUG(this, "Preconditioner::SolveZeroSol()",
            "calling this->Solve()");

  this->Solve(rhs, x);

}


template <class OperatorType, class VectorType, typename ValueType>
Jacobi<OperatorType, VectorType, ValueType>::Jacobi() {

  LOG_DEBUG(this, "Jacobi::Jacobi()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
Jacobi<OperatorType, VectorType, ValueType>::~Jacobi() {

  LOG_DEBUG(this, "Jacobi::~Jacobi()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Jacobi preconditioner");

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "Jacobi::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->inv_diag_entries_.CloneBackend(*this->op_);
  this->op_->ExtractInverseDiagonal(&this->inv_diag_entries_);

  LOG_DEBUG(this, "Jacobi::Build()",
            this->build_ <<
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::ResetOperator(const OperatorType &op) {

  LOG_DEBUG(this, "Jacobi::ResetOperator()",
            this->build_);

  assert(this->op_ != NULL);

  this->inv_diag_entries_.Clear();
  this->inv_diag_entries_.CloneBackend(*this->op_);
  this->op_->ExtractInverseDiagonal(&this->inv_diag_entries_);

}


template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "Jacobi::Clear()",
            this->build_);

  this->inv_diag_entries_.Clear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                        VectorType *x) {

  LOG_DEBUG(this, "Jacobi::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);

  if (x != &rhs) {

    x->PointWiseMult(this->inv_diag_entries_,
                     rhs);

  } else {

    x->PointWiseMult(this->inv_diag_entries_);

  }

  LOG_DEBUG(this, "Jacobi::Solve()",
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "Jacobi::MoveToHostLocalData_()",
            this->build_);  

  this->inv_diag_entries_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void Jacobi<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "Jacobi::MoveToAcceleratorLocalData_()",
            this->build_);


  this->inv_diag_entries_.MoveToAccelerator();

}




template <class OperatorType, class VectorType, typename ValueType>
GS<OperatorType, VectorType, ValueType>::GS() {

  LOG_DEBUG(this, "GS::GS()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
GS<OperatorType, VectorType, ValueType>::~GS() {

  LOG_DEBUG(this, "GS::~GS()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Gauss-Seidel (GS) preconditioner");

}

template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "GS::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->GS_.CloneFrom(*this->op_);
  this->GS_.LAnalyse(false);
  this->GS_.UAnalyse(false);

  LOG_DEBUG(this, "GS::Build()",
            this->build_ <<
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::ResetOperator(const OperatorType &op) {

  LOG_DEBUG(this, "GS::ResetOperator()",
            this->build_);

  assert(this->op_ != NULL);

  this->GS_.Clear();
  this->GS_.CloneFrom(*this->op_);
  this->GS_.LAnalyse(false);
  this->GS_.UAnalyse(false);

}


template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "GS::Clear()",
            this->build_);

  this->GS_.Clear();

  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                        VectorType *x) {

  LOG_DEBUG(this, "GS::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);

  this->GS_.LSolve(rhs, x);

  LOG_DEBUG(this, "GS::Solve()",
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "GS::MoveToHostLocalData_()",
            this->build_);

  this->GS_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void GS<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "GS::MoveToAcceleratorLocalData_()",
            this->build_);

  this->GS_.MoveToAccelerator();

}






template <class OperatorType, class VectorType, typename ValueType>
SGS<OperatorType, VectorType, ValueType>::SGS() {

  LOG_DEBUG(this, "SGS::SGS()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
SGS<OperatorType, VectorType, ValueType>::~SGS() {

  LOG_DEBUG(this, "SGS::~SGS()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Symmetric Gauss-Seidel (SGS) preconditioner");

}

template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "SGS::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->SGS_.CloneFrom(*this->op_);
  this->SGS_.LAnalyse(false);
  this->SGS_.UAnalyse(false);

  this->diag_entries_.CloneBackend(*this->op_);
  this->SGS_.ExtractInverseDiagonal(&this->diag_entries_);

  this->v_.CloneBackend(*this->op_);
  this->v_.Allocate("v", this->op_->get_nrow());

  LOG_DEBUG(this, "SGS::Build()",
            this->build_ <<
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::ResetOperator(const OperatorType &op) {

  LOG_DEBUG(this, "SGS::ResetOperator()",
            this->build_);

  assert(this->op_ != NULL);

  this->SGS_.Clear();
  this->SGS_.CloneFrom(*this->op_);
  this->SGS_.LAnalyse(false);
  this->SGS_.UAnalyse(false);

  this->diag_entries_.Clear();
  this->diag_entries_.CloneBackend(*this->op_);
  this->SGS_.ExtractDiagonal(&this->diag_entries_);

  this->v_.Clear();
  this->v_.CloneBackend(*this->op_);
  this->v_.Allocate("v", this->op_->get_nrow());

}


template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "SGS::Clear()",
            this->build_);

  this->SGS_.Clear();
  this->diag_entries_.Clear();
  this->v_.Clear();

  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                     VectorType *x) {

  LOG_DEBUG(this, "SGS::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);

  this->SGS_.LSolve(rhs, &this->v_);
  this->v_.PointWiseMult(this->diag_entries_);
  this->SGS_.USolve(this->v_, x);

  LOG_DEBUG(this, "SGS::Solve()",
            " #*# end");


}

template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "SGS::MoveToHostLocalData_()",
            this->build_);

  this->SGS_.MoveToHost();
  this->diag_entries_.MoveToHost();
  this->v_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void SGS<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "SGS::MoveToAcceleratorLocalData_()",
            this->build_);

  this->SGS_.MoveToAccelerator();
  this->diag_entries_.MoveToAccelerator();
  this->v_.MoveToAccelerator();

}







template <class OperatorType, class VectorType, typename ValueType>
ILU<OperatorType, VectorType, ValueType>::ILU() {

  LOG_DEBUG(this, "ILU::ILU()",
            "default constructor");

  this->p_ = 0;
  this->level_ = true;

}

template <class OperatorType, class VectorType, typename ValueType>
ILU<OperatorType, VectorType, ValueType>::~ILU() {

  LOG_DEBUG(this, "ILU::ILU()",
            "destructor");

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
void ILU<OperatorType, VectorType, ValueType>::Set(const int p, const bool level) {

  LOG_DEBUG(this, "ILU::Set()",
            "p=" << p <<
            " level=" << level);


  assert(p >= 0);
  assert(this->build_ == false);

  this->p_     = p;
  this->level_ = level;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "ILU::Build()",
            this->build_ <<
            " #*# begin");

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

  LOG_DEBUG(this, "ILU::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "ILU::Clear()",
            this->build_);

  this->ILU_.Clear();
  this->ILU_.LUAnalyseClear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "ILU::MoveToHostLocalData_()",
            this->build_);  

  this->ILU_.MoveToHost();
  this->ILU_.LUAnalyse();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "ILU::MoveToAcceleratorLocalData_()",
            this->build_);

  this->ILU_.MoveToAccelerator();
  this->ILU_.LUAnalyse();

}


template <class OperatorType, class VectorType, typename ValueType>
void ILU<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                     VectorType *x) {

  LOG_DEBUG(this, "ILU::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->ILU_.LUSolve(rhs, x);

  LOG_DEBUG(this, "ILU::Solve()",
            " #*# end");
}







template <class OperatorType, class VectorType, typename ValueType>
ILUT<OperatorType, VectorType, ValueType>::ILUT() {

  LOG_DEBUG(this, "ILUT::ILUT()",
            "default constructor");


  this->t_ = ValueType(0.05);
  this->max_row_ = 100;

}

template <class OperatorType, class VectorType, typename ValueType>
ILUT<OperatorType, VectorType, ValueType>::~ILUT() {

  LOG_DEBUG(this, "ILUT::~ILUT()",
            "destructor");

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
void ILUT<OperatorType, VectorType, ValueType>::Set(const ValueType t) {

  LOG_DEBUG(this, "ILUT::Set()",
            t);


  assert(t >= 0);
  assert(this->build_ == false);

  this->t_ = t;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Set(const ValueType t, const int maxrow) {

  LOG_DEBUG(this, "ILUT::Set()",
            "t=" << t <<
            " maxrow=" << maxrow);


  assert(t >= 0);
  assert(this->build_ == false);

  this->t_ = t;
  this->max_row_ = maxrow;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "ILUT::Build()",
            this->build_ <<
            " #*# begin");


  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->ILUT_.CloneFrom(*this->op_);
  this->ILUT_.ILUTFactorize(this->t_, this->max_row_);
  this->ILUT_.LUAnalyse();

  LOG_DEBUG(this, "ILUT::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "ILUT::Clear()",
            this->build_);

  this->ILUT_.Clear();
  this->ILUT_.LUAnalyseClear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "ILUT::MoveToHostLocalData_()",
            this->build_);  

  this->ILUT_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "ILUT::MoveToAcceleratorLocalData_()",
            this->build_);

  this->ILUT_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void ILUT<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                     VectorType *x) {

  LOG_DEBUG(this, "ILUT::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->ILUT_.LUSolve(rhs, x);

  LOG_DEBUG(this, "ILUT::Solve()",
            " #*# begin");


}







template <class OperatorType, class VectorType, typename ValueType>
IC<OperatorType, VectorType, ValueType>::IC() {

  LOG_DEBUG(this, "IC::IC()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
IC<OperatorType, VectorType, ValueType>::~IC() {

  LOG_DEBUG(this, "IC::IC()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("IC preconditioner");

  if (this->build_ == true) {
    LOG_INFO("IC nnz = " << this->IC_.get_nnz());
  }

}


template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "IC::Build()",
            this->build_ <<
            " #*# begin");


  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);

  this->IC_.CloneBackend(*this->op_);
  this->inv_diag_entries_.CloneBackend(*this->op_);

  this->op_->ExtractL(&this->IC_, true);
  this->IC_.ICFactorize(&this->inv_diag_entries_);
  this->IC_.LLAnalyse();

  LOG_DEBUG(this, "IC::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "IC::Clear()",
            this->build_);

  this->inv_diag_entries_.Clear();
  this->IC_.Clear();
  this->IC_.LLAnalyseClear();
  this->build_ = false;

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "IC::MoveToHostLocalData_()",
            this->build_);  

  // this->inv_diag_entries_ is NOT needed on accelerator!
  this->IC_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "IC::MoveToAcceleratorLocalData_()",
            this->build_);

  // this->inv_diag_entries_ is NOT needed on accelerator!
  this->IC_.MoveToAccelerator();

}


template <class OperatorType, class VectorType, typename ValueType>
void IC<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                    VectorType *x) {

  LOG_DEBUG(this, "IC::Solve()",
            " #*# begin");

  assert(this->build_ == true);
  assert(x != NULL);
  assert(x != &rhs);

  this->IC_.LLSolve(rhs, this->inv_diag_entries_, x);
  
  LOG_DEBUG(this, "IC::Solve()",
            " #*# end");

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

template class GS< LocalMatrix<double>, LocalVector<double>, double >;
template class GS< LocalMatrix<float>,  LocalVector<float>, float >;

template class SGS< LocalMatrix<double>, LocalVector<double>, double >;
template class SGS< LocalMatrix<float>,  LocalVector<float>, float >;

template class ILU< LocalMatrix<double>, LocalVector<double>, double >;
template class ILU< LocalMatrix<float>,  LocalVector<float>, float >;

template class ILUT< LocalMatrix<double>, LocalVector<double>, double >;
template class ILUT< LocalMatrix<float>,  LocalVector<float>, float >;

template class IC< LocalMatrix<double>, LocalVector<double>, double >;
template class IC< LocalMatrix<float>,  LocalVector<float>, float >;

}

