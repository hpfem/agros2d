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

#include <iostream>
#include <fstream>

#include "log.hpp"
#include "paths.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

int main(int argc, char* argv[]) {

  cl_int          err;
  cl_uint         ocl_numPlatforms;
  cl_uint         ocl_numDevices;
  cl_platform_id  *ocl_platform = NULL;
  cl_device_id    *ocl_devices = NULL;
  cl_device_type  ocl_typeDevice;
  std::string     ocl_deviceType;
  cl_int          ocl_dev;
  cl_int          ocl_plat;
  char            ocl_namePlatform[256];
  char            ocl_nameDevice[256];
  char            ocl_nameVendor[256];
  cl_uint         ocl_cu;
  size_t          ocl_blocksize;
  cl_uint         ocl_warpsize = 32; // TODO hardcoded warpsize

  // Query for the number of OpenCL platforms in the system
  err = clGetPlatformIDs( 0, NULL, &ocl_numPlatforms );
  if (err != CL_SUCCESS )
    FATAL_ERROR( __FILE__, __LINE__ );

  if ( ocl_numPlatforms == 0 ) {
    std::cout << "No OpenCL platforms available. Please compile without OpenCL support." << std::endl;
    FATAL_ERROR( __FILE__, __LINE__ );
  }

  // Allocate host memory for the platforms
  ocl_platform = ( cl_platform_id* ) malloc( ocl_numPlatforms * sizeof( cl_platform_id ) );

  // Get the OpenCL platforms
  err = clGetPlatformIDs( ocl_numPlatforms, ocl_platform, NULL );
  if (err != CL_SUCCESS )
    FATAL_ERROR( __FILE__, __LINE__ );

  if ( ocl_platform == NULL ) {
    std::cout << "No OpenCL platforms available. Please compile without OpenCL support." << std::endl;
    FATAL_ERROR( __FILE__, __LINE__ );
  }

  std::cout << "-- Available OpenCL platforms:" << std::endl;
  for ( cl_uint i = 0; i < ocl_numPlatforms; ++i ) {

    err = clGetPlatformInfo( ocl_platform[i], CL_PLATFORM_NAME, sizeof(ocl_namePlatform), &ocl_namePlatform, NULL );
    if (err != CL_SUCCESS )
      FATAL_ERROR( __FILE__, __LINE__ );

    std::cout << "-- \t" << i+1 << ") " << ocl_namePlatform << std::endl;

  }

  if ( argc > 1 ) {

    ocl_plat = atoi(argv[1]);
    ocl_plat--;

  } else {

    if ( ocl_numPlatforms > 1 ) {

      std::cout << "-- Please choose an OpenCL platform: ";
      std::cin >> ocl_plat;
      ocl_plat--;

    } else {

      ocl_plat = 0;    

    }

  }

  if ( !(ocl_plat > -1 && ocl_plat < ocl_numPlatforms) ) {
    std::cout << "-- \tInvalid choice. Default platform 0 is being used." << std::endl;
    ocl_plat = 0;
  }

  // Query for the number of OpenCL devices (any type) for the current OpenCL platform
  err = clGetDeviceIDs( ocl_platform[ocl_plat], CL_DEVICE_TYPE_ALL, 0, NULL, &ocl_numDevices );
  if (err != CL_SUCCESS )
    FATAL_ERROR( __FILE__, __LINE__ );

  if ( ocl_numDevices == 0 ) {

    std::cout << "-- \t\tNo OpenCL devices for this platform" << std::endl;

  } else {

    // Allocate host memory for the devices
    ocl_devices = ( cl_device_id* ) malloc( ocl_numDevices * sizeof( cl_device_id ) );

    // Get the OpenCL devices
    err = clGetDeviceIDs( ocl_platform[ocl_plat], CL_DEVICE_TYPE_ALL, ocl_numDevices, ocl_devices, NULL );
    if (err != CL_SUCCESS )
      FATAL_ERROR( __FILE__, __LINE__ );


    if ( ocl_devices == NULL ) {

      std::cout << "-- \tNo OpenCL devices for this platform" << std::endl;

    } else {

      std::cout << "-- Available OpenCL devices:" << std::endl;
      for ( cl_uint j = 0; j < ocl_numDevices; ++j ) {

        err = clGetDeviceInfo( ocl_devices[j], CL_DEVICE_NAME, sizeof(ocl_nameDevice), &ocl_nameDevice, NULL );
        if (err != CL_SUCCESS )
          FATAL_ERROR( __FILE__, __LINE__ );

        err = clGetDeviceInfo( ocl_devices[j], CL_DEVICE_VENDOR, sizeof(ocl_nameVendor), &ocl_nameVendor, NULL );
        if (err != CL_SUCCESS )
          FATAL_ERROR( __FILE__, __LINE__ );

        err = clGetDeviceInfo( ocl_devices[j], CL_DEVICE_TYPE, sizeof(ocl_typeDevice), &ocl_typeDevice, NULL );
        if (err != CL_SUCCESS )
          FATAL_ERROR( __FILE__, __LINE__ );

        if ( ocl_typeDevice & CL_DEVICE_TYPE_CPU )         ocl_deviceType = "CPU";
        if ( ocl_typeDevice & CL_DEVICE_TYPE_GPU )         ocl_deviceType = "GPU";
        if ( ocl_typeDevice & CL_DEVICE_TYPE_ACCELERATOR ) ocl_deviceType = "ACCELERATOR";
        if ( ocl_typeDevice & CL_DEVICE_TYPE_DEFAULT )     ocl_deviceType = "DEFAULT";

        std::cout << "-- \t" << j+1 << ") " << ocl_nameDevice << " (" << ocl_nameVendor << ", " <<ocl_deviceType.c_str() <<")" << std::endl;

      }

      if ( argc > 2 ) {

        ocl_dev = atoi(argv[2]);
        ocl_dev--;

      } else {

        if ( ocl_numDevices > 1 ) {

          std::cout << "-- Please choose an OpenCL device: ";
          std::cin >> ocl_dev;
          ocl_dev--;

        } else {

          ocl_dev = 0;

        }

      }

      if ( !(ocl_dev > -1 && ocl_dev < ocl_numDevices) ) {
        std::cout << "-- \tInvalid choice. Default device 0 is being used." << std::endl;
        ocl_dev = 0;
      }

      err = clGetDeviceInfo( ocl_devices[ocl_dev],
                             CL_DEVICE_MAX_COMPUTE_UNITS,
                             sizeof( ocl_cu ),
                             &ocl_cu,
                             NULL );
      if ( err != CL_SUCCESS )
        FATAL_ERROR( __FILE__, __LINE__ );

      err = clGetDeviceInfo( ocl_devices[ocl_dev],
                             CL_DEVICE_MAX_WORK_GROUP_SIZE,
                             sizeof( ocl_blocksize ),
                             &ocl_blocksize,
                             NULL );
      if ( err != CL_SUCCESS )
        FATAL_ERROR( __FILE__, __LINE__ );

        err = clGetDeviceInfo( ocl_devices[ocl_dev], 
			       CL_DEVICE_TYPE, 
			       sizeof(ocl_typeDevice), 
			       &ocl_typeDevice, NULL );
        if (err != CL_SUCCESS )
          FATAL_ERROR( __FILE__, __LINE__ );

        if ( ocl_typeDevice & CL_DEVICE_TYPE_CPU )         ocl_deviceType = "CPU";
        if ( ocl_typeDevice & CL_DEVICE_TYPE_GPU )         ocl_deviceType = "GPU";
        if ( ocl_typeDevice & CL_DEVICE_TYPE_ACCELERATOR ) ocl_deviceType = "ACCELERATOR";
        if ( ocl_typeDevice & CL_DEVICE_TYPE_DEFAULT )     ocl_deviceType = "DEFAULT";

        // TODO hardcoded divisor
        //      if ( ocl_deviceType == "CPU" ) ocl_blocksize /= 16;
        // Uncomment for NVIDIA GPU:
        //      if ( ocl_deviceType == "GPU" ) ocl_blocksize /= 8;
        // Uncomment for AMD GPU:
        //      if ( ocl_deviceType == "GPU" ) ocl_blocksize /= 1;

        if ( ocl_deviceType == "CPU" ) ocl_blocksize /= 16;
        if ( ocl_deviceType == "GPU" ) ocl_blocksize = 128;

    }

  }

  std::string file = utilsPath;
  std::ofstream hwparam((file.append("HardwareParameters.hpp")).c_str());

  if (hwparam.is_open()) {

    hwparam << "#define OPENCL_PLATFORM "  << ocl_plat << std::endl;
    hwparam << "#define OPENCL_DEVICE "    << ocl_dev << std::endl;
    hwparam << "#define OPENCL_TYPE \""    << ocl_deviceType.c_str() <<"\"" << std::endl;
    hwparam << "#define OPENCL_CU "        << ocl_cu << std::endl;
    hwparam << "#define OPENCL_BLOCKSIZE " << ocl_blocksize << std::endl;
    hwparam << "#define OPENCL_WARPSIZE "  << ocl_warpsize << std::endl;

  }

  return 0;

}
