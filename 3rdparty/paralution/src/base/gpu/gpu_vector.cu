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


#include "gpu_vector.hpp"
#include "../base_vector.hpp"
#include "../host/host_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../../utils/math_functions.hpp"
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"
#include "cuda_kernels_vector.hpp"
#include "gpu_allocate_free.hpp"

#include <cuda.h>
#include <cublas_v2.h>
#include <assert.h>


namespace paralution {

template <typename ValueType>
GPUAcceleratorVector<ValueType>::GPUAcceleratorVector() {

  // no default constructors
    LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
GPUAcceleratorVector<ValueType>::GPUAcceleratorVector(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "GPUAcceleratorVector::GPUAcceleratorVector()",
            "constructor with local_backend");

  this->vec_ = NULL;
  this->set_backend(local_backend); 

  CHECK_CUDA_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
GPUAcceleratorVector<ValueType>::~GPUAcceleratorVector() {

  LOG_DEBUG(this, "GPUAcceleratorVector::~GPUAcceleratorVector()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::info(void) const {

  LOG_INFO("GPUAcceleratorVector<ValueType>");

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::Allocate(const int n) {

  assert(n >= 0);

  if (this->get_size() >0)
    this->Clear();

  if (n > 0) {

    allocate_gpu(n, &this->vec_);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    n, this->vec_);

    this->size_ = n;
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::SetDataPtr(ValueType **ptr, const int size) {

  assert(*ptr != NULL);
  assert(size > 0);

  cudaDeviceSynchronize();

  this->vec_ = *ptr;
  this->size_ = size;

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::LeaveDataPtr(ValueType **ptr) {

  assert(this->get_size() > 0);

  cudaDeviceSynchronize();
  *ptr = this->vec_;
  this->vec_ = NULL;

  this->size_ = 0 ;

}


template <typename ValueType>
void GPUAcceleratorVector<ValueType>::Clear(void) {
  
  if (this->get_size() >0) {

    free_gpu(&this->vec_);

    this->size_ = 0 ;

  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFromHost(const HostVector<ValueType> &src) {

  // CPU to GPU copy
  const HostVector<ValueType> *cast_vec;
  if ((cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src)) != NULL) {

  if (this->get_size() == 0)
    this->Allocate(cast_vec->get_size());
    
    assert(cast_vec->get_size() == this->get_size());

    if (this->get_size() >0) {
      
      
      cublasStatus_t stat_t;
      stat_t = cublasSetVector(this->get_size(), sizeof(ValueType),
                               cast_vec->vec_, // src
                               1,
                               this->vec_, // dst
                               1);
      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
    }

  } else {
    
    LOG_INFO("Error unsupported GPU vector type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}



template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyToHost(HostVector<ValueType> *dst) const {

  // GPU to CPU copy
  HostVector<ValueType> *cast_vec;
  if ((cast_vec = dynamic_cast<HostVector<ValueType>*> (dst)) != NULL) {

  if (cast_vec->get_size() == 0)
    cast_vec->Allocate(this->get_size());  
    
    assert(cast_vec->get_size() == this->get_size());

    if (this->get_size() >0) {

      cublasStatus_t stat_t;
      stat_t = cublasGetVector(this->get_size(), sizeof(ValueType),
                               this->vec_, // src
                               1,
                               cast_vec->vec_, // dst
                               1);
      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
    }

  } else {
    
    LOG_INFO("Error unsupported GPU vector type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

  
}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFromHostAsync(const HostVector<ValueType> &src) {

  // CPU to GPU copy
  const HostVector<ValueType> *cast_vec;
  if ((cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src)) != NULL) {

  if (this->get_size() == 0)
    this->Allocate(cast_vec->get_size());
    
    assert(cast_vec->get_size() == this->get_size());

    if (this->get_size() >0) {

      cudaMemcpyAsync(this->vec_,     // dst
                      cast_vec->vec_, // src
                      this->get_size()*sizeof(ValueType), // size
                      cudaMemcpyHostToDevice);
      CHECK_CUDA_ERROR(__FILE__, __LINE__);     
      
    }

  } else {
    
    LOG_INFO("Error unsupported GPU vector type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}



template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyToHostAsync(HostVector<ValueType> *dst) const {

  // GPU to CPU copy
  HostVector<ValueType> *cast_vec;
  if ((cast_vec = dynamic_cast<HostVector<ValueType>*> (dst)) != NULL) {

  if (cast_vec->get_size() == 0)
    cast_vec->Allocate(this->get_size());  
    
    assert(cast_vec->get_size() == this->get_size());

    if (this->get_size() >0) {

      cudaMemcpyAsync(cast_vec->vec_,  // dst
                      this->vec_,      // src
                      this->get_size()*sizeof(ValueType), // size
                      cudaMemcpyDeviceToHost);
      CHECK_CUDA_ERROR(__FILE__, __LINE__);     


    }

  } else {
    
    LOG_INFO("Error unsupported GPU vector type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

  
}


template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFrom(const BaseVector<ValueType> &src) {

  const GPUAcceleratorVector<ValueType> *gpu_cast_vec;
  const HostVector<ValueType> *host_cast_vec;


    // GPU to GPU copy
    if ((gpu_cast_vec = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&src)) != NULL) {

      if (this->get_size() == 0)
        this->Allocate(gpu_cast_vec->get_size());

      assert(gpu_cast_vec->get_size() == this->get_size());

      if (this != gpu_cast_vec)  {  

        if (this->get_size() >0) {

          cudaMemcpy(this->vec_,         // dst
                     gpu_cast_vec->vec_, // src
                     this->get_size()*sizeof(ValueType), // size
                     cudaMemcpyDeviceToDevice);
          CHECK_CUDA_ERROR(__FILE__, __LINE__);     
        }

      }

    } else {
      
      //GPU to CPU copy
      if ((host_cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src)) != NULL) {
        

        this->CopyFromHost(*host_cast_vec);
        
      
      } else {

        LOG_INFO("Error unsupported GPU vector type");
        this->info();
        src.info();
        FATAL_ERROR(__FILE__, __LINE__);
        
      }
      
    }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFromAsync(const BaseVector<ValueType> &src) {

  const GPUAcceleratorVector<ValueType> *gpu_cast_vec;
  const HostVector<ValueType> *host_cast_vec;


    // GPU to GPU copy
    if ((gpu_cast_vec = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&src)) != NULL) {

      if (this->get_size() == 0)
        this->Allocate(gpu_cast_vec->get_size());

      assert(gpu_cast_vec->get_size() == this->get_size());

      if (this != gpu_cast_vec)  {  

        if (this->get_size() >0) {

          cudaMemcpy(this->vec_,         // dst
                     gpu_cast_vec->vec_, // src
                     this->get_size()*sizeof(ValueType), // size
                     cudaMemcpyDeviceToDevice);
          CHECK_CUDA_ERROR(__FILE__, __LINE__);     
        }

      }

    } else {
      
      //GPU to CPU copy
      if ((host_cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src)) != NULL) {
        

        this->CopyFromHostAsync(*host_cast_vec);
        
      
      } else {

        LOG_INFO("Error unsupported GPU vector type");
        this->info();
        src.info();
        FATAL_ERROR(__FILE__, __LINE__);
        
      }
      
    }

}


template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFrom(const BaseVector<ValueType> &src,
                                               const int src_offset,
                                               const int dst_offset,
                                               const int size) {

  assert(&src != this);
  assert(this->get_size() > 0);
  assert(src.  get_size() > 0);
  assert(size > 0);

  assert(src_offset + size <= src.get_size());
  assert(dst_offset + size <= this->get_size());

  const GPUAcceleratorVector<ValueType> *cast_src = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&src);
  assert(cast_src != NULL);

  dim3 BlockSize(this->local_backend_.GPU_block_size);
  dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);

  kernel_copy_offset_from<ValueType, int> <<<GridSize, BlockSize>>> (size, src_offset, dst_offset,
                                                                     cast_src->vec_, this->vec_);

  CHECK_CUDA_ERROR(__FILE__, __LINE__);      

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyTo(BaseVector<ValueType> *dst) const{

  GPUAcceleratorVector<ValueType> *gpu_cast_vec;
  HostVector<ValueType> *host_cast_vec;

    // GPU to GPU copy
    if ((gpu_cast_vec = dynamic_cast<GPUAcceleratorVector<ValueType>*> (dst)) != NULL) {

      if (gpu_cast_vec->get_size() == 0)
        gpu_cast_vec->Allocate(this->get_size());

      assert(gpu_cast_vec->get_size() == this->get_size());

      if (this != gpu_cast_vec)  {  

        if (this->get_size() >0) {

          cudaMemcpy(gpu_cast_vec->vec_, // dst
                     this->vec_,         // src
                     this->get_size()*sizeof(ValueType), // size
                     cudaMemcpyDeviceToDevice);
          CHECK_CUDA_ERROR(__FILE__, __LINE__);      
        }
      }

    } else {
      
      //GPU to CPU copy
      if ((host_cast_vec = dynamic_cast<HostVector<ValueType>*> (dst)) != NULL) {
        

        this->CopyToHost(host_cast_vec);
        
      
      } else {

        LOG_INFO("Error unsupported GPU vector type");
        this->info();
        dst->info();
        FATAL_ERROR(__FILE__, __LINE__);
        
      }
      
    }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyToAsync(BaseVector<ValueType> *dst) const{

  GPUAcceleratorVector<ValueType> *gpu_cast_vec;
  HostVector<ValueType> *host_cast_vec;

    // GPU to GPU copy
    if ((gpu_cast_vec = dynamic_cast<GPUAcceleratorVector<ValueType>*> (dst)) != NULL) {

      if (gpu_cast_vec->get_size() == 0)
        gpu_cast_vec->Allocate(this->get_size());

      assert(gpu_cast_vec->get_size() == this->get_size());

      if (this != gpu_cast_vec)  {  

        if (this->get_size() >0) {

          cudaMemcpy(gpu_cast_vec->vec_, // dst
                     this->vec_,         // src
                     this->get_size()*sizeof(ValueType), // size
                     cudaMemcpyDeviceToDevice);
          CHECK_CUDA_ERROR(__FILE__, __LINE__);      
        }
      }

    } else {
      
      //GPU to CPU copy
      if ((host_cast_vec = dynamic_cast<HostVector<ValueType>*> (dst)) != NULL) {
        

        this->CopyToHostAsync(host_cast_vec);
        
      
      } else {

        LOG_INFO("Error unsupported GPU vector type");
        this->info();
        dst->info();
        FATAL_ERROR(__FILE__, __LINE__);
        
      }
      
    }

}


template <typename ValueType>
void GPUAcceleratorVector<ValueType>::Zeros(void) {

  if (this->get_size() > 0) {

    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    this->get_size(), this->vec_);
    
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::Ones(void) {

  if (this->get_size() > 0)
    set_to_one_gpu(this->local_backend_.GPU_block_size, 
                   this->local_backend_.GPU_max_threads,
                   this->get_size(), this->vec_);

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::SetValues(const ValueType val) {

  LOG_INFO("GPUAcceleratorVector::SetValues NYI");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <>
void GPUAcceleratorVector<double>::AddScale(const BaseVector<double> &x, const double alpha) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const GPUAcceleratorVector<double> *cast_x = dynamic_cast<const GPUAcceleratorVector<double>*> (&x);
    assert(cast_x != NULL);
    
    cublasStatus_t stat_t;
    
    stat_t = cublasDaxpy(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), 
                         &alpha, 
                         cast_x->vec_, 1,
                         this->vec_, 1);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }

}

template <>
void GPUAcceleratorVector<float>::AddScale(const BaseVector<float> &x, const float alpha) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const GPUAcceleratorVector<float> *cast_x = dynamic_cast<const GPUAcceleratorVector<float>*> (&x);
    assert(cast_x != NULL);
    
    cublasStatus_t stat_t;
    
    stat_t = cublasSaxpy(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), 
                         &alpha, 
                         cast_x->vec_, 1,
                         this->vec_, 1);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }

}

template <>
void GPUAcceleratorVector<int>::AddScale(const BaseVector<int> &x, const int alpha) {

  LOG_INFO("No int CUBLAS axpy function");
  FATAL_ERROR(__FILE__, __LINE__);
 
}



template <typename ValueType>
void GPUAcceleratorVector<ValueType>::ScaleAdd(const ValueType alpha, const BaseVector<ValueType> &x) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const GPUAcceleratorVector<ValueType> *cast_x = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);

    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    kernel_scaleadd<ValueType, int> <<<GridSize, BlockSize>>> (size, alpha, cast_x->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      

  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const GPUAcceleratorVector<ValueType> *cast_x = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    kernel_scaleaddscale<ValueType, int> <<<GridSize, BlockSize>>> (size, alpha, beta, cast_x->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta,
                                          const int src_offset, const int dst_offset,const int size) {

  if (this->get_size() > 0) {

    assert(this->get_size()  == x.get_size());
    assert(src_offset + size <= x.get_size());
    assert(dst_offset + size <= this->get_size());

    const GPUAcceleratorVector<ValueType> *cast_x = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    kernel_scaleaddscale_offset<ValueType, int> <<<GridSize, BlockSize>>> (size, src_offset, dst_offset,
                                                                           alpha, beta, cast_x->vec_, this->vec_);

    CHECK_CUDA_ERROR(__FILE__, __LINE__);

  }

}


template <typename ValueType>
void GPUAcceleratorVector<ValueType>::ScaleAdd2(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta, const BaseVector<ValueType> &y, const ValueType gamma) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    assert(this->get_size() == y.get_size());
    
    const GPUAcceleratorVector<ValueType> *cast_x = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&x);
    const GPUAcceleratorVector<ValueType> *cast_y = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&y);
    assert(cast_x != NULL);
    assert(cast_y != NULL);
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    kernel_scaleadd2<ValueType, int> <<<GridSize, BlockSize>>> (size, alpha, beta, gamma, cast_x->vec_, cast_y->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <>
void GPUAcceleratorVector<double>::Scale(const double alpha) {

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    stat_t = cublasDscal(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), &alpha, 
                         this->vec_, 1);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
  }

}

template <>
void GPUAcceleratorVector<float>::Scale(const float alpha) {

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    stat_t = cublasSscal(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), &alpha, 
                         this->vec_, 1);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
  }

}

template <>
void GPUAcceleratorVector<int>::Scale(const int alpha) {


  LOG_INFO("No int CUBLAS scale function");
  FATAL_ERROR(__FILE__, __LINE__);  

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::PartialSum(const BaseVector<ValueType> &x) {

  LOG_INFO("GPUAcceleratorVector::PartialSum() NYI");
  FATAL_ERROR(__FILE__, __LINE__); 

}

template <>
double GPUAcceleratorVector<double>::Dot(const BaseVector<double> &x) const {

  assert(this->get_size() == x.get_size());

  const GPUAcceleratorVector<double> *cast_x = dynamic_cast<const GPUAcceleratorVector<double>*> (&x);
  assert(cast_x != NULL);

  double res = 0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    stat_t = cublasDdot(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                        this->get_size(), 
                        this->vec_, 1, 
                        cast_x->vec_, 1, &res);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }
  return res;
}

template <>
float GPUAcceleratorVector<float>::Dot(const BaseVector<float> &x) const {

  assert(this->get_size() == x.get_size());

  const GPUAcceleratorVector<float> *cast_x = dynamic_cast<const GPUAcceleratorVector<float>*> (&x);
  assert(cast_x != NULL);

  float res = 0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    
    stat_t = cublasSdot(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                        this->get_size(), 
                        this->vec_, 1, 
                        cast_x->vec_, 1, &res);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
  }

  return res;
}

template <>
int GPUAcceleratorVector<int>::Dot(const BaseVector<int> &x) const {

  LOG_INFO("No int CUBLAS dot function");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <>
double GPUAcceleratorVector<double>::Norm(void) const {

  double res = 0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    
    stat_t = cublasDnrm2(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), 
                         this->vec_, 1, &res);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
  }

  return res;

}

template <>
float GPUAcceleratorVector<float>::Norm(void) const {

  float res = 0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    
    stat_t = cublasSnrm2(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), 
                         this->vec_, 1, &res);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);
  }

  return res;

}

template <>
int GPUAcceleratorVector<int>::Norm(void) const {

  LOG_INFO("What is int GPUAcceleratorVector<ValueType>::Norm(void) const?");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <>
double GPUAcceleratorVector<double>::Reduce(void) const {

  double res = 0.0;

  if (this->get_size() > 0) {

    double *d_buffer = NULL;
    double *h_buffer = NULL;
    int GROUP_SIZE;
    int LOCAL_SIZE;
    int FinalReduceSize;

    allocate_gpu<double>(this->local_backend_.GPU_wrap * 4, &d_buffer);

    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(this->local_backend_.GPU_wrap * 4);

    GROUP_SIZE = ( size_t( ( size_t( size / ( this->local_backend_.GPU_wrap * 4 ) ) + 1 ) 
                 / this->local_backend_.GPU_block_size ) + 1 ) * this->local_backend_.GPU_block_size;
    LOCAL_SIZE = GROUP_SIZE / this->local_backend_.GPU_block_size;
    

    kernel_reduce<double, int, 256> <<<GridSize, BlockSize>>> (size,
                                                               this->vec_,
                                                               d_buffer,
                                                               GROUP_SIZE,
                                                               LOCAL_SIZE);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    FinalReduceSize = this->local_backend_.GPU_wrap * 4;
    allocate_host(FinalReduceSize, &h_buffer);

    cudaMemcpy(h_buffer, // dst
               d_buffer, // src
               FinalReduceSize*sizeof(double), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    free_gpu<double>(&d_buffer);

    for ( int i=0; i<FinalReduceSize; ++i ) {
      res += h_buffer[i];
    }

    free_host(&h_buffer);

  }

  return res;

}

template <>
float GPUAcceleratorVector<float>::Reduce(void) const {

  float res = 0.0;

  if (this->get_size() > 0) {

    float *d_buffer = NULL;
    float *h_buffer = NULL;
    int GROUP_SIZE;
    int LOCAL_SIZE;
    int FinalReduceSize;

    allocate_gpu<float>(this->local_backend_.GPU_wrap * 4, &d_buffer);

    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(this->local_backend_.GPU_wrap * 4);

    GROUP_SIZE = ( size_t( ( size_t( size / ( this->local_backend_.GPU_wrap * 4 ) ) + 1 ) 
                 / this->local_backend_.GPU_block_size ) + 1 ) * this->local_backend_.GPU_block_size;
    LOCAL_SIZE = GROUP_SIZE / this->local_backend_.GPU_block_size;
    
    kernel_reduce<float, int, 256> <<<GridSize, BlockSize>>> (size,
                                                              this->vec_,
                                                              d_buffer,
                                                              GROUP_SIZE,
                                                              LOCAL_SIZE);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    FinalReduceSize = this->local_backend_.GPU_wrap * 4;
    allocate_host(FinalReduceSize, &h_buffer);

    cudaMemcpy(h_buffer,         // dst
               d_buffer, // src
               FinalReduceSize*sizeof(float), // size
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__); 

    free_gpu<float>(&d_buffer);

    for ( int i=0; i<FinalReduceSize; ++i ) {
      res += h_buffer[i];
    }

    free_host(&h_buffer);

  }

  return res;

}

template <>
int GPUAcceleratorVector<int>::Reduce(void) const {

  LOG_INFO("Reduce<int> not implemented");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <>
double GPUAcceleratorVector<double>::Asum(void) const {

  double res = 0.0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    stat_t = cublasDasum(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), 
                         this->vec_, 1, 
                         &res);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }

  return res;

}

template <>
float GPUAcceleratorVector<float>::Asum(void) const {

  float res = 0.0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;
    stat_t = cublasSasum(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), 
                         this->get_size(), 
                         this->vec_, 1, 
                         &res);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }

  return res;

}

template <>
int GPUAcceleratorVector<int>::Asum(void) const {

  LOG_INFO("Asum<int> not implemented");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <>
int GPUAcceleratorVector<float>::Amax(float &value) const {

  int index = 0;
  value = 0.0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;

    stat_t = cublasIsamax(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle),
                          this->get_size(),
                          this->vec_, 1, &index);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    // cublas returns 1-based indexing
    --index;

    cudaMemcpy(&value,
               this->vec_+index,
               sizeof(float),
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

  }

  value = paralution_abs(value);
  return index;

}

template <>
int GPUAcceleratorVector<double>::Amax(double &value) const {

  int index = 0;
  value = 0.0;

  if (this->get_size() > 0) {

    cublasStatus_t stat_t;

    stat_t = cublasIdamax(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle),
                          this->get_size(),
                          this->vec_, 1, &index);
    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    // cublas returns 1-based indexing
    --index;

    cudaMemcpy(&value,
               this->vec_+index,
               sizeof(double),
               cudaMemcpyDeviceToHost);
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

  }

  value = paralution_abs(value);
  return index;

}

template <>
int GPUAcceleratorVector<int>::Amax(int &value) const {

  LOG_INFO("Amax<int> not implemented");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::PointWiseMult(const BaseVector<ValueType> &x) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const GPUAcceleratorVector<ValueType> *cast_x = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    kernel_pointwisemult<ValueType, int> <<<GridSize, BlockSize>>> (size, cast_x->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::PointWiseMult(const BaseVector<ValueType> &x, const BaseVector<ValueType> &y) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    assert(this->get_size() == y.get_size());
    
    const GPUAcceleratorVector<ValueType> *cast_x = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&x);
    const GPUAcceleratorVector<ValueType> *cast_y = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&y);
    assert(cast_x != NULL);
    assert(cast_y != NULL);
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    kernel_pointwisemult2<ValueType, int> <<<GridSize, BlockSize>>> (size, cast_x->vec_, cast_y->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::Permute(const BaseVector<int> &permutation) {

  if (this->get_size() > 0) {

    assert(&permutation != NULL);
    assert(this->get_size() == permutation.get_size());
    
    const GPUAcceleratorVector<int> *cast_perm = dynamic_cast<const GPUAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);
    
    GPUAcceleratorVector<ValueType> vec_tmp(this->local_backend_);     
    vec_tmp.Allocate(this->get_size());
    vec_tmp.CopyFrom(*this);
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    //    this->vec_[ cast_perm->vec_[i] ] = vec_tmp.vec_[i];  
    kernel_permute<ValueType, int> <<<GridSize, BlockSize>>> (size, cast_perm->vec_, vec_tmp.vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {

  if (this->get_size() > 0) {

    assert(&permutation != NULL);
    assert(this->get_size() == permutation.get_size());
    
    const GPUAcceleratorVector<int> *cast_perm = dynamic_cast<const GPUAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);
    
    GPUAcceleratorVector<ValueType> vec_tmp(this->local_backend_);   
    vec_tmp.Allocate(this->get_size());
    vec_tmp.CopyFrom(*this);
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    //    this->vec_[i] = vec_tmp.vec_[ cast_perm->vec_[i] ];
    kernel_permute_backward<ValueType, int> <<<GridSize, BlockSize>>> (size, cast_perm->vec_, vec_tmp.vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFromPermute(const BaseVector<ValueType> &src,
                                                      const BaseVector<int> &permutation) { 

  if (this->get_size() > 0) {

    assert(this != &src);
    
    const GPUAcceleratorVector<ValueType> *cast_vec = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&src);
    const GPUAcceleratorVector<int> *cast_perm      = dynamic_cast<const GPUAcceleratorVector<int>*> (&permutation) ; 
    assert(cast_perm != NULL);
    assert(cast_vec  != NULL);
    
    assert(cast_vec ->get_size() == this->get_size());
    assert(cast_perm->get_size() == this->get_size());
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    //    this->vec_[ cast_perm->vec_[i] ] = cast_vec->vec_[i];
    kernel_permute<ValueType, int> <<<GridSize, BlockSize>>> (size, cast_perm->vec_, cast_vec->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}

template <typename ValueType>
void GPUAcceleratorVector<ValueType>::CopyFromPermuteBackward(const BaseVector<ValueType> &src,
                                                              const BaseVector<int> &permutation) {

  if (this->get_size() > 0) {

    assert(this != &src);
    
    const GPUAcceleratorVector<ValueType> *cast_vec = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&src);
    const GPUAcceleratorVector<int> *cast_perm      = dynamic_cast<const GPUAcceleratorVector<int>*> (&permutation) ; 
    assert(cast_perm != NULL);
    assert(cast_vec  != NULL);
    
    assert(cast_vec ->get_size() == this->get_size());
    assert(cast_perm->get_size() == this->get_size());
    
    
    int size = this->get_size();
    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(size / this->local_backend_.GPU_block_size + 1);
    
    //    this->vec_[i] = cast_vec->vec_[ cast_perm->vec_[i] ];
    kernel_permute_backward<ValueType, int> <<<GridSize, BlockSize>>> (size, cast_perm->vec_, cast_vec->vec_, this->vec_);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
  }

}


template class GPUAcceleratorVector<double>;
template class GPUAcceleratorVector<float>;

template class GPUAcceleratorVector<int>;

}

