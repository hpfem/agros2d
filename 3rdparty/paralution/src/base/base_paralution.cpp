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


#include "base_paralution.hpp"
#include <assert.h>
#include "../utils/log.hpp"


namespace paralution {

template <typename ValueType>
BaseParalution<ValueType>::BaseParalution() {

  LOG_DEBUG(this, "BaseParalution::BaseParalution()",
              "default constructor");

  // copy the backend description
  this->local_backend_ = _Backend_Descriptor;

  this->asyncf = false;
  
  assert(_Backend_Descriptor.init == true);

}

template <typename ValueType>
BaseParalution<ValueType>::BaseParalution(const BaseParalution<ValueType> &src) {

  LOG_DEBUG(this, "BaseParalution::BaseParalution()",
            "copy constructor");

  LOG_INFO("no copy constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
BaseParalution<ValueType>::~BaseParalution() {

  LOG_DEBUG(this, "BaseParalution::~BaseParalution()",
            "default destructor");

}

template<typename ValueType>
BaseParalution<ValueType>& BaseParalution<ValueType>::operator=(const BaseParalution<ValueType> &src) {

  LOG_DEBUG(this, "BaseParalution::operator=()",
            "");

  LOG_INFO("no overloaded operator=()");
  FATAL_ERROR(__FILE__, __LINE__);

}

template<typename ValueType>
void BaseParalution<ValueType>::CloneBackend(const BaseParalution<ValueType> &src) {

  LOG_DEBUG(this, "BaseParalution::CloneBackend()",
            "with the same ValueType");


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

  LOG_DEBUG(this, "BaseParalution::CloneBackend()",
            "with different ValueType");


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



template<typename ValueType>
void BaseParalution<ValueType>::MoveToAcceleratorAsync(void) {

  // default call
  this->MoveToAccelerator();

}

template<typename ValueType>
void BaseParalution<ValueType>::MoveToHostAsync(void) {

  // default call
  this->MoveToHost();

}

template<typename ValueType>
void BaseParalution<ValueType>::Sync(void) {

  _paralution_sync();
  this->asyncf = false;

}


template class BaseParalution<double>;
template class BaseParalution<float>;
template class BaseParalution<int>;

template void BaseParalution<int>::CloneBackend(const BaseParalution<double> &src);
template void BaseParalution<int>::CloneBackend(const BaseParalution<float> &src);

}
