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

#include "gpu_matrix_csr.hpp"
#include "gpu_matrix_coo.hpp"
#include "gpu_matrix_dia.hpp"
#include "gpu_matrix_ell.hpp"
#include "gpu_matrix_hyb.hpp"
#include "gpu_matrix_mcsr.hpp"
#include "gpu_matrix_bcsr.hpp"
#include "gpu_matrix_dense.hpp"
#include "gpu_vector.hpp"
#include "../host/host_matrix_dia.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"
#include "cuda_kernels_dia.hpp"
#include "cuda_kernels_vector.hpp"
#include "gpu_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <cuda.h>
#include <cusparse_v2.h>


namespace paralution {

template <typename ValueType>
GPUAcceleratorMatrixDIA<ValueType>::GPUAcceleratorMatrixDIA() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
GPUAcceleratorMatrixDIA<ValueType>::GPUAcceleratorMatrixDIA(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.val = NULL;
  this->mat_.offset = NULL;  
  this->mat_.num_diag = 0 ;
  this->set_backend(local_backend); 

  CHECK_CUDA_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
GPUAcceleratorMatrixDIA<ValueType>::~GPUAcceleratorMatrixDIA() {

  this->Clear();

}

template <typename ValueType>
void GPUAcceleratorMatrixDIA<ValueType>::info(void) const {

  LOG_INFO("GPUAcceleratorMatrixDIA<ValueType> diag=" << this->get_ndiag() << " nnz=" << this->get_nnz() );

}

template <typename ValueType>
void GPUAcceleratorMatrixDIA<ValueType>::AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    assert(ndiag > 0);


    allocate_gpu(nnz, &this->mat_.val);
    allocate_gpu(ndiag, &this->mat_.offset);
 
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nnz, mat_.val);
    
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    ndiag, mat_.offset);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;
    this->mat_.num_diag = ndiag;

  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDIA<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_gpu(&this->mat_.val);
    free_gpu(&this->mat_.offset);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;
    this->mat_.num_diag = 0 ;

  }


}

template <typename ValueType>
void GPUAcceleratorMatrixDIA<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixDIA<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDIA(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), cast_mat->get_ndiag());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      cudaMemcpy(this->mat_.offset,     // dst
                 cast_mat->mat_.offset, // src
                 this->get_ndiag()*sizeof(int), // size
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
void GPUAcceleratorMatrixDIA<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixDIA<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixDIA<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateDIA(this->get_nnz(), this->get_nrow(), this->get_ncol(), this->get_ndiag());

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

      cudaMemcpy(cast_mat->mat_.offset, // dst
                 this->mat_.offset,     // src
                 this->get_ndiag()*sizeof(int), // size
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
void GPUAcceleratorMatrixDIA<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixDIA<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixDIA<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDIA(gpu_cast_mat->get_nnz(), gpu_cast_mat->get_nrow(), gpu_cast_mat->get_ncol(), gpu_cast_mat->get_ndiag());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      cudaMemcpy(this->mat_.offset,         // dst
                 gpu_cast_mat->mat_.offset, // src
                 this->get_ndiag()*sizeof(int), // size
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
void GPUAcceleratorMatrixDIA<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixDIA<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixDIA<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateDIA(gpu_cast_mat->get_nnz(), gpu_cast_mat->get_nrow(), gpu_cast_mat->get_ncol(), gpu_cast_mat->get_ndiag());

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) { 

      cudaMemcpy(gpu_cast_mat->mat_.offset, // dst
                 this->mat_.offset,         // src
                 this->get_ndiag()*sizeof(int), // size
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
bool GPUAcceleratorMatrixDIA<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const GPUAcceleratorMatrixDIA<ValueType>   *cast_mat_dia;
  
  if ((cast_mat_dia = dynamic_cast<const GPUAcceleratorMatrixDIA<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_dia);
      return true;

  }

  const GPUAcceleratorMatrixCSR<ValueType>   *cast_mat_csr;
  if ((cast_mat_csr = dynamic_cast<const GPUAcceleratorMatrixCSR<ValueType>*> (&mat)) != NULL) {

    this->Clear();

    int nrow = cast_mat_csr->get_nrow();
    int ncol = cast_mat_csr->get_ncol();
    int *diag_map = NULL;

    // Get diagonal mapping vector
    allocate_gpu<int>(nrow+ncol, &diag_map);

    set_to_zero_gpu(this->local_backend_.GPU_block_size,
                    this->local_backend_.GPU_max_threads,
                    nrow+ncol, diag_map);

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);

    cudaDeviceSynchronize();

    kernel_dia_diag_map<int> <<<GridSize, BlockSize>>> (nrow, cast_mat_csr->mat_.row_offset,
                                                        cast_mat_csr->mat_.col, diag_map);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    // Reduction to obtain number of occupied diagonals
    int *d_buffer = NULL;
    int *h_buffer = NULL;
    int GROUP_SIZE;
    int LOCAL_SIZE;
    int FinalReduceSize;

    allocate_gpu<int>(this->local_backend_.GPU_wrap * 4, &d_buffer);

    dim3 GridSize2(this->local_backend_.GPU_wrap * 4);

    GROUP_SIZE = ( size_t( ( size_t( nrow+ncol / ( this->local_backend_.GPU_wrap * 4 ) ) + 1 ) 
                 / this->local_backend_.GPU_block_size ) + 1 ) * this->local_backend_.GPU_block_size;
    LOCAL_SIZE = GROUP_SIZE / this->local_backend_.GPU_block_size;

    cudaDeviceSynchronize();

    kernel_reduce<int, int, 256> <<<GridSize2, BlockSize>>> (nrow+ncol, diag_map, d_buffer, GROUP_SIZE, LOCAL_SIZE);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    FinalReduceSize = this->local_backend_.GPU_wrap * 4;
    allocate_host(FinalReduceSize, &h_buffer);

    cudaMemcpy(h_buffer, // dst
               d_buffer, // src
               FinalReduceSize*sizeof(int), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    free_gpu<int>(&d_buffer);

    int num_diag = 0;
    for ( int i=0; i<FinalReduceSize; ++i )
      num_diag += h_buffer[i];

    free_host(&h_buffer);

    int nnz_dia;
    if (nrow < ncol)
      nnz_dia = ncol * num_diag;
    else
      nnz_dia = nrow * num_diag;

    // Allocate DIA structure
    this->AllocateDIA(nnz_dia, nrow, ncol, num_diag);

    set_to_zero_gpu(this->local_backend_.GPU_block_size,
                    this->local_backend_.GPU_max_threads,
                    nnz_dia, this->mat_.val);
    set_to_zero_gpu(this->local_backend_.GPU_block_size,
                    this->local_backend_.GPU_max_threads,
                    num_diag, this->mat_.offset);

    // Fill diagonal offset array
    allocate_gpu<int>(nrow+ncol+1, &d_buffer);

    // TODO currently performing partial sum on host
    allocate_host(nrow+ncol+1, &h_buffer);
    cudaMemcpy(h_buffer+1, // dst
               diag_map, // src
               (nrow+ncol)*sizeof(int), // size
               cudaMemcpyDeviceToHost);

    h_buffer[0] = 0;
    for (int i=2; i<nrow+ncol+1; ++i)
      h_buffer[i] += h_buffer[i-1];

    cudaMemcpy(d_buffer, // dst
               h_buffer, // src
               (nrow+ncol)*sizeof(int), // size
               cudaMemcpyHostToDevice);

    free_host(&h_buffer);
    // end TODO

    dim3 GridSize3((nrow+ncol) / this->local_backend_.GPU_block_size + 1);

    cudaDeviceSynchronize();

    kernel_dia_fill_offset<int> <<<GridSize3, BlockSize>>> (nrow, ncol, diag_map,
                                                            d_buffer, this->mat_.offset);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);
    free_gpu<int>(&d_buffer);

    cudaDeviceSynchronize();

    kernel_dia_convert<ValueType, int> <<<GridSize, BlockSize>>> (nrow, num_diag, cast_mat_csr->mat_.row_offset,
                                                                  cast_mat_csr->mat_.col, cast_mat_csr->mat_.val,
                                                                  diag_map, this->mat_.val);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);
    free_gpu<int>(&diag_map);

    this->nrow_ = cast_mat_csr->get_nrow();
    this->ncol_ = cast_mat_csr->get_ncol();
    this->nnz_  = nnz_dia;
    this->mat_.num_diag = num_diag;

    return true;

  }

  return false;

}

template <typename ValueType>
void GPUAcceleratorMatrixDIA<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    int ncol = this->get_ncol();
    int num_diag = this->get_ndiag();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);

    cudaDeviceSynchronize();
    kernel_dia_spmv<ValueType, int> <<<GridSize, BlockSize>>> (nrow, ncol, num_diag,
                                                               this->mat_.offset, this->mat_.val,
                                                               cast_in->vec_, cast_out->vec_ );
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

  }

}


template <typename ValueType>
void GPUAcceleratorMatrixDIA<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    int ncol = this->get_ncol();
    int num_diag = this->get_ndiag();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);

    cudaDeviceSynchronize();
    kernel_dia_add_spmv<ValueType, int> <<<GridSize, BlockSize>>> (nrow, ncol, num_diag,
                                                                   this->mat_.offset, this->mat_.val,
                                                                   scalar,
                                                                   cast_in->vec_, cast_out->vec_ );
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

  }

}


template class GPUAcceleratorMatrixDIA<double>;
template class GPUAcceleratorMatrixDIA<float>;

}

