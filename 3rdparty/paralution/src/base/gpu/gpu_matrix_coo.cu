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
#include "../host/host_matrix_coo.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"
#include "cuda_kernels_coo.hpp"
#include "gpu_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <cuda.h>
#include <cusparse_v2.h>


namespace paralution {

template <typename ValueType>
GPUAcceleratorMatrixCOO<ValueType>::GPUAcceleratorMatrixCOO() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
GPUAcceleratorMatrixCOO<ValueType>::GPUAcceleratorMatrixCOO(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "GPUAcceleratorMatrixCOO::GPUAcceleratorMatrixCOO()",
            "constructor with local_backend");

  this->mat_.row = NULL;  
  this->mat_.col = NULL;  
  this->mat_.val = NULL;
  this->set_backend(local_backend); 

  CHECK_CUDA_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
GPUAcceleratorMatrixCOO<ValueType>::~GPUAcceleratorMatrixCOO() {

  LOG_DEBUG(this, "GPUAcceleratorMatrixCOO::~GPUAcceleratorMatrixCOO()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::info(void) const {

  LOG_INFO("GPUAcceleratorMatrixCOO<ValueType>");

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::AllocateCOO(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_gpu(nnz, &this->mat_.row);
    allocate_gpu(nnz, &this->mat_.col);
    allocate_gpu(nnz, &this->mat_.val);
 
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nnz, this->mat_.row);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nnz, this->mat_.col);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nnz, this->mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::SetDataPtrCOO(int **row, int **col, ValueType **val,
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
void GPUAcceleratorMatrixCOO<ValueType>::LeaveDataPtrCOO(int **row, int **col, ValueType **val) {

  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);
  assert(this->get_nnz() > 0);

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
void GPUAcceleratorMatrixCOO<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_gpu(&this->mat_.row);
    free_gpu(&this->mat_.col);
    free_gpu(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }


}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

  if (this->get_nnz() > 0) {

      assert(this->get_nnz()  == src.get_nnz());
      assert(this->get_nrow()  == src.get_nrow());
      assert(this->get_ncol()  == src.get_ncol());
      
      cudaMemcpy(this->mat_.row,     // dst
                 cast_mat->mat_.row, // src
                 (this->get_nnz())*sizeof(int), // size
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
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixCOO<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol() );

  if (this->get_nnz() > 0) {

      assert(this->get_nnz()  == dst->get_nnz());
      assert(this->get_nrow() == dst->get_nrow());
      assert(this->get_ncol() == dst->get_ncol());
      
      cudaMemcpy(cast_mat->mat_.row, // dst
                 this->mat_.row,     // src
                 this->get_nnz()*sizeof(int), // size           
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
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixCOO<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixCOO<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    if (this->get_nnz() > 0) {

      cudaMemcpy(this->mat_.row,         // dst
                 gpu_cast_mat->mat_.row, // src
                 (this->get_nnz())*sizeof(int), // size
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
    }

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
void GPUAcceleratorMatrixCOO<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixCOO<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixCOO<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateCOO(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    if (this->get_nnz() > 0) {

      cudaMemcpy(gpu_cast_mat->mat_.row, // dst
                 this->mat_.row,         // src
                 (this->get_nnz())*sizeof(int), // size
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
    }
    
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
void GPUAcceleratorMatrixCOO<ValueType>::CopyFromHostAsync(const HostMatrix<ValueType> &src) {

  const HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

  if (this->get_nnz() > 0) {

      assert(this->get_nnz()  == src.get_nnz());
      assert(this->get_nrow()  == src.get_nrow());
      assert(this->get_ncol()  == src.get_ncol());
      
      cudaMemcpyAsync(this->mat_.row,     // dst
                      cast_mat->mat_.row, // src
                      (this->get_nnz())*sizeof(int), // size
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
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::CopyToHostAsync(HostMatrix<ValueType> *dst) const {

  HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixCOO<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol() );

  if (this->get_nnz() > 0) {

      assert(this->get_nnz()  == dst->get_nnz());
      assert(this->get_nrow() == dst->get_nrow());
      assert(this->get_ncol() == dst->get_ncol());
      
      cudaMemcpyAsync(cast_mat->mat_.row, // dst
                      this->mat_.row,     // src
                      this->get_nnz()*sizeof(int), // size           
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
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::CopyFromAsync(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixCOO<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixCOO<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    if (this->get_nnz() > 0) {

      cudaMemcpy(this->mat_.row,         // dst
                 gpu_cast_mat->mat_.row, // src
                 (this->get_nnz())*sizeof(int), // size
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
    }

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
void GPUAcceleratorMatrixCOO<ValueType>::CopyToAsync(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixCOO<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixCOO<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateCOO(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    if (this->get_nnz() > 0) {

      cudaMemcpy(gpu_cast_mat->mat_.row, // dst
                 this->mat_.row,         // src
                 (this->get_nnz())*sizeof(int), // size
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
    }
    
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
bool GPUAcceleratorMatrixCOO<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const GPUAcceleratorMatrixCOO<ValueType>   *cast_mat_coo;
  if ((cast_mat_coo = dynamic_cast<const GPUAcceleratorMatrixCOO<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_coo);
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
void GPUAcceleratorMatrixCOO<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const GPUAcceleratorVector<ValueType> *cast_in = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&in) ; 
    GPUAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      GPUAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    cast_out->Zeros();

    // do not support super small matrices
    assert(this->get_nnz() > this->local_backend_.GPU_wrap); 

    // ----------------------------------------------------------
    // Modified and adopted from CUSP 0.3.1, 
    // http://code.google.com/p/cusp-library/
    // NVIDIA, APACHE LICENSE 2.0
    // ----------------------------------------------------------
    // see __spmv_coo_flat(...)
    // ----------------------------------------------------------
    // CHANGELOG
    // - adopted interface
    // ----------------------------------------------------------  

    //TODO
    //move in extra file -  max_active_blocks, warp_size, block_size

    const unsigned int BLOCK_SIZE = this->local_backend_.GPU_block_size;
    //    const unsigned int MAX_BLOCKS = this->local_backend_.GPU_max_blocks;

    const unsigned int MAX_BLOCKS = 32; //  cusp::detail::device::arch::max_active_blocks(spmv_coo_flat_kernel<IndexType, ValueType, BLOCK_SIZE, UseCache>, BLOCK_SIZE, (size_t) 0);

    const unsigned int WARPS_PER_BLOCK = BLOCK_SIZE / this->local_backend_.GPU_wrap;


    const unsigned int num_units  = this->get_nnz() / this->local_backend_.GPU_wrap; 
    const unsigned int num_warps  = std::min(num_units, WARPS_PER_BLOCK * MAX_BLOCKS);
    const unsigned int num_blocks = (num_warps + (WARPS_PER_BLOCK-1)) / WARPS_PER_BLOCK; // (N + (granularity - 1)) / granularity
    const unsigned int num_iters  = (num_units +  (num_warps-1)) / num_warps;
    
    const unsigned int interval_size = this->local_backend_.GPU_wrap * num_iters;

    const int tail = num_units * this->local_backend_.GPU_wrap; // do the last few nonzeros separately (fewer than this->local_backend_.GPU_wrap elements)

    const unsigned int active_warps = (interval_size == 0) ? 0 : ((tail + (interval_size-1))/interval_size);

    int *temp_rows = NULL;
    ValueType *temp_vals = NULL;

    allocate_gpu(active_warps, &temp_rows);
    allocate_gpu(active_warps, &temp_vals);

    //  LOG_INFO("active_warps = " << active_warps);
    //  LOG_INFO("tail =" << tail);
    //  LOG_INFO("interval_size =" << interval_size);
    //  LOG_INFO("num_iters =" << num_iters);
    //  LOG_INFO("num_blocks =" << num_blocks);
    //  LOG_INFO("num_warps =" << num_warps);
    //  LOG_INFO("num_units =" << num_units);
    //  LOG_INFO("WARPS_PER_BLOCK =" << WARPS_PER_BLOCK);
    //  LOG_INFO("MAX_BLOCKS =" << MAX_BLOCKS);
    //  LOG_INFO("BLOCK_SIZE =" << BLOCK_SIZE);
    //  LOG_INFO("WARP_SIZE =" << WARP_SIZE);
    //  LOG_INFO("WARP_SIZE =" << this->local_backend_.GPU_wrap);

    // TODO 
    // BLOCK_SIZE == 256 
    // WARP_SIZE == 32
    kernel_spmv_coo_flat<int, ValueType, 256, 32> <<<num_blocks, BLOCK_SIZE>>>
      (tail, interval_size, 
       this->mat_.row, this->mat_.col, this->mat_.val, 
       ValueType(1.0),
       cast_in->vec_, cast_out->vec_,
       temp_rows, temp_vals);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    // TODO 
    // BLOCK_SIZE == 256 
    kernel_spmv_coo_reduce_update<int, ValueType, 256> <<<1, BLOCK_SIZE>>>
      (active_warps, temp_rows, temp_vals, cast_out->vec_);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    kernel_spmv_coo_serial<int, ValueType> <<<1,1>>>
      (this->get_nnz() - tail, 
       this->mat_.row + tail, 
       this->mat_.col + tail, 
       this->mat_.val + tail, 
       ValueType(1.0),
       cast_in->vec_, cast_out->vec_);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    free_gpu(&temp_rows);
    free_gpu(&temp_vals);
    
  }
  
}


template <typename ValueType>
void GPUAcceleratorMatrixCOO<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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

    // do not support super small matrices
    assert(this->get_nnz() > this->local_backend_.GPU_wrap); 

    // ----------------------------------------------------------
    // Modified and adopted from CUSP 0.3.1, 
    // http://code.google.com/p/cusp-library/
    // NVIDIA, APACHE LICENSE 2.0
    // ----------------------------------------------------------
    // see __spmv_coo_flat(...)
    // ----------------------------------------------------------
    // CHANGELOG
    // - adopted interface
    // ----------------------------------------------------------  


    const unsigned int BLOCK_SIZE = this->local_backend_.GPU_block_size;
    //    const unsigned int MAX_BLOCKS = this->local_backend_.GPU_max_blocks;

    const unsigned int MAX_BLOCKS = 32; //  cusp::detail::device::arch::max_active_blocks(spmv_coo_flat_kernel<IndexType, ValueType, BLOCK_SIZE, UseCache>, BLOCK_SIZE, (size_t) 0);

    const unsigned int WARPS_PER_BLOCK = BLOCK_SIZE / this->local_backend_.GPU_wrap;


    const unsigned int num_units  = this->get_nnz() / this->local_backend_.GPU_wrap; 
    const unsigned int num_warps  = std::min(num_units, WARPS_PER_BLOCK * MAX_BLOCKS);
    const unsigned int num_blocks = (num_warps + (WARPS_PER_BLOCK-1)) / WARPS_PER_BLOCK; // (N + (granularity - 1)) / granularity
    const unsigned int num_iters  = (num_units +  (num_warps-1)) / num_warps;
    
    const unsigned int interval_size = this->local_backend_.GPU_wrap * num_iters;

    const int tail = num_units * this->local_backend_.GPU_wrap; // do the last few nonzeros separately (fewer than this->local_backend_.GPU_wrap elements)

    const unsigned int active_warps = (interval_size == 0) ? 0 : ((tail + (interval_size-1))/interval_size);

    int *temp_rows = NULL;
    ValueType *temp_vals = NULL;

    allocate_gpu(active_warps, &temp_rows);
    allocate_gpu(active_warps, &temp_vals);

    
    // TODO 
    // BLOCK_SIZE == 256 
    // WARP_SIZE == 32
    kernel_spmv_coo_flat<int, ValueType, 256, 32> <<<num_blocks, BLOCK_SIZE>>>
      (tail, interval_size, 
       this->mat_.row, this->mat_.col, this->mat_.val, 
       scalar,
       cast_in->vec_, cast_out->vec_,
       temp_rows, temp_vals);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    // TODO 
    // BLOCK_SIZE == 256 
    kernel_spmv_coo_reduce_update<int, ValueType, 256> <<<1, BLOCK_SIZE>>>
      (active_warps, temp_rows, temp_vals, cast_out->vec_);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    
    kernel_spmv_coo_serial<int, ValueType> <<<1,1>>>
      (this->get_nnz() - tail, 
       this->mat_.row + tail, 
       this->mat_.col + tail, 
       this->mat_.val + tail, 
       scalar,
       cast_in->vec_, cast_out->vec_);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);     

    free_gpu(&temp_rows);
    free_gpu(&temp_vals);

  }
 

}


template <typename ValueType>
bool GPUAcceleratorMatrixCOO<ValueType>::Permute(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);

  // symmetric permutation only
  assert(permutation.get_size() == this->get_nrow());
  assert(permutation.get_size() == this->get_ncol());

  if (this->get_nnz() > 0) {

    const GPUAcceleratorVector<int> *cast_perm = dynamic_cast<const GPUAcceleratorVector<int>*> (&permutation) ;   
    assert(cast_perm != NULL);
    
    GPUAcceleratorMatrixCOO<ValueType> src(this->local_backend_);       
    src.AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol());
    src.CopyFrom(*this);

    int nnz = this->get_nnz();
    int s = nnz;
    int k = (nnz/this->local_backend_.GPU_block_size)/this->local_backend_.GPU_max_threads + 1;
    if (k > 1) s = nnz / k;

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(s / this->local_backend_.GPU_block_size + 1);
    
    kernel_coo_permute<ValueType, int> <<<GridSize, BlockSize>>> (nnz,
                                                                  src.mat_.row, src.mat_.col,
                                                                  cast_perm->vec_,
                                                                  this->mat_.row, this->mat_.col);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      

  }

  return true;
}


template <typename ValueType>
bool GPUAcceleratorMatrixCOO<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);

  // symmetric permutation only
  assert(permutation.get_size() == this->get_nrow());
  assert(permutation.get_size() == this->get_ncol());

  if (this->get_nnz() > 0) {

    const GPUAcceleratorVector<int> *cast_perm = dynamic_cast<const GPUAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);
    
    int *pb = NULL;
    allocate_gpu(this->get_nrow(), &pb);

    int n = this->get_nrow();
    dim3 BlockSize1(this->local_backend_.GPU_block_size);
    dim3 GridSize1(n / this->local_backend_.GPU_block_size + 1);
    
    kernel_reverse_index<int> <<<GridSize1, BlockSize1>>> (n,
                                                           cast_perm->vec_,
                                                           pb);
    

    GPUAcceleratorMatrixCOO<ValueType> src(this->local_backend_);       
    src.AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol());
    src.CopyFrom(*this);

    int nnz = this->get_nnz();
    int s = nnz;
    int k = (nnz/this->local_backend_.GPU_block_size)/this->local_backend_.GPU_max_threads + 1;
    if (k > 1) s = nnz / k;

    dim3 BlockSize2(this->local_backend_.GPU_block_size);
    dim3 GridSize2(s / this->local_backend_.GPU_block_size + 1);
    
    kernel_coo_permute<ValueType, int> <<<GridSize2, BlockSize2>>> (nnz,
                                                                    src.mat_.row, src.mat_.col,
                                                                    pb,
                                                                    this->mat_.row, this->mat_.col);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      

    free_gpu(&pb);
  }

  return true;

}


template class GPUAcceleratorMatrixCOO<double>;
template class GPUAcceleratorMatrixCOO<float>;

}
