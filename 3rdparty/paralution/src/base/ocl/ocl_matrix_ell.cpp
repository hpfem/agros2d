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
#include "../host/host_matrix_ell.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "ocl_utils.hpp"
#include "ocl_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>

namespace paralution {


template <typename ValueType>
OCLAcceleratorMatrixELL<ValueType>::OCLAcceleratorMatrixELL() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorMatrixELL<ValueType>::OCLAcceleratorMatrixELL(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorMatrixELL::OCLAcceleratorMatrixELL()",
            "constructor with local_backend");

  this->mat_.val = NULL;
  this->mat_.col = NULL;
  this->mat_.max_row = 0;
  this->set_backend(local_backend); 

}


template <typename ValueType>
OCLAcceleratorMatrixELL<ValueType>::~OCLAcceleratorMatrixELL() {

  LOG_DEBUG(this, "OCLAcceleratorMatrixELL::~OCLAcceleratorMatrixELL()",
            "destructor");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorMatrixELL<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorMatrixELL<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorMatrixELL<ValueType>::AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row) {

  assert( nnz   >= 0);
  assert( ncol  >= 0);
  assert( nrow  >= 0);
  assert( max_row >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    assert(nnz == max_row * nrow);

    allocate_ocl<int>      (nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.col);
    allocate_ocl<ValueType>(nnz, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->mat_.val);

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( nnz / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nnz,
                    0,
                    this->mat_.col);

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          nnz,
                          0.0,
                          this->mat_.val);
    
    this->mat_.max_row = max_row;
    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixELL<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_ocl(&this->mat_.val);
    free_ocl(&this->mat_.col);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}


template <typename ValueType>
void OCLAcceleratorMatrixELL<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixELL<ValueType> *cast_mat;

  // copy only in the same format
  // assert(this->get_mat_format() == src.get_mat_format());

  // CPU to OCL copy
  if ((cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateELL(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), cast_mat->get_max_row());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) { 

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
void OCLAcceleratorMatrixELL<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixELL<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixELL<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateELL(this->get_nnz(), this->get_nrow(), this->get_ncol(), this->get_max_row() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

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
void OCLAcceleratorMatrixELL<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const OCLAcceleratorMatrixELL<ValueType> *ocl_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  // assert(this->get_mat_format() == src.get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<const OCLAcceleratorMatrixELL<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateELL(ocl_cast_mat->get_nnz(), ocl_cast_mat->get_nrow(), ocl_cast_mat->get_ncol(), ocl_cast_mat->get_max_row() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

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
void OCLAcceleratorMatrixELL<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  OCLAcceleratorMatrixELL<ValueType> *ocl_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // OCL to OCL copy
  if ((ocl_cast_mat = dynamic_cast<OCLAcceleratorMatrixELL<ValueType>*> (dst)) != NULL) {

    ocl_cast_mat->set_backend(this->local_backend_);       

    if (this->get_nnz() == 0)
      ocl_cast_mat->AllocateELL(ocl_cast_mat->get_nnz(), ocl_cast_mat->get_nrow(), ocl_cast_mat->get_ncol(), ocl_cast_mat->get_max_row() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

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
bool OCLAcceleratorMatrixELL<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const OCLAcceleratorMatrixELL<ValueType> *cast_mat_ell;
  
  if ((cast_mat_ell = dynamic_cast<const OCLAcceleratorMatrixELL<ValueType>*> (&mat)) != NULL) {

    this->CopyFrom(*cast_mat_ell);
    return true;

  }

  const OCLAcceleratorMatrixCSR<ValueType> *cast_mat_csr;
  if ((cast_mat_csr = dynamic_cast<const OCLAcceleratorMatrixCSR<ValueType>*> (&mat)) != NULL) {

    this->Clear();

    assert(cast_mat_csr->get_nrow() > 0);
    assert(cast_mat_csr->get_ncol() > 0);
    assert(cast_mat_csr->get_nnz() > 0);

    int max_row = 0;
    int nrow = cast_mat_csr->get_nrow();

    cl_int    err;
    cl_event  ocl_event;
    cl_mem    *d_buffer = NULL;
    int       *h_buffer = NULL;
    int       FinalReduceSize;
    int       GROUP_SIZE;
    int       LOCAL_SIZE;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    allocate_ocl<int>(int(this->local_backend_.OCL_computeUnits) * 4,
                      OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context,
                      &d_buffer);

    localWorkSize[0] = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = this->local_backend_.OCL_computeUnits * 4 * localWorkSize[0];

    GROUP_SIZE = ( ( ( ( nrow / ( int(this->local_backend_.OCL_computeUnits) * 4 ) ) + 1 ) 
                 / int(localWorkSize[0]) ) + 1 ) * int(localWorkSize[0]);
    LOCAL_SIZE = GROUP_SIZE / int(localWorkSize[0]);

    err  = clSetKernelArg( CL_KERNEL_ELL_MAX_ROW, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_ELL_MAX_ROW, 1, sizeof(cl_mem), (void *) cast_mat_csr->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_ELL_MAX_ROW, 2, sizeof(cl_mem), (void *) d_buffer );
    err |= clSetKernelArg( CL_KERNEL_ELL_MAX_ROW, 3, sizeof(int),    (void *) &GROUP_SIZE );
    err |= clSetKernelArg( CL_KERNEL_ELL_MAX_ROW, 4, sizeof(int),    (void *) &LOCAL_SIZE );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_ELL_MAX_ROW,
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

    FinalReduceSize = int(this->local_backend_.OCL_computeUnits) * 4;
    allocate_host(FinalReduceSize, &h_buffer);

    ocl_dev2host<int>(FinalReduceSize, d_buffer, h_buffer,
                      OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

    free_ocl(&d_buffer);

    for ( int i=0; i<FinalReduceSize; ++i )
      if (max_row < h_buffer[i]) max_row = h_buffer[i];

    free_host(&h_buffer);

    int nnz_ell = max_row * nrow;

    this->AllocateELL(nnz_ell, nrow, cast_mat_csr->get_ncol(), max_row);

    globalWorkSize[0] = ( size_t( nnz_ell / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          nnz_ell,
                          0.0,
                          this->mat_.val);

    ocl_set_to<int>(CL_KERNEL_SET_TO_INT,
                    OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                    localWorkSize[0],
                    globalWorkSize[0],
                    nnz_ell,
                    0,
                    this->mat_.col);

    err  = clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 1, sizeof(int),    (void *) &max_row );
    err |= clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 2, sizeof(cl_mem), (void *) cast_mat_csr->mat_.row_offset );
    err |= clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 3, sizeof(cl_mem), (void *) cast_mat_csr->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 4, sizeof(cl_mem), (void *) cast_mat_csr->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 5, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_ELL_CSR_TO_ELL, 6, sizeof(cl_mem), (void *) this->mat_.val );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_ELL_CSR_TO_ELL,
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

    this->mat_.max_row = max_row;
    this->nrow_ = cast_mat_csr->get_nrow();
    this->ncol_ = cast_mat_csr->get_ncol();
    this->nnz_  = max_row * nrow;

    return true;

  }

  return false;

}


template <typename ValueType>
void OCLAcceleratorMatrixELL<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    int max_row = this->get_max_row();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_ELL_SPMV, 0, sizeof(int),    (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 1, sizeof(int),    (void *) &ncol );
    err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 2, sizeof(int),    (void *) &max_row );
    err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 3, sizeof(cl_mem), (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 4, sizeof(cl_mem), (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 5, sizeof(cl_mem), (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_ELL_SPMV, 6, sizeof(cl_mem), (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

	  // Nathan Bell and Michael Garland
	  // Efficient Sparse Matrix-Vector Multiplication on {CUDA}
    // NVR-2008-004 / NVIDIA Technical Report
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

}


template <typename ValueType>
void OCLAcceleratorMatrixELL<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    int ncol = this->get_ncol();
    int max_row = this->get_max_row();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 0, sizeof(int),       (void *) &nrow );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 1, sizeof(int),       (void *) &ncol );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 2, sizeof(int),       (void *) &max_row );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 3, sizeof(cl_mem),    (void *) this->mat_.col );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 4, sizeof(cl_mem),    (void *) this->mat_.val );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 5, sizeof(ValueType), (void *) &scalar );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 6, sizeof(cl_mem),    (void *) cast_in->vec_ );
    err |= clSetKernelArg( CL_KERNEL_ELL_ADD_SPMV, 7, sizeof(cl_mem),    (void *) cast_out->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( nrow / localWorkSize[0] ) + 1 ) * localWorkSize[0];

	  // Nathan Bell and Michael Garland
	  // Efficient Sparse Matrix-Vector Multiplication on {CUDA}
    // NVR-2008-004 / NVIDIA Technical Report
    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_ELL_ADD_SPMV,
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


template class OCLAcceleratorMatrixELL<double>;
template class OCLAcceleratorMatrixELL<float>;

}
