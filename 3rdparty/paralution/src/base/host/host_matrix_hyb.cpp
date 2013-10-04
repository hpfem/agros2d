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

#include "host_matrix_hyb.hpp"
#include "host_matrix_ell.hpp"
#include "host_matrix_dia.hpp"
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
HostMatrixHYB<ValueType>::HostMatrixHYB() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixHYB<ValueType>::HostMatrixHYB(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.ELL.val = NULL;
  this->mat_.ELL.col = NULL;
  this->mat_.ELL.max_row = 0;

  this->mat_.COO.row = NULL;  
  this->mat_.COO.col = NULL;  
  this->mat_.COO.val = NULL;

  this->ell_nnz_ = 0;
  this->coo_nnz_ = 0;

  this->set_backend(local_backend); 

}

template <typename ValueType>
HostMatrixHYB<ValueType>::~HostMatrixHYB() {

  this->Clear();

}

template <typename ValueType>
void HostMatrixHYB<ValueType>::info(void) const {

  LOG_INFO("HostMatrixHYB<ValueType>" << 
           " ELL nnz=" << this->get_ell_nnz() <<
           " ELL max row=" << this->get_ell_max_row() <<
           " COO nnz=" << this->get_coo_nnz());

}


template <typename ValueType>
void HostMatrixHYB<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_host(&this->mat_.COO.row);
    free_host(&this->mat_.COO.col);
    free_host(&this->mat_.COO.val);

    free_host(&this->mat_.ELL.val);
    free_host(&this->mat_.ELL.col);

    this->ell_nnz_ = 0;
    this->coo_nnz_ = 0;
    this->mat_.ELL.max_row = 0;

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void HostMatrixHYB<ValueType>::AllocateHYB(const int ell_nnz, const int coo_nnz, const int ell_max_row, 
                                           const int nrow, const int ncol) {

  assert( ell_nnz   >= 0);
  assert( coo_nnz   >= 0);
  assert( ell_max_row >= 0);

  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (ell_nnz + coo_nnz > 0) {

    // ELL
    assert(ell_nnz == ell_max_row*nrow);

    allocate_host(ell_nnz, &this->mat_.ELL.val);
    allocate_host(ell_nnz, &this->mat_.ELL.col);
    
    set_to_zero_host(ell_nnz, this->mat_.ELL.val);
    set_to_zero_host(ell_nnz, this->mat_.ELL.col);

    this->mat_.ELL.max_row = ell_max_row;
    this->ell_nnz_ = ell_nnz;

    // COO
    allocate_host(coo_nnz, &this->mat_.COO.row);
    allocate_host(coo_nnz, &this->mat_.COO.col);
    allocate_host(coo_nnz, &this->mat_.COO.val);
 
    set_to_zero_host(coo_nnz, this->mat_.COO.row);
    set_to_zero_host(coo_nnz, this->mat_.COO.col);
    set_to_zero_host(coo_nnz, this->mat_.COO.val);
    this->coo_nnz_ = coo_nnz;

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = ell_nnz + coo_nnz;

  }


}

template <typename ValueType>
void HostMatrixHYB<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixHYB<ValueType> *cast_mat = dynamic_cast<const HostMatrixHYB<ValueType>*> (&mat)) {

    this->AllocateHYB(cast_mat->get_ell_nnz(), cast_mat->get_coo_nnz(), cast_mat->get_ell_max_row(),
                      cast_mat->get_nrow(), cast_mat->get_ncol());

    assert((this->get_nnz()      == cast_mat->get_nnz())  &&
           (this->get_ell_nnz()  == cast_mat->get_ell_nnz())  &&
           (this->get_coo_nnz()  == cast_mat->get_coo_nnz())  &&
	   (this->get_nrow()     == cast_mat->get_nrow()) &&
	   (this->get_ncol()     == cast_mat->get_ncol()) );
    
    if (this->get_ell_nnz() > 0) {

      // ELL

      for (int i=0; i<this->get_ell_nnz(); ++i)
        this->mat_.ELL.col[i] = cast_mat->mat_.ELL.col[i];

      for (int i=0; i<this->get_ell_nnz(); ++i)
        this->mat_.ELL.val[i] = cast_mat->mat_.ELL.val[i];
      
    }
    
    if (this->get_coo_nnz() > 0) {

      // COO
      for (int i=0; i<this->get_coo_nnz(); ++i)
        this->mat_.COO.row[i] = cast_mat->mat_.COO.row[i];

      for (int i=0; i<this->get_coo_nnz(); ++i)
        this->mat_.COO.col[i] = cast_mat->mat_.COO.col[i];

      for (int i=0; i<this->get_coo_nnz(); ++i)
        this->mat_.COO.val[i] = cast_mat->mat_.COO.val[i];

    }

    
  } else {
    
    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);
    
  }  

}

template <typename ValueType>
void HostMatrixHYB<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixHYB<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

    if (const HostMatrixHYB<ValueType> *cast_mat = dynamic_cast<const HostMatrixHYB<ValueType>*> (&mat)) {

      this->CopyFrom(*cast_mat);
      return true;

  }


    if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

      this->Clear();
      int nnz = 0;
      int coo_nnz = 0;
      int ell_nnz = 0;

      //      this->mat_.ELL.max_row = ( cast_mat->get_nnz() / cast_mat->get_nrow() );

      csr_to_hyb(this->local_backend_.OpenMP_threads,
                 cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(),
                 cast_mat->mat_, &this->mat_, 
                 &nnz, &ell_nnz, &coo_nnz);

      this->nrow_ = cast_mat->get_nrow();
      this->ncol_ = cast_mat->get_ncol();
      this->nnz_ = nnz;
      this->ell_nnz_ = ell_nnz;
      this->coo_nnz_ = coo_nnz;

    return true;

  }
  

  return false;

}

template <typename ValueType>
void HostMatrixHYB<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    
    // ELL
    if (this->get_ell_nnz() > 0) {

#pragma omp parallel for
      for (int ai=0; ai<this->get_nrow(); ++ai) {
        cast_out->vec_[ai] = ValueType(0.0);
        
        for (int n=0; n<this->get_ell_max_row(); ++n) {
          
          int aj = ELL_IND(ai, n, this->get_nrow(), this->get_ell_max_row());
          
          if ((this->mat_.ELL.col[aj] >= 0) && (this->mat_.ELL.col[aj] < this->get_ncol()))
            cast_out->vec_[ai] += this->mat_.ELL.val[aj] * cast_in->vec_[ this->mat_.ELL.col[aj] ];
        }
      }
    }
    
    // COO
    if (this->get_coo_nnz() > 0) {
      
      for (int i=0; i<this->get_coo_nnz(); ++i)
        cast_out->vec_[this->mat_.COO.row[i] ] += this->mat_.COO.val[i] * cast_in->vec_[ this->mat_.COO.col[i] ];
    }
  }

}


template <typename ValueType>
void HostMatrixHYB<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    
    // ELL
    if (this->get_ell_nnz() > 0) {

#pragma omp parallel for
      for (int ai=0; ai<this->get_nrow(); ++ai) {
        
        for (int n=0; n<this->get_ell_max_row(); ++n) {
          
          int aj = ELL_IND(ai, n, this->get_nrow(), this->get_ell_max_row());
          
          if ((this->mat_.ELL.col[aj] >= 0) && (this->mat_.ELL.col[aj] < this->get_ncol()))
            cast_out->vec_[ai] += scalar*this->mat_.ELL.val[aj] * cast_in->vec_[ this->mat_.ELL.col[aj] ];
        }
      }
    }
    
    // COO
    if (this->get_coo_nnz() > 0) {
      
      for (int i=0; i<this->get_coo_nnz(); ++i)
        cast_out->vec_[this->mat_.COO.row[i] ] += scalar*this->mat_.COO.val[i] * cast_in->vec_[ this->mat_.COO.col[i] ];
    }
  }

}


template class HostMatrixHYB<double>;
template class HostMatrixHYB<float>;

}
