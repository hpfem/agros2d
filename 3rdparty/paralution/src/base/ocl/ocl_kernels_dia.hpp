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


#ifndef PARALUTION_OCL_KERNELS_DIA_HPP_
#define PARALUTION_OCL_KERNELS_DIA_HPP_

namespace paralution {

const char *ocl_kernels_dia =
// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
	"__kernel void kernel_dia_spmv(const int num_rows, \n"
	"                              const int num_cols, \n"
	"                              const int num_diags,\n"
	"                              __global const int *Aoffsets,\n"
	"                              __global const ValueType *Aval, \n"
	"                              __global const ValueType *x, \n"
	"                              __global       ValueType *y)\n"
	"{\n"
	"\n"
	"  int row = get_global_id(0);\n"
	"\n"
	"  if (row < num_rows) {\n"
	"\n"
	"    ValueType sum = 0.0;\n"
	"\n"
	"    for (int n=0; n<num_diags; ++n) {\n"
	"\n"
	"      const int ind = n * num_rows + row;\n"
	"      const int col = row + Aoffsets[n];\n"
	"      \n"
	"      if ((col >= 0) && (col < num_cols))\n"
	"        sum += Aval[ind] * x[col];\n"
	"\n"
	"    }\n"
	"        \n"
	"    y[row] = sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
	"__kernel void kernel_dia_add_spmv(const int num_rows,\n"
	"                                  const int num_cols,\n"
	"                                  const int num_diags,\n"
	"                                  __global const int *Aoffsets,\n"
	"                                  __global const ValueType *Aval, \n"
	"                                           const ValueType scalar,\n"
	"                                  __global const ValueType *x, \n"
	"                                  __global       ValueType *y)\n"
	"{\n"
	"\n"
	"  int row = get_global_id(0);\n"
	"\n"
	"  if (row < num_rows) {\n"
	"\n"
	"    ValueType sum = 0.0;\n"
	"\n"
	"    for (int n=0; n<num_diags; ++n) {\n"
	"\n"
	"      const int ind = n * num_rows + row;\n"
	"      const int col = row + Aoffsets[n];\n"
	"      \n"
	"      if ((col >= 0) && (col < num_cols))\n"
	"        sum += Aval[ind] * x[col];\n"
	"\n"
	"    }\n"
	"\n"
	"    y[row] += scalar*sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
;
}

#endif // PARALUTION_OCL_KERNELS_DIA_HPP_
