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
#include "../host/host_matrix_mcsr.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "ocl_utils.hpp"
#include "ocl_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>

namespace paralution {


template <typename ValueType>
OCLAcceleratorMatrixMCSR<ValueType>::OCLAcceleratorMatrixMCSR() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixMCSR<ValueType>::OCLAcceleratorMatrixMCSR(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorMatrixMCSR::OCLAcceleratorMatrixMCSR()",
            "constructor with local_backend");

  this->mat_.row_offset = NULL;  
  this->mat_.col        = NULL;  
  this->mat_.val        = NULL;
  this->mat_.diag       = NULL;
  this->set_backend(local_backend); 

}


template <typename ValueType>
OCLAcceleratorMatrixMCSR<ValueType>::~OCLAcceleratorMatrixMCSR() {

  LOG_DEBUG(this, "OCLAcceleratorMatrixMCSR::~OCLAcceleratorMatrixMCSR()",
            "destructor");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorMatrixMCSR<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorMatrixMCSR<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorMatrixMCSR<ValueType>::AllocateMCSR(const int nnz, const int nrow, const int ncol) {

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

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( nrow+1 / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nrow+1,
                    0,
                    mat_.row_offset);

    globalWorkSize[0] = ( size_t( nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nnz,
                    0,
                    mat_.col);

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
void OCLAcceleratorMatrixMCSR<ValueType>::Clear() {

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
void OCLAcceleratorMatrixMCSR<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixMCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to OCL copy
  if ((cast_mat = dynamic_cast<const HostMatrixMCSR<ValueType>*> (&src)) != NULL) {
    
    if (this->get_nnz() == 0)
      this->AllocateMCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      ocl_host2dev<int>((this->get_nrow()+1), // size
                        cast_mat->mat_.row_offset, // src
                        this->mat_.row_offset,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_host2dev<int>(this->get_nnz(), // size
                        cast_mat->mat_.col, // src
                        this->mat_.col,     // dst
                        OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

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
void OCLAcceleratorMatrixMCSR<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixMCSR<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixMCSR<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

    if (dst->get_nnz() == 0)
      cast_mat->AllocateMCSR(this->get_nnz(), this->get_nrow(), this->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

        ocl_dev2host<int>((this->get_nrow()+1), // size
                          this->mat_.row_offset,     // src
                          cast_mat->mat_.row_offset, // dst
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

        ocl_dev2host<int>(this->get_nnz(), // size
                          this->mat_.col,     // src
                          cast_mat->mat_.col, // dst
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

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
void OCLAcceleratorMatrixMCSR<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const OCLAcceleratorMatrixMCSR<ValueType> *ocl_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<const OCLAcceleratorMatrixMCSR<ValueType>*> (&src)) != NULL) {
    
    if (this->get_nnz() == 0)
      this->AllocateMCSR(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

          // must be within same opencl context
          ocl_dev2dev<int>(this->get_nrow()+1, // size
                           ocl_cast_mat->mat_.row_offset, // src
                           this->mat_.row_offset,         // dst
                           OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

          ocl_dev2dev<int>(this->get_nnz(), // size
                           ocl_cast_mat->mat_.col, // src
                           this->mat_.col,         // dst
                           OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

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
void OCLAcceleratorMatrixMCSR<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  OCLAcceleratorMatrixMCSR<ValueType> *ocl_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<OCLAcceleratorMatrixMCSR<ValueType>*> (dst)) != NULL) {

    ocl_cast_mat->set_backend(this->local_backend_);       

    if (this->get_nnz() == 0)
      ocl_cast_mat->AllocateMCSR(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

      // must be within same opencl context
      ocl_dev2dev<int>(this->get_nrow()+1, // size
                       this->mat_.row_offset,         // src
                       ocl_cast_mat->mat_.row_offset, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      ocl_dev2dev<int>(this->get_nnz(), // size
                       this->mat_.col,         // src
                       ocl_cast_mat->mat_.col, // dst
                       OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

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
bool OCLAcceleratorMatrixMCSR<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const OCLAcceleratorMatrixMCSR<ValueType> *cast_mat_mcsr;
  
  if ((cast_mat_mcsr = dynamic_cast<const OCLAcceleratorMatrixMCSR<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_mcsr);
      return true;

  }

  /*
  const OCLAcceleratorMatrixCSR<ValueType>  *cast_mat_csr;
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
void OCLAcceleratorMatrixMCSR<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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

    err  = clSetKernelArg( CL_KERNEL_MCSR_SPMV_SCALAR, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_MCSR_SPMV_SCALAR, 1, sizeof(cl_mem), (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_MCSR_SPMV_SCALAR, 2, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_MCSR_SPMV_SCALAR, 3, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_MCSR_SPMV_SCALAR, 4, sizeof(cl_mem), (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_MCSR_SPMV_SCALAR, 5, sizeof(cl_mem), (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_MCSR_SPMV_SCALAR,
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

}


template <typename ValueType>
void OCLAcceleratorMatrixMCSR<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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

    err  = clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 1, sizeof(cl_mem),    (void *) this->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 2, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 3, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 4, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 5, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_MCSR_ADD_SPMV_SCALAR, 6, sizeof(cl_mem),    (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_MCSR_ADD_SPMV_SCALAR,
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

}


template class OCLAcceleratorMatrixMCSR<double>;
template class OCLAcceleratorMatrixMCSR<float>;

}
