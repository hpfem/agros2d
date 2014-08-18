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
#include "../host/host_matrix_hyb.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "ocl_utils.hpp"
#include "ocl_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
#include <algorithm>

namespace paralution {


template <typename ValueType>
OCLAcceleratorMatrixHYB<ValueType>::OCLAcceleratorMatrixHYB() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixHYB<ValueType>::OCLAcceleratorMatrixHYB(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorMatrixHYB::OCLAcceleratorMatrixHYB()",
            "constructor with local_backend");

  this->mat_.ELL.val = NULL;
  this->mat_.ELL.col = NULL;
  this->mat_.ELL.max_row = 0;

  this->mat_.COO.row = NULL;
  this->mat_.COO.col = NULL;
  this->mat_.COO.val = NULL;

  this->ell_nnz_ = 0;
  this->coo_nnz_ = 0;

  this->set_backend(local_backend); 

  // not working yet
  LOG_INFO("HYB format is not yet implemented for OpenCL backend");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixHYB<ValueType>::~OCLAcceleratorMatrixHYB() {

  LOG_DEBUG(this, "OCLAcceleratorMatrixHYB::~OCLAcceleratorMatrixHYB()",
            "destructor");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorMatrixHYB<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorMatrixHYB<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorMatrixHYB<ValueType>::AllocateHYB(const int ell_nnz, const int coo_nnz, const int ell_max_row, 
                                                     const int nrow, const int ncol) {

  assert( ell_nnz   >= 0);
  assert( coo_nnz   >= 0);
  assert( ell_max_row >= 0);

  assert( ncol  >= 0);
  assert( nrow  >= 0);
  
  if (this->get_nnz() > 0)
    this->Clear();

  if (ell_nnz + coo_nnz > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( ell_nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];
    
    // ELL
    assert(ell_nnz == ell_max_row*nrow);

    allocate_ocl<int>      (ell_nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.ELL.col);
    allocate_ocl<ValueType>(ell_nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.ELL.val);

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    ell_nnz,
                    0,
                    mat_.ELL.col);

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          ell_nnz,
                          0.0,
                          mat_.ELL.val);

    this->mat_.ELL.max_row = ell_max_row;
    this->ell_nnz_ = ell_nnz;

    // COO
    globalWorkSize[0] = ( size_t( coo_nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    allocate_ocl<int>      (coo_nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.COO.row);
    allocate_ocl<int>      (coo_nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.COO.col);
    allocate_ocl<ValueType>(coo_nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.COO.val);

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    coo_nnz,
                    0,
                    mat_.COO.row);

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    coo_nnz,
                    0,
                    mat_.COO.col);

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          coo_nnz,
                          0.0,
                          mat_.COO.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = ell_nnz + coo_nnz;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixHYB<ValueType>::Clear() {

  if (this->get_nnz() > 0) {
    
    free_ocl(&this->mat_.COO.row);
    free_ocl(&this->mat_.COO.col);
    free_ocl(&this->mat_.COO.val);
    
    free_ocl(&this->mat_.ELL.val);
    free_ocl(&this->mat_.ELL.col);
    
    this->ell_nnz_ = 0;
    this->coo_nnz_ = 0;
    this->mat_.ELL.max_row = 0;
    
    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixHYB<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixHYB<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to OCL copy
  if ((cast_mat = dynamic_cast<const HostMatrixHYB<ValueType>*> (&src)) != NULL) {
    
    if (this->get_nnz() == 0)
      this->AllocateHYB(cast_mat->get_ell_nnz(), cast_mat->get_coo_nnz(), cast_mat->get_ell_max_row(),
                        cast_mat->get_nrow(), cast_mat->get_ncol());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_ell_nnz() > 0) {

      // ELL
      ocl_host2dev<int>(this->get_ell_nnz(), // size
                        cast_mat->mat_.ELL.col, // src
                        this->mat_.ELL.col,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_host2dev<ValueType>(this->get_ell_nnz(), // size
                              cast_mat->mat_.ELL.val, // src
                              this->mat_.ELL.val,     // dst
                              OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }

    if (this->get_coo_nnz() > 0) {

      // COO
      ocl_host2dev<int>(this->get_coo_nnz(), // size
                        cast_mat->mat_.COO.row, // src
                        this->mat_.COO.row,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_host2dev<int>(this->get_coo_nnz(), // size
                        cast_mat->mat_.COO.col, // src
                        this->mat_.COO.col,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_host2dev<ValueType>(this->get_coo_nnz(), // size
                              cast_mat->mat_.COO.val, // src
                              this->mat_.COO.val,     // dst
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
void OCLAcceleratorMatrixHYB<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixHYB<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixHYB<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

    if (dst->get_nnz() == 0)
      cast_mat->AllocateHYB(this->get_ell_nnz(), this->get_coo_nnz(), this->get_ell_max_row(),
                            this->get_nrow(), this->get_ncol());

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_ell_nnz() > 0) {

      // ELL
      ocl_dev2host<int>(this->get_ell_nnz(), // size
                        this->mat_.ELL.col,     // src
                        cast_mat->mat_.ELL.col, // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2host<ValueType>(this->get_ell_nnz(), // size
                              this->mat_.ELL.val,     // src
                              cast_mat->mat_.ELL.val, // dst
                              OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }


    if (this->get_coo_nnz() > 0) {

      // COO
      ocl_dev2host<int>(this->get_coo_nnz(), // size
                        this->mat_.COO.row,     // src
                        cast_mat->mat_.COO.row, // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2host<int>(this->get_coo_nnz(), // size
                        this->mat_.COO.col,     // src
                        cast_mat->mat_.COO.col, // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2host<ValueType>(this->get_coo_nnz(), // size
                              this->mat_.COO.val,     // src
                              cast_mat->mat_.COO.val, // dst
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
void OCLAcceleratorMatrixHYB<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const OCLAcceleratorMatrixHYB<ValueType> *ocl_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<const OCLAcceleratorMatrixHYB<ValueType>*> (&src)) != NULL) {
    
    if (this->get_nnz() == 0)
      this->AllocateHYB(ocl_cast_mat->get_ell_nnz(), ocl_cast_mat->get_coo_nnz(), ocl_cast_mat->get_ell_max_row(),
                        ocl_cast_mat->get_nrow(), ocl_cast_mat->get_ncol());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );


    if (this->get_ell_nnz() > 0) {

      // ELL
      // must be within same opencl context
      ocl_dev2dev<int>(this->get_ell_nnz(), // size
                       ocl_cast_mat->mat_.ELL.col, // src
                       this->mat_.ELL.col,         // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<ValueType>(this->get_ell_nnz(), // size
                             ocl_cast_mat->mat_.ELL.val, // src
                             this->mat_.ELL.val,         // dst
                             OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }

    if (this->get_coo_nnz() > 0) {

      // COO
      // must be within same opencl context
      ocl_dev2dev<int>(this->get_coo_nnz(), // size
                       ocl_cast_mat->mat_.COO.row, // src
                       this->mat_.COO.row,         // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<int>(this->get_coo_nnz(), // size
                       ocl_cast_mat->mat_.COO.col, // src
                       this->mat_.COO.col,         // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<ValueType>(this->get_coo_nnz(), // size
                             ocl_cast_mat->mat_.COO.val, // src
                             this->mat_.COO.val,         // dst
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
void OCLAcceleratorMatrixHYB<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  OCLAcceleratorMatrixHYB<ValueType> *ocl_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<OCLAcceleratorMatrixHYB<ValueType>*> (dst)) != NULL) {

    ocl_cast_mat->set_backend(this->local_backend_);       

    if (this->get_nnz() == 0)
      ocl_cast_mat->AllocateHYB(this->get_ell_nnz(), this->get_coo_nnz(), this->get_ell_max_row(),
                                this->get_nrow(), this->get_ncol());

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_ell_nnz() > 0) {

      // ELL
      // must be within same opencl context
      ocl_dev2dev<int>(this->get_ell_nnz(), // size
                       this->mat_.ELL.col,         // src
                       ocl_cast_mat->mat_.ELL.col, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<ValueType>(this->get_ell_nnz(), // size
                             this->mat_.ELL.val,         // src
                             ocl_cast_mat->mat_.ELL.val, // dst
                             OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

    }

    if (this->get_coo_nnz() > 0) {

      // COO
      // must be within same opencl context
      ocl_dev2dev<int>(this->get_coo_nnz(), // size
                       this->mat_.COO.row,         // src
                       ocl_cast_mat->mat_.COO.row, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<int>(this->get_coo_nnz(), // size
                       this->mat_.COO.col,         // src
                       ocl_cast_mat->mat_.COO.col, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<ValueType>(this->get_coo_nnz(), // size
                             this->mat_.COO.val,         // src
                             ocl_cast_mat->mat_.COO.val, // dst
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
bool OCLAcceleratorMatrixHYB<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const OCLAcceleratorMatrixHYB<ValueType>   *cast_mat_hyb;
  
  if ((cast_mat_hyb = dynamic_cast<const OCLAcceleratorMatrixHYB<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_hyb);
      return true;

  }

  /*
  const OCLAcceleratorMatrixCSR<ValueType>   *cast_mat_csr;
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
void OCLAcceleratorMatrixHYB<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {
    
    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const OCLAcceleratorVector<ValueType> *cast_in = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&in) ; 
    OCLAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      OCLAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    // ELL
    if (this->get_ell_nnz() > 0) {

      int nrow = this->get_nrow();
      int ncol = this->get_ncol();
      int max_row = this->get_ell_max_row();

      cl_int    err;
      cl_event  ocl_event;
      size_t    localWorkSize[1];
      size_t    globalWorkSize[1];

      err  = clSetKernelArg( CL_KERNEL_ELL_SPMV, 0, sizeof(int),    (void *) &nrow );
      err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 1, sizeof(int),    (void *) &ncol );
      err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 2, sizeof(int),    (void *) &max_row );
      err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 3, sizeof(cl_mem), (void *) this->mat_.ELL.col );
      err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 4, sizeof(cl_mem), (void *) this->mat_.ELL.val );
      err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 5, sizeof(cl_mem), (void *) cast_in->vec_ );
      err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 6, sizeof(cl_mem), (void *) cast_out->vec_ );
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

      localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
      globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

      err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                    CL_KERNEL_ELL_SPMV,
                                    1,
                                    NULL,
                                    &globalWorkSize[0],
                                    &localWorkSize[0],
                                    0,
                                    NULL,
                                    &ocl_event);
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

      err = clWaitForEvents( 1, &ocl_event );
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

      err = clReleaseEvent( ocl_event );
      CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    }

    // COO
    if (this->get_coo_nnz() > 0) {

      // do not support super small matrices
      assert(this->get_coo_nnz() > this->local_backend_.OCL_warp_size); 
      
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

      const int BLOCK_SIZE = int(this->local_backend_.OCL_max_work_group_size);
      //    const unsigned int MAX_BLOCKS = this->local_backend_.GPU_max_blocks;
      
      const unsigned int MAX_BLOCKS = 32; //  cusp::detail::device::arch::max_active_blocks(spmv_coo_flat_kernel<IndexType, ValueType, BLOCK_SIZE, UseCache>, BLOCK_SIZE, (size_t) 0);
      
      const unsigned int WARPS_PER_BLOCK = BLOCK_SIZE / this->local_backend_.OCL_warp_size;
      
      
      const unsigned int num_units  = this->get_coo_nnz() / this->local_backend_.OCL_warp_size; 
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

      cl_int    err;
      cl_event  ocl_event;
      size_t    localWorkSize[1];
      size_t    globalWorkSize[1];

      ValueType scalar = 1.0;

      // Set arguments for kernel call
      err  = clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 0, sizeof(int),       (void *) &tail );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 1, sizeof(int),       (void *) &interval_size );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 2, sizeof(cl_mem),    (void *) this->mat_.COO.row );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 3, sizeof(cl_mem),    (void *) this->mat_.COO.col );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_FLAT, 4, sizeof(cl_mem),    (void *) this->mat_.COO.val );
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

      int nnz = this->get_coo_nnz();

      // Set arguments for kernel call
      err  = clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 0, sizeof(int),       (void *) &nnz );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 1, sizeof(cl_mem),    (void *) this->mat_.COO.row );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 2, sizeof(cl_mem),    (void *) this->mat_.COO.col );
      err |= clSetKernelArg( CL_KERNEL_COO_SPMV_SERIAL, 3, sizeof(cl_mem),    (void *) this->mat_.COO.val );
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

}


template <typename ValueType>
void OCLAcceleratorMatrixHYB<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                  BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {
/*
    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const OCLAcceleratorVector<ValueType> *cast_in = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&in) ; 
    OCLAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      OCLAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);
*/
    LOG_INFO("OCLAcceleratorMatrixHYB::ApplyAdd NYI");
    FATAL_ERROR( __FILE__, __LINE__ );

  }

}


template class OCLAcceleratorMatrixHYB<double>;
template class OCLAcceleratorMatrixHYB<float>;

}
