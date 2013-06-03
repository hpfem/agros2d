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

#include "base_paralution.hpp"
#include <assert.h>

namespace paralution {

template <typename ValueType>
BaseParalution<ValueType>::BaseParalution() {

  // copy the backend description
  this->local_backend_ = _Backend_Descriptor;

}

template <typename ValueType>
BaseParalution<ValueType>::~BaseParalution() {
}

template<typename ValueType>
void BaseParalution<ValueType>::CloneBackend(const BaseParalution<ValueType> &src) {


  assert(this != &src);

  this->local_backend_ = src.local_backend_; 

  if (src.is_host()) {

    // move to host
    this->MoveToHost();
    
  } else {

    assert(src.is_accel());

    // move to accelerator
    this->MoveToAccelerator();

  }

}


template <typename ValueType>
template<typename ValueType2>
void BaseParalution<ValueType>::CloneBackend(const BaseParalution<ValueType2> &src) {


  this->local_backend_ = src.local_backend_; 

  if (src.is_host()) {

    // move to host
    this->MoveToHost();
    
  } else {

    assert(src.is_accel());

    // move to accelerator
    this->MoveToAccelerator();

  }

}

template class BaseParalution<double>;
template class BaseParalution<float>;
template class BaseParalution<int>;

template void BaseParalution<int>::CloneBackend(const BaseParalution<double> &src);
template void BaseParalution<int>::CloneBackend(const BaseParalution<float> &src);

}
