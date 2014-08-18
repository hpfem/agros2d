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


#ifndef PARALUTION_OCL_ALLOCATE_FREE_HPP_
#define PARALUTION_OCL_ALLOCATE_FREE_HPP_

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


namespace paralution {

/// Allocate device memory
template <typename DataType>
void allocate_ocl(const int, cl_context, cl_mem**);

/// Free device memory
void free_ocl(cl_mem**);

/// Set device object to specific values
template <typename DataType>
void ocl_set_to(cl_kernel, cl_command_queue, const size_t, const size_t, const int, const DataType, cl_mem*);

/// Copy object from host to device memory
template <typename DataType>
void ocl_host2dev(const int, const DataType*, cl_mem*, cl_command_queue);

/// Copy object from device to host memory
template <typename DataType>
void ocl_dev2host(const int, cl_mem*, DataType*, cl_command_queue);

/// Copy object from device to device (intra) memory
template <typename DataType>
void ocl_dev2dev(const int, cl_mem*, cl_mem*, cl_command_queue);


}

#endif // PARALUTION_OCL_ALLOCATE_FREE_HPP_

