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


#ifndef PARALUTION_GPU_CUDA_KERNELS_COO_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_COO_HPP_

#include "../matrix_formats_ind.hpp"

namespace paralution {

template <typename ValueType, typename IndexType>
__global__ void kernel_coo_permute(const IndexType nnz, 
                                   const IndexType *in_row, const IndexType *in_col, 
                                   const IndexType *perm,
                                   IndexType *out_row,  IndexType *out_col) {


  IndexType ind = blockIdx.x*blockDim.x+threadIdx.x;

  for (int i=ind; i<nnz; i+=gridDim.x) {

    out_row[i] = perm[ in_row[i] ];
    out_col[i] = perm[ in_col[i] ];

  }

}

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
template <typename IndexType, typename ValueType>
__device__ ValueType segreduce_warp(const IndexType thread_lane, IndexType row, ValueType val, IndexType * rows, ValueType * vals)
{
  rows[threadIdx.x] = row;
  vals[threadIdx.x] = val;
  
  if( thread_lane >=  1 && row == rows[threadIdx.x -  1] ) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  1]; } 
  if( thread_lane >=  2 && row == rows[threadIdx.x -  2] ) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  2]; }
  if( thread_lane >=  4 && row == rows[threadIdx.x -  4] ) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  4]; }
  if( thread_lane >=  8 && row == rows[threadIdx.x -  8] ) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  8]; }
  if( thread_lane >= 16 && row == rows[threadIdx.x - 16] ) { vals[threadIdx.x] = val = val + vals[threadIdx.x - 16]; }
  
  return val;
}

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
template <typename IndexType, typename ValueType>
__device__ void segreduce_block(const IndexType * idx, ValueType * val)
{
  ValueType left = 0;
  if( threadIdx.x >=   1 && idx[threadIdx.x] == idx[threadIdx.x -   1] ) { left = val[threadIdx.x -   1]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();  
  if( threadIdx.x >=   2 && idx[threadIdx.x] == idx[threadIdx.x -   2] ) { left = val[threadIdx.x -   2]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
  if( threadIdx.x >=   4 && idx[threadIdx.x] == idx[threadIdx.x -   4] ) { left = val[threadIdx.x -   4]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
  if( threadIdx.x >=   8 && idx[threadIdx.x] == idx[threadIdx.x -   8] ) { left = val[threadIdx.x -   8]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
  if( threadIdx.x >=  16 && idx[threadIdx.x] == idx[threadIdx.x -  16] ) { left = val[threadIdx.x -  16]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
  if( threadIdx.x >=  32 && idx[threadIdx.x] == idx[threadIdx.x -  32] ) { left = val[threadIdx.x -  32]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();  
  if( threadIdx.x >=  64 && idx[threadIdx.x] == idx[threadIdx.x -  64] ) { left = val[threadIdx.x -  64]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
  if( threadIdx.x >= 128 && idx[threadIdx.x] == idx[threadIdx.x - 128] ) { left = val[threadIdx.x - 128]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
  if( threadIdx.x >= 256 && idx[threadIdx.x] == idx[threadIdx.x - 256] ) { left = val[threadIdx.x - 256]; } __syncthreads(); val[threadIdx.x] += left; left = 0; __syncthreads();
}


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
template <typename IndexType, typename ValueType, unsigned int BLOCK_SIZE, unsigned int WARP_SIZE>
__launch_bounds__(BLOCK_SIZE,1)
__global__ void
kernel_spmv_coo_flat(const IndexType num_nonzeros,
                     const IndexType interval_size,
                     const IndexType * I, 
                     const IndexType * J, 
                     const ValueType * V, 
                     const ValueType scalar,
                     const ValueType * x, 
                           ValueType * y,
                           IndexType * temp_rows,
                           ValueType * temp_vals)
{
    __shared__ volatile IndexType rows[48 *(BLOCK_SIZE/32)];
    __shared__ volatile ValueType vals[BLOCK_SIZE];

    const IndexType thread_id   = BLOCK_SIZE * blockIdx.x + threadIdx.x;                         // global thread index
    const IndexType thread_lane = threadIdx.x & (WARP_SIZE-1);                                   // thread index within the warp
    const IndexType warp_id     = thread_id   / WARP_SIZE;                                       // global warp index

    const IndexType interval_begin = warp_id * interval_size;                                    // warp's offset into I,J,V
    IndexType interval_end2 = interval_begin + interval_size;
    if (interval_end2 > num_nonzeros)
      interval_end2 = num_nonzeros;

    const IndexType interval_end   = interval_end2; // min(interval_begin + interval_size, num_nonzeros);  // end of warps's work

    const IndexType idx = 16 * (threadIdx.x/32 + 1) + threadIdx.x;                               // thread's index into padded rows array

    rows[idx - 16] = -1;                                                                         // fill padding with invalid row index

    if(interval_begin >= interval_end)                                                           // warp has no work to do 
        return;

    if (thread_lane == 31)
    {
        // initialize the carry in values
        rows[idx] = I[interval_begin]; 
	vals[threadIdx.x] = ValueType(0);
    }
  
    for(IndexType n = interval_begin + thread_lane; n < interval_end; n += WARP_SIZE)
    {
        IndexType row = I[n];                                         // row index (i)
        ValueType val = scalar * V[n] * x[J[n]]; // fetch_x<UseCache>(J[n], x);            // A(i,j) * x(j)
        
        if (thread_lane == 0)
        {
            if(row == rows[idx + 31])
                val += vals[threadIdx.x + 31];                        // row continues
            else
                y[rows[idx + 31]] += vals[threadIdx.x + 31];  // row terminated
        }
        
        rows[idx]         = row;
        vals[threadIdx.x] = val;

        if(row == rows[idx -  1]) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  1]; } 
        if(row == rows[idx -  2]) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  2]; }
        if(row == rows[idx -  4]) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  4]; }
        if(row == rows[idx -  8]) { vals[threadIdx.x] = val = val + vals[threadIdx.x -  8]; }
        if(row == rows[idx - 16]) { vals[threadIdx.x] = val = val + vals[threadIdx.x - 16]; }

        if(thread_lane < 31 && row != rows[idx + 1])
            y[row] += vals[threadIdx.x];                                            // row terminated
    }

    if(thread_lane == 31)
    {
        // write the carry out values
        temp_rows[warp_id] = rows[idx];
        temp_vals[warp_id] = vals[threadIdx.x];
    }
}



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
template <typename IndexType, typename ValueType, unsigned int BLOCK_SIZE>
  __launch_bounds__(BLOCK_SIZE,1)
__global__ void  kernel_spmv_coo_reduce_update(const IndexType num_warps,
                                               const IndexType * temp_rows,
                                               const ValueType * temp_vals,
                                               ValueType * y)
  {
    __shared__ IndexType rows[BLOCK_SIZE + 1];    
    __shared__ ValueType vals[BLOCK_SIZE + 1];    

    const IndexType end = num_warps - (num_warps & (BLOCK_SIZE - 1));

    if (threadIdx.x == 0)
      {
        rows[BLOCK_SIZE] = (IndexType) -1;
        vals[BLOCK_SIZE] = (ValueType)  0;
      }
    
    __syncthreads();

    IndexType i = threadIdx.x;

    while (i < end)
      {
        // do full blocks
        rows[threadIdx.x] = temp_rows[i];
        vals[threadIdx.x] = temp_vals[i];

        __syncthreads();

        segreduce_block(rows, vals);

        if (rows[threadIdx.x] != rows[threadIdx.x + 1])
          y[rows[threadIdx.x]] += vals[threadIdx.x];

        __syncthreads();

        i += BLOCK_SIZE; 
      }

    if (end < num_warps){
      if (i < num_warps){
        rows[threadIdx.x] = temp_rows[i];
        vals[threadIdx.x] = temp_vals[i];
      } else {
        rows[threadIdx.x] = (IndexType) -1;
        vals[threadIdx.x] = (ValueType)  0;
      }

      __syncthreads();
   
      segreduce_block(rows, vals);

      if (i < num_warps)
        if (rows[threadIdx.x] != rows[threadIdx.x + 1])
          y[rows[threadIdx.x]] += vals[threadIdx.x];
    }
  }


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
template <typename IndexType, typename ValueType>
__global__ void
kernel_spmv_coo_serial(const IndexType num_entries,
                       const IndexType * I, 
                       const IndexType * J, 
                       const ValueType * V, 
                       const ValueType scalar,
                       const ValueType * x, 
                       ValueType * y)
{
  for(IndexType n = 0; n < num_entries; n++)
    {
      y[I[n]] += scalar*V[n] * x[J[n]];
    }
}

  
}

#endif

