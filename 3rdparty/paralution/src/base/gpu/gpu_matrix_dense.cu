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
#include "../host/host_matrix_dense.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"
#include "cuda_kernels_dense.hpp"
#include "gpu_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <cuda.h>


namespace paralution {

template <typename ValueType>
GPUAcceleratorMatrixDENSE<ValueType>::GPUAcceleratorMatrixDENSE() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
GPUAcceleratorMatrixDENSE<ValueType>::GPUAcceleratorMatrixDENSE(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.val = NULL;
  this->set_backend(local_backend); 

  CHECK_CUDA_ERROR(__FILE__, __LINE__);

  FATAL_ERROR(__FILE__, __LINE__);
}


template <typename ValueType>
GPUAcceleratorMatrixDENSE<ValueType>::~GPUAcceleratorMatrixDENSE() {

  this->Clear();

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::info(void) const {

  LOG_INFO("GPUAcceleratorMatrixDENSE<ValueType>");

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::AllocateDENSE(const int nrow, const int ncol) {

  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nrow*ncol > 0) {

    allocate_gpu(nrow*ncol, &this->mat_.val);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nrow*ncol, mat_.val);   

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nrow*ncol;

  }


}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_gpu(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

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
void GPUAcceleratorMatrixDENSE<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixDENSE<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateDENSE(this->get_nrow(), this->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {
      
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
void GPUAcceleratorMatrixDENSE<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixDENSE<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) { 

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
void GPUAcceleratorMatrixDENSE<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixDENSE<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixDENSE<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateDENSE(dst->get_nrow(), dst->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

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
bool GPUAcceleratorMatrixDENSE<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const GPUAcceleratorMatrixDENSE<ValueType>   *cast_mat_dense;
  
  if ((cast_mat_dense = dynamic_cast<const GPUAcceleratorMatrixDENSE<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_dense);
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
void GPUAcceleratorMatrixDENSE<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {
/*
    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const GPUAcceleratorVector<ValueType> *cast_in = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&in) ; 
    GPUAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      GPUAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);
*/
    FATAL_ERROR(__FILE__, __LINE__);    
  }

}


template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                  BaseVector<ValueType> *out) const {
  FATAL_ERROR(__FILE__, __LINE__);
}


template class GPUAcceleratorMatrixDENSE<double>;
template class GPUAcceleratorMatrixDENSE<float>;

}

