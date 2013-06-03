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

#ifndef PARALUTION_BACKEND_OCL_HPP_
#define PARALUTION_BACKEND_OCL_HPP_

#include "../backend_manager.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


namespace paralution {

template <typename ValueType>
class AcceleratorVector;
template <typename ValueType>
class AcceleratorMatrix;
template <typename ValueType>
class HostMatrix;

// OCL handle type that contains all the relevant
// OpenCL data such as device, kernels, etc.
struct oclHandle_t {

  // OCL context
  cl_context OCL_context;

  // OCL command queue
  cl_command_queue OCL_cmdQueue;

  // OCL program for single precision
  cl_program OCL_program_sp;
  // OCL program for double precision
  cl_program OCL_program_dp;
  // OCL program for int
  cl_program OCL_program_int;

  // OCL platform array
  cl_platform_id *OCL_platforms;

  // OCL device array
  cl_device_id *OCL_devices;

  // OCL kernel array
  cl_kernel OCL_kernel[100];

};

/// Set all OpenCL kernels
void paralution_set_ocl_kernels();
/// Get OpenCL kernel
template <typename ValueType>
cl_kernel paralution_get_ocl_kernel(int);
/// Initialize OpenCL
bool paralution_init_ocl();
/// Release the OpenCL resources
void paralution_stop_ocl();

/// Print information about the GPUs in the systems
void paralution_info_ocl(const struct Paralution_Backend_Descriptor);


/// Build (and return) an OpenCL vector
template <typename ValueType>
AcceleratorVector<ValueType>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);

/// Build (and return) an OpenCL matrix
template <typename ValueType>
AcceleratorMatrix<ValueType>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                               const unsigned int matrix_format);

};

#endif // PARALUTION_BACKEND_OCL_HPP_
