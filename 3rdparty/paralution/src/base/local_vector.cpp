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

  this->object_name_ = "";
  this->local_backend_ = _Backend_Descriptor;

  // Create empty vector on the host
  this->vector_host_= new HostVector<ValueType>(this->local_backend_);
  this->vector_accel_= NULL;
  this->vector_ = this->vector_host_ ;
 
}

template <typename ValueType>
LocalVector<ValueType>::~LocalVector() {

  this->Clear();
  delete this->vector_;

}

template <typename ValueType>
int LocalVector<ValueType>::get_size(void) const { 

  return this->vector_->get_size(); 

}


template <typename ValueType>
void LocalVector<ValueType>::Allocate(std::string name, const int size) {

  this->object_name_ = name;

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

template <typename ValueType>
void LocalVector<ValueType>::SetDataPtr(ValueType **ptr, std::string name, const int size) {

  assert(*ptr != NULL);
  assert(size > 0);

  this->Clear();

  this->object_name_ = name;

  this->vector_->SetDataPtr(ptr, size);

}

template <typename ValueType>
void LocalVector<ValueType>::LeaveDataPtr(ValueType **ptr) {

  assert(this->get_size() > 0);

  this->vector_->LeaveDataPtr(ptr);

}

template <typename ValueType>
void LocalVector<ValueType>::Clear(void) {

  this->vector_->Clear();
  
}

template <typename ValueType>
void LocalVector<ValueType>::Zeros(void) {

  this->vector_->Zeros();

}

template <typename ValueType>
void LocalVector<ValueType>::Ones(void) {

  this->vector_->Ones();

}

template <typename ValueType>
void LocalVector<ValueType>::SetValues(const ValueType val) {

  this->vector_->SetValues(val);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFrom(const LocalVector<ValueType> &src) {

  assert(this != &src);

  this->vector_->CopyFrom(*src.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromFloat(const LocalVector<float> &src) {

  this->vector_->CopyFromFloat(*src.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromDouble(const LocalVector<double> &src) {

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

  assert(this != &src);

  this->CloneBackend(src);
  this->CopyFrom(src);

}

template <typename ValueType>
void LocalVector<ValueType>::MoveToAccelerator(void) {

  if (_paralution_available_accelerator() == false)
    LOG_VERBOSE_INFO(4,"*** info: LocalVector:MoveToAccelerator() no accelerator available - doing nothing");  

  if ( (_paralution_available_accelerator() == true) && 
       (this->vector_ == this->vector_host_)) {
    
    this->vector_accel_ = _paralution_init_base_backend_vector<ValueType>(this->local_backend_);

    // Copy to accel
    this->vector_accel_->CopyFrom(*this->vector_host_);
    
    this->vector_ = this->vector_accel_;
    delete this->vector_host_;
    this->vector_host_ = NULL;
        
    LOG_VERBOSE_INFO(4, "*** info: LocalVector:MoveToAccelerator() host to accelerator transfer");        
  }

}

template <typename ValueType>
void LocalVector<ValueType>::MoveToHost(void) {

  if (this->vector_ == this->vector_accel_) {
    
    this->vector_host_ = new HostVector<ValueType>(this->local_backend_); 

    // Copy to host
    this->vector_host_->CopyFrom(*this->vector_accel_);

    this->vector_ = this->vector_host_;
    delete this->vector_accel_;
    this->vector_accel_ = NULL;
       
    LOG_VERBOSE_INFO(4, "*** info: LocalVector:MoveToHost() accelerator to host transfer");        
  }
 
}

template <typename ValueType>
ValueType&  LocalVector<ValueType>::operator[](const int i) { 

  assert(this->vector_host_ != NULL);
  assert((i >= 0) && (i < vector_host_->size_));

  return vector_host_->vec_[i]; 

}

template <typename ValueType>
const ValueType&  LocalVector<ValueType>::operator[](const int i) const { 

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

  // host only
  assert(this->vector_ == this->vector_host_);
  this->vector_host_->WriteFileASCII(name);

}


template <typename ValueType>
void LocalVector<ValueType>::AddScale(const LocalVector<ValueType> &x, const ValueType alpha) {

  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );

  this->vector_->AddScale(*x.vector_, alpha);
          
}

template <typename ValueType>
void LocalVector<ValueType>::ScaleAdd(const ValueType alpha, const LocalVector<ValueType> &x) {
  
  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );

  this->vector_->ScaleAdd(alpha, *x.vector_);

}
 
template <typename ValueType>
void LocalVector<ValueType>::ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta) {
  
  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );

  this->vector_->ScaleAddScale(alpha, *x.vector_, beta);

}

template <typename ValueType>
void LocalVector<ValueType>::ScaleAdd2(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta, const LocalVector<ValueType> &y, const ValueType gamma) {
  
  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_) && (y.vector_ == y.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_) && (y.vector_ == y.vector_accel_)) );

  this->vector_->ScaleAdd2(alpha, *x.vector_, beta, *y.vector_, gamma);

}

template <typename ValueType>
void LocalVector<ValueType>::Scale(const ValueType alpha) {

  this->vector_->Scale(alpha);

}

template <typename ValueType>
void LocalVector<ValueType>::PartialSum(const LocalVector<ValueType> &x) {

  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );

  this->vector_->PartialSum(*x.vector_);

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Dot(const LocalVector<ValueType> &x) const {
  
  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );

  return this->vector_->Dot(*x.vector_);

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Norm(void) const {

  return this->vector_->Norm();

}

template <typename ValueType>
ValueType LocalVector<ValueType>::Reduce(void) const {

  return this->vector_->Reduce();

}

template <typename ValueType>
void LocalVector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x) {

  assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_)) );

  this->vector_->PointWiseMult(*x.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::PointWiseMult(const LocalVector<ValueType> &x, const LocalVector<ValueType> &y) {

   assert( ( (this->vector_ == this->vector_host_)  && (x.vector_ == x.vector_host_) && (y.vector_ == y.vector_host_)) ||
           ( (this->vector_ == this->vector_accel_) && (x.vector_ == x.vector_accel_) && (y.vector_ == y.vector_accel_)) );

   this->vector_->PointWiseMult(*x.vector_, *y.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFrom(const LocalVector<ValueType> &src,
                                      const int src_offset,
                                      const int dst_offset,
                                      const int size) {
  assert(&src != this);

  assert( ( (this->vector_ == this->vector_host_)  && (src.vector_ == src.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (src.vector_ == src.vector_accel_)) );

  this->vector_->CopyFrom(*src.vector_, src_offset, dst_offset, size); 

  
}

template <typename ValueType>
void LocalVector<ValueType>::Permute(const LocalVector<int> &permutation) {


  this->vector_->Permute(*permutation.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::PermuteBackward(const LocalVector<int> &permutation) {

  this->vector_->PermuteBackward(*permutation.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromPermute(const LocalVector<ValueType> &src,
                                             const LocalVector<int> &permutation) {

  assert(&src != this);   
    
  this->vector_->CopyFromPermute(*src.vector_,
                                 *permutation.vector_);

}

template <typename ValueType>
void LocalVector<ValueType>::CopyFromPermuteBackward(const LocalVector<ValueType> &src,
                                                     const LocalVector<int> &permutation) {

  assert(&src != this);

  this->vector_->CopyFromPermuteBackward(*src.vector_,
                                         *permutation.vector_);

}


template <typename ValueType>
void LocalVector<ValueType>::Restriction(const LocalVector<ValueType> &vec_fine, 
                                         const LocalVector<int> &map) {

  assert(&vec_fine != this);

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

template <typename ValueType>
void LocalVector<ValueType>::Prolongation(const LocalVector<ValueType> &vec_coarse, 
                                          const LocalVector<int> &map) {

  assert( ( (this->vector_ == this->vector_host_)  && (vec_coarse.vector_ == vec_coarse.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (vec_coarse.vector_ == vec_coarse.vector_accel_) ) );

  assert( ( (this->vector_ == this->vector_host_)  && (map.vector_ == map.vector_host_)) ||
          ( (this->vector_ == this->vector_accel_) && (map.vector_ == map.vector_accel_) ) );

  assert(&vec_coarse != this);

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



template class LocalVector<double>;
template class LocalVector<float>;

template class LocalVector<int>;

}
