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


#include "global_matrix.hpp"
#include "global_vector.hpp"
#include "local_matrix.hpp"
#include "local_vector.hpp"

#include "../utils/log.hpp"

#include <assert.h>

namespace paralution {

template <typename ValueType>
GlobalMatrix<ValueType>::GlobalMatrix() {
  this->object_name_ = "";

  FATAL_ERROR(__FILE__, __LINE__);    

}

template <typename ValueType>
GlobalMatrix<ValueType>::~GlobalMatrix() {
}

template <typename ValueType>
void GlobalMatrix<ValueType>::Apply(const GlobalVector<ValueType> &in, GlobalVector<ValueType> *out) const {

  assert(in.global_size_   == this->global_ncol_);
  assert(out->global_size_ == this->global_nrow_);

}

template <typename ValueType>
void GlobalMatrix<ValueType>::ApplyAdd(const GlobalVector<ValueType> &in, const ValueType scalar, 
                                       GlobalVector<ValueType> *out) const {

  assert(in.global_size_   == this->global_ncol_);
  assert(out->global_size_ == this->global_nrow_);

}

template <typename ValueType>
void GlobalMatrix<ValueType>::CopyFrom(const GlobalMatrix<ValueType> &src) {

  assert( src.global_nnz_ != 0 );
  assert( src.local_nnz_  != 0 );

}


template class GlobalMatrix<double>;
template class GlobalMatrix<float>;

}
