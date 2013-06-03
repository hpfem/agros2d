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
__kernel void kernel_dia_spmv(const int num_rows, 
                              const int num_cols, 
                              const int num_diags,
                              __global const int *Aoffsets,
                              __global const ValueType *Aval, 
                              __global const ValueType *x, 
                              __global       ValueType *y)
{

  int row = get_global_id(0);

  if (row < num_rows) {

    ValueType sum = 0.0;

    for (int n=0; n<num_diags; ++n) {

      const int ind = n * num_rows + row;
      const int col = row + Aoffsets[n];
      
      if ((col >= 0) && (col < num_cols))
        sum += Aval[ind] * x[col];

    }
        
    y[row] = sum;

  }

}


// Nathan Bell and Michael Garland
// Efficient Sparse Matrix-Vector Multiplication on {CUDA}
// NVR-2008-004 / NVIDIA Technical Report
__kernel void kernel_dia_add_spmv(const int num_rows,
                                  const int num_cols,
                                  const int num_diags,
                                  __global const int *Aoffsets,
                                  __global const ValueType *Aval, 
                                           const ValueType scalar,
                                  __global const ValueType *x, 
                                  __global       ValueType *y)
{

  int row = get_global_id(0);

  if (row < num_rows) {

    ValueType sum = 0.0;

    for (int n=0; n<num_diags; ++n) {

      const int ind = n * num_rows + row;
      const int col = row + Aoffsets[n];
      
      if ((col >= 0) && (col < num_cols))
        sum += Aval[ind] * x[col];

    }

    y[row] += scalar*sum;

  }

}

