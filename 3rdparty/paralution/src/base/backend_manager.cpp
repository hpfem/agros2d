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


#include "version.hpp" 
#include "backend_manager.hpp" 
#include "base_vector.hpp"
#include "base_matrix.hpp"
#include "host/host_affinity.hpp"
#include "host/host_vector.hpp"
#include "host/host_matrix_csr.hpp"
#include "host/host_matrix_coo.hpp"
#include "host/host_matrix_dia.hpp"
#include "host/host_matrix_ell.hpp"
#include "host/host_matrix_hyb.hpp"
#include "host/host_matrix_dense.hpp"
#include "host/host_matrix_mcsr.hpp"
#include "host/host_matrix_bcsr.hpp"
#include "../utils/log.hpp"

#include <assert.h>
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef SUPPORT_MKL
#include <mkl.h>
#include <mkl_spblas.h>
#endif

#ifdef SUPPORT_CUDA
#include "gpu/backend_gpu.hpp"
#endif

#ifdef SUPPORT_OCL
#include "ocl/backend_ocl.hpp"
#endif

#ifdef SUPPORT_MIC
#include "mic/backend_mic.hpp"
#endif


namespace paralution {

// Global backend descriptor and default values
Paralution_Backend_Descriptor _Backend_Descriptor = {
  false, // Init
#ifdef SUPPORT_CUDA
  GPU,   // default backend
#else
  #ifdef SUPPORT_OCL
  OCL,
   #else
     #ifdef SUPPORT_MIC
     MIC,
    #else
     None,
   #endif
#endif
#endif
  false, // use Accelerator
  1,     // OpenMP threads
  -1,    // pre-init OpenMP threads
  0,    // pre-init OpenMP threads
  true,  // host affinity (active)
  10000, // threshold size
  // GPU section
  NULL,  // *GPU_cublas_handle
  NULL,  // *GPU_cusparse_handle
  -1,    // GPU_dev;
  32,    // GPU_wrap;
  256,   // GPU_blocksize;
  65535, // Maximum threads in the block
  // OCL section
  NULL,  // OCL_handle
  -1,    // OCL_platform;
  -1,    // OCL_device;
  0,     // OCL_max_work_group_size;
  0,     // OCL_max_compute_units
  -1,     // OCL_warp_size
  // MIC
  0      // default is zero device
};

/// Host name
const std::string _paralution_host_name [1] = 
#ifdef SUPPORT_MKL
  {"CPU(MKL/OpenMP)"};
#else 
  {"CPU(OpenMP)"};
#endif

/// Backend names
const std::string _paralution_backend_name [4] =
  {"None",
   "GPU(CUDA)",
   "OpenCL",
   "MIC(OpenMP)"};

  
int init_paralution(void) {

  LOG_DEBUG(0, "init_paralution()",
            "* begin");


  if (_Backend_Descriptor.init == true) {
    LOG_INFO("PARALUTION platform has been initialized - restarting");
    stop_paralution();
  }

#ifdef SUPPORT_CUDA
  _Backend_Descriptor.backend = GPU;
#else
  #ifdef SUPPORT_OCL
    _Backend_Descriptor.backend = OCL;
  #else
    #ifdef SUPPORT_MIC
    _Backend_Descriptor.backend = MIC;
   #else
    _Backend_Descriptor.backend = None;
  #endif
 #endif
#endif

#ifdef _OPENMP
  _Backend_Descriptor.OpenMP_def_threads = omp_get_max_threads();
  _Backend_Descriptor.OpenMP_threads = omp_get_max_threads();
  _Backend_Descriptor.OpenMP_def_nested = omp_get_nested();

  // the default in PARALUTION is 0
  omp_set_nested(0);

  paralution_set_omp_affinity(_Backend_Descriptor.OpenMP_affinity);
#else 
  _Backend_Descriptor.OpenMP_threads = 1;
#endif

#ifdef SUPPORT_CUDA
  _Backend_Descriptor.accelerator = paralution_init_gpu();
#endif

#ifdef SUPPORT_OCL
  _Backend_Descriptor.accelerator = paralution_init_ocl();
#endif

#ifdef SUPPORT_MIC

#ifdef __INTEL_OFFLOAD

  _Backend_Descriptor.accelerator = paralution_init_mic();

#else

  LOG_INFO("The MIC backend is compiled without __INTEL_OFFLOAD - Double check the compilation process!");
  FATAL_ERROR(__FILE__, __LINE__);

#endif

#endif


  LOG_DEBUG(0, "init_paralution()",
            "* end");

  _Backend_Descriptor.init = true ;
  return 0;

}

int stop_paralution(void) {

  LOG_DEBUG(0, "stop_paralution()",
            "* begin");

#ifdef SUPPORT_CUDA
  paralution_stop_gpu();
#endif

#ifdef SUPPORT_OCL
  paralution_stop_ocl();
#endif

#ifdef SUPPORT_MIC
  paralution_stop_mic();
#endif

#ifdef _OPENMP
  assert(_Backend_Descriptor.OpenMP_def_threads > 0);
  omp_set_num_threads(_Backend_Descriptor.OpenMP_def_threads);

  assert((_Backend_Descriptor.OpenMP_def_nested == 0) ||
         (_Backend_Descriptor.OpenMP_def_nested == 1));

  omp_set_nested(_Backend_Descriptor.OpenMP_def_nested);
#endif

  _Backend_Descriptor.init = false;

  LOG_DEBUG(0, "stop_paralution()",
            "* end");

  return 0;
}

int set_device_paralution(int dev) {

  LOG_DEBUG(0, "set_device_paralution()",
            dev);

  assert(_Backend_Descriptor.init == false);

#ifdef SUPPORT_CUDA
  set_gpu_cuda_paralution(dev);
#endif

#ifdef SUPPORT_OCL
  _Backend_Descriptor.OCL_dev = dev;
#endif

#ifdef SUPPORT_MIC
  _Backend_Descriptor.MIC_dev = dev;
#endif

  return 0;

}

void set_omp_threads_paralution(int nthreads) {

  LOG_DEBUG(0, "set_omp_threads_paralution()",
            nthreads);

  assert(_Backend_Descriptor.init == true);

#ifdef _OPENMP
  _Backend_Descriptor.OpenMP_threads = nthreads;

  omp_set_num_threads(nthreads);  

 #if defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__)

  paralution_set_omp_affinity(_Backend_Descriptor.OpenMP_affinity);

#endif // linux

#else // !omp
  LOG_INFO("No OpenMP support");
  _Backend_Descriptor.OpenMP_threads = 1;
#endif // omp


}

void set_gpu_cuda_paralution(int ngpu) {

  LOG_DEBUG(0, "set_gpu_cuda_paralution()",
            ngpu);

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.GPU_dev = ngpu;

}

void set_ocl_paralution(int nplatform, int ndevice) {

  LOG_DEBUG(0, "set_ocl_paralution()",
            "nplatform=" << nplatform << 
            " ndevice" << ndevice);


  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.OCL_plat = nplatform;
  _Backend_Descriptor.OCL_dev = ndevice;

}

void set_ocl_platform_paralution(int platform) {

  LOG_DEBUG(0, "set_ocl_platform_paralution()",
            "platform=" << platform);

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.OCL_plat = platform;

}

void set_ocl_work_group_size_paralution(size_t size) {

  LOG_DEBUG(0, "set_ocl_work_group_size()",
            "size=" << size);

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.OCL_max_work_group_size = size;

}

void set_ocl_compute_units_paralution(size_t cu) {

  LOG_DEBUG(0, "set_ocl_compute_units()",
            "cu=" << cu);

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.OCL_computeUnits = cu;

}

void set_ocl_warp_size_paralution(int size) {

  LOG_DEBUG(0, "set_ocl_warp_size()",
            "size=" << size);

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.OCL_warp_size = size;

}

void info_paralution(void) {

  LOG_INFO("PARALUTION ver " <<
           __PARALUTION_VER_MAJOR << "." <<
           __PARALUTION_VER_MINOR << "." <<
           __PARALUTION_VER_REV << 
           __PARALUTION_VER_PRE);

#if defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__)

  LOG_VERBOSE_INFO(3, "Compiled for Linux/Unix OS");

#else // Linux

#if  defined(__APPLE__)

  LOG_VERBOSE_INFO(3, "Compiled for Mac OS");

#else // Apple

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN64) && !defined(__CYGWIN__)

  LOG_VERBOSE_INFO(3, "Compiled for Windows OS");

#else // Win

  // unknown
  LOG_VERBOSE_INFO(3, "Compiled for unknown OS");


#endif // Win

#endif // Apple

#endif // Linux


  info_paralution(_Backend_Descriptor);

}

void info_paralution(const struct Paralution_Backend_Descriptor backend_descriptor) {

  if (backend_descriptor.init == true) {
    LOG_INFO("PARALUTION platform is initialized");
  } else {
    LOG_INFO("PARALUTION platform is NOT initialized");
  }

  LOG_INFO("Accelerator backend: " << _paralution_backend_name[backend_descriptor.backend]);

#ifdef _OPENMP
  LOG_INFO("OpenMP threads:" << backend_descriptor.OpenMP_threads);
#else 
  LOG_INFO("No OpenMP support");
#endif

#ifdef SUPPORT_MKL
  LOG_INFO("MKL threads:" << mkl_get_max_threads() );
#else
  LOG_VERBOSE_INFO(3, "No MKL support");
#endif

#ifdef SUPPORT_CUDA
  if (backend_descriptor.accelerator)
    paralution_info_gpu(backend_descriptor);
  else
    LOG_INFO("GPU is not initialized");
#else
  LOG_VERBOSE_INFO(3, "No CUDA/GPU support");
#endif

#ifdef SUPPORT_OCL
  if (backend_descriptor.accelerator)
    paralution_info_ocl(backend_descriptor);
  else
    LOG_INFO("OpenCL is not initialized");
#else
  LOG_VERBOSE_INFO(3, "No OpenCL support");
#endif

#ifdef SUPPORT_MIC
  if (backend_descriptor.accelerator)
    paralution_info_mic(backend_descriptor);
  else
    LOG_INFO("MIC/OpenMP is not initialized");
#else
  LOG_VERBOSE_INFO(3, "No MIC/OpenMP support");
#endif

}


void set_omp_affinity(bool affinity) {

  assert(_Backend_Descriptor.init == false);
  _Backend_Descriptor.OpenMP_affinity = affinity;

}

void set_omp_threshold(const int threshold) {

  _Backend_Descriptor.OpenMP_threshold = threshold;

}


bool _paralution_available_accelerator(void) {

  return _Backend_Descriptor.accelerator;

}


void _get_backend_descriptor(struct Paralution_Backend_Descriptor *backend_descriptor) {

  *backend_descriptor = _Backend_Descriptor;

}


void _set_backend_descriptor(const struct Paralution_Backend_Descriptor backend_descriptor) {

  _Backend_Descriptor = backend_descriptor;

}


template <typename ValueType>
AcceleratorVector<ValueType>* _paralution_init_base_backend_vector(const struct Paralution_Backend_Descriptor backend_descriptor) {

  LOG_DEBUG(0, "_paralution_init_base_backend_vector()",
            "");

  switch (backend_descriptor.backend) {

#ifdef SUPPORT_CUDA
  // GPU
  case GPU:
    return _paralution_init_base_gpu_vector<ValueType>(backend_descriptor);
    break;
#endif

#ifdef SUPPORT_OCL
  // OCL
  case OCL:
    return _paralution_init_base_ocl_vector<ValueType>(backend_descriptor);
    break;
#endif

#ifdef SUPPORT_MIC
  // GPU
  case MIC:
    return _paralution_init_base_mic_vector<ValueType>(backend_descriptor);
    break;
#endif


  case 979753345:
    LOG_INFO("This is the impossible but VS cannot handle switch statement with 'default' but no 'case' labels");
    FATAL_ERROR(__FILE__, __LINE__);
    return NULL;    
    break;

  default:
    // No backend supported!
    LOG_INFO("Paralution was not compiled with " << _paralution_backend_name[backend_descriptor.backend] << " support");
    LOG_INFO("Building Vector on " << _paralution_backend_name[backend_descriptor.backend] << " failed"); 
    FATAL_ERROR(__FILE__, __LINE__);
    return NULL;
  }

}
  
template <typename ValueType>
AcceleratorMatrix<ValueType>* _paralution_init_base_backend_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                   const unsigned int matrix_format) {

  LOG_DEBUG(0, "_paralution_init_base_backend_matrix()",
            matrix_format);

  switch (backend_descriptor.backend) {

#ifdef SUPPORT_CUDA      
  case GPU:
    return _paralution_init_base_gpu_matrix<ValueType>(backend_descriptor, matrix_format);
    break;
#endif

#ifdef SUPPORT_OCL
  case OCL:
    return _paralution_init_base_ocl_matrix<ValueType>(backend_descriptor, matrix_format);
    break;
#endif

#ifdef SUPPORT_MIC      
  case MIC:
    return _paralution_init_base_mic_matrix<ValueType>(backend_descriptor, matrix_format);
    break;
#endif

  case 979753345:
    LOG_INFO("This is the impossible but VS cannot handle switch statement with 'default' but no 'case' labels");
    FATAL_ERROR(__FILE__, __LINE__);
    return NULL;    
    break;


  default:
    LOG_INFO("Paralution was not compiled with " << _paralution_backend_name[backend_descriptor.backend] << " support");
    LOG_INFO("Building " << _matrix_format_names[matrix_format] << " Matrix on " << _paralution_backend_name[backend_descriptor.backend] << " failed"); 
    
    FATAL_ERROR(__FILE__, __LINE__);
    return NULL;
  }

}


template <typename ValueType>
HostMatrix<ValueType>* _paralution_init_base_host_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                         const unsigned int matrix_format) {

  LOG_DEBUG(0, "_paralution_init_base_host_matrix()",
            matrix_format);

  switch (matrix_format) {
      
  case CSR:
    return new HostMatrixCSR<ValueType>(backend_descriptor);
    break;
      
  case COO:
    return new HostMatrixCOO<ValueType>(backend_descriptor);
    break;
 
  case DIA:
    return new HostMatrixDIA<ValueType>(backend_descriptor);
    break;

  case ELL:
    return new HostMatrixELL<ValueType>(backend_descriptor);
    break;
    
  case HYB:
    return new HostMatrixHYB<ValueType>(backend_descriptor);
    break;

  case DENSE:
    return new HostMatrixDENSE<ValueType>(backend_descriptor);
    break;

  case MCSR:
    return new HostMatrixMCSR<ValueType>(backend_descriptor);
    break;

  case BCSR:
    return new HostMatrixBCSR<ValueType>(backend_descriptor);
    break;

  default:
    return NULL;
  }

}


void _paralution_sync(void) {

  if (_paralution_available_accelerator() == true) {

#ifdef SUPPORT_CUDA
    paralution_gpu_sync();
#endif
    
#ifdef SUPPORT_OCL
    //  paralution_ocl_sync();
#endif
    
#ifdef SUPPORT_MIC
    //  paralution_mic_sync();
#endif
    
  }

}

void _set_omp_backend_threads(const struct Paralution_Backend_Descriptor backend_descriptor,
                              const int size) {

  // if the threshold is disabled or if the size is not in the threshold limit
  if ((backend_descriptor.OpenMP_threshold > 0) && 
      (size <= backend_descriptor.OpenMP_threshold) &&
      (size >= 0)) {
#ifdef _OPENMP
    omp_set_num_threads(1);
#endif
  } else {
#ifdef _OPENMP
    omp_set_num_threads(backend_descriptor.OpenMP_threads);
#endif     
  }

}

template AcceleratorVector<float>* _paralution_init_base_backend_vector(const struct Paralution_Backend_Descriptor backend_descriptor);
template AcceleratorVector<double>* _paralution_init_base_backend_vector(const struct Paralution_Backend_Descriptor backend_descriptor);
template AcceleratorVector<int>* _paralution_init_base_backend_vector(const struct Paralution_Backend_Descriptor backend_descriptor);

template AcceleratorMatrix<float>* _paralution_init_base_backend_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                        const unsigned int matrix_format);
template AcceleratorMatrix<double>* _paralution_init_base_backend_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                         const unsigned int matrix_format);


template HostMatrix<float>* _paralution_init_base_host_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                              const unsigned int matrix_format);
template HostMatrix<double>* _paralution_init_base_host_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                               const unsigned int matrix_format);

}

