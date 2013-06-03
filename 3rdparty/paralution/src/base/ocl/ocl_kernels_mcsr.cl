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

__kernel void kernel_mcsr_spmv_scalar(const int nrow, __global const int *row_offset, 
                                      __global const int *col, __global const ValueType *val, 
                                      __global const ValueType *in, __global ValueType *out) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

    ValueType sum = val[ai] * in[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      sum += val[aj] * in[col[aj]];

    out[ai] = sum;

  }

}


__kernel void kernel_mcsr_add_spmv_scalar(const int nrow, __global const int *row_offset, 
                                          __global const int *col, __global const ValueType *val, 
                                          const ValueType scalar, __global const ValueType *in,
                                          __global ValueType *out) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

    out[ai] += scalar * val[ai] * in[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      out[ai] += scalar * val[aj] * in[col[aj]];

  }

}

