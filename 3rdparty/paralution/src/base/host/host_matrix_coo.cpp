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


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN64) && !defined(__CYGWIN__)
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "host_matrix_coo.hpp"
#include "host_matrix_csr.hpp"
#include "host_conversion.hpp"
#include "host_vector.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"

extern "C" {
#include "../../../thirdparty/matrix-market/mmio.h"
}

#include <assert.h>
#include <stdio.h>

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
HostMatrixCOO<ValueType>::HostMatrixCOO() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixCOO<ValueType>::HostMatrixCOO(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "HostMatrixCOO::HostMatrixCOO()",
            "constructor with local_backend");

    this->mat_.row = NULL;  
    this->mat_.col = NULL;  
    this->mat_.val = NULL;
    this->set_backend(local_backend); 
  
}

template <typename ValueType>
HostMatrixCOO<ValueType>::~HostMatrixCOO() {

  LOG_DEBUG(this, "HostMatrixCOO::~HostMatrixCOO()",
            "destructor");

  this->Clear();

}
template <typename ValueType>
void HostMatrixCOO<ValueType>::info(void) const {

  LOG_INFO("HostMatrixCOO<ValueType>");

}


template <typename ValueType>
void HostMatrixCOO<ValueType>::Clear() {

  if (this->nnz_ > 0) {

    free_host(&this->mat_.row);
    free_host(&this->mat_.col);
    free_host(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;
  }

}

template <typename ValueType>
void HostMatrixCOO<ValueType>::AllocateCOO(const int nnz, const int nrow, const int ncol) {

  assert( nnz >= 0);
  assert( ncol >= 0);
  assert( nrow >= 0);

  if (this->nnz_ > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_host(nnz, &this->mat_.row);
    allocate_host(nnz, &this->mat_.col);
    allocate_host(nnz, &this->mat_.val);
 
    set_to_zero_host(nnz, this->mat_.row);
    set_to_zero_host(nnz, this->mat_.col);
    set_to_zero_host(nnz, this->mat_.val);
    
    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;
  }

}

template <typename ValueType>
void HostMatrixCOO<ValueType>::SetDataPtrCOO(int **row, int **col, ValueType **val,
                                             const int nnz, const int nrow, const int ncol) {

  assert(*row != NULL);
  assert(*col != NULL);
  assert(*val != NULL);
  assert(nnz > 0);
  assert(nrow > 0);
  assert(ncol > 0);

  this->Clear();

  this->nrow_ = nrow;
  this->ncol_ = ncol;
  this->nnz_  = nnz;

  this->mat_.row = *row;
  this->mat_.col = *col;
  this->mat_.val = *val;

}

template <typename ValueType>
void HostMatrixCOO<ValueType>::LeaveDataPtrCOO(int **row, int **col, ValueType **val) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);

  // see free_host function for details
  *row = this->mat_.row;
  *col = this->mat_.col;
  *val = this->mat_.val;

  this->mat_.row = NULL;
  this->mat_.col = NULL;
  this->mat_.val = NULL;

  this->nrow_ = 0;
  this->ncol_ = 0;
  this->nnz_  = 0;

}

template <typename ValueType>
void HostMatrixCOO<ValueType>::CopyFromCOO(const int *row, const int *col, const ValueType *val) {

  assert(this->nnz_ > 0);
  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for      
  for (int i=0; i<this->nnz_; ++i)
    this->mat_.row[i] = row[i];

#pragma omp parallel for      
  for (int j=0; j<this->nnz_; ++j)
    this->mat_.col[j] = col[j];

#pragma omp parallel for      
  for (int j=0; j<this->nnz_; ++j)
    this->mat_.val[j] = val[j];

}


template <typename ValueType>
void HostMatrixCOO<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixCOO<ValueType> *cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&mat)) {
    
    if (this->nnz_ == 0)
      this->AllocateCOO(cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_ );

    assert((this->nnz_  == cast_mat->nnz_)  &&
	   (this->nrow_ == cast_mat->nrow_) &&
	   (this->ncol_ == cast_mat->ncol_) );

    if (this->nnz_ > 0) {
      
  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for      
      for (int j=0; j<this->nnz_; ++j)
        this->mat_.row[j] = cast_mat->mat_.row[j];

#pragma omp parallel for      
      for (int j=0; j<this->nnz_; ++j)
        this->mat_.col[j] = cast_mat->mat_.col[j];

#pragma omp parallel for      
      for (int j=0; j<this->nnz_; ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];

    }

  } else {

    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);

  }

}

template <typename ValueType>
void HostMatrixCOO<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  if (const HostMatrixCOO<ValueType> *cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&mat)) {

    this->CopyFrom(*cast_mat);
    return true;

  }

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    this->Clear();
    csr_to_coo(this->local_backend_.OpenMP_threads,
               cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_, cast_mat->mat_, &this->mat_);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = cast_mat->nnz_;

    return true;

  }
  
  return false;

}

template <typename ValueType>
void HostMatrixCOO<ValueType>::ReadFileMTX(const std::string filename) { 

  // Follow example_read.c (from Matrix Market web site)
  int ret_code;
  MM_typecode matcode;
  FILE *f;
  int M, N;
  int fnz, nnz;  // file nnz, real nnz
  bool sym = false;

  LOG_INFO("ReadFileMTX: filename="<< filename << "; reading...");

  if ((f = fopen(filename.c_str(), "r")) == NULL) {
    LOG_INFO("ReadFileMTX cannot open file " << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }

  if (mm_read_banner(f, &matcode) != 0)
    {
      LOG_INFO("ReadFileMTX could not process Matrix Market banner.");
      FATAL_ERROR(__FILE__, __LINE__);
    }


  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */

  if (mm_is_complex(matcode) && mm_is_matrix(matcode) && 
      mm_is_sparse(matcode) )
    {
      LOG_INFO("ReadFileMTX does not support Market Market type:" << mm_typecode_to_str(matcode));
      FATAL_ERROR(__FILE__, __LINE__);
    }

  /* find out size of sparse matrix .... */

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &fnz)) !=0) {
    LOG_INFO("ReadFileMTX matrix size error");
    FATAL_ERROR(__FILE__, __LINE__);
  }

  nnz = fnz ; 

  /* reseve memory for matrices */
  if(mm_is_symmetric(matcode)) {

    if (N != M) {
      LOG_INFO("ReadFileMTX non-squared symmetric matrices are not supported");
      LOG_INFO("What is symmetric and non-squared matrix? e-mail me");
      FATAL_ERROR(__FILE__, __LINE__);
    }

    nnz = 2*(nnz - N) + N;
    sym = true ;
  }

  this->AllocateCOO(nnz,M,N);

  int ii=0;
  int col, row;
  double val;
  int ret;
  for (int i=0; i<fnz; ++i) {
    ret = fscanf(f, "%d %d %lg\n", &row, &col, &val);
    if (!ret) FATAL_ERROR(__FILE__, __LINE__);

    row--; /* adjust from 1-based to 0-based */
    col--;

    assert (ret == 3);

    //    LOG_INFO(row << " " << col << " " << val);

    this->mat_.row[ii] = row;
    this->mat_.col[ii] = col;
    this->mat_.val[ii] = ValueType(val);

    if (sym && (row!=col)) {
      ++ii;
      this->mat_.row[ii] = col;
      this->mat_.col[ii] = row;
      this->mat_.val[ii] = ValueType(val);
    }
      
    ++ii;
  }

  LOG_INFO("ReadFileMTX: filename="<< filename << "; done");

  fclose(f);

}

template <>
void HostMatrixCOO<int>::WriteFileMTX(const std::string filename) const {

  MM_typecode matcode; 
  FILE *f;

  LOG_INFO("WriteFileMTX: filename="<< filename << "; writing...");

  if ((f = fopen(filename.c_str(), "w")) == NULL) {
    LOG_INFO("WriteFileMTX cannot open file " << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }

  mm_initialize_typecode(&matcode);
  mm_set_matrix(&matcode);
  mm_set_coordinate(&matcode);
  mm_set_real(&matcode);
  
  mm_write_banner(f, matcode); 

  //  mm_write_mtx_crd_size(f, this->ncol_, this->nrow_, this->nnz_);
  mm_write_mtx_crd_size(f, this->nrow_, this->ncol_, this->nnz_);

  /* NOTE: matrix market files use 1-based indices, i.e. first element
     of a vector has index 1, not 0.  */

  for (int i=0; i<this->nnz_; i++)
    fprintf(f, "%d %d %d\n",
            this->mat_.row[i]+1, 
            this->mat_.col[i]+1, 
            this->mat_.val[i]);
  
  LOG_INFO("WriteFileMTX: filename="<< filename << "; done");

  fclose(f);

}


template <typename ValueType>
void HostMatrixCOO<ValueType>::WriteFileMTX(const std::string filename) const {

  MM_typecode matcode; 
  FILE *f;

  LOG_INFO("WriteFileMTX: filename="<< filename << "; writing...");

  if ((f = fopen(filename.c_str(), "w")) == NULL) {
    LOG_INFO("WriteFileMTX cannot open file " << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }

  mm_initialize_typecode(&matcode);
  mm_set_matrix(&matcode);
  mm_set_coordinate(&matcode);
  mm_set_real(&matcode);
  
  mm_write_banner(f, matcode); 

  //  mm_write_mtx_crd_size(f, this->ncol_, this->nrow_, this->nnz_);
  mm_write_mtx_crd_size(f, this->nrow_, this->ncol_, this->nnz_);

  /* NOTE: matrix market files use 1-based indices, i.e. first element
     of a vector has index 1, not 0.  */

  for (int i=0; i<this->nnz_; i++)
    fprintf(f, "%d %d %2.14e\n",
            this->mat_.row[i]+1, 
            this->mat_.col[i]+1, 
            this->mat_.val[i]);
  
  LOG_INFO("WriteFileMTX: filename="<< filename << "; done");

  fclose(f);

}

#ifdef SUPPORT_MKL

template <>
void HostMatrixCOO<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);
 
  char transp='N';
  int nrow = this->nrow_;
  int nnz  = this->nnz_;

  mkl_cspblas_dcoogemv(&transp, &nrow,
                       this->mat_.val, this->mat_.row, this->mat_.col,
                       &nnz,
                       cast_in->vec_, cast_out->vec_);

}

template <>
void HostMatrixCOO<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {
  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<float> *cast_in = dynamic_cast<const HostVector<float>*> (&in) ; 
  HostVector<float> *cast_out      = dynamic_cast<      HostVector<float>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  char transp='N';
  int nrow = this->nrow_;
  int nnz  = this->nnz_;

  mkl_cspblas_scoogemv(&transp, &nrow,
                       this->mat_.val, this->mat_.row, this->mat_.col,
                       &nnz,
                       cast_in->vec_, cast_out->vec_);

}

#else

template <typename ValueType>
void HostMatrixCOO<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for      
  for (int i=0; i<this->nrow_; ++i)
    cast_out->vec_[i] = ValueType(0.0);  

  for (int i=0; i<this->nnz_; ++i)
    cast_out->vec_[this->mat_.row[i] ] += this->mat_.val[i] * cast_in->vec_[ this->mat_.col[i] ];

}

#endif

template <typename ValueType>
void HostMatrixCOO<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                        BaseVector<ValueType> *out) const {

  if (this->nnz_ > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->ncol_);
    assert(out->get_size() == this->nrow_);
    
    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    for (int i=0; i<this->nnz_; ++i)
      cast_out->vec_[this->mat_.row[i] ] += scalar*this->mat_.val[i] * cast_in->vec_[ this->mat_.col[i] ];
  }

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::Permute(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);

  // symmetric permutation only
  assert( (permutation.get_size() == this->nrow_) &&
          (permutation.get_size() == this->ncol_) );

  const HostVector<int> *cast_perm = dynamic_cast<const HostVector<int>*> (&permutation) ;   
  assert(cast_perm != NULL);

  HostMatrixCOO<ValueType> src(this->local_backend_);       
  src.AllocateCOO(this->nnz_, this->nrow_, this->ncol_);
  src.CopyFrom(*this);

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i) {
 
    this->mat_.row[i] = cast_perm->vec_[ src.mat_.row[i] ];
    this->mat_.col[i] = cast_perm->vec_[ src.mat_.col[i] ];

  }

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);

  // symmetric permutation only
  assert( (permutation.get_size() == this->nrow_) &&
          (permutation.get_size() == this->ncol_) );

  const HostVector<int> *cast_perm = dynamic_cast<const HostVector<int>*> (&permutation);
  assert(cast_perm != NULL);

  HostMatrixCOO<ValueType> src(this->local_backend_);       
  src.AllocateCOO(this->nnz_, this->nrow_, this->ncol_);
  src.CopyFrom(*this);

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

  // TODO 
  // Is there a better way?
  int *pb = NULL;
  allocate_host(this->nrow_, &pb);

#pragma omp parallel for      
  for (int i=0; i<this->nrow_; ++i)
    pb [ cast_perm->vec_[i] ] = i;

#pragma omp parallel for      
  for (int i=0; i<this->nnz_; ++i) {

    this->mat_.row[i] = pb[src.mat_.row[i]];
    this->mat_.col[i] = pb[src.mat_.col[i]];

  }

  free_host(&pb);

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::Scale(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i)
      this->mat_.val[i] *= alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::ScaleDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i)
    if (this->mat_.row[i] == this->mat_.col[i])
      this->mat_.val[i] *= alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::ScaleOffDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i)
    if (this->mat_.row[i] != this->mat_.col[i])
      this->mat_.val[i] *= alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::AddScalar(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i)
    this->mat_.val[i] += alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::AddScalarDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i)
    if (this->mat_.row[i] == this->mat_.col[i])
      this->mat_.val[i] += alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCOO<ValueType>::AddScalarOffDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nnz_);  

#pragma omp parallel for
  for (int i=0; i<this->nnz_; ++i)
    if (this->mat_.row[i] != this->mat_.col[i])
      this->mat_.val[i] += alpha;

  return true;

}


template class HostMatrixCOO<double>;
template class HostMatrixCOO<float>;

}

