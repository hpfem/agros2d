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


#include "host_matrix_csr.hpp"
#include "host_matrix_coo.hpp"
#include "host_matrix_mcsr.hpp"
#include "host_matrix_bcsr.hpp"
#include "host_matrix_dia.hpp"
#include "host_matrix_ell.hpp"
#include "host_matrix_hyb.hpp"
#include "host_matrix_dense.hpp"
#include "host_conversion.hpp"
#include "host_vector.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../../utils/math_functions.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <limits>

#ifdef _OPENMP
  #include <omp.h>
#else
  #define omp_set_num_threads(num) ;
  #define omp_get_max_threads() 1
  #define omp_get_thread_num() 0
  #define omp_get_num_threads() 1
  #define omp_set_nested(num)  ;
#endif

#ifdef SUPPORT_MKL
  #include <mkl.h>
  #include <mkl_spblas.h>
#endif

namespace paralution {

template <typename ValueType>
HostMatrixCSR<ValueType>::HostMatrixCSR() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixCSR<ValueType>::HostMatrixCSR(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "HostMatrixCSR::HostMatrixCSR()",
            "constructor with local_backend");

  this->mat_.row_offset = NULL;  
  this->mat_.col        = NULL;  
  this->mat_.val        = NULL;
  this->set_backend(local_backend); 
  
#ifdef SUPPORT_MKL
  this->mkl_tmp_vec_ = NULL;
#endif

  this->L_diag_unit_ = true;
  this->U_diag_unit_ = false;

}

template <typename ValueType>
HostMatrixCSR<ValueType>::~HostMatrixCSR() {

  LOG_DEBUG(this, "HostMatrixCSR::~HostMatrixCSR()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::Clear() {

  if (this->nnz_ > 0) {

    free_host(&this->mat_.row_offset);
    free_host(&this->mat_.col);
    free_host(&this->mat_.val);
    
    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

#ifdef SUPPORT_MKL
    if (this->mkl_tmp_vec_ != NULL) {
      free_host(&this->mkl_tmp_vec_);
      this->mkl_tmp_vec_ = NULL;
    }
#endif
    
  }

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::Zeros(void) {

  if (this->nnz_ > 0)
    set_to_zero_host(this->nnz_, mat_.val);  

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::info(void) const {

  LOG_INFO("HostMatrixCSR<ValueType>, OpenMP threads: " << this->local_backend_.OpenMP_threads);

}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::Check(void) const {

  bool sorted = true;

  // check nnz
  if ((paralution_abs(this->nnz_) == 
       std::numeric_limits<ValueType>::infinity()) || // inf
      ( this->nnz_ != this->nnz_)) { // NaN
    LOG_VERBOSE_INFO(2,"*** error: Matrix CSR:Check - problems with matrix nnz"); 
    return false;
  }

  // nrow
  if ((paralution_abs(this->nrow_) == 
       std::numeric_limits<ValueType>::infinity()) || // inf
      ( this->nrow_ != this->nrow_)) { // NaN
    LOG_VERBOSE_INFO(2,"*** error: Matrix CSR:Check - problems with matrix nrow"); 
    return false;
  }

  // ncol
  if ((paralution_abs(this->ncol_) == 
       std::numeric_limits<ValueType>::infinity()) || // inf
      ( this->ncol_ != this->ncol_)) { // NaN
    LOG_VERBOSE_INFO(2,"*** error: Matrix CSR:Check - problems with matrix ncol"); 
    return false;
  }


  if (this->nnz_ > 0) {

    for (int ai=0; ai<this->nrow_+1; ++ai) { 
      int row = this->mat_.row_offset[ai];
      if ((row < 0) ||
          (row > this->nnz_)) {
        LOG_VERBOSE_INFO(2,"*** error: Matrix CSR:Check - problems with matrix row offset pointers"); 
        return false;
      }
    }

    for (int ai=0; ai<this->nrow_; ++ai) {

      int s = this->mat_.col[this->mat_.row_offset[ai]];

      for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {

        int col = this->mat_.col[aj];

        if ((col < 0) ||
            (col  > this->ncol_) ||
            (col == std::numeric_limits<ValueType>::infinity()) ||      
            (col != col)) {
          LOG_VERBOSE_INFO(2,"*** error: Matrix CSR:Check - problems with matrix col values");
          return false;
        }

        ValueType val = this->mat_.val[aj];
        if ((val == std::numeric_limits<ValueType>::infinity()) ||      
            (val != val)) {
          LOG_VERBOSE_INFO(2,"*** error: Matrix CSR:Check - problems with matrix values");
          return false;
        }

        if ((aj > this->mat_.row_offset[ai]) &&
            (s >= col)) {
          sorted = false;
        }

        s = this->mat_.col[aj];
          
      }
    
    }

  }

  if (sorted == false)
    LOG_VERBOSE_INFO(2,"*** warning: Matrix CSR:Check - the matrix has not sorted columns");
  
  return true;


}


template <typename ValueType>
void HostMatrixCSR<ValueType>::AllocateCSR(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->nnz_ > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_host(nrow+1, &this->mat_.row_offset);
    allocate_host(nnz,    &this->mat_.col);
    allocate_host(nnz,    &this->mat_.val);
    
    set_to_zero_host(nrow+1, mat_.row_offset);
    set_to_zero_host(nnz, mat_.col);
    set_to_zero_host(nnz, mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

#ifdef SUPPORT_MKL
    allocate_host(nrow, &this->mkl_tmp_vec_);
#endif

  }

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::SetDataPtrCSR(int **row_offset, int **col, ValueType **val,
                                             const int nnz, const int nrow, const int ncol) {

  assert(*row_offset != NULL);
  assert(*col != NULL);
  assert(*val != NULL);
  assert(nnz > 0);
  assert(nrow > 0);
  assert(ncol > 0);

  this->Clear();

  this->nrow_ = nrow;
  this->ncol_ = ncol;
  this->nnz_  = nnz;

  this->mat_.row_offset = *row_offset;
  this->mat_.col = *col;
  this->mat_.val = *val;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::LeaveDataPtrCSR(int **row_offset, int **col, ValueType **val) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);

  // see free_host function for details
  *row_offset = this->mat_.row_offset;
  *col = this->mat_.col;
  *val = this->mat_.val;

  this->mat_.row_offset = NULL;
  this->mat_.col = NULL;
  this->mat_.val = NULL;

  this->nrow_ = 0;
  this->ncol_ = 0;
  this->nnz_  = 0;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val) {

  assert(this->nnz_ > 0);
  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  
  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for      
  for (int i=0; i<this->nrow_+1; ++i)
    this->mat_.row_offset[i] = row_offsets[i];
  
#pragma omp parallel for      
  for (int j=0; j<this->nnz_; ++j) {
    this->mat_.col[j] = col[j];
    this->mat_.val[j] = val[j];
  }
  
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::CopyToCSR(int *row_offsets, int *col, ValueType *val) const {

  assert(this->nnz_ > 0);
  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for      
  for (int i=0; i<this->nrow_+1; ++i)
    row_offsets[i] = this->mat_.row_offset[i]; 

#pragma omp parallel for
  for (int j=0; j<this->nnz_; ++j) {
    col[j] = this->mat_.col[j];
    val[j] = this->mat_.val[j];
  }

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    if (this->nnz_ == 0)
      this->AllocateCSR(cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_ );

    assert((this->nnz_  == cast_mat->nnz_)  &&
           (this->nrow_ == cast_mat->nrow_) &&
           (this->ncol_ == cast_mat->ncol_) );
        
    if (this->nnz_ > 0) {

      _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for      
      for (int i=0; i<this->nrow_+1; ++i)
        this->mat_.row_offset[i] = cast_mat->mat_.row_offset[i] ;

#pragma omp parallel for
      for (int j=0; j<this->nnz_; ++j) {
        this->mat_.col[j] = cast_mat->mat_.col[j];
        this->mat_.val[j] = cast_mat->mat_.val[j];
      }

    }

  } else {

    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);

  }

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::ReadFileCSR(const std::string filename) { 

  LOG_INFO("ReadFileCSR: filename="<< filename << "; reading...");

  std::ifstream out(filename.c_str(), std::ios::in | std::ios::binary);

  int nrow;
  int ncol;
  int nnz;

  out.read((char*)&nrow, sizeof(int));
  out.read((char*)&ncol, sizeof(int));
  out.read((char*)&nnz, sizeof(int));

  this->AllocateCSR(nnz, nrow, ncol);

  out.read((char*)this->mat_.row_offset, (nrow+1)*sizeof(int));
  out.read((char*)this->mat_.col, nnz*sizeof(int));
  out.read((char*)this->mat_.val, nnz*sizeof(ValueType));

  LOG_INFO("ReadFileCSR: filename="<< filename << "; done");

  out.close();

  // Sorting the col (per row)
  // Bubble sort algorithm

 #pragma omp parallel for      
  for (int i=0; i<nrow; ++i)
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j)
      for (int jj=this->mat_.row_offset[i]; jj<this->mat_.row_offset[i+1]-1; ++jj)
        if (this->mat_.col[jj] > this->mat_.col[jj+1]) {
          //swap elements

          int ind = this->mat_.col[jj];
          ValueType val = this->mat_.val[jj];

          this->mat_.col[jj] = this->mat_.col[jj+1];
          this->mat_.val[jj] = this->mat_.val[jj+1];          

          this->mat_.col[jj+1] = ind;
          this->mat_.val[jj+1] = val;
        }


}

template <typename ValueType>
void HostMatrixCSR<ValueType>::WriteFileCSR(const std::string filename) const {

  LOG_INFO("WriteFileCSR: filename="<< filename << "; writing...");

  std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);

  out.write((char*)&this->nrow_, sizeof(int));
  out.write((char*)&this->ncol_, sizeof(int));
  out.write((char*)&this->nnz_, sizeof(int));
  out.write((char*)this->mat_.row_offset, (this->nrow_+1)*sizeof(int));
  out.write((char*)this->mat_.col, this->nnz_*sizeof(int));
  out.write((char*)this->mat_.val, this->nnz_*sizeof(ValueType));

  LOG_INFO("WriteFileCSR: filename="<< filename << "; done");

  out.close();

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {
    
    this->CopyFrom(*cast_mat);
    return true;
    
  }

  if (const HostMatrixCOO<ValueType> *cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&mat)) {

    this->Clear();
    coo_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_, 
               cast_mat->mat_, &this->mat_);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = cast_mat->nnz_;

    return true;

  }

  if (const HostMatrixDENSE<ValueType> *cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&mat)) {

    this->Clear();
    int nnz = 0;

    dense_to_csr(this->local_backend_.OpenMP_threads,
                 cast_mat->nrow_, cast_mat->ncol_, 
                 cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = nnz;

    return true;

  }


  if (const HostMatrixDIA<ValueType> *cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&mat)) {

    this->Clear();
    int nnz;

    dia_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_,
               cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = nnz;

    return true;

  }

  if (const HostMatrixELL<ValueType> *cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&mat)) {

    this->Clear();
    int nnz;

    ell_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_, 
               cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = nnz ;

    return true;

  }

  if (const HostMatrixMCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixMCSR<ValueType>*> (&mat)) {

    this->Clear();
    mcsr_to_csr(this->local_backend_.OpenMP_threads,
                cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_,
                cast_mat->mat_, &this->mat_);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = cast_mat->nnz_;

    return true;

  }

  if (const HostMatrixHYB<ValueType> *cast_mat = dynamic_cast<const HostMatrixHYB<ValueType>*> (&mat)) {

    this->Clear();
    
    int nnz;
    hyb_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_,
               cast_mat->ell_nnz_, cast_mat->coo_nnz_,
               cast_mat->mat_, 
               &this->mat_, &nnz);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_  = nnz;

    return true;

  }

  return false;

}

#ifdef SUPPORT_MKL

template <>
void HostMatrixCSR<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

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

  mkl_cspblas_dcsrgemv(&transp, &nrow,
                       this->mat_.val, this->mat_.row_offset, this->mat_.col,
                       cast_in->vec_, cast_out->vec_);
    
}

template <>
void HostMatrixCSR<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {

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

  mkl_cspblas_scsrgemv(&transp, &nrow,
                       this->mat_.val, this->mat_.row_offset, this->mat_.col,
                       cast_in->vec_, cast_out->vec_);
    
}

#else

template <typename ValueType>
void HostMatrixCSR<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) {

    ValueType sum = ValueType(0.0);
    int row_beg = this->mat_.row_offset[ai];
    int row_end = this->mat_.row_offset[ai+1];

    for (int aj=row_beg; aj<row_end; ++aj)
      sum += this->mat_.val[aj] * cast_in->vec_[this->mat_.col[aj]];

    cast_out->vec_[ai] = sum;

  }
    
}

#endif

template <typename ValueType>
void HostMatrixCSR<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    
    _set_omp_backend_threads(this->local_backend_, this->nrow_);  
    
#pragma omp parallel for
    for (int ai=0; ai<this->nrow_; ++ai)
      for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
        cast_out->vec_[ai] += scalar*this->mat_.val[aj] * cast_in->vec_[ this->mat_.col[aj] ];
  }
    
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractDiagonal(BaseVector<ValueType> *vec_diag) const {

  assert(vec_diag != NULL);
  assert(vec_diag->get_size() == this->nrow_);

  HostVector<ValueType> *cast_vec_diag  = dynamic_cast<HostVector<ValueType>*> (vec_diag) ; 

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {
      
      if (ai == this->mat_.col[aj]) {
        
        cast_vec_diag->vec_[ai] = this->mat_.val[aj];
        break;
        
      }
      
    }

  return true;
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractInverseDiagonal(BaseVector<ValueType> *vec_inv_diag) const {

  assert(vec_inv_diag != NULL);
  assert(vec_inv_diag->get_size() == this->nrow_);

  HostVector<ValueType> *cast_vec_inv_diag  = dynamic_cast<HostVector<ValueType>*> (vec_inv_diag) ; 

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {
      
      if (ai == this->mat_.col[aj]) {
        
        cast_vec_inv_diag->vec_[ai] = ValueType(1.0) / this->mat_.val[aj];
        break;
        
      }
      
    }
  
  return true;
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractSubMatrix(const int row_offset,
                                                const int col_offset,
                                                const int row_size,
                                                const int col_size,
                                                BaseMatrix<ValueType> *mat) const {

  assert(mat != NULL);

  assert(row_offset >= 0);
  assert(col_offset >= 0);

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  HostMatrixCSR<ValueType> *cast_mat  = dynamic_cast<HostMatrixCSR<ValueType>*> (mat) ; 
  assert(cast_mat != NULL);

  int mat_nnz = 0;

  // use omp in local_matrix (higher level)

  //  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

  // #pragma omp parallel for reduction(+:mat_nnz)
  for (int ai=row_offset; ai<row_offset+row_size; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if ((this->mat_.col[aj] >= col_offset) &&
          (this->mat_.col[aj] < col_offset + col_size) )
        ++mat_nnz;

  // not empty submatrix
  if (mat_nnz > 0) {

    cast_mat->AllocateCSR(mat_nnz, row_size, col_size);

    int mat_row_offset = 0;
    cast_mat->mat_.row_offset[0] = mat_row_offset;

    for (int ai=row_offset; ai<row_offset+row_size; ++ai) {

      for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
        if ((this->mat_.col[aj] >= col_offset) &&
            (this->mat_.col[aj] < col_offset + col_size) ) {

          cast_mat->mat_.col[mat_row_offset] = this->mat_.col[aj] - col_offset; 
          cast_mat->mat_.val[mat_row_offset] = this->mat_.val[aj]; 
          ++mat_row_offset;

        }

      cast_mat->mat_.row_offset[ai - row_offset+1] = mat_row_offset;
    }

    cast_mat->mat_.row_offset[row_size] = mat_row_offset;
    assert(mat_row_offset == mat_nnz);
  }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractU(BaseMatrix<ValueType> *U) const {

  assert(U != NULL);

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  HostMatrixCSR<ValueType> *cast_U = dynamic_cast<HostMatrixCSR<ValueType>*> (U);

  assert(cast_U != NULL);

  // count nnz of upper triangular part
  int nnz_U = 0;
#pragma omp parallel for reduction(+:nnz_U)
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] < ai)
        ++nnz_U;

  // allocate upper triangular part structure
  int *row_offset = NULL;
  int *col = NULL;
  ValueType *val = NULL;

  allocate_host(this->nrow_+1, &row_offset);
  allocate_host(nnz_U, &col);
  allocate_host(nnz_U, &val);

  // fill upper triangular part
  int nnz = 0;
  row_offset[0] = 0;
  for (int ai=0; ai<this->nrow_; ++ai) {
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] > ai) {
        col[nnz] = this->mat_.col[aj];
        val[nnz] = this->mat_.val[aj];
        ++nnz;
      }
    row_offset[ai+1] = nnz;
  }

  cast_U->Clear();
  cast_U->SetDataPtrCSR(&row_offset, &col, &val, nnz_U, this->nrow_, this->ncol_);

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractUDiagonal(BaseMatrix<ValueType> *U) const {

  assert(U != NULL);

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  HostMatrixCSR<ValueType> *cast_U = dynamic_cast<HostMatrixCSR<ValueType>*> (U);

  assert(cast_U != NULL);

  // count nnz of upper triangular part
  int nnz_U = 0;
#pragma omp parallel for reduction(+:nnz_U)
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] <= ai)
        ++nnz_U;

  // allocate upper triangular part structure
  int *row_offset = NULL;
  int *col = NULL;
  ValueType *val = NULL;

  allocate_host(this->nrow_+1, &row_offset);
  allocate_host(nnz_U, &col);
  allocate_host(nnz_U, &val);

  // fill upper triangular part
  int nnz = 0;
  row_offset[0] = 0;
  for (int ai=0; ai<this->nrow_; ++ai) {
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] >= ai) {
        col[nnz] = this->mat_.col[aj];
        val[nnz] = this->mat_.val[aj];
        ++nnz;
      }
    row_offset[ai+1] = nnz;
  }

  cast_U->Clear();
  cast_U->SetDataPtrCSR(&row_offset, &col, &val, nnz_U, this->nrow_, this->ncol_);

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractL(BaseMatrix<ValueType> *L) const {

  assert(L != NULL);

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  HostMatrixCSR<ValueType> *cast_L = dynamic_cast<HostMatrixCSR<ValueType>*> (L);

  assert(cast_L != NULL);

  // count nnz of lower triangular part
  int nnz_L = 0;
#pragma omp parallel for reduction(+:nnz_L)
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] < ai)
        ++nnz_L;

  // allocate lower triangular part structure
  int *row_offset = NULL;
  int *col = NULL;
  ValueType *val = NULL;

  allocate_host(this->nrow_+1, &row_offset);
  allocate_host(nnz_L, &col);
  allocate_host(nnz_L, &val);

  // fill lower triangular part
  int nnz = 0;
  row_offset[0] = 0;
  for (int ai=0; ai<this->nrow_; ++ai) {
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] < ai) {
        col[nnz] = this->mat_.col[aj];
        val[nnz] = this->mat_.val[aj];
        ++nnz;
      }
    row_offset[ai+1] = nnz;
  }

  cast_L->Clear();
  cast_L->SetDataPtrCSR(&row_offset, &col, &val, nnz_L, this->nrow_, this->ncol_);

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractLDiagonal(BaseMatrix<ValueType> *L) const {

  assert(L != NULL);

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);

  HostMatrixCSR<ValueType> *cast_L = dynamic_cast<HostMatrixCSR<ValueType>*> (L);

  assert(cast_L != NULL);

  // count nnz of lower triangular part
  int nnz_L = 0;
#pragma omp parallel for reduction(+:nnz_L)
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] <= ai)
        ++nnz_L;

  // allocate lower triangular part structure
  int *row_offset = NULL;
  int *col = NULL;
  ValueType *val = NULL;

  allocate_host(this->nrow_+1, &row_offset);
  allocate_host(nnz_L, &col);
  allocate_host(nnz_L, &val);

  // fill lower triangular part
  int nnz = 0;
  row_offset[0] = 0;
  for (int ai=0; ai<this->nrow_; ++ai) {
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] <= ai) {
        col[nnz] = this->mat_.col[aj];
        val[nnz] = this->mat_.val[aj];
        ++nnz;
      }
    row_offset[ai+1] = nnz;
  }

  cast_L->Clear();
  cast_L->SetDataPtrCSR(&row_offset, &col, &val, nnz_L, this->nrow_, this->ncol_);

  return true;

}

#ifdef SUPPORT_MKL

template <>
bool HostMatrixCSR<double>::LUSolve(const BaseVector<double> &in, BaseVector<double> *out) const {
  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  char transp = 'N';
  char matdescra[6];
  double one = double(1.0);
  int nrow = this->nrow_;
  
  matdescra[0] = 'T';
  matdescra[1] = 'L'; // L or U
  matdescra[2] = 'U'; // non-unit ot unit
  matdescra[3] = 'C'; // zero base

  mkl_dcsrsv(&transp, &nrow, &one, matdescra, 
             this->mat_.val, this->mat_.col, this->mat_.row_offset, &this->mat_.row_offset[1],
             cast_in->vec_, this->mkl_tmp_vec_);

  matdescra[0] = 'T';
  matdescra[1] = 'U';
  matdescra[2] = 'N';
  matdescra[3] = 'C';

  mkl_dcsrsv(&transp, &nrow, &one, matdescra, 
             this->mat_.val, this->mat_.col, this->mat_.row_offset, &this->mat_.row_offset[1],
             this->mkl_tmp_vec_, cast_out->vec_);

  return true;

}

template <>
bool HostMatrixCSR<float>::LUSolve(const BaseVector<float> &in, BaseVector<float> *out) const {

  LOG_INFO("No float MKL LU solver");
  FATAL_ERROR(__FILE__, __LINE__);
}

#else 

template <typename ValueType>
bool HostMatrixCSR<ValueType>::LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  // Solve L
  for (int ai=0; ai<this->nrow_; ++ai) {

    cast_out->vec_[ai] = cast_in->vec_[ai];

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (this->mat_.col[aj] < ai) {
        // under the diagonal
        cast_out->vec_[ai] -= this->mat_.val[aj] * cast_out->vec_[ this->mat_.col[aj] ];
      } else {
        // CSR should be sorted
        break;
      }

  }

  // last elements should be the diagonal one (last)
  int diag_aj = this->nnz_-1;

  // Solve U
  for (int ai=this->nrow_-1; ai>=0; --ai) {

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {

      if (this->mat_.col[aj] > ai) {
        // above the diagonal
        cast_out->vec_[ai] -= this->mat_.val[aj] * cast_out->vec_[ this->mat_.col[aj] ];
      } 
      
      if (this->mat_.col[aj] == ai) {
        diag_aj = aj;
      }
    }

    cast_out->vec_[ai] /= this->mat_.val[diag_aj];
  }

  return true;

}

#endif

template <typename ValueType>
void HostMatrixCSR<ValueType>::LLAnalyse(void) {
  // do nothing
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::LLAnalyseClear(void) {
  // do nothing
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::LUAnalyse(void) {
  // do nothing
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::LUAnalyseClear(void) {
  // do nothing
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::LLSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in);
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  // Solve L
  for (int ai=0; ai<this->nrow_; ++ai) {

    ValueType value = cast_in->vec_[ai];
    int diag_idx = this->mat_.row_offset[ai+1]-1;

    for (int aj = this->mat_.row_offset[ai]; aj<diag_idx; ++aj)
      value -= this->mat_.val[aj] * cast_out->vec_[this->mat_.col[aj]];

    cast_out->vec_[ai] = value / this->mat_.val[diag_idx];

  }

  // Solve L^T
  for (int ai=this->nrow_-1; ai>=0; --ai) {

    int diag_idx = this->mat_.row_offset[ai+1]-1;
    ValueType value = cast_out->vec_[ai] / this->mat_.val[diag_idx];

    for (int aj=this->mat_.row_offset[ai]; aj<diag_idx; ++aj)
      cast_out->vec_[this->mat_.col[aj]] -= value * this->mat_.val[aj];

    cast_out->vec_[ai] = value;

  }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::LLSolve(const BaseVector<ValueType> &in, const BaseVector<ValueType> &inv_diag,
                                       BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in   = dynamic_cast<const HostVector<ValueType>*> (&in);
  const HostVector<ValueType> *cast_diag = dynamic_cast<const HostVector<ValueType>*> (&inv_diag);
  HostVector<ValueType> *cast_out        = dynamic_cast<      HostVector<ValueType>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  // Solve L
  for (int ai=0; ai<this->nrow_; ++ai) {

    ValueType value = cast_in->vec_[ai];
    int diag_idx = this->mat_.row_offset[ai+1]-1;

    for (int aj = this->mat_.row_offset[ai]; aj<diag_idx; ++aj)
      value -= this->mat_.val[aj] * cast_out->vec_[this->mat_.col[aj]];

    cast_out->vec_[ai] = value * cast_diag->vec_[ai];

  }

  // Solve L^T
  for (int ai=this->nrow_-1; ai>=0; --ai) {

    int diag_idx = this->mat_.row_offset[ai+1]-1;
    ValueType value = cast_out->vec_[ai] * cast_diag->vec_[ai];

    for (int aj=this->mat_.row_offset[ai]; aj<diag_idx; ++aj)
      cast_out->vec_[this->mat_.col[aj]] -= value * this->mat_.val[aj];

    cast_out->vec_[ai] = value;

  }

  return true;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::LAnalyse(const bool diag_unit) {

  this->L_diag_unit_ = diag_unit;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::LAnalyseClear(void) {
  // do nothing
  this->L_diag_unit_ = true;
}

// TODO - make mkl interface
template <typename ValueType>
bool HostMatrixCSR<ValueType>::LSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  int diag_aj = 0;

  // Solve L
  for (int ai=0; ai<this->nrow_; ++ai) {
    
    cast_out->vec_[ai] = cast_in->vec_[ai];
    
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {
      
      if (this->mat_.col[aj] < ai) {
        // under the diagonal
        cast_out->vec_[ai] -= this->mat_.val[aj] * cast_out->vec_[ this->mat_.col[aj] ];
      } else {
        
        // CSR should be sorted
        if (this->L_diag_unit_ == false) {
          assert(this->mat_.col[aj] == ai);
          diag_aj = aj;        
        }
        break;
      }
    }

    if (this->L_diag_unit_ == false) 
      cast_out->vec_[ai] /= this->mat_.val[diag_aj];
    
  }

  return true;
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::UAnalyse(const bool diag_unit) {

  this->U_diag_unit_ = diag_unit;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::UAnalyseClear(void) {
  // do nothing
  this->U_diag_unit_ = false;
}

// TODO - make mkl interface
template <typename ValueType>
bool HostMatrixCSR<ValueType>::USolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  // last elements should the diagonal one (last)
  int diag_aj = this->nnz_-1;

  // Solve U
  for (int ai=this->nrow_-1; ai>=0; --ai) {

    cast_out->vec_[ai] = cast_in->vec_[ai];

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {

      if (this->mat_.col[aj] > ai) {
        // above the diagonal
        cast_out->vec_[ai] -= this->mat_.val[aj] * cast_out->vec_[ this->mat_.col[aj] ];
      } 

      if (this->L_diag_unit_ == false)      
        if (this->mat_.col[aj] == ai) {
          diag_aj = aj;
        }
    }

    if (this->L_diag_unit_ == false) 
    cast_out->vec_[ai] /= this->mat_.val[diag_aj];
  }

  return true;
}


#ifdef SUPPORT_MKL

template <>
bool HostMatrixCSR<double>::ILU0Factorize(void) {

  //TODO define ipar?
  int ipar[128];
  double dpar[128];
  int ierr;

  // go to one-based array
  for (int i=0; i<this->nrow_+1; ++i)
    ++this->mat_.row_offset[i];

  // go to one-based array
  for (int i=0; i<this->nnz_; ++i)
    ++this->mat_.col[i];

  dcsrilu0(&this->nrow_,
           this->mat_.val, this->mat_.row_offset, this->mat_.col,
           this->mat_.val,
           ipar, dpar,
           &ierr);
					 
							 
  // go to one-based array
  for (int i=0; i<this->nrow_+1; ++i)
    --this->mat_.row_offset[i];

  // go to one-based array
  for (int i=0; i<this->nnz_; ++i)
    --this->mat_.col[i];

  
  return true;
}

template <>
bool HostMatrixCSR<float>::ILU0Factorize(void) {

  LOG_INFO("No float MKL ILU0 factorization");
  FATAL_ERROR(__FILE__, __LINE__); 
}


#else 

// Algorithm for ILU factorization is based on
// Y. Saad, Iterative methods for sparse linear systems, 2nd edition, SIAM
template <typename ValueType>
bool HostMatrixCSR<ValueType>::ILU0Factorize(void) {

  assert(this->nrow_ == this->ncol_);
  assert(this->nnz_ > 0);

  // pointer of upper part of each row
  int *diag_offset = NULL;
  int *nnz_entries = NULL;

  allocate_host(this->nrow_, &diag_offset);
  allocate_host(this->nrow_, &nnz_entries);

  for (int i=0; i<this->nrow_; ++i)
    nnz_entries[i] = 0;

  // ai = 0 to N loop over all rows
  for (int ai=0; ai<this->nrow_; ++ai) {

    // ai-th row entries
    int row_start = this->mat_.row_offset[ai];
    int row_end   = this->mat_.row_offset[ai+1];
    int j;

    // nnz position of ai-th row in mat_.val array
    for (j=row_start; j<row_end; ++j)
      nnz_entries[this->mat_.col[j]] = j;

    // loop over ai-th row nnz entries
    for (j=row_start; j<row_end; ++j) {

      // if nnz entry is in lower matrix
      if (this->mat_.col[j] < ai) {

        int col_j  = this->mat_.col[j];
        int diag_j = diag_offset[col_j];

        if (this->mat_.val[diag_j] != 0.0) {

          // multiplication factor
          this->mat_.val[j] = this->mat_.val[j] / this->mat_.val[diag_j];

          // loop over upper offset pointer and do linear combination for nnz entry
          for (int k=diag_j+1; k<this->mat_.row_offset[col_j+1]; ++k)

            // if nnz at this position do linear combination
            if (nnz_entries[this->mat_.col[k]] != 0)
              this->mat_.val[nnz_entries[this->mat_.col[k]]] -= this->mat_.val[j] * this->mat_.val[k];
        }

      } else
        break;
    }

    // set diagonal pointer to diagonal element
    diag_offset[ai] = j;

    // clear nnz entries
    for (j=row_start; j<row_end; ++j)
      nnz_entries[this->mat_.col[j]] = 0;

  }

  free_host(&diag_offset);
  free_host(&nnz_entries);

  return true;

}
#endif

// Algorithm for ILUT factorization is based on
// Y. Saad, Iterative methods for sparse linear systems, 2nd edition, SIAM
template <typename ValueType>
bool HostMatrixCSR<ValueType>::ILUTFactorize(const ValueType t, const int maxrow) {

  assert(this->nrow_ == this->ncol_);
  assert(this->nnz_ > 0);

  int nrow = this->nrow_;
  int ncol = this->ncol_;

  int *row_offset = NULL;
  int *diag_offset = NULL;
  int *nnz_entries = NULL;
  bool *nnz_pos = (bool*) malloc(nrow*sizeof(bool));
  ValueType *w = NULL;

  allocate_host(nrow+1, &row_offset);
  allocate_host(nrow, &diag_offset);
  allocate_host(nrow, &nnz_entries);
  allocate_host(nrow, &w);

  for (int i=0; i<nrow; ++i) {
    w[i] = 0.0;
    nnz_entries[i] = -1;
    nnz_pos[i] = false;
    diag_offset[i] = 0;
  }

  // pre-allocate 1.5x nnz arrays for preconditioner matrix
  int nnzA = this->nnz_;
  int alloc_size = int(nnzA*1.5);
  int *col = (int*) malloc(alloc_size*sizeof(int));
  ValueType *val = (ValueType*) malloc(alloc_size*sizeof(ValueType));

  // initialize row_offset
  row_offset[0] = 0;
  int nnz = 0;

  // loop over all rows
  for (int ai=0; ai<this->nrow_; ++ai) {

    row_offset[ai+1] = row_offset[ai];

    int row_begin = this->mat_.row_offset[ai];
    int row_end   = this->mat_.row_offset[ai+1];
    ValueType row_norm = ValueType(0.0);

    // fill working array with ai-th row
    int m = 0;
    for (int aj=row_begin; aj<row_end; ++aj) {

      int idx = this->mat_.col[aj];
      w[idx] = this->mat_.val[aj];
      nnz_entries[m] = idx;
      nnz_pos[idx] = true;

      row_norm += paralution_abs(this->mat_.val[aj]);
      ++m;
    }

    // threshold for dropping strategy
    ValueType threshold = t * row_norm / (row_end - row_begin);

    for (int k=0; k<nrow; ++k) {

      if (nnz_entries[k] == -1) break;

      int aj = nnz_entries[k];

      // get smallest column index
      int sidx = k;
      for (int j=k+1; j<nrow; ++j) {
        if (nnz_entries[j] == -1) break;
        if (nnz_entries[j] < aj) {
          sidx = j;
          aj = nnz_entries[j];
        }
      }

      aj = nnz_entries[k];

      // swap column index
      if (k != sidx) {
        nnz_entries[k] = nnz_entries[sidx];
        nnz_entries[sidx] = aj;
        aj = nnz_entries[k];
      }

      // lower matrix part
      if (aj < ai) {

        // if zero diagonal entry do nothing
        if (val[diag_offset[aj]] == 0.0) {
          LOG_INFO("(ILUT) zero row");
          continue;
        }

        w[aj] /= val[diag_offset[aj]];

        // do linear combination with previous row
        for (int l=diag_offset[aj]+1; l<row_offset[aj+1]; ++l) {

          int idx = col[l];
          ValueType fillin = w[aj] * val[l];

          // drop off strategy for fill in
          if (nnz_pos[idx] == false) {
            if (paralution_abs(fillin) >= threshold) {

              nnz_entries[m] = idx;
              nnz_pos[idx] = true;
              w[idx] -= fillin;
              ++m;

            }

          } else
            w[idx] -= fillin;

        }

      }

    }

    // fill ai-th row of preconditioner matrix
    for (int k=0, num_lower=0, num_upper=0; k<nrow; ++k) {

      int aj = nnz_entries[k];

      if (aj == -1)
        break;

      if (nnz_pos[aj] == false)
        break;

      // lower part
      if (aj < ai && num_lower < maxrow) {

        val[nnz] = w[aj];
        col[nnz] = aj;

        ++row_offset[ai+1];
        ++num_lower;
        ++nnz;

      // upper part
      } else if (aj > ai && num_upper < maxrow) {

        val[nnz] = w[aj];
        col[nnz] = aj;

        ++row_offset[ai+1];
        ++num_upper;
        ++nnz;

      // diagonal part
      } else if (aj == ai) {

        val[nnz] = w[aj];
        col[nnz] = aj;

        diag_offset[ai] = row_offset[ai+1];

        ++row_offset[ai+1];
        ++nnz;

      }

      // clear working arrays
      w[aj] = 0.0;
      nnz_entries[k] = -1;
      nnz_pos[aj] = false;

    }

    // resize preconditioner matrix if needed
    if (alloc_size < nnz + 2 * maxrow + 1) {
      alloc_size += int(nnzA*1.5);
      col = (int*) realloc(col, alloc_size*sizeof(int));
      val = (ValueType*) realloc(val, alloc_size*sizeof(ValueType));
    }

  }

  //  col = (int*) realloc(col, nnz*sizeof(int));
  //  val = (ValueType*) realloc(val, nnz*sizeof(ValueType));

  free_host(&w);
  free_host(&diag_offset);
  free_host(&nnz_entries);
  free(nnz_pos);

  // pinned memory
  int *p_col = NULL;
  ValueType *p_val = NULL;

  allocate_host(nnz, &p_col);
  allocate_host(nnz, &p_val);

  for (int i=0; i<nnz; ++i) {
    p_col[i] = col[i];
    p_val[i] = val[i];
  }

  free(col);
  free(val);

  this->Clear();
  this->SetDataPtrCSR(&row_offset, &p_col, &p_val, nnz, nrow, ncol);

#ifdef SUPPORT_MKL
  allocate_host(nrow, &this->mkl_tmp_vec_);
#endif

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ICFactorize(BaseVector<ValueType> *inv_diag) {

  assert(this->nrow_ == this->ncol_);
  assert(this->nnz_ > 0);

  assert(inv_diag != NULL);
  HostVector<ValueType> *cast_diag = dynamic_cast<HostVector<ValueType>*> (inv_diag);
  assert(cast_diag != NULL);

  cast_diag->Allocate(this->nrow_);

  // i=0,..n
  for (int i=0; i<this->nrow_; ++i)
    // j=0,..i
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      // k=0,..j-1
      for (int k=this->mat_.row_offset[i]; k<j; ++k)
        // loop through j-th row to find matching k
        for (int l=this->mat_.row_offset[this->mat_.col[j]]; l<this->mat_.row_offset[this->mat_.col[j]+1]; ++l)
          if (this->mat_.col[l] == this->mat_.col[k]) {
            this->mat_.val[j] -= this->mat_.val[k] * this->mat_.val[l];
            break;
          }

      if (i > this->mat_.col[j]) {
        // Fill lower part
        this->mat_.val[j] /= this->mat_.val[this->mat_.row_offset[this->mat_.col[j]+1]-1];
      } else if (this->mat_.val[j] > 0.0) {
        // Fill diagonal part
        this->mat_.val[j] = sqrt(this->mat_.val[j]);
        cast_diag->vec_[i] = ValueType(1.0) / this->mat_.val[j];
      } else {
        LOG_INFO("IC breakdown");
        FATAL_ERROR(__FILE__, __LINE__);
      }

    }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::MultiColoring(int &num_colors,
                                             int **size_colors,
                                             BaseVector<int> *permutation) const {

  assert(permutation != NULL);
  HostVector<int> *cast_perm = dynamic_cast<HostVector<int>*> (permutation) ; 
  assert(cast_perm != NULL);

  // node colors (init value = 0 i.e. no color)
  int *color = NULL;
  allocate_host(this->nrow_, &color);

  memset(color, 0, sizeof(int)*this->nrow_);
  num_colors = 0;
  std::vector<bool> row_col;

  for (int ai=0; ai<this->nrow_; ++ai) {
    color[ai] = 1;
    row_col.clear();
    row_col.assign(num_colors+2, false);

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai != this->mat_.col[aj])
        row_col[color[this->mat_.col[aj]]] = true;

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (row_col[color[ai]] == true)
        ++color[ai];

    if (color[ai] > num_colors)
      num_colors = color[ai];

  }

  allocate_host(num_colors, size_colors);
  set_to_zero_host(num_colors, *size_colors);

  int *offsets_color = NULL;
  allocate_host(num_colors, &offsets_color);
  memset(offsets_color, 0, sizeof(int)*num_colors);

  for (int i=0; i<this->nrow_; ++i) 
    ++(*size_colors)[color[i]-1];

  int total=0;
  for (int i=1; i<num_colors; ++i) {

    total += (*size_colors)[i-1];
    offsets_color[i] = total; 
    //   LOG_INFO("offsets = " << total);

  }

  cast_perm->Allocate(this->nrow_);

  for (int i=0; i<permutation->get_size(); ++i) {

    cast_perm->vec_[ i ] = offsets_color[ color[i]-1 ] ;
    ++offsets_color[color[i]-1];

  }

  free_host(&color);
  free_host(&offsets_color);

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::MaximalIndependentSet(int &size,
                                                     BaseVector<int> *permutation) const {

  assert(permutation != NULL);
  assert(this->nrow_ == this->ncol_);

  HostVector<int> *cast_perm = dynamic_cast<HostVector<int>*> (permutation) ; 
  assert(cast_perm != NULL);


  int *mis = NULL;
  allocate_host(this->nrow_, &mis);
  memset(mis, 0, sizeof(int)*this->nrow_);

  size = 0 ;

  for (int ai=0; ai<this->nrow_; ++ai) {

    if (mis[ai] == 0) {

      // set the node
      mis[ai] = 1;
      ++size ;

      //remove all nbh nodes (without diagonal)
      for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
        if (ai != this->mat_.col[aj])
          mis[this->mat_.col[aj]] = -1 ;
      
    }
  }

  cast_perm->Allocate(this->nrow_);

  int pos = 0;
  for (int ai=0; ai<this->nrow_; ++ai) {

    if (mis[ai] == 1) {

      cast_perm->vec_[ai] = pos;
      ++pos;

    } else {

      cast_perm->vec_[ai] = size + ai - pos;
     
    }

  }

  // Check the permutation
  //
  //  for (int ai=0; ai<this->nrow_; ++ai) {
  //    assert( cast_perm->vec_[ai] >= 0 );
  //    assert( cast_perm->vec_[ai] < this->nrow_ );
  //  }

  free_host(&mis);

  return true;
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::ZeroBlockPermutation(int &size,
                                                    BaseVector<int> *permutation) const {

  assert(permutation != NULL);
  assert( (permutation->get_size() == this->nrow_) &&
          (permutation->get_size() == this->ncol_) );
  
  HostVector<int> *cast_perm = dynamic_cast<HostVector<int>*> (permutation) ; 
  assert(cast_perm != NULL);

  size = 0;

  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai == this->mat_.col[aj])
        ++size;


  int k_z  = size ;
  int k_nz = 0 ;

  for (int ai=0; ai<this->nrow_; ++ai) {

    bool hit = false ;

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai == this->mat_.col[aj]) {

        cast_perm->vec_[ai] = k_nz;
        ++k_nz;
        hit = true ;

      }

    if (hit == false) {
        cast_perm->vec_[ai] = k_z;
        ++k_z;
    }

  }

}

// following R.E.Bank and C.C.Douglas paper
template <typename ValueType>
void HostMatrixCSR<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &src) {

  assert(&src != NULL);

  const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&src);

  assert(cast_mat != NULL);
  assert(this->ncol_ == cast_mat->nrow_);

  std::vector<int> row_offset; 
  std::vector<int> *new_col = new std::vector<int> [this->nrow_];
  
  row_offset.resize(this->nrow_+1);

  row_offset[0] = 0;

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for  
  for (int i=0; i<this->nrow_; ++i) {

    // loop over the row
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      int ii = this->mat_.col[j];

      // loop corresponding row
      for (int k=cast_mat->mat_.row_offset[ii]; k<cast_mat->mat_.row_offset[ii+1]; ++k)         
        new_col[i].push_back(cast_mat->mat_.col[k]);                
      
    }
    
    std::sort( new_col[i].begin(), new_col[i].end() );
    new_col[i].erase( std::unique( new_col[i].begin(), new_col[i].end() ), new_col[i].end() );

    row_offset[i+1] = int(new_col[i].size());

  }

  for (int i=0; i<this->nrow_; ++i)
    row_offset[i+1] += row_offset[i];
    

  this->AllocateCSR(row_offset[this->nrow_], this->nrow_, this->ncol_);

#pragma omp parallel for  
  for (int i=0; i<this->nrow_+1; ++i)
    this->mat_.row_offset[i] = row_offset[i];


#pragma omp parallel for
  for (int i=0; i<this->nrow_; ++i) {
    int jj=0;
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      this->mat_.col[j] = new_col[i][jj]; 
      ++jj;
    }
  }
    
  //#pragma omp parallel for
  //  for (unsigned int i=0; i<this->nnz_; ++i)
  //    this->mat_.val[i] = ValueType(1.0);
  
  delete[] new_col;

}

// ----------------------------------------------------------
// original function prod(const spmat1 &A, const spmat2 &B)
// ----------------------------------------------------------
// Modified and adopted from AMGCL,
// https://github.com/ddemidov/amgcl
// MIT License
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// sorting is added
// ----------------------------------------------------------
template <typename ValueType>
bool HostMatrixCSR<ValueType>::MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  assert((this != &A) && (this != &B));

  //  this->SymbolicMatMatMult(A, B);
  //  this->NumericMatMatMult (A, B);
  //
  //  return true;

  const HostMatrixCSR<ValueType> *cast_mat_A = dynamic_cast<const HostMatrixCSR<ValueType>*> (&A);
  const HostMatrixCSR<ValueType> *cast_mat_B = dynamic_cast<const HostMatrixCSR<ValueType>*> (&B);

  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);
  assert(cast_mat_A->ncol_ == cast_mat_B->nrow_);

  int n = cast_mat_A->nrow_;
  int m = cast_mat_B->ncol_;


  int *row_offset = NULL;
  allocate_host(n+1, &row_offset);
  int *col = NULL;
  ValueType *val = NULL;

  for (int i=0; i<n+1; ++i)
    row_offset[i] = 0;

#pragma omp parallel
  {
    std::vector<int> marker(m, -1);

#ifdef _OPENMP
          int nt  = omp_get_num_threads();
	  int tid = omp_get_thread_num();

	  int chunk_size  = (n + nt - 1) / nt;
	  int chunk_start = tid * chunk_size;
	  int chunk_end   = std::min(n, chunk_start + chunk_size);
#else
	  int chunk_start = 0;
	  int chunk_end   = n;
#endif

    for (int ia=chunk_start; ia<chunk_end; ++ia) {
      for (int ja=cast_mat_A->mat_.row_offset[ia], ea=cast_mat_A->mat_.row_offset[ia+1]; ja<ea; ++ja) {
        int ca = cast_mat_A->mat_.col[ja];
        for (int jb=cast_mat_B->mat_.row_offset[ca], eb=cast_mat_B->mat_.row_offset[ca+1]; jb<eb; ++jb) {
          int cb = cast_mat_B->mat_.col[jb];

          if (marker[cb] != ia) {
            marker[cb] = ia;
            ++row_offset[ia+1];
          }
        }
      }
    }

    std::fill(marker.begin(), marker.end(), -1);

#pragma omp barrier
#pragma omp single
    {
      for (int i=1; i<n+1; ++i)
        row_offset[i] += row_offset[i-1];

      allocate_host(row_offset[n], &col);
      allocate_host(row_offset[n], &val);
    }

    for (int ia=chunk_start; ia<chunk_end; ++ia) {
      int row_begin = row_offset[ia];
      int row_end   = row_begin;

      for (int ja=cast_mat_A->mat_.row_offset[ia], ea=cast_mat_A->mat_.row_offset[ia+1]; ja<ea; ++ja) {
        int ca = cast_mat_A->mat_.col[ja];
        ValueType va = cast_mat_A->mat_.val[ja];

        for (int jb=cast_mat_B->mat_.row_offset[ca], eb=cast_mat_B->mat_.row_offset[ca+1]; jb<eb; ++jb) {
          int cb = cast_mat_B->mat_.col[jb];
          ValueType vb = cast_mat_B->mat_.val[jb];

          if (marker[cb] < row_begin) {
            marker[cb] = row_end;
            col[row_end] = cb;
            val[row_end] = va * vb;
            ++row_end;
          } else {
            val[marker[cb]] += va * vb;
          }
        }
      }
    }
  }

  this->SetDataPtrCSR(&row_offset, &col, &val, row_offset[n], cast_mat_A->nrow_, cast_mat_B->ncol_);

  // Sorting the col (per row)
  // Bubble sort algorithm

 #pragma omp parallel for      
  for (int i=0; i<this->nrow_; ++i)
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j)
      for (int jj=this->mat_.row_offset[i]; jj<this->mat_.row_offset[i+1]-1; ++jj)
        if (this->mat_.col[jj] > this->mat_.col[jj+1]) {
          //swap elements

          int ind = this->mat_.col[jj];
          ValueType val = this->mat_.val[jj];

          this->mat_.col[jj] = this->mat_.col[jj+1];
          this->mat_.val[jj] = this->mat_.val[jj+1];          

          this->mat_.col[jj+1] = ind;
          this->mat_.val[jj+1] = val;
        }


  return true;

}

// following R.E.Bank and C.C.Douglas paper
// this = A * B
template <typename ValueType>
void HostMatrixCSR<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  assert(&A != NULL);
  assert(&B != NULL);

  const HostMatrixCSR<ValueType> *cast_mat_A = dynamic_cast<const HostMatrixCSR<ValueType>*> (&A);
  const HostMatrixCSR<ValueType> *cast_mat_B = dynamic_cast<const HostMatrixCSR<ValueType>*> (&B);

  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);
  assert(cast_mat_A->ncol_ == cast_mat_B->nrow_);

  std::vector<int> row_offset; 
  std::vector<int> *new_col = new std::vector<int> [cast_mat_A->nrow_];

  row_offset.resize(cast_mat_A->nrow_+1);

  row_offset[0] = 0;

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int i=0; i<cast_mat_A->nrow_; ++i) {

    // loop over the row
    for (int j=cast_mat_A->mat_.row_offset[i]; j<cast_mat_A->mat_.row_offset[i+1]; ++j) {
      int ii = cast_mat_A->mat_.col[j];
      //      new_col[i].push_back(ii);

      // loop corresponding row
      for (int k=cast_mat_B->mat_.row_offset[ii]; k<cast_mat_B->mat_.row_offset[ii+1]; ++k)
        new_col[i].push_back(cast_mat_B->mat_.col[k]);
      
    }
    
    std::sort( new_col[i].begin(), new_col[i].end() );
    new_col[i].erase( std::unique( new_col[i].begin(), new_col[i].end() ), new_col[i].end() );

    row_offset[i+1] = int(new_col[i].size());

  }

  for (int i=0; i<cast_mat_A->nrow_; ++i)
    row_offset[i+1] += row_offset[i];
    

  this->AllocateCSR(row_offset[cast_mat_A->nrow_], cast_mat_A->nrow_, cast_mat_B->ncol_);

#pragma omp parallel for  
  for (int i=0; i<cast_mat_A->nrow_+1; ++i)
    this->mat_.row_offset[i] = row_offset[i];

#pragma omp parallel for
  for (int i=0; i<cast_mat_A->nrow_; ++i) {
    int jj=0;
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      this->mat_.col[j] = new_col[i][jj];
      ++jj;
    }
  }

  //#pragma omp parallel for
  //      for (unsigned int i=0; i<this->nnz_; ++i)
  //      this->mat_.val[i] = ValueType(1.0);

  delete [] new_col;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::NumericMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  assert(&A != NULL);
  assert(&B != NULL);

  const HostMatrixCSR<ValueType> *cast_mat_A = dynamic_cast<const HostMatrixCSR<ValueType>*> (&A);
  const HostMatrixCSR<ValueType> *cast_mat_B = dynamic_cast<const HostMatrixCSR<ValueType>*> (&B);
  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);
  assert(cast_mat_A->ncol_ == cast_mat_B->nrow_);
  assert(this->nrow_ == cast_mat_A->nrow_);
  assert(this->ncol_ == cast_mat_B->ncol_);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for  
  for (int i=0; i<cast_mat_A->nrow_; ++i) {

    // loop over the row
    for (int j=cast_mat_A->mat_.row_offset[i]; j<cast_mat_A->mat_.row_offset[i+1]; ++j) {
      int ii = cast_mat_A->mat_.col[j];

      // loop corresponding row
      for (int k=cast_mat_B->mat_.row_offset[ii]; k<cast_mat_B->mat_.row_offset[ii+1]; ++k) {

        for (int p=this->mat_.row_offset[i]; p<this->mat_.row_offset[i+1]; ++p) {
          if (cast_mat_B->mat_.col[k] == this->mat_.col[p]) {
            this->mat_.val[p] += cast_mat_B->mat_.val[k] * cast_mat_A->mat_.val[j];
            break;
          }
        }
      }
    }
  }
  
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::SymbolicPower(const int p) {

  assert(p > 1);

  // The optimal values for the firsts values

  if (p == 2)
    this->SymbolicMatMatMult(*this);

  if (p == 3) {
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    tmp.CopyFrom(*this);

    this->SymbolicPower(2);
    this->SymbolicMatMatMult(tmp);
  }

  if (p == 4) {
    this->SymbolicPower(2);
    this->SymbolicPower(2);
  }

  if (p == 5) {
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    tmp.CopyFrom(*this);

    this->SymbolicPower(4);
    this->SymbolicMatMatMult(tmp);
  }

  if (p == 6) {
    this->SymbolicPower(2);
    this->SymbolicPower(3);
  }

  if (p == 7) {
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    tmp.CopyFrom(*this);

    this->SymbolicPower(6);
    this->SymbolicMatMatMult(tmp);
  }

  if (p == 8) {
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    tmp.CopyFrom(*this);

    this->SymbolicPower(6);
    tmp.  SymbolicPower(2);

    this->SymbolicMatMatMult(tmp);
  }

  if (p > 8) {
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    tmp.CopyFrom(*this);
    
    for (int i=0; i<p; ++i)
      this->SymbolicMatMatMult(tmp);
  }
    

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::ILUpFactorizeNumeric(const int p, const BaseMatrix<ValueType> &mat) {

  assert(&mat != NULL);

  const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat);

  assert(cast_mat != NULL);
  assert(cast_mat->nrow_ == this->nrow_);
  assert(cast_mat->ncol_ == this->ncol_);
  assert(this    ->nnz_ > 0);  
  assert(cast_mat->nnz_ > 0);

  int *row_offset  = NULL;
  int *ind_diag    = NULL;
  int *levels      = NULL;
  ValueType *val   = NULL;

  allocate_host(cast_mat->nrow_+1, &row_offset);
  allocate_host(cast_mat->nrow_, &ind_diag);
  allocate_host(cast_mat->nnz_, &levels);
  allocate_host(cast_mat->nnz_, &val);


  const int inf_level = 99999;
  int nnz = 0;

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

  // find diagonals
#pragma omp parallel for
  for (int ai=0; ai<cast_mat->nrow_; ++ai)
    for (int aj=cast_mat->mat_.row_offset[ai]; aj<cast_mat->mat_.row_offset[ai+1]; ++aj)      
      if (ai == cast_mat->mat_.col[aj]) {
        ind_diag[ai] = aj;
        break;
      }

  // init row_offset
#pragma omp parallel for
  for (int i=0; i<cast_mat->nrow_+1; ++i)
    row_offset[i] = 0;

  // init inf levels 
#pragma omp parallel for
  for (int i=0; i<cast_mat->nnz_; ++i)
    levels[i] = inf_level;

#pragma omp parallel for
  for (int i=0; i<cast_mat->nnz_; ++i)
    val[i] = ValueType(0.0);

  // fill levels and values
#pragma omp parallel for
  for (int ai=0; ai<cast_mat->nrow_; ++ai) 
    for (int aj=cast_mat->mat_.row_offset[ai]; aj<cast_mat->mat_.row_offset[ai+1]; ++aj)
      for (int ajj=this->mat_.row_offset[ai]; ajj<this->mat_.row_offset[ai+1]; ++ajj)
        if (cast_mat->mat_.col[aj] == this->mat_.col[ajj]) {
          val[aj] = this->mat_.val[ajj];
          levels[aj] = 0;
          break ; 
        }

  // ai = 1 to N
  for (int ai=1; ai<cast_mat->nrow_; ++ai) {
    
    // ak = 1 to ai-1
    for (int ak=cast_mat->mat_.row_offset[ai]; ai>cast_mat->mat_.col[ak]; ++ak)
      if (levels[ak] <= p) {
      
        val[ak] /= val[ ind_diag[cast_mat->mat_.col[ak]] ];
        
        // aj = ak+1 to N
        for (int aj=ak+1; aj<cast_mat->mat_.row_offset[ai+1]; ++aj) {
          
          ValueType val_kj = ValueType(0.0);
          int level_kj      = inf_level;

          // find a_k,j
          for (int kj=cast_mat->mat_.row_offset[ cast_mat->mat_.col[ak] ];
               kj < cast_mat->mat_.row_offset[ cast_mat->mat_.col[ak]+1 ];
               ++kj)
            if (cast_mat->mat_.col[aj] == cast_mat->mat_.col[kj]) {
              level_kj = levels[kj];
              val_kj   = val[kj];
              break;
            }

          int lev = level_kj + levels[ak] + 1;

          if (levels[aj] > lev)
            levels[aj] = lev ;

          // a_i,j = a_i,j - a_i,k * a_k,j
          val[aj] -= val[ak]*val_kj;
          
        }

      }

    
    for (int ak=cast_mat->mat_.row_offset[ai]; ak<cast_mat->mat_.row_offset[ai+1]; ++ak)
      if (levels[ak] > p) {
        levels[ak] = inf_level ;
        val[ak]    = ValueType(0.0); 
      } else {
        ++row_offset[ai+1];
      }
    
  }

  row_offset[0] = this->mat_.row_offset[0];
  row_offset[1] = this->mat_.row_offset[1];
  
  for (int i=0; i<cast_mat->nrow_; ++i)
    row_offset[i+1] += row_offset[i];

  nnz = row_offset[cast_mat->nrow_];

  this->AllocateCSR(nnz, cast_mat->nrow_, cast_mat->ncol_);

  int jj=0;
  for (int i=0; i<cast_mat->nrow_; ++i)
    for (int j=cast_mat->mat_.row_offset[i]; j<cast_mat->mat_.row_offset[i+1]; ++j) 
           if (levels[j] <= p) {
             this->mat_.col[jj] = cast_mat->mat_.col[j];
             this->mat_.val[jj] = val[j]; 
             ++jj;
           }

  assert(jj==nnz);

#pragma omp parallel for
  for (int i=0; i<this->nrow_+1; ++i)
   this->mat_.row_offset[i] = row_offset[i];          

  free_host(&row_offset);
  free_host(&ind_diag);
  free_host(&levels);    
  free_host(&val);

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                                         const ValueType beta, const bool structure) {

  assert(&mat != NULL);

  const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat);

  assert(cast_mat != NULL);
  assert(cast_mat->nrow_ == this->nrow_);
  assert(cast_mat->ncol_ == this->ncol_);
  assert(this    ->nnz_ > 0);  
  assert(cast_mat->nnz_ > 0);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

  // the structure is sub-set
  if (structure == false) {

    // CSR should be sorted  
#pragma omp parallel for
    for (int ai=0; ai<cast_mat->nrow_; ++ai) {
      
      int first_col = cast_mat->mat_.row_offset[ai];
      
      for (int ajj=this->mat_.row_offset[ai]; ajj<this->mat_.row_offset[ai+1]; ++ajj)
        for (int aj=first_col; aj<cast_mat->mat_.row_offset[ai+1]; ++aj)
          if (cast_mat->mat_.col[aj] == this->mat_.col[ajj]) {
            
            this->mat_.val[ajj] = alpha*this->mat_.val[ajj] + beta*cast_mat->mat_.val[aj];
            ++first_col;
            break ; 
            
          }
    }

  } else {

    std::vector<int> row_offset; 
    std::vector<int> *new_col = new std::vector<int> [this->nrow_];

    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    
    tmp.CopyFrom(*this);

    row_offset.resize(this->nrow_+1);
    
    row_offset[0] = 0;
    
#pragma omp parallel for  
    for (int i=0; i<this->nrow_; ++i) {
      
      for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
        new_col[i].push_back(this->mat_.col[j]);   
      }

      for (int k=cast_mat->mat_.row_offset[i]; k<cast_mat->mat_.row_offset[i+1]; ++k)         
        new_col[i].push_back(cast_mat->mat_.col[k]);                          
      
      std::sort( new_col[i].begin(), new_col[i].end() );
      new_col[i].erase( std::unique( new_col[i].begin(), new_col[i].end() ), new_col[i].end() );
      
      row_offset[i+1] = int(new_col[i].size());
      
    }
    
    for (int i=0; i<this->nrow_; ++i)
      row_offset[i+1] += row_offset[i];
    
    
    this->AllocateCSR(row_offset[this->nrow_], this->nrow_, this->ncol_);

    // copy structure    
#pragma omp parallel for  
    for (int i=0; i<this->nrow_+1; ++i)
      this->mat_.row_offset[i] = row_offset[i];

#pragma omp parallel for
  for (int i=0; i<this->nrow_; ++i) {
    int jj=0;
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      this->mat_.col[j] = new_col[i][jj]; 
      ++jj;
    }
  }
    
  // add values
#pragma omp parallel for
    for (int i=0; i<this->nrow_; ++i) {

      int Aj = tmp.mat_.row_offset[i];
      int Bj = cast_mat->mat_.row_offset[i];

      for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {

        for (int jj=Aj; jj<tmp.mat_.row_offset[i+1]; ++jj)
          if (this->mat_.col[j] == tmp.mat_.col[jj]) { 
            this->mat_.val[j] += alpha*tmp.mat_.val[jj];
            ++Aj;
            break;
          }
        
        for (int jj=Bj; jj<cast_mat->mat_.row_offset[i+1]; ++jj)
          if (this->mat_.col[j] == cast_mat->mat_.col[jj]) {
            this->mat_.val[j] += beta*cast_mat->mat_.val[jj];
            ++Bj;
            break;
          }

      }

    }    
    delete[] new_col;    

  }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::Gershgorin(ValueType &lambda_min,
                                          ValueType &lambda_max) const {


  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

  lambda_min = ValueType(0.0);
  lambda_max = ValueType(0.0);

  // TODO (parallel max, min)
  // #pragma omp parallel for

  for (int ai=0; ai<this->nrow_; ++ai) {

    ValueType sum  = ValueType(0.0);
    ValueType diag = ValueType(0.0);
    
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) 
      if (ai != this->mat_.col[aj]) {
        sum += paralution_abs(this->mat_.val[aj]);
      } else {
        diag = this->mat_.val[aj];
      }

    if (sum + diag > lambda_max)
      lambda_max = sum + diag;

    if (diag - sum < lambda_min)
      lambda_min = diag - sum;

  }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::Scale(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nnz_; ++ai) 
    this->mat_.val[ai] *= alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ScaleDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai == this->mat_.col[aj]) {
        this->mat_.val[aj] *= alpha;
        break;
      }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ScaleOffDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai != this->mat_.col[aj])
        this->mat_.val[aj] *= alpha;

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::AddScalar(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nnz_; ++ai) 
    this->mat_.val[ai] += alpha;

  return true;

}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::AddScalarDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai == this->mat_.col[aj]) {
        this->mat_.val[aj] += alpha;
        break;
      }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::AddScalarOffDiagonal(const ValueType alpha) {

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai != this->mat_.col[aj])
        this->mat_.val[aj] += alpha;

  return true;

}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::DiagonalMatrixMult(const BaseVector<ValueType> &diag) {

  assert(diag.get_size() == this->ncol_);

  const HostVector<ValueType> *cast_diag = dynamic_cast<const HostVector<ValueType>*> (&diag) ; 
  assert(cast_diag!= NULL);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) {
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      this->mat_.val[aj] *= cast_diag->vec_[this->mat_.col[aj]];
  }
    
  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::Compress(const ValueType drop_off) {

  if (this->nnz_ > 0) {
    
    std::vector<int> row_offset; 
    
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    
    tmp.CopyFrom(*this);

    row_offset.resize(this->nrow_+1);
    
    row_offset[0] = 0;

    _set_omp_backend_threads(this->local_backend_, this->nrow_);  
    
#pragma omp parallel for  
    for (int i=0; i<this->nrow_; ++i) {

      row_offset[i+1] = 0;
      
      for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j)
        if (( paralution_abs(this->mat_.val[j]) > drop_off )  ||
            ( this->mat_.col[j] == i))
          row_offset[i+1] += 1;
    }

    
    for (int i=0; i<this->nrow_; ++i)
      row_offset[i+1] += row_offset[i];

    this->AllocateCSR(row_offset[this->nrow_], this->nrow_, this->ncol_);
    
#pragma omp parallel for  
    for (int i=0; i<this->nrow_+1; ++i)
      this->mat_.row_offset[i] = row_offset[i];



#pragma omp parallel for
    for (int i=0; i<this->nrow_; ++i) {

      int jj = this->mat_.row_offset[i];
      
      for (int j=tmp.mat_.row_offset[i]; j<tmp.mat_.row_offset[i+1]; ++j)
       if (( paralution_abs(tmp.mat_.val[j]) > drop_off )  ||
           ( tmp.mat_.col[j] == i)) {
         this->mat_.col[jj] = tmp.mat_.col[j];
         this->mat_.val[jj] = tmp.mat_.val[j]; 
         ++jj;
       }
             
     }
    
  }

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::Transpose(void) {

  if (this->nnz_ > 0) {

    HostMatrixCSR<ValueType> tmp(this->local_backend_);

    tmp.CopyFrom(*this);

    this->Clear();
    this->AllocateCSR(tmp.nnz_, tmp.ncol_, tmp.nrow_);

    for (int i=0; i<this->nnz_; ++i)
      this->mat_.row_offset[ tmp.mat_.col[i]+1 ] += 1;

    for (int i=0; i<this->nrow_; ++i)
      this->mat_.row_offset[i+1] += this->mat_.row_offset[i];

    for (int ai=0; ai<this->ncol_; ++ai)  
      for (int aj=tmp.mat_.row_offset[ai]; aj<tmp.mat_.row_offset[ai+1]; ++aj) {

        const int ind_col = tmp.mat_.col[aj];
        const int ind = this->mat_.row_offset[ ind_col ];

        this->mat_.col[ind] = ai;
        this->mat_.val[ind] = tmp.mat_.val[aj];        

        this->mat_.row_offset[ ind_col ] += 1;

      }
    
    int shift = 0 ;
    for (int i=0; i<this->nrow_; ++i) {
      int tmp = this->mat_.row_offset[i] ;
      this->mat_.row_offset[i] = shift ;
      shift = tmp;
    }
    
    this->mat_.row_offset[this->nrow_] = shift ;
    
    assert(tmp.nnz_ == shift);
  }

  return true;
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::Permute(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);
  assert((permutation.get_size() == this->nrow_) &&
         (permutation.get_size() == this->ncol_));

  if( this->nnz_ > 0 ) {

    const HostVector<int> *cast_perm = dynamic_cast<const HostVector<int>*>(&permutation);
    assert(cast_perm != NULL);

    _set_omp_backend_threads(this->local_backend_, this->nrow_);  
    
	
    //Calculate nnz per row
    int* row_nnz = NULL;
    allocate_host<int>(this->nrow_, &row_nnz);

#pragma omp parallel for  		
    for(int i = 0; i < this->nrow_; ++i) {
      row_nnz[i] = this->mat_.row_offset[i+1] - this->mat_.row_offset[i];
    }

    //Permute vector of nnz per row
    int* perm_row_nnz = NULL;
    allocate_host<int>(this->nrow_, &perm_row_nnz);

#pragma omp parallel for  
    for(int i = 0; i < this->nrow_; ++i) {
      perm_row_nnz[cast_perm->vec_[i]] = row_nnz[i];
    }	
    
    //Calculate new nnz
    int* perm_nnz = NULL;
    allocate_host<int>(this->nrow_+1, &perm_nnz);
    int sum = 0;

    for(int i = 0; i < this->nrow_; ++i) {
      perm_nnz[i] = sum;
      sum += perm_row_nnz[i];
    }
    perm_nnz[this->nrow_] = sum;

    //Permute rows
    int* col = NULL;
    ValueType* val = NULL;
    allocate_host<int>(this->nnz_, &col);
    allocate_host<ValueType>(this->nnz_, &val);

#pragma omp parallel for  
    for(int i = 0; i < this->nrow_; ++i) {

      int permIndex = perm_nnz[cast_perm->vec_[i]];
      int prevIndex = this->mat_.row_offset[i];

      for(int j = 0; j < row_nnz[i]; ++j) {
        col[permIndex+j] = this->mat_.col[prevIndex+j];
        val[permIndex+j] = this->mat_.val[prevIndex+j];
      }

    }
    
    //Permute columns
#pragma omp parallel for
    for(int i = 0; i < this->nrow_; ++i) {

      int row_index = perm_nnz[i];

      for(int j = 0; j < perm_row_nnz[i]; ++j) {
        int k = j-1;
        int aComp = col[row_index+j];
        int comp = cast_perm->vec_[aComp];
        for(;k>=0;--k){

          if(this->mat_.col[row_index+k]>comp) {
            this->mat_.val[row_index+k+1] = this->mat_.val[row_index+k];
            this->mat_.col[row_index+k+1] = this->mat_.col[row_index+k];
          } else
            break;
        }

        this->mat_.val[row_index+k+1] = val[row_index+j];
        this->mat_.col[row_index+k+1] = comp;

      }

    }
    

    free_host<int>(&this->mat_.row_offset);
    this->mat_.row_offset = perm_nnz;
    free_host<int>(&col);
    free_host<ValueType>(&val);
    free_host<int>(&row_nnz);
    free_host<int>(&perm_row_nnz);
    
  }

    return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::CMK(BaseVector<int> *permutation) const {

  assert(this->nnz_ > 0);
  assert(permutation != NULL);

  HostVector<int> *cast_perm = dynamic_cast<HostVector<int>*> (permutation);
  assert(cast_perm != NULL);

  cast_perm->Clear();
  cast_perm->Allocate(this->nrow_);

  int next = 0;
  int head = 0;
  int tmp = 0;
  int test = 1;
  int position = 0;
  int maxdeg = 0;

  int *nd = NULL;
  int *marker = NULL;
  int *levset = NULL;
  int *nextlevset = NULL;

  allocate_host<int>(this->nrow_, &nd);
  allocate_host<int>(this->nrow_, &marker);
  allocate_host<int>(this->nrow_, &levset);
  allocate_host<int>(this->nrow_, &nextlevset);

  int qlength = 1;


  for(int k=0; k<this->nrow_; ++k) {

    marker[k] = 0;
    nd[k] = this->mat_.row_offset[k+1] - this->mat_.row_offset[k]-1;

    if (nd[k] > maxdeg) 
      maxdeg= nd[k];

  }

  head = this->mat_.col[0];
  levset[0] = head;
  cast_perm->vec_[0] = 0;
  ++next;
  marker[head] = 1;

  while(next <this->nrow_) {
     
    position = 0;
    
    for(int h=0; h<qlength; ++h) {

      head = levset[h];

      for(int k=this->mat_.row_offset[head]; k<this->mat_.row_offset[head+1]; ++k) {
        tmp = this->mat_.col[k];

        if ((marker[tmp] == 0) && (tmp != head)) {

          nextlevset[position] = tmp;
          marker[tmp] = 1;
          cast_perm->vec_[tmp] =  next;
          ++next;
          ++position;

        }
      }
    }

    qlength= position;
   
    while (test == 1) {

      test = 0;

      for(int j=position-1; j>0; --j) 
        if(nd[nextlevset[j]] < nd[nextlevset[j-1]]) {
          tmp= nextlevset[j];
          nextlevset[j]= nextlevset[j-1];
          nextlevset[j-1]= tmp;
          test=1;
        }
      
    }
    
    for(int i=0; i<position; ++i)
      levset[i]=nextlevset[i];
    
    if(qlength == 0)
      for(int i=0; i<this->nrow_; ++i) 
        if(marker[i] == 0) {
          levset[0] = i;
          qlength = 1;
          cast_perm->vec_[i] = next;
          marker[i] = 1;
          ++next;
        }


  }

  free_host(&nd);
  free_host(&marker);
  free_host(&levset);
  free_host(&nextlevset);

  return true;

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::RCMK(BaseVector<int> *permutation) const {

  HostVector<int> *cast_perm = dynamic_cast<HostVector<int>*> (permutation);
  assert(cast_perm != NULL);

  cast_perm->Clear();
  cast_perm->Allocate(this->nrow_);


  HostVector<int> tmp_perm(this->local_backend_);

  this->CMK(&tmp_perm);

  for (int i=0; i<this->nrow_; ++i)
    cast_perm->vec_[i] = this->nrow_ - tmp_perm.vec_[i] -1;
  

  return true;


}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::CreateFromMap(const BaseVector<int> &map, const int n, const int m) {

  assert(&map != NULL);

  const HostVector<int> *cast_map = dynamic_cast<const HostVector<int>*>(&map);
  assert(cast_map != NULL);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

  int nnz = map.get_size();

  // nnz = int(map.size());
  assert(nnz == n);

  int *row_nnz = NULL;
  int *row_buffer = NULL;
  allocate_host<int>(m, &row_nnz);
  allocate_host<int>(m+1, &row_buffer);

#pragma omp parallel for  
  for (int i=0; i<m; ++i)
    row_nnz[i] = 0;

  for (int i=0; i<n; ++i) {
    assert(cast_map->vec_[i] < m);
    row_nnz[cast_map->vec_[i]]++;
  }

  this->Clear();
  this->AllocateCSR(nnz, m, n);

  this->mat_.row_offset[0] = 0;
  row_buffer[0] = 0;

  for (int i=0; i<m; ++i) {
    this->mat_.row_offset[i+1] = this->mat_.row_offset[i] + row_nnz[i];
    row_buffer[i+1] = this->mat_.row_offset[i+1];
  }

  for (int i=0; i<nnz; ++i) {
    this->mat_.col[row_buffer[cast_map->vec_[i]]] = i;
    this->mat_.val[i] = ValueType(1.0);
    row_buffer[cast_map->vec_[i]]++;
  }

  assert(this->mat_.row_offset[m] == nnz);
  
  free_host<int>(&row_nnz);
  free_host<int>(&row_buffer);

  return true;

}

// ----------------------------------------------------------
// original function connect(const spmat &A,
//                           float eps_strong)
// ----------------------------------------------------------
// Modified and adopted from AMGCL,
// https://github.com/ddemidov/amgcl
// MIT License
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
template <typename ValueType>
void HostMatrixCSR<ValueType>::AMGConnect(const ValueType eps, BaseVector<int> *connections) const {

  assert(connections != NULL);

  HostVector<int> *cast_conn = dynamic_cast<HostVector<int>*> (connections);
  assert(cast_conn != NULL);

  cast_conn->Clear();
  cast_conn->Allocate(this->nnz_);

  ValueType eps2 = eps * eps;

  HostVector<ValueType> vec_diag(this->local_backend_);
  vec_diag.Allocate(this->nrow_);
  this->ExtractDiagonal(&vec_diag);

#pragma omp parallel for schedule(dynamic, 1024)
  for (int i=0; i<this->nrow_; ++i) {

    ValueType eps_dia_i = eps2 * vec_diag.vec_[i];

    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {

      int       c = this->mat_.col[j];
      ValueType v = this->mat_.val[j];

      cast_conn->vec_[j] = (c != i) && (v * v > eps_dia_i * vec_diag.vec_[c]);

    }

  }

}

// ----------------------------------------------------------
// original function aggregates( const spmat &A,
//                               const std::vector<char> &S )
// ----------------------------------------------------------
// Modified and adopted from AMGCL,
// https://github.com/ddemidov/amgcl
// MIT License
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
template <typename ValueType>
void HostMatrixCSR<ValueType>::AMGAggregate(const BaseVector<int> &connections, BaseVector<int> *aggregates) const {

  assert(aggregates   != NULL);
  assert(&connections != NULL);

  HostVector<int> *cast_agg        = dynamic_cast<HostVector<int>*> (aggregates); 
  const HostVector<int> *cast_conn = dynamic_cast<const HostVector<int>*>(&connections);

  assert(cast_agg != NULL);
  assert(cast_conn != NULL);

  aggregates->Clear();
  aggregates->Allocate(this->nrow_);

  const int undefined = -1;
  const int removed   = -2;

  // Remove nodes without neighbours
  int max_neib = 0;
  for (int i=0; i<this->nrow_; ++i) {
    int j = this->mat_.row_offset[i];
    int e = this->mat_.row_offset[i+1];

    max_neib = std::max(e - j, max_neib);

    int state = removed;
    for (; j<e; ++j) {
      if (cast_conn->vec_[j]) {
        state = undefined;
        break;
      }
    }

    cast_agg->vec_[i] = state;

  }

  std::vector<int> neib;
  neib.reserve(max_neib);

  int last_g = -1;

  // Perform plain aggregation
  for(int i=0; i<this->nrow_; ++i) {

    if (cast_agg->vec_[i] != undefined) continue;

      // The point is not adjacent to a core of any previous aggregate:
      // so its a seed of a new aggregate.
      cast_agg->vec_[i] = ++last_g;

      neib.clear();

      // Include its neighbors as well.
      for (int j=this->mat_.row_offset[i], e=this->mat_.row_offset[i+1]; j<e; ++j) {
        int c = this->mat_.col[j];
        if (cast_conn->vec_[j] && cast_agg->vec_[c] != removed) {
          cast_agg->vec_[c] = last_g;
          neib.push_back(c);
        }
      }

      // Temporarily mark undefined points adjacent to the new aggregate as
      // belonging to the aggregate. If nobody claims them later, they will
      // stay here.
      for (typename std::vector<int>::const_iterator nb = neib.begin(); nb != neib.end(); ++nb)
        for (int j=this->mat_.row_offset[*nb], e=this->mat_.row_offset[*nb+1]; j<e; ++j)
          if (cast_conn->vec_[j] && cast_agg->vec_[this->mat_.col[j]] == undefined)
            cast_agg->vec_[this->mat_.col[j]] = last_g;

    }

}

// ----------------------------------------------------------
// original function interp(const sparse::matrix<value_t,
//                          index_t> &A, const params &prm)
// ----------------------------------------------------------
// Modified and adopted from AMGCL,
// https://github.com/ddemidov/amgcl
// MIT License
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
template <typename ValueType>
void HostMatrixCSR<ValueType>::AMGSmoothedAggregation(const ValueType relax,
                                                      const BaseVector<int> &aggregates,
                                                      const BaseVector<int> &connections,
                                                            BaseMatrix<ValueType> *prolong,
                                                            BaseMatrix<ValueType> *restrict) const {

  assert(&aggregates  != NULL);
  assert(&connections != NULL);
  assert(prolong      != NULL);
  assert(restrict     != NULL);

  const HostVector<int> *cast_agg         = dynamic_cast<const HostVector<int>*>(&aggregates);
  const HostVector<int> *cast_conn        = dynamic_cast<const HostVector<int>*>(&connections);  
  HostMatrixCSR<ValueType> *cast_prolong  = dynamic_cast<HostMatrixCSR<ValueType>*>(prolong);
  HostMatrixCSR<ValueType> *cast_restrict = dynamic_cast<HostMatrixCSR<ValueType>*>(restrict);

  assert(cast_agg      != NULL);
  assert(cast_conn     != NULL);
  assert(cast_prolong  != NULL);
  assert(cast_restrict != NULL);

  // Allocate
  cast_prolong->Clear();
  cast_prolong->AllocateCSR(this->nnz_, this->nrow_, this->ncol_);

  int ncol = 0;

  for (int i=0; i<cast_agg->get_size(); ++i)
    if (cast_agg->vec_[i] > ncol)
      ncol = cast_agg->vec_[i];

  ++ncol;

#pragma omp parallel
  {
    std::vector<int> marker(ncol, -1);

#ifdef _OPENMP
    int nt  = omp_get_num_threads();
    int tid = omp_get_thread_num();

    int chunk_size  = (this->nrow_ + nt - 1) / nt;
    int chunk_start = tid * chunk_size;
    int chunk_end   = std::min(this->nrow_, chunk_start + chunk_size);
#else

    int chunk_start = 0;
    int chunk_end   = this->nrow_;
#endif

    // Count number of entries in prolong
    for (int i=chunk_start; i<chunk_end; ++i) {
      for (int j=this->mat_.row_offset[i], e=this->mat_.row_offset[i+1]; j<e; ++j) {
        int c = this->mat_.col[j];

        if (c != i && !cast_conn->vec_[j]) continue;

        int g = cast_agg->vec_[c];

        if (g >= 0 && marker[g] != i) {
          marker[g] = i;
          ++cast_prolong->mat_.row_offset[i+1];
        }
      }
    }

    std::fill(marker.begin(), marker.end(), -1);

#pragma omp barrier
#pragma omp single
    {
      int *row_offset = NULL;
      allocate_host(cast_prolong->nrow_+1, &row_offset);

      int *col = NULL;
      ValueType *val = NULL;

      int nnz = 0;
      int nrow = cast_prolong->nrow_;

      row_offset[0] = 0;
      for (int i=1; i<nrow+1; ++i)
        row_offset[i] = cast_prolong->mat_.row_offset[i] + row_offset[i-1];

      nnz = row_offset[nrow];

      allocate_host(nnz, &col);
      allocate_host(nnz, &val);

      cast_prolong->Clear();
      cast_prolong->SetDataPtrCSR(&row_offset, &col, &val, nnz, nrow, ncol);
    }

    // Fill the interpolation matrix.
    for (int i=chunk_start; i<chunk_end; ++i) {

      // Diagonal of the filtered matrix is original matrix diagonal minus
      // its weak connections.
      ValueType dia = ValueType(0.0);
      for (int j=this->mat_.row_offset[i], e=this->mat_.row_offset[i+1]; j<e; ++j) {
        if (this->mat_.col[j] == i)
          dia += this->mat_.val[j];
        else if (!cast_conn->vec_[j])
          dia -= this->mat_.val[j];
      }

      dia = ValueType(1.0) / dia;

      int row_begin = cast_prolong->mat_.row_offset[i];
      int row_end   = row_begin;

      for (int j = this->mat_.row_offset[i], e=this->mat_.row_offset[i+1]; j<e; ++j) {
        int c = this->mat_.col[j];

        // Skip weak couplings, ...
        if (c != i && !cast_conn->vec_[j]) continue;

        // ... and the ones not in any aggregate.
        int g = cast_agg->vec_[c];
        if (g < 0) continue;

        ValueType v = (c == i) ? ValueType(1.0) - relax : -relax * dia * this->mat_.val[j];

        if (marker[g] < row_begin) {
          marker[g] = row_end;
          cast_prolong->mat_.col[row_end] = g;
          cast_prolong->mat_.val[row_end] = v;
          ++row_end;
        } else {
          cast_prolong->mat_.val[marker[g]] += v;
        }

      }

    }

  }

#pragma omp parallel for      
  for (int i=0; i<cast_prolong->nrow_; ++i)
    for (int j=cast_prolong->mat_.row_offset[i]; j<cast_prolong->mat_.row_offset[i+1]; ++j)
      for (int jj=cast_prolong->mat_.row_offset[i]; jj<cast_prolong->mat_.row_offset[i+1]-1; ++jj)
        if (cast_prolong->mat_.col[jj] > cast_prolong->mat_.col[jj+1]) {
          //swap elements

          int ind = cast_prolong->mat_.col[jj];
          ValueType val = cast_prolong->mat_.val[jj];

          cast_prolong->mat_.col[jj] = cast_prolong->mat_.col[jj+1];
          cast_prolong->mat_.val[jj] = cast_prolong->mat_.val[jj+1];          

          cast_prolong->mat_.col[jj+1] = ind;
          cast_prolong->mat_.val[jj+1] = val;
        }

  cast_restrict->CopyFrom(*cast_prolong);
  cast_restrict->Transpose();

}

// ----------------------------------------------------------
// original function interp(const sparse::matrix<value_t,
//                          index_t> &A, const params &prm)
// ----------------------------------------------------------
// Modified and adopted from AMGCL,
// https://github.com/ddemidov/amgcl
// MIT License
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
template <typename ValueType>
void HostMatrixCSR<ValueType>::AMGAggregation(const BaseVector<int> &aggregates,
                                                    BaseMatrix<ValueType> *prolong,
                                                    BaseMatrix<ValueType> *restrict) const {

  assert(&aggregates  != NULL);
  assert(prolong      != NULL);
  assert(restrict     != NULL);

  const HostVector<int> *cast_agg         = dynamic_cast<const HostVector<int>*>(&aggregates);
  HostMatrixCSR<ValueType> *cast_prolong  = dynamic_cast<HostMatrixCSR<ValueType>*>(prolong);
  HostMatrixCSR<ValueType> *cast_restrict = dynamic_cast<HostMatrixCSR<ValueType>*>(restrict);

  assert(cast_agg      != NULL);
  assert(cast_prolong  != NULL);
  assert(cast_restrict != NULL);

  int ncol = 0;

  for (int i=0; i<cast_agg->get_size(); ++i)
    if (cast_agg->vec_[i] > ncol)
      ncol = cast_agg->vec_[i];

  ++ncol;

  int *row_offset = NULL;
  allocate_host(this->nrow_+1, &row_offset);

  int *col = NULL;
  ValueType *val = NULL;

  row_offset[0] = 0;
  for (int i=0; i<this->nrow_; ++i) {

    if (cast_agg->vec_[i] >= 0)
      row_offset[i+1] = row_offset[i] + 1;
    else
      row_offset[i+1] = row_offset[i];

  }

  allocate_host(row_offset[this->nrow_], &col);
  allocate_host(row_offset[this->nrow_], &val);

  for (int i=0, j=0; i<this->nrow_; ++i) {

    if (cast_agg->vec_[i] >= 0) {
      col[j] = cast_agg->vec_[i];
      val[j] = 1.0;
      ++j;
    }

  }

  cast_prolong->Clear();
  cast_prolong->SetDataPtrCSR(&row_offset, &col, &val, row_offset[this->nrow_], this->nrow_, ncol);

  cast_restrict->CopyFrom(*cast_prolong);
  cast_restrict->Transpose();

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::FSAI(const int power, const BaseMatrix<ValueType> *pattern) {

  // Extract lower triangular matrix of A
  HostMatrixCSR<ValueType> L(this->local_backend_);

  const HostMatrixCSR<ValueType> *cast_pattern = NULL;
  if (pattern != NULL) {
    cast_pattern = dynamic_cast<const HostMatrixCSR<ValueType>*>(pattern);
    assert(cast_pattern != NULL);

    cast_pattern->ExtractLDiagonal(&L);
  } else if (power > 1) {
    HostMatrixCSR<ValueType> structure(this->local_backend_);
    structure.CopyFrom(*this);
    structure.SymbolicPower(power);
    structure.ExtractLDiagonal(&L);
  } else {
    this->ExtractLDiagonal(&L);
  }

  int nnz = L.nnz_;
  int nrow = L.nrow_;
  int ncol = L.ncol_;
  int *row_offset = NULL;
  int *col = NULL;
  ValueType *val = NULL;

  L.LeaveDataPtrCSR(&row_offset, &col, &val);

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) {

    // entries of ai-th row
    int nnz_row = row_offset[ai+1] - row_offset[ai];

    if (nnz_row == 1) {

      int aj = this->mat_.row_offset[ai];
      if (this->mat_.col[aj] == ai)
        val[row_offset[ai]] = ValueType(1.0) / this->mat_.val[aj];

    } else {

      HostVector<ValueType> mk(this->local_backend_);
      HostVector<ValueType> ek(this->local_backend_);
      HostMatrixDENSE<ValueType> Asub(this->local_backend_);

      Asub.AllocateDENSE(nnz_row, nnz_row);
      mk.Allocate(nnz_row);
      ek.Allocate(nnz_row);

      ek.vec_[nnz_row-1] = 1.;

      // create submatrix taking only the lower tridiagonal part into account
      for (int k=0; k<nnz_row; ++k) {

        int row_begin = this->mat_.row_offset[col[row_offset[ai]+k]];
        int row_end   = this->mat_.row_offset[col[row_offset[ai]+k]+1];
        int j = 0;

        for (int aj=row_begin; aj<row_end; ++aj)
          for (; j<nnz_row; ++j) {

            int Asub_col = col[row_offset[ai]+j];

            if (this->mat_.col[aj] < Asub_col)
              break;

            if (this->mat_.col[aj] == Asub_col)
              Asub.mat_.val[DENSE_IND(k, j, nnz_row, nnz_row)] = this->mat_.val[aj];

          }

      }

      // solve Asub * mk = ek with LU factorization
      Asub.LUFactorize();
      Asub.LUSolve(ek, &mk);

      // update the preconditioner matrix with mk
      for (int aj=row_offset[ai], k=0; aj<row_offset[ai+1]; ++aj, ++k)
        val[aj] = mk.vec_[k];

      // free data structures
      mk.Clear();
      ek.Clear();
      Asub.Clear();

    }

  }

  // scaling
  for (int ai=0; ai<nrow; ++ai) {
    ValueType fac = sqrt(ValueType(1.0) / paralution_abs(val[row_offset[ai+1]-1]));
    for (int aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      val[aj] *= fac;
  }

  this->Clear();
  this->SetDataPtrCSR(&row_offset, &col, &val, nnz, nrow, ncol);

  return true;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::SPAI(void) {

  int nrow = this->nrow_;
  int nnz  = this->nnz_;

  ValueType *val = NULL;
  allocate_host(nnz, &val);

  HostMatrixCSR<ValueType> T(this->local_backend_);
  T.CopyFrom(*this);
  this->Transpose();

  // Loop over each row to get J indexing vector
#pragma omp parallel for
  for (int i=0; i<nrow; ++i) {

    int *J = NULL;
    int Jsize = this->mat_.row_offset[i+1]-this->mat_.row_offset[i];
    allocate_host(Jsize, &J);
    std::vector<int> I;

    // Setup J = {j | m(j) != 0}
    for (int j=this->mat_.row_offset[i], idx = 0; j<this->mat_.row_offset[i+1]; ++j, ++idx)
      J[idx] = this->mat_.col[j];

    // Setup I = {i | row A(i,J) != 0}
    for (int idx=0; idx<Jsize; ++idx)
      for (int j=this->mat_.row_offset[J[idx]]; j<this->mat_.row_offset[J[idx]+1]; ++j)
        if (std::find(I.begin(), I.end(), this->mat_.col[j]) == I.end())
          I.push_back(this->mat_.col[j]);

    // Build dense matrix
    HostMatrixDENSE<ValueType> Asub(this->local_backend_);
    Asub.AllocateDENSE(int(I.size()), Jsize);

    for (int k=0; k<Asub.nrow_; ++k) {
      for (int aj = T.mat_.row_offset[I[k]]; aj < T.mat_.row_offset[I[k]+1]; ++aj) {
        for (int j=0; j<Jsize; ++j) {

          if (T.mat_.col[aj] == J[j])
            Asub.mat_.val[DENSE_IND(k,j,Asub.nrow_,Asub.ncol_)] = T.mat_.val[aj];

        }
      }
    }

    // QR Decomposition of dense submatrix Ak
    Asub.QRDecompose();

    // Solve least squares
    HostVector<ValueType> ek(this->local_backend_);
    HostVector<ValueType> mk(this->local_backend_);

    ek.Allocate(Asub.nrow_);
    mk.Allocate(Asub.ncol_);

    for (int j=0; j<ek.get_size(); ++j)
      if (I[j] == i)
        ek.vec_[j] = 1.0;

    Asub.QRSolve(ek, &mk);

    // Write m_k into preconditioner matrix
    for (int j=0; j<Jsize; ++j)
      val[this->mat_.row_offset[i]+j] = mk.vec_[j];

    // Clear index vectors
    I.clear();
    ek.Clear();
    mk.Clear();
    Asub.Clear();
    free_host(&J);

  }

  // Only reset value array since we keep the sparsity pattern of A
  free_host(&this->mat_.val);
  this->mat_.val = val;

  this->Transpose();

}

//
// Note
// the code works with CSC matrix
// so the ii and jj input parameters are exchanged
//
// Adapted from http://user.it.uu.se/~stefane/freeware.html
//
template <typename ValueType>
void HostMatrixCSR<ValueType>::Assemble(const int *jj, const int *ii, const ValueType *sr,
                                        const int len, const int m, const int n,
                                        int **pp_assembly_rank,
                                        int **pp_assembly_irank,
                                        int **pp_assembly_loop_start,
                                        int **pp_assembly_loop_end,
                                        int &nThreads) {
  
  LOG_DEBUG(this, "HostMatrixCSR::Assemble()",
            "begin");

  assert(ii != NULL);
  assert(jj != NULL);
  assert(len > 0);

  assert(*pp_assembly_rank == NULL);
  assert(*pp_assembly_irank == NULL);
  assert(*pp_assembly_loop_start == NULL);
  assert(*pp_assembly_loop_end == NULL);
  assert(nThreads == 0);

  this->Clear();

  _set_omp_backend_threads(this->local_backend_, -1);  
  nThreads = omp_get_max_threads();
  
  int M=m;
  int N=n;

  if ((M == 0) || (N == 0)) {
    
    // not nice
    // but otherwise makes problems with gcc/omp/1 thread
    if (nThreads > 1) {
      
#pragma omp parallel for
      for (int i=0; i<len; ++i) {
        
        assert(ii[i] >= 0);
        assert(jj[i] >= 0);
        
        int ival = ii[i]+1;
        int jval = jj[i]+1;
        
        if ((ival > M) || (jval > N)) {
#pragma omp critical
          {
            if (ival > M)
              M = ival;
            if (jval > N)
              N = jval;
          }
        }
      }
      
    } else {
      
      for (int i=0; i<len; ++i) {
        
        assert(ii[i] >= 0);
        assert(jj[i] >= 0);
        
        int ival = ii[i]+1;
        int jval = jj[i]+1;
        
        if ((ival > M) || (jval > N)) {
          if (ival > M)
            M = ival;
          if (jval > N)
            N = jval;
        }

      }
    }
    
  } // M, N == 0

  _set_omp_backend_threads(this->local_backend_, M);  
  nThreads = omp_get_max_threads();


  LOG_DEBUG(this, "HostMatrixCSR::Assemble()",
            "M=" << M
            << " N=" << N
            << " threads=" << this->local_backend_.OpenMP_threads);

  // serial version
  if (this->local_backend_.OpenMP_threads == 1) {
  
  LOG_DEBUG(this, "HostMatrixCSR::Assemble()",
            "Serial version");

    int *jcS = NULL;  
    int *jrS = NULL; 
    int *hcol = NULL;
    
    allocate_host(M+1, &jrS);
    allocate_host(N+1, &jcS);
    allocate_host(len, pp_assembly_irank);
    allocate_host(len, pp_assembly_rank);
    allocate_host(N, &hcol);

    set_to_zero_host(M+1, jrS);
    set_to_zero_host(N+1, jcS);
    set_to_zero_host(len, *pp_assembly_irank);
    set_to_zero_host(len, *pp_assembly_rank);
    set_to_zero_host(N, hcol);

    for (int i = 0; i < len; i++) 
      jrS[ii[i]+1]++;
    
    for (int r = 2; r <= M; r++) 
      jrS[r] += jrS[r-1];
    
    for (int i = 0; i < len; i++)
      (*pp_assembly_rank)[jrS[ii[i]]++] = i;
    
    for (int row = 1,i = 0; row <= M; row++)
      for ( ; i < jrS[row-1]; i++) {
        
        const int ixijs = (*pp_assembly_rank)[i]; 
        const int col = jj[ixijs]; 
        
        if (hcol[col] < row) {
          hcol[col] = row; 
          jcS[col+1]++;    
        }
        
        (*pp_assembly_irank)[ixijs] = jcS[col+1]-1;
      }

    
    free_host(&hcol);
    free_host(&jrS);
    
    for (int c = 2; c <= N; c++) 
      jcS[c] += jcS[c-1];
    
    for (int i = 0; i < len; i++) 
      (*pp_assembly_irank)[i] += jcS[jj[i]];
    
    int nnz = jcS[N];
  
    int *irS = NULL;
    ValueType *prS = NULL;
    allocate_host(nnz, &irS);
    allocate_host(nnz, &prS);

    set_to_zero_host(nnz, irS);
    set_to_zero_host(nnz, prS);
    
    if ( sr != NULL) {

      for (int i = 0; i < len; i++) {
        irS[(*pp_assembly_irank)[i]] = ii[i];
        prS[(*pp_assembly_irank)[i]] += sr[i];
      }

    } else {

      for (int i = 0; i < len; i++) 
        irS[(*pp_assembly_irank)[i]] = ii[i];

    }

    allocate_host(1, pp_assembly_loop_end);
    (*pp_assembly_loop_end)[0] = len;

    this->nrow_ = M;
    this->ncol_ = N;
    this->nnz_  = nnz;
    
    this->mat_.row_offset = jcS;
    this->mat_.col = irS;
    this->mat_.val = prS;
    
  } else {
    // parallel version

  LOG_DEBUG(this, "HostMatrixCSR::Assemble()",
            "Parallel version");

    int **jcS;
    int **jrS;

    allocate_host(len, pp_assembly_rank);
    allocate_host(len, pp_assembly_irank);

    set_to_zero_host(len, *pp_assembly_rank);
    set_to_zero_host(len, *pp_assembly_irank);
    
    jcS = (int **) malloc((nThreads+1)*sizeof(int*));
    jrS = (int **) malloc((nThreads+1)*sizeof(int*));
    
    for (int k = 0; k <= nThreads; k++) {
      jcS[k] = NULL;
      allocate_host(N+1, &jcS[k]);
      set_to_zero_host(N+1, jcS[k]);
      
      jrS[k] = NULL;
      allocate_host(M+1, &jrS[k]);
      set_to_zero_host(M+1, jrS[k]);
    }
    
#pragma omp parallel
    {
      const int myId = omp_get_thread_num();
      const int istart = len*myId/nThreads;
      const int iend = len*(myId+1)/nThreads;
      for (int i = istart; i < iend; i++) {
        jrS[myId+1][ii[i]]++;
      }
      
#pragma omp barrier
      
#pragma omp for
      for (int r = 1; r <= M; r++)
        for (int k = 1; k < nThreads; k++)
          jrS[k+1][r-1] += jrS[k][r-1];
            
#pragma omp single
      for (int r = 1; r <= M; r++)
        jrS[0][r+1-1] += jrS[0][r-1]+jrS[nThreads][r-1];      
      
#pragma omp for
      for (int r = 1; r <= M; r++)
        for (int k = 1; k < nThreads; k++)
          jrS[k][r-1] += jrS[0][r-1];        
    } // end parallel

#pragma omp parallel
    {
      const int myId = omp_get_thread_num();
      const int istart = len*myId/nThreads;
      const int iend = len*(myId+1)/nThreads;
      
      for (int i = istart; i < iend; i++)
        (*pp_assembly_rank)[jrS[myId][ii[i]]++] = i;
      
    } // end parallel

#pragma omp parallel
    {
      int *hcol = NULL;
#pragma omp critical
      allocate_host(N, &hcol);
      set_to_zero_host(N, hcol);
      
      const int myId = omp_get_thread_num();
      const int rstart = 1+M*myId/nThreads;
      const int rend = M*(myId+1)/nThreads;
      int istart;
      if (rstart == 1)
        istart = 0;
      else {
        istart = jrS[nThreads-1][rstart-2];
      }
      
      for (int row = rstart,i = istart; row <= rend; row++)
        
        for ( ; i < jrS[nThreads-1][row-1]; i++) {
          const int ixijs = (*pp_assembly_rank)[i]; 
          const int col = jj[ixijs]+1;
          
          if (hcol[col-1] < row) {
            hcol[col-1] = row;  
            jcS[myId+1][col]++; 
          }
          
          (*pp_assembly_irank)[i] = jcS[myId+1][col]-1;
        }
#pragma omp critical
      free_host(&hcol);
      
#pragma omp barrier
      
#pragma omp for
      for (int c = 1; c <= N; c++)
        for (int k = 1; k < nThreads; k++)
          jcS[k+1][c] += jcS[k][c];
        
      
#pragma omp single
      {
        for (int c = 1; c <= N; c++) {
          jcS[0][c] += jcS[0][c-1]+jcS[nThreads][c];
        }
        jcS[0]--;
      }
      
#pragma omp for
      for (int c = 1; c <= N; c++)
        for (int k = 1; k < nThreads; k++)
          jcS[k][c] += jcS[0][c];
            
      if (rend >= 1)
        for (int i = istart; i < jrS[nThreads-1][rend-1]; i++)
          (*pp_assembly_irank)[i] += jcS[myId][jj[(*pp_assembly_rank)[i]]+1];
              
    } // end parallel
    
    
    jcS[0]++;
    
    int nnz = jcS[0][N];       
    
    int *irS = NULL;
    ValueType *prS = NULL;
    allocate_host(nnz, &irS);
    allocate_host(nnz, &prS);

    set_to_zero_host(nnz, irS);
    set_to_zero_host(nnz, prS);
    
    int *jrS_ = jrS[nThreads-1];

    allocate_host(nThreads, pp_assembly_loop_start);
    allocate_host(nThreads, pp_assembly_loop_end);
    
    for (int i=0; i<nThreads; ++i) {
      const int rstart = 1+M*i/nThreads;
      const int rend = M*(i+1)/nThreads;

      if (rstart == 1)
        (*pp_assembly_loop_start)[i] = 0;
      else
        (*pp_assembly_loop_start)[i] = jrS_[rstart-1-1];
      
      (*pp_assembly_loop_end)[i] = jrS_[rend-1];        
    }
    
#pragma omp parallel
    {
      const int myId = omp_get_thread_num();
      const int rend = M*(myId+1)/nThreads;
      
      if (rend >= 1)
        for (int i = (*pp_assembly_loop_start)[myId]; i < (*pp_assembly_loop_end)[myId]; i++) 
          irS[(*pp_assembly_irank)[i]] = ii[(*pp_assembly_rank)[i]];
    }


    if ( sr != NULL) {

#pragma omp parallel
      {
        const int myId = omp_get_thread_num();
        const int rend = M*(myId+1)/nThreads;
        
        if (rend >= 1)
          for (int i = (*pp_assembly_loop_start)[myId]; i < (*pp_assembly_loop_end)[myId]; i++) 
            prS[(*pp_assembly_irank)[i]] += sr[(*pp_assembly_rank)[i]];
      }

    }
    
    for (int k = 0; k <= nThreads; k++)
      free_host(&jrS[k]);
    
    for (int k = 1; k <= nThreads; k++)
      free_host(&jcS[k]);
    
    free(jrS);
    
    this->nrow_ = M;
    this->ncol_ = N;
    this->nnz_  = nnz;
    
    this->mat_.row_offset = jcS[0];
    this->mat_.col = irS;
    this->mat_.val = prS;

    jcS[0] = NULL;
    free(jcS);
    
  }

  LOG_DEBUG(this, "HostMatrixCSR::Assemble()",
            "end" <<
            " nnz=" << this->nnz_);

 
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::AssembleUpdate(const ValueType *sr,
                                              const int *assembly_rank,
                                              const int *assembly_irank,
                                              const int *assembly_loop_start,
                                              const int *assembly_loop_end,
                                              const int nThreads) {


  assert(sr != NULL);
  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_  > 0);

  assert(assembly_rank != NULL);
  assert(assembly_irank != NULL);
  assert(assembly_loop_end != NULL);

  _set_omp_backend_threads(this->local_backend_, this->nrow_);  

  if (this->local_backend_.OpenMP_threads == 1) {
    // serial

  LOG_DEBUG(this, "HostMatrixCSR::AssembleUpdate()",
            "Serial threads=" << this->local_backend_.OpenMP_threads);

    for (int i = 0; i < assembly_loop_end[0]; i++)
      this->mat_.val[assembly_irank[i]] += sr[i];


  } else {
    // parallel 

  LOG_DEBUG(this, "HostMatrixCSR::AssembleUpdate()",
            "Parallel threads=" << this->local_backend_.OpenMP_threads);
  
  assert(assembly_loop_start != NULL);
  assert(nThreads == omp_get_max_threads());

#pragma omp parallel
    {
      const int myId = omp_get_thread_num();
      const int rend = this->nrow_*(myId+1)/nThreads;
      
      if (rend >= 1)
        for (int i = assembly_loop_start[myId]; i < assembly_loop_end[myId]; i++) 
          this->mat_.val[assembly_irank[i]] += sr[assembly_rank[i]];
    }
  }

  LOG_DEBUG(this, "HostMatrixCSR::AssembleUpdate()",
            "end" <<
            " nnz=" << this->nnz_);


}

template class HostMatrixCSR<double>;
template class HostMatrixCSR<float>;

}

