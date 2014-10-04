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


#ifndef PARALUTION_GPU_CUDA_KERNELS_VECTOR_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_VECTOR_HPP_

namespace paralution {

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleadd(const IndexType n, const ValueType alpha, const ValueType *x, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind] = alpha * out[ind] + x[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleaddscale(const IndexType n, const ValueType alpha, const ValueType beta,
                                     const ValueType *x, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind] = alpha*out[ind] + beta*x[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleaddscale_offset(const IndexType n,
                                            const IndexType src_offset, const IndexType dst_offset,
                                            const ValueType alpha, const ValueType beta,
                                            const ValueType *x, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind+dst_offset] = alpha*out[ind+dst_offset] + beta*x[ind+src_offset];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_scaleadd2(const IndexType n, const ValueType alpha, const ValueType beta, const ValueType gamma,
                                 const ValueType *x, const ValueType *y, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind] = alpha*out[ind] + beta*x[ind] + gamma*y[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_pointwisemult(const IndexType n, const ValueType *x, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind] = out[ind] * x[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_pointwisemult2(const IndexType n, const ValueType *x, const ValueType *y, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind] = y[ind] * x[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_copy_offset_from(const IndexType n, const IndexType src_offset, const IndexType dst_offset,
                                        const ValueType *in, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind+dst_offset] = in[ind+src_offset];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_permute(const IndexType n, const IndexType *permute,
                               const ValueType *in, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[permute[ind]] = in[ind];

}

template <typename ValueType, typename IndexType>
__global__ void kernel_permute_backward(const IndexType n, const IndexType *permute,
                                        const ValueType *in, ValueType *out) {

  IndexType ind = blockIdx.x * blockDim.x + threadIdx.x;

  if (ind < n)
    out[ind] = in[permute[ind]];

}

template <typename ValueType, typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_reduce(const IndexType n, const ValueType *data, ValueType *out,
                              const IndexType GROUP_SIZE, const IndexType LOCAL_SIZE) {

    IndexType tid = threadIdx.x;

    __shared__ ValueType sdata[BLOCK_SIZE];
    sdata[tid] = ValueType(0.0);

    // get global id
    IndexType gid = GROUP_SIZE * blockIdx.x + tid;

    for (IndexType i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE)
      if ( gid < n )
        sdata[tid] += data[gid];

    __syncthreads();

#pragma unroll
    for (IndexType i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        sdata[tid] += sdata[tid + i];

      __syncthreads();

    }

    if (tid == 0)
      out[blockIdx.x] = sdata[tid];

}

template <typename ValueType, typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_max(const IndexType n, const ValueType *data, ValueType *out,
                           const IndexType GROUP_SIZE, const IndexType LOCAL_SIZE) {

    IndexType tid = threadIdx.x;

    __shared__ ValueType sdata[BLOCK_SIZE];
    sdata[tid] = ValueType(0);

    // get global id
    IndexType gid = GROUP_SIZE * blockIdx.x + tid;

    for (IndexType i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < n) {
        ValueType tmp = data[gid];
        if (tmp > sdata[tid])
          sdata[tid] = tmp;
      }

    }

    __syncthreads();

#pragma unroll
    for (IndexType i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        if (sdata[tid+i] > sdata[tid])
          sdata[tid] = sdata[tid+i];

      __syncthreads();

    }

    if (tid == 0)
      out[blockIdx.x] = sdata[tid];

}

template <typename ValueType, typename IndexType, unsigned int BLOCK_SIZE>
__global__ void kernel_amax(const IndexType n, const ValueType *data, ValueType *out,
                            const IndexType GROUP_SIZE, const IndexType LOCAL_SIZE) {

    IndexType tid = threadIdx.x;

    __shared__ ValueType sdata[BLOCK_SIZE];
    sdata[tid] = ValueType(0);

    // get global id
    IndexType gid = GROUP_SIZE * blockIdx.x + tid;

    for (IndexType i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < n) {
        ValueType tmp = data[gid];
        tmp = max(tmp, ValueType(-1.0)*tmp);
        if (tmp > sdata[tid])
          sdata[tid] = tmp;
      }

    }

    __syncthreads();

#pragma unroll
    for (IndexType i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i) {
        ValueType tmp = sdata[tid+i];
        tmp = max(tmp, ValueType(-1.0)*tmp);
        if (tmp > sdata[tid])
          sdata[tid] = tmp;
      }

      __syncthreads();

    }

    if (tid == 0)
      out[blockIdx.x] = sdata[tid];

}


}

#endif

