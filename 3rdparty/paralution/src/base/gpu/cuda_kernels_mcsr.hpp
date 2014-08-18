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


#ifndef PARALUTION_GPU_CUDA_KERNELS_MCSR_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_MCSR_HPP_

#include "../matrix_formats_ind.hpp"

namespace paralution {

template <typename ValueType, typename IndexType>
__global__ void kernel_mcsr_spmv_scalar(const IndexType nrow, const IndexType *row_offset, 
                                        const IndexType *col, const ValueType *val, 
                                        const ValueType *in, ValueType *out) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    ValueType sum = val[ai] * in[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      sum += val[aj]*in[col[aj]];

    out[ai] = sum;

  }

}


template <typename ValueType, typename IndexType>
__global__ void kernel_mcsr_add_spmv_scalar(const IndexType nrow, const IndexType *row_offset, 
                                            const IndexType *col, const ValueType *val, 
                                            const ValueType scalar,
                                            const ValueType *in, ValueType *out) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    out[ai] += scalar*val[ai] * in[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      out[ai] += scalar*val[aj]*in[col[aj]];
    }

  }
}


}

#endif

