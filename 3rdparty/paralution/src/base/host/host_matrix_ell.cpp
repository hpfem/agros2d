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

#include "host_matrix_ell.hpp"
#include "host_matrix_dia.hpp"
#include "host_matrix_hyb.hpp"
#include "host_matrix_coo.hpp"
#include "host_matrix_csr.hpp"
#include "host_conversion.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "host_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

extern "C" {
#include "../../../thirdparty/matrix-market/mmio.h"
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_set_num_threads(num) ;
#endif


namespace paralution {

template <typename ValueType>
HostMatrixELL<ValueType>::HostMatrixELL() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixELL<ValueType>::HostMatrixELL(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.val = NULL;
  this->mat_.col = NULL;
  this->mat_.max_row = 0;

  this->set_backend(local_backend); 

}

template <typename ValueType>
HostMatrixELL<ValueType>::~HostMatrixELL() {

  this->Clear();

}

template <typename ValueType>
void HostMatrixELL<ValueType>::info(void) const {

  LOG_INFO("HostMatrixELL<ValueType>");
}


template <typename ValueType>
void HostMatrixELL<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_host(&this->mat_.val);
    free_host(&this->mat_.col);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row) {

  assert( nnz   >= 0);
  assert( ncol  >= 0);
  assert( nrow  >= 0);
  assert( max_row >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    assert(nnz == max_row * nrow);

    allocate_host(nnz, &this->mat_.val);
    allocate_host(nnz, &this->mat_.col);
    
    set_to_zero_host(nnz, this->mat_.val);
    set_to_zero_host(nnz, this->mat_.col);
    
    this->mat_.max_row = max_row;
    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixELL<ValueType> *cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&mat)) {
    
    this->AllocateELL(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), cast_mat->get_max_row() );

    assert((this->get_nnz()  == mat.get_nnz())  &&
	   (this->get_nrow() == mat.get_nrow()) &&
	   (this->get_ncol() == mat.get_ncol()) );    
    

    
    if (this->get_nnz() > 0) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for                  
  for (int i=0; i<this->get_nnz(); ++i)
    this->mat_.val[i] = cast_mat->mat_.val[i];

#pragma omp parallel for                  
  for (int i=0; i<this->get_nnz(); ++i)
    this->mat_.col[i] = cast_mat->mat_.col[i];

    }
    
    
  } else {
    
    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);
    
  }  

}

template <typename ValueType>
void HostMatrixELL<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixELL<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  if (const HostMatrixELL<ValueType> *cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&mat)) {

    this->CopyFrom(*cast_mat);
    return true;

  }


  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    this->Clear();
    int nnz = 0;

    csr_to_ell(this->local_backend_.OpenMP_threads,
               cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(),
               cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
    this->nnz_ = nnz;

    return true;

  }
  
  return false;

}

template <typename ValueType>
void HostMatrixELL<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);
    
    omp_set_num_threads(this->local_backend_.OpenMP_threads);  
    
#pragma omp parallel for
    for (int ai=0; ai<this->get_nrow(); ++ai) {
      cast_out->vec_[ai] = ValueType(0.0);

      for (int n=0; n<this->get_max_row(); ++n) {

        int aj = ELL_IND(ai, n, this->get_nrow(), this->get_max_row());

        if ((this->mat_.col[aj] >= 0) && (this->mat_.col[aj] < this->get_ncol())) {
          cast_out->vec_[ai] += this->mat_.val[aj] * cast_in->vec_[ this->mat_.col[aj] ];
        } else {
          break;
        }
      }
    }
    
  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                        BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    omp_set_num_threads(this->local_backend_.OpenMP_threads);  
    
#pragma omp parallel for
    for (int ai=0; ai<this->get_nrow(); ++ai) 
      for (int n=0; n<this->get_max_row(); ++n) {

        int aj = ELL_IND(ai, n, this->get_nrow(), this->get_max_row());


        if ((this->mat_.col[aj] >= 0) && (this->mat_.col[aj] < this->get_ncol())) {
          cast_out->vec_[ai] += scalar*this->mat_.val[aj] * cast_in->vec_[ this->mat_.col[aj] ];
        } else {
          break;
        }
      }  
  }
 
}

template class HostMatrixELL<double>;
template class HostMatrixELL<float>;

}
