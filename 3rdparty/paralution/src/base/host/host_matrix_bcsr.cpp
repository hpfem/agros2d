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

#include "host_matrix_bcsr.hpp"
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

#ifdef SUPPORT_MKL
#include <mkl.h>
#include <mkl_spblas.h>
#endif


namespace paralution {

template <typename ValueType>
HostMatrixBCSR<ValueType>::HostMatrixBCSR() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixBCSR<ValueType>::HostMatrixBCSR(const Paralution_Backend_Descriptor local_backend) {

  this->set_backend(local_backend); 

  // not implemented yet
  FATAL_ERROR(__FILE__, __LINE__);
}

template <typename ValueType>
HostMatrixBCSR<ValueType>::~HostMatrixBCSR() {

  this->Clear();

}

template <typename ValueType>
void HostMatrixBCSR<ValueType>::info(void) const {

  //TODO 
  LOG_INFO("HostMatrixBCSR<ValueType>");

}


template <typename ValueType>
void HostMatrixBCSR<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void HostMatrixBCSR<ValueType>::AllocateBCSR(const int nnz, const int nrow, const int ncol) {

  assert( nnz   >= 0);
  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}

template <typename ValueType>
void HostMatrixBCSR<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixBCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixBCSR<ValueType>*> (&mat)) {
    
    this->AllocateBCSR(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol());

    assert((this->get_nnz()  == mat.get_nnz())  &&
	   (this->get_nrow() == mat.get_nrow()) &&
	   (this->get_ncol() == mat.get_ncol()) );    
    
    if (this->get_nnz() > 0) {

      omp_set_num_threads(this->local_backend_.OpenMP_threads);  

      // TODO
    
      FATAL_ERROR(__FILE__, __LINE__);
    }
    
    
  } else {
    
    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);
    
  }  

}

template <typename ValueType>
void HostMatrixBCSR<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixBCSR<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

    if (const HostMatrixBCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixBCSR<ValueType>*> (&mat)) {

      this->CopyFrom(*cast_mat);
      return true;

  }


    if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

      this->Clear();
      int nnz = 0;

      // TODO
     

      //      csr_to_bcsr(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(),
      //                 cast_mat->mat_, &this->mat_, &nnz);

      FATAL_ERROR(__FILE__, __LINE__);

      this->nrow_ = cast_mat->get_nrow();
      this->ncol_ = cast_mat->get_ncol();
      this->nnz_ = nnz;

    return true;

  }
  

  return false;

}

#ifdef SUPPORT_MKL

template <>
void HostMatrixBCSR<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

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

  // TODO

  FATAL_ERROR(__FILE__, __LINE__);

  /*
  mkl_cspblas_ddiagemv(&transp, &nrow, 
		       this->mat_.val, this->mat_.row, this->mat_.col,
		       &nnz,
		       cast_in->vec_, cast_out->vec_);
  */
}

template <>
void HostMatrixBCSR<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {
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

  // TODO
  
  FATAL_ERROR(__FILE__, __LINE__);

  /*
  mkl_cspblas_scoogemv(&transp, &nrow,
		       this->mat_.val, this->mat_.row, this->mat_.col,
		       &nnz,
		       cast_in->vec_, cast_out->vec_);
  */

}

#else

template <typename ValueType>
void HostMatrixBCSR<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
//    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
//    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 
    
//    assert(cast_in != NULL);
//    assert(cast_out!= NULL);
    
    omp_set_num_threads(this->local_backend_.OpenMP_threads);  
  
      // TODO

    FATAL_ERROR(__FILE__, __LINE__);

  }

}

#endif

template <typename ValueType>
void HostMatrixBCSR<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                        BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());

//    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
//    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 
    
//    assert(cast_in != NULL);
//    assert(cast_out!= NULL);

    omp_set_num_threads(this->local_backend_.OpenMP_threads);  

    // TODO

    FATAL_ERROR(__FILE__, __LINE__);

  }

}

template class HostMatrixBCSR<double>;
template class HostMatrixBCSR<float>;

}
