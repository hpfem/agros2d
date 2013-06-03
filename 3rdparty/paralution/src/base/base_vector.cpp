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

#include "base_vector.hpp"
#include "backend_manager.hpp"
#include "../utils/log.hpp"

#include <stdlib.h>
#include <assert.h>
#include <fstream>

namespace paralution {

template <typename ValueType>
BaseVector<ValueType>::BaseVector() {

  this->size_ = 0;

}

template <typename ValueType>
BaseVector<ValueType>::~BaseVector() {
}

template <typename ValueType>
void BaseVector<ValueType>::set_backend(const Paralution_Backend_Descriptor local_backend) {

  this->local_backend_ = local_backend;

}

template <typename ValueType>
void BaseVector<ValueType>::CopyFromFloat(const BaseVector<float> &vec) {

  LOG_INFO("CopyFromFloat(const BaseVector<float> &vec)");
  this->info();
  vec.info();
  LOG_INFO("Float casting is not available for this backend");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseVector<ValueType>::CopyFromDouble(const BaseVector<double> &vec) {

  LOG_INFO("CopyFromDouble(const BaseVector<double> &vec)");
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
AcceleratorVector<ValueType>::AcceleratorVector() {
}

template <typename ValueType>
AcceleratorVector<ValueType>::~AcceleratorVector() {
}



template class BaseVector<double>;
template class BaseVector<float>;

template class AcceleratorVector<double>;
template class AcceleratorVector<float>;

template class BaseVector<int>;

template class AcceleratorVector<int>;


}
