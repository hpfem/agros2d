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


#include "base_vector.hpp"
#include "backend_manager.hpp"
#include "../utils/log.hpp"

#include <stdlib.h>
#include <assert.h>
#include <fstream>

namespace paralution {

template <typename ValueType>
BaseVector<ValueType>::BaseVector() {

  LOG_DEBUG(this, "BaseVector::BaseVector()",
            "default constructor");

  this->size_ = 0;

}

template <typename ValueType>
BaseVector<ValueType>::~BaseVector() {

  LOG_DEBUG(this, "BaseVector::~BaseVector()",
            "default destructor");

}

template <typename ValueType>
inline int BaseVector<ValueType>::get_size(void) const { 
  
  return this->size_; 

}

template <typename ValueType>
void BaseVector<ValueType>::set_backend(const Paralution_Backend_Descriptor local_backend) {

  this->local_backend_ = local_backend;

}

template <typename ValueType>
bool BaseVector<ValueType>::Check(void) const {

  LOG_INFO("BaseVector::Check()");
  this->info();
  LOG_INFO("Only host version!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseVector<ValueType>::Assemble(const int *i, const ValueType *v,
                                     int size, const int n) {
  
  LOG_INFO("BaseVector::Assemble()");
  this->info();
  LOG_INFO("Only host version!");
  FATAL_ERROR(__FILE__, __LINE__);
  
}

template <typename ValueType>
void BaseVector<ValueType>::CopyFromFloat(const BaseVector<float> &vec) {

  LOG_INFO("BaseVector::CopyFromFloat(const BaseVector<float> &vec)");
  this->info();
  vec.info();
  LOG_INFO("Float casting is not available for this backend");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseVector<ValueType>::CopyFromDouble(const BaseVector<double> &vec) {

  LOG_INFO("BaseVector::CopyFromDouble(const BaseVector<double> &vec)");
  this->info();
  vec.info();
  LOG_INFO("Float casting is not available for this backend");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
bool BaseVector<ValueType>::Restriction(const BaseVector<ValueType> &vec_fine, 
                                        const BaseVector<int> &map) {
  return false;
}

template <typename ValueType>
bool BaseVector<ValueType>::Prolongation(const BaseVector<ValueType> &vec_coarse, 
                                        const BaseVector<int> &map) {
  return false;
}


template <typename ValueType>
void BaseVector<ValueType>::CopyFromAsync(const BaseVector<ValueType> &vec) {

  // default is no async
  LOG_VERBOSE_INFO(4, "*** info: BaseVector::CopyFromAsync() no async available)");        

  this->CopyFrom(vec);

}

template <typename ValueType>
void BaseVector<ValueType>::CopyToAsync(BaseVector<ValueType> *vec) const {

  // default is no async
  LOG_VERBOSE_INFO(4, "*** info: BaseVector::CopyToAsync() no async available)");        

  this->CopyTo(vec);

}







template <typename ValueType>
AcceleratorVector<ValueType>::AcceleratorVector() {
}

template <typename ValueType>
AcceleratorVector<ValueType>::~AcceleratorVector() {
}


template <typename ValueType>
void AcceleratorVector<ValueType>::CopyFromHostAsync(const HostVector<ValueType> &src) {

  // default is no async
  this->CopyFromHost(src);

}


template <typename ValueType>
void AcceleratorVector<ValueType>::CopyToHostAsync(HostVector<ValueType> *dst) const {

  // default is no async
  this->CopyToHostAsync(dst);

}


template class BaseVector<double>;
template class BaseVector<float>;
template class BaseVector<int>;

template class AcceleratorVector<double>;
template class AcceleratorVector<float>;
template class AcceleratorVector<int>;

}
