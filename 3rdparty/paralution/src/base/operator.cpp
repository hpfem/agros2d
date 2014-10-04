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


#include "operator.hpp"

#include "vector.hpp"
#include "global_vector.hpp"
#include "local_vector.hpp"

#include "../utils/log.hpp"

namespace paralution {

template <typename ValueType>
Operator<ValueType>::Operator() {

  LOG_DEBUG(this, "Operator::Operator()",
            "default constructor");

  this->object_name_ = "";

}

template <typename ValueType>
Operator<ValueType>::~Operator() {

  LOG_DEBUG(this, "Operator::~Operator()",
            "default destructor");

}

template <typename ValueType>
void Operator<ValueType>::Apply(const GlobalVector<ValueType> &in, GlobalVector<ValueType> *out) const {

  LOG_INFO("Operator<ValueType>::Apply(const GlobalVector<ValueType> &in, GlobalVector<ValueType> *out)");
  LOG_INFO("Mismatched types:");
  this->info();
  in.info();
  out->info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Operator<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  LOG_INFO("Operator<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out)");
  LOG_INFO("Mismatched types:");
  this->info();
  in.info();
  out->info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Operator<ValueType>::ApplyAdd(const GlobalVector<ValueType> &in, const ValueType scalar, GlobalVector<ValueType> *out) const {

  LOG_INFO("Operator<ValueType>::ApplyAdd(const GlobalVector<ValueType> &in, const ValueType scalar, GlobalVector<ValueType> *out)");
  LOG_INFO("Mismatched types:");
  this->info();
  in.info();
  out->info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Operator<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, LocalVector<ValueType> *out) const {

  LOG_INFO("Operator<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, LocalVector<ValueType> *out)");
  LOG_INFO("Mismatched types:");
  this->info();
  in.info();
  out->info();
  FATAL_ERROR(__FILE__, __LINE__); 

}


template class Operator<double>;
template class Operator<float>;

template class Operator<int>;

}
