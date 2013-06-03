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

#include "preconditioner_multicolored_gs.hpp"
#include "preconditioner_multicolored.hpp"
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

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredSGS<OperatorType, VectorType, ValueType>::MultiColoredSGS() {

  this->omega_ = ValueType(1.0);

}

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredSGS<OperatorType, VectorType, ValueType>::~MultiColoredSGS() {

  this->Clear();

}

// TODO
// not optimal implementation - scale the diagonal vectors in the building phase
template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SetRelaxation(const ValueType omega) {

  this->omega_ = omega ;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Multicolored Symmetric Gauss-Seidel (SGS) preconditioner");  

  if (this->build_ == true) {
    LOG_INFO("number of colors = " << this->num_blocks_ ); 
  }


}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::ReBuildNumeric(void) {


  if (this->preconditioner_ != NULL) {
    this->preconditioner_->Clear();
    delete this->preconditioner_;
  }
  
  for(int i=0; i<this->num_blocks_; ++i) {
    
    delete this->x_block_[i] ;
    delete this->diag_block_[i];
    delete this->diag_solver_[i];

    for(int j=0; j<this->num_blocks_; ++j)
      delete this->preconditioner_block_[i][j];

    delete[] this->preconditioner_block_[i];

  }

  delete[] this->preconditioner_block_;
  delete[] this->x_block_;
  delete[] this->diag_block_;
  delete[] this->diag_solver_;

  this->preconditioner_ = new OperatorType ;
  this->preconditioner_->CloneFrom(*this->op_);

  this->Permute_();
  this->Factorize_();
  this->Decompose_();

}


template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::PostAnalyse_(void) {

  assert(this->build_ == true);  
  this->preconditioner_->LAnalyse(false);
  this->preconditioner_->UAnalyse(false);
  
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SolveL_(void) {

  assert(this->build_ == true);

  for (int i=0; i<this->num_blocks_; ++i){

    for (int j=0; j<i; ++j)
      this->preconditioner_block_[i][j]->ApplyAdd(*this->x_block_[j],
                                                  ValueType(-1.0),
                                                  this->x_block_[i]);

    this->diag_solver_[i]->Solve(*this->x_block_[i],
                                 this->x_block_[i]);

    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(ValueType(1.0)/this->omega_);
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SolveD_(void) {

  assert(this->build_ == true);

  for (int i=0; i<this->num_blocks_; ++i) {
    this->x_block_[i]->PointWiseMult(*this->diag_block_[i]);

    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(this->omega_/(ValueType(2.0) - this->omega_));

  }
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SolveR_(void) {

  assert(this->build_ == true);
  
  for (int i=this->num_blocks_-1; i>=0; --i){
    
    for (int j=this->num_blocks_-1; j>i; --j)
      this->preconditioner_block_[i][j]->ApplyAdd(*this->x_block_[j],
                                                  ValueType(-1.0),
                                                  this->x_block_[i]);

    this->diag_solver_[i]->Solve(*this->x_block_[i],
                                 this->x_block_[i]);
    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(ValueType(1.0)/this->omega_);

  }


}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::Solve_(const VectorType &rhs,
                                                                  VectorType *x) {

  this->x_.CopyFromPermute(rhs,
                           this->permutation_);   
  

  this->preconditioner_->LSolve(this->x_, x);
  
  x->PointWiseMult(this->diag_);

  this->preconditioner_->USolve(*x, &this->x_);
  
  x->CopyFromPermuteBackward(this->x_,
                             this->permutation_);

}











template <class OperatorType, class VectorType, typename ValueType>
MultiColoredGS<OperatorType, VectorType, ValueType>::MultiColoredGS() {
}

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredGS<OperatorType, VectorType, ValueType>::~MultiColoredGS() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Multicolored Gauss-Seidel (GS) preconditioner");  

  if (this->build_ == true) {
    LOG_INFO("number of colors = " << this->num_blocks_ ); 
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::PostAnalyse_(void) {

  assert(this->build_ == true);  
  this->preconditioner_->UAnalyse(false);
  
}


template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::SolveL_(void) {
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::SolveD_(void) {
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::SolveR_(void) {

  assert(this->build_ == true);
  
  for (int i=this->num_blocks_-1; i>=0; --i){
    
    for (int j=this->num_blocks_-1; j>i; --j)
      this->preconditioner_block_[i][j]->ApplyAdd(*this->x_block_[j],
                                                  ValueType(-1.0),
                                                  this->x_block_[i]);
    
    this->diag_solver_[i]->Solve(*this->x_block_[i], 
                                 this->x_block_[i]);

    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(ValueType(1.0)/this->omega_);

  }


}


template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::Solve_(const VectorType &rhs,
                                                                 VectorType *x) {

  LOG_INFO("No implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}


template class MultiColoredSGS< LocalMatrix<double>, LocalVector<double>, double >;
template class MultiColoredSGS< LocalMatrix<float>,  LocalVector<float>, float >;

template class MultiColoredGS< LocalMatrix<double>, LocalVector<double>, double >;
template class MultiColoredGS< LocalMatrix<float>,  LocalVector<float>, float >;

};
