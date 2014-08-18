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


#ifndef PARALUTION_OCL_KERNELS_GENERAL_HPP_
#define PARALUTION_OCL_KERNELS_GENERAL_HPP_

namespace paralution {

const char *ocl_kernels_general =
	"__kernel void kernel_set_to(const int size, const ValueType val, __global ValueType *data) {\n"
	"\n"
	"  int gid = get_global_id(0);\n"
	"\n"
	"  if (gid < size)\n"
	"    data[gid] = val;\n"
	"\n"
	"}\n"
	"\n"
	"__kernel void kernel_red_recurse(__global int *dst, __global int *src, int numElems) {\n"
	"\n"
	"  int index = BLOCK_SIZE * get_global_id(0);\n"
	"\n"
	"  if (index >= numElems)\n"
	"    return;\n"
	"\n"
	"  int i = index;\n"
	"\n"
	"  if (i < BLOCK_SIZE)\n"
	"    return;\n"
	"\n"
	"  int a = 0;\n"
	"\n"
	"  while (i >= BLOCK_SIZE) {\n"
	"    a += src[i];\n"
	"    i -= BLOCK_SIZE;\n"
	"  }\n"
	"\n"
	"  dst[index] = a;\n"
	"\n"
	"}\n"
	"\n"
	"__kernel void kernel_red_partial_sum(__global int *dst, __global const int *src,\n"
	"                                        const int numElems,      const int shift) {\n"
	"\n"
	"  int index = get_global_id(0);\n"
	"  int tid   = get_local_id(0);\n"
	"  int gid   = get_group_id(0);\n"
	"\n"
	"  if (index < numElems) {\n"
	"\n"
	"    __local int data[BLOCK_SIZE];\n"
	"\n"
	"    data[tid] = src[index];\n"
	"\n"
	"    barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"    for (int i = BLOCK_SIZE/2; i > 0; i/=2) {\n"
	"\n"
	"      if (tid < i)\n"
	"        data[tid] = data[tid] + data[tid+i];\n"
	"\n"
	"      barrier(CLK_LOCAL_MEM_FENCE);\n"
	"\n"
	"    }\n"
	"\n"
	"    if (tid == 0 && BLOCK_SIZE*(1+gid)-1<numElems)\n"
	"      dst[BLOCK_SIZE*(1+gid)-1+shift] = data[0];\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"__kernel void kernel_red_extrapolate(__global       int *dst,\n"
	"                                     __global const int *srcBorder,\n"
	"                                     __global const int *srcData,\n"
	"                                                    int  numElems,\n"
	"                                              const int  shift) {\n"
	"\n"
	"  int index = get_local_size(0) * get_local_id(0);\n"
	"\n"
	"  if (index < numElems-1) {\n"
	"\n"
	"    int sum = srcBorder[index];\n"
	"\n"
	"    for(int i = 0; i < get_local_size(0) && index+i<numElems; ++i) {\n"
	"      sum += srcData[index+i];\n"
	"      dst[index+i+shift] = sum;\n"
	"    }\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
;
}

#endif // PARALUTION_OCL_KERNELS_GENERAL_HPP_
