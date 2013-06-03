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

#include "host_matrix_dense.hpp"
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
HostMatrixDENSE<ValueType>::HostMatrixDENSE() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}
template <typename ValueType>
HostMatrixDENSE<ValueType>::HostMatrixDENSE(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.val = NULL;
  this->set_backend(local_backend);

}

template <typename ValueType>
HostMatrixDENSE<ValueType>::~HostMatrixDENSE() {

  this->Clear();

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::info(void) const {

  LOG_INFO("HostMatrixDENSE<ValueType>");
}


template <typename ValueType>
void HostMatrixDENSE<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_host(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::AllocateDENSE(const int nrow, const int ncol) {

  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nrow*ncol > 0) {

    allocate_host(nrow*ncol, &this->mat_.val);
    set_to_zero_host(nrow*ncol, mat_.val);   

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nrow*ncol;

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixDENSE<ValueType> *cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&mat)) {
    
    this->AllocateDENSE(cast_mat->get_nrow(), cast_mat->get_ncol());

    assert((this->get_nnz()  == mat.get_nnz())  &&
	   (this->get_nrow() == mat.get_nrow()) &&
	   (this->get_ncol() == mat.get_ncol()) );    

    if (this->get_nnz() > 0) {

      omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for      
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];
      
    }
    
    
  } else {
    
    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);
    
  }  

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixDENSE<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

    if (const HostMatrixDENSE<ValueType> *cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&mat)) {

      this->CopyFrom(*cast_mat);
      return true;

  }


    if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

      this->Clear();

      csr_to_dense(this->local_backend_.OpenMP_threads,
                   cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(),
                   cast_mat->mat_, &this->mat_);

      this->nrow_ = cast_mat->get_nrow();
      this->ncol_ = cast_mat->get_ncol();
      this->nnz_ = this->nrow_ * this->ncol_;

      return true;

  }
  

  return false;

}

#ifdef SUPPORT_MKL

template <>
void HostMatrixDENSE<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);
 
  int nrow = this->get_nrow() ;
  int ncol = this->get_ncol();
  double alpha = double(1.0);
  double beta = double(0.0);

  cblas_dgemv(CblasRowMajor, CblasNoTrans, 
              nrow, ncol,
              alpha, this->mat_.val,
              nrow, 
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);

}

template <>
void HostMatrixDENSE<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {
  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<float> *cast_in = dynamic_cast<const HostVector<float>*> (&in) ; 
  HostVector<float> *cast_out      = dynamic_cast<      HostVector<float>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  int nrow = this->get_nrow() ;
  int ncol = this->get_ncol();
  float alpha = float(1.0);
  float beta = float(0.0);

  cblas_sgemv(CblasRowMajor, CblasNoTrans, 
              nrow, ncol,
              alpha, this->mat_.val,
              nrow, 
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);


}

#else

template <typename ValueType>
void HostMatrixDENSE<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
      for (int aj=0; aj<this->get_ncol(); ++aj) 
        cast_out->vec_[ai] += this->mat_.val[DENSE_IND(ai,aj,this->get_nrow(),this->get_ncol())] * cast_in->vec_[aj];
  }


}

#endif


#ifdef SUPPORT_MKL

template <>
void HostMatrixDENSE<double>::ApplyAdd(const BaseVector<double> &in, const double scalar,
                                       BaseVector<double> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);
 
  int nrow = this->get_nrow() ;
  int ncol = this->get_ncol();
  double beta = double(1.0);

  cblas_dgemv(CblasRowMajor, CblasNoTrans, 
              nrow, ncol,
              scalar, this->mat_.val,
              nrow, 
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);

}

template <>
void HostMatrixDENSE<float>::ApplyAdd(const BaseVector<float> &in, const float scalar,
                                      BaseVector<float> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<float> *cast_in = dynamic_cast<const HostVector<float>*> (&in) ; 
  HostVector<float> *cast_out      = dynamic_cast<      HostVector<float>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  int nrow = this->get_nrow() ;
  int ncol = this->get_ncol();
  float beta = float(1.0);

  cblas_sgemv(CblasRowMajor, CblasNoTrans, 
              nrow, ncol,
              scalar, this->mat_.val,
              nrow, 
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);


}

#else

template <typename ValueType>
void HostMatrixDENSE<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    for (int aj=0; aj<this->get_ncol(); ++aj) 
      cast_out->vec_[ai] += this->mat_.val[DENSE_IND(ai,aj,this->get_nrow(),this->get_ncol())] * cast_in->vec_[aj];
  
    

  }

}

#endif

template class HostMatrixDENSE<double>;
template class HostMatrixDENSE<float>;

}
