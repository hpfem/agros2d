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


#ifndef PARALUTION_OCL_OCL_UTILS_HPP_
#define PARALUTION_OCL_OCL_UTILS_HPP_

#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "backend_ocl.hpp"

#if defined(__APPLE__) && defined(__MACH__)
  #include <OpenCL/cl.h>
#else
  #include <CL/cl.h>
#endif


#define OCL_HANDLE(handle) static_cast<oclHandle_t*>(handle)

static const char *OCL_ERROR[] = {
  "CL_SUCCESS",
  "CL_DEVICE_NOT_FOUND",
  "CL_DEVICE_NOT_AVAILABLE",
  "CL_COMPILER_NOT_AVAILABLE",
  "CL_MEM_OBJECT_ALLOCATION_FAILURE",
  "CL_OUT_OF_RESOURCES",
  "CL_OUT_OF_HOST_MEMORY",
  "CL_PROFILING_INFO_NOT_AVAILABLE",
  "CL_MEM_COPY_OVERLAP",
  "CL_IMAGE_FORMAT_MISMATCH",
  "CL_IMAGE_FORMAT_NOT_SUPPORTED",
  "CL_BUILD_PROGRAM_FAILURE",
  "CL_MAP_FAILURE",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "CL_INVALID_VALUE",
  "CL_INVALID_DEVICE_TYPE",
  "CL_INVALID_PLATFORM",
  "CL_INVALID_DEVICE",
  "CL_INVALID_CONTEXT",
  "CL_INVALID_QUEUE_PROPERTIES",
  "CL_INVALID_COMMAND_QUEUE",
  "CL_INVALID_HOST_PTR",
  "CL_INVALID_MEM_OBJECT",
  "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
  "CL_INVALID_IMAGE_SIZE",
  "CL_INVALID_SAMPLER",
  "CL_INVALID_BINARY",
  "CL_INVALID_BUILD_OPTIONS",
  "CL_INVALID_PROGRAM",
  "CL_INVALID_PROGRAM_EXECUTABLE",
  "CL_INVALID_KERNEL_NAME",
  "CL_INVALID_KERNEL_DEFINITION",
  "CL_INVALID_KERNEL",
  "CL_INVALID_ARG_INDEX",
  "CL_INVALID_ARG_VALUE",
  "CL_INVALID_ARG_SIZE",
  "CL_INVALID_KERNEL_ARGS",
  "CL_INVALID_WORK_DIMENSION",
  "CL_INVALID_WORK_GROUP_SIZE",
  "CL_INVALID_WORK_ITEM_SIZE",
  "CL_INVALID_GLOBAL_OFFSET",
  "CL_INVALID_EVENT_WAIT_LIST",
  "CL_INVALID_EVENT",
  "CL_INVALID_OPERATION",
  "CL_INVALID_GL_OBJECT",
  "CL_INVALID_BUFFER_SIZE",
  "CL_INVALID_MIP_LEVEL",
  "CL_INVALID_GLOBAL_WORK_SIZE"
};

#define CHECK_OCL_ERROR(err_t, file, line) {                                                    \
  if ( err_t != CL_SUCCESS ) {                                                                  \
    LOG_INFO("OPENCL ERROR: " <<OCL_ERROR[-err_t]);                                             \
    LOG_INFO("File: " << file << "; line: " << line);                                           \
    paralution_stop_ocl();                                                                      \
    exit(1);                                                                                    \
  }                                                                                             \
}

#define CL_KERNEL_SET_TO_INT                        paralution_get_kernel_ocl<int>      (  0)
#define CL_KERNEL_SET_TO                            paralution_get_kernel_ocl<ValueType>(  1)
#define CL_KERNEL_REVERSE_INDEX                     paralution_get_kernel_ocl<ValueType>(  3)
#define CL_KERNEL_BUFFER_ADDSCALAR                  paralution_get_kernel_ocl<ValueType>(  5)
#define CL_KERNEL_SCALEADD                          paralution_get_kernel_ocl<ValueType>(  7)
#define CL_KERNEL_SCALEADDSCALE                     paralution_get_kernel_ocl<ValueType>(  9)
#define CL_KERNEL_SCALEADD2                         paralution_get_kernel_ocl<ValueType>( 11)
#define CL_KERNEL_POINTWISEMULT                     paralution_get_kernel_ocl<ValueType>( 13)
#define CL_KERNEL_POINTWISEMULT2                    paralution_get_kernel_ocl<ValueType>( 15)
#define CL_KERNEL_COPY_OFFSET_FROM                  paralution_get_kernel_ocl<ValueType>( 17)
#define CL_KERNEL_PERMUTE                           paralution_get_kernel_ocl<ValueType>( 19)
#define CL_KERNEL_PERMUTE_BACKWARD                  paralution_get_kernel_ocl<ValueType>( 21)
#define CL_KERNEL_DOT                               paralution_get_kernel_ocl<ValueType>( 23)
#define CL_KERNEL_AXPY                              paralution_get_kernel_ocl<ValueType>( 25)
#define CL_KERNEL_CSR_SPMV_SCALAR                   paralution_get_kernel_ocl<ValueType>( 27)
#define CL_KERNEL_CSR_ADD_SPMV_SCALAR               paralution_get_kernel_ocl<ValueType>( 29)
#define CL_KERNEL_CSR_SCALE_DIAGONAL                paralution_get_kernel_ocl<ValueType>( 31)
#define CL_KERNEL_CSR_SCALE_OFFDIAGONAL             paralution_get_kernel_ocl<ValueType>( 33)
#define CL_KERNEL_CSR_ADD_DIAGONAL                  paralution_get_kernel_ocl<ValueType>( 35)
#define CL_KERNEL_CSR_ADD_OFFDIAGONAL               paralution_get_kernel_ocl<ValueType>( 37)
#define CL_KERNEL_CSR_EXTRACT_DIAG                  paralution_get_kernel_ocl<ValueType>( 39)
#define CL_KERNEL_CSR_EXTRACT_INV_DIAG              paralution_get_kernel_ocl<ValueType>( 41)
#define CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ     paralution_get_kernel_ocl<ValueType>( 43)
#define CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY        paralution_get_kernel_ocl<ValueType>( 45)
#define CL_KERNEL_CSR_DIAGMATMULT                   paralution_get_kernel_ocl<ValueType>( 47)
#define CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT           paralution_get_kernel_ocl<ValueType>( 49)
#define CL_KERNEL_SCALE                             paralution_get_kernel_ocl<ValueType>( 51)
#define CL_KERNEL_MCSR_SPMV_SCALAR                  paralution_get_kernel_ocl<ValueType>( 53)
#define CL_KERNEL_MCSR_ADD_SPMV_SCALAR              paralution_get_kernel_ocl<ValueType>( 55)
#define CL_KERNEL_ELL_SPMV                          paralution_get_kernel_ocl<ValueType>( 57)
#define CL_KERNEL_ELL_ADD_SPMV                      paralution_get_kernel_ocl<ValueType>( 59)
#define CL_KERNEL_DIA_SPMV                          paralution_get_kernel_ocl<ValueType>( 61)
#define CL_KERNEL_DIA_ADD_SPMV                      paralution_get_kernel_ocl<ValueType>( 63)
#define CL_KERNEL_COO_PERMUTE                       paralution_get_kernel_ocl<ValueType>( 65)
#define CL_KERNEL_COO_SPMV_FLAT                     paralution_get_kernel_ocl<ValueType>( 67)
#define CL_KERNEL_COO_SPMV_REDUCE_UPDATE            paralution_get_kernel_ocl<ValueType>( 69)
#define CL_KERNEL_COO_SPMV_SERIAL                   paralution_get_kernel_ocl<ValueType>( 71)
#define CL_KERNEL_RED_RECURSE                       paralution_get_kernel_ocl<ValueType>( 73)
#define CL_KERNEL_RED_PARTIAL_SUM                   paralution_get_kernel_ocl<ValueType>( 75)
#define CL_KERNEL_RED_EXTRAPOLATE                   paralution_get_kernel_ocl<ValueType>( 77)
#define CL_KERNEL_CSR_PERMUTE_ROWS                  paralution_get_kernel_ocl<ValueType>( 79)
#define CL_KERNEL_CSR_PERMUTE_COLS                  paralution_get_kernel_ocl<ValueType>( 81)
#define CL_KERNEL_CSR_CALC_ROW_NNZ                  paralution_get_kernel_ocl<ValueType>( 83)
#define CL_KERNEL_CSR_PERMUTE_ROW_NNZ               paralution_get_kernel_ocl<ValueType>( 85)
#define CL_KERNEL_REDUCE                            paralution_get_kernel_ocl<ValueType>( 87)
#define CL_KERNEL_ELL_MAX_ROW                       paralution_get_kernel_ocl<ValueType>( 89)
#define CL_KERNEL_ELL_CSR_TO_ELL                    paralution_get_kernel_ocl<ValueType>( 91)
#define CL_KERNEL_ASUM                              paralution_get_kernel_ocl<ValueType>( 93)
#define CL_KERNEL_AMAX                              paralution_get_kernel_ocl<ValueType>( 95)
#define CL_KERNEL_DENSE_SPMV                        paralution_get_kernel_ocl<ValueType>( 97)
#define CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR          paralution_get_kernel_ocl<ValueType>( 99)
#define CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW            paralution_get_kernel_ocl<ValueType>(101)
#define CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW            paralution_get_kernel_ocl<ValueType>(103)
#define CL_KERNEL_CSR_LOWER_NNZ_PER_ROW             paralution_get_kernel_ocl<ValueType>(105)
#define CL_KERNEL_CSR_UPPER_NNZ_PER_ROW             paralution_get_kernel_ocl<ValueType>(107)
#define CL_KERNEL_CSR_COMPRESS_COUNT_NROW           paralution_get_kernel_ocl<ValueType>(109)
#define CL_KERNEL_CSR_COMPRESS_COPY                 paralution_get_kernel_ocl<ValueType>(111)
#define CL_KERNEL_SCALEADDSCALE_OFFSET              paralution_get_kernel_ocl<ValueType>(113)
#define CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR          paralution_get_kernel_ocl<ValueType>(115)


#endif // PARALUTION_OCL_OCL_UTILS_HPP_

