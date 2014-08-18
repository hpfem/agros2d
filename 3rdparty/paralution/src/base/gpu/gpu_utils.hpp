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


#ifndef PARALUTION_GPU_GPU_UTILS_HPP_
#define PARALUTION_GPU_GPU_UTILS_HPP_

#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "backend_gpu.hpp"
#include "cuda_kernels_general.hpp"
#include "gpu_allocate_free.hpp"

#include <stdlib.h>

#include <cuda.h>
#include <cublas_v2.h>
#include <cusparse_v2.h>

#define CUBLAS_HANDLE(handle) *static_cast<cublasHandle_t*>(handle)
#define CUSPARSE_HANDLE(handle) *static_cast<cusparseHandle_t*>(handle)

#define CHECK_CUDA_ERROR(file, line) {                                  \
    cudaError_t err_t;                                                  \
    if ((err_t = cudaGetLastError() ) != cudaSuccess) {                 \
      LOG_INFO("Cuda error: " << cudaGetErrorString(err_t));            \
      LOG_INFO("File: " << file << "; line: " << line);                 \
      exit(1);                                                          \
    }                                                                   \
  }   

#define CHECK_CUBLAS_ERROR(stat_t, file, line) {                        \
  if (stat_t  != CUBLAS_STATUS_SUCCESS) {                               \
  LOG_INFO("Cublas error!");                                            \
  if (stat_t == CUBLAS_STATUS_NOT_INITIALIZED)                          \
    LOG_INFO("CUBLAS_STATUS_NOT_INITIALIZED");                          \
  if (stat_t == CUBLAS_STATUS_ALLOC_FAILED)                             \
    LOG_INFO("CUBLAS_STATUS_ALLOC_FAILED");                             \
  if (stat_t == CUBLAS_STATUS_INVALID_VALUE)                            \
    LOG_INFO("CUBLAS_STATUS_INVALID_VALUE");                            \
  if (stat_t == CUBLAS_STATUS_ARCH_MISMATCH)                            \
    LOG_INFO("CUBLAS_STATUS_ARCH_MISMATCH");                            \
  if (stat_t == CUBLAS_STATUS_MAPPING_ERROR)                            \
    LOG_INFO("CUBLAS_STATUS_MAPPING_ERROR");                            \
  if (stat_t == CUBLAS_STATUS_EXECUTION_FAILED)                         \
    LOG_INFO("CUBLAS_STATUS_EXECUTION_FAILED");                         \
  if (stat_t == CUBLAS_STATUS_INTERNAL_ERROR)                           \
    LOG_INFO("CUBLAS_STATUS_INTERNAL_ERROR");                           \
  LOG_INFO("File: " << file << "; line: " << line);                     \
  exit(1);                                                              \
  }                                                                     \
  }   

#define CHECK_CUSPARSE_ERROR(stat_t, file, line) {                      \
  if (stat_t  != CUSPARSE_STATUS_SUCCESS) {                             \
  LOG_INFO("Cusparse error!");                                          \
  if (stat_t == CUSPARSE_STATUS_NOT_INITIALIZED)                        \
    LOG_INFO("CUSPARSE_STATUS_NOT_INITIALIZED");                        \
  if (stat_t == CUSPARSE_STATUS_ALLOC_FAILED)                           \
    LOG_INFO("CUSPARSE_STATUS_ALLOC_FAILED");                           \
  if (stat_t == CUSPARSE_STATUS_INVALID_VALUE)                          \
    LOG_INFO("CUSPARSE_STATUS_INVALID_VALUE");                          \
  if (stat_t == CUSPARSE_STATUS_ARCH_MISMATCH)                          \
    LOG_INFO("CUSPARSE_STATUS_ARCH_MISMATCH");                          \
  if (stat_t == CUSPARSE_STATUS_MAPPING_ERROR)                          \
    LOG_INFO("CUSPARSE_STATUS_MAPPING_ERROR");                          \
  if (stat_t == CUSPARSE_STATUS_EXECUTION_FAILED)                       \
    LOG_INFO("CUSPARSE_STATUS_EXECUTION_FAILED");                       \
  if (stat_t == CUSPARSE_STATUS_INTERNAL_ERROR)                         \
    LOG_INFO("CUSPARSE_STATUS_INTERNAL_ERROR");                         \
  if (stat_t == CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED)              \
    LOG_INFO("CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED");              \
  LOG_INFO("File: " << file << "; line: " << line);                     \
  exit(1);                                                              \
  }                                                                     \
  }   

namespace paralution {

template <typename IndexType, unsigned int BLOCK_SIZE>
bool cum_sum( IndexType*  dst,
              const IndexType*  src,
              const IndexType   numElems) {
  
  cudaMemset(dst, 0, (numElems+1)*sizeof(IndexType));
  CHECK_CUDA_ERROR(__FILE__, __LINE__);
  
  IndexType* d_temp = NULL;
  allocate_gpu<IndexType>(numElems+1, &d_temp);
  
  cudaMemset(d_temp, 0, (numElems+1)*sizeof(IndexType));
  CHECK_CUDA_ERROR(__FILE__, __LINE__);
  
  kernel_red_partial_sum <IndexType, BLOCK_SIZE> <<< numElems/BLOCK_SIZE+1, BLOCK_SIZE>>>(dst+1, src, numElems);
  CHECK_CUDA_ERROR(__FILE__,__LINE__);
  
  kernel_red_recurse <IndexType> <<< numElems/(BLOCK_SIZE*BLOCK_SIZE)+1, BLOCK_SIZE>>>(d_temp, dst+BLOCK_SIZE, BLOCK_SIZE, (numElems+1));
  CHECK_CUDA_ERROR(__FILE__,__LINE__);
  
  kernel_red_extrapolate<IndexType> <<< numElems/(BLOCK_SIZE*BLOCK_SIZE)+1, BLOCK_SIZE>>>(dst+1, d_temp, src, numElems);
  CHECK_CUDA_ERROR(__FILE__,__LINE__);
  free_gpu<int>(&d_temp);
  
  return true;
  
}


}

#endif // PARALUTION_GPU_GPU_UTILS_HPP_

