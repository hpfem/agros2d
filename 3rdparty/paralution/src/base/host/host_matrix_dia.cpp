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

#ifdef SUPPORT_MKL
#include <mkl.h>
#include <mkl_spblas.h>
#endif


namespace paralution {

template <typename ValueType>
HostMatrixDIA<ValueType>::HostMatrixDIA() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixDIA<ValueType>::HostMatrixDIA(const Paralution_Backend_Descriptor local_backend) {

    this->mat_.val = NULL;
    this->mat_.offset = NULL;  
    this->mat_.num_diag = 0 ;
    this->set_backend(local_backend); 

}

template <typename ValueType>
HostMatrixDIA<ValueType>::~HostMatrixDIA() {

  this->Clear();

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::info(void) const {

  LOG_INFO("HostMatrixDIA<ValueType>, diag = " << this->mat_.num_diag << " nnz=" << this->get_nnz());
}


template <typename ValueType>
void HostMatrixDIA<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_host(&this->mat_.val);
    free_host(&this->mat_.offset);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;
    this->mat_.num_diag = 0 ;

  }

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag) {

  assert( nnz   >= 0);
  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    assert( ndiag > 0);

    allocate_host(nnz, &this->mat_.val);
    allocate_host(ndiag, &this->mat_.offset);
 
    set_to_zero_host(nnz, mat_.val);
    set_to_zero_host(ndiag, mat_.offset);
    
    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;
    this->mat_.num_diag = ndiag ;

  }

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixDIA<ValueType> *cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&mat)) {
    
    this->AllocateDIA(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), cast_mat->get_ndiag() );

    assert((this->get_nnz()  == mat.get_nnz())  &&
	   (this->get_nrow() == mat.get_nrow()) &&
	   (this->get_ncol() == mat.get_ncol()) );    

    if (this->get_nnz() > 0) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for            
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];
      
      for (int j=0; j<this->mat_.num_diag; ++j)
        this->mat_.offset[j] = cast_mat->mat_.offset[j];
      
    }
    
    
  } else {
    
    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);
    
  }  

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixDIA<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

    if (const HostMatrixDIA<ValueType> *cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&mat)) {

      this->CopyFrom(*cast_mat);
      return true;

  }


    if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

      this->Clear();
      int nnz = 0;

      csr_to_dia(this->local_backend_.OpenMP_threads,
                 cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(),
      		 cast_mat->mat_, &this->mat_, &nnz);

      this->nrow_ = cast_mat->get_nrow();
      this->ncol_ = cast_mat->get_ncol();
      this->nnz_ = nnz;

    return true;

  }
  
  return false;

}


#ifdef SUPPORT_MKL__DIA__

template <>
void HostMatrixDIA<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);
 
  char transp='N'; 
  int nrow = this->get_nrow() ;
  int nnz = this->get_nnz();

  FATAL_ERROR(__FILE__, __LINE__);

  // TODO
  // mkl dia is one-based!
  /*
    
  */
}

template <>
void HostMatrixDIA<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {
  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<float> *cast_in = dynamic_cast<const HostVector<float>*> (&in) ; 
  HostVector<float> *cast_out      = dynamic_cast<      HostVector<float>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  char transp='N'; 
  int nrow = this->get_nrow() ;
  int nnz = this->get_nnz();

  FATAL_ERROR(__FILE__, __LINE__);

  // TODO
  // mkl dia is one-based!
  /*

  */

}

#else

template <typename ValueType>
void HostMatrixDIA<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    for (int i=0; i<this->get_nrow(); ++i) {
      
      cast_out->vec_[i] = ValueType(0.0);
      
      for (int j=0; j<this->get_ndiag(); ++j) {
        
        int start  = 0;
        int end = this->get_nrow();
        int v_offset = 0; 
        
        if ( this->mat_.offset[j] < 0) {
          start -= mat_.offset[j];
          v_offset = -start;
        } else {
          end -= mat_.offset[j];
          v_offset = mat_.offset[j];
        }
        
        if ( (i >= start) && (i < end)) {
          cast_out->vec_[i] += this->mat_.val[DIA_IND(i, j, this->get_nrow(), this->get_ndiag())] * cast_in->vec_[i+v_offset];
        } else {
          if (i >= end)
            break;
        }
      }
      
    }
    
  }
}

#endif

template <typename ValueType>
void HostMatrixDIA<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    for (int i=0; i<this->get_nrow(); ++i) {
      
      for (int j=0; j<this->get_ndiag(); ++j) {
        
        int start  = 0;
        int end = this->get_nrow();
        int v_offset = 0; 
        
        if ( this->mat_.offset[j] < 0) {
          start -= mat_.offset[j];
          v_offset = -start;
        } else {
          end -= mat_.offset[j];
          v_offset = mat_.offset[j];
        }
        
        if ( (i >= start) && (i < end)) {
          cast_out->vec_[i] += scalar*this->mat_.val[DIA_IND(i, j, this->get_nrow(), this->get_ndiag())] * cast_in->vec_[i+v_offset];
        } else {
          if (i >= end)
            break;
        }
      }
      
    }
    
  }
}


template class HostMatrixDIA<double>;
template class HostMatrixDIA<float>;

}
