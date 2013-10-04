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

#ifndef PARALUTION_GPU_CUDA_KERNELS_CSR_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_CSR_HPP_

#include "../matrix_formats_ind.hpp"

namespace paralution {

// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
template <typename ValueType, typename IndexType>
__global__ void kernel_csr_spmv_scalar(const IndexType nrow, const IndexType *row_offset, 
                                       const IndexType *col, const ValueType *val, 
                                       const ValueType *in, ValueType *out) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    out[ai] = ValueType(0.0);

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      out[ai] += val[aj]*in[col[aj]];
    }

  }
}

// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
template <typename ValueType, typename IndexType>
__global__ void kernel_csr_add_spmv_scalar(const IndexType nrow, const IndexType *row_offset, 
                                           const IndexType *col, const ValueType *val, 
                                           const ValueType scalar,
                                           const ValueType *in, ValueType *out) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      out[ai] += scalar*val[aj]*in[col[aj]];
    }

  }
}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_scale_diagonal(const IndexType nrow, const IndexType *row_offset, 
                                          const IndexType *col, const ValueType alpha, ValueType *val) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      if (ai == col[aj])
        val[aj] = alpha*val[aj];
    }

  }
}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_scale_offdiagonal(const IndexType nrow, const IndexType *row_offset, 
                                             const IndexType *col, const ValueType alpha, ValueType *val) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      if (ai != col[aj])
        val[aj] = alpha*val[aj];
    }

  }
}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_add_diagonal(const IndexType nrow, const IndexType *row_offset, 
                                        const IndexType *col, const ValueType alpha, ValueType *val) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      if (ai == col[aj])
        val[aj] = val[aj] + alpha;
    }

  }
}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_add_offdiagonal(const IndexType nrow, const IndexType *row_offset, 
                                           const IndexType *col, const ValueType alpha, ValueType *val) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      if (ai != col[aj])
        val[aj] = val[aj] + alpha;
    }

  }
}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_extract_diag(const IndexType nrow, const IndexType *row_offset, const IndexType *col, const ValueType *val,
                                        ValueType *vec) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      if (ai == col[aj])
        vec[ai] = val[aj];
    }

  }
}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_extract_inv_diag(const IndexType nrow, const IndexType *row_offset,
                                            const IndexType *col, const ValueType *val, ValueType *vec) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        vec[ai] = ValueType(1) / val[aj];

  }

}

template <typename ValueType, typename IndexType>
__global__ void kernel_csr_extract_submatrix_row_nnz(const IndexType *row_offset, const IndexType *col, const ValueType *val,
                                                     const IndexType smrow_offset, const IndexType smcol_offset, 
                                                     const IndexType smrow_size, const IndexType smcol_size,
                                                     IndexType *row_nnz) {
  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <smrow_size) {

    int nnz = 0 ;

    IndexType ind = ai+smrow_offset;

    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj) {

      IndexType c = col[aj];

      if ((c >= smcol_offset) &&
          (c < smcol_offset + smcol_size) )
        ++nnz;
    
    }
    
    row_nnz[ai] = nnz;

  }

}


template <typename ValueType, typename IndexType>
__global__ void kernel_csr_extract_submatrix_copy(const IndexType *row_offset, const IndexType *col, const ValueType *val,
                                                  const IndexType smrow_offset, const IndexType smcol_offset, 
                                                  const IndexType smrow_size, const IndexType smcol_size,
                                                  const IndexType *sm_row_offset, IndexType *sm_col, ValueType *sm_val) {
  
  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <smrow_size) {

    IndexType row_nnz = sm_row_offset[ai];
    IndexType ind = ai+smrow_offset;

    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj) {

      IndexType c = col[aj];
      if ((c >= smcol_offset) &&
          (c < smcol_offset + smcol_size) ) {

        sm_col[row_nnz] = c - smcol_offset;
        sm_val[row_nnz] = val[aj];
        ++row_nnz;

      }

    }

  }

}

template <typename ValueType, typename IndexType>
__global__ void kernel_csr_diagmatmult(const IndexType nrow, const IndexType *row_offset, 
                                       const IndexType *col, 
                                       const ValueType *diag, 
                                       ValueType *val) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj;

  if (ai <nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      val[aj] *= diag[ col[aj] ] ; 
    }

  }
}

// Calculates the number of non-zero elements per row
template <typename IndexType>
__global__ void kernel_calc_row_nnz( const IndexType nrow,
                              const IndexType *row_offset,
                              IndexType *row_nnz){
  int ai = blockIdx.x*blockDim.x + threadIdx.x;
  if(ai < nrow){
    row_nnz[ai] = row_offset[ai+1]-row_offset[ai];
  }
}

// Performs a permutation on the vector of non-zero elements per row
//
// Inputs:   nrow:         number of rows in matrix
//           row_nnz_src:  original number of non-zero elements per row 
//           perm_vec:     permutation vector
// Outputs:  row_nnz_dst   permuted number of non-zero elements per row
template <typename IndexType>
__global__ void kernel_permute_row_nnz(const IndexType nrow,
                                       const IndexType *row_nnz_src,
                                       const int *perm_vec,
                                       IndexType *row_nnz_dst) {

  int ai = blockIdx.x*blockDim.x + threadIdx.x;

  if (ai < nrow) {
    row_nnz_dst[perm_vec[ai]] = row_nnz_src[ai];
																}
}

// Permutes rows
// 
// Inputs:   nrow:             number of rows in matrix
//           row_offset:       original row pointer
//           perm_row_offset:  permuted row pointer
//           col:              original column indices of elements
//           data:             original data vector
//           perm_vec:         permutation vector
//           row_nnz:          number of non-zero elements per row
// Outputs:  perm_col:         permuted column indices of elements
//           perm_data:        permuted data vector
template <typename ValueType, typename IndexType>
__global__ void kernel_permute_rows(const IndexType nrow,
                                    const IndexType *row_offset,
                                    const IndexType *perm_row_offset,
                                    const IndexType *col,
                                    const ValueType *data,
                                    const IndexType *perm_vec,
                                    const IndexType *row_nnz,
                                    IndexType *perm_col,
                                    ValueType *perm_data) {

  int ai = blockIdx.x*blockDim.x + threadIdx.x;

  if (ai < nrow) {

    int num_elems = row_nnz[ai];
    int perm_index = perm_row_offset[perm_vec[ai]];
    int prev_index = row_offset[ai];

    for (int i = 0; i < num_elems; ++i) {
      perm_data[perm_index + i] = data[prev_index + i];
      perm_col[perm_index + i]  = col[prev_index + i];
    }

  }

}

// Permutes columns
//
// Inputs:   nrow:             number of rows in matrix
//           row_offset:       row pointer
//           perm_vec:         permutation vector
//           row_nnz:          number of non-zero elements per row
//           perm_col:         row-permuted column indices of elements
//           perm_data:        row-permuted data
// Outputs:  col:              fully permuted column indices of elements
//           data:             fully permuted data
template <typename ValueType, typename IndexType, const int size>
__global__ void kernel_permute_cols(const IndexType nrow,
                                    const IndexType *row_offset,
                                    const IndexType *perm_vec,
                                    const IndexType *row_nnz,
                                    const IndexType *perm_col,
                                    const ValueType *perm_data,
                                    IndexType *col,
                                    ValueType *data) {

  int ai = blockIdx.x*blockDim.x + threadIdx.x;
  int j;

  IndexType ccol[size];
  ValueType cval[size];

  if (ai < nrow) {

    IndexType num_elems = row_nnz[ai];
    IndexType elem_index = row_offset[ai];

    for (int i=0; i<num_elems; ++i) {
      ccol[i] = col[elem_index+i];
      cval[i] = data[elem_index+i];
    }

    for (int i = 0; i < num_elems; ++i) {

      IndexType comp = perm_vec[perm_col[elem_index+i]];

      for (j = i-1; j >= 0 ; --j) {
        IndexType c = ccol[j];
        if(c>comp){
          cval[j+1] = cval[j];
          ccol[j+1] = c;
        } else
          break;
      }

      cval[j+1] = perm_data[elem_index+i];
      ccol[j+1] = comp;

    }

    for (int i=0; i<num_elems; ++i) {
      col[elem_index+i] = ccol[i];
      data[elem_index+i] = cval[i];
    }

  }

}

// TODO
// kind of ugly and inefficient ... but works 
template <typename ValueType, typename IndexType>
__global__ void kernel_csr_add_csr_same_struct(const IndexType nrow,
                                               const IndexType *out_row_offset, const IndexType *out_col,
                                               const IndexType *in_row_offset,  const IndexType *in_col, const ValueType *in_val,
                                               const ValueType alpha, const ValueType beta,
                                               ValueType *out_val) {

  IndexType ai = blockIdx.x*blockDim.x+threadIdx.x;
  IndexType aj, ajj;

  if (ai <nrow) {

    int first_col = in_row_offset[ai];
      
    for (ajj=out_row_offset[ai]; ajj<out_row_offset[ai+1]; ++ajj)
      for (aj=first_col; aj<in_row_offset[ai+1]; ++aj)
        if (in_col[aj] == out_col[ajj]) {
          
          out_val[ajj] = alpha*out_val[ajj] + beta*in_val[aj];
          ++first_col;
          break ; 
          
        }
  }

}


}

#endif

