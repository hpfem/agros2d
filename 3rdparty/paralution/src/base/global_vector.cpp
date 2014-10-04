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


#include "global_vector.hpp"
#include "local_vector.hpp"

#include "../utils/log.hpp"

#include <assert.h>

namespace paralution {

template <typename ValueType>
GlobalVector<ValueType>::GlobalVector() {

  this->object_name_ = "";
  FATAL_ERROR(__FILE__, __LINE__);    
  
}

template <typename ValueType>
GlobalVector<ValueType>::~GlobalVector() {

  this->Clear();

}

template <typename ValueType>
int GlobalVector<ValueType>::get_size(void) const { 
  return 0;
}


template <typename ValueType>
void GlobalVector<ValueType>::Allocate(std::string name, const unsigned int size) {
}

template <typename ValueType>
void GlobalVector<ValueType>::Clear(void) {
}

template <typename ValueType>
void GlobalVector<ValueType>::Zeros(void) {
}

template <typename ValueType>
void GlobalVector<ValueType>::Ones(void) {
}

template <typename ValueType>
void GlobalVector<ValueType>::SetValues(const ValueType val) {
}

template <typename ValueType>
void GlobalVector<ValueType>::SetRandom(const ValueType a, const ValueType b, const int seed) {
}

template <typename ValueType>
void GlobalVector<ValueType>::CopyFrom(const GlobalVector<ValueType> &src) {
}

template <typename ValueType>
void GlobalVector<ValueType>::CloneFrom(const GlobalVector<ValueType> &src) {
}

template <typename ValueType>
void GlobalVector<ValueType>::MoveToAccelerator(void) {
}

template <typename ValueType>
void GlobalVector<ValueType>::MoveToHost(void) {
}

template <typename ValueType>
ValueType&  GlobalVector<ValueType>::operator[](const unsigned int i) { 
  // not correct
  return this->vector_interior_[i];
}

template <typename ValueType>
const ValueType&  GlobalVector<ValueType>::operator[](const unsigned int i) const { 
  // not correct
  return this->vector_interior_[i];
}

template <typename ValueType>
void GlobalVector<ValueType>::info(void) const {
}

template <typename ValueType>
void GlobalVector<ValueType>::ReadFileASCII(const std::string name) {
}

template <typename ValueType>
void GlobalVector<ValueType>::WriteFileASCII(const std::string name) const {
}

template <typename ValueType>
void GlobalVector<ValueType>::ReadFileBinary(const std::string name) {
}

template <typename ValueType>
void GlobalVector<ValueType>::WriteFileBinary(const std::string name) const {
}

template <typename ValueType>
void GlobalVector<ValueType>::AddScale(const GlobalVector<ValueType> &x, const ValueType alpha) {          
}

template <typename ValueType>
void GlobalVector<ValueType>::ScaleAdd(const ValueType alpha, const GlobalVector<ValueType> &x) {
}

template <typename ValueType>
void GlobalVector<ValueType>::Scale(const ValueType alpha) {
}

template <typename ValueType>
ValueType GlobalVector<ValueType>::Dot(const GlobalVector<ValueType> &x) const {
  return ValueType(0.0);
}

template <typename ValueType>
ValueType GlobalVector<ValueType>::Norm(void) const {
  return ValueType(0.0);
}

template <typename ValueType>
ValueType GlobalVector<ValueType>::Reduce(void) const {
  return ValueType(0.0);
}

template <typename ValueType>
ValueType GlobalVector<ValueType>::Asum(void) const {
  return ValueType(0.0);
}

template <typename ValueType>
int GlobalVector<ValueType>::Amax(ValueType &value) const {
  return 0;
}

template <typename ValueType>
void GlobalVector<ValueType>::PointWiseMult(const GlobalVector<ValueType> &x) {
}

template <typename ValueType>
void GlobalVector<ValueType>::PointWiseMult(const GlobalVector<ValueType> &x, const GlobalVector<ValueType> &y) {
}



template class GlobalVector<double>;
template class GlobalVector<float>;

}
