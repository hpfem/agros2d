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


#include "inversion.hpp"

#include "../../base/local_matrix.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"

#include <assert.h>
#include <math.h>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
Inversion<OperatorType, VectorType, ValueType>::Inversion() {

  LOG_DEBUG(this, "Inversion::Inversion()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
Inversion<OperatorType, VectorType, ValueType>::~Inversion() {

  LOG_DEBUG(this, "Inversion::~Inversion()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::Print(void) const {
  
  LOG_INFO("Inversion solver");
  
}


template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  LOG_INFO("Inversion direct solver starts");

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  LOG_INFO("Inversion ends");

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "Inversion::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());
  assert(this->op_->get_nrow() > 0);

  this->inverse_.CloneFrom(*this->op_);
  this->inverse_.Invert();

  LOG_DEBUG(this, "Inversion::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "Inversion::Clear()",
            this->build_);

  if (this->build_ == true) {

    this->inverse_.Clear();
    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "Inversion::MoveToHostLocalData_()",
            this->build_);  

  if (this->build_ == true)
    this->inverse_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "Inversion::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true)
    this->inverse_.MoveToAccelerator();

}

template <class OperatorType, class VectorType, typename ValueType>
void Inversion<OperatorType, VectorType, ValueType>::Solve_(const VectorType &rhs, VectorType *x) {

  LOG_DEBUG(this, "Inversion::Solve_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(&this->inverse_ != NULL);
  assert(this->build_ == true);

  this->inverse_.Apply(rhs, x);

  LOG_DEBUG(this, "Inversion::Solve_()",
            " #*# end");

}


template class Inversion< LocalMatrix<double>, LocalVector<double>, double >;
template class Inversion< LocalMatrix<float>,  LocalVector<float>, float >;

}

