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


#ifndef PARALUTION_BACKEND_MANAGER_HPP_
#define PARALUTION_BACKEND_MANAGER_HPP_

#include <iostream>

namespace paralution {

template <typename ValueType>
class AcceleratorVector;
template <typename ValueType>
class AcceleratorMatrix;
template <typename ValueType>
class HostMatrix;

/// Backend descriptor - keeps information about the
/// hardware - OpenMP (threads); CUDA (blocksizes, handles, etc);
/// OpenCL (workgroupsizes, handles, etc);
struct Paralution_Backend_Descriptor {

  // set by initbackend();
  bool init;
  
  // current backend
  int backend;
  bool accelerator;
  
  // OpenMP threads
  int OpenMP_threads;
  // OpenMP threads before PARALUTION init
  int OpenMP_def_threads;
  // OpenMP nested before PARALUTION init
  int OpenMP_def_nested;
  // Host affinity (true-yes/false-no)
  bool OpenMP_affinity;
  // Host threshold size
  int OpenMP_threshold;

  // GPU section
  // gpu handles
  // cublasHandle_t casted in void **
  void *GPU_cublas_handle;
  // cusparseHandle_t casted in void **
  void *GPU_cusparse_handle;
  
  int GPU_dev;
  int GPU_wrap;
  int GPU_block_size;
  int GPU_max_threads;

  // OCL section
  // ocl handle
  void *OCL_handle;

  int OCL_plat;
  int OCL_dev;
  size_t OCL_max_work_group_size;
  size_t OCL_computeUnits;
  int OCL_warp_size;

  // MIC section
  int MIC_dev;
};
  
/// Global backend descriptor
extern struct Paralution_Backend_Descriptor _Backend_Descriptor;

/// Host name
extern const std::string _paralution_host_name [1];

/// Backend names
extern const std::string _paralution_backend_name [4];

/// Backend IDs
enum _paralution_backend_id {None=0,
                             GPU=1,
                             OCL=2,
                             MIC=3};



/// Initialization of the paralution platform
int init_paralution(void);

/// Shutdown the paralution platform
int stop_paralution(void);

/// Select a device
int set_device_paralution(int dev);

/// Set the number of threads in the platform
void set_omp_threads_paralution(int nthreads);

/// Set host affinity (true-on/false-off)
void set_omp_affinity(bool affinity);

/// Set a specific GPU device
void set_gpu_cuda_paralution(int ngpu);

/// Set a specific OpenCL platform and device
void set_ocl_paralution(int nplatform, int ndevice);

/// Set a specific OpenCL platform
void set_ocl_platform_paralution(int platform);

/// Set OpenCL work group size
void set_ocl_work_group_size_paralution(size_t size);

/// Set OpenCL compute units
void set_ocl_compute_units_paralution(size_t cu);

/// Set OpenCL warp size
void set_ocl_warp_size_paralution(int size);

/// Set OpenMP threshold size
void set_omp_threshold(const int threshold);

/// Print information about the platform
void info_paralution(void);

/// Print information about the platform via specific backend descriptor
void info_paralution(const struct Paralution_Backend_Descriptor backend_descriptor);

/// Return true if any accelerator is available
bool _paralution_available_accelerator(void);

/// Return backend descriptor
void _get_backend_descriptor(struct Paralution_Backend_Descriptor *backend_descriptor);

/// Set backend descriptor
void _set_backend_descriptor(const struct Paralution_Backend_Descriptor backend_descriptor);

/// Set the OMP threads based on the size threshold
void _set_omp_backend_threads(const struct Paralution_Backend_Descriptor backend_descriptor,
                              const int size);

/// Build (and return) a vector on the selected in the descriptor accelerator
template <typename ValueType>
AcceleratorVector<ValueType>* _paralution_init_base_backend_vector(const struct Paralution_Backend_Descriptor backend_descriptor);

/// Build (and return) a matrix on the host
template <typename ValueType>
HostMatrix<ValueType>* _paralution_init_base_host_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                         const unsigned int matrix_format);

/// Build (and return) a matrix on the selected in the descriptor accelerator
template <typename ValueType>
AcceleratorMatrix<ValueType>* _paralution_init_base_backend_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                   const unsigned int matrix_format);


/// Sync the active async transfers
void _paralution_sync(void);

}

#endif // PARALUTION_BACKEND_MANAGER_HPP_
