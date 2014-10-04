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


#include "ocl_matrix_csr.hpp"
#include "ocl_matrix_coo.hpp"
#include "ocl_matrix_dia.hpp"
#include "ocl_matrix_ell.hpp"
#include "ocl_matrix_hyb.hpp"
#include "ocl_matrix_mcsr.hpp"
#include "ocl_matrix_bcsr.hpp"
#include "ocl_matrix_dense.hpp"
#include "ocl_vector.hpp"
#include "../host/host_matrix_csr.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "ocl_utils.hpp"
#include "ocl_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <string.h>

namespace paralution {

template <typename ValueType>
OCLAcceleratorMatrixCSR<ValueType>::OCLAcceleratorMatrixCSR() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixCSR<ValueType>::OCLAcceleratorMatrixCSR(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorMatrixCSR::OCLAcceleratorMatrixCSR()",
            "constructor with local_backend");

  this->mat_.row_offset = NULL;  
  this->mat_.col        = NULL;  
  this->mat_.val        = NULL;
  this->set_backend(local_backend); 

  this->tmp_vec_ = NULL;

}


template <typename ValueType>
OCLAcceleratorMatrixCSR<ValueType>::~OCLAcceleratorMatrixCSR() {

  LOG_DEBUG(this, "OCLAcceleratorMatrixCSR::~OCLAcceleratorMatrixCSR()",
            "destructor");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorMatrixCSR<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::AllocateCSR(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_ocl<int>   (nrow+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.row_offset);
    allocate_ocl<int>      (nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.col);
    allocate_ocl<ValueType>(nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.val);

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow+1 / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Set entries of device object to zero
    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nrow+1,
                    0,
                    mat_.row_offset);

    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Set entries of device object to zero
    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nnz,
                    0,
                    mat_.col);

    // Set entries of device object to zero
    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          nnz,
                          0.0,
                          mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_ocl(&this->mat_.row_offset);
    free_ocl(&this->mat_.col);
    free_ocl(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::Zeros() {

  if (this->get_nnz() > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( this->get_nnz() / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Set entries of device object to zero
    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          this->get_nnz(),
                          0.0,
                          mat_.val);

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to OCL copy
  if ((cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

   
    if (this->get_nnz() > 0) {

      // Copy object from host to device memory
      ocl_host2dev<int>((this->get_nrow()+1), // size
                        cast_mat->mat_.row_offset, // src
                        this->mat_.row_offset,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from host to device memory
      ocl_host2dev<int>(this->get_nnz(), // size
                        cast_mat->mat_.col, // src
                        this->mat_.col,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from host to device memory
      ocl_host2dev<ValueType>(this->get_nnz(), // size
                              cast_mat->mat_.val, // src
                              this->mat_.val,     // dst
                              OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }
    
  } else {
    
    LOG_INFO("Error unsupported OpenCL matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixCSR<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateCSR(this->get_nnz(), this->get_nrow(), this->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

   
    if (this->get_nnz() > 0) {

      // Copy object from device to host memory
      ocl_dev2host<int>((this->get_nrow()+1), // size
                        this->mat_.row_offset,     // src
                        cast_mat->mat_.row_offset, // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from device to host memory
      ocl_dev2host<int>(this->get_nnz(), // size
                        this->mat_.col,     // src
                        cast_mat->mat_.col, // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from device to host memory
      ocl_dev2host<ValueType>(this->get_nnz(), // size
                              this->mat_.val,     // src
                              cast_mat->mat_.val, // dst
                              OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }

    
  } else {
    
    LOG_INFO("Error unsupported OpenCL matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const OCLAcceleratorMatrixCSR<ValueType> *ocl_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<const OCLAcceleratorMatrixCSR<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<int>(this->get_nrow()+1, // size
                       ocl_cast_mat->mat_.row_offset, // src
                       this->mat_.row_offset,         // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<int>(this->get_nnz(), // size
                       ocl_cast_mat->mat_.col, // src
                       this->mat_.col,         // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<ValueType>(this->get_nnz(), // size
                             ocl_cast_mat->mat_.val, // src
                             this->mat_.val,         // dst
                             OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }

  } else {

    //CPU to OCL
    if ((host_cast_mat = dynamic_cast<const HostMatrix<ValueType>*> (&src)) != NULL) {
      
      this->CopyFromHost(*host_cast_mat);
      
    } else {
      
      LOG_INFO("Error unsupported OpenCL matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  OCLAcceleratorMatrixCSR<ValueType> *ocl_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<OCLAcceleratorMatrixCSR<ValueType>*> (dst)) != NULL) {

    ocl_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    ocl_cast_mat->AllocateCSR(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<int>(this->get_nrow()+1, // size
                       this->mat_.row_offset,         // src
                       ocl_cast_mat->mat_.row_offset, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<int>(this->get_nnz(), // size
                       this->mat_.col,         // src
                       ocl_cast_mat->mat_.col, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<ValueType>(this->get_nnz(), // size
                             this->mat_.val,         // src
                             ocl_cast_mat->mat_.val, // dst
                             OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );
    
    }
    
  } else {

    //OCL to CPU
    if ((host_cast_mat = dynamic_cast<HostMatrix<ValueType>*> (dst)) != NULL) {
      
      this->CopyToHost(host_cast_mat);

    } else {
      
      LOG_INFO("Error unsupported OpenCL matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  return false;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::Permute(const BaseVector<int> &permutation) {

  // TODO fix error in extrapolation kernel
  return false;

  assert(&permutation != NULL);
  assert((permutation.get_size() == this->get_nrow()) &&
         (permutation.get_size() == this->get_ncol()));

  if (this->get_nnz() > 0) {

    cl_mem *d_nnzr     = NULL;
    cl_mem *d_nnzrPerm = NULL;
    cl_mem *d_nnzPerm  = NULL;
    cl_mem *d_offset   = NULL;
    cl_mem *d_data     = NULL;

    allocate_ocl<int>(this->get_nrow(), OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &d_nnzr);
    allocate_ocl<int>(this->get_nrow(), OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &d_nnzrPerm);
    allocate_ocl<int>((this->get_nrow()+1), OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &d_nnzPerm);
    allocate_ocl<ValueType>(this->get_nnz(), OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &d_data);
    allocate_ocl<int>(this->get_nnz(), OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &d_offset);

    const OCLAcceleratorVector<int> *cast_perm = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_CALC_ROW_NNZ, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_CALC_ROW_NNZ, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_CALC_ROW_NNZ, 2, sizeof(cl_mem), (void *) d_nnzr );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_CALC_ROW_NNZ,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROW_NNZ, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROW_NNZ, 1, sizeof(cl_mem), (void *) d_nnzr );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROW_NNZ, 2, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROW_NNZ, 3, sizeof(cl_mem), (void *) d_nnzrPerm );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_PERMUTE_ROW_NNZ,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow+1 / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Set entries of device object to zero
    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nrow+1,
                    0,
                    d_nnzPerm);

    cl_mem *d_temp = NULL;

    allocate_ocl<int>(this->get_nrow()+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &d_temp);

    // Set entries of device object to zero
    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nrow+1,
                    0,
                    d_temp);

    int shift = 1;

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_RED_PARTIAL_SUM, 0, sizeof(cl_mem), (void *) d_nnzPerm );
    err |= clSetKernelArg( CL_KERNEL_RED_PARTIAL_SUM, 1, sizeof(cl_mem), (void *) d_nnzrPerm );
    err |= clSetKernelArg( CL_KERNEL_RED_PARTIAL_SUM, 2, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_RED_PARTIAL_SUM, 3, sizeof(int),    (void *) &shift );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_RED_PARTIAL_SUM,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    int nrowp = nrow+1;

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_RED_RECURSE, 0, sizeof(cl_mem), (void *) d_temp );
    err |= clSetKernelArg( CL_KERNEL_RED_RECURSE, 1, sizeof(cl_mem), (void *) d_nnzPerm );
    err |= clSetKernelArg( CL_KERNEL_RED_RECURSE, 2, sizeof(int),    (void *) &nrowp );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / (localWorkSize[0]*localWorkSize[0]) ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_RED_RECURSE,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    shift = 1;

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / (localWorkSize[0]*localWorkSize[0]) ) + 1 ) * localWorkSize[0];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_RED_EXTRAPOLATE, 0, sizeof(cl_mem), (void *) d_nnzPerm );
    err |= clSetKernelArg( CL_KERNEL_RED_EXTRAPOLATE, 1, sizeof(cl_mem), (void *) d_temp );
    err |= clSetKernelArg( CL_KERNEL_RED_EXTRAPOLATE, 2, sizeof(cl_mem), (void *) d_nnzrPerm );
    err |= clSetKernelArg( CL_KERNEL_RED_EXTRAPOLATE, 3, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_RED_EXTRAPOLATE, 4, sizeof(int),    (void *) &shift );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_RED_EXTRAPOLATE,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  0,
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    free_ocl(&d_temp);

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 2, sizeof(cl_mem), (void *) d_nnzPerm );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 3, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 4, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 5, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 6, sizeof(cl_mem), (void *) d_nnzr );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 7, sizeof(cl_mem), (void *) d_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_ROWS, 8, sizeof(cl_mem), (void *) d_data );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_PERMUTE_ROWS,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    free_ocl(&this->mat_.row_offset);	

    this->mat_.row_offset = d_nnzPerm;

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 2, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 3, sizeof(cl_mem), (void *) d_nnzrPerm );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 4, sizeof(cl_mem), (void *) d_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 5, sizeof(cl_mem), (void *) d_data );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 6, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_PERMUTE_COLS, 7, sizeof(cl_mem), (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_PERMUTE_COLS,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    free_ocl(&d_offset);
    free_ocl(&d_data);
    free_ocl(&d_nnzrPerm);
    free_ocl(&d_nnzr);

  }

  return true;

}

template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    const OCLAcceleratorVector<ValueType> *cast_in = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&in) ; 
    OCLAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      OCLAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_SPMV_SCALAR, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_SPMV_SCALAR, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_SPMV_SCALAR, 2, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_SPMV_SCALAR, 3, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_CSR_SPMV_SCALAR, 4, sizeof(cl_mem), (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_CSR_SPMV_SCALAR, 5, sizeof(cl_mem), (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
	  // Nathan Bell and Michael Garland
	  // Efficient Sparse Matrix-Vector Multiplication on {CUDA}
    // NVR-2008-004 / NVIDIA Technical Report
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_SPMV_SCALAR,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }
    
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                        BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    const OCLAcceleratorVector<ValueType> *cast_in = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&in) ; 
    OCLAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      OCLAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);    

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 1, sizeof(cl_mem),    (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 3, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 4, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 5, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_SPMV_SCALAR, 6, sizeof(cl_mem),    (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
	  // Nathan Bell and Michael Garland
	  // Efficient Sparse Matrix-Vector Multiplication on {CUDA}
    // NVR-2008-004 / NVIDIA Technical Report
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_ADD_SPMV_SCALAR,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }
    
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ILU0Factorize(void) {
  return false;
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::IC0Factorize(void) {
  return false;
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::LUAnalyse(void) {
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::LUAnalyseClear(void) {
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::LLAnalyse(void) {
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::LLAnalyseClear(void) {
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::LLSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::LLSolve(const BaseVector<ValueType> &in, const BaseVector<ValueType> &inv_diag,
                                                 BaseVector<ValueType> *out) const {
  return false;
}

template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::LAnalyse(const bool diag_unit) {
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::UAnalyse(const bool diag_unit) {
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::LAnalyseClear(void) {
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::UAnalyseClear(void) {
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::LSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::USolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractDiagonal(BaseVector<ValueType> *vec_diag) const {

  if (this->get_nnz() > 0)  {

    assert(vec_diag != NULL);
    assert(vec_diag->get_size() == this->get_nrow());
    
    OCLAcceleratorVector<ValueType> *cast_vec_diag  = dynamic_cast<OCLAcceleratorVector<ValueType>*> (vec_diag);

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_DIAG, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_DIAG, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_DIAG, 2, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_DIAG, 3, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_DIAG, 4, sizeof(cl_mem), (void *) cast_vec_diag->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_EXTRACT_DIAG,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractInverseDiagonal(BaseVector<ValueType> *vec_inv_diag) const { 

  if (this->get_nnz() > 0)  {

    assert(vec_inv_diag != NULL);
    assert(vec_inv_diag->get_size() == this->get_nrow());
    
    OCLAcceleratorVector<ValueType> *cast_vec_inv_diag  = dynamic_cast<OCLAcceleratorVector<ValueType>*> (vec_inv_diag);

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_INV_DIAG, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_INV_DIAG, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_INV_DIAG, 2, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_INV_DIAG, 3, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_INV_DIAG, 4, sizeof(cl_mem), (void *) cast_vec_inv_diag->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_EXTRACT_INV_DIAG,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractSubMatrix(const int row_offset,
                                                          const int col_offset,
                                                          const int row_size,
                                                          const int col_size,
                                                          BaseMatrix<ValueType> *mat) const {

  assert(mat != NULL);

  assert(row_offset >= 0);
  assert(col_offset >= 0);

  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);

  OCLAcceleratorMatrixCSR<ValueType> *cast_mat  = dynamic_cast<OCLAcceleratorMatrixCSR<ValueType>*> (mat) ; 
  assert(cast_mat != NULL);

  int mat_nnz = 0;

  cl_mem *row_nnz = NULL;
  int *red_row_nnz = NULL;

  allocate_host(row_size+1, &red_row_nnz);
  allocate_ocl<int>(row_size+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &row_nnz);

  // compute the nnz per row in the new matrix

  cl_int    err;
  cl_event  ocl_event;
  size_t    localWorkSize[1];
  size_t    globalWorkSize[1];

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 0, sizeof(cl_mem), (void *) this->mat_.row_offset );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 1, sizeof(cl_mem), (void *) this->mat_.col );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 2, sizeof(cl_mem), (void *) this->mat_.val );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 3, sizeof(int),    (void *) &row_offset );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 4, sizeof(int),    (void *) &col_offset );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 5, sizeof(int),    (void *) &row_size );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 6, sizeof(int),    (void *) &col_size );
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ, 7, sizeof(cl_mem), (void *) row_nnz );
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Determine local work size for kernel call
  localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
  localWorkSize[0] /= 2;
  // Determine global work size for kernel call
  globalWorkSize[0] = ( size_t( ( row_size + 1 ) / localWorkSize[0] ) + 1 ) * localWorkSize[0];

  // Start kernel run
  err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ,
                                1,
                                NULL,
                                &globalWorkSize[0],
                                &localWorkSize[0],
                                0,
                                NULL,
                                &ocl_event);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Wait for kernel run to finish
  err = clWaitForEvents( 1, &ocl_event );
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Release event when kernel run finished
  err = clReleaseEvent( ocl_event );
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // compute the new nnz by reduction on CPU

  // Copy object from device to host memory
  ocl_dev2host<int>((row_size+1), row_nnz, red_row_nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  int sum = 0;
  for (int i=0; i<row_size; ++i) {
    int tmp = red_row_nnz[i];
    red_row_nnz[i] = sum;
    sum += tmp;
  }

  mat_nnz = red_row_nnz[row_size] = sum;


  // TODO
  //  redSubSum


  // not empty submatrix
  if (mat_nnz > 0) {

    cast_mat->AllocateCSR(mat_nnz, row_size, col_size);

    // part of the CPU reduction section
    // Copy object from host to device memory
    ocl_host2dev<int>((row_size+1), red_row_nnz, cast_mat->mat_.row_offset, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

    // copying the sub matrix

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 0, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 1, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 2, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 3, sizeof(int),    (void *) &row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 4, sizeof(int),    (void *) &col_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 5, sizeof(int),    (void *) &row_size );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 6, sizeof(int),    (void *) &col_size );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 7, sizeof(cl_mem), (void *) cast_mat->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 8, sizeof(cl_mem), (void *) cast_mat->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY, 9, sizeof(cl_mem), (void *) cast_mat->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( ( row_size + 1 ) / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  free_ocl(&row_nnz);
  free_host(&red_row_nnz);

  return true;

}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractL(BaseMatrix<ValueType> *L) const {

  assert(L != NULL);
  
  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);
  
  OCLAcceleratorMatrixCSR<ValueType> *cast_L = dynamic_cast<OCLAcceleratorMatrixCSR<ValueType>*> (L);
  
  assert(cast_L != NULL);
  
  cast_L->Clear();
  
  // compute nnz per row
  int nrow = this->get_nrow();
  
  allocate_ocl<int>(nrow+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_L->mat_.row_offset);

  cl_int    err;
  cl_event  ocl_event;
  size_t    localWorkSize[1];
  size_t    globalWorkSize[1];

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW, 3, sizeof(cl_mem), (void *) cast_L->mat_.row_offset);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Determine local work size for kernel call
  localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
  // Determine global work size for kernel call
  globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

  // Start kernel run
	// Computes the stricktly lower triangular part nnz per row
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  
  // partial sum row_nnz to obtain row_offset vector
  // TODO currently performing partial sum on host
  int *h_buffer = NULL;
  allocate_host(nrow+1, &h_buffer);

  ocl_dev2host<int>(nrow+1, // size
                    cast_L->mat_.row_offset,     // src
                    h_buffer, // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  h_buffer[0] = 0;
  for (int i=1; i<nrow+1; ++i)
    h_buffer[i] += h_buffer[i-1];
  
  int nnz_L = h_buffer[nrow];

  ocl_host2dev<int>(nrow+1, // size
                    h_buffer, // src
                    cast_L->mat_.row_offset,     // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  free_host(&h_buffer);
  // end TODO
  
  // allocate lower triangular part structure
  allocate_ocl<int>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_L->mat_.col);
  allocate_ocl<ValueType>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_L->mat_.val);

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 3, sizeof(cl_mem), (void *) this->mat_.val);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 4, sizeof(cl_mem), (void *) cast_L->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 5, sizeof(cl_mem), (void *) cast_L->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 6, sizeof(cl_mem), (void *) cast_L->mat_.val);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Start kernel run
	// Extracts lower/upper triangular part for given nnz per row array (partial sums nnz)
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  cast_L->nrow_ = this->get_nrow();
  cast_L->ncol_ = this->get_ncol();
  cast_L->nnz_  = nnz_L;
  
  return true;
  
}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractLDiagonal(BaseMatrix<ValueType> *L) const {

  assert(L != NULL);
  
  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);
  
  OCLAcceleratorMatrixCSR<ValueType> *cast_L = dynamic_cast<OCLAcceleratorMatrixCSR<ValueType>*> (L);
  
  assert(cast_L != NULL);
  
  cast_L->Clear();
  
  // compute nnz per row
  int nrow = this->get_nrow();
  
  allocate_ocl<int>(nrow+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_L->mat_.row_offset);

  cl_int    err;
  cl_event  ocl_event;
  size_t    localWorkSize[1];
  size_t    globalWorkSize[1];

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_LOWER_NNZ_PER_ROW, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_LOWER_NNZ_PER_ROW, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_LOWER_NNZ_PER_ROW, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_LOWER_NNZ_PER_ROW, 3, sizeof(cl_mem), (void *) cast_L->mat_.row_offset);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Determine local work size for kernel call
  localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
  // Determine global work size for kernel call
  globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

  // Start kernel run
	// Computes the lower triangular part nnz per row
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_LOWER_NNZ_PER_ROW,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // partial sum row_nnz to obtain row_offset vector
  // TODO currently performing partial sum on host
  int *h_buffer = NULL;
  allocate_host(nrow+1, &h_buffer);

  ocl_dev2host<int>(nrow+1, // size
                    cast_L->mat_.row_offset,     // src
                    h_buffer, // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
  
  h_buffer[0] = 0;
  for (int i=1; i<nrow+1; ++i)
    h_buffer[i] += h_buffer[i-1];

  int nnz_L = h_buffer[nrow];

  ocl_host2dev<int>(nrow+1, // size
                    h_buffer, // src
                    cast_L->mat_.row_offset,     // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  free_host(&h_buffer);
  // end TODO
  
  // allocate lower triangular part structure
  allocate_ocl<int>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_L->mat_.col);
  allocate_ocl<ValueType>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_L->mat_.val);

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 3, sizeof(cl_mem), (void *) this->mat_.val);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 4, sizeof(cl_mem), (void *) cast_L->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 5, sizeof(cl_mem), (void *) cast_L->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR, 6, sizeof(cl_mem), (void *) cast_L->mat_.val);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Start kernel run
	// Extracts lower/upper triangular part for given nnz per row array (partial sums nnz)
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  cast_L->nrow_ = this->get_nrow();
  cast_L->ncol_ = this->get_ncol();
  cast_L->nnz_ = nnz_L;

  return true;

}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractU(BaseMatrix<ValueType> *U) const {

  assert(U != NULL);
  
  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);
  
  OCLAcceleratorMatrixCSR<ValueType> *cast_U = dynamic_cast<OCLAcceleratorMatrixCSR<ValueType>*> (U);
  
  assert(cast_U != NULL);
  
  cast_U->Clear();
  
  // compute nnz per row
  int nrow = this->get_nrow();
  
  allocate_ocl<int>(nrow+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_U->mat_.row_offset);

  cl_int    err;
  cl_event  ocl_event;
  size_t    localWorkSize[1];
  size_t    globalWorkSize[1];

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW, 3, sizeof(cl_mem), (void *) cast_U->mat_.row_offset);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Determine local work size for kernel call
  localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
  // Determine global work size for kernel call
  globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

  // Start kernel run
	// Computes the stricktly upper triangular part nnz per row
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  
  // partial sum row_nnz to obtain row_offset vector
  // TODO currently performing partial sum on host
  int *h_buffer = NULL;
  allocate_host(nrow+1, &h_buffer);

  ocl_dev2host<int>(nrow+1, // size
                    cast_U->mat_.row_offset,     // src
                    h_buffer, // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  h_buffer[0] = 0;
  for (int i=1; i<nrow+1; ++i)
    h_buffer[i] += h_buffer[i-1];
  
  int nnz_L = h_buffer[nrow];

  ocl_host2dev<int>(nrow+1, // size
                    h_buffer, // src
                    cast_U->mat_.row_offset,     // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  free_host(&h_buffer);
  // end TODO
  
  // allocate lower triangular part structure
  allocate_ocl<int>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_U->mat_.col);
  allocate_ocl<ValueType>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_U->mat_.val);

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 3, sizeof(cl_mem), (void *) this->mat_.val);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 4, sizeof(cl_mem), (void *) cast_U->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 5, sizeof(cl_mem), (void *) cast_U->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 6, sizeof(cl_mem), (void *) cast_U->mat_.val);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Start kernel run
	// Extracts lower/upper triangular part for given nnz per row array (partial sums nnz)
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  cast_U->nrow_ = this->get_nrow();
  cast_U->ncol_ = this->get_ncol();
  cast_U->nnz_  = nnz_L;

  return true;

}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ExtractUDiagonal(BaseMatrix<ValueType> *U) const {

  assert(U != NULL);

  assert(this->get_nrow() > 0);
  assert(this->get_ncol() > 0);
  
  OCLAcceleratorMatrixCSR<ValueType> *cast_U = dynamic_cast<OCLAcceleratorMatrixCSR<ValueType>*> (U);
  
  assert(cast_U != NULL);
  
  cast_U->Clear();
  
  // compute nnz per row
  int nrow = this->get_nrow();
  
  allocate_ocl<int>(nrow+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_U->mat_.row_offset);

  cl_int    err;
  cl_event  ocl_event;
  size_t    localWorkSize[1];
  size_t    globalWorkSize[1];

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_UPPER_NNZ_PER_ROW, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_UPPER_NNZ_PER_ROW, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_UPPER_NNZ_PER_ROW, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_UPPER_NNZ_PER_ROW, 3, sizeof(cl_mem), (void *) cast_U->mat_.row_offset);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Determine local work size for kernel call
  localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
  // Determine global work size for kernel call
  globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

  // Start kernel run
	// Computes the upper triangular part nnz per row
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_UPPER_NNZ_PER_ROW,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // partial sum row_nnz to obtain row_offset vector
  // TODO currently performing partial sum on host
  int *h_buffer = NULL;
  allocate_host(nrow+1, &h_buffer);

  ocl_dev2host<int>(nrow+1, // size
                    cast_U->mat_.row_offset,     // src
                    h_buffer, // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  h_buffer[0] = 0;
  for (int i=1; i<nrow+1; ++i)
    h_buffer[i] += h_buffer[i-1];

  int nnz_L = h_buffer[nrow];

  ocl_host2dev<int>(nrow+1, // size
                    h_buffer, // src
                    cast_U->mat_.row_offset, // dst
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  free_host(&h_buffer);
  // end TODO

  // allocate lower triangular part structure
  allocate_ocl<int>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_U->mat_.col);
  allocate_ocl<ValueType>(nnz_L, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &cast_U->mat_.val);

  // Set arguments for kernel call
  err  = clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 0, sizeof(int),    (void *) &nrow);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 1, sizeof(cl_mem), (void *) this->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 2, sizeof(cl_mem), (void *) this->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 3, sizeof(cl_mem), (void *) this->mat_.val);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 4, sizeof(cl_mem), (void *) cast_U->mat_.row_offset);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 5, sizeof(cl_mem), (void *) cast_U->mat_.col);
  err |= clSetKernelArg( CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR, 6, sizeof(cl_mem), (void *) cast_U->mat_.val);
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  // Start kernel run
	// Extracts lower/upper triangular part for given nnz per row array (partial sums nnz)
  err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                               CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR,
                               1,
                               NULL,
                               &globalWorkSize[0],
                               &localWorkSize[0],
                               0,
                               NULL,
                               &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Wait for kernel run to finish
  err = clWaitForEvents(1, &ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Release event when kernel run finished
  err = clReleaseEvent(ocl_event);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  cast_U->nrow_ = this->get_nrow();
  cast_U->ncol_ = this->get_ncol();
  cast_U->nnz_  = nnz_L;

  return true;

}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::MaximalIndependentSet(int &size,
                                                               BaseVector<int> *permutation) const {
  assert(permutation != NULL);
  OCLAcceleratorVector<int> *cast_perm = dynamic_cast<OCLAcceleratorVector<int>*> (permutation);
  assert(cast_perm != NULL);
  assert(this->get_nrow() == this->get_ncol());

  int *h_row_offset = NULL;
  int *h_col = NULL;

  allocate_host(this->get_nrow()+1, &h_row_offset);
  allocate_host(this->get_nnz(), &h_col);

  ocl_dev2host(this->get_nrow()+1, this->mat_.row_offset, h_row_offset,
               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
  ocl_dev2host(this->get_nnz(), this->mat_.col, h_col,
               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  int *mis = NULL;
  allocate_host(this->get_nrow(), &mis);
  memset(mis, 0, sizeof(int)*this->get_nrow());

  size = 0 ;

  for (int ai=0; ai<this->get_nrow(); ++ai) {

    if (mis[ai] == 0) {

      // set the node
      mis[ai] = 1;
      ++size ;

      //remove all nbh nodes (without diagonal)
      for (int aj=h_row_offset[ai]; aj<h_row_offset[ai+1]; ++aj)
        if (ai != h_col[aj])
          mis[h_col[aj]] = -1 ;
      
    }
  }

  int *h_perm = NULL;
  allocate_host(this->get_nrow(), &h_perm);

  int pos = 0;
  for (int ai=0; ai<this->get_nrow(); ++ai) {

    if (mis[ai] == 1) {

      h_perm[ai] = pos;
      ++pos;

    } else {

      h_perm[ai] = size + ai - pos;

    }

  }
  
  // Check the permutation
  //
  //  for (int ai=0; ai<this->get_nrow(); ++ai) {
  //    assert( h_perm[ai] >= 0 );
  //    assert( h_perm[ai] < this->get_nrow() );
  //  }


  cast_perm->Allocate(this->get_nrow());
  ocl_host2dev(permutation->get_size(), h_perm, cast_perm->vec_,
               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  free_host(&h_row_offset);
  free_host(&h_col);

  free_host(&h_perm);
  free_host(&mis);

  return true;
}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::MultiColoring(int &num_colors,
                                                       int **size_colors,
                                                       BaseVector<int> *permutation) const {

  assert(permutation != NULL);
  OCLAcceleratorVector<int> *cast_perm = dynamic_cast<OCLAcceleratorVector<int>*> (permutation);
  assert(cast_perm != NULL);

  // node colors (init value = 0 i.e. no color)
  int *color = NULL;
  int *h_row_offset = NULL;
  int *h_col = NULL;
  int size = this->get_nrow();
  allocate_host(size, &color);
  allocate_host(this->get_nrow()+1, &h_row_offset);
  allocate_host(this->get_nnz(), &h_col);

  ocl_dev2host(this->get_nrow()+1, this->mat_.row_offset, h_row_offset,
               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
  ocl_dev2host(this->get_nnz(), this->mat_.col, h_col,
               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  memset(color, 0, size*sizeof(int));
  num_colors = 0;
  std::vector<bool> row_col;

  for (int ai=0; ai<this->get_nrow(); ++ai) {
    color[ai] = 1;
    row_col.clear();
    row_col.assign(num_colors+2, false);

    for (int aj=h_row_offset[ai]; aj<h_row_offset[ai+1]; ++aj)
      if (ai != h_col[aj])
        row_col[color[h_col[aj]]] = true;

    for (int aj=h_row_offset[ai]; aj<h_row_offset[ai+1]; ++aj)
      if (row_col[color[ai]] == true)
        ++color[ai];

    if (color[ai] > num_colors)
      num_colors = color[ai];

  }

  free_host(&h_row_offset);
  free_host(&h_col);

  allocate_host(num_colors, size_colors);
  set_to_zero_host(num_colors, *size_colors);

  int *offsets_color = NULL;
  allocate_host(num_colors, &offsets_color);
  memset(offsets_color, 0, sizeof(int)*num_colors);

  for (int i=0; i<this->get_nrow(); ++i) 
    ++(*size_colors)[color[i]-1];

  int total=0;
  for (int i=1; i<num_colors; ++i) {

    total += (*size_colors)[i-1];
    offsets_color[i] = total; 
    //   LOG_INFO("offsets = " << total);

  }

  int *h_perm = NULL;
  allocate_host(this->get_nrow(), &h_perm);

  for (int i=0; i<this->get_nrow(); ++i) {

    h_perm[i] = offsets_color[ color[i]-1 ] ;
    ++offsets_color[color[i]-1];

  }

  cast_perm->Allocate(this->get_nrow());
  ocl_host2dev(permutation->get_size(), h_perm, cast_perm->vec_,
               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

  free_host(&h_perm);
  free_host(&color);
  free_host(&offsets_color);

  return true;

}

template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::Scale(const ValueType alpha) { 

  if (this->get_nnz() > 0) {

    int       nnz = this->get_nnz();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_SCALE, 0, sizeof(int),       (void *) &nnz );
    err |= clSetKernelArg( CL_KERNEL_SCALE, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_SCALE, 2, sizeof(cl_mem),    (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_SCALE,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ScaleDiagonal(const ValueType alpha) { 

  if (this->get_nnz() > 0) {

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_SCALE_DIAGONAL, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_DIAGONAL, 1, sizeof(cl_mem),    (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_DIAGONAL, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_DIAGONAL, 3, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_DIAGONAL, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_SCALE_DIAGONAL,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::ScaleOffDiagonal(const ValueType alpha) {

  if (this->get_nnz() > 0) {

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_SCALE_OFFDIAGONAL, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_OFFDIAGONAL, 1, sizeof(cl_mem),    (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_OFFDIAGONAL, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_OFFDIAGONAL, 3, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_CSR_SCALE_OFFDIAGONAL, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_SCALE_OFFDIAGONAL,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::AddScalarDiagonal(const ValueType alpha) {

  if (this->get_nnz() > 0) {

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_ADD_DIAGONAL, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_DIAGONAL, 1, sizeof(cl_mem),    (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_DIAGONAL, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_DIAGONAL, 3, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_DIAGONAL, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_ADD_DIAGONAL,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}

 
template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::AddScalarOffDiagonal(const ValueType alpha) {

  if (this->get_nnz() > 0) {

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_ADD_OFFDIAGONAL, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_OFFDIAGONAL, 1, sizeof(cl_mem),    (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_OFFDIAGONAL, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_OFFDIAGONAL, 3, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_CSR_ADD_OFFDIAGONAL, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_ADD_OFFDIAGONAL,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::AddScalar(const ValueType alpha) { 

  if (this->get_nnz() > 0) {

    int nnz = this->get_nnz();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_BUFFER_ADDSCALAR, 0, sizeof(int),       (void *) &nnz );
    err |= clSetKernelArg( CL_KERNEL_BUFFER_ADDSCALAR, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_BUFFER_ADDSCALAR, 2, sizeof(cl_mem),    (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_BUFFER_ADDSCALAR,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::DiagonalMatrixMult(const BaseVector<ValueType> &diag) {

  assert(diag.get_size() == this->get_ncol());
  
  const OCLAcceleratorVector<ValueType> *cast_diag = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&diag) ; 
  assert(cast_diag!= NULL);

  if (this->get_nnz() > 0) {

    int nrow = this->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_DIAGMATMULT, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_CSR_DIAGMATMULT, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_CSR_DIAGMATMULT, 2, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_CSR_DIAGMATMULT, 3, sizeof(cl_mem), (void *) cast_diag->vec_ );
    err |= clSetKernelArg( CL_KERNEL_CSR_DIAGMATMULT, 4, sizeof(cl_mem), (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_CSR_DIAGMATMULT,
                                  1,
                                  NULL,
                                  &globalWorkSize[0],
                                  &localWorkSize[0],
                                  0,
                                  NULL,
                                  &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );
  }

  return true;

}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::SymbolicPower(const int p) {
  FATAL_ERROR(__FILE__, __LINE__);
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &src) {
  FATAL_ERROR(__FILE__, __LINE__);
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {
  return false;
}


template <typename ValueType>
void OCLAcceleratorMatrixCSR<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &A,
                                                            const BaseMatrix<ValueType> &B) {
  FATAL_ERROR(__FILE__, __LINE__);
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::Gershgorin(ValueType &lambda_min,
                                                    ValueType &lambda_max) const {
  return false;
}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                                                   const ValueType beta, const bool structure) {

  if (this->get_nnz() > 0) {

    const OCLAcceleratorMatrixCSR<ValueType> *cast_mat = dynamic_cast<const OCLAcceleratorMatrixCSR<ValueType>*> (&mat);
    assert(cast_mat != NULL);
    
    assert(cast_mat->get_nrow() == this->get_nrow());
    assert(cast_mat->get_ncol() == this->get_ncol());
    assert(this    ->get_nnz() > 0);  
    assert(cast_mat->get_nnz() > 0);

    if (structure == false) {

      int nrow = this->get_nrow();

      cl_int    err;
      cl_event  ocl_event;
      size_t    localWorkSize[1];
      size_t    globalWorkSize[1];

      // Set arguments for kernel call
      err  = clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 0, sizeof(int),    (void *) &nrow );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 2, sizeof(cl_mem), (void *) this->mat_.col );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 3, sizeof(cl_mem), (void *) cast_mat->mat_.row_offset );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 4, sizeof(cl_mem), (void *) cast_mat->mat_.col );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 5, sizeof(cl_mem), (void *) cast_mat->mat_.val );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 6, sizeof(int),    (void *) &alpha );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 7, sizeof(int),    (void *) &beta );
      err |= clSetKernelArg( CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT, 8, sizeof(cl_mem), (void *) this->mat_.val );
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

      // Determine local work size for kernel call
      localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
      localWorkSize[0] /= 2;
      // Determine global work size for kernel call
      globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

      // Start kernel run
      err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                    CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT,
                                    1,
                                    NULL,
                                    &globalWorkSize[0],
                                    &localWorkSize[0],
                                    0,
                                    NULL,
                                    &ocl_event);
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

      // Wait for kernel run to finish
      err = clWaitForEvents( 1, &ocl_event );
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

      // Release event when kernel run finished
      err = clReleaseEvent( ocl_event );
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    } else {

      return false;

      // New structure
      LOG_INFO("OCL MatrixAdd NYI for new structures");
      FATAL_ERROR( __FILE__, __LINE__ );
      
    }

  }

  return true;

}


template <typename ValueType>
bool OCLAcceleratorMatrixCSR<ValueType>::Compress(const ValueType drop_off) {

  if (this->get_nnz() > 0) {

    OCLAcceleratorMatrixCSR<ValueType> tmp(this->local_backend_);

    tmp.CopyFrom(*this);

    int mat_nnz = 0;

    cl_mem *row_offset = NULL;
    allocate_ocl<int>(this->get_nrow()+1, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &row_offset);

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];
    int       nrow = this->get_nrow();

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( (this->get_nrow()+1) / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    this->get_nrow()+1,
                    0,
                    row_offset);

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COUNT_NROW, 0, sizeof(cl_mem),    (void *) this->mat_.row_offset);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COUNT_NROW, 1, sizeof(cl_mem),    (void *) this->mat_.col);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COUNT_NROW, 2, sizeof(cl_mem),    (void *) this->mat_.val);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COUNT_NROW, 3, sizeof(int),       (void *) &nrow);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COUNT_NROW, 4, sizeof(ValueType), (void *) &drop_off);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COUNT_NROW, 5, sizeof(cl_mem),    (void *) row_offset);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                 CL_KERNEL_CSR_COMPRESS_COUNT_NROW,
                                 1,
                                 NULL,
                                 &globalWorkSize[0],
                                 &localWorkSize[0],
                                 0,
                                 NULL,
                                 &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    int *red_row_offset = NULL;
    allocate_host(nrow+1, &red_row_offset);

    // Copy object from device to host memory
    ocl_dev2host<int>((nrow+1), row_offset, red_row_offset, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

    int sum = 0;
    for (int i=0; i<nrow; ++i) {
      int tmp = red_row_offset[i];
      red_row_offset[i] = sum;
      sum += tmp;
    }

    mat_nnz = red_row_offset[nrow] = sum;

    this->AllocateCSR(mat_nnz, this->get_nrow(), this->get_ncol());

    ocl_host2dev<int>((nrow+1), red_row_offset, this->mat_.row_offset, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

    free_host(&red_row_offset);

    nrow = tmp.get_nrow();

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 0, sizeof(cl_mem),    (void *) tmp.mat_.row_offset);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 1, sizeof(cl_mem),    (void *) tmp.mat_.col);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 2, sizeof(cl_mem),    (void *) tmp.mat_.val);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 3, sizeof(int),       (void *) &nrow);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 4, sizeof(ValueType), (void *) &drop_off);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 5, sizeof(cl_mem),    (void *) this->mat_.row_offset);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 6, sizeof(cl_mem),    (void *) this->mat_.col);
    err |= clSetKernelArg( CL_KERNEL_CSR_COMPRESS_COPY, 7, sizeof(cl_mem),    (void *) this->mat_.val);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Start kernel run
    err = clEnqueueNDRangeKernel(OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                 CL_KERNEL_CSR_COMPRESS_COPY,
                                 1,
                                 NULL,
                                 &globalWorkSize[0],
                                 &localWorkSize[0],
                                 0,
                                 NULL,
                                 &ocl_event);
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Wait for kernel run to finish
    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Release event when kernel run finished
    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    free_ocl(&row_offset);

  }

  return true;

}


template class OCLAcceleratorMatrixCSR<double>;
template class OCLAcceleratorMatrixCSR<float>;

}
