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

__kernel void kernel_dense_spmv(const int nrow, const int ncol, __global const ValueType *val,
                                __global const ValueType *in, __global ValueType *out) {

  int ai = get_global_id(0);

  if (ai < nrow) {

    ValueType sum = (ValueType)(0.0);

    for (int aj=0; aj<ncol; ++aj)
      sum += val[ai+aj*nrow] * in[aj];

    out[ai] = sum;

  }

}
