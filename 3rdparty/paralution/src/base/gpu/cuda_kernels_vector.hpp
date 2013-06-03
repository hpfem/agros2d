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

#ifndef PARALUTION_GPU_CUDA_KERNELS_VECTOR_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_VECTOR_HPP_

namespace paralution {

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleadd(const IndexType n, const ValueType alpha, const ValueType *x, ValueType *out) {

  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind < n)
    out[ind] = alpha * out[ind] + x[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleaddscale(const IndexType n, const ValueType alpha, const ValueType beta,
                                     const ValueType *x, ValueType *out) {

  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind < n)
    out[ind] = alpha*out[ind] + beta*x[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleadd2(const IndexType n, 
                                 const ValueType alpha, const ValueType beta, const ValueType gamma, 
                                 const ValueType *x, const ValueType *y, ValueType *out) 
{
  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind <n) {
    out[ind] = alpha*out[ind] + beta*x[ind] + gamma*y[ind];
  }
}

template <typename ValueType, typename IndexType>
__global__ void kernel_pointwisemult(const IndexType n, const ValueType *x, ValueType *out) 
{
  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind <n) {
    out[ind] = out[ind] * x[ind];
  }
}

template <typename ValueType, typename IndexType>
__global__ void kernel_pointwisemult2(const IndexType n, const ValueType *x, const ValueType *y, ValueType *out) 
{
  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  if (ind <n) {
    out[ind] = y[ind] * x[ind];
  }
}

template <typename ValueType, typename IndexType>
__global__ void kernel_copy_offset_from(const IndexType size, const IndexType src_offset, const IndexType dst_offset,
                                        const ValueType *in, ValueType *out) {

  IndexType i = blockIdx.x*blockDim.x+threadIdx.x;

  if (i <size) {
    out[i+dst_offset] = in[i+src_offset];
  }

}

template <typename ValueType, typename IndexType>
__global__ void kernel_permute(const IndexType size, const IndexType *permute, 
                               const ValueType *in, ValueType *out) {

  IndexType i = blockIdx.x*blockDim.x+threadIdx.x;

  if (i <size) {
    out[ permute[i] ] = in[i];
  }

}

template <typename ValueType, typename IndexType>
__global__ void kernel_permute_backward(const IndexType size, const IndexType *permute, 
                                        const ValueType *in, ValueType *out) {

  IndexType i = blockIdx.x*blockDim.x+threadIdx.x;

  if (i <size) {
    out[i] = in[ permute[i] ];
  }

}

template <typename ValueType, typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_reduce(const IndexType  size,
                              const ValueType *data,
                                    ValueType *out,
                              const IndexType  GROUP_SIZE,
                              const IndexType  LOCAL_SIZE) {

    int tid = threadIdx.x;

    __shared__ ValueType sdata[BLOCK_SIZE];
    sdata[tid] = 0.0;

    // get global id
    int gid = GROUP_SIZE * blockIdx.x + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if ( gid < size )
        sdata[tid] += data[gid];

    }

    __syncthreads();

#pragma unroll
    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if ( tid < i )
        sdata[tid] += sdata[tid + i];

      __syncthreads();

    }

    if (tid == 0)
      out[blockIdx.x] = sdata[tid];

}

template <typename ValueType, typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_max(const IndexType  size,
                           const ValueType *data,
                                 ValueType *out,
                           const IndexType  GROUP_SIZE,
                           const IndexType  LOCAL_SIZE) {

    int tid = threadIdx.x;

    __shared__ ValueType sdata[BLOCK_SIZE];
    sdata[tid] = ValueType(0);

    // get global id
    int gid = GROUP_SIZE * blockIdx.x + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size) {
        ValueType tmp = data[gid];
        if (tmp > sdata[tid])
          sdata[tid] = tmp;
      }

    }

    __syncthreads();

#pragma unroll
    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if ( tid < i )
        if (sdata[tid+i] > sdata[tid])
          sdata[tid] = sdata[tid+i];

      __syncthreads();

    }

    if (tid == 0)
      out[blockIdx.x] = sdata[tid];

}


}


#endif

