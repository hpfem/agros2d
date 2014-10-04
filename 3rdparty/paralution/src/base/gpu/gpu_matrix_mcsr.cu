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


#include "gpu_matrix_csr.hpp"
#include "gpu_matrix_coo.hpp"
#include "gpu_matrix_dia.hpp"
#include "gpu_matrix_ell.hpp"
#include "gpu_matrix_hyb.hpp"
#include "gpu_matrix_mcsr.hpp"
#include "gpu_matrix_bcsr.hpp"
#include "gpu_matrix_dense.hpp"
#include "gpu_vector.hpp"
#include "../host/host_matrix_mcsr.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"
#include "cuda_kernels_mcsr.hpp"
#include "gpu_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <cuda.h>
#include <cusparse_v2.h>


namespace paralution {

template <typename ValueType>
GPUAcceleratorMatrixMCSR<ValueType>::GPUAcceleratorMatrixMCSR() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
GPUAcceleratorMatrixMCSR<ValueType>::GPUAcceleratorMatrixMCSR(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "GPUAcceleratorMatrixMCSR::GPUAcceleratorMatrixMCSR()",
            "constructor with local_backend");

  this->mat_.row_offset = NULL;  
  this->mat_.col = NULL;  
  this->mat_.val = NULL;
  this->set_backend(local_backend); 

  CHECK_CUDA_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
GPUAcceleratorMatrixMCSR<ValueType>::~GPUAcceleratorMatrixMCSR() {

  LOG_DEBUG(this, "GPUAcceleratorMatrixMCSR::~GPUAcceleratorMatrixMCSR()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::info(void) const {

  LOG_INFO("GPUAcceleratorMatrixMCSR<ValueType>");

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::AllocateMCSR(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_gpu(nrow+1, &this->mat_.row_offset);
    allocate_gpu(nnz,    &this->mat_.col);
    allocate_gpu(nnz,    &this->mat_.val);
    
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nrow+1, mat_.row_offset);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nnz, mat_.col);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nnz, mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_gpu(&this->mat_.row_offset);
    free_gpu(&this->mat_.col);
    free_gpu(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }


}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixMCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixMCSR<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateMCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    cudaMemcpy(this->mat_.row_offset,     // dst
               cast_mat->mat_.row_offset, // src
               (this->get_nrow()+1)*sizeof(int), // size
               cudaMemcpyHostToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(this->mat_.col,     // dst
               cast_mat->mat_.col, // src
               this->get_nnz()*sizeof(int), // size
               cudaMemcpyHostToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(this->mat_.val,     // dst
               cast_mat->mat_.val, // src
               this->get_nnz()*sizeof(ValueType), // size
               cudaMemcpyHostToDevice);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixMCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixMCSR<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateMCSR(this->get_nnz(), this->get_nrow(), this->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    cudaMemcpy(cast_mat->mat_.row_offset, // dst
               this->mat_.row_offset,     // src
               (this->get_nrow()+1)*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(cast_mat->mat_.col, // dst
               this->mat_.col,     // src
               this->get_nnz()*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(cast_mat->mat_.val, // dst
               this->mat_.val,     // src
               this->get_nnz()*sizeof(ValueType), // size
               cudaMemcpyDeviceToHost);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
   
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixMCSR<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixMCSR<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateMCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    cudaMemcpy(this->mat_.row_offset,         // dst
               gpu_cast_mat->mat_.row_offset, // src
               (this->get_nrow()+1)*sizeof(int), // size
               cudaMemcpyDeviceToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(this->mat_.col,         // dst
               gpu_cast_mat->mat_.col, // src
               this->get_nnz()*sizeof(int), // size
               cudaMemcpyDeviceToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(this->mat_.val,         // dst
               gpu_cast_mat->mat_.val, // src
               this->get_nnz()*sizeof(ValueType), // size
               cudaMemcpyDeviceToDevice);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
  } else {

    //CPU to GPU
    if ((host_cast_mat = dynamic_cast<const HostMatrix<ValueType>*> (&src)) != NULL) {
      
      this->CopyFromHost(*host_cast_mat);
      
    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixMCSR<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixMCSR<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateMCSR(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    cudaMemcpy(gpu_cast_mat->mat_.row_offset, // dst
               this->mat_.row_offset,         // src
               (this->get_nrow()+1)*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(gpu_cast_mat->mat_.col, // dst
               this->mat_.col,         // src
               this->get_nnz()*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(gpu_cast_mat->mat_.val, // dst
               this->mat_.val,         // src
               this->get_nnz()*sizeof(ValueType), // size
               cudaMemcpyDeviceToHost);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
   
    
  } else {

    //GPU to CPU
    if ((host_cast_mat = dynamic_cast<HostMatrix<ValueType>*> (dst)) != NULL) {
      
      this->CopyToHost(host_cast_mat);

    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }


}


template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyFromHostAsync(const HostMatrix<ValueType> &src) {

  const HostMatrixMCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixMCSR<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateMCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    cudaMemcpyAsync(this->mat_.row_offset,     // dst
                    cast_mat->mat_.row_offset, // src
                    (this->get_nrow()+1)*sizeof(int), // size
                    cudaMemcpyHostToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpyAsync(this->mat_.col,     // dst
                    cast_mat->mat_.col, // src
                    this->get_nnz()*sizeof(int), // size
                    cudaMemcpyHostToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpyAsync(this->mat_.val,     // dst
                    cast_mat->mat_.val, // src
                    this->get_nnz()*sizeof(ValueType), // size
                    cudaMemcpyHostToDevice);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyToHostAsync(HostMatrix<ValueType> *dst) const {

  HostMatrixMCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixMCSR<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateMCSR(this->get_nnz(), this->get_nrow(), this->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    cudaMemcpyAsync(cast_mat->mat_.row_offset, // dst
                    this->mat_.row_offset,     // src
                    (this->get_nrow()+1)*sizeof(int), // size
                    cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpyAsync(cast_mat->mat_.col, // dst
                    this->mat_.col,     // src
                    this->get_nnz()*sizeof(int), // size
                    cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpyAsync(cast_mat->mat_.val, // dst
                    this->mat_.val,     // src
                    this->get_nnz()*sizeof(ValueType), // size
                    cudaMemcpyDeviceToHost);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
   
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyFromAsync(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixMCSR<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixMCSR<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateMCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    cudaMemcpy(this->mat_.row_offset,         // dst
               gpu_cast_mat->mat_.row_offset, // src
               (this->get_nrow()+1)*sizeof(int), // size
               cudaMemcpyDeviceToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(this->mat_.col,         // dst
               gpu_cast_mat->mat_.col, // src
               this->get_nnz()*sizeof(int), // size
               cudaMemcpyDeviceToDevice);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(this->mat_.val,         // dst
               gpu_cast_mat->mat_.val, // src
               this->get_nnz()*sizeof(ValueType), // size
               cudaMemcpyDeviceToDevice);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
  } else {

    //CPU to GPU
    if ((host_cast_mat = dynamic_cast<const HostMatrix<ValueType>*> (&src)) != NULL) {
      
      this->CopyFromHostAsync(*host_cast_mat);
      
    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::CopyToAsync(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixMCSR<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixMCSR<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateMCSR(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    cudaMemcpy(gpu_cast_mat->mat_.row_offset, // dst
               this->mat_.row_offset,         // src
               (this->get_nrow()+1)*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(gpu_cast_mat->mat_.col, // dst
               this->mat_.col,         // src
               this->get_nnz()*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    cudaMemcpy(gpu_cast_mat->mat_.val, // dst
               this->mat_.val,         // src
               this->get_nnz()*sizeof(ValueType), // size
               cudaMemcpyDeviceToHost);    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
   
    
  } else {

    //GPU to CPU
    if ((host_cast_mat = dynamic_cast<HostMatrix<ValueType>*> (dst)) != NULL) {
      
      this->CopyToHostAsync(host_cast_mat);

    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }


}


template <typename ValueType>
bool GPUAcceleratorMatrixMCSR<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const GPUAcceleratorMatrixMCSR<ValueType>   *cast_mat_mcsr;
  
  if ((cast_mat_mcsr = dynamic_cast<const GPUAcceleratorMatrixMCSR<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_mcsr);
      return true;

  }

  /*
  const GPUAcceleratorMatrixCSR<ValueType>   *cast_mat_csr;
  if ((cast_mat_csr = dynamic_cast<const GPUAcceleratorMatrixCSR<ValueType>*> (&mat)) != NULL) {
    
    this->Clear();
    
    FATAL_ERROR(__FILE__, __LINE__);
    
    this->nrow_ = cast_mat_csr->get_nrow();
    this->ncol_ = cast_mat_csr->get_ncol();
    this->nnz_  = cast_mat_csr->get_nnz();
    
    return true;
    
  }
  */

  return false;

}

template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    const GPUAcceleratorVector<ValueType> *cast_in = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&in) ; 
    GPUAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      GPUAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    int nrow = this->get_nrow();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);
    
    kernel_mcsr_spmv_scalar<ValueType, int> <<<GridSize, BlockSize>>> (nrow, this->mat_.row_offset, this->mat_.col, this->mat_.val,
                                                                       cast_in->vec_, cast_out->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      

  }


}


template <typename ValueType>
void GPUAcceleratorMatrixMCSR<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                  BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    const GPUAcceleratorVector<ValueType> *cast_in = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&in) ; 
    GPUAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      GPUAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);    

    int nrow = this->get_nrow();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);
    
    kernel_mcsr_add_spmv_scalar<ValueType, int> <<<GridSize, BlockSize>>> (nrow, this->mat_.row_offset, this->mat_.col, this->mat_.val,
                                                                           scalar, cast_in->vec_, cast_out->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }


}


template class GPUAcceleratorMatrixMCSR<double>;
template class GPUAcceleratorMatrixMCSR<float>;

}

