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


#include "ocl_allocate_free.hpp"
#include "../../utils/log.hpp"
#include "ocl_utils.hpp"

#include <assert.h>


namespace paralution {


// Allocate memory on device
template <typename DataType>
void allocate_ocl(const int size, cl_context ocl_context, cl_mem **ptr) {

  LOG_DEBUG(0, "allocate_ocl()",
            size);

  if (size > 0) {

    assert(*ptr == NULL);

    cl_int err;

    // Allocate memory on device
    *ptr = new cl_mem;
    **ptr = clCreateBuffer(ocl_context,
                           CL_MEM_READ_WRITE,
                           sizeof(DataType)*size,
                           NULL,
                           &err );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    assert(*ptr != NULL);

  }

}


// Free memory on device
void free_ocl(cl_mem **ptr) {

  LOG_DEBUG(0, "free_ocl()",
            "");

  cl_int err;

  // Free memory on device
  err = clReleaseMemObject(**ptr);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // since we allocated cl_mem object with new
  delete *ptr;

  *ptr = NULL;

}


// Set object on device to specific value
template<typename DataType>
void ocl_set_to(cl_kernel ocl_kernel,
                cl_command_queue ocl_cmdQueue,
                const size_t localWorkSize,
                const size_t globalWorkSize,
                const int size,
                const DataType val,
                cl_mem *ptr ) {

  LOG_DEBUG(0, "ocl_set_to()",
            size);

  assert(ptr != NULL);

  if (size > 0) {

    cl_int    err;
    cl_event  ocl_event;

    // Set kernel arguments
    err  = clSetKernelArg( ocl_kernel, 0, sizeof(int),      (void *) &size );
    err |= clSetKernelArg( ocl_kernel, 1, sizeof(DataType), (void *) &val );
    err |= clSetKernelArg( ocl_kernel, 2, sizeof(cl_mem),   (void *) ptr );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Start kernel run
    err = clEnqueueNDRangeKernel( ocl_cmdQueue, ocl_kernel, 1, NULL, &globalWorkSize,
                                  &localWorkSize, 0, NULL, &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

}


// Copy object from host to device memory
template <typename DataType>
void ocl_host2dev(const int size, const DataType *src, cl_mem *dst, cl_command_queue ocl_cmdQueue) {

  LOG_DEBUG(0, "ocl_host2dev()",
            size);

  if (size > 0) {

    assert(src != NULL);
    assert(dst != NULL);

    cl_int err;
    cl_event ocl_event;

    // Copy object from host to device memory
    err = clEnqueueWriteBuffer( ocl_cmdQueue, *dst, CL_TRUE, 0, size*sizeof(DataType), src, 0, NULL, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

}


// Copy object from device to host memory
template<typename DataType>
void ocl_dev2host(const int size, cl_mem *src, DataType *dst, cl_command_queue ocl_cmdQueue ) {

  LOG_DEBUG(0, "ocl_dev2host()",
            size);

  if (size > 0) {

    assert(*src != NULL);

    cl_int err;
    cl_event ocl_event;

    // Copy object from device to host memory
    err = clEnqueueReadBuffer( ocl_cmdQueue, *src, CL_TRUE, 0, size*sizeof(DataType), dst, 0, NULL, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

}


// Copy object from device to device memory (internal copy)
template<typename DataType>
void ocl_dev2dev(const int size, cl_mem *src, cl_mem *dst, cl_command_queue ocl_cmdQueue) {

  LOG_DEBUG(0, "ocl_dev2dev()",
            size);

  if (size > 0) {

    assert(*src != NULL);

    cl_int err;
    cl_event ocl_event;

    // Copy object from device to device memory (internal copy)
    err = clEnqueueCopyBuffer( ocl_cmdQueue, *src, *dst, 0, 0, size*sizeof(DataType), 0, NULL, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

}


template void allocate_ocl<float       >(const int size, cl_context ocl_context, cl_mem **ptr);
template void allocate_ocl<double      >(const int size, cl_context ocl_context, cl_mem **ptr);
template void allocate_ocl<int         >(const int size, cl_context ocl_context, cl_mem **ptr);
template void allocate_ocl<unsigned int>(const int size, cl_context ocl_context, cl_mem **ptr);
template void allocate_ocl<char        >(const int size, cl_context ocl_context, cl_mem **ptr);

template void ocl_set_to<float       >(cl_kernel ocl_kernel, cl_command_queue ocl_cmdQueue, const size_t localWorkSize,
                                       const size_t globalWorkSize, const int size, const float val, cl_mem *ptr);
template void ocl_set_to<double      >(cl_kernel ocl_kernel, cl_command_queue ocl_cmdQueue, const size_t localWorkSize,
                                       const size_t globalWorkSize, const int size, const double val, cl_mem *ptr);
template void ocl_set_to<int         >(cl_kernel ocl_kernel, cl_command_queue ocl_cmdQueue, const size_t localWorkSize,
                                       const size_t globalWorkSize, const int size, const int val, cl_mem *ptr);
template void ocl_set_to<unsigned int>(cl_kernel ocl_kernel, cl_command_queue ocl_cmdQueue, const size_t localWorkSize,
                                       const size_t globalWorkSize, const int size, const unsigned int val, cl_mem *ptr);
template void ocl_set_to<char        >(cl_kernel ocl_kernel, cl_command_queue ocl_cmdQueue, const size_t localWorkSize,
                                       const size_t globalWorkSize, const int size, const char val, cl_mem *ptr);

template void ocl_host2dev<float       >(const int size, const float        *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_host2dev<double      >(const int size, const double       *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_host2dev<int         >(const int size, const int          *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_host2dev<unsigned int>(const int size, const unsigned int *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_host2dev<char        >(const int size, const char         *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);

template void ocl_dev2host<float       >(const int size, cl_mem *src, float        *dst, cl_command_queue ocl_cmdQueue );
template void ocl_dev2host<double      >(const int size, cl_mem *src, double       *dst, cl_command_queue ocl_cmdQueue );
template void ocl_dev2host<int         >(const int size, cl_mem *src, int          *dst, cl_command_queue ocl_cmdQueue );
template void ocl_dev2host<unsigned int>(const int size, cl_mem *src, unsigned int *dst, cl_command_queue ocl_cmdQueue );
template void ocl_dev2host<char        >(const int size, cl_mem *src, char         *dst, cl_command_queue ocl_cmdQueue );

template void ocl_dev2dev<float       >(const int size, cl_mem *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_dev2dev<double      >(const int size, cl_mem *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_dev2dev<int         >(const int size, cl_mem *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_dev2dev<unsigned int>(const int size, cl_mem *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);
template void ocl_dev2dev<char        >(const int size, cl_mem *src, cl_mem *dst, cl_command_queue ocl_cmdQueue);

}

