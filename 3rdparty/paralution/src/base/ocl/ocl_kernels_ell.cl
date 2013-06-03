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
__kernel void kernel_ell_spmv(const int num_rows, 
                              const int num_cols, 
                              const int num_cols_per_row,
                              __global const int *Acol,
                              __global const ValueType *Aval, 
                              __global const ValueType *x, 
                              __global       ValueType *y)
{

  int row = get_global_id(0);

  if (row < num_rows) {

    ValueType sum = (ValueType)0;

    for (int n=0; n<num_cols_per_row; ++n) {

      const int ind = n * num_rows + row;
      const int col = Acol[ind];

      if ((col >= 0) && (col < num_cols))
        sum += Aval[ind] * x[col];

    }

    y[row] = sum;

  }

}


// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
__kernel void kernel_ell_add_spmv(const int num_rows, 
                                  const int num_cols, 
                                  const int num_cols_per_row,
                                  __global const int *Acol,
                                  __global const ValueType *Aval, 
                                  const ValueType scalar,
                                  __global const ValueType *x, 
                                  __global       ValueType *y)
{

  int row = get_global_id(0);

  if (row < num_rows) {

    ValueType sum = (ValueType)0;

    for (int n=0; n<num_cols_per_row; ++n) {

      const int ind = n * num_rows + row;
      const int col = Acol[ind];
      
      if ((col >= 0) && (col < num_cols))
        sum += Aval[ind] * x[col];

    }
        
    y[row] += scalar*sum;

  }

}


__kernel void kernel_ell_max_row(         const int  nrow,
                                 __global const int *data,
                                 __global       int *out,
                                          const int  GROUP_SIZE,
                                          const int  LOCAL_SIZE) {

    int tid = get_local_id(0);

    __local int sdata[BLOCK_SIZE];

    sdata[tid] = 0;

    int max;

    // get global id
    int gid = GROUP_SIZE * get_group_id(0) + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < nrow) {
        max = data[gid+1] - data[gid];
        if (max > sdata[tid]) sdata[tid] = max;
      }

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        if (sdata[tid+i] > sdata[tid]) sdata[tid] = sdata[tid+i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0)
      out[get_group_id(0)] = sdata[tid];

}


__kernel void kernel_ell_csr_to_ell(const int nrow, const int max_row,
                                    __global const int *src_row_offset, __global const int *src_col,
                                    __global const ValueType *src_val,  __global int *ell_col,
                                    __global ValueType *ell_val) {

  int ai = get_global_id(0);
  int aj;
  int n;
  int ell_ind;

  if (ai < nrow) {

    n = (ValueType)0;

    for (aj=src_row_offset[ai]; aj<src_row_offset[ai+1]; ++aj) {

      ell_ind = n * nrow + ai;

      ell_col[ell_ind] = src_col[aj];
      ell_val[ell_ind] = src_val[aj];

      ++n;

    }

    for (aj=src_row_offset[ai+1]-src_row_offset[ai]; aj<max_row; ++aj) {

      ell_ind = n * nrow + ai;

      ell_col[ell_ind] = (int)-1;
      ell_val[ell_ind] = (ValueType)0;

      ++n;

    }

  }

}

