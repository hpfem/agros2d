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


#ifndef PARALUTION_OCL_KERNELS_ELL_HPP_
#define PARALUTION_OCL_KERNELS_ELL_HPP_

namespace paralution {

const char *ocl_kernels_ell =
// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
	"__kernel void kernel_ell_spmv(const int num_rows, \n"
	"                              const int num_cols, \n"
	"                              const int num_cols_per_row,\n"
	"                              __global const int *Acol,\n"
	"                              __global const ValueType *Aval, \n"
	"                              __global const ValueType *x, \n"
	"                              __global       ValueType *y)\n"
	"{\n"
	"\n"
	"  int row = get_global_id(0);\n"
	"\n"
	"  if (row < num_rows) {\n"
	"\n"
	"    ValueType sum = (ValueType)0;\n"
	"\n"
	"    for (int n=0; n<num_cols_per_row; ++n) {\n"
	"\n"
	"      const int ind = n * num_rows + row;\n"
	"      const int col = Acol[ind];\n"
	"\n"
	"      if ((col >= 0) && (col < num_cols))\n"
	"        sum += Aval[ind] * x[col];\n"
	"\n"
	"    }\n"
	"\n"
	"    y[row] = sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
	"__kernel void kernel_ell_add_spmv(const int num_rows, \n"
	"                                  const int num_cols, \n"
	"                                  const int num_cols_per_row,\n"
	"                                  __global const int *Acol,\n"
	"                                  __global const ValueType *Aval, \n"
	"                                  const ValueType scalar,\n"
	"                                  __global const ValueType *x, \n"
	"                                  __global       ValueType *y)\n"
	"{\n"
	"\n"
	"  int row = get_global_id(0);\n"
	"\n"
	"  if (row < num_rows) {\n"
	"\n"
	"    ValueType sum = (ValueType)0;\n"
	"\n"
	"    for (int n=0; n<num_cols_per_row; ++n) {\n"
	"\n"
	"      const int ind = n * num_rows + row;\n"
	"      const int col = Acol[ind];\n"
	"      \n"
	"      if ((col >= 0) && (col < num_cols))\n"
	"        sum += Aval[ind] * x[col];\n"
	"\n"
	"    }\n"
	"        \n"
	"    y[row] += scalar*sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"__kernel void kernel_ell_max_row(         const int  nrow,\n"
	"                                 __global const int *data,\n"
	"                                 __global       int *out,\n"
	"                                          const int  GROUP_SIZE,\n"
	"                                          const int  LOCAL_SIZE) {\n"
	"\n"
	"    int tid = get_local_id(0);\n"
	"\n"
	"    __local int sdata[BLOCK_SIZE];\n"
	"\n"
	"    sdata[tid] = 0;\n"
	"\n"
	"    int max;\n"
	"\n"
	"    int gid = GROUP_SIZE * get_group_id(0) + tid;\n"
	"\n"
	"    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {\n"
	"\n"
	"      if (gid < nrow) {\n"
	"        max = data[gid+1] - data[gid];\n"
	"        if (max > sdata[tid]) sdata[tid] = max;\n"
	"      }\n"
	"\n"
	"    }\n"
	"\n"
	"    barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {\n"
	"\n"
	"      if (tid < i)\n"
	"        if (sdata[tid+i] > sdata[tid]) sdata[tid] = sdata[tid+i];\n"
	"\n"
	"      barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"    }\n"
	"\n"
	"    if (tid == 0)\n"
	"      out[get_group_id(0)] = sdata[tid];\n"
	"\n"
	"}\n"
	"\n"
	"__kernel void kernel_ell_csr_to_ell(const int nrow, const int max_row,\n"
	"                                    __global const int *src_row_offset, __global const int *src_col,\n"
	"                                    __global const ValueType *src_val,  __global int *ell_col,\n"
	"                                    __global ValueType *ell_val) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"  int n;\n"
	"  int ell_ind;\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    n = (ValueType)0;\n"
	"\n"
	"    for (aj=src_row_offset[ai]; aj<src_row_offset[ai+1]; ++aj) {\n"
	"\n"
	"      ell_ind = n * nrow + ai;\n"
	"\n"
	"      ell_col[ell_ind] = src_col[aj];\n"
	"      ell_val[ell_ind] = src_val[aj];\n"
	"\n"
	"      ++n;\n"
	"\n"
	"    }\n"
	"\n"
	"    for (aj=src_row_offset[ai+1]-src_row_offset[ai]; aj<max_row; ++aj) {\n"
	"\n"
	"      ell_ind = n * nrow + ai;\n"
	"\n"
	"      ell_col[ell_ind] = (int)-1;\n"
	"      ell_val[ell_ind] = (ValueType)0;\n"
	"\n"
	"      ++n;\n"
	"\n"
	"    }\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
;
}

#endif // PARALUTION_OCL_KERNELS_ELL_HPP_
