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
#include "../host/host_matrix_dense.hpp"
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
OCLAcceleratorMatrixDENSE<ValueType>::OCLAcceleratorMatrixDENSE() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixDENSE<ValueType>::OCLAcceleratorMatrixDENSE(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorMatrixDENSE::OCLAcceleratorMatrixDENSE()",
            "constructor with local_backend");

  this->mat_.val = NULL;
  this->set_backend(local_backend); 

}


template <typename ValueType>
OCLAcceleratorMatrixDENSE<ValueType>::~OCLAcceleratorMatrixDENSE() {

  LOG_DEBUG(this, "OCLAcceleratorMatrixDENSE::~OCLAcceleratorMatrixDENSE()",
            "constructor with local_backend");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorMatrixDENSE<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorMatrixDENSE<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorMatrixDENSE<ValueType>::AllocateDENSE(const int nrow, const int ncol) {

  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nrow*ncol > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    allocate_ocl<ValueType>(nrow*ncol, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.val);

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( nrow*ncol / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          nrow*ncol,
                          0.0,
                          mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nrow*ncol;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixDENSE<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_ocl(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixDENSE<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to OCL copy
  if ((cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

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
void OCLAcceleratorMatrixDENSE<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixDENSE<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateDENSE(this->get_nrow(), this->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

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
void OCLAcceleratorMatrixDENSE<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const OCLAcceleratorMatrixDENSE<ValueType> *ocl_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<const OCLAcceleratorMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

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
void OCLAcceleratorMatrixDENSE<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  OCLAcceleratorMatrixDENSE<ValueType> *ocl_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<OCLAcceleratorMatrixDENSE<ValueType>*> (dst)) != NULL) {

    ocl_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    ocl_cast_mat->AllocateDENSE(dst->get_nrow(), dst->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

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
bool OCLAcceleratorMatrixDENSE<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const OCLAcceleratorMatrixDENSE<ValueType> *cast_mat_dense;
  
  if ((cast_mat_dense = dynamic_cast<const OCLAcceleratorMatrixDENSE<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_dense);
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

    return 0;

  }
  */

  return false;

}


template <typename ValueType>
void OCLAcceleratorMatrixDENSE<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    int ncol = this->get_ncol();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // Set arguments for kernel call
    err  = clSetKernelArg( CL_KERNEL_DENSE_SPMV, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_DENSE_SPMV, 1, sizeof(int),    (void *) &ncol );
    err |= clSetKernelArg( CL_KERNEL_DENSE_SPMV, 2, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_DENSE_SPMV, 3, sizeof(cl_mem), (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_DENSE_SPMV, 4, sizeof(cl_mem), (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    // Determine local work size for kernel call
    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    localWorkSize[0] /= 2;
    // Determine global work size for kernel call
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    // Start kernel run
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_DENSE_SPMV,
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
void OCLAcceleratorMatrixDENSE<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                  BaseVector<ValueType> *out) const {

  FATAL_ERROR(__FILE__, __LINE__);

}


template class OCLAcceleratorMatrixDENSE<double>;
template class OCLAcceleratorMatrixDENSE<float>;

}
