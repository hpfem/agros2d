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


#ifndef PARALUTION_OCL_KERNELS_COO_HPP_
#define PARALUTION_OCL_KERNELS_COO_HPP_

namespace paralution {

const char *ocl_kernels_coo =
	"__kernel void kernel_coo_permute(         const int  nnz,\n"
	"                                 __global const int *in_row,\n"
	"                                 __global const int *in_col,\n"
	"                                 __global const int *perm,\n"
	"                                 __global       int *out_row,\n"
	"                                 __global       int *out_col) {\n"
	"\n"
	"  int ind = get_global_id(0);\n"
	"\n"
	"  for (int i=ind; i<nnz; i+=get_local_size(0)) {\n"
	"\n"
	"    out_row[i] = perm[ in_row[i] ];\n"
	"    out_col[i] = perm[ in_col[i] ];\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
// ----------------------------------------------------------
// function segreduce_warp(...)
// ----------------------------------------------------------
// Modified and adopted from CUSP 0.3.1, 
// http://code.google.com/p/cusp-library/
// NVIDIA, APACHE LICENSE 2.0
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
	"inline ValueType segreduce_warp(const int thread_lane, int row, ValueType val, __local int *rows, __local ValueType *vals) {\n"
	"\n"
	"  int tid = get_local_id(0);\n"
	"\n"
	"  rows[tid] = row;\n"
	"  vals[tid] = val;\n"
	"  \n"
	"  if( thread_lane >=  1 && row == rows[tid -  1] ) { vals[tid] = val = val + vals[tid -  1]; } \n"
	"  if( thread_lane >=  2 && row == rows[tid -  2] ) { vals[tid] = val = val + vals[tid -  2]; }\n"
	"  if( thread_lane >=  4 && row == rows[tid -  4] ) { vals[tid] = val = val + vals[tid -  4]; }\n"
	"  if( thread_lane >=  8 && row == rows[tid -  8] ) { vals[tid] = val = val + vals[tid -  8]; }\n"
	"  if( thread_lane >= 16 && row == rows[tid - 16] ) { vals[tid] = val = val + vals[tid - 16]; }\n"
	"  \n"
	"  return val;\n"
	"\n"
	"}\n"
	"\n"
// ----------------------------------------------------------
// function segreduce_block(...)
// ----------------------------------------------------------
// Modified and adopted from CUSP 0.3.1, 
// http://code.google.com/p/cusp-library/
// NVIDIA, APACHE LICENSE 2.0
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
	"inline void segreduce_block(__local const int *idx, __local ValueType *val) {\n"
	"\n"
	"  ValueType left = 0;\n"
	"  int tid = get_local_id(0);\n"
	"\n"
	"  if( tid >=   1 && idx[tid] == idx[tid -   1] ) { left = val[tid -   1]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);  \n"
	"  if( tid >=   2 && idx[tid] == idx[tid -   2] ) { left = val[tid -   2]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"  if( tid >=   4 && idx[tid] == idx[tid -   4] ) { left = val[tid -   4]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"  if( tid >=   8 && idx[tid] == idx[tid -   8] ) { left = val[tid -   8]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"  if( tid >=  16 && idx[tid] == idx[tid -  16] ) { left = val[tid -  16]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"  if( tid >=  32 && idx[tid] == idx[tid -  32] ) { left = val[tid -  32]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);  \n"
	"  if( tid >=  64 && idx[tid] == idx[tid -  64] ) { left = val[tid -  64]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"  if( tid >= 128 && idx[tid] == idx[tid - 128] ) { left = val[tid - 128]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"  if( tid >= 256 && idx[tid] == idx[tid - 256] ) { left = val[tid - 256]; } barrier(CLK_LOCAL_MEM_FENCE); val[tid] += left; left = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"}\n"
	"\n"
// ----------------------------------------------------------
// function kernel_spmv_coo_flat(...)
// ----------------------------------------------------------
// Modified and adopted from CUSP 0.3.1, 
// http://code.google.com/p/cusp-library/
// NVIDIA, APACHE LICENSE 2.0
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
	"__kernel void kernel_coo_spmv_flat(         const       int  num_nonzeros,\n"
	"                                            const       int  interval_size,\n"
	"                                   __global const       int *I, \n"
	"                                   __global const       int *J, \n"
	"                                   __global const ValueType *V, \n"
	"                                            const ValueType  scalar,\n"
	"                                   __global const ValueType *x, \n"
	"                                   __global       ValueType *y,\n"
	"                                   __global             int *temp_rows,\n"
	"                                   __global       ValueType *temp_vals) {\n"
	"\n"
	"  __local volatile int rows[48 * (BLOCK_SIZE/32)];\n"
	"  __local volatile ValueType vals[BLOCK_SIZE];\n"
	"\n"
	"        int tid         = get_local_id(0);\n"
	"  const int thread_id   = BLOCK_SIZE * get_group_id(0) + tid;\n"
	"  const int thread_lane = tid & (WARP_SIZE-1);\n"
	"  const int warp_id     = thread_id   / WARP_SIZE;\n"
	"\n"
	"  const int interval_begin = warp_id * interval_size;\n"
	"  int interval_end2 = interval_begin + interval_size;\n"
	"  if (interval_end2 > num_nonzeros)\n"
	"    interval_end2 = num_nonzeros;\n"
	"\n"
	"  const int interval_end = interval_end2;\n"
	"\n"
	"  const int idx = 16 * (tid/32 + 1) + tid;\n"
	"\n"
	"  rows[idx - 16] = -1;\n"
	"\n"
	"  if(interval_begin >= interval_end)\n"
	"    return;\n"
	"\n"
	"  if (thread_lane == 31) {\n"
	"    rows[idx] = I[interval_begin]; \n"
	"    vals[tid] = (ValueType) 0.0;\n"
	"  }\n"
	"\n"
	"  for(int n = interval_begin + thread_lane; n < interval_end; n += WARP_SIZE) {\n"
	"    int row = I[n];\n"
	"    ValueType val = scalar * V[n] * x[J[n]];\n"
	"    \n"
	"    if (thread_lane == 0) {\n"
	"      if(row == rows[idx + 31])\n"
	"        val += vals[tid + 31];\n"
	"      else\n"
	"        y[rows[idx + 31]] += vals[tid + 31];\n"
	"    }\n"
	"\n"
	"    rows[idx] = row;\n"
	"    vals[tid] = val;\n"
	"\n"
	"    if (row == rows[idx -  1]) { vals[tid] = val = val + vals[tid -  1]; } \n"
	"    if (row == rows[idx -  2]) { vals[tid] = val = val + vals[tid -  2]; }\n"
	"    if (row == rows[idx -  4]) { vals[tid] = val = val + vals[tid -  4]; }\n"
	"    if (row == rows[idx -  8]) { vals[tid] = val = val + vals[tid -  8]; }\n"
	"    if (row == rows[idx - 16]) { vals[tid] = val = val + vals[tid - 16]; }\n"
	"\n"
	"    if(thread_lane < 31 && row != rows[idx + 1])\n"
	"      y[row] += vals[tid];\n"
	"  }\n"
	"\n"
	"  if (thread_lane == 31) {\n"
	"    temp_rows[warp_id] = rows[idx];\n"
	"    temp_vals[warp_id] = vals[tid];\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
// ----------------------------------------------------------
// function kernel_spmv_coo_reduce_update(...)
// ----------------------------------------------------------
// Modified and adopted from CUSP 0.3.1, 
// http://code.google.com/p/cusp-library/
// NVIDIA, APACHE LICENSE 2.0
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
	"__kernel void kernel_coo_spmv_reduce_update(         const       int  num_warps,\n"
	"                                            __global const       int *temp_rows,\n"
	"                                            __global const ValueType *temp_vals,\n"
	"                                            __global       ValueType *y) {\n"
	"\n"
	"  __local int rows[BLOCK_SIZE + 1];\n"
	"  __local ValueType vals[BLOCK_SIZE + 1];\n"
	"\n"
	"  int tid = get_local_id(0);\n"
	"\n"
	"  const int end = num_warps - (num_warps & (BLOCK_SIZE - 1));\n"
	"\n"
	"  if (tid == 0) {\n"
	"    rows[BLOCK_SIZE] = (int) -1;\n"
	"    vals[BLOCK_SIZE] = (ValueType) 0.0;\n"
	"  }\n"
	"  \n"
	"  barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"  int i = tid;\n"
	"\n"
	"  while (i < end) {\n"
	"    rows[tid] = temp_rows[i];\n"
	"    vals[tid] = temp_vals[i];\n"
	"\n"
	"    barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"    segreduce_block(rows, vals);\n"
	"\n"
	"    if (rows[tid] != rows[tid + 1])\n"
	"      y[rows[tid]] += vals[tid];\n"
	"\n"
	"    barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"    i += BLOCK_SIZE; \n"
	"  }\n"
	"\n"
	"  if (end < num_warps) {\n"
	"    if (i < num_warps) {\n"
	"      rows[tid] = temp_rows[i];\n"
	"      vals[tid] = temp_vals[i];\n"
	"    } else {\n"
	"      rows[tid] = (int) -1;\n"
	"      vals[tid] = (ValueType) 0.0;\n"
	"    }\n"
	"\n"
	"    barrier(CLK_LOCAL_MEM_FENCE);\n"
	" \n"
	"    segreduce_block(rows, vals);\n"
	"\n"
	"    if (i < num_warps)\n"
	"      if (rows[tid] != rows[tid + 1])\n"
	"        y[rows[tid]] += vals[tid];\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
// ----------------------------------------------------------
// function spmv_coo_serial_kernel(...)
// ----------------------------------------------------------
// Modified and adopted from CUSP 0.3.1, 
// http://code.google.com/p/cusp-library/
// NVIDIA, APACHE LICENSE 2.0
// ----------------------------------------------------------
// CHANGELOG
// - adopted interface
// ----------------------------------------------------------
	"__kernel void kernel_coo_spmv_serial(         const       int  num_entries,\n"
	"                                     __global const       int *I, \n"
	"                                     __global const       int *J, \n"
	"                                     __global const ValueType *V, \n"
	"                                              const ValueType  scalar,\n"
	"                                     __global const ValueType *x, \n"
	"                                     __global       ValueType *y,\n"
	"                                              const       int  shift) {\n"
	"\n"
	"  for(int n = 0; n < num_entries-shift; n++)\n"
	"    y[I[n+shift]] += scalar * V[n+shift] * x[J[n+shift]];\n"
	"\n"
	"}\n"
	"\n"
	"\n"
;
}

#endif // PARALUTION_OCL_KERNELS_COO_HPP_
