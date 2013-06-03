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

__kernel void kernel_set_to(const int size, const ValueType val, __global ValueType *data) {

  int gid = get_global_id(0);

  if (gid < size)
    data[gid] = val;

}

__kernel void kernel_red_recurse(__global int *dst, __global int *src, int numElems) {

  int index = BLOCK_SIZE * get_global_id(0);

  if (index >= numElems)
    return;

  int i = index;

  if (i < BLOCK_SIZE)
    return;

  int a = 0;

  while (i >= BLOCK_SIZE) {
    a += src[i];
    i -= BLOCK_SIZE;
  }

  dst[index] = a;

}

__kernel void kernel_red_partial_sum(__global int *dst, __global const int *src,
                                        const int numElems,      const int shift) {

  int index = get_global_id(0);
  int tid   = get_local_id(0);
  int gid   = get_group_id(0);

  if (index < numElems) {

    __local int data[BLOCK_SIZE];

    data[tid] = src[index];

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i/=2) {

      if (tid < i)
        data[tid] = data[tid] + data[tid+i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0 && BLOCK_SIZE*(1+gid)-1<numElems)
      dst[BLOCK_SIZE*(1+gid)-1+shift] = data[0];

  }

}

__kernel void kernel_red_extrapolate(__global       int *dst,
                                     __global const int *srcBorder,
                                     __global const int *srcData,
                                                    int  numElems,
                                              const int  shift) {

  int index = get_local_size(0) * get_local_id(0);

  if (index < numElems-1) {

    int sum = srcBorder[index];

    for(int i = 0; i < get_local_size(0) && index+i<numElems; ++i) {
      sum += srcData[index+i];
      dst[index+i+shift] = sum;
    }

  }

}

