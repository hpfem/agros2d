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

#ifndef PARALUTION_GPU_CUDA_KERNELS_GENERAL_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_GENERAL_HPP_

namespace paralution {


// 1D accessing with stride
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_zeros(const IndexType n, ValueType *data) 
{

  IndexType ind = blockIdx.x*blockDim.x + threadIdx.x;

  for (int i=ind; i<n; i+=gridDim.x)
    data[i] = 0;

}


/*
// Pure 1D accessing
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_zeros(const IndexType n, ValueType *data) 
{

  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind <n) {
    // set values to zero
    data[ind] = 0;
  }
}
 */

// 1D accessing with stride
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_ones(const IndexType n, ValueType *data) 
{

  IndexType ind = blockIdx.x*blockDim.x + threadIdx.x;

  for (int i=ind; i<n; i+=gridDim.x)
    data[i] = ValueType(1.0);

}

/*
// Pure 1D accessing
template <typename ValueType, typename IndexType>
__global__ void kernel_set_to_ones(const IndexType n, ValueType *data) 
{
  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind <n) {
    // set values to one
    data[ind] = ValueType(1.0);
  }
}
*/


template <typename IndexType>
__device__ IndexType  red_recurse(IndexType *src,
                                  IndexType *srcStart,
                                  IndexType  stride) {

  IndexType a = 0;

  // warp divergence
  if (src < srcStart)
    return 0;

  a =*src;
  a += red_recurse<IndexType>(src-stride,srcStart, stride);

  return a;
}

template <typename IndexType>
__global__ void kernel_red_recurse( IndexType *dst,
                                    IndexType *src,
                                    IndexType  stride,
                                    IndexType  numElems) {

  IndexType index = stride*(threadIdx.x + blockIdx.x*blockDim.x);

  if(index >= numElems) {
    return;
  }

  *(dst+index) = red_recurse<IndexType>(src+index-stride,src,stride);

}

template <typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_red_partial_sum(IndexType *dst,
                                       const IndexType *src,
                                       const IndexType  numElems) {

  IndexType index = threadIdx.x + blockIdx.x*blockDim.x;

  if (index < numElems) {

    __shared__ IndexType data[BLOCK_SIZE];

    data[threadIdx.x] = src[index];

    __syncthreads();

    for (int i = BLOCK_SIZE/2; i > 0; i/=2) {

      if (threadIdx.x < i){
        data[threadIdx.x] = data[threadIdx.x] + data[threadIdx.x+i];
      }

      __syncthreads();

    }

    if (threadIdx.x == 0 && BLOCK_SIZE*(1+blockIdx.x)-1<numElems) {
      dst[BLOCK_SIZE*(1+blockIdx.x)-1] = data[0];
    }

  }

}

template <typename IndexType>
__global__ void kernel_red_extrapolate(IndexType *dst,
                                       const IndexType *srcBorder,
                                       const IndexType *srcData,
                                       IndexType numElems) {

  IndexType index = blockDim.x*(threadIdx.x + blockIdx.x*blockDim.x);

  if (index < numElems-1) {

    IndexType sum = srcBorder[index];
    for(int i = 0; i < blockDim.x && index+i<numElems; ++i){
      sum += srcData[index+i];
      dst[index+i] = sum;
    }
  }

}

template <typename IndexType>
__global__ void kernel_reverse_index(const IndexType n, 
                                     const IndexType *perm,
                                     IndexType *out) {

  IndexType i = blockIdx.x*blockDim.x+threadIdx.x;

  if (i <n)
    out[ perm[i] ] = i ;


}

template <typename ValueType, typename IndexType>
__global__ void kernel_buffer_addscalar(const IndexType size, 
                                        const ValueType scalar,
                                        ValueType *buff) {

  IndexType i = blockIdx.x*blockDim.x+threadIdx.x;

  if (i <size)
    buff[i] += scalar;

}


};
#endif

