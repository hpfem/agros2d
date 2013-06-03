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

#include "local_matrix.hpp"
#include "local_vector.hpp"
#include "base_vector.hpp"
#include "base_matrix.hpp"
#include "host/host_matrix_csr.hpp"
#include "host/host_matrix_coo.hpp"
#include "backend_manager.hpp"
#include "../utils/log.hpp"

#include <assert.h>
#include <algorithm>
#include <sstream>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif


namespace paralution {

template <typename ValueType>
LocalMatrix<ValueType>::LocalMatrix() {
  
  this->object_name_ = "";
  this->local_backend_ = _Backend_Descriptor;
  
  // Create empty matrix on the host
  // CSR is the default format
  this->matrix_host_= new HostMatrixCSR<ValueType>(this->local_backend_);

  this->matrix_accel_= NULL;
  this->matrix_ = this->matrix_host_ ;
 
}

template <typename ValueType>
LocalMatrix<ValueType>::~LocalMatrix() {

  this->Clear();
  delete this->matrix_;

}

template <typename ValueType>
void LocalMatrix<ValueType>::Clear(void) {

  this->matrix_->Clear();

}

template <typename ValueType>
void LocalMatrix<ValueType>::Zeros(void) {

  this->matrix_->Zeros();

}


template <typename ValueType>
void LocalMatrix<ValueType>::AllocateCSR(const std::string name, const int nnz, const int nrow, const int ncol) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToCSR();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

    // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);    
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
  
  this->matrix_->AllocateCSR(nnz, nrow, ncol);

}

template <typename ValueType>
void LocalMatrix<ValueType>::AllocateCOO(const std::string name, const int nnz, const int nrow, const int ncol) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToCOO();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

    // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
 
  this->matrix_->AllocateCOO(nnz, nrow, ncol);

}

template <typename ValueType>
void LocalMatrix<ValueType>::AllocateDIA(const std::string name, const int nnz, const int nrow, const int ncol, const int ndiag) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToDIA();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

    // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
 
  this->matrix_->AllocateDIA(nnz, nrow, ncol, ndiag);

}

template <typename ValueType>
void LocalMatrix<ValueType>::AllocateMCSR(const std::string name, const int nnz, const int nrow, const int ncol) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToMCSR();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

    // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);    
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
  
  this->matrix_->AllocateMCSR(nnz, nrow, ncol);

}

template <typename ValueType>
void LocalMatrix<ValueType>::AllocateELL(const std::string name, const int nnz, const int nrow, const int ncol, const int max_row) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToELL();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

    // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);    
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
  
  this->matrix_->AllocateELL(nnz, nrow, ncol, max_row);

}

template <typename ValueType>
void LocalMatrix<ValueType>::SetDataPtrCOO(int **row, int **col, ValueType **val,
                                           std::string name, 
                                           const int nnz, const int nrow, const int ncol) {

  assert(*row != NULL);
  assert(*col != NULL);
  assert(*val != NULL);
  assert(nnz > 0);
  assert(nrow > 0);
  assert(ncol > 0);

  this->Clear();

  this->object_name_ = name;

  this->MoveToHost();
  this->ConvertToCOO();
  
  this->matrix_->SetDataPtrCOO(row, col, val,
                               nnz, nrow, ncol);

}

template <typename ValueType>
void LocalMatrix<ValueType>::SetDataPtrCSR(int **row_offset, int **col, ValueType **val,
                                           std::string name, 
                                           const int nnz, const int nrow, const int ncol) {

  assert(*row_offset != NULL);
  assert(*col != NULL);
  assert(*val != NULL);
  assert(nnz > 0);
  assert(nrow > 0);
  assert(ncol > 0);

  this->Clear();

  this->object_name_ = name;

  this->MoveToHost();
  this->ConvertToCSR();
  
  this->matrix_->SetDataPtrCSR(row_offset, col, val,
                               nnz, nrow, ncol);

}

template <typename ValueType>
void LocalMatrix<ValueType>::AllocateHYB(const std::string name, const int ell_nnz, const int coo_nnz, const int ell_max_row,
                                         const int nrow, const int ncol) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToHYB();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

    // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);    
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
  
  this->matrix_->AllocateHYB(ell_nnz, coo_nnz, ell_max_row, nrow, ncol);

}


template <typename ValueType>
void LocalMatrix<ValueType>::AllocateDENSE(const std::string name, const int nrow, const int ncol) {

  this->Clear();
  this->object_name_ = name;
  this->ConvertToDENSE();

  Paralution_Backend_Descriptor backend = this->local_backend_;
  unsigned int mat = this->get_format();

  // init host matrix
  if (this->matrix_ == this->matrix_host_) {

    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      mat);    
    this->matrix_ = this->matrix_host_;

  } else { 
    // init accel matrix
    assert(this->matrix_ == this->matrix_accel_);

    delete this->matrix_accel_;
    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           mat);
    this->matrix_ = this->matrix_accel_;

  }
  
  this->matrix_->AllocateDENSE(nrow, ncol);

}


template <typename ValueType>
void LocalMatrix<ValueType>::CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val) {

  // Only host can read
  assert(this->matrix_ == this->matrix_host_);

  const unsigned int mat_format = this->get_format();
  this->ConvertToCSR();
  this->matrix_host_->CopyFromCSR(row_offsets, col, val);
  this->ConvertTo(mat_format);

  this->object_name_ = "Imported from CSR matrix";
}

template <typename ValueType>
void LocalMatrix<ValueType>::CopyToCSR(int *row_offsets, int *col, ValueType *val) const {

  // Only host can write
  assert(this->matrix_ == this->matrix_host_);

  if (this->get_format() == CSR) {

    this->matrix_host_->CopyToCSR(row_offsets, col, val);

  } else {

    HostMatrixCSR<ValueType> csr_mat(this->local_backend_);
    csr_mat.ConvertFrom(*this->matrix_host_);
    csr_mat.CopyToCSR(row_offsets, col, val);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::CopyFromCOO(const int *row, const int *col, const ValueType *val) {

  // Only host can read
  assert(this->matrix_ == this->matrix_host_);

  const unsigned int mat_format = this->get_format();
  this->ConvertToCOO();
  this->matrix_host_->CopyFromCOO(row, col, val);
  this->ConvertTo(mat_format);

  this->object_name_ = "Imported from COO matrix";
}


template <typename ValueType>
void LocalMatrix<ValueType>::ReadFileMTX(const std::string filename) {

  // Only COO host can read

  this->Clear();

  bool on_host = this->is_host();
  if (on_host == false)
    this->MoveToHost();

  const unsigned int mat_format = this->get_format();
  this->ConvertToCOO();
  this->matrix_host_->ReadFileMTX(filename);
  this->ConvertTo(mat_format);

  this->object_name_ = filename; 

  if (on_host == false)
    this->MoveToAccelerator();

}

template <typename ValueType>
void LocalMatrix<ValueType>::WriteFileMTX(const std::string filename) const {

  // only COO on host can write to file

  if ((this->get_format() == COO) && (this->is_host() == true)) {

    this->matrix_host_->WriteFileMTX(filename);

  } else {

    LocalMatrix<ValueType> coo_mat;
    coo_mat.CloneFrom(*this);
    coo_mat.MoveToHost();
    coo_mat.ConvertToCOO();
    coo_mat.WriteFileMTX(filename);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::CopyFrom(const LocalMatrix<ValueType> &src) {

  assert(this != &src);

  this->matrix_->CopyFrom(*src.matrix_);

}

template <typename ValueType>
void LocalMatrix<ValueType>::CloneFrom(const LocalMatrix<ValueType> &src) {

  assert(this != &src);

  this->object_name_    = "Cloned from (";
  this->object_name_   += src.object_name_ + ")";
  this->local_backend_ = src.local_backend_; 

  Paralution_Backend_Descriptor backend = this->local_backend_;

  if (src.matrix_ == src.matrix_host_) {

    // host
    delete this->matrix_host_;
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(backend,
                                                                      src.get_format());
    this->matrix_ = this->matrix_host_;

  } else {
    // accel

    delete this->matrix_accel_;

    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(backend,
                                                                           src.get_format());
    this->matrix_ = this->matrix_accel_;

  }
 
  this->matrix_->CopyFrom(*src.matrix_);

}

template <typename ValueType>
bool LocalMatrix<ValueType>::is_host(void) const {
  return (this->matrix_ == this->matrix_host_);
}

template <typename ValueType>
bool LocalMatrix<ValueType>::is_accel(void) const {
  return (this->matrix_ == this->matrix_accel_);
}


template <typename ValueType>
void LocalMatrix<ValueType>::info(void) const {
  
  std::string current_backend_name;
  
  if (this->matrix_ == this->matrix_host_) {
    current_backend_name = _paralution_host_name[0];
  } else {
    assert(this->matrix_ == this->matrix_accel_);
    current_backend_name = _paralution_backend_name[this->local_backend_.backend];
  }
  
  
  LOG_INFO("LocalMatrix" 
           << " name=" << this->object_name_ << ";" 
           << " rows=" << this->get_nrow() << ";" 
           << " cols=" << this->get_ncol() << ";" 
           << " nnz=" << this->get_nnz() << ";"
           << " prec=" << 8*sizeof(ValueType) << "bit;"
           << " format=" << _matrix_format_names[this->get_format()] << ";"
           << " host backend={" << _paralution_host_name[0] << "};" 
           << " accelerator backend={" << _paralution_backend_name[this->local_backend_.backend] << "};"
           << " current=" << current_backend_name);          

  // this->matrix_->info();
}

template <typename ValueType>
void LocalMatrix<ValueType>::MoveToAccelerator(void) {

  if (_paralution_available_accelerator() == false)
    LOG_VERBOSE_INFO(4,"*** info: LocalMatrix:MoveToAccelerator() no accelerator available - doing nothing");  

  if ( (_paralution_available_accelerator()) && 
       (this->matrix_ == this->matrix_host_)) {

    this->matrix_accel_ = _paralution_init_base_backend_matrix<ValueType>(this->local_backend_,
                                                                           this->get_format());
    this->matrix_accel_->CopyFrom(*this->matrix_host_); 
    
    this->matrix_ = this->matrix_accel_;
    delete this->matrix_host_;
    this->matrix_host_ = NULL;

    LOG_VERBOSE_INFO(4, "*** info: LocalMatrix:MoveToAccelerator() host to accelerator transfer");        
  }
  // if on accelerator - do nothing

}

template <typename ValueType>
void LocalMatrix<ValueType>::MoveToHost(void) {

  if (this->matrix_ == this->matrix_accel_) {

    
    this->matrix_host_ = _paralution_init_base_host_matrix<ValueType>(this->local_backend_,
                                                                      this->get_format());   
    this->matrix_host_->CopyFrom(*this->matrix_accel_); 
    
    this->matrix_ = this->matrix_host_;
    delete this->matrix_accel_;
    this->matrix_accel_ = NULL;

    LOG_VERBOSE_INFO(4, "*** info: LocalMatrix:MoveToHost() accelerator to host transfer");    
  }
  // if on host - do nothing

}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToCSR(void) {
  this->ConvertTo(CSR);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToMCSR(void) {
  this->ConvertTo(MCSR);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToBCSR(void) {
  this->ConvertTo(BCSR);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToCOO(void) {
  this->ConvertTo(COO);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToELL(void) {
  this->ConvertTo(ELL);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToDIA(void) {
  this->ConvertTo(DIA);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToHYB(void) {
  this->ConvertTo(HYB);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertToDENSE(void) {
  this->ConvertTo(DENSE);
}

template <typename ValueType>
void LocalMatrix<ValueType>::ConvertTo(const unsigned int matrix_format) {
  
  //  LOG_INFO("Convert " << _matrix_format_names[matrix_format] << " <- " << _matrix_format_names[this->get_format()] );      

  if (this->get_format() != matrix_format) {

    if ((this->get_format() != CSR) && (matrix_format != CSR))
      this->ConvertToCSR();
    
    // CPU matrix
    if (this->matrix_ == this->matrix_host_) {      
      assert(this->matrix_host_ != NULL);
      
      HostMatrix<ValueType> *new_mat;
      new_mat = _paralution_init_base_host_matrix<ValueType>(this->local_backend_,
                                                             matrix_format);
      assert(new_mat != NULL);
      
      if (new_mat->ConvertFrom(*this->matrix_host_) == false) {
        LOG_INFO("Unsupported (on host) convertion type to " << _matrix_format_names[matrix_format]);
        this->info();
        FATAL_ERROR(__FILE__, __LINE__);
      }     
        
      delete this->matrix_host_;
        
      this->matrix_host_ = new_mat ;
      this->matrix_ = this->matrix_host_; 
      
    } else {
      
      // Accelerator Matrix
      assert(this->matrix_accel_ != NULL);
      
      AcceleratorMatrix<ValueType> *new_mat;
      new_mat = _paralution_init_base_backend_matrix<ValueType>(this->local_backend_,
                                                                matrix_format);
      assert(new_mat != NULL);
      
      if (new_mat->ConvertFrom(*this->matrix_accel_) == false) {

        delete new_mat;

        this->MoveToHost();
        this->ConvertTo(matrix_format);
        this->MoveToAccelerator();

        LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ConvertTo() is performed on the host");

      } else {
        
        delete this->matrix_accel_;
        
        this->matrix_accel_ = new_mat ;
        this->matrix_ = this->matrix_accel_; 
 
     }
    }
    
    assert( this->get_format() == matrix_format );
  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  assert( ( (this->matrix_ == this->matrix_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  this->matrix_->Apply(*in.vector_, out->vector_);

}

template <typename ValueType>
void LocalMatrix<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, 
                                      LocalVector<ValueType> *out) const {

  assert( ( (this->matrix_ == this->matrix_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  this->matrix_->ApplyAdd(*in.vector_, scalar, out->vector_);

}


template <typename ValueType>
void LocalMatrix<ValueType>::ExtractDiagonal(LocalVector<ValueType> *vec_diag) const {

  assert(vec_diag != NULL);
    
  assert( ( (this->matrix_ == this->matrix_host_)  && (vec_diag->vector_ == vec_diag->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (vec_diag->vector_ == vec_diag->vector_accel_) ) );

  std::string vec_diag_name = "Diagonal elements of " + this->object_name_;
  vec_diag->Allocate(vec_diag_name, std::min(this->get_nrow(), this->get_ncol()));

  bool err = this->matrix_->ExtractDiagonal(vec_diag->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::ExtractDiagonal() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }

  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    tmp_mat.MoveToHost();
    vec_diag->MoveToHost();

    if (tmp_mat.matrix_->ExtractDiagonal(vec_diag->vector_) == false) {
      LOG_INFO("Computation of LocalMatrix::ExtractDiagonal() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ExtractDiagonal() is performed on the host");

    vec_diag->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::ExtractInverseDiagonal(LocalVector<ValueType> *vec_inv_diag) const {

  assert(vec_inv_diag != NULL);

  assert( ( (this->matrix_ == this->matrix_host_)  && (vec_inv_diag->vector_ == vec_inv_diag->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (vec_inv_diag->vector_ == vec_inv_diag->vector_accel_) ) );

  std::string vec_inv_diag_name = "Inverse of the diagonal elements of " + this->object_name_;
  vec_inv_diag->Allocate(vec_inv_diag_name, std::min(this->get_nrow(), this->get_ncol()));

  bool err = this->matrix_->ExtractInverseDiagonal(vec_inv_diag->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::ExtractInverseDiagonal() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    tmp_mat.MoveToHost();
    vec_inv_diag->MoveToHost();

    if (tmp_mat.matrix_->ExtractInverseDiagonal(vec_inv_diag->vector_) == false) {
      LOG_INFO("Computation of LocalMatrix::ExtractInverseDiagonal() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ExtractInverseDiagonal() is performed on the host");

    vec_inv_diag->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::ExtractSubMatrix(const int row_offset,
                                              const int col_offset,
                                              const int row_size,
                                              const int col_size,
                                              LocalMatrix<ValueType> *mat) const {
  assert(this != mat);
  assert(mat != NULL);
  assert(row_size > 0 && col_size > 0);

  assert( ( (this->matrix_ == this->matrix_host_)  && (mat->matrix_ == mat->matrix_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (mat->matrix_ == mat->matrix_accel_) ) );

  std::string mat_name = "Submatrix of " + this->object_name_ + " "
    + "[" + static_cast<std::ostringstream*>( &(std::ostringstream() << row_offset) )->str()
    + "," + static_cast<std::ostringstream*>( &(std::ostringstream() << col_offset) )->str() + "]-"
    + "[" + static_cast<std::ostringstream*>( &(std::ostringstream() << row_offset+row_size-1) )->str()
    + "," + static_cast<std::ostringstream*>( &(std::ostringstream() << col_offset+row_size-1) )->str() + "]";

  mat->object_name_ = mat_name;

  bool err = false;

  // if the sub matrix has only 1 row
  // it is computed on the host
  if ((this->is_host() == true) || (row_size > 1))
    err = this->matrix_->ExtractSubMatrix(row_offset, col_offset,
                                          row_size,   col_size,
                                          mat->matrix_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::ExtractSubMatrix() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    tmp_mat.MoveToHost();
    mat->MoveToHost();

    if (tmp_mat.matrix_->ExtractSubMatrix(row_offset, col_offset,
                                          row_size,   col_size,
                                          mat->matrix_) == false) {
      LOG_INFO("Computation of LocalMatrix::ExtractSubMatrix() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ExtractSubMatrix() is performed on the host");

    mat->MoveToAccelerator();

  }  


}

template <typename ValueType>
void LocalMatrix<ValueType>::ExtractSubMatrices(const int row_num_blocks,
                                                const int col_num_blocks,
                                                const int *row_offset,
                                                const int *col_offset,
                                                LocalMatrix<ValueType> ***mat) const {

  assert(row_num_blocks >0);
  assert(col_num_blocks >0);

  // implementation via ExtractSubMatrix() calls
//OMP not working with OpenCL
//#pragma omp parallel for schedule(dynamic,1) collapse(2)
  for (int i=0; i<row_num_blocks; ++i)
    for (int j=0; j<col_num_blocks; ++j)
      this->ExtractSubMatrix(row_offset[i],
                             col_offset[j],
                             row_offset[i+1] - row_offset[i],
                             col_offset[j+1] - col_offset[j],
                             mat[i][j]);

}

template <typename ValueType>
void LocalMatrix<ValueType>::LUAnalyse(void) {
  this->matrix_->LUAnalyse();
}

template <typename ValueType>
void LocalMatrix<ValueType>::LUAnalyseClear(void) {
  this->matrix_->LUAnalyseClear();
}

template <typename ValueType>
void LocalMatrix<ValueType>::LUSolve(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  assert( ( (this->matrix_ == this->matrix_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );


  bool err = this->matrix_->LUSolve(*in.vector_, out->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::LUSolve() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    LocalVector<ValueType> tmp_vec;
    tmp_vec.CloneFrom(in);

    tmp_mat.MoveToHost();
    tmp_vec.MoveToHost();
    out->MoveToHost();

    if (tmp_mat.matrix_->LUSolve(*tmp_vec.vector_, out->vector_) ==  false) {
      LOG_INFO("Computation of LocalMatrix::LUSolve() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::LUSolve() is performed on the host");

    out->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::LLAnalyse(void) {
  this->matrix_->LLAnalyse();
}

template <typename ValueType>
void LocalMatrix<ValueType>::LLAnalyseClear(void) {
  this->matrix_->LLAnalyseClear();
}


template <typename ValueType>
void LocalMatrix<ValueType>::LLSolve(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  assert( ( (this->matrix_ == this->matrix_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  bool err = this->matrix_->LLSolve(*in.vector_, out->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::LLSolve() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    LocalVector<ValueType> tmp_vec;
    tmp_vec.CloneFrom(in);

    tmp_mat.MoveToHost();
    tmp_vec.MoveToHost();
    out->MoveToHost();

    if (tmp_mat.matrix_->LLSolve(*tmp_vec.vector_, out->vector_) ==  false) {
      LOG_INFO("Computation of LocalMatrix::LLSolve() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::LLSolve() is performed on the host");

    out->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::LAnalyse(const bool diag_unit) {

  this->matrix_->LAnalyse(diag_unit);

}

template <typename ValueType>
void LocalMatrix<ValueType>::LAnalyseClear(void) {

  this->matrix_->LAnalyseClear();

}

template <typename ValueType>
void LocalMatrix<ValueType>::LSolve(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  assert( ( (this->matrix_ == this->matrix_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  bool err = this->matrix_->LSolve(*in.vector_, out->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::LSolve() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    LocalVector<ValueType> tmp_vec;
    tmp_vec.CloneFrom(in);

    tmp_mat.MoveToHost();
    tmp_vec.MoveToHost();
    out->MoveToHost();

    if (tmp_mat.matrix_->LSolve(*tmp_vec.vector_, out->vector_) ==  false) {
      LOG_INFO("Computation of LocalMatrix::LSolve() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::LSolve() is performed on the host");

    out->MoveToAccelerator();

  }  


}


template <typename ValueType>
void LocalMatrix<ValueType>::UAnalyse(const bool diag_unit) {

  this->matrix_->UAnalyse(diag_unit);

}

template <typename ValueType>
void LocalMatrix<ValueType>::UAnalyseClear(void) {

  this->matrix_->UAnalyseClear();

}

template <typename ValueType>
void LocalMatrix<ValueType>::USolve(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  assert( ( (this->matrix_ == this->matrix_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  bool err = this->matrix_->USolve(*in.vector_, out->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::USolve() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);

    LocalVector<ValueType> tmp_vec;
    tmp_vec.CloneFrom(in);

    tmp_mat.MoveToHost();
    tmp_vec.MoveToHost();
    out->MoveToHost();

    if (tmp_mat.matrix_->USolve(*tmp_vec.vector_, out->vector_) ==  false) {
      LOG_INFO("Computation of LocalMatrix::USolve() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ULSolve() is performed on the host");

    out->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::ILU0Factorize(void) {

  // Only CSR factorization
  this->ConvertToCSR();

  bool err = this->matrix_->ILU0Factorize();

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::ILU0Factorize() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->ILU0Factorize() ==  false) {
      LOG_INFO("Computation of LocalMatrix::ILU0Factorize() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ILU0Factorize() is performed on the host");

    this->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::ILUTFactorize(const ValueType t, const int maxrow) {

  // host only
  bool on_host = this->is_host();
  if (on_host == false) 
    this->MoveToHost();

  this->ConvertToCSR();

  this->matrix_->ILUTFactorize(t, maxrow);

  if (on_host == false) {
    this->MoveToAccelerator();
    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ILUTFactorize() is performed on the host");
  }
}

template <typename ValueType>
void LocalMatrix<ValueType>::ILUpFactorize(const int p, const bool level) {

  // host only
  bool on_host = this->is_host();
  if (on_host == false) 
    this->MoveToHost();

  this->ConvertToCSR();

  if (p == 0) {

    this->ILU0Factorize();

  } else {

    if (level == true) {
      
      // with control levels
      LocalMatrix structure;
      structure.CloneFrom(*this);
      
      structure.SymbolicPower(p+1);
      
      this->matrix_->ILUpFactorizeNumeric(p, *structure.matrix_);
      
    } else {
      
      // without control levels
      LocalMatrix values;
      values.CloneFrom(*this);
      
      this->SymbolicPower(p+1);
      this->MatrixAdd(values);
      
      this->matrix_->ILU0Factorize();
    }
  }

  if (on_host == false) {
    this->MoveToAccelerator();
    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ILUpFactorize() is performed on the host");
  }
}

template <typename ValueType>
void LocalMatrix<ValueType>::IC0Factorize(void) {

  // TODO - under development 
  FATAL_ERROR(__FILE__, __LINE__);

  // Only CSR factorization
  this->ConvertToCSR();

  bool err = this->matrix_->IC0Factorize();

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::IC0Factorize() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->IC0Factorize() ==  false) {
      LOG_INFO("Computation of LocalMatrix::IC0Factorize() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::IC0Factorize() is performed on the host");

    this->MoveToAccelerator();

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::MultiColoring(int &num_colors,
                                           int **size_colors,
                                           LocalVector<int> *permutation) const {

  bool err = this->matrix_->MultiColoring(num_colors, size_colors, permutation->vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::MultiColoring() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    LocalMatrix<ValueType> mat_host;
    mat_host.CloneFrom(*this);

    mat_host.MoveToHost();
    permutation->MoveToHost();

    if (mat_host.matrix_->MultiColoring(num_colors, size_colors, permutation->vector_) == false) {
      LOG_INFO("Computation of LocalMatrix::MultiColoring() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::MultiColoring() is performed on the host");

    permutation->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::MaximalIndependentSet(int &size,
                                                   LocalVector<int> *permutation) const {

  assert(permutation != NULL);
    
  assert(this->get_nrow() == this->get_ncol());

  // only CSR, only squared matrices, only on host
  if ((this->is_accel() == true)  || 
      (this->get_format() != CSR)) {

    LocalMatrix<ValueType> mat_host;
    mat_host.CloneFrom(*this);

    if (this->is_accel() == true) {
      mat_host.MoveToHost();
      permutation->MoveToHost();
    }

    mat_host.ConvertToCSR();


    mat_host.MaximalIndependentSet(size, permutation);

    if (this->is_accel() == true) {
      permutation->MoveToAccelerator();

      LOG_VERBOSE_INFO(3, "*** warning: LocalMatrix::MaximalIndependentSet() is performed on the host");
    }

  } else {

    std::string vec_perm_name = "MaximalIndependentSet permutation of " + this->object_name_;
    
    permutation->Allocate(vec_perm_name, this->get_nrow());
    
    this->matrix_->MaximalIndependentSet(size, permutation->vector_);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::Permute(const LocalVector<int> &permutation) {

  bool err = this->matrix_->Permute(*permutation.vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::Permute() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }

  if (err == false) {

    this->MoveToHost();

    LocalVector<int> perm_tmp;
    perm_tmp.CopyFrom(permutation);
    perm_tmp.MoveToHost();

    if (this->matrix_->Permute(*perm_tmp.vector_) == false) {
      LOG_INFO("Computation of LocalMatrix::Permute() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::Permute() is performed on the host");

    this->MoveToAccelerator();

  }


}

template <typename ValueType>
void LocalMatrix<ValueType>::PermuteBackward(const LocalVector<int> &permutation) {

  // Only COO can permute (so far)

  if (this->get_format() == COO) {

    bool err = this->matrix_->PermuteBackward(*permutation.vector_);
    
    if (err == false) {
           
      this->MoveToHost();
      
      LocalVector<int> perm_tmp;
      perm_tmp.CopyFrom(permutation);
      perm_tmp.MoveToHost();
      
      if (this->matrix_->PermuteBackward(*perm_tmp.vector_) == false) {
        LOG_INFO("Computation of LocalMatrix::PermuteBackwar() fail");
        this->info();
        FATAL_ERROR(__FILE__, __LINE__);
      }
      
      LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::PermuteBackward() is performed on the host");

      this->MoveToAccelerator();
      
    }
    
  } else {
    
    const unsigned int mat_format = this->get_format();
    this->ConvertToCOO();
    
    this->PermuteBackward(permutation);
    
    this->ConvertTo(mat_format);
  }
  

}


template <typename ValueType>
void LocalMatrix<ValueType>::SymbolicPower(const int p) {

  // CSR, host only
  bool on_host = this->is_host();
  if (on_host == false)
    this->MoveToHost();

  this->ConvertToCSR();

  this->matrix_->SymbolicPower(p);

  if (on_host == false) {
    this->MoveToAccelerator();

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::SymbolicPower() is performed on the host");
  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::MatrixAdd(const LocalMatrix<ValueType> &mat, const ValueType alpha, 
                                       const ValueType beta, const bool structure) {

  assert(&mat != this);
    
  assert( ( (this->matrix_ == this->matrix_host_)  && (mat.matrix_ == mat.matrix_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (mat.matrix_ == mat.matrix_accel_) ) );

  bool err = this->matrix_->MatrixAdd(*mat.matrix_,
                                      alpha, beta, 
                                      structure);


  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::MatrixAdd() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {
   
    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(mat);
    
    tmp_mat.MoveToHost();
    this->MoveToHost();

    if (this->matrix_->MatrixAdd(*tmp_mat.matrix_,
                                 alpha, beta, 
                                 structure) ==  false) {

      LOG_INFO("Computation of LocalMatrix::MatrixAdd() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::MatrixAdd() is performed on the host");

    this->MoveToAccelerator();

  }  


}

template <typename ValueType>
void LocalMatrix<ValueType>::Gershgorin(ValueType &lambda_min,
                                        ValueType &lambda_max) const {


  bool err = this->matrix_->Gershgorin(lambda_min, lambda_max);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::Gershgorin() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {
   
    LocalMatrix<ValueType> tmp_mat;
    tmp_mat.CloneFrom(*this);
    tmp_mat.MoveToHost();

    if (tmp_mat.matrix_->Gershgorin(lambda_min, lambda_max) ==  false) {

      LOG_INFO("Computation of LocalMatrix::Gershgorin() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::Gershgorin() is performed on the host");

  }  

}

template <typename ValueType>
void LocalMatrix<ValueType>::Scale(const ValueType alpha) {

  bool err = this->matrix_->Scale(alpha);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::Scale() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }

  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->Scale(alpha) == false) {
      LOG_INFO("Computation of LocalMatrix::Scale() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::Scale() is performed on the host");

    this->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::ScaleDiagonal(const ValueType alpha) {

  bool err = this->matrix_->ScaleDiagonal(alpha);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::ScaleDiagonal() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->ScaleDiagonal(alpha) == false) {
      LOG_INFO("Computation of LocalMatrix::ScaleDiagonal() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ScaleDiagonal() is performed on the host");

    this->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::ScaleOffDiagonal(const ValueType alpha) {

  bool err = this->matrix_->ScaleOffDiagonal(alpha);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::ScaleOffDiagonal() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }

  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->ScaleOffDiagonal(alpha) == false) {
      LOG_INFO("Computation of LocalMatrix::ScaleOffDiagonal() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::ScaleOffDiagonal() is performed on the host");

    this->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::AddScalar(const ValueType alpha) {

  bool err = this->matrix_->AddScalar(alpha);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::AddScalar() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->AddScalar(alpha) == false) {
      LOG_INFO("Computation of LocalMatrix::AddScalar() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::AddScalar() is performed on the host");

    this->MoveToAccelerator();

  }

}


template <typename ValueType>
void LocalMatrix<ValueType>::AddScalarDiagonal(const ValueType alpha) {

  bool err = this->matrix_->AddScalarDiagonal(alpha);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::AddScalarDiagonal() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->AddScalarDiagonal(alpha) == false) {
      LOG_INFO("Computation of LocalMatrix::AddScalarDiagonal() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::AddScalarDiagonal() is performed on the host");

    this->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::AddScalarOffDiagonal(const ValueType alpha) {

  bool err = this->matrix_->AddScalarOffDiagonal(alpha);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::AddScalarOffDiagonal() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->AddScalarOffDiagonal(alpha) == false) {
      LOG_INFO("Computation of LocalMatrix::AddScalarOffDiagonal() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::AddScalarOffDiagonal() is performed on the host");

    this->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::MatrixMult(const LocalMatrix<ValueType> &A, const LocalMatrix<ValueType> &B) {

  assert(&A != this);
  assert(&B != this);
    
  assert( ( (this->matrix_ == this->matrix_host_)  && (A.matrix_ == A.matrix_host_)  && (B.matrix_ == B.matrix_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (A.matrix_ == A.matrix_accel_) && (B.matrix_ == B.matrix_accel_)) );

  this->object_name_ = A.object_name_ + "x" + B.object_name_;

  bool err = this->matrix_->MatMatMult(*A.matrix_, *B.matrix_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::MatMatMult() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {
   
    LocalMatrix<ValueType> tmp_matA;
    tmp_matA.CloneFrom(A);

    LocalMatrix<ValueType> tmp_matB;
    tmp_matB.CloneFrom(B);
    
    tmp_matA.MoveToHost();
    tmp_matB.MoveToHost();
    this->MoveToHost();

    if (this->matrix_->MatMatMult(*tmp_matA.matrix_, *tmp_matB.matrix_) ==  false) {

      LOG_INFO("Computation of LocalMatrix::MatMatMult() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::MatMatMult() is performed on the host");

    this->MoveToAccelerator();

  }  


  //  this->matrix_->SymbolicMatMatMult(*A.matrix_, *B.matrix_);
  //  this->matrix_->NumericMatMatMult (*A.matrix_, *B.matrix_);

}

template <typename ValueType>
void LocalMatrix<ValueType>::DiagonalMatrixMult(const LocalVector<ValueType> &diag) {

  assert( ( (this->matrix_ == this->matrix_host_)  && (diag.vector_ == diag.vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (diag.vector_ == diag.vector_accel_) ) );


  bool err = this->matrix_->DiagMatMult(*diag.vector_);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::MatMatMult() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {
   
    LocalVector<ValueType> tmp_vec;
    tmp_vec.CloneFrom(diag);

    tmp_vec.MoveToHost();
    this->MoveToHost();

    if (this->matrix_->DiagMatMult(*tmp_vec.vector_) ==  false) {

      LOG_INFO("Computation of LocalMatrix::MatMatMult() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::MatMatMult() is performed on the host");

    this->MoveToAccelerator();

  }  
  
}
  
template <typename ValueType>
void LocalMatrix<ValueType>::Compress(const ValueType drop_off) {

  assert(drop_off > ValueType(0.0));

  // only CSR, only on host
  if ((this->is_accel() == true)  || 
      (this->get_format() != CSR)) {

    unsigned int mat = this->get_format();
    bool host = this->is_host();

    if (mat != CSR)
      this->ConvertToCSR();

    if (host == false)
      this->MoveToHost();

    this->Compress(drop_off);

    if (mat != CSR)
      this->ConvertTo(mat);

    if (host == false) {
      this->MoveToAccelerator();
      LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::Compress() is performed on the host");
    }

  } else {

    bool err = this->matrix_->Compress(drop_off);

    if (err == true) {
      LOG_INFO("Computation of LocalMatrix::Compress() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

  }  
  
}


template <typename ValueType>
void LocalMatrix<ValueType>::Transpose(void) {

  bool err = this->matrix_->Transpose();

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::Transpose() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {

    this->MoveToHost();

    if (this->matrix_->Transpose() == false) {
      LOG_INFO("Computation of LocalMatrix::Transpose() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::Transpose() is performed on the host");

    this->MoveToAccelerator();

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::AMGConnect(const ValueType eps, LocalVector<int> *connections) const {

  // only CSR, only on host - for now
  if ((this->is_accel() == true)  ||
      (this->get_format() != CSR)) {

    LocalMatrix<ValueType> mat_host;
    mat_host.CloneFrom(*this);

    if (this->is_accel() == true) {
      mat_host.MoveToHost();
      connections->MoveToHost();
    }

    mat_host.ConvertToCSR();
    mat_host.AMGConnect(eps, connections);

    if (this->is_accel() == true) {
      connections->MoveToAccelerator();

      LOG_VERBOSE_INFO(2, "*** warning: LocalMatirx::AMGConnect() is performed on the host");
    }

  } else {

    std::string vec_name = "Connections of " + this->object_name_;

    this->matrix_->AMGConnect(eps, connections->vector_);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::AMGAggregate(const LocalVector<int> &connections, LocalVector<int> *aggregates) const {

  // only CSR, only on host
  if ((this->is_accel() == true)  || 
      (this->get_format() != CSR)) {

    LocalMatrix<ValueType> mat_host;
    mat_host.CloneFrom(*this);
    LocalVector<int> conn_host;

    if (this->is_accel() == true) {
      conn_host.CloneFrom(connections);
      mat_host.MoveToHost();
      aggregates->MoveToHost();
      conn_host.MoveToHost();
    }

    mat_host.ConvertToCSR();

    mat_host.AMGAggregate(conn_host, aggregates);

    if (this->is_accel() == true) {
      aggregates->MoveToAccelerator();

      LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::AMGAggregate() is performed on the host");
    }

  } else {
    
    std::string vec_name = "Aggregate of " + this->object_name_;

    this->matrix_->AMGAggregate(*connections.vector_, aggregates->vector_);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::AMGSmoothedAggregation(const ValueType relax,
                                                    const LocalVector<int> &aggregates,
                                                    const LocalVector<int> &connections,
                                                          LocalMatrix<ValueType> *prolong,
                                                          LocalMatrix<ValueType> *restrict) const {

  // only CSR, only on host
  if ((this->is_accel() == true)  ||
      (this->get_format() != CSR)) {

    LocalMatrix<ValueType> mat_host;
    mat_host.CloneFrom(*this);

    LocalVector<int> vec_agg;
    LocalVector<int> vec_conn;

    if (this->is_accel() == true) {
      vec_agg.CloneFrom(aggregates);
      vec_conn.CloneFrom(connections);
      mat_host.MoveToHost();
      vec_agg.MoveToHost();
      vec_conn.MoveToHost();
      prolong->MoveToHost();
      restrict->MoveToHost();
    }

    mat_host.ConvertToCSR();

    mat_host.AMGSmoothedAggregation(relax, vec_agg, vec_conn, prolong, restrict);


  if (this->is_accel() == true) {
      prolong->MoveToAccelerator();
      restrict->MoveToAccelerator();

      LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::AMGSmoothedAggregation() is performed on the host");
    }

  } else {

    std::string prolong_name = "Prolongation Operator of " + this->object_name_;
    std::string restrict_name = "Restriction Operator of " + this->object_name_;

    this->matrix_->AMGSmoothedAggregation(relax, *aggregates.vector_, *connections.vector_, prolong->matrix_, restrict->matrix_);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::AMGAggregation(const LocalVector<int> &aggregates,
                                                  LocalMatrix<ValueType> *prolong,
                                                  LocalMatrix<ValueType> *restrict) const {

  // only CSR, only on host
  if ((this->is_accel() == true)  ||
      (this->get_format() != CSR)) {

    LocalMatrix<ValueType> mat_host;
    mat_host.CloneFrom(*this);

    LocalVector<int> vec_agg;

    if (this->is_accel() == true) {
      vec_agg.CloneFrom(aggregates);
      mat_host.MoveToHost();
      vec_agg.MoveToHost();
      prolong->MoveToHost();
      restrict->MoveToHost();
    }

    mat_host.ConvertToCSR();

    mat_host.AMGAggregation(vec_agg, prolong, restrict);


  if (this->is_accel() == true) {
      prolong->MoveToAccelerator();
      restrict->MoveToAccelerator();

      LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::AMGAggregation() is performed on the host");
    }

  } else {

    std::string prolong_name = "Prolongation Operator of " + this->object_name_;
    std::string restrict_name = "Restriction Operator of " + this->object_name_;

    this->matrix_->AMGAggregation(*aggregates.vector_, prolong->matrix_, restrict->matrix_);

  }

}

template <typename ValueType>
void LocalMatrix<ValueType>::CreateFromMap(const LocalVector<int> &map, const int n, const int m) {

  assert( ( (this->matrix_ == this->matrix_host_)  && (map.vector_ == map.vector_host_)) ||
          ( (this->matrix_ == this->matrix_accel_) && (map.vector_ == map.vector_accel_) ) );

  assert(map.get_size() == n);

  bool err = this->matrix_->CreateFromMap(*map.vector_, n , m);

  if ((err == false) && (this->is_host() == true)) {
    LOG_INFO("Computation of LocalMatrix::CreateFromMap() fail");
    this->info();
    FATAL_ERROR(__FILE__, __LINE__);    
  }


  if (err == false) {
   
    LocalVector<int> tmp_map;
    tmp_map.CloneFrom(map);

    tmp_map.MoveToHost();
    this->MoveToHost();

    if (this->matrix_->CreateFromMap(*map.vector_, n, m) ==  false) {

      LOG_INFO("Computation of LocalMatrix::CreateFromMap() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    LOG_VERBOSE_INFO(2, "*** warning: LocalMatrix::CreateFromMap() is performed on the host");

    this->MoveToAccelerator();

  }  

}

template class LocalMatrix<double>;
template class LocalMatrix<float>;

}
