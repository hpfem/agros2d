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

#include "host_matrix_csr.hpp"
#include "host_matrix_coo.hpp"
#include "host_matrix_mcsr.hpp"
#include "host_matrix_bcsr.hpp"
#include "host_matrix_dia.hpp"
#include "host_matrix_ell.hpp"
#include "host_matrix_hyb.hpp"
#include "host_matrix_dense.hpp"
#include "host_conversion.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "host_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <vector>

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
HostMatrixCSR<ValueType>::HostMatrixCSR() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}
template <typename ValueType>
HostMatrixCSR<ValueType>::HostMatrixCSR(const Paralution_Backend_Descriptor local_backend) {

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

  this->Clear();

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

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

  if (this->get_nnz() > 0)
    set_to_zero_host(this->get_nnz(), mat_.val);  

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::info(void) const {

  LOG_INFO("HostMatrixCSR<ValueType>, OpenMP threads: " << this->local_backend_.OpenMP_threads);

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::AllocateCSR(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
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
void HostMatrixCSR<ValueType>::CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val) {

  assert(this->get_nnz() > 0);
  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for      
  for (int i=0; i<this->get_nrow()+1; ++i)
    this->mat_.row_offset[i] = row_offsets[i];
  
#pragma omp parallel for      
  for (int j=0; j<this->get_nnz(); ++j)
    this->mat_.col[j] = col[j];
  
#pragma omp parallel for      
  for (int j=0; j<this->get_nnz(); ++j)
    this->mat_.val[j] = val[j];
  
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::CopyToCSR(int *row_offsets, int *col, ValueType *val) const {

  assert(this->get_nnz() > 0);
  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for      
  for (int i=0; i<this->get_nrow()+1; ++i)
    row_offsets[i] = this->mat_.row_offset[i]; 
  
#pragma omp parallel for      
  for (int j=0; j<this->get_nnz(); ++j)
    col[j] = this->mat_.col[j];
  
#pragma omp parallel for      
  for (int j=0; j<this->get_nnz(); ++j)
    val[j] = this->mat_.val[j];

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    if (this->get_nnz() == 0)
      this->AllocateCSR(mat.get_nnz(), mat.get_nrow(), mat.get_ncol() );

    assert((this->get_nnz()  == mat.get_nnz())  &&
           (this->get_nrow() == mat.get_nrow()) &&
           (this->get_ncol() == mat.get_ncol()) );
        
    if (this->get_nnz() > 0) {

      omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for      
      for (int i=0; i<this->get_nrow()+1; ++i)
        this->mat_.row_offset[i] = cast_mat->mat_.row_offset[i] ;

#pragma omp parallel for
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.col[j] = cast_mat->mat_.col[j];

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
void HostMatrixCSR<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

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
               cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), 
               cast_mat->mat_, &this->mat_);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
    this->nnz_  = cast_mat->get_nnz();

    return true;

  }

  if (const HostMatrixDENSE<ValueType> *cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&mat)) {

    this->Clear();
    int nnz = 0;

    dense_to_csr(this->local_backend_.OpenMP_threads,
                 cast_mat->get_nrow(), cast_mat->get_ncol(), 
                 cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
    this->nnz_  = nnz;

    return true;

  }


  if (const HostMatrixDIA<ValueType> *cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&mat)) {

    this->Clear();
    int nnz;

    dia_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), 
               cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
    this->nnz_  = nnz ;

    return true;

  }

  if (const HostMatrixELL<ValueType> *cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&mat)) {

    this->Clear();
    int nnz;

    ell_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), 
               cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
    this->nnz_  = nnz ;

    return true;

  }

  if (const HostMatrixMCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixMCSR<ValueType>*> (&mat)) {

    this->Clear();
    mcsr_to_csr(this->local_backend_.OpenMP_threads,
                cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), 
                cast_mat->mat_, &this->mat_);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
    this->nnz_  = cast_mat->get_nnz();

    return true;

  }

  if (const HostMatrixHYB<ValueType> *cast_mat = dynamic_cast<const HostMatrixHYB<ValueType>*> (&mat)) {

    this->Clear();
    
    int nnz;
    hyb_to_csr(this->local_backend_.OpenMP_threads,
               cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), 
               cast_mat->get_ell_nnz(), cast_mat->get_coo_nnz(),
               cast_mat->mat_, 
               &this->mat_, &nnz);

    this->nrow_ = cast_mat->get_nrow();
    this->ncol_ = cast_mat->get_ncol();
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
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  char transp='N'; 
  int nrow = this->get_nrow() ;

  mkl_cspblas_dcsrgemv(&transp, &nrow, 
                       this->mat_.val, this->mat_.row_offset, this->mat_.col,
                       cast_in->vec_, cast_out->vec_);
    
}

template <>
void HostMatrixCSR<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {

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

  mkl_cspblas_scsrgemv(&transp, &nrow,
                       this->mat_.val, this->mat_.row_offset, this->mat_.col,
                       cast_in->vec_, cast_out->vec_);
    
}


#else

template <typename ValueType>
void HostMatrixCSR<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      cast_out->vec_[ai] += this->mat_.val[aj] * cast_in->vec_[ this->mat_.col[aj] ];
  }
    
}

#endif

template <typename ValueType>
void HostMatrixCSR<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
      for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
        cast_out->vec_[ai] += scalar*this->mat_.val[aj] * cast_in->vec_[ this->mat_.col[aj] ];
  }
    
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ExtractDiagonal(BaseVector<ValueType> *vec_diag) const {

  assert(vec_diag != NULL);
  assert(vec_diag->get_size() == this->get_nrow());

  HostVector<ValueType> *cast_vec_diag  = dynamic_cast<HostVector<ValueType>*> (vec_diag) ; 

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai)
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
  assert(vec_inv_diag->get_size() == this->get_nrow());

  HostVector<ValueType> *cast_vec_inv_diag  = dynamic_cast<HostVector<ValueType>*> (vec_inv_diag) ; 

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai)
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

  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);

  HostMatrixCSR<ValueType> *cast_mat  = dynamic_cast<HostMatrixCSR<ValueType>*> (mat) ; 
  assert(cast_mat != NULL);

  int mat_nnz = 0;

  // use omp in local_matrix (higher level)

  //  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

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

#ifdef SUPPORT_MKL

template <>
bool HostMatrixCSR<double>::LUSolve(const BaseVector<double> &in, BaseVector<double> *out) const {
  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in) ; 
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  char transp = 'N'; 
  char matdescra[6];
  double one = double(1.0);
  int nrow = this->get_nrow();
  
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
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  // Solve L
  for (int ai=0; ai<this->get_nrow(); ++ai) {

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
  int diag_aj = this->get_nnz()-1;

  // Solve U
  for (int ai=this->get_nrow()-1; ai>=0; --ai) {

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
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);
  int diag_aj = 0 ;

  // Solve L
  for (int ai=0; ai<this->get_nrow(); ++ai) {

    cast_out->vec_[ai] = cast_in->vec_[ai];

    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) {

      if (this->mat_.col[aj] < ai) {
        // under the diagonal
        cast_out->vec_[ai] -= this->mat_.val[aj] * cast_out->vec_[ this->mat_.col[aj] ];
      } 

      if (this->mat_.col[aj] == ai) {
        diag_aj = aj;
      }

      cast_out->vec_[ai] /= this->mat_.val[diag_aj];

    }

  }


  // last elements should be the diagonal one (last)
  diag_aj = this->get_nnz()-1;

  // Solve U
  for (int ai=this->get_nrow()-1; ai>=0; --ai) {

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
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  int diag_aj = 0;

  // Solve L
  for (int ai=0; ai<this->get_nrow(); ++ai) {
    
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
  assert(in.  get_size() == this->get_ncol());
  assert(out->get_size() == this->get_nrow());

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in) ; 
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out) ; 

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  // last elements should the diagonal one (last)
  int diag_aj = this->get_nnz()-1;

  // Solve U
  for (int ai=this->get_nrow()-1; ai>=0; --ai) {

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

  int nrow = this->get_nrow();

  //TODO define ipar?
  int ipar[128];
  double dpar[128];
  int ierr;

  // go to one-based array
  for (int i=0; i<nrow+1; ++i)
    ++this->mat_.row_offset[i];

  // go to one-based array
  for (int i=0; i<this->get_nnz(); ++i)
    ++this->mat_.col[i];

  dcsrilu0(&nrow, 
           this->mat_.val, this->mat_.row_offset, this->mat_.col,
           this->mat_.val,
           ipar, dpar,
           &ierr);
					 
							 
  // go to one-based array
  for (int i=0; i<nrow+1; ++i)
    --this->mat_.row_offset[i];

  // go to one-based array
  for (int i=0; i<this->get_nnz(); ++i)
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

  assert(this->get_nrow() == this->get_ncol());
  assert(this->get_nnz() > 0);

  // pointer of upper part of each row
  int *diag_offset = NULL;
  int *nnz_entries = NULL;

  allocate_host(this->get_nrow(), &diag_offset);
  allocate_host(this->get_nrow(), &nnz_entries);

  for (int i=0; i<this->get_nrow(); ++i)
    nnz_entries[i] = 0;

  // ai = 0 to N loop over all rows
  for (int ai=0; ai<this->get_nrow(); ++ai) {

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

        if (this->mat_.val[diag_offset[this->mat_.col[j]]] != 0.0) {

          // multiplication factor
          this->mat_.val[j] = this->mat_.val[j] / this->mat_.val[diag_offset[this->mat_.col[j]]];

          // loop over upper offset pointer and do linear combination for nnz entry
          for (int k=diag_offset[this->mat_.col[j]]+1; k<this->mat_.row_offset[this->mat_.col[j]+1]; ++k)

            // if nnz at this position do linear combination
            if (nnz_entries[this->mat_.col[k]])
              this->mat_.val[nnz_entries[this->mat_.col[k]]] -= this->mat_.val[j] * this->mat_.val[k];
        }

      } else
        break;
    }

    // set diagonal pointer to diagonal element
    diag_offset[ai] = j;

    // clear nnz entries
    for (j=row_start; j<row_end; j++)
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

  assert(this->get_nrow() == this->get_ncol());
  assert(this->get_nnz() > 0);

  int nrow = this->get_nrow();
  int ncol = this->get_ncol();

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
  int nnzA = this->get_nnz();
  int alloc_size = nnzA*1.5;
  int *col = (int*) malloc(alloc_size*sizeof(int));
  ValueType *val = (ValueType*) malloc(alloc_size*sizeof(ValueType));

  // initialize row_offset
  row_offset[0] = 0;
  int nnz = 0;

  // loop over all rows
  for (int ai=0; ai<this->get_nrow(); ++ai) {

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

      row_norm += fabs(this->mat_.val[aj]);
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
            if (fabs(fillin) >= threshold) {

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
      alloc_size += nnzA*1.5;
      col = (int*) realloc(col, alloc_size*sizeof(ValueType));
      val = (ValueType*) realloc(val, alloc_size*sizeof(ValueType));
    }

  }

  col = (int*) realloc(col, nnz*sizeof(ValueType));
  val = (ValueType*) realloc(val, nnz*sizeof(ValueType));

  free_host(&w);
  free_host(&diag_offset);
  free_host(&nnz_entries);
  free(nnz_pos);

  this->Clear();
  this->SetDataPtrCSR(&row_offset, &col, &val, nnz, nrow, ncol);

  return true;

}

// TODO
template <typename ValueType>
bool HostMatrixCSR<ValueType>::IC0Factorize(void) {

  FATAL_ERROR(__FILE__, __LINE__);

  return false;
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
  allocate_host(this->get_nrow(), &color);

  memset(color, 0, sizeof(int)*this->get_nrow());
  num_colors = 0;
  std::vector<bool> row_col;

  for (int ai=0; ai<this->get_nrow(); ++ai) {
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

  for (int i=0; i<this->get_nrow(); ++i) 
    ++(*size_colors)[color[i]-1];

  int total=0;
  for (int i=1; i<num_colors; ++i) {

    total += (*size_colors)[i-1];
    offsets_color[i] = total; 
    //   LOG_INFO("offsets = " << total);

  }

  cast_perm->Allocate(this->get_nrow());

  for (int i=0; i<permutation->get_size(); ++i) {

    cast_perm->vec_[ i ] = offsets_color[ color[i]-1 ] ;
    ++offsets_color[color[i]-1];

  }

  free_host(&color);
  free_host(&offsets_color);

  return true;

}

template <typename ValueType>
void HostMatrixCSR<ValueType>::MaximalIndependentSet(int &size,
                                                     BaseVector<int> *permutation) const {

  assert(permutation != NULL);
  assert( (permutation->get_size() == this->get_nrow()) &&
          (permutation->get_size() == this->get_ncol()) );

  HostVector<int> *cast_perm = dynamic_cast<HostVector<int>*> (permutation) ; 
  assert(cast_perm != NULL);


  int *mis = NULL;
  allocate_host(cast_perm->get_size(), &mis);
  memset(mis, 0, sizeof(int)*cast_perm->get_size());

  size = 0 ;

  for (int ai=0; ai<this->get_nrow(); ++ai) {

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

  int pos = 0;
  for (int ai=0; ai<this->get_nrow(); ++ai) {

    if (mis[ai] == 1) {

      cast_perm->vec_[ai] = pos;
      ++pos;

    } else {

      cast_perm->vec_[ai] = size + ai - pos;

    }

  }

  free_host(&mis);
}

// following R.E.Bank and C.C.Douglas paper
template <typename ValueType>
void HostMatrixCSR<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &src) {

  assert(this->get_ncol() == src.get_nrow());

  const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&src);
  assert(cast_mat != NULL);

  std::vector<int> row_offset; 
  std::vector<int> *new_col = new std::vector<int> [this->get_nrow()];
  
  row_offset.resize(this->get_nrow()+1);

  row_offset[0] = 0;

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for  
  for (int i=0; i<this->get_nrow(); ++i) {

    // loop over the row
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      int ii = this->mat_.col[j];

      // loop corresponding row
      for (int k=cast_mat->mat_.row_offset[ii]; k<cast_mat->mat_.row_offset[ii+1]; ++k)         
        new_col[i].push_back(cast_mat->mat_.col[k]);                
      
    }
    
    std::sort( new_col[i].begin(), new_col[i].end() );
    new_col[i].erase( std::unique( new_col[i].begin(), new_col[i].end() ), new_col[i].end() );

    row_offset[i+1] = new_col[i].size();

  }

  for (int i=0; i<this->get_nrow(); ++i)
    row_offset[i+1] += row_offset[i];
    

  this->AllocateCSR(row_offset[this->get_nrow()], this->get_nrow(), this->get_ncol());

#pragma omp parallel for  
  for (int i=0; i<this->get_nrow()+1; ++i)
    this->mat_.row_offset[i] = row_offset[i];


#pragma omp parallel for
  for (int i=0; i<this->get_nrow(); ++i) {
    int jj=0;
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      this->mat_.col[j] = new_col[i][jj]; 
      ++jj;
    }
  }
    
  //#pragma omp parallel for
  //  for (unsigned int i=0; i<this->get_nnz(); ++i)
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
// ----------------------------------------------------------
template <typename ValueType>
bool HostMatrixCSR<ValueType>::MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  assert((this != &A) && (this != &B));
  assert(A.get_ncol() == B.get_nrow());

  const HostMatrixCSR<ValueType> *cast_mat_A = dynamic_cast<const HostMatrixCSR<ValueType>*> (&A);
  const HostMatrixCSR<ValueType> *cast_mat_B = dynamic_cast<const HostMatrixCSR<ValueType>*> (&B);
  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);

  int n = cast_mat_A->get_nrow();
  int m = cast_mat_B->get_ncol();

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

  this->SetDataPtrCSR(&row_offset, &col, &val, row_offset[n], cast_mat_A->get_nrow(), cast_mat_B->get_ncol());

/*
  this->SymbolicMatMatMult(A, B);
  this->NumericMatMatMult (A, B);
*/
  return true;
}

// following R.E.Bank and C.C.Douglas paper
// this = A * B
template <typename ValueType>
void HostMatrixCSR<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  assert(A.get_ncol() == B.get_nrow());

  const HostMatrixCSR<ValueType> *cast_mat_A = dynamic_cast<const HostMatrixCSR<ValueType>*> (&A);
  const HostMatrixCSR<ValueType> *cast_mat_B = dynamic_cast<const HostMatrixCSR<ValueType>*> (&B);
  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);

  std::vector<int> row_offset; 
  std::vector<int> *new_col = new std::vector<int> [cast_mat_A->get_nrow()];

  row_offset.resize(cast_mat_A->get_nrow()+1);

  row_offset[0] = 0;

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int i=0; i<cast_mat_A->get_nrow(); ++i) {

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

    row_offset[i+1] = new_col[i].size();

  }

  for (int i=0; i<cast_mat_A->get_nrow(); ++i)
    row_offset[i+1] += row_offset[i];
    

  this->AllocateCSR(row_offset[cast_mat_A->get_nrow()], cast_mat_A->get_nrow(), cast_mat_B->get_ncol());

#pragma omp parallel for  
  for (int i=0; i<cast_mat_A->get_nrow()+1; ++i)
    this->mat_.row_offset[i] = row_offset[i];

#pragma omp parallel for
  for (int i=0; i<cast_mat_A->get_nrow(); ++i) {
    int jj=0;
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      this->mat_.col[j] = new_col[i][jj];
      ++jj;
    }
  }

  //#pragma omp parallel for
  //      for (unsigned int i=0; i<this->get_nnz(); ++i)
  //      this->mat_.val[i] = ValueType(1.0);

  delete [] new_col;
}

template <typename ValueType>
void HostMatrixCSR<ValueType>::NumericMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  assert(A.get_ncol() == B.get_nrow());
  assert(this->get_nrow() == A.get_nrow());
  assert(this->get_ncol() == B.get_ncol());

  const HostMatrixCSR<ValueType> *cast_mat_A = dynamic_cast<const HostMatrixCSR<ValueType>*> (&A);
  const HostMatrixCSR<ValueType> *cast_mat_B = dynamic_cast<const HostMatrixCSR<ValueType>*> (&B);
  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for  
  for (int i=0; i<cast_mat_A->get_nrow(); ++i) {

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

  const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat);
  assert(cast_mat != NULL);

  assert(cast_mat->get_nrow() == this->get_nrow());
  assert(cast_mat->get_ncol() == this->get_ncol());
  assert(this    ->get_nnz() > 0);  
  assert(cast_mat->get_nnz() > 0);

  int *row_offset  = NULL;
  int *ind_diag    = NULL;
  int *levels      = NULL;
  ValueType *val   = NULL;

  allocate_host(cast_mat->get_nrow()+1, &row_offset);
  allocate_host(cast_mat->get_nrow(), &ind_diag);
  allocate_host(cast_mat->get_nnz(), &levels);
  allocate_host(cast_mat->get_nnz(), &val);


  const int inf_level = 99999;
  int nnz = 0;

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

  // find diagonals
#pragma omp parallel for
  for (int ai=0; ai<cast_mat->get_nrow(); ++ai)
    for (int aj=cast_mat->mat_.row_offset[ai]; aj<cast_mat->mat_.row_offset[ai+1]; ++aj)      
      if (ai == cast_mat->mat_.col[aj]) {
        ind_diag[ai] = aj;
        break;
      }

  // init row_offset
#pragma omp parallel for
  for (int i=0; i<cast_mat->get_nrow()+1; ++i)
    row_offset[i] = 0;

  // init inf levels 
#pragma omp parallel for
  for (int i=0; i<cast_mat->get_nnz(); ++i)
    levels[i] = inf_level;

#pragma omp parallel for
  for (int i=0; i<cast_mat->get_nnz(); ++i)
    val[i] = ValueType(0.0);

  // fill levels and values
#pragma omp parallel for
  for (int ai=0; ai<cast_mat->get_nrow(); ++ai) 
    for (int aj=cast_mat->mat_.row_offset[ai]; aj<cast_mat->mat_.row_offset[ai+1]; ++aj)
      for (int ajj=this->mat_.row_offset[ai]; ajj<this->mat_.row_offset[ai+1]; ++ajj)
        if (cast_mat->mat_.col[aj] == this->mat_.col[ajj]) {
          val[aj] = this->mat_.val[ajj];
          levels[aj] = 0;
          break ; 
        }

  // ai = 1 to N
  for (int ai=1; ai<cast_mat->get_nrow(); ++ai) {
    
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
  
  for (int i=0; i<cast_mat->get_nrow(); ++i)
    row_offset[i+1] += row_offset[i];

  nnz = row_offset[cast_mat->get_nrow()];

  this->AllocateCSR(nnz, cast_mat->get_nrow(), cast_mat->get_ncol());

  int jj=0;
  for (int i=0; i<cast_mat->get_nrow(); ++i)
    for (int j=cast_mat->mat_.row_offset[i]; j<cast_mat->mat_.row_offset[i+1]; ++j) 
           if (levels[j] <= p) {
             this->mat_.col[jj] = cast_mat->mat_.col[j];
             this->mat_.val[jj] = val[j]; 
             ++jj;
           }

  assert(jj==nnz);

#pragma omp parallel for
  for (int i=0; i<this->get_nrow()+1; ++i)
   this->mat_.row_offset[i] = row_offset[i];          

  free_host(&row_offset);
  free_host(&ind_diag);
  free_host(&levels);    
  free_host(&val);

}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                                         const ValueType beta, const bool structure) {

  const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat);
  assert(cast_mat != NULL);

  assert(cast_mat->get_nrow() == this->get_nrow());
  assert(cast_mat->get_ncol() == this->get_ncol());
  assert(this    ->get_nnz() > 0);  
  assert(cast_mat->get_nnz() > 0);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

  // the structure is sub-set
  if (structure == false) {

    // CSR should be sorted  
#pragma omp parallel for
    for (int ai=0; ai<cast_mat->get_nrow(); ++ai) {
      
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
    std::vector<int> *new_col = new std::vector<int> [this->get_nrow()];

    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    
    tmp.CopyFrom(*this);

    row_offset.resize(this->get_nrow()+1);
    
    row_offset[0] = 0;
    
#pragma omp parallel for  
    for (int i=0; i<this->get_nrow(); ++i) {
      
      for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
        new_col[i].push_back(this->mat_.col[j]);   
      }

      for (int k=cast_mat->mat_.row_offset[i]; k<cast_mat->mat_.row_offset[i+1]; ++k)         
        new_col[i].push_back(cast_mat->mat_.col[k]);                          
      
      std::sort( new_col[i].begin(), new_col[i].end() );
      new_col[i].erase( std::unique( new_col[i].begin(), new_col[i].end() ), new_col[i].end() );
      
      row_offset[i+1] = new_col[i].size();
      
    }
    
    for (int i=0; i<this->get_nrow(); ++i)
      row_offset[i+1] += row_offset[i];
    
    
    this->AllocateCSR(row_offset[this->get_nrow()], this->get_nrow(), this->get_ncol());

    // copy structure    
#pragma omp parallel for  
    for (int i=0; i<this->get_nrow()+1; ++i)
      this->mat_.row_offset[i] = row_offset[i];

#pragma omp parallel for
  for (int i=0; i<this->get_nrow(); ++i) {
    int jj=0;
    for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j) {
      this->mat_.col[j] = new_col[i][jj]; 
      ++jj;
    }
  }
    
  // add values
#pragma omp parallel for
    for (int i=0; i<this->get_nrow(); ++i) {

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


  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

  lambda_min = ValueType(0.0);
  lambda_max = ValueType(0.0);

  // TODO (parallel max, min)
  // #pragma omp parallel for

  for (int ai=0; ai<this->get_nrow(); ++ai) {

    ValueType sum  = ValueType(0.0);
    ValueType diag = ValueType(0.0);
    
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj) 
      if (ai != this->mat_.col[aj]) {
        sum += fabs(this->mat_.val[aj]);
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

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nnz(); ++ai) 
    this->mat_.val[ai] *= alpha;

  return true;
}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::ScaleDiagonal(const ValueType alpha) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai == this->mat_.col[aj]) {
        this->mat_.val[aj] *= alpha;
        break;
      }

  return true;
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::ScaleOffDiagonal(const ValueType alpha) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai != this->mat_.col[aj])
        this->mat_.val[aj] *= alpha;

  return true;    
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::AddScalar(const ValueType alpha) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nnz(); ++ai) 
    this->mat_.val[ai] += alpha;

  return true;
}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::AddScalarDiagonal(const ValueType alpha) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai == this->mat_.col[aj]) {
        this->mat_.val[aj] += alpha;
        break;
      }

  return true;
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::AddScalarOffDiagonal(const ValueType alpha) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai) 
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      if (ai != this->mat_.col[aj])
        this->mat_.val[aj] += alpha;

  return true;
}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::DiagMatMult(const BaseVector<ValueType> &diag) {

  assert(diag.get_size() == this->get_ncol());

  const HostVector<ValueType> *cast_diag = dynamic_cast<const HostVector<ValueType>*> (&diag) ; 
  assert(cast_diag!= NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int ai=0; ai<this->get_nrow(); ++ai) {
    for (int aj=this->mat_.row_offset[ai]; aj<this->mat_.row_offset[ai+1]; ++aj)
      this->mat_.val[aj] *= cast_diag->vec_[this->mat_.col[aj]];
  }
    
  return true;
}


template <typename ValueType>
bool HostMatrixCSR<ValueType>::Compress(const ValueType drop_off) {

  assert(drop_off >= ValueType(0.0));

  if (this->get_nnz() > 0) {
    
    std::vector<int> row_offset; 
    
    HostMatrixCSR<ValueType> tmp(this->local_backend_);
    
    tmp.CopyFrom(*this);

    row_offset.resize(this->get_nrow()+1);
    
    row_offset[0] = 0;

    omp_set_num_threads(this->local_backend_.OpenMP_threads);  
    
#pragma omp parallel for  
    for (int i=0; i<this->get_nrow(); ++i) {

      row_offset[i+1] = 0;
      
      for (int j=this->mat_.row_offset[i]; j<this->mat_.row_offset[i+1]; ++j)
        if (( fabs(this->mat_.val[j]) > drop_off )  ||
            ( this->mat_.col[j] == i))
          row_offset[i+1] += 1;
    }

    
    for (int i=0; i<this->get_nrow(); ++i)
      row_offset[i+1] += row_offset[i];

    this->AllocateCSR(row_offset[this->get_nrow()], this->get_nrow(), this->get_ncol());
    
#pragma omp parallel for  
    for (int i=0; i<this->get_nrow()+1; ++i)
      this->mat_.row_offset[i] = row_offset[i];



#pragma omp parallel for
    for (int i=0; i<this->get_nrow(); ++i) {

      int jj = this->mat_.row_offset[i];
      
      for (int j=tmp.mat_.row_offset[i]; j<tmp.mat_.row_offset[i+1]; ++j)
       if (( fabs(tmp.mat_.val[j]) > drop_off )  ||
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

  if (this->get_nnz() > 0) {

    HostMatrixCSR<ValueType> tmp(this->local_backend_);

    tmp.CopyFrom(*this);

//TODO only re-allocate if nrow != ncol
    this->Clear();
    this->AllocateCSR(tmp.get_nnz(), tmp.get_ncol(), tmp.get_nrow());

    for (int i=0; i<this->get_nnz(); ++i)
      this->mat_.row_offset[ tmp.mat_.col[i]+1 ] += 1;

    for (int i=0; i<this->get_nrow(); ++i)
      this->mat_.row_offset[i+1] += this->mat_.row_offset[i];

    for (int ai=0; ai<this->get_ncol(); ++ai)  
      for (int aj=tmp.mat_.row_offset[ai]; aj<tmp.mat_.row_offset[ai+1]; ++aj) {

        const int ind_col = tmp.mat_.col[aj];
        const int ind = this->mat_.row_offset[ ind_col ];

        this->mat_.col[ind] = ai;
        this->mat_.val[ind] = tmp.mat_.val[aj];        

        this->mat_.row_offset[ ind_col ] += 1;

      }
    
    int shift = 0 ;
    for (int i=0; i<this->get_nrow(); ++i) {
      int tmp = this->mat_.row_offset[i] ;
      this->mat_.row_offset[i] = shift ;
      shift = tmp;
    }
    
    this->mat_.row_offset[this->get_nrow()] = shift ;
    
    assert(tmp.get_nnz() == shift);
  }

  return true;
}

template <typename ValueType>
bool HostMatrixCSR<ValueType>::Permute(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);
  assert((permutation.get_size() == this->get_nrow()) &&
         (permutation.get_size() == this->get_ncol()));

  if( this->get_nnz() > 0 ) {

    const HostVector<int> *cast_perm = dynamic_cast<const HostVector<int>*>(&permutation);
    assert(cast_perm != NULL);

    omp_set_num_threads(this->local_backend_.OpenMP_threads);  
    
	
    //Calculate nnz per row
    int* row_nnz = NULL;
    allocate_host<int>(this->get_nrow(), &row_nnz);

#pragma omp parallel for  		
    for(int i = 0; i < this->get_nrow(); ++i) {
      row_nnz[i] = this->mat_.row_offset[i+1] - this->mat_.row_offset[i];
    }

    //Permute vector of nnz per row
    int* perm_row_nnz = NULL;
    allocate_host<int>(this->get_nrow(), &perm_row_nnz);

#pragma omp parallel for  
    for(int i = 0; i < this->get_nrow(); ++i) {
      perm_row_nnz[cast_perm->vec_[i]] = row_nnz[i];
    }	
    
    //Calculate new nnz
    int* perm_nnz = NULL;
    allocate_host<int>(this->get_nrow()+1, &perm_nnz);
    int sum = 0;

    for(int i = 0; i < this->get_nrow(); ++i) {
      perm_nnz[i] = sum;
      sum += perm_row_nnz[i];
    }
    perm_nnz[this->get_nrow()] = sum;

    //Permute rows
    int* col = NULL;
    ValueType* val = NULL;
    allocate_host<int>(this->get_nnz(), &col);
    allocate_host<ValueType>(this->get_nnz(), &val);

#pragma omp parallel for  
    for(int i = 0; i < this->get_nrow(); ++i) {

      int permIndex = perm_nnz[cast_perm->vec_[i]];
      int prevIndex = this->mat_.row_offset[i];

      for(int j = 0; j < row_nnz[i]; ++j) {
        col[permIndex+j] = this->mat_.col[prevIndex+j];
        val[permIndex+j] = this->mat_.val[prevIndex+j];
      }

    }
    
    //Permute columns
#pragma omp parallel for
    for(int i = 0; i < this->get_nrow(); ++i) {

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
bool HostMatrixCSR<ValueType>::CreateFromMap(const BaseVector<int> &map, const int n, const int m) {

  assert(&map != NULL);

  const HostVector<int> *cast_map = dynamic_cast<const HostVector<int>*>(&map);
  assert(cast_map != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

  int nnz = map.get_size();

  // nnz = map.size()
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
  cast_conn->Allocate(this->get_nnz());

  ValueType eps2 = eps * eps;

  HostVector<ValueType> vec_diag(this->local_backend_);
  vec_diag.Allocate(this->get_nrow());
  this->ExtractDiagonal(&vec_diag);

#pragma omp parallel for schedule(dynamic, 1024)
  for (int i=0; i<this->get_nrow(); ++i) {

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
  aggregates->Allocate(this->get_nrow());

  const int undefined = -1;
  const int removed   = -2;

  // Remove nodes without neughbours
  int max_neib = 0;
  for (int i=0; i<this->get_nrow(); ++i) {
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
  for(int i=0; i<this->get_nrow(); ++i) {

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
  cast_prolong->AllocateCSR(this->get_nnz(), this->get_nrow(), this->get_ncol());

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

    int chunk_size  = (this->get_nrow() + nt - 1) / nt;
    int chunk_start = tid * chunk_size;
    int chunk_end   = std::min(this->get_nrow(), chunk_start + chunk_size);
#else

    int chunk_start = 0;
    int chunk_end   = this->get_nrow();
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
      allocate_host(cast_prolong->get_nrow()+1, &row_offset);

      int *col = NULL;
      ValueType *val = NULL;

      int nnz = 0;
      int nrow = cast_prolong->get_nrow();

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
      ValueType dia = 0.0;
      for (int j=this->mat_.row_offset[i], e=this->mat_.row_offset[i+1]; j<e; ++j) {
        if (this->mat_.col[j] == i)
          dia += this->mat_.val[j];
        else if (!cast_conn->vec_[j])
          dia -= this->mat_.val[j];
      }

      dia = 1. / dia;

      int row_begin = cast_prolong->mat_.row_offset[i];
      int row_end   = row_begin;

      for (int j = this->mat_.row_offset[i], e=this->mat_.row_offset[i+1]; j<e; ++j) {
        int c = this->mat_.col[j];

        // Skip weak couplings, ...
        if (c != i && !cast_conn->vec_[j]) continue;

        // ... and the ones not in any aggregate.
        int g = cast_agg->vec_[c];
        if (g < 0) continue;

        ValueType v = (c == i) ? 1. - relax : -relax * dia * this->mat_.val[j];

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
  for (int i=0; i<cast_prolong->get_nrow(); ++i)
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
  allocate_host(this->get_nrow()+1, &row_offset);

  int *col = NULL;
  ValueType *val = NULL;

  row_offset[0] = 0;
  for (int i=0; i<this->get_nrow(); ++i) {

    if (cast_agg->vec_[i] >= 0)
      row_offset[i+1] = row_offset[i] + 1;
    else
      row_offset[i+1] = row_offset[i];

  }

  allocate_host(row_offset[this->get_nrow()], &col);
  allocate_host(row_offset[this->get_nrow()], &val);

  for (int i=0, j=0; i<this->get_nrow(); ++i) {

    if (cast_agg->vec_[i] >= 0) {
      col[j] = cast_agg->vec_[i];
      val[j] = 1.0;
      ++j;
    }

  }

  cast_prolong->Clear();
  cast_prolong->SetDataPtrCSR(&row_offset, &col, &val, row_offset[this->get_nrow()], this->get_nrow(), ncol);

  cast_restrict->CopyFrom(*cast_prolong);
  cast_restrict->Transpose();

}


template class HostMatrixCSR<double>;
template class HostMatrixCSR<float>;

}
