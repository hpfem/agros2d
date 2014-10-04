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


#ifndef PARALUTION_GPU_CUDA_KERNELS_GENERAL_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_GENERAL_HPP_

#include "../matrix_formats_ind.hpp"

namespace paralution {

/*
// 1D accessing with stride
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_zeros(const IndexType n, ValueType *data) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  for (IndexType i=ind; i<n; i+=gridDim.x)
    data[i] = ValueType(0.0);

}
*/

// Pure 1D accessing
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_zeros(const IndexType n, ValueType *data) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    data[ind] = ValueType(0.0);

}

/*
// 1D accessing with stride
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_ones(const IndexType n, ValueType *data) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  for (IndexType i=ind; i<n; i+=gridDim.x)
    data[i] = ValueType(1.0);

}
*/

// Pure 1D accessing
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_ones(const IndexType n, ValueType *data) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    data[ind] = ValueType(1.0);

}

template <typename IndexType>
__device__ IndexType  red_recurse(IndexType *src, IndexType *srcStart, IndexType stride) {

  IndexType a = 0;

  if (src < srcStart)
    return 0;

  a = *src;
  a += red_recurse<IndexType>(src-stride, srcStart, stride);

  return a;

}

template <typename IndexType>
__global__ void kernel_red_recurse(IndexType *dst, IndexType *src, IndexType stride, IndexType numElems) {

  IndexType ind = stride * (threadIdx.x + blockIdx.x * blockDim.x);

  if (ind >= numElems)
    return;

  *(dst+ind) = red_recurse<IndexType>(src+ind-stride, src, stride);

}

template <typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_red_partial_sum(IndexType *dst, const IndexType *src, const IndexType numElems) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < numElems) {

    __shared__ IndexType data[BLOCK_SIZE];

    data[threadIdx.x] = src[ind];

    __syncthreads();

    for (IndexType i=BLOCK_SIZE/2; i>0; i/=2) {

      if (threadIdx.x < i)
        data[threadIdx.x] = data[threadIdx.x] + data[threadIdx.x+i];

      __syncthreads();

    }

    if (threadIdx.x == 0 && BLOCK_SIZE*(1+blockIdx.x)-1 < numElems)
      dst[BLOCK_SIZE*(1+blockIdx.x)-1] = data[0];

  }

}

template <typename IndexType>
__global__ void kernel_red_extrapolate(IndexType *dst, const IndexType *srcBorder,
                                       const IndexType *srcData, IndexType numElems) {

  IndexType ind = blockDim.x*(threadIdx.x + blockIdx.x*blockDim.x);

  if (ind < numElems-1) {

    IndexType sum = srcBorder[ind];
    IndexType limit = blockDim.x;

    if (ind+blockDim.x >= numElems)
      limit = numElems - ind;

//    for(IndexType i=0; i<blockDim.x && ind+i<numElems; ++i) {
    for(IndexType i=0; i<limit; ++i) {

      sum += srcData[ind+i];
      dst[ind+i] = sum;

    }

  }

}

template <typename IndexType>
__global__ void kernel_reverse_index(const IndexType n, const IndexType *perm, IndexType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[perm[ind]] = ind;

}

template <typename ValueType, typename IndexType>
__global__ void kernel_buffer_addscalar(const IndexType n, const ValueType scalar, ValueType *buff) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    buff[ind] += scalar;

}


}

#endif

