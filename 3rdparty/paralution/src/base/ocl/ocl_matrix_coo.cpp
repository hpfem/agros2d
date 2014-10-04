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
#include "../host/host_matrix_coo.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "ocl_utils.hpp"
#include "ocl_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <algorithm>

namespace paralution {

template <typename ValueType>
OCLAcceleratorMatrixCOO<ValueType>::OCLAcceleratorMatrixCOO() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixCOO<ValueType>::OCLAcceleratorMatrixCOO(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorMatrixCOO::OCLAcceleratorMatrixCOO()",
            "constructor with local_backend");

  this->mat_.row = NULL;
  this->mat_.col = NULL;
  this->mat_.val = NULL;
  this->set_backend(local_backend); 

}


template <typename ValueType>
OCLAcceleratorMatrixCOO<ValueType>::~OCLAcceleratorMatrixCOO() {

  LOG_DEBUG(this, "OCLAcceleratorMatrixCOO::~OCLAcceleratorMatrixCOO()",
            "destructor");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorMatrixCOO<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::AllocateCOO(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_ocl<int>      (nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.row);
    allocate_ocl<int>      (nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.col);
    allocate_ocl<ValueType>(nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.val);

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Set entries of device object to zero
    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nnz,
                    0,
                    mat_.row);

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
void OCLAcceleratorMatrixCOO<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_ocl(&this->mat_.row);
    free_ocl(&this->mat_.col);
    free_ocl(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }


}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to OCL copy
  if ((cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&src)) != NULL) {
    
    if (this->get_nnz() == 0)
      this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    if (this->get_nnz() > 0) {

      assert((this->get_nnz()  == src.get_nnz())  &&
             (this->get_nrow() == src.get_nrow()) &&
             (this->get_ncol() == src.get_ncol()) );

      // Copy object from host to device memory
      ocl_host2dev<int>(this->get_nnz(), // size
                        cast_mat->mat_.row, // src
                        this->mat_.row,     // dst
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
    
    LOG_INFO("Error unsupported OCL matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixCOO<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

    if (dst->get_nnz() == 0)
      cast_mat->AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol() );

    if (this->get_nnz() > 0) {

      assert((this->get_nnz()  == dst->get_nnz())  &&
             (this->get_nrow() == dst->get_nrow()) &&
             (this->get_ncol() == dst->get_ncol()) );

      // Copy object from device to host memory
      ocl_dev2host<int>(this->get_nnz(), // size
                        this->mat_.row,     // src
                        cast_mat->mat_.row, // dst
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
    
    LOG_INFO("Error unsupported OCL matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const OCLAcceleratorMatrixCOO<ValueType> *ocl_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<const OCLAcceleratorMatrixCOO<ValueType>*> (&src)) != NULL) {
    
    if (this->get_nnz() == 0)
      this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<int>(this->get_nnz(), // size
                       ocl_cast_mat->mat_.row, // src
                       this->mat_.row,         // dst
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
      
      LOG_INFO("Error unsupported OCL matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  OCLAcceleratorMatrixCOO<ValueType> *ocl_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<OCLAcceleratorMatrixCOO<ValueType>*> (dst)) != NULL) {

    ocl_cast_mat->set_backend(this->local_backend_);       

    if (this->get_nnz() == 0)
      ocl_cast_mat->AllocateCOO(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

      // Copy object from device to device memory (internal copy)
      ocl_dev2dev<int>(this->get_nnz(), // size
                       this->mat_.row,         // src
                       ocl_cast_mat->mat_.row, // dst
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
      
      LOG_INFO("Error unsupported OCL matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }


}


template <typename ValueType>
bool OCLAcceleratorMatrixCOO<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const OCLAcceleratorMatrixCOO<ValueType> *cast_mat_coo;
  
  if ((cast_mat_coo = dynamic_cast<const OCLAcceleratorMatrixCOO<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_coo);
      return true;

  }

  /*
    const OCLAcceleratorMatrixCSR<ValueType> *cast_mat_csr;
    if ((cast_mat_csr = dynamic_cast<const OCLAcceleratorMatrixCSR<ValueType>*> (&mat)) != NULL) {

      this->Clear();

      FATAL_ERROR(__FILE__, __LINE__);

      this->nrow_ = cast_mat_csr->get_nrow();
      this->ncol_ = cast_mat_csr->get_ncol();
      this->nnz_  = cast_mat_csr->get_nnz();

    return true;

  }
  */

  return false;

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  // TODO some devices hang up while waiting for COO_SPMV_FLAT event to finish
  // this is a bug we are fixing in some future release
  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const OCLAcceleratorVector<ValueType> *cast_in = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&in) ; 
    OCLAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      OCLAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    cast_out->Zeros();

    // do not support super small matrices
    assert(this->get_nnz() > this->local_backend_.OCL_warp_size); 

    // ----------------------------------------------------------
    // Modified and adopted from CUSP 0.3.1, 
    // http://code.google.com/p/cusp-library/
    // NVIDIA, APACHE LICENSE 2.0
    // ----------------------------------------------------------
    // see __spmv_coo_flat(...)
    // ----------------------------------------------------------
    // CHANGELOG
    // - adopted interface
    // ----------------------------------------------------------  

    //TODO
    //move in extra file -  max_active_blocks, warp_size, block_size

    const int BLOCK_SIZE = int(this->local_backend_.OCL_max_work_group_size);
    //    const unsigned int MAX_BLOCKS = this->local_backend_.GPU_max_blocks;

    const int MAX_BLOCKS = 32; //  cusp::detail::device::arch::max_active_blocks(spmv_coo_flat_kernel<IndexType, ValueType, BLOCK_SIZE, UseCache>, BLOCK_SIZE, (size_t) 0);

    const int WARPS_PER_BLOCK = BLOCK_SIZE / this->local_backend_.OCL_warp_size;


    const int num_units = this->get_nnz() / this->local_backend_.OCL_warp_size; 
    const unsigned int num_warps  = std::min(num_units, WARPS_PER_BLOCK * MAX_BLOCKS);
    const unsigned int num_blocks = (num_warps + (WARPS_PER_BLOCK-1)) / WARPS_PER_BLOCK; // (N + (granularity - 1)) / granularity
    const unsigned int num_iters  = (num_units +  (num_warps-1)) / num_warps;
    
    const unsigned int interval_size = this->local_backend_.OCL_warp_size * num_iters;

    const int tail = num_units * this->local_backend_.OCL_warp_size; // do the last few nonzeros separately (fewer than this->local_backend_.GPU_wrap elements)

    const unsigned int active_warps = (interval_size == 0) ? 0 : ((tail + (interval_size-1))/interval_size);

    cl_mem *temp_rows = NULL;
    cl_mem *temp_vals = NULL;

    allocate_ocl<int>      (active_warps, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &temp_rows);
    allocate_ocl<ValueType>(active_warps, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &temp_vals);

    //  LOG_INFO("active_warps = " << active_warps);
    //  LOG_INFO("tail =" << tail);
    //  LOG_INFO("interval_size =" << interval_size);
    //  LOG_INFO("num_iters =" << num_iters);
    //  LOG_INFO("num_blocks =" << num_blocks);
    //  LOG_INFO("num_warps =" << num_warps);
    //  LOG_INFO("num_units =" << num_units);
    //  LOG_INFO("WARPS_PER_BLOCK =" << WARPS_PER_BLOCK);
    //  LOG_INFO("MAX_BLOCKS =" << MAX_BLOCKS);
    //  LOG_INFO("BLOCK_SIZE =" << BLOCK_SIZE);
    //  LOG_INFO("WARP_SIZE =" << WARP_SIZE);
    //  LOG_INFO("WARP_SIZE =" << this->local_backend_.GPU_wrap);

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    ValueType scalar = 1.0;

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 0, sizeof(int),       (void *) &tail );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 1, sizeof(int),       (void *) &interval_size );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 2, sizeof(cl_mem),    (void *) this->mat_.row );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 3, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 5, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 6, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 7, sizeof(cl_mem),    (void *) cast_out->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 8, sizeof(cl_mem),    (void *) temp_rows );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 9, sizeof(cl_mem),    (void *) temp_vals );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = BLOCK_SIZE;
    // Determine global work size for kernel call
    globalWorkSize[0] = num_blocks * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_COO_SPMV_FLAT,
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
    err  = clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 0, sizeof(int),    (void *) &active_warps );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 1, sizeof(cl_mem), (void *) temp_rows );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 2, sizeof(cl_mem), (void *) temp_vals );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 3, sizeof(cl_mem), (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine global work size for kernel call
    globalWorkSize[0] = localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_COO_SPMV_REDUCE_UPDATE,
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

    int nnz = this->get_nnz();

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 0, sizeof(int),       (void *) &nnz );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 1, sizeof(cl_mem),    (void *) this->mat_.row );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 3, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 4, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 5, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 6, sizeof(cl_mem),    (void *) cast_out->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 7, sizeof(int),       (void *) &tail );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = 1;
    // Determine global work size for kernel call
    globalWorkSize[0] = 1;

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_COO_SPMV_SERIAL,
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
    
    free_ocl(&temp_rows);
    free_ocl(&temp_vals);

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixCOO<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                        BaseVector<ValueType> *out) const {

  // TODO some devices hang up while waiting for COO_SPMV_FLAT event to finish
  // this is a bug we are fixing in some future release
  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const OCLAcceleratorVector<ValueType> *cast_in = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&in) ; 
    OCLAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      OCLAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    // do not support super small matrices
    assert(this->get_nnz() > this->local_backend_.OCL_warp_size); 

    // ----------------------------------------------------------
    // Modified and adopted from CUSP 0.3.1, 
    // http://code.google.com/p/cusp-library/
    // NVIDIA, APACHE LICENSE 2.0
    // ----------------------------------------------------------
    // see __spmv_coo_flat(...)
    // ----------------------------------------------------------
    // CHANGELOG
    // - adopted interface
    // ----------------------------------------------------------  

    //TODO
    //move in extra file -  max_active_blocks, warp_size, block_size

    const int BLOCK_SIZE = int(this->local_backend_.OCL_max_work_group_size);
    //    const unsigned int MAX_BLOCKS = this->local_backend_.GPU_max_blocks;

    const unsigned int MAX_BLOCKS = 32; //  cusp::detail::device::arch::max_active_blocks(spmv_coo_flat_kernel<IndexType, ValueType, BLOCK_SIZE, UseCache>, BLOCK_SIZE, (size_t) 0);

    const unsigned int WARPS_PER_BLOCK = BLOCK_SIZE / this->local_backend_.OCL_warp_size;


    const unsigned int num_units  = this->get_nnz() / this->local_backend_.OCL_warp_size; 
    const unsigned int num_warps  = std::min(num_units, WARPS_PER_BLOCK * MAX_BLOCKS);
    const unsigned int num_blocks = (num_warps + (WARPS_PER_BLOCK-1)) / WARPS_PER_BLOCK; // (N + (granularity - 1)) / granularity
    const unsigned int num_iters  = (num_units +  (num_warps-1)) / num_warps;
    
    const unsigned int interval_size = this->local_backend_.OCL_warp_size * num_iters;

    const int tail = num_units * this->local_backend_.OCL_warp_size; // do the last few nonzeros separately (fewer than this->local_backend_.GPU_wrap elements)

    const unsigned int active_warps = (interval_size == 0) ? 0 : ((tail + (interval_size-1))/interval_size);

    cl_mem *temp_rows = NULL;
    cl_mem *temp_vals = NULL;

    allocate_ocl<int>      (active_warps, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &temp_rows);
    allocate_ocl<ValueType>(active_warps, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &temp_vals);

    //  LOG_INFO("active_warps = " << active_warps);
    //  LOG_INFO("tail =" << tail);
    //  LOG_INFO("interval_size =" << interval_size);
    //  LOG_INFO("num_iters =" << num_iters);
    //  LOG_INFO("num_blocks =" << num_blocks);
    //  LOG_INFO("num_warps =" << num_warps);
    //  LOG_INFO("num_units =" << num_units);
    //  LOG_INFO("WARPS_PER_BLOCK =" << WARPS_PER_BLOCK);
    //  LOG_INFO("MAX_BLOCKS =" << MAX_BLOCKS);
    //  LOG_INFO("BLOCK_SIZE =" << BLOCK_SIZE);
    //  LOG_INFO("WARP_SIZE =" << WARP_SIZE);
    //  LOG_INFO("WARP_SIZE =" << this->local_backend_.GPU_wrap);

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 0, sizeof(int),       (void *) &tail );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 1, sizeof(int),       (void *) &interval_size );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 2, sizeof(cl_mem),    (void *) this->mat_.row );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 3, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 5, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 6, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 7, sizeof(cl_mem),    (void *) cast_out->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 8, sizeof(cl_mem),    (void *) temp_rows );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 9, sizeof(cl_mem),    (void *) temp_vals );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = BLOCK_SIZE;
    // Determine global work size for kernel call
    globalWorkSize[0] = num_blocks * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_COO_SPMV_FLAT,
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
    err  = clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 0, sizeof(int),    (void *) &active_warps );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 1, sizeof(cl_mem), (void *) temp_rows );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 2, sizeof(cl_mem), (void *) temp_vals );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_REDUCE_UPDATE, 3, sizeof(cl_mem), (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine global work size for kernel call
    globalWorkSize[0] = localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_COO_SPMV_REDUCE_UPDATE,
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

    int nnz = this->get_nnz();

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 0, sizeof(int),       (void *) &nnz );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 1, sizeof(cl_mem),    (void *) this->mat_.row );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 3, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 4, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 5, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 6, sizeof(cl_mem),    (void *) cast_out->vec_ );
    err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 7, sizeof(int),       (void *) &tail );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = 1;
    // Determine global work size for kernel call
    globalWorkSize[0] = 1;

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_COO_SPMV_SERIAL,
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
    
    free_ocl(&temp_rows);
    free_ocl(&temp_vals);

  }

}


template <typename ValueType>
bool OCLAcceleratorMatrixCOO<ValueType>::Permute(const BaseVector<int> &permutation) {
  /*
  assert(&permutation != NULL);

  // symmetric permutation only
  assert( (permutation.get_size() == this->get_nrow()) &&
          (permutation.get_size() == this->get_ncol()) );

  if (this->get_nnz() > 0) {

    const OCLAcceleratorVector<int> *cast_perm = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);
    
    OCLAcceleratorMatrixCOO<ValueType> src(this->local_backend_);       
    src.AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol());
    src.CopyFrom(*this);

    LOG_INFO("OCLAcceleratorMatrixCOO::Permute NYI")
    FATAL_ERROR( __FILE__, __LINE__ );

  }
  */
  return false;
}


template <typename ValueType>
bool OCLAcceleratorMatrixCOO<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {
  /*
  assert(&permutation != NULL);

  // symmetric permutation only
  assert( (permutation.get_size() == this->get_nrow()) &&
          (permutation.get_size() == this->get_ncol()) );

  if (this->get_nnz() > 0) {

    const OCLAcceleratorVector<int> *cast_perm = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);

    LOG_INFO("OCLAcceleratorMatrixCOO::PermuteBackward NYI");
    FATAL_ERROR( __FILE__, __LINE__ );
    
  }
  */
  return false;
}


template class OCLAcceleratorMatrixCOO<double>;
template class OCLAcceleratorMatrixCOO<float>;

}

