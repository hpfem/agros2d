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

#define CL_KERNEL_SET_TO_INT                        paralution_get_ocl_kernel<int>      ( 0)
#define CL_KERNEL_SET_TO                            paralution_get_ocl_kernel<ValueType>( 1)
#define CL_KERNEL_REVERSE_INDEX                     paralution_get_ocl_kernel<ValueType>( 3)
#define CL_KERNEL_BUFFER_ADDSCALAR                  paralution_get_ocl_kernel<ValueType>( 5)
#define CL_KERNEL_SCALEADD                          paralution_get_ocl_kernel<ValueType>( 8)
#define CL_KERNEL_SCALEADDSCALE                     paralution_get_ocl_kernel<ValueType>(10)
#define CL_KERNEL_SCALEADD2                         paralution_get_ocl_kernel<ValueType>(12)
#define CL_KERNEL_POINTWISEMULT                     paralution_get_ocl_kernel<ValueType>(14)
#define CL_KERNEL_POINTWISEMULT2                    paralution_get_ocl_kernel<ValueType>(16)
#define CL_KERNEL_COPY_OFFSET_FROM                  paralution_get_ocl_kernel<ValueType>(18)
#define CL_KERNEL_PERMUTE                           paralution_get_ocl_kernel<ValueType>(20)
#define CL_KERNEL_PERMUTE_BACKWARD                  paralution_get_ocl_kernel<ValueType>(22)
#define CL_KERNEL_DOT                               paralution_get_ocl_kernel<ValueType>(24)
#define CL_KERNEL_AXPY                              paralution_get_ocl_kernel<ValueType>(26)
#define CL_KERNEL_CSR_SPMV_SCALAR                   paralution_get_ocl_kernel<ValueType>(28)
#define CL_KERNEL_CSR_ADD_SPMV_SCALAR               paralution_get_ocl_kernel<ValueType>(30)
#define CL_KERNEL_CSR_SCALE_DIAGONAL                paralution_get_ocl_kernel<ValueType>(32)
#define CL_KERNEL_CSR_SCALE_OFFDIAGONAL             paralution_get_ocl_kernel<ValueType>(34)
#define CL_KERNEL_CSR_ADD_DIAGONAL                  paralution_get_ocl_kernel<ValueType>(36)
#define CL_KERNEL_CSR_ADD_OFFDIAGONAL               paralution_get_ocl_kernel<ValueType>(38)
#define CL_KERNEL_CSR_EXTRACT_DIAG                  paralution_get_ocl_kernel<ValueType>(40)
#define CL_KERNEL_CSR_EXTRACT_INV_DIAG              paralution_get_ocl_kernel<ValueType>(42)
#define CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ     paralution_get_ocl_kernel<ValueType>(44)
#define CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY        paralution_get_ocl_kernel<ValueType>(46)
#define CL_KERNEL_CSR_DIAGMATMULT                   paralution_get_ocl_kernel<ValueType>(48)
#define CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT           paralution_get_ocl_kernel<ValueType>(50)
#define CL_KERNEL_SCALE                             paralution_get_ocl_kernel<ValueType>(52)
#define CL_KERNEL_MCSR_SPMV_SCALAR                  paralution_get_ocl_kernel<ValueType>(54)
#define CL_KERNEL_MCSR_ADD_SPMV_SCALAR              paralution_get_ocl_kernel<ValueType>(56)
#define CL_KERNEL_ELL_SPMV                          paralution_get_ocl_kernel<ValueType>(58)
#define CL_KERNEL_ELL_ADD_SPMV                      paralution_get_ocl_kernel<ValueType>(60)
#define CL_KERNEL_DIA_SPMV                          paralution_get_ocl_kernel<ValueType>(62)
#define CL_KERNEL_DIA_ADD_SPMV                      paralution_get_ocl_kernel<ValueType>(64)
#define CL_KERNEL_COO_PERMUTE                       paralution_get_ocl_kernel<ValueType>(66)
#define CL_KERNEL_COO_SPMV_FLAT                     paralution_get_ocl_kernel<ValueType>(68)
#define CL_KERNEL_COO_SPMV_REDUCE_UPDATE            paralution_get_ocl_kernel<ValueType>(70)
#define CL_KERNEL_COO_SPMV_SERIAL                   paralution_get_ocl_kernel<ValueType>(72)
#define CL_KERNEL_RED_RECURSE                       paralution_get_ocl_kernel<ValueType>(74)
#define CL_KERNEL_RED_PARTIAL_SUM                   paralution_get_ocl_kernel<ValueType>(76)
#define CL_KERNEL_RED_EXTRAPOLATE                   paralution_get_ocl_kernel<ValueType>(78)
#define CL_KERNEL_CSR_PERMUTE_ROWS                  paralution_get_ocl_kernel<ValueType>(80)
#define CL_KERNEL_CSR_PERMUTE_COLS                  paralution_get_ocl_kernel<ValueType>(82)
#define CL_KERNEL_CSR_CALC_ROW_NNZ                  paralution_get_ocl_kernel<ValueType>(84)
#define CL_KERNEL_CSR_PERMUTE_ROW_NNZ               paralution_get_ocl_kernel<ValueType>(86)
#define CL_KERNEL_REDUCE                            paralution_get_ocl_kernel<ValueType>(88)
#define CL_KERNEL_ELL_MAX_ROW                       paralution_get_ocl_kernel<ValueType>(92)
#define CL_KERNEL_ELL_CSR_TO_ELL                    paralution_get_ocl_kernel<ValueType>(94)
#define CL_KERNEL_ASUM                              paralution_get_ocl_kernel<ValueType>(100)
#define CL_KERNEL_AMAX                              paralution_get_ocl_kernel<ValueType>(102)


#endif // PARALUTION_OCL_OCL_UTILS_HPP_
