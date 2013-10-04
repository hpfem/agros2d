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

#include "backend_manager.hpp" 
#include "base_vector.hpp"
#include "base_matrix.hpp"
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
  0      // OCL_max_compute_units
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
  {"No Accelerator",
   "GPU(CUDA)",
   "OpenCL",
   "MIC(OpenMP)"};

  
int init_paralution(void) {

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
  _Backend_Descriptor.OpenMP_threads = omp_get_max_threads();
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
  _Backend_Descriptor.accelerator = paralution_init_mic();
#endif

  _Backend_Descriptor.init = true ;
  return 0;

}

int stop_paralution(void) {

#ifdef SUPPORT_CUDA
  paralution_stop_gpu();
#endif

#ifdef SUPPORT_OCL
  paralution_stop_ocl();
#endif

#ifdef SUPPORT_MIC
  paralution_stop_mic();
#endif

  _Backend_Descriptor.init = false;

  return 0;
}

void set_omp_threads_paralution(int nthreads) {

  assert(_Backend_Descriptor.init == true);

#ifdef _OPENMP
  _Backend_Descriptor.OpenMP_threads = nthreads;
#else 
  LOG_INFO("No OpenMP support");
  _Backend_Descriptor.OpenMP_threads = 1;
#endif


}

void set_gpu_cuda_paralution(int ngpu) {

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.GPU_dev = ngpu;

}

void set_ocl_paralution(int nplatform, int ndevice) {

  assert(_Backend_Descriptor.init == false);

  _Backend_Descriptor.OCL_plat = nplatform;
  _Backend_Descriptor.OCL_dev = ndevice;

}

void info_paralution(void) {

  info_paralution(_Backend_Descriptor);

}

void info_paralution(const struct Paralution_Backend_Descriptor backend_descriptor) {

  if (backend_descriptor.init == true) {
    LOG_INFO("PARALUTION platform is initialized");
  } else {
    LOG_INFO("PARALUTION platform is NOT initialized");
  }

  //  LOG_INFO("Accelerator Backend:" << _paralution_backend_name[backend_descriptor.backend]);

#ifdef _OPENMP
  LOG_INFO("OpenMP threads:" << backend_descriptor.OpenMP_threads);
#else 
  LOG_INFO("No OpenMP support");
#endif

#ifdef SUPPORT_MKL
  LOG_INFO("MKL threads:" << mkl_get_max_threads() );
#else
  LOG_INFO("No MKL support");
#endif

#ifdef SUPPORT_CUDA
  if (backend_descriptor.accelerator)
    paralution_info_gpu(backend_descriptor);
  else
    LOG_INFO("GPU is not initialized");
#else
  LOG_INFO("No CUDA/GPU support");
#endif

#ifdef SUPPORT_OCL
  if (backend_descriptor.accelerator)
    paralution_info_ocl(backend_descriptor);
  else
    LOG_INFO("OpenCL is not initialized");
#else
  LOG_INFO("No OpenCL support");
#endif

#ifdef SUPPORT_MIC
  if (backend_descriptor.accelerator)
    paralution_info_mic(backend_descriptor);
  else
    LOG_INFO("MIC/OpenMP is not initialized");
#else
  LOG_INFO("No MIC/OpenMP support");
#endif

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

