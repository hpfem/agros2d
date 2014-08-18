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


#ifndef PARALUTION_GPU_CUDA_KERNELS_HYB_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_HYB_HPP_

#include "cuda_kernels_ell.hpp"
#include "cuda_kernels_coo.hpp"
#include "../matrix_formats_ind.hpp"

namespace paralution {

template <typename IndexType>
__global__ void kernel_ell_nnz_coo(const IndexType nrow, const IndexType max_row,
                                   const IndexType *row_offset, IndexType *nnz_coo) {

  IndexType gid = threadIdx.x + blockIdx.x * blockDim.x;

  if (gid < nrow) {

    nnz_coo[gid] = 0;
    IndexType nnz_per_row = row_offset[gid+1] - row_offset[gid];

    if (nnz_per_row > max_row)

      nnz_coo[gid] = nnz_per_row - max_row;

  }

}

template <typename ValueType, typename IndexType>
__global__ void kernel_ell_fill_ell(const IndexType nrow, const IndexType max_row,
                                    const IndexType *row_offset, const IndexType *col,
                                    const ValueType *val, IndexType *ELL_col,
                                    ValueType *ELL_val, IndexType *nnz_ell) {

  IndexType gid = threadIdx.x + blockIdx.x * blockDim.x;

  if (gid < nrow) {

    IndexType n = 0;

    for (IndexType i=row_offset[gid]; i<row_offset[gid+1]; ++i) {

      if (n >= max_row) break;

      IndexType idx = ELL_IND(gid, n, nrow, max_row);

      ELL_col[idx] = col[i];
      ELL_val[idx] = val[i];

      ++n;

    }

    nnz_ell[gid] = n;

  }

}

template <typename ValueType, typename IndexType>
__global__ void kernel_ell_fill_coo(const IndexType nrow, const IndexType *row_offset,
                                    const IndexType *col, const ValueType *val,
                                    const IndexType *nnz_coo, const IndexType *nnz_ell,
                                    IndexType *COO_row, IndexType *COO_col, ValueType *COO_val) {

  IndexType gid = threadIdx.x + blockIdx.x * blockDim.x;

  if (gid < nrow) {

    IndexType row_ptr = row_offset[gid+1];

    for (IndexType i=row_ptr - nnz_coo[gid]; i<row_ptr; ++i) {

      IndexType idx = i - nnz_ell[gid];

      COO_row[idx] = gid;
      COO_col[idx] = col[i];
      COO_val[idx] = val[i];

    }

  }

}


}

#endif

