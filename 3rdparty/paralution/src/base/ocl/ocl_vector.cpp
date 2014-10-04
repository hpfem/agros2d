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


#include "ocl_vector.hpp"
#include "../base_vector.hpp"
#include "../host/host_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../../utils/math_functions.hpp"
#include "ocl_utils.hpp"
#include "ocl_allocate_free.hpp"

#include <assert.h>
#include <cmath>


namespace paralution {


template <typename ValueType>
OCLAcceleratorVector<ValueType>::OCLAcceleratorVector() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
OCLAcceleratorVector<ValueType>::OCLAcceleratorVector(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "OCLAcceleratorVector::OCLAcceleratorVector()",
            "constructor with local_backend");

  this->vec_ = NULL;
  this->set_backend(local_backend);

}


template <typename ValueType>
OCLAcceleratorVector<ValueType>::~OCLAcceleratorVector() {

  LOG_DEBUG(this, "OCLAcceleratorVector::~OCLAcceleratorVector()",
            "destructor");

  this->Clear();

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::info(void) const {

  LOG_INFO("OCLAcceleratorVector<ValueType>");

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::Allocate(const int n) {

  assert(n >= 0);

  if (this->get_size() >0)
    this->Clear();

  if (n > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    allocate_ocl<ValueType>(n, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &this->vec_);

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( n / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          n,
                          ValueType(0),
                          this->vec_);

    this->size_ = n;

  }

}

template <typename ValueType>
void OCLAcceleratorVector<ValueType>::SetDataPtr(ValueType **ptr, const int size) {

  assert(*ptr != NULL);
  assert(size > 0);

  LOG_INFO("OCLAcceleratorVector::SetDataPtr NYI");
  FATAL_ERROR( __FILE__, __LINE__ );

//  TODO fix for cl_mem object
//  this->vec_ = *ptr;
//  this->size_ = size;

}

template <typename ValueType>
void OCLAcceleratorVector<ValueType>::LeaveDataPtr(ValueType **ptr) {

  assert(this->get_size() > 0);

  LOG_INFO("OCLAcceleratorVector::LeaveDataPtr NYI");
  FATAL_ERROR( __FILE__, __LINE__ );

//  TODO fix for cl_mem object
//  *ptr = this->vec_;
//  this->vec_ = NULL;

//  this->size_ = 0 ;

}

template <typename ValueType>
void OCLAcceleratorVector<ValueType>::Clear(void) {
  
  if (this->get_size() > 0) {

    free_ocl(&this->vec_);

    this->size_ = 0 ;

  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::CopyFromHost(const HostVector<ValueType> &src) {

  // CPU to OCL copy
  const HostVector<ValueType> *cast_vec;
  if ((cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src)) != NULL) {

    if (this->get_size() == 0)
      this->Allocate(cast_vec->get_size());
    
    assert(cast_vec->get_size() == this->get_size());

    if (this->get_size() >0) {

      ocl_host2dev<ValueType>(this->get_size(),
                              cast_vec->vec_, // src
                              this->vec_, // dst
                              OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);

    }

  } else {
    
    LOG_INFO("Error unsupported OpenCL vector type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::CopyToHost(HostVector<ValueType> *dst) const {

  // OCL to CPU copy
  HostVector<ValueType> *cast_vec;
  if ((cast_vec = dynamic_cast<HostVector<ValueType>*> (dst)) != NULL) {

  if (cast_vec->get_size() == 0)
    cast_vec->Allocate(this->get_size());  
    
    assert(cast_vec->get_size() == this->get_size());

    if (this->get_size() >0) {

      ocl_dev2host<ValueType>(this->get_size(),
                              this->vec_, // src
                              cast_vec->vec_, // dst
                              OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );
    }

  } else {

    LOG_INFO("Error unsupported OCL vector type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);

  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::CopyFrom(const BaseVector<ValueType> &src) {

  const OCLAcceleratorVector<ValueType> *ocl_cast_vec;
  const HostVector<ValueType> *host_cast_vec;


  // OCL to OCL copy
  if ((ocl_cast_vec = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&src)) != NULL) {

    if (this->get_size() == 0)
      this->Allocate(src.get_size());

    assert(ocl_cast_vec->get_size() == this->get_size());

    if (this != ocl_cast_vec)  {  

      if (this->get_size() >0) {

        // must be within same opencl context
        ocl_dev2dev<ValueType>(this->get_size(), // size
                               ocl_cast_vec->vec_, // src
                               this->vec_,         // dst
                               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );

      }

    }

  } else {
      
    //OCL to CPU copy
    if ((host_cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src)) != NULL) {
        
      this->CopyFromHost(*host_cast_vec);
        
    } else {

      LOG_INFO("Error unsupported OpenCL vector type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
        
    }
      
  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::CopyFrom(const BaseVector<ValueType> &src,
                                               const int src_offset,
                                               const int dst_offset,
                                               const int size) {

  assert(&src != this);
  assert(this->get_size() > 0);
  assert(src.  get_size() > 0);
  assert(size > 0);

  assert(src_offset + size <= src.get_size());
  assert(dst_offset + size <= this->get_size());

  const OCLAcceleratorVector<ValueType> *cast_src = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&src);
  assert(cast_src != NULL);

  cl_int    err;
  cl_event  ocl_event;
  size_t    localWorkSize[1];
  size_t    globalWorkSize[1];

  err  = clSetKernelArg( CL_KERNEL_COPY_OFFSET_FROM, 0, sizeof(int),    (void *) &size );
  err |= clSetKernelArg( CL_KERNEL_COPY_OFFSET_FROM, 1, sizeof(int),    (void *) &src_offset );
  err |= clSetKernelArg( CL_KERNEL_COPY_OFFSET_FROM, 2, sizeof(int),    (void *) &dst_offset );
  err |= clSetKernelArg( CL_KERNEL_COPY_OFFSET_FROM, 3, sizeof(cl_mem), (void *) cast_src->vec_ );
  err |= clSetKernelArg( CL_KERNEL_COPY_OFFSET_FROM, 4, sizeof(cl_mem), (void *) this->vec_ );
  CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
  globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

  err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                CL_KERNEL_COPY_OFFSET_FROM,
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


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::CopyTo(BaseVector<ValueType> *dst) const{

  OCLAcceleratorVector<ValueType> *ocl_cast_vec;
  HostVector<ValueType> *host_cast_vec;

  // OCL to OCL copy
  if ((ocl_cast_vec = dynamic_cast<OCLAcceleratorVector<ValueType>*> (dst)) != NULL) {

    if (this != ocl_cast_vec)  {  

      if (dst->get_size() == 0)
        dst->Allocate(this->get_size());
      
      assert(ocl_cast_vec->get_size() == this->get_size());

      if (this->get_size() > 0) {

        // must be within same opencl context
        ocl_dev2dev<ValueType>(this->get_size(), // size
                               this->vec_,         // src
                               ocl_cast_vec->vec_, // dst
                               OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue );
      }

    }

  } else {
    
    //OCL to CPU copy
    if ((host_cast_vec = dynamic_cast<HostVector<ValueType>*> (dst)) != NULL) {
      

      this->CopyToHost(host_cast_vec);
      
    
    } else {

      LOG_INFO("Error unsupported OpenCL vector type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::Zeros(void) {

  if (this->get_size() > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( this->get_size() / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          this->get_size(),
                          ValueType(0),
                          this->vec_);

  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::Ones(void) {

  if (this->get_size() > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( this->get_size() / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          this->get_size(),
                          ValueType(1),
                          this->vec_);

  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::SetValues(const ValueType val) {

  if (this->get_size() > 0) {

    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( this->get_size() / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    ocl_set_to<ValueType>(CL_KERNEL_SET_TO,
                          OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                          localWorkSize[0],
                          globalWorkSize[0],
                          this->get_size(),
                          val,
                          this->vec_);

  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::AddScale(const BaseVector<ValueType> &x, const ValueType alpha) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);

    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_AXPY, 0, sizeof(int),       (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_AXPY, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_AXPY, 2, sizeof(cl_mem),    (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_AXPY, 3, sizeof(cl_mem),    (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_AXPY,
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
void OCLAcceleratorVector<ValueType>::ScaleAdd(const ValueType alpha, const BaseVector<ValueType> &x) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);
    
    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_SCALEADD, 0, sizeof(int),       (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD, 2, sizeof(cl_mem),    (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD, 3, sizeof(cl_mem),    (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_SCALEADD,
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
void OCLAcceleratorVector<ValueType>::ScaleAddScale(const ValueType alpha,
                                                    const BaseVector<ValueType> &x,
                                                    const ValueType beta) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);

    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_SCALEADDSCALE, 0, sizeof(int),       (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE, 2, sizeof(ValueType), (void *) &beta );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE, 3, sizeof(cl_mem),    (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE, 4, sizeof(cl_mem),    (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_SCALEADDSCALE,
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
void OCLAcceleratorVector<ValueType>::ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta,
                                          const int src_offset, const int dst_offset,const int size) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 0, sizeof(int),       (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 1, sizeof(int),       (void *) &src_offset );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 2, sizeof(int),       (void *) &dst_offset );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 3, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 4, sizeof(ValueType), (void *) &beta );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 5, sizeof(cl_mem),    (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_SCALEADDSCALE_OFFSET, 6, sizeof(cl_mem),    (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_SCALEADDSCALE_OFFSET,
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
void OCLAcceleratorVector<ValueType>::ScaleAdd2(const ValueType alpha, const BaseVector<ValueType> &x,
                                                const ValueType beta,  const BaseVector<ValueType> &y,
                                                const ValueType gamma) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    assert(this->get_size() == y.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    const OCLAcceleratorVector<ValueType> *cast_y = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&y);
    assert(cast_x != NULL);
    assert(cast_y != NULL);

    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_SCALEADD2, 0, sizeof(int),       (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD2, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD2, 2, sizeof(ValueType), (void *) &beta );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD2, 3, sizeof(ValueType), (void *) &gamma );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD2, 4, sizeof(cl_mem),    (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD2, 5, sizeof(cl_mem),    (void *) cast_y->vec_ );
    err |= clSetKernelArg( CL_KERNEL_SCALEADD2, 6, sizeof(cl_mem),    (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_SCALEADD2,
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
void OCLAcceleratorVector<ValueType>::Scale(const ValueType alpha) {

  if (this->get_size() > 0) {

    int       size = this->get_size();
    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_SCALE, 0, sizeof(int),       (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_SCALE, 1, sizeof(ValueType), (void *) &alpha );
    err |= clSetKernelArg( CL_KERNEL_SCALE, 2, sizeof(cl_mem),    (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

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

    err = clWaitForEvents( 1, &ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clReleaseEvent( ocl_event );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

  }

}


template <typename ValueType>
void OCLAcceleratorVector<ValueType>::PartialSum(const BaseVector<ValueType> &x) {

  LOG_INFO("OCLAcceleratorVector::PartialSum() NYI");
  FATAL_ERROR(__FILE__, __LINE__); 

}


template <typename ValueType>
ValueType OCLAcceleratorVector<ValueType>::Dot(const BaseVector<ValueType> &x) const {

  assert(this->get_size() == x.get_size());

  const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
  assert(cast_x != NULL);

  ValueType res = ValueType(0);

  if (this->get_size() > 0) {

    cl_int    err;
    cl_event  ocl_event;
    cl_mem    *deviceBuffer;
    int       size = this->get_size();
    int       FinalReduceSize;
    int       GROUP_SIZE;
    int       LOCAL_SIZE;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];
    ValueType *hostBuffer;

    deviceBuffer = NULL;
    hostBuffer   = NULL;

    localWorkSize[0] = this->local_backend_.OCL_max_work_group_size;
    GROUP_SIZE = ( ( ( ( size / ( int(this->local_backend_.OCL_computeUnits) * 4 ) ) + 1 )
		       / int(localWorkSize[0]) ) + 1 ) * int(localWorkSize[0]);
    LOCAL_SIZE = GROUP_SIZE / int(localWorkSize[0]);
    globalWorkSize[0] = this->local_backend_.OCL_computeUnits * 4 * localWorkSize[0];

    allocate_ocl<ValueType>(int(this->local_backend_.OCL_computeUnits) * 4, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &deviceBuffer);

    err  = clSetKernelArg( CL_KERNEL_DOT, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_DOT, 1, sizeof(cl_mem), (void *) this->vec_ );
    err |= clSetKernelArg( CL_KERNEL_DOT, 2, sizeof(cl_mem), (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_DOT, 3, sizeof(cl_mem), (void *) deviceBuffer );
    err |= clSetKernelArg( CL_KERNEL_DOT, 4, sizeof(ValueType) * localWorkSize[0], NULL );
    err |= clSetKernelArg( CL_KERNEL_DOT, 5, sizeof(int),    (void *) &GROUP_SIZE );
    err |= clSetKernelArg( CL_KERNEL_DOT, 6, sizeof(int),    (void *) &LOCAL_SIZE );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_DOT,
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
    allocate_host(FinalReduceSize, &hostBuffer);

    ocl_dev2host<ValueType>(FinalReduceSize, deviceBuffer, hostBuffer, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
    free_ocl(&deviceBuffer);

    for ( int i=0; i<FinalReduceSize; ++i ) {
      res += hostBuffer[i];
    }

    free_host(&hostBuffer);

  }

  return res;

}


template <>
double OCLAcceleratorVector<double>::Norm(void) const {

  double res = 0.0;

  if (this->get_size() > 0) {

    const OCLAcceleratorVector<double> *cast_in = dynamic_cast<const OCLAcceleratorVector<double>*> (this);

    res = sqrt(double(this->Dot(*cast_in)));

  }

  return res;

}

template <>
float OCLAcceleratorVector<float>::Norm(void) const {

  float res = 0.0;

  if (this->get_size() > 0) {

    const OCLAcceleratorVector<float> *cast_in = dynamic_cast<const OCLAcceleratorVector<float>*> (this);

    res = sqrt(float(this->Dot(*cast_in)));

  }

  return res;

}

template <>
int OCLAcceleratorVector<int>::Norm(void) const {

  LOG_INFO("What is int OCLAcceleratorVector<ValueType>::Norm(void) const?");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
ValueType OCLAcceleratorVector<ValueType>::Reduce(void) const {

  ValueType res = ValueType(0);

  if (this->get_size() > 0) {

    cl_int    err;
    cl_event  ocl_event;
    cl_mem    *deviceBuffer = NULL;
    int       size = this->get_size();
    int       FinalReduceSize;
    int       GROUP_SIZE;
    int       LOCAL_SIZE;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];
    ValueType *hostBuffer = NULL;

    localWorkSize[0] = this->local_backend_.OCL_max_work_group_size;
    GROUP_SIZE = ( ( ( ( size / ( int(this->local_backend_.OCL_computeUnits) * 4 ) ) + 1 ) / int(localWorkSize[0]) ) + 1 ) * int(localWorkSize[0]);
    LOCAL_SIZE = GROUP_SIZE / int(localWorkSize[0]);
    globalWorkSize[0] = this->local_backend_.OCL_computeUnits * 4 * localWorkSize[0];

    allocate_ocl<ValueType>(int(this->local_backend_.OCL_computeUnits) * 4, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &deviceBuffer);

    err  = clSetKernelArg( CL_KERNEL_REDUCE, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_REDUCE, 1, sizeof(cl_mem), (void *) this->vec_ );
    err |= clSetKernelArg( CL_KERNEL_REDUCE, 2, sizeof(cl_mem), (void *) deviceBuffer );
    err |= clSetKernelArg( CL_KERNEL_REDUCE, 3, sizeof(ValueType) * localWorkSize[0], NULL );
    err |= clSetKernelArg( CL_KERNEL_REDUCE, 4, sizeof(int),    (void *) &GROUP_SIZE );
    err |= clSetKernelArg( CL_KERNEL_REDUCE, 5, sizeof(int),    (void *) &LOCAL_SIZE );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_REDUCE,
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
    allocate_host(FinalReduceSize, &hostBuffer);

    ocl_dev2host<ValueType>(FinalReduceSize, deviceBuffer, hostBuffer, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
    free_ocl(&deviceBuffer);

    for ( int i=0; i<FinalReduceSize; ++i ) {
      res += hostBuffer[i];
    }

    free_host(&hostBuffer);

  }

  return res;

}

template <typename ValueType>
ValueType OCLAcceleratorVector<ValueType>::Asum(void) const {

  ValueType res = ValueType(0);

  if (this->get_size() > 0) {

    cl_int    err;
    cl_event  ocl_event;
    cl_mem    *deviceBuffer = NULL;
    int       size = this->get_size();
    int       FinalReduceSize;
    int       GROUP_SIZE;
    int       LOCAL_SIZE;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];
    ValueType *hostBuffer = NULL;

    localWorkSize[0] = this->local_backend_.OCL_max_work_group_size;
    GROUP_SIZE = ( ( ( ( size / ( int(this->local_backend_.OCL_computeUnits) * 4 ) ) + 1 ) / int(localWorkSize[0]) ) + 1 ) * int(localWorkSize[0]);
    LOCAL_SIZE = GROUP_SIZE / int(localWorkSize[0]);
    globalWorkSize[0] = this->local_backend_.OCL_computeUnits * 4 * localWorkSize[0];

    allocate_ocl<ValueType>(int(this->local_backend_.OCL_computeUnits) * 4, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &deviceBuffer);

    err  = clSetKernelArg( CL_KERNEL_ASUM, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_ASUM, 1, sizeof(cl_mem), (void *) this->vec_ );
    err |= clSetKernelArg( CL_KERNEL_ASUM, 2, sizeof(cl_mem), (void *) deviceBuffer );
    err |= clSetKernelArg( CL_KERNEL_ASUM, 3, sizeof(ValueType) * localWorkSize[0], NULL );
    err |= clSetKernelArg( CL_KERNEL_ASUM, 4, sizeof(int),    (void *) &GROUP_SIZE );
    err |= clSetKernelArg( CL_KERNEL_ASUM, 5, sizeof(int),    (void *) &LOCAL_SIZE );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_ASUM,
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
    allocate_host(FinalReduceSize, &hostBuffer);

    ocl_dev2host<ValueType>(FinalReduceSize, deviceBuffer, hostBuffer, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
    free_ocl(&deviceBuffer);

    for ( int i=0; i<FinalReduceSize; ++i ) {
      res += paralution_abs(hostBuffer[i]);
    }

    free_host(&hostBuffer);

  }

  return res;

}

template <typename ValueType>
int OCLAcceleratorVector<ValueType>::Amax(ValueType &value) const {

  ValueType res = ValueType(0);
  int idx = 0;

  if (this->get_size() > 0) {

    cl_int    err;
    cl_event  ocl_event;
    cl_mem    *deviceBuffer = NULL;
    cl_mem    *iDeviceBuffer = NULL;
    int       size = this->get_size();
    int       FinalReduceSize;
    int       GROUP_SIZE;
    int       LOCAL_SIZE;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];
    ValueType *hostBuffer = NULL;
    int       *iHostBuffer = NULL;

    localWorkSize[0] = this->local_backend_.OCL_max_work_group_size;
    GROUP_SIZE = ( ( ( ( size / ( int(this->local_backend_.OCL_computeUnits) * 4 ) ) + 1 ) / int(localWorkSize[0]) ) + 1 ) * int(localWorkSize[0]);
    LOCAL_SIZE = GROUP_SIZE / int(localWorkSize[0]);
    globalWorkSize[0] = this->local_backend_.OCL_computeUnits * 4 * localWorkSize[0];

    allocate_ocl<ValueType>(int(this->local_backend_.OCL_computeUnits) * 4, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &deviceBuffer);
    allocate_ocl<int>(int(this->local_backend_.OCL_computeUnits) * 4, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_context, &iDeviceBuffer);

    err  = clSetKernelArg( CL_KERNEL_AMAX, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_AMAX, 1, sizeof(cl_mem), (void *) this->vec_ );
    err |= clSetKernelArg( CL_KERNEL_AMAX, 2, sizeof(cl_mem), (void *) deviceBuffer );
    err |= clSetKernelArg( CL_KERNEL_AMAX, 3, sizeof(cl_mem), (void *) iDeviceBuffer );
    err |= clSetKernelArg( CL_KERNEL_AMAX, 4, sizeof(ValueType) * localWorkSize[0], NULL );
    err |= clSetKernelArg( CL_KERNEL_AMAX, 5, sizeof(int) * localWorkSize[0], NULL);
    err |= clSetKernelArg( CL_KERNEL_AMAX, 6, sizeof(int),    (void *) &GROUP_SIZE );
    err |= clSetKernelArg( CL_KERNEL_AMAX, 7, sizeof(int),    (void *) &LOCAL_SIZE );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_AMAX,
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
    allocate_host(FinalReduceSize, &hostBuffer);
    allocate_host(FinalReduceSize, &iHostBuffer);

    ocl_dev2host<ValueType>(FinalReduceSize, deviceBuffer, hostBuffer, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
    free_ocl(&deviceBuffer);
    ocl_dev2host<int>(FinalReduceSize, iDeviceBuffer, iHostBuffer, OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue);
    free_ocl(&iDeviceBuffer);

    for (int i=0; i<FinalReduceSize; ++i) {
      ValueType tmp = paralution_abs(hostBuffer[i]);
      if (res < tmp) {
        res = tmp;
        idx = iHostBuffer[i];
      }
    }

    free_host(&hostBuffer);
    free_host(&iHostBuffer);

  }

  value = res;

  return idx;

}

template <typename ValueType>
void OCLAcceleratorVector<ValueType>::PointWiseMult(const BaseVector<ValueType> &x) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    assert(cast_x != NULL);
    
    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_POINTWISEMULT, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_POINTWISEMULT, 1, sizeof(cl_mem), (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_POINTWISEMULT, 2, sizeof(cl_mem), (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_POINTWISEMULT,
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
void OCLAcceleratorVector<ValueType>::PointWiseMult(const BaseVector<ValueType> &x, const BaseVector<ValueType> &y) {

  if (this->get_size() > 0) {

    assert(this->get_size() == x.get_size());
    assert(this->get_size() == y.get_size());
    
    const OCLAcceleratorVector<ValueType> *cast_x = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&x);
    const OCLAcceleratorVector<ValueType> *cast_y = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&y);
    assert(cast_x != NULL);
    assert(cast_y != NULL);
    
    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_POINTWISEMULT2, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_POINTWISEMULT2, 1, sizeof(cl_mem), (void *) cast_x->vec_ );
    err |= clSetKernelArg( CL_KERNEL_POINTWISEMULT2, 2, sizeof(cl_mem), (void *) cast_y->vec_ );
    err |= clSetKernelArg( CL_KERNEL_POINTWISEMULT2, 3, sizeof(cl_mem), (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_POINTWISEMULT2,
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
void OCLAcceleratorVector<ValueType>::Permute(const BaseVector<int> &permutation) {

  if (this->get_size() > 0) {

    assert(&permutation != NULL);
    assert(this->get_size() == permutation.get_size());
    
    const OCLAcceleratorVector<int> *cast_perm = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);
    
    OCLAcceleratorVector<ValueType> vec_tmp(this->local_backend_);     
    vec_tmp.Allocate(this->get_size());
    vec_tmp.CopyFrom(*this);

    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_PERMUTE, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE, 1, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE, 2, sizeof(cl_mem), (void *) vec_tmp.vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE, 3, sizeof(cl_mem), (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_PERMUTE,
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
void OCLAcceleratorVector<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {

  if (this->get_size() > 0) {

    assert(&permutation != NULL);
    assert(this->get_size() == permutation.get_size());
    
    const OCLAcceleratorVector<int> *cast_perm = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation);
    assert(cast_perm != NULL);
    
    OCLAcceleratorVector<ValueType> vec_tmp(this->local_backend_);   
    vec_tmp.Allocate(this->get_size());
    vec_tmp.CopyFrom(*this);
    
    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 1, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 2, sizeof(cl_mem), (void *) vec_tmp.vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 3, sizeof(cl_mem), (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_PERMUTE_BACKWARD,
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
void OCLAcceleratorVector<ValueType>::CopyFromPermute(const BaseVector<ValueType> &src,
                                                      const BaseVector<int> &permutation) { 

  if (this->get_size() > 0) {

    assert(this != &src);
    
    const OCLAcceleratorVector<ValueType> *cast_vec = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&src);
    const OCLAcceleratorVector<int> *cast_perm      = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation) ; 
    assert(cast_perm != NULL);
    assert(cast_vec  != NULL);
    
    assert(cast_vec ->get_size() == this->get_size());
    assert(cast_perm->get_size() == this->get_size());
    
    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    // kernel_permute
    err  = clSetKernelArg( CL_KERNEL_PERMUTE, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE, 1, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE, 2, sizeof(cl_mem), (void *) cast_vec->vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE, 3, sizeof(cl_mem), (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_PERMUTE,
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
void OCLAcceleratorVector<ValueType>::CopyFromPermuteBackward(const BaseVector<ValueType> &src,
                                                              const BaseVector<int> &permutation) {

  if (this->get_size() > 0) {

    assert(this != &src);
    
    const OCLAcceleratorVector<ValueType> *cast_vec = dynamic_cast<const OCLAcceleratorVector<ValueType>*> (&src);
    const OCLAcceleratorVector<int> *cast_perm      = dynamic_cast<const OCLAcceleratorVector<int>*> (&permutation) ; 
    assert(cast_perm != NULL);
    assert(cast_vec  != NULL);
    
    assert(cast_vec ->get_size() == this->get_size());
    assert(cast_perm->get_size() == this->get_size());
    
    
    int size = this->get_size();

    cl_int    err;
    cl_event  ocl_event;
    size_t    localWorkSize[1];
    size_t    globalWorkSize[1];

    err  = clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 0, sizeof(int),    (void *) &size );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 1, sizeof(cl_mem), (void *) cast_perm->vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 2, sizeof(cl_mem), (void *) cast_vec->vec_ );
    err |= clSetKernelArg( CL_KERNEL_PERMUTE_BACKWARD, 3, sizeof(cl_mem), (void *) this->vec_ );
    CHECK_OCL_ERROR( err, __FILE__, __LINE__ );

    localWorkSize[0]  = this->local_backend_.OCL_max_work_group_size;
    globalWorkSize[0] = ( size_t( size / localWorkSize[0] ) + 1 ) * localWorkSize[0];

    err = clEnqueueNDRangeKernel( OCL_HANDLE(this->local_backend_.OCL_handle)->OCL_cmdQueue,
                                  CL_KERNEL_PERMUTE_BACKWARD,
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


template class OCLAcceleratorVector<double>;
template class OCLAcceleratorVector<float>;
template class OCLAcceleratorVector<int>;

}
