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

#include <cmath>
#include "gpu_allocate_free.hpp"
#include <cuda.h>
#include <assert.h>
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"

namespace paralution {

template <typename DataType>
void allocate_gpu(const int size, DataType **ptr) {

  if (size > 0) {

    assert(*ptr == NULL);
    
    cudaMalloc( (void **)ptr, size*sizeof(DataType));
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    assert(*ptr != NULL);
  }

};

template <typename DataType>
void free_gpu(DataType **ptr) {

  assert(*ptr != NULL);
  
  cudaDeviceSynchronize();
  CHECK_CUDA_ERROR(__FILE__, __LINE__);

  cudaFree(*ptr);
  CHECK_CUDA_ERROR(__FILE__, __LINE__);
  
  *ptr = NULL;

};

template <typename DataType>
void set_to_zero_gpu(const int blocksize,
                     const int max_threads,
                     const int size, DataType *ptr) {
  
  if (size > 0) {

    assert(ptr != NULL);

    cudaMemset(ptr, 0, size*sizeof(DataType));
    CHECK_CUDA_ERROR(__FILE__, __LINE__);

    /*
    int s = size ;
    int k = (size/blocksize)/max_threads + 1;       
    if (k > 1) s = size / k;
    

    dim3 BlockSize(blocksize);
    dim3 GridSize(s / blocksize + 1);

    cudaDeviceSynchronize();
    
    kernel_set_to_zeros<DataType, int> <<<GridSize, BlockSize>>> (size, ptr);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
    */

    /*
    // 1D accessing, no stride
    dim3 BlockSize(blocksize);
    dim3 GridSize(size / blocksize + 1);

    cudaDeviceSynchronize();
    
    kernel_set_to_zeros<DataType, int> <<<GridSize, BlockSize>>> (size, ptr);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
    */
    
  }

}


template <typename DataType>
void set_to_one_gpu(const int blocksize,
                    const int max_threads,
                    const int size, DataType *ptr) {
  
  if (size > 0) {

    assert(ptr != NULL);


    int s = size ;
    int k = (size/blocksize)/max_threads + 1;       
    if (k > 1) s = size / k;
    

    dim3 BlockSize(blocksize);
    dim3 GridSize(s / blocksize + 1);

    cudaDeviceSynchronize();
    
    kernel_set_to_ones<DataType, int> <<<GridSize, BlockSize>>> (size, ptr);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      

    /*
    // 1D accessing, no stride
    dim3 BlockSize(blocksize);
    dim3 GridSize(size / blocksize + 1);

    cudaDeviceSynchronize();
    
    kernel_set_to_ones<DataType, int> <<<GridSize, BlockSize>>> (size, ptr);
    
    CHECK_CUDA_ERROR(__FILE__, __LINE__);      
    */

  }

}



template void allocate_gpu<float       >(const int size, float        **ptr);
template void allocate_gpu<double      >(const int size, double       **ptr);
template void allocate_gpu<int         >(const int size, int          **ptr);
template void allocate_gpu<unsigned int>(const int size, unsigned int **ptr);
template void allocate_gpu<char        >(const int size, char         **ptr);

template void free_gpu<float       >(float        **ptr);
template void free_gpu<double      >(double       **ptr);
template void free_gpu<int         >(int          **ptr);
template void free_gpu<unsigned int>(unsigned int **ptr);
template void free_gpu<char        >(char         **ptr);

template void set_to_zero_gpu<float       >(const int blocksize, const int max_threads, const int size, float        *ptr);
template void set_to_zero_gpu<double      >(const int blocksize, const int max_threads, const int size, double       *ptr);
template void set_to_zero_gpu<int         >(const int blocksize, const int max_threads, const int size, int          *ptr);
template void set_to_zero_gpu<unsigned int>(const int blocksize, const int max_threads, const int size, unsigned int *ptr);
template void set_to_zero_gpu<char        >(const int blocksize, const int max_threads, const int size, char         *ptr);


template void set_to_one_gpu<float       >(const int blocksize, const int max_threads, const int size, float        *ptr);
template void set_to_one_gpu<double      >(const int blocksize, const int max_threads, const int size, double       *ptr);
template void set_to_one_gpu<int         >(const int blocksize, const int max_threads, const int size, int          *ptr);
template void set_to_one_gpu<unsigned int>(const int blocksize, const int max_threads, const int size, unsigned int *ptr);
template void set_to_one_gpu<char        >(const int blocksize, const int max_threads, const int size, char         *ptr);


};


