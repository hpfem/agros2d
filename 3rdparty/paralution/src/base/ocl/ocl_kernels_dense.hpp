#ifndef PARALUTION_OCL_KERNELS_DENSE_HPP_
#define PARALUTION_OCL_KERNELS_DENSE_HPP_

namespace paralution {

const char *ocl_kernels_dense =
	"// *************************************************************************\n"
	"//\n"
	"//    PARALUTION   www.paralution.com\n"
	"//\n"
	"//    Copyright (C) 2012-2013 Dimitar Lukarski\n"
	"//\n"
	"//    This program is free software: you can redistribute it and/or modify\n"
	"//    it under the terms of the GNU General Public License as published by\n"
	"//    the Free Software Foundation, either version 3 of the License, or\n"
	"//    (at your option) any later version.\n"
	"//\n"
	"//    This program is distributed in the hope that it will be useful,\n"
	"//    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"//    GNU General Public License for more details.\n"
	"//\n"
	"//    You should have received a copy of the GNU General Public License\n"
	"//    along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
	"//\n"
	"// *************************************************************************\n"
	"\n"
	"__kernel void kernel_dense_spmv(const int nrow, const int ncol, __global const ValueType *val,\n"
	"                                __global const ValueType *in, __global ValueType *out) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    ValueType sum = (ValueType)(0.0);\n"
	"\n"
	"    for (int aj=0; aj<ncol; ++aj)\n"
	"      sum += val[ai+aj*nrow] * in[aj];\n"
	"\n"
	"    out[ai] = sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n";
}

#endif // PARALUTION_OCL_KERNELS_DENSE_HPP_
