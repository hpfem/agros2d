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


#include "vector.hpp"
#include "global_vector.hpp"
#include "local_vector.hpp"

#include "../utils/log.hpp"


namespace paralution {

template <typename ValueType>
Vector<ValueType>::Vector() {

  LOG_DEBUG(this, "Vector::Vector()",
            "default constructor");

  this->object_name_ = "";

}

template <typename ValueType>
Vector<ValueType>::~Vector() {

  LOG_DEBUG(this, "Vector::~Vector()",
            "default destructor");

}

template <typename ValueType>
void Vector<ValueType>::CopyFrom(const GlobalVector<ValueType> &src) {

  LOG_INFO("Vector<ValueType>::CopyFrom(const GlobalVector<ValueType> &src)");
  LOG_INFO("Mismatched types:");
  this->info();
  src.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::CopyFrom(const LocalVector<ValueType> &src) {

  LOG_INFO("Vector<ValueType>::CopyFrom(const LocalVector<ValueType> &src)");
  LOG_INFO("Mismatched types:");
  this->info();
  src.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::CloneFrom(const GlobalVector<ValueType> &src) {

  LOG_INFO("Vector<ValueType>::CloneFrom(const GlobalVector<ValueType> &src)");
  LOG_INFO("Mismatched types:");
  this->info();
  src.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::CloneFrom(const LocalVector<ValueType> &src) {

  LOG_INFO("Vector<ValueType>::CloneFrom(const LocalVector<ValueType> &src)");
  LOG_INFO("Mismatched types:");
  this->info();
  src.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::AddScale(const LocalVector<ValueType> &x, const ValueType alpha) {

  LOG_INFO("Vector<ValueType>::AddScale(const LocalVector<ValueType> &x, const ValueType alpha)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::AddScale(const GlobalVector<ValueType> &x, const ValueType alpha) {

  LOG_INFO("Vector<ValueType>::AddScale(const GlobalVector<ValueType> &x, const ValueType alpha)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::ScaleAdd(const ValueType alpha, const LocalVector<ValueType> &x) {

  LOG_INFO("Vector<ValueType>::ScaleAdd(const ValueType alpha, const LocalVector<ValueType> &x)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::ScaleAdd(const ValueType alpha, const GlobalVector<ValueType> &x) {

  LOG_INFO("Vector<ValueType>::ScaleAdd(const ValueType alpha, const GlobalVector<ValueType> &x)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
ValueType Vector<ValueType>::Dot(const LocalVector<ValueType> &x) const {

  LOG_INFO("Vector<ValueType>::Dot(const LocalVector<ValueType> &x) const");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
ValueType Vector<ValueType>::Dot(const GlobalVector<ValueType> &x) const {

  LOG_INFO("Vector<ValueType>::Dot(const GlobalVector<ValueType> &x) const");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x) {

  LOG_INFO("Vector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::PointWiseMult(const GlobalVector<ValueType> &x) {

  LOG_INFO("Vector<ValueType>::PointWiseMult(const GlobalVector<ValueType> &x)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x, const LocalVector<ValueType> &y) {

  LOG_INFO("Vector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x, const LocalVector<ValueType> &y)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  y.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::PointWiseMult(const GlobalVector<ValueType> &x, const LocalVector<ValueType> &y) {

  LOG_INFO("Vector<ValueType>::PointWiseMult(const GlobalVector<ValueType> &x, const LocalVector<ValueType> &y)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  y.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta) {

  LOG_INFO("ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::ScaleAddScale(const ValueType alpha, const GlobalVector<ValueType> &x, const ValueType beta) {

  LOG_INFO("ScaleAddScale(const ValueType alpha, const GlobalVector<ValueType> &x, const ValueType beta)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::ScaleAdd2(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta, const LocalVector<ValueType> &y, const ValueType gamma) {

  LOG_INFO("ScaleAdd2(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta, const LocalVector<ValueType> &y, const ValueType gamma)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  y.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::ScaleAdd2(const ValueType alpha, const GlobalVector<ValueType> &x,const ValueType beta, const GlobalVector<ValueType> &y, const ValueType gamma) {

  LOG_INFO("ScaleAdd2(const ValueType alpha, const GlobalVector<ValueType> &x,const ValueType beta, const GlobalVector<ValueType> &y, const ValueType gamma)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  y.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::PartialSum(const LocalVector<ValueType> &x) {

  LOG_INFO("PartialSum(const LocalVector<ValueType> &x)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Vector<ValueType>::PartialSum(const GlobalVector<ValueType> &x) {

  LOG_INFO("PartialSum(const GlobalVector<ValueType> &x)");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}

  /*
template <typename ValueType>
void Vector<ValueType>::

  LOG_INFO("");
  LOG_INFO("Mismatched types:");
  this->info();
  x.info();
  FATAL_ERROR(__FILE__, __LINE__);

}
  */

template class Vector<double>;
template class Vector<float>;

template class Vector<int>;

}
