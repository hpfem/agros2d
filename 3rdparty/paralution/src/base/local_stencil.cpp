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


#include "local_stencil.hpp"
#include "local_vector.hpp"

#include "../utils/log.hpp"

#include <assert.h>

namespace paralution {

template <typename ValueType>
LocalStencil<ValueType>::LocalStencil() {

  this->object_name_ = "";
  this->ndim_ = 0;
  this->dim_ = NULL;

  FATAL_ERROR(__FILE__, __LINE__);    

}

template <typename ValueType>
LocalStencil<ValueType>::~LocalStencil() {
}

template <typename ValueType>
int LocalStencil<ValueType>::get_nrow(void) const {

  int dim = 0;

  if (this->get_ndim() > 0) {

    for (int i=0; i<this->get_ndim(); ++i)
      dim += this->dim_[i];

  }

  return dim;
}

template <typename ValueType>
int LocalStencil<ValueType>::get_ncol(void) const {

  return this->get_nrow();
}

template <typename ValueType>
void LocalStencil<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  assert(this->ndim_ > 0);

}

template <typename ValueType>
void LocalStencil<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, 
                                       LocalVector<ValueType> *out) const {

  assert(this->ndim_ > 0);

}


template class LocalStencil<double>;
template class LocalStencil<float>;

}
