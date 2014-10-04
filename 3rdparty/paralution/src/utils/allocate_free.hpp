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


#ifndef PARALUTION_UTILS_ALLOCATE_FREE_HPP_
#define PARALUTION_UTILS_ALLOCATE_FREE_HPP_

#include <iostream>

// When CUDA backend is available the host memory allocation
// can use cudaMallocHost() function for pinned memory
// thus the memory transfers to the GPU are faster
// and this also enables the async transfers.
// Uncomment to use pinned memory
//
// To force standard CPU allocation comment the following line 
// #define PARALUTION_CUDA_PINNED_MEMORY

namespace paralution {

/// Allocate buffer on the host
template <typename DataType>
void allocate_host(const int size, DataType **ptr);

/// Free buffer on the host
template <typename DataType>
void free_host(DataType **ptr);

/// set a buffer to zero on the host
template <typename DataType>
void set_to_zero_host(const int size, DataType *ptr);

}

#endif // PARALUTION_UTILS_ALLOCATE_FREE_HPP_

