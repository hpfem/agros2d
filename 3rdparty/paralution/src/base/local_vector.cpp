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


#include "backend_manager.hpp"
#include "local_vector.hpp"
#include "local_matrix.hpp"
#include "base_vector.hpp"
#include "backend_manager.hpp"
#include "../utils/log.hpp"

#include <assert.h>
#include <stdlib.h>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace paralution {

template <typename ValueType>
LocalVector<ValueType>::LocalVector() {

  LOG_DEBUG(this, "LocalVector::LocalVector()",
            "default constructor");

  this->object_name_ = "";
  this->local_backend_ = _Backend_Descriptor;

  // Create empty vector on the host
  this->vector_host_= new HostVector<ValueType>(this->local_backend_);
  this->vector_accel_= NULL;
  this->vector_ = this->vector_host_ ;
 
}

template <typename ValueType>
LocalVector<ValueType>::~LocalVector() {

  LOG_DEBUG(this, "LocalVector::~LocalVector()",
            "default destructor");

  this->Clear();
  delete this->vector_;

}

template <typename ValueType>
int LocalVector<ValueType>::get_size(void) const { 

  return this->vector_->get_size(); 

}


template <typename ValueType>
void LocalVector<ValueType>::Allocate(std::string name, const int size) {

  LOG_DEBUG(this, "LocalVector::Allocate()",
            "");

  this->object_name_ = name;

  if (size > 0) {

    Paralution_Backend_Descriptor backend = this->local_backend_;
    
    // init host vector
    if (this->vector_ == this->vector_host_) {
      
      delete this->vector_host_;
      
      this->vector_host_ = new HostVector<ValueType>(backend);
      assert(this->vector_host_ != NULL);    
      this->vector_host_->Allocate(size);   
      this->vector_ = this->vector_host_;
      
    } else { 
      // init accel vector
      assert(this->vector_ == this->vector_accel_);
      
      delete this->vector_accel_;
      
      this->vector_accel_ = _paralution_init_base_backend_vector<ValueType>(backend);
      assert(this->vector_accel_ != NULL);
      this->vector_accel_->Allocate(size);   
      this->vector_ = this->vector_accel_;
      
    }
  }

}

template <typename ValueType>
bool LocalVector<ValueType>::Check(void) const { 

  bool check = false;

  if (this->is_accel() == true) {
    
    LocalVector<ValueType> vec; 

    vec.CopyFrom(*this);
    
    // host check
    check = vec.Check();

  } else {
    // Host

    check = this->vector_->Check();

  }

  return check;

}

template <typename ValueType>
void LocalVector<ValueType>::Assemble(const int *i, const ValueType *v,
                                      int size, std::string name, const int n) {
  LOG_DEBUG(this, "LocalVector::Assemble()",
            "");

  assert(i != NULL);
  assert(v != NULL);
  assert(size > 0);
  assert(n >= 0);
  
  this->object_name_ = name;

  bool on_host = this->is_host();
  if (on_host == false)
    this->MoveToHost();

  this->vector_->Assemble(i, v, size, n);

  if (on_host == false)
    this->MoveToAccelerator();
}

template <typename ValueType>
void LocalVector<ValueType>::SetDataPtr(ValueType **ptr, std::string name, const int size) {

  assert(*ptr != NULL);
  assert(size > 0);

  this->Clear();

  this->object_name_ = name;

  this->vector_->SetDataPtr(ptr, size);

  *ptr = NULL;
}

template <typename ValueType>
void LocalVector<ValueType>::LeaveDataPtr(ValueType **ptr) {

  LOG_DEBUG(this, "LocalVector::LeaveDataPtr()",
            "");

  assert(this->get_size() > 0);

  this->vector_->LeaveDataPtr(ptr);

}

template <typename ValueType>
void LocalVector<ValueType>::Clear(void) {

  LOG_DEBUG(this, "LocalVector::Clear()",
            "");

  this->vector_->Clear();
  
}

template <typename ValueType>
void LocalVector<ValueType>::Zeros(void) {

  LOG_DEBUG(this, "LocalVector::Zeros()",
            "");

  if (this->get_size() > 0) {
    this->vector_->Zeros();
  }
}

template <typename ValueType>
void LocalVector<ValueType>::Ones(void) {

  LOG_DEBUG(this, "LocalVector::Ones()",
            "");

  if (this->get_size() > 0) {
    this->vector_->Ones();
  }

}

template <typename ValueType>
void LocalVector<ValueType>::SetValues(const ValueType val) {

  LOG_DEBUG(this, "LocalVector::SetValues()",
            "");


  if (this->get_size() > 0) {
    this->vector_->SetValues(val);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::SetRandom(const ValueType a, const ValueType b, const int seed) {

  LOG_DEBUG(this, "LocalVector::SetRandom()",
            "");

  if (this->get_size() > 0) {

    assert(b > a);

    // host only
    bool on_host = this->is_host();
    if (on_host == false)
      this->MoveToHost();
    
    assert(this->vector_ == this->vector_host_);
    this->vector_host_->SetRandom(a, b, seed);
    
    if (on_host == false)
      this->MoveToAccelerator();
    
  }

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFrom(const LocalVector<ValueType> &src) {

  LOG_DEBUG(this, "LocalVector::CopyFrom()",
            "");

  assert(&src != NULL);
  assert(this != &src);

  this->vector_->CopyFrom(*src.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromAsync(const LocalVector<ValueType> &src) {

  LOG_DEBUG(this, "LocalVector::CopyFromAsync()",
            "");

  assert(this->asyncf == false);
  assert(&src != NULL);
  assert(this != &src);

  this->vector_->CopyFromAsync(*src.vector_);

  this->asyncf = true;

}


template <typename ValueType>
void LocalVector<ValueType>::CopyFromFloat(const LocalVector<float> &src) {

  LOG_DEBUG(this, "LocalVector::CopyFromFloat()",
            "");

  assert(&src != NULL);
  this->vector_->CopyFromFloat(*src.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromDouble(const LocalVector<double> &src) {

  LOG_DEBUG(this, "LocalVector::CopyFromDouble()",
            "");

  assert(&src != NULL);
  this->vector_->CopyFromDouble(*src.vector_);

}


  
template <typename ValueType>
bool LocalVector<ValueType>::is_host(void) const {
  return (this->vector_ == this->vector_host_);
}

template <typename ValueType>
bool LocalVector<ValueType>::is_accel(void) const {
  return (this->vector_ == this->vector_accel_);
}


template <typename ValueType>
void LocalVector<ValueType>::CloneFrom(const LocalVector<ValueType> &src) {

  LOG_DEBUG(this, "LocalVector::CloneFrom()",
            "");


  assert(&src != NULL);
  assert(this != &src);

  this->CloneBackend(src);
  this->CopyFrom(src);

}

template <typename ValueType>
void LocalVector<ValueType>::MoveToAccelerator(void) {

  LOG_DEBUG(this, "LocalVector::MoveToAccelerator()",
            "");

  if (_paralution_available_accelerator() == false)
    LOG_VERBOSE_INFO(4,"*** info: LocalVector::MoveToAccelerator() no accelerator available - doing nothing");  

  if ( (_paralution_available_accelerator() == true) && 
       (this->vector_ == this->vector_host_)) {
    
    this->vector_accel_ = _paralution_init_base_backend_vector<ValueType>(this->local_backend_);

    // Copy to accel
    this->vector_accel_->CopyFrom(*this->vector_host_);
    
    this->vector_ = this->vector_accel_;
    delete this->vector_host_;
    this->vector_host_ = NULL;
        
    LOG_VERBOSE_INFO(4, "*** info: LocalVector::MoveToAccelerator() host to accelerator transfer");        
  }

}

template <typename ValueType>
void LocalVector<ValueType>::MoveToHost(void) {

  LOG_DEBUG(this, "LocalVector::MoveToHost()",
            "");

  if (_paralution_available_accelerator() == false)
    LOG_VERBOSE_INFO(4,"*** info: LocalVector::MoveToHost() no accelerator available - doing nothing");  

  if ( (_paralution_available_accelerator() == true) && 
       (this->vector_ == this->vector_accel_)) {
    
    this->vector_host_ = new HostVector<ValueType>(this->local_backend_); 

    // Copy to host
    this->vector_host_->CopyFrom(*this->vector_accel_);

    this->vector_ = this->vector_host_;
    delete this->vector_accel_;
    this->vector_accel_ = NULL;
       
    LOG_VERBOSE_INFO(4, "*** info: LocalVector::MoveToHost() accelerator to host transfer");        
  }
 
}

template <typename ValueType>
void LocalVector<ValueType>::MoveToAcceleratorAsync(void) {

  LOG_DEBUG(this, "LocalVector::MoveToAcceleratorAsync()",
            "");

  assert(this->asyncf == false);

  if (_paralution_available_accelerator() == false)
    LOG_VERBOSE_INFO(4,"*** info: LocalVector::MoveToAcceleratorAsync() no accelerator available - doing nothing");  

  if ( (_paralution_available_accelerator() == true) && 
       (this->vector_ == this->vector_host_)) {
    
    this->vector_accel_ = _paralution_init_base_backend_vector<ValueType>(this->local_backend_);

    // Copy to accel
    this->vector_accel_->CopyFromAsync(*this->vector_host_);

    this->asyncf = true;            

    LOG_VERBOSE_INFO(4, "*** info: LocalVector::MoveToAcceleratorAsync() host to accelerator transfer");        
  }

}

template <typename ValueType>
void LocalVector<ValueType>::MoveToHostAsync(void) {

  LOG_DEBUG(this, "LocalVector::MoveToHostAsync()",
            "");

  assert(this->asyncf == false);

  if (_paralution_available_accelerator() == false)
    LOG_VERBOSE_INFO(4,"*** info: LocalVector::MoveToAcceleratorAsync() no accelerator available - doing nothing");  

  if ( (_paralution_available_accelerator() == true) && 
       (this->vector_ == this->vector_accel_)) {
    
    this->vector_host_ = new HostVector<ValueType>(this->local_backend_); 

    // Copy to host
    this->vector_host_->CopyFromAsync(*this->vector_accel_);

    this->asyncf = true;
       
    LOG_VERBOSE_INFO(4, "*** info: LocalVector::MoveToHostAsync() accelerator to host transfer (started)");        
  }
 
}

template <typename ValueType>
void LocalVector<ValueType>::Sync(void) {

  LOG_DEBUG(this, "LocalVector::Sync()",
            "");

  // check for active async transfer
  if (this->asyncf == true) {

    // The Move*Async function is active
    if ( (this->vector_accel_ != NULL) &&
         (this->vector_host_  != NULL)) {
      
      // MoveToHostAsync();
      if ( (_paralution_available_accelerator() == true) && 
           (this->vector_ == this->vector_accel_)) {
        
        _paralution_sync();
        
        this->vector_ = this->vector_host_;
        delete this->vector_accel_;
        this->vector_accel_ = NULL;
        
        LOG_VERBOSE_INFO(4, "*** info: LocalVector::MoveToHostAsync() accelerator to host transfer (synced)");        
      }
      
      // MoveToAcceleratorAsync();
      if ( (_paralution_available_accelerator() == true) && 
           (this->vector_ == this->vector_host_)) {
        
        _paralution_sync();      
        
        this->vector_ = this->vector_accel_;
        delete this->vector_host_;
        this->vector_host_ = NULL;
        LOG_VERBOSE_INFO(4, "*** info: LocalVector::MoveToAcceleratorAsync() host to accelerator transfer (synced)");        
      }
      
    } else {
      // The Copy*Async function is active
      
      _paralution_sync();
      LOG_VERBOSE_INFO(4, "*** info: LocalVector::Copy*Async() transfer (synced)");        
    }

  }

  this->asyncf = false;

}


template <typename ValueType>
ValueType&  LocalVector<ValueType>::operator[](const int i) { 

  LOG_DEBUG(this, "LocalVector::operator[]()",
            i);

  assert(this->vector_host_ != NULL);
  assert((i >= 0) && (i < vector_host_->size_));

  return vector_host_->vec_[i]; 

}

template <typename ValueType>
const ValueType&  LocalVector<ValueType>::operator[](const int i) const { 

  LOG_DEBUG(this, "LocalVector::operator[]()",
            i);

  assert(this->vector_host_ != NULL);
  assert((i >= 0) && (i < vector_host_->size_));

  return vector_host_->vec_[i]; 

}

template <typename ValueType>
void LocalVector<ValueType>::info(void) const {

  std::string current_backend_name;

  if (this->vector_ == this->vector_host_) {
    current_backend_name = _paralution_host_name[0];
  } else {
    assert(this->vector_ == this->vector_accel_);
    current_backend_name = _paralution_backend_name[this->local_backend_.backend];
  }
  
 
  LOG_INFO("LocalVector" 
           << " name=" << this->object_name_  << ";"
           << " size=" << this->get_size() << ";"
           << " prec=" << 8*sizeof(ValueType) << "bit;"
           << " host backend={" << _paralution_host_name[0] << "};" 
           << " accelerator backend={" << _paralution_backend_name[this->local_backend_.backend] << "};"
           << " current=" << current_backend_name);          

}

template <typename ValueType>
void LocalVector<ValueType>::ReadFileASCII(const std::string name) {

  LOG_DEBUG(this, "LocalVector::ReadFileASCII()",
            name);

  // host only
  bool on_host = this->is_host();
  if (on_host == false)
    this->MoveToHost();
  
  assert(this->vector_ == this->vector_host_);
  this->vector_host_->ReadFileASCII(name);
  
  this->object_name_ = name;

  if (on_host == false)
    this->MoveToAccelerator();

}

template <typename ValueType>
void LocalVector<ValueType>::WriteFileASCII(const std::string name) const {

  LOG_DEBUG(this, "LocalVector::WriteFileASCII()",
            name);

  // host only
  assert(this->vector_ == this->vector_host_);
  this->vector_host_->WriteFileASCII(name);

}

template <typename ValueType>
void LocalVector<ValueType>::ReadFileBinary(const std::string name) {

  LOG_DEBUG(this, "LocalVector::ReadFileBinary()",
            name);

  // host only
  bool on_host = this->is_host();
  if (on_host == false)
    this->MoveToHost();
  
  assert(this->vector_ == this->vector_host_);
  this->vector_host_->ReadFileBinary(name);
  
  this->object_name_ = name;

  if (on_host == false)
    this->MoveToAccelerator();

}

template <typename ValueType>
void LocalVector<ValueType>::WriteFileBinary(const std::string name) const {

  LOG_DEBUG(this, "LocalVector::WriteFileBinary()",
            name);

  // host only
  assert(this->vector_ == this->vector_host_);
  this->vector_host_->WriteFileBinary(name);

}

template <typename ValueType>
void LocalVector<ValueType>::AddScale(const LocalVector<ValueType> &x, const ValueType alpha) {

  LOG_DEBUG(this, "LocalVector::AddScale()",
            alpha);

  assert(this->get_size() == x.get_size());

  if (this->get_size() > 0 ) {
    
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    this->vector_->AddScale(*x.vector_, alpha);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::ScaleAdd(const ValueType alpha, const LocalVector<ValueType> &x) {

  LOG_DEBUG(this, "LocalVector::ScaleAdd()",
            alpha);

  assert(this->get_size() == x.get_size());

  if (this->get_size() > 0 ) {
  
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    this->vector_->ScaleAdd(alpha, *x.vector_);
  }

}
 
template <typename ValueType>
void LocalVector<ValueType>::ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta) {

  LOG_DEBUG(this, "LocalVector::ScaleAddScale()",
            alpha << " " << beta);

  assert(this->get_size() == x.get_size());

  if (this->get_size() > 0 ) {
  
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    this->vector_->ScaleAddScale(alpha, *x.vector_, beta);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta,
                                           const int src_offset,
                                           const int dst_offset,
                                           const int size) {

  LOG_DEBUG(this, "LocalVector::ScaleAddScale()",
            alpha << " " << beta << " " <<
            "src_offset=" << src_offset <<
            " dst_offset=" << dst_offset <<
            " size=" << size);

  assert(&x != NULL);
  assert(src_offset < x.get_size());
  assert(dst_offset < this->get_size());

  if (this->get_size() > 0 ) {
  
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    this->vector_->ScaleAddScale(alpha, *x.vector_, beta,
                                 src_offset, dst_offset, size);
  }

}


template <typename ValueType>
void LocalVector<ValueType>::ScaleAdd2(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta, const LocalVector<ValueType> &y, const ValueType gamma) {

  LOG_DEBUG(this, "LocalVector::ScaleAdd2()",
            alpha << " " << beta);

  assert(this->get_size() == x.get_size());
  assert(this->get_size() == y.get_size());

  if (this->get_size() > 0 ) {
  
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_) && (y.vector_ == y.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_) && (y.vector_ == y.vector_accel_)) );
    
    this->vector_->ScaleAdd2(alpha, *x.vector_, beta, *y.vector_, gamma);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::Scale(const ValueType alpha) {

  LOG_DEBUG(this, "LocalVector::Scale()",
            alpha);

  if (this->get_size() > 0 ) {
    this->vector_->Scale(alpha);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::PartialSum(const LocalVector<ValueType> &x) {

  LOG_DEBUG(this, "LocalVector::PartialSum()",
            "");

  assert(this->get_size() == x.get_size());

  if (this->get_size() > 0 ) {
    
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    this->vector_->PartialSum(*x.vector_);
  }

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Dot(const LocalVector<ValueType> &x) const {

  LOG_DEBUG(this, "LocalVector::Dot()",
            "");

  assert(this->get_size() == x.get_size());

  if (this->get_size() > 0 ) {
    
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    return this->vector_->Dot(*x.vector_);
  } else {
    return ValueType(0.0);
  }

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Norm(void) const {

  LOG_DEBUG(this, "LocalVector::Norm()",
            "");

  if (this->get_size() > 0 ) {

    return this->vector_->Norm();

  } else {
    return ValueType(0.0);
  }

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Reduce(void) const {

  LOG_DEBUG(this, "LocalVector::Reduce()",
            "");

  if (this->get_size() > 0 ) {

  return this->vector_->Reduce();

  } else {
    return ValueType(0.0);
  }

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Asum(void) const {

  LOG_DEBUG(this, "LocalVector::Asum()",
            "");

  if (this->get_size() > 0 ) {

  return this->vector_->Asum();

  } else {
    return ValueType(0.0);
  }

}

template <typename ValueType>
int LocalVector<ValueType>::Amax(ValueType &value) const {

  LOG_DEBUG(this, "LocalVector::Amax()",
            "");

  if (this->get_size() > 0 ) {

  return this->vector_->Amax(value);

  } else {
    value = ValueType(0.0);
    return -1;
  }

}

template <typename ValueType>
void LocalVector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x) {

  LOG_DEBUG(this, "LocalVector::PointWiseMult(x)",
            "");

  assert(this->get_size() == x.get_size());

  if (this->get_size() > 0 ) {
    
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );
    
    this->vector_->PointWiseMult(*x.vector_);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x, const LocalVector<ValueType> &y) {

  LOG_DEBUG(this, "LocalVector::(x, y)",
            "");

  assert(this->get_size() == x.get_size());
  assert(this->get_size() == y.get_size());

  if (this->get_size() > 0 ) {
    
    assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_) && (y.vector_ == y.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_) && (y.vector_ == y.vector_accel_)) );
    
    this->vector_->PointWiseMult(*x.vector_, *y.vector_);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFrom(const LocalVector<ValueType> &src,
                                      const int src_offset,
                                      const int dst_offset,
                                      const int size) {

  LOG_DEBUG(this, "LocalVector::CopyFrom()",
            "src_offset=" << src_offset <<
            " dst_offset=" << dst_offset <<
            " size=" << size);


  assert(&src != NULL);
  assert(&src != this);
  assert(src_offset < src.get_size());
  assert(dst_offset < this->get_size());

  assert( ( (this->vector_ == this->vector_host_)  && (src.vector_ == src.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (src.vector_ == src.vector_accel_)) );

  this->vector_->CopyFrom(*src.vector_, src_offset, dst_offset, size); 

  
}

template <typename ValueType>
void LocalVector<ValueType>::Permute(const LocalVector<int> &permutation) {

  LOG_DEBUG(this, "LocalVector::Permute()",
            "");

  assert(&permutation != NULL);
  assert(permutation.get_size() == this->get_size());

  if (this->get_size() > 0 ) {
    this->vector_->Permute(*permutation.vector_);
  }
}

template <typename ValueType>
void LocalVector<ValueType>::PermuteBackward(const LocalVector<int> &permutation) {

  LOG_DEBUG(this, "LocalVector::PermuteBackward()",
            "");

  assert(&permutation != NULL);
  assert(permutation.get_size() == this->get_size());

  if (this->get_size() > 0 ) {
    this->vector_->PermuteBackward(*permutation.vector_);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromPermute(const LocalVector<ValueType> &src,
                                             const LocalVector<int> &permutation) {

  LOG_DEBUG(this, "LocalVector::CopyFromPermute()",
            "");

  assert(&src != NULL);   
  assert(&src != this);   
  assert(&permutation != NULL);
  assert(permutation.get_size() == this->get_size());
  assert(this->get_size() == src.get_size());

  if (this->get_size() > 0 ) {
    
    this->vector_->CopyFromPermute(*src.vector_,
                                   *permutation.vector_);
  }

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromPermuteBackward(const LocalVector<ValueType> &src,
                                                     const LocalVector<int> &permutation) {

  LOG_DEBUG(this, "LocalVector::CopyFromPermuteBackward()",
            "");

  assert(&src != NULL);   
  assert(&src != this);   
  assert(&permutation != NULL);
  assert(permutation.get_size() == this->get_size());
  assert(this->get_size() == src.get_size());

  if (this->get_size() > 0 ) {

    this->vector_->CopyFromPermuteBackward(*src.vector_,
                                           *permutation.vector_);
  }

}


template <typename ValueType>
void LocalVector<ValueType>::Restriction(const LocalVector<ValueType> &vec_fine, 
                                         const LocalVector<int> &map) {

  LOG_DEBUG(this, "LocalVector::Restriction()",
            "");

  assert(&vec_fine != NULL);
  assert(&map != NULL);
  assert(&vec_fine != this);

  if (this->get_size() > 0) {

    assert( ( (this->vector_ == this->vector_host_)  && (vec_fine.vector_ == vec_fine.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (vec_fine.vector_ == vec_fine.vector_accel_) ) );
    
    assert( ( (this->vector_ == this->vector_host_)  && (map.vector_ == map.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (map.vector_ == map.vector_accel_) ) );

    bool err = this->vector_->Restriction(*vec_fine.vector_,
                                          *map.vector_);
    
    if ((err == false) && (this->is_host() == true)) {
      LOG_INFO("Computation of LocalVector::Restriction() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);    
    }
    
    if (err == false) {
      
      this->MoveToHost();
      
      LocalVector<int> map_tmp;
      map_tmp.CopyFrom(map);
      map_tmp.MoveToHost();
      
      LocalVector<ValueType> vec_fine_tmp;
      vec_fine_tmp.CopyFrom(vec_fine);
      vec_fine_tmp.MoveToHost();
      
      if (this->vector_->Restriction(*vec_fine_tmp.vector_, *map_tmp.vector_) == false) {
        LOG_INFO("Computation of LocalVector::Restriction() fail");
        this->info();
        FATAL_ERROR(__FILE__, __LINE__);
      }
      
      LOG_VERBOSE_INFO(2, "*** warning: LocalVector::Restriction() is performed on the host");
      
      this->MoveToAccelerator();
      
    }
  }


}

template <typename ValueType>
void LocalVector<ValueType>::Prolongation(const LocalVector<ValueType> &vec_coarse, 
                                          const LocalVector<int> &map) {

  LOG_DEBUG(this, "LocalVector::Prolongation()",
            "");

  assert(&vec_coarse != NULL);
  assert(&map != NULL);
  assert(&vec_coarse != this);

  if (this->get_size() > 0) {
    
    assert( ( (this->vector_ == this->vector_host_)  && (vec_coarse.vector_ == vec_coarse.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (vec_coarse.vector_ == vec_coarse.vector_accel_) ) );
    
    assert( ( (this->vector_ == this->vector_host_)  && (map.vector_ == map.vector_host_)) ||
            ( (this->vector_ == this->vector_accel_) && (map.vector_ == map.vector_accel_) ) );
    
    
    bool err = this->vector_->Prolongation(*vec_coarse.vector_,
                                           *map.vector_);
    
    if ((err == false) && (this->is_host() == true)) {
      LOG_INFO("Computation of LocalVector::Prolongation() fail");
      this->info();
      FATAL_ERROR(__FILE__, __LINE__);    
    }
    
    if (err == false) {
      
      this->MoveToHost();
      
      LocalVector<int> map_tmp;
      map_tmp.CopyFrom(map);
      map_tmp.MoveToHost();
      
      LocalVector<ValueType> vec_coarse_tmp;
      vec_coarse_tmp.CopyFrom(vec_coarse);
      vec_coarse_tmp.MoveToHost();
      
      if (this->vector_->Prolongation(*vec_coarse_tmp.vector_, *map_tmp.vector_) == false) {
        LOG_INFO("Computation of LocalVector::Prolongation() fail");
        this->info();
        FATAL_ERROR(__FILE__, __LINE__);
      }
      
      LOG_VERBOSE_INFO(2, "*** warning: LocalVector::Prolongation() is performed on the host");
      
      this->MoveToAccelerator();
      
    }
  }

}



template class LocalVector<double>;
template class LocalVector<float>;

template class LocalVector<int>;

}
