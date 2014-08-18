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


#ifndef PARALUTION_GPU_ALLOCATE_FREE_HPP_
#define PARALUTION_GPU_ALLOCATE_FREE_HPP_

#include <iostream>

namespace paralution {

template <typename DataType>
void allocate_gpu(const int size, DataType **ptr);

template <typename DataType>
void free_gpu(DataType **ptr);

template <typename DataType>
void set_to_zero_gpu(const int blocksize,
                     const int max_threads,
                     const int size, DataType *ptr);

template <typename DataType>
void set_to_one_gpu(const int blocksize,
                    const int max_threads,
                    const int size, DataType *ptr);


}

#endif // PARALUTION_GPU_ALLOCATE_FREE_HPP_

