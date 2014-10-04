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


#ifndef PARALUTION_BACKEND_GPU_HPP_
#define PARALUTION_BACKEND_GPU_HPP_

#include "../backend_manager.hpp"
#include <iostream>


namespace paralution {

template <typename ValueType>
class AcceleratorVector;
template <typename ValueType>
class AcceleratorMatrix;
template <typename ValueType>
class HostMatrix;

/// Initialize a GPU (CUDA, CUBLAS, CUSPARSE)
bool paralution_init_gpu();
/// Release the GPU resources (CUDA, CUBLAS, CUSPARSE)
void paralution_stop_gpu();

/// Print information about the GPUs in the systems
void paralution_info_gpu(const struct Paralution_Backend_Descriptor);

/// Sync the device (for async transfers)
void paralution_gpu_sync(void);

/// Build (and return) a vector on GPU
template <typename ValueType>
AcceleratorVector<ValueType>* _paralution_init_base_gpu_vector(const struct Paralution_Backend_Descriptor backend_descriptor);

/// Build (and return) a matrix on GPU
template <typename ValueType>
AcceleratorMatrix<ValueType>* _paralution_init_base_gpu_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                               const unsigned int matrix_format);

}

#endif // PARALUTION_BACKEND_GPU_HPP_

