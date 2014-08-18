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


#include "../backend_manager.hpp"
#include "backend_ocl.hpp" 
#include "../../utils/log.hpp"
#include "ocl_utils.hpp"
#include "../base_vector.hpp"
#include "../base_matrix.hpp"
#include "kernels_ocl.hpp"

#include "ocl_vector.hpp"
#include "ocl_matrix_csr.hpp"
#include "ocl_matrix_coo.hpp"
#include "ocl_matrix_mcsr.hpp"
#include "ocl_matrix_bcsr.hpp"
#include "ocl_matrix_hyb.hpp"
#include "ocl_matrix_dia.hpp"
#include "ocl_matrix_ell.hpp"
#include "ocl_matrix_dense.hpp"

#include "ocl_kernels_general.hpp"
#include "ocl_kernels_vector.hpp"
#include "ocl_kernels_csr.hpp"
#include "ocl_kernels_bcsr.hpp"
#include "ocl_kernels_mcsr.hpp"
#include "ocl_kernels_dense.hpp"
#include "ocl_kernels_ell.hpp"
#include "ocl_kernels_dia.hpp"
#include "ocl_kernels_coo.hpp"
#include "ocl_kernels_hyb.hpp"

#include <assert.h>
#include <sstream>

namespace paralution {

bool paralution_get_platform_ocl(cl_platform_id **ocl_platforms, cl_uint *ocl_numPlatforms) {

  LOG_DEBUG(0, "paralution_get_platform_ocl()",
            "* begin");

  assert(*ocl_platforms == NULL);

  cl_int  err;

  // Query for the number of OpenCL platforms in the system
  err = clGetPlatformIDs(0, NULL, ocl_numPlatforms);
  if (*ocl_numPlatforms < 1 || err != CL_SUCCESS) {
    LOG_INFO("No OpenCL platform available");
    LOG_INFO("OpenCL has NOT been initialized!");
    return false;
  }

  // Allocate host memory for the platforms
  *ocl_platforms = new cl_platform_id[*ocl_numPlatforms];

  // Get all OpenCL platforms
  err = clGetPlatformIDs(*ocl_numPlatforms, *ocl_platforms, NULL);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot retrieve OpenCL platform information");
    paralution_stop_ocl();
    return false;
  }

  LOG_DEBUG(0, "paralution_get_platform_ocl()",
            "* end");

  return true;

}

bool paralution_get_device_ocl(const cl_platform_id &ocl_platform, cl_device_id **ocl_devices,
                               cl_uint *ocl_numDevices) {

  LOG_DEBUG(0, "paralution_get_device_ocl()",
            "* begin");

  assert(*ocl_devices == NULL);
  assert(&ocl_platform != NULL);

  cl_int  err;

  // Query for the number of OpenCL devices (any type) for the current OpenCL platform
  err = clGetDeviceIDs(ocl_platform, CL_DEVICE_TYPE_ALL, 0, NULL, ocl_numDevices);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot query current OpenCL platform for devices");
    paralution_stop_ocl();
    return false;
  }

  if (*ocl_numDevices < 1 ) {
    LOG_INFO("No OpenCL device available");
    paralution_stop_ocl();
    return false;
  }

  // Allocate host memory for the devices
  *ocl_devices = new cl_device_id[*ocl_numDevices];

  // Get the OpenCL devices
  err = clGetDeviceIDs(ocl_platform, CL_DEVICE_TYPE_ALL, *ocl_numDevices, *ocl_devices, NULL);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot get OpenCL devices");
    paralution_stop_ocl();
    return false;
  }

  LOG_DEBUG(0, "paralution_get_device_ocl()",
            "* end");

  return true;

}

bool paralution_set_kernels_ocl(cl_kernel *ocl_kernels) {

  LOG_DEBUG(0, "paralution_set_kernels_ocl()",
            "* begin");

  assert(ocl_kernels != NULL);

  cl_int err;

  // Fill Integer kernel
  ocl_kernels[0] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int, kernels_ocl[0].c_str(), &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  int k = 1;
  for (int i=0; i<KERNELCOUNT; ++i) {

    // Single precision kernels
    ocl_kernels[k] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, kernels_ocl[i].c_str(), &err);
    if (err != CL_SUCCESS) {
      LOG_INFO("Cannot create OpenCL single precision kernels");
      paralution_stop_ocl();
      return false;
    }
    ++k;

    // Double precision kernels
    ocl_kernels[k] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, kernels_ocl[i].c_str(), &err);
    if (err != CL_SUCCESS) {
      LOG_INFO("Cannot create OpenCL double precision kernels");
      paralution_stop_ocl();
      return false;
    }
    ++k;

  }

  LOG_DEBUG(0, "paralution_set_kernels_ocl()",
            "* end");

  return true;

}

// Returns the kernel for double precision
template <>
cl_kernel paralution_get_kernel_ocl<double>(int kernel) {

  // return double kernel
  return (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels)[kernel + 1];

}


// Returns the kernel for single precision
template <>
cl_kernel paralution_get_kernel_ocl<float>(int kernel) {

  // return float kernel
  return (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels)[kernel];

}


// Returns the int kernel
template <>
cl_kernel paralution_get_kernel_ocl<int>(int kernel) {

  // return int kernel
  // theres only 1 int kernel
  return (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels)[0];

}


// Initalizes the OpenCL backend
bool paralution_init_ocl(void) {

  LOG_DEBUG(0, "paralution_init_ocl()",
            "* begin");

  assert(_Backend_Descriptor.OCL_handle == NULL);

  // OpenCL specific variables
  _Backend_Descriptor.OCL_handle = new oclHandle_t;

  bool              stat;
  cl_int            err;
  cl_uint           ocl_numPlatforms;
  cl_uint           *ocl_numDevices;
  cl_uint           total_devices = 0;
  cl_device_type    ocl_typeDevice;
  cl_platform_id    *ocl_platforms = NULL;
  cl_device_id      **ocl_devices  = NULL;
  std::string       sourceStr_sp;
  std::string       sourceStr_dp;
  std::string       sourceStr_int;
  char              *source_sp;
  char              *source_dp;
  char              *source_int;

  // Get OpenCL platforms
  stat = paralution_get_platform_ocl(&ocl_platforms, &ocl_numPlatforms);
  if (stat != true) return false;

  // Allocate device array for each platform
  ocl_devices = new cl_device_id*[ocl_numPlatforms];
  ocl_numDevices = new cl_uint[ocl_numPlatforms];

  // Get OpenCL devices for each platform
  for (cl_uint p=0; p<ocl_numPlatforms; ++p) {
    ocl_devices[p] = NULL;
    stat = paralution_get_device_ocl(ocl_platforms[p], &(ocl_devices[p]), &(ocl_numDevices[p]));
    if (stat != true) return false;
    total_devices += ocl_numDevices[p];
  }

  LOG_INFO("Number of OpenCL devices in the system: " << total_devices);

  // Find a GPU to make it default if nothing has been specified yet
  if (_Backend_Descriptor.OCL_plat == -1 || _Backend_Descriptor.OCL_dev == -1) {

    bool GPU = false;

    // Keep querying CL_DEVICE_TYPE, until a GPU has been found, we will use the FIRST GPU discovered
    for (cl_uint p=0; p<ocl_numPlatforms; ++p) {
      for (cl_uint d=0; d<ocl_numDevices[p]; ++d) {

        err = clGetDeviceInfo(ocl_devices[p][d],
                              CL_DEVICE_TYPE,
                              sizeof(ocl_typeDevice),
                              &ocl_typeDevice,
                              NULL);
        if (err != CL_SUCCESS) {
          LOG_INFO("Cannot query for CL_DEVICE_TYPE");
          paralution_stop_ocl();
          return false;
        }

        if (ocl_typeDevice & CL_DEVICE_TYPE_GPU) {
          GPU = true;
          _Backend_Descriptor.OCL_plat = p;
          _Backend_Descriptor.OCL_dev  = d;
          p = ocl_numPlatforms;
          break;
        }

      }
    }

    if (GPU == false) {
      _Backend_Descriptor.OCL_plat = 0;
      _Backend_Descriptor.OCL_dev  = 0;
    }

  }

  // Check for valid platform
  if ((cl_uint)(_Backend_Descriptor.OCL_plat) >= ocl_numPlatforms ||
      _Backend_Descriptor.OCL_plat < 0) {
    LOG_INFO("Invalid OpenCL platform");
    paralution_stop_ocl();
    return false;
  }

  // Check for valid device
  if ((cl_uint)(_Backend_Descriptor.OCL_dev) >= ocl_numDevices[_Backend_Descriptor.OCL_plat] ||
      _Backend_Descriptor.OCL_dev < 0) {
    LOG_INFO("Invalid OpenCL device");
    paralution_stop_ocl();
    return false;
  }

  // Query device properties
  err = clGetDeviceInfo(ocl_devices[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                        CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(_Backend_Descriptor.OCL_max_work_group_size),
                        &_Backend_Descriptor.OCL_max_work_group_size,
                        NULL);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot query for CL_DEVICE_MAX_WORK_GROUP_SIZE");
    paralution_stop_ocl();
    return false;
  }

  err = clGetDeviceInfo(ocl_devices[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                        CL_DEVICE_TYPE,
                        sizeof(ocl_typeDevice),
                        &ocl_typeDevice,
                        NULL);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot query for CL_DEVICE_TYPE");
    paralution_stop_ocl();
    return false;
  }

  if (ocl_typeDevice & CL_DEVICE_TYPE_CPU) _Backend_Descriptor.OCL_max_work_group_size /= 16;
  if (ocl_typeDevice & CL_DEVICE_TYPE_GPU) _Backend_Descriptor.OCL_max_work_group_size = 128;

  err = clGetDeviceInfo(ocl_devices[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                        CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(_Backend_Descriptor.OCL_computeUnits),
                        &_Backend_Descriptor.OCL_computeUnits,
                        NULL);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot query for CL_DEVICE_TYPE");
    paralution_stop_ocl();
    return false;
  }

  if (_Backend_Descriptor.OCL_warp_size == -1)
    _Backend_Descriptor.OCL_warp_size = 32;

  // Create OpenCL context for computing device
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context = clCreateContext(0, 1,
                               &ocl_devices[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                               NULL, NULL, &err);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot create OpenCL context for current device");
    paralution_stop_ocl();
    return false;
  }

  // Create OpenCL command queue
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue = clCreateCommandQueue(
                                 OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                 ocl_devices[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                                 0, &err);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot create OpenCL command queue for current device");
    paralution_stop_ocl();
    return false;
  }

  // Create OpenCL program from source for single precision
  std::stringstream ss;
  ss << "#define BLOCK_SIZE " << _Backend_Descriptor.OCL_max_work_group_size << "\n";
  ss << "#define WARP_SIZE "  << _Backend_Descriptor.OCL_warp_size  << "\n";
  sourceStr_sp  = ("#pragma OPENCL EXTENSION cl_khr_fp64 : disable\n");
  sourceStr_sp.append("#define ValueType float\n");
  sourceStr_sp.append(ss.str());
  sourceStr_sp.append(ocl_kernels_general);
  sourceStr_sp.append(ocl_kernels_vector);
  sourceStr_sp.append(ocl_kernels_csr);
  sourceStr_sp.append(ocl_kernels_bcsr);
  sourceStr_sp.append(ocl_kernels_mcsr);
  sourceStr_sp.append(ocl_kernels_dense);
  sourceStr_sp.append(ocl_kernels_ell);
  sourceStr_sp.append(ocl_kernels_dia);
  sourceStr_sp.append(ocl_kernels_coo);
  sourceStr_sp.append(ocl_kernels_hyb);
  // Create OpenCL program from source for double precision
  sourceStr_dp  = ("#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
  sourceStr_dp.append("#define ValueType double\n");
  sourceStr_dp.append(ss.str());
  sourceStr_dp.append(ocl_kernels_general);
  sourceStr_dp.append(ocl_kernels_vector);
  sourceStr_dp.append(ocl_kernels_csr);
  sourceStr_dp.append(ocl_kernels_bcsr);
  sourceStr_dp.append(ocl_kernels_mcsr);
  sourceStr_dp.append(ocl_kernels_dense);
  sourceStr_dp.append(ocl_kernels_ell);
  sourceStr_dp.append(ocl_kernels_dia);
  sourceStr_dp.append(ocl_kernels_coo);
  sourceStr_dp.append(ocl_kernels_hyb);
  sourceStr_int = ("#define ValueType int\n");
  sourceStr_int.append(ss.str());
  sourceStr_int.append(ocl_kernels_general);

  source_sp  = new char[sourceStr_sp.size()  + 1];
  std::copy(sourceStr_sp.begin(),  sourceStr_sp.end(),  source_sp);
  source_sp[sourceStr_sp.size()]   = '\0';
  source_dp  = new char[sourceStr_dp.size()  + 1];
  std::copy(sourceStr_dp.begin(),  sourceStr_dp.end(),  source_dp);
  source_dp[sourceStr_dp.size()]   = '\0';
  source_int = new char[sourceStr_int.size() + 1];
  std::copy(sourceStr_int.begin(), sourceStr_int.end(), source_int);
  source_int[sourceStr_int.size()] = '\0';

  if (source_sp == NULL || source_dp == NULL || source_int == NULL) {
    LOG_INFO("Failed to load OpenCL kernel source files");
    paralution_stop_ocl();
    return false;
  }

  // Create OpenCL program for single precision kernels
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp = clCreateProgramWithSource(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                                                         1,
                                                                                         (const char **) &source_sp,
                                                                                         NULL,
                                                                                         &err);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot create OpenCL single precision program");
    paralution_stop_ocl();
    return false;
  }

  // Create OpenCL program for double precision kernels
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp = clCreateProgramWithSource(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                                                         1,
                                                                                         (const char **) &source_dp,
                                                                                         NULL,
                                                                                         &err);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot create OpenCL double precision program");
    paralution_stop_ocl();
    return false;
  }

  // Create OpenCL program for int kernels
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int = clCreateProgramWithSource(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                                                          1,
                                                                                          (const char **) &source_int,
                                                                                          NULL,
                                                                                          &err);
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot create OpenCL integer program");
    paralution_stop_ocl();
    return false;
  }

  // Compile OpenCL programs
  err  = clBuildProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp,  0,
                        NULL, "-w -cl-mad-enable -cl-no-signed-zeros", NULL, NULL);
  err |= clBuildProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp,  0,
                        NULL, "-w -cl-mad-enable -cl-no-signed-zeros", NULL, NULL);
  err |= clBuildProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int, 0,
                        NULL, "-w -cl-mad-enable -cl-no-signed-zeros", NULL, NULL);
  if (err == CL_BUILD_PROGRAM_FAILURE) {

    // If compilation failed, get and print build log
    LOG_INFO("CL_BUILD_PROGRAM_FAILURE");
    char ocl_buildLog[1024];
    err = clGetProgramBuildInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp,
                                ocl_devices[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                                CL_PROGRAM_BUILD_LOG,
                                sizeof(ocl_buildLog),
                                &ocl_buildLog,
                                NULL);
    if (err != CL_SUCCESS) {
      paralution_stop_ocl();
      return false;
    }

    LOG_INFO("OCL BuildLog:\n" <<ocl_buildLog);
    paralution_stop_ocl();
    return false;

  }
  if (err != CL_SUCCESS) {
    LOG_INFO("Cannot build OpenCL program");
    paralution_stop_ocl();
    return false;
  }

  // Allocate kernel array for double + float + int kernels
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels = new cl_kernel[KERNELCOUNT+KERNELCOUNT+1];
  // Write kernels to backend kernel array
  stat = paralution_set_kernels_ocl(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels);
  if (stat != true) return false;

  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms = ocl_platforms;
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numPlatforms = ocl_numPlatforms;
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices = ocl_devices;
  OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numDevices = ocl_numDevices;

  // Free kernel sources
  delete[] source_sp;
  delete[] source_dp;
  delete[] source_int;

  LOG_DEBUG(0, "paralution_init_ocl()",
            "* end");

  return true;

}


// Stop OpenCL backend
void paralution_stop_ocl(void) {

  LOG_DEBUG(0, "paralution_stop_ocl()",
            "* begin");

  if (_Backend_Descriptor.accelerator) {

    cl_int err;

    // If OpenCL command queue was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue) {
      err = clReleaseCommandQueue(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue = NULL;
    }

    // If OpenCL kernel was created, release it
    if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels != NULL) {
      for (int i=0; i<KERNELCOUNT+KERNELCOUNT+1; ++i) {
        if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels[i] != NULL) {
          err = clReleaseKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels[i]);
          CHECK_OCL_ERROR(err, __FILE__, __LINE__);
        }
      }
      delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernels;
    }

    // If OpenCL program was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp) {
      err = clReleaseProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp = NULL;
    }
    // If OpenCL program was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp) {
      err = clReleaseProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp = NULL;
    }
    // If OpenCL program was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int) {
      err = clReleaseProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int = NULL;
    }

    // If OpenCL context was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context) {
      err = clReleaseContext(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context = NULL;
    }

    // If OpenCL platforms were queried, release them
    if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms != NULL)
      delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms;


    // If OpenCL devices were queried, release them
    if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices != NULL) {
      for (cl_uint p=0; p<OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numPlatforms; ++p)
        if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices[p] != NULL)
          delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices[p];
      delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices;
    }

    if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numDevices != NULL)
      delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numDevices;

    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numPlatforms = 0;

  }

  delete static_cast<oclHandle_t*> (_Backend_Descriptor.OCL_handle);
  _Backend_Descriptor.OCL_handle = NULL;

  _Backend_Descriptor.OCL_plat = -1;
  _Backend_Descriptor.OCL_dev  = -1;

  LOG_DEBUG(0, "paralution_stop_ocl()",
            "* end");

}


// Print OpenCL computation device information to screen
void paralution_info_ocl(const struct Paralution_Backend_Descriptor backend_descriptor) {

  LOG_DEBUG(0, "paralution_info_ocl()",
            "* begin");

  cl_int          err;
  cl_uint         ocl_freq;
  cl_ulong        ocl_global_mem_size;
  cl_device_type  ocl_typeDevice;

  char            ocl_namePlatform[256];
  char            ocl_nameDevice[256];
  char            ocl_ver[256];
  std::string     ocl_type;

  if (_Backend_Descriptor.OCL_dev >= 0) {
    LOG_INFO("Selected OpenCL platform: " << _Backend_Descriptor.OCL_plat);
    LOG_INFO("Selected OpenCL device: " << _Backend_Descriptor.OCL_dev);
  } else {
    LOG_INFO("No OpenCL device is selected!");
  }

  for (cl_uint p=0; p<OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numPlatforms; ++p) {

    for (cl_uint d=0; d<OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_numDevices[p]; ++d) {

      err = clGetPlatformInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms[p],
                              CL_PLATFORM_NAME, sizeof(ocl_namePlatform), &ocl_namePlatform, NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);

      // Get and print OpenCL device name
      err = clGetDeviceInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices[p][d],
                            CL_DEVICE_NAME, sizeof(ocl_nameDevice), &ocl_nameDevice, NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);

      // Get and print OpenCL device global memory
      err = clGetDeviceInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices[p][d],
                            CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ocl_global_mem_size), &ocl_global_mem_size, NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);

      // Get and print OpenCL device clock frequency
      err = clGetDeviceInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices[p][d],
                            CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(ocl_freq), &ocl_freq, NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);

      // Get and print OpenCL device version
      err = clGetDeviceInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices[p][d],
                            CL_DEVICE_VERSION, sizeof(ocl_ver), &ocl_ver, NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);

      // Get and print OpenCL device type
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_plat][_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_TYPE,
                            sizeof(ocl_typeDevice),
                            &ocl_typeDevice,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      if (ocl_typeDevice & CL_DEVICE_TYPE_CPU) ocl_type = "CPU";
      if (ocl_typeDevice & CL_DEVICE_TYPE_GPU) ocl_type = "GPU";
      if (ocl_typeDevice & CL_DEVICE_TYPE_ACCELERATOR) ocl_type = "ACCELERATOR";
      if (ocl_typeDevice & CL_DEVICE_TYPE_DEFAULT) ocl_type = "DEFAULT";


      LOG_INFO("------------------------------------------------");

      LOG_INFO("Platform number: "              << p);
      LOG_INFO("Platform name: "                << ocl_namePlatform);
      LOG_INFO("Device number: "                << d);
      LOG_INFO("Device name: "                  << ocl_nameDevice);
      LOG_INFO("Device type: "                  << ocl_type);
      LOG_INFO("totalGlobalMem: "               << (ocl_global_mem_size >> 20) <<" MByte");
      LOG_INFO("clockRate: "                    << ocl_freq);
      LOG_INFO("OpenCL version: "               << ocl_ver);

      LOG_INFO("------------------------------------------------");  

    }

  }

  LOG_DEBUG(0, "paralution_info_ocl()",
            "* end");

}


template <typename ValueType>
AcceleratorMatrix<ValueType>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                               const unsigned int matrix_format) {
  assert(backend_descriptor.backend == OCL);

  switch (matrix_format) {

  case CSR:
    return new OCLAcceleratorMatrixCSR<ValueType>(backend_descriptor);
    
  case COO:
    return new OCLAcceleratorMatrixCOO<ValueType>(backend_descriptor);

  case MCSR:
    return new OCLAcceleratorMatrixMCSR<ValueType>(backend_descriptor);

  case DIA:
    return new OCLAcceleratorMatrixDIA<ValueType>(backend_descriptor);
    
  case ELL:
    return new OCLAcceleratorMatrixELL<ValueType>(backend_descriptor);

  case DENSE:
    return new OCLAcceleratorMatrixDENSE<ValueType>(backend_descriptor);

  case HYB:
    return new OCLAcceleratorMatrixHYB<ValueType>(backend_descriptor);

  case BCSR:
    return new OCLAcceleratorMatrixBCSR<ValueType>(backend_descriptor);


      
  default:
    LOG_INFO("This backend is not supported for Matrix types");
    FATAL_ERROR(__FILE__, __LINE__);   
    return NULL;
  }

}


template <typename ValueType>
AcceleratorVector<ValueType>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor) {

  assert(backend_descriptor.backend == OCL);

  return new OCLAcceleratorVector<ValueType>(backend_descriptor);

}


template AcceleratorVector<float>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);
template AcceleratorVector<double>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);
template AcceleratorVector<int>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);

template AcceleratorMatrix<float>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                     const unsigned int matrix_format);
template AcceleratorMatrix<double>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                     const unsigned int matrix_format);

}

