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

// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
__kernel void kernel_csr_spmv_scalar(const int nrow, __global const int *row_offset, __global const int *col,
                                     __global const ValueType *val, __global const ValueType *in,
                                     __global ValueType *out) {

  int ai = get_global_id(0);

  if (ai < nrow) {

    ValueType sum = 0;

    for (int aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      sum += val[aj] * in[col[aj]];

    out[ai] = sum;

  }

}


// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
__kernel void kernel_csr_add_spmv_scalar(const int nrow, __global const int *row_offset,
                                         __global const int *col, __global const ValueType *val,
                                         const ValueType scalar, __global const ValueType *in,
                                         __global ValueType *out) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

    ValueType sum = out[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      sum += scalar * val[aj] * in[col[aj]];

    out[ai] = sum;

  }

}


__kernel void kernel_csr_scale_diagonal(const int nrow, __global const int *row_offset, __global const int *col,
                                        const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        val[aj] = alpha * val[aj];

}


__kernel void kernel_csr_scale_offdiagonal(const int nrow, __global const int *row_offset, __global const int *col,
                                           const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai != col[aj])
        val[aj] = alpha * val[aj];

}


__kernel void kernel_csr_add_diagonal(const int nrow, __global const int *row_offset,
                                      __global const int *col, const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        val[aj] = val[aj] + alpha;

}


__kernel void kernel_csr_add_offdiagonal(const int nrow, __global const int *row_offset,
                                         __global const int *col, const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai != col[aj])
        val[aj] = val[aj] + alpha;

}


__kernel void kernel_csr_extract_diag(const int nrow, __global const int *row_offset, __global const int *col,
                                      __global const ValueType *val, __global ValueType *vec) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        vec[ai] = val[aj];

}


__kernel void kernel_csr_extract_inv_diag(const int nrow, __global const int *row_offset,
                                          __global const int *col, __global const ValueType *val,
                                          __global ValueType *vec) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        vec[ai] = 1.0 / val[aj];

}


__kernel void kernel_csr_extract_submatrix_row_nnz(__global const int *row_offset, __global const int *col,
                                                   __global const ValueType *val, const int smrow_offset,
                                                   const int smcol_offset, const int smrow_size,
                                                   const int smcol_size, __global int *row_nnz) {

  int ai = get_global_id(0);
  int aj;

  if (ai < smrow_size) {

    int nnz = 0;
    int ind = ai + smrow_offset;

    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj)

      if ( (col[aj] >= smcol_offset) &&
           (col[aj] <  smcol_offset + smcol_size) )
        ++nnz;

    row_nnz[ai] = nnz;

  }

}


__kernel void kernel_csr_extract_submatrix_copy(__global const int *row_offset, __global const int *col,
                                                __global const ValueType *val, const int smrow_offset,
                                                const int smcol_offset, const int smrow_size,
                                                const int smcol_size, __global const int *sm_row_offset,
                                                __global int *sm_col, __global ValueType *sm_val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < smrow_size) {

    int row_nnz = sm_row_offset[ai];
    int ind = ai + smrow_offset;

    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj) {

      if ( (col[aj] >= smcol_offset) &&
           (col[aj] <  smcol_offset + smcol_size) ) {

        sm_col[row_nnz] = col[aj] - smcol_offset;
        sm_val[row_nnz] = val[aj];
        ++row_nnz;

      }

    }

  }

}


__kernel void kernel_csr_diagmatmult(const int nrow, __global const int *row_offset, __global const int *col,
                                     __global const ValueType *diag, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      val[aj] *= diag[ col[aj] ];

}


__kernel void kernel_csr_add_csr_same_struct(const int nrow, __global const int *out_row_offset,
                                             __global const int *out_col, __global const int *in_row_offset,
                                             __global const int *in_col, __global const ValueType *in_val,
                                             const ValueType alpha, const ValueType beta, __global ValueType *out_val) {

  int ai = get_global_id(0);
  int aj, ajj;

  if (ai < nrow) {

    int first_col = in_row_offset[ai];
      
    for (ajj=out_row_offset[ai]; ajj<out_row_offset[ai+1]; ++ajj)
      for (aj=first_col; aj<in_row_offset[ai+1]; ++aj)
        if (in_col[aj] == out_col[ajj]) {
          
          out_val[ajj] = alpha * out_val[ajj] + beta * in_val[aj];
          ++first_col;
          break;

        }

  }

}


__kernel void kernel_buffer_addscalar(const int size, const ValueType scalar, __global ValueType *buff) {

  int gid = get_global_id(0);

  if (gid < size)
    buff[gid] += scalar;

}


__kernel void kernel_reverse_index(const int size, __global const int *perm, __global int *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[perm[gid]] = gid;

}


__kernel void kernel_csr_calc_row_nnz(const int nrow, __global const int *row_offset, __global int *row_nnz) {

  int ai = get_global_id(0);

  if (ai < nrow)
    row_nnz[ai] = row_offset[ai+1]-row_offset[ai];

}


__kernel void kernel_csr_permute_row_nnz(         const int  nrow,
                                         __global const int *row_nnz_src,
                                         __global const int *perm_vec,
                                         __global       int *row_nnz_dst) {

  int ai = get_global_id(0);

  if (ai < nrow)
    row_nnz_dst[perm_vec[ai]] = row_nnz_src[ai];

}


__kernel void kernel_csr_permute_rows(       const       int  nrow,
                                    __global const       int *row_offset,
                                    __global const       int *perm_row_offset,
                                    __global const       int *col,
                                    __global const ValueType *data,
                                    __global const       int *perm_vec,
                                    __global const       int *row_nnz,
                                    __global             int *perm_col,
                                    __global       ValueType *perm_data) {

  int ai = get_global_id(0);

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


__kernel void kernel_csr_permute_cols(         const       int  nrow,
                                      __global const       int *row_offset,
                                      __global const       int *perm_vec,
                                      __global const       int *row_nnz,
                                      __global const       int *perm_col,
                                      __global const ValueType *perm_data,
                                      __global             int *col,
                                      __global       ValueType *data) {

  int ai = get_global_id(0);
  int j;

  if (ai < nrow) {

    int num_elems = row_nnz[ai];
    int elem_index = row_offset[ai];

    for (int i = 0; i < num_elems; ++i) {

      int comp = perm_vec[perm_col[elem_index+i]];

      for (j = i-1; j >= 0 ; --j) {

        if (col[elem_index+j]>comp) {
          data[elem_index+j+1] = data[elem_index+j];
          col[elem_index+j+1]  = col[elem_index+j];
        } else
          break;
      }

      data[elem_index+j+1] = perm_data[elem_index+i];
      col[elem_index+j+1]  = comp;

    }

  }

}

