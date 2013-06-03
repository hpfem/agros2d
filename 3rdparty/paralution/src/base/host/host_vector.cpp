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

#include "host_vector.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"

#include <typeinfo>
#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_set_num_threads(num) ;
#endif

#ifdef SUPPORT_MKL
#include <mkl.h>
#include <mkl_spblas.h>
#endif



namespace paralution {

template <typename ValueType>
HostVector<ValueType>::HostVector() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostVector<ValueType>::HostVector(const Paralution_Backend_Descriptor local_backend) {

  this->vec_ = NULL ; 
  this->set_backend(local_backend); 

}

template <typename ValueType>
HostVector<ValueType>::~HostVector() {

  this->Clear();

}

template <typename ValueType>
void HostVector<ValueType>::info(void) const {

  LOG_INFO("HostVector<ValueType>, OpenMP threads: " << this->local_backend_.OpenMP_threads);

}

template <typename ValueType>
void HostVector<ValueType>::Allocate(const int n) {

  assert(n >= 0);

  if (this->size_ >0)
    this->Clear();

  if (n > 0) {

    allocate_host(n, &this->vec_);
    
    set_to_zero_host(n, this->vec_);

    this->size_ = n;
  }

}

template <typename ValueType>
void HostVector<ValueType>::SetDataPtr(ValueType **ptr, const int size) {

  assert(*ptr != NULL);
  assert(size > 0);

  this->Clear();

  this->vec_ = *ptr;
  this->size_ = size ;

}


template <typename ValueType>
void HostVector<ValueType>::LeaveDataPtr(ValueType **ptr) {

  assert(this->get_size() > 0);

  // see free_host function for details
  *ptr = this->vec_;
  this->vec_ = NULL;

  this->size_ = 0 ;

}


template <typename ValueType>
void HostVector<ValueType>::Clear(void) {

  if (this->size_ >0) {

    free_host(&this->vec_);

    this->size_ = 0 ;
  }

}

template <typename ValueType>
void HostVector<ValueType>::CopyFrom(const BaseVector<ValueType> &vec) {

  if (this != &vec)  {
    
    if (const HostVector<ValueType> *cast_vec = dynamic_cast<const HostVector<ValueType>*> (&vec)) {
     
      if (this->get_size() == 0)
        this->Allocate(cast_vec->get_size());

      assert(cast_vec->get_size() == this->get_size());

      for (int i=0; i<this->size_; ++i)
        this->vec_[i] = cast_vec->vec_[i];
      
      
    } else {
      
      // non-host type

    vec.CopyTo(this);
    
    }
  }

}

template <typename ValueType>
void HostVector<ValueType>::CopyTo(BaseVector<ValueType> *vec) const {

  vec->CopyFrom(*this);

}

template <typename ValueType>
void HostVector<ValueType>::CopyFromFloat(const BaseVector<float> &vec) {

  if (const HostVector<float> *cast_vec = dynamic_cast<const HostVector<float>*> (&vec)) {
     
    if (this->get_size() == 0)
      this->Allocate(cast_vec->get_size());
    
    assert(cast_vec->get_size() == this->get_size());
    
    for (int i=0; i<this->size_; ++i)
      this->vec_[i] = ValueType(cast_vec->vec_[i]);
    
    
  } else {
    
    LOG_INFO("No cross backend casting");
    FATAL_ERROR(__FILE__, __LINE__);
    
  }
  
}

template <typename ValueType>
void HostVector<ValueType>::CopyFromDouble(const BaseVector<double> &vec) {

  if (const HostVector<double> *cast_vec = dynamic_cast<const HostVector<double>*> (&vec)) {
     
    if (this->get_size() == 0)
      this->Allocate(cast_vec->get_size());
    
    assert(cast_vec->get_size() == this->get_size());
    
    for (int i=0; i<this->size_; ++i)
      this->vec_[i] = ValueType(cast_vec->vec_[i]);
    
    
  } else {
  
    LOG_INFO("No cross backend casting");
    FATAL_ERROR(__FILE__, __LINE__);
  
  }

}

template <typename ValueType>
void HostVector<ValueType>::Zeros(void) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = ValueType(0.0);

}

template <typename ValueType>
void HostVector<ValueType>::Ones(void) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = ValueType(1.0);

}

template <typename ValueType>
void HostVector<ValueType>::SetValues(const ValueType val) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = val;

}

template <typename ValueType>
void HostVector<ValueType>::ReadFileASCII(const std::string filename) {

  std::ifstream file;
  std::string line;
  int n = 0;

  LOG_INFO("ReadFileASCII: filename="<< filename << "; reading...");

  file.open((char*)filename.c_str(), std::ifstream::in);
   
  if (!file.is_open()) {
    LOG_INFO("Can not open vector file [read]:" << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }

  this->Clear();
  
  // get the size of the vector
  while (std::getline(file, line))
    ++n;

  this->Allocate(n);

  file.clear();
  file.seekg(0, std::ios_base::beg);
  n = 0 ;

  while (std::getline(file, line)) {
    
    this->vec_[n] = atof(line.c_str());
    ++n;
    
  }

  file.close();

  LOG_INFO("ReadFileASCII: filename="<< filename << "; done");

}

template <>
void HostVector<int>::ReadFileASCII(const std::string filename) {

  std::ifstream file;
  std::string line;
  int n = 0;

  LOG_INFO("ReadFileASCII: filename="<< filename << "; reading...");

  file.open((char*)filename.c_str(), std::ifstream::in);
   
  if (!file.is_open()) {
    LOG_INFO("Can not open vector file [read]:" << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }

  this->Clear();
  
  // get the size of the vector
  while (std::getline(file, line))
    ++n;

  this->Allocate(n);

  file.clear();
  file.seekg(0, std::ios_base::beg);
  n = 0 ;

  while (std::getline(file, line)) {
    
    this->vec_[n] = atoi(line.c_str());
    ++n;
    
  }

  file.close();

  LOG_INFO("ReadFileASCII: filename="<< filename << "; done");

}

template <typename ValueType>
void HostVector<ValueType>::WriteFileASCII(const std::string filename) const {

  std::ofstream file;
  std::string line;

  LOG_INFO("WriteFileASCII: filename="<< filename << "; writing...");


  file.open((char*)filename.c_str(), std::ifstream::out );
  
  if (!file.is_open()) {
    LOG_INFO("Can not open vector file [write]:" << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }
      
  file.setf(std::ios::scientific);

  for (int n=0; n<this->size_; n++)
    file << this->vec_[n] << std::endl;

  file.close();

  LOG_INFO("WriteFileASCII: filename="<< filename << "; done");

}

#ifdef SUPPORT_MKL

template <>
void HostVector<double>::AddScale(const BaseVector<double> &x, const double alpha) {

  assert(this->get_size() == x.get_size());

  const HostVector<double> *cast_x = dynamic_cast<const HostVector<double>*> (&x);
  assert(cast_x != NULL);

  cblas_daxpy(this->get_size(), alpha, cast_x->vec_, 1, this->vec_, 1);
    
}

template <>
void HostVector<float>::AddScale(const BaseVector<float> &x, const float alpha) {

  assert(this->get_size() == x.get_size());

  const HostVector<float> *cast_x = dynamic_cast<const HostVector<float>*> (&x);
  assert(cast_x != NULL);

  cblas_saxpy(this->get_size(), alpha, cast_x->vec_, 1, this->vec_, 1);
    
}

template <>
void HostVector<int>::AddScale(const BaseVector<int> &x, const int alpha) {

  assert(this->get_size() == x.get_size());

  const HostVector<int> *cast_x = dynamic_cast<const HostVector<int>*> (&x);
  assert(cast_x != NULL);

  LOG_INFO("No int MKL axpy function");
  FATAL_ERROR(__FILE__, __LINE__);
     
}


#else

template <typename ValueType>
void HostVector<ValueType>::AddScale(const BaseVector<ValueType> &x, const ValueType alpha) {

  assert(this->get_size() == x.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  assert(cast_x != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = this->vec_[i] + alpha*cast_x->vec_[i];

}

#endif

template <typename ValueType>
void HostVector<ValueType>::ScaleAdd(const ValueType alpha, const BaseVector<ValueType> &x) {

  assert(this->get_size() == x.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  assert(cast_x != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = alpha*this->vec_[i] + cast_x->vec_[i];

}


template <typename ValueType>
void HostVector<ValueType>::ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta) {

  assert(this->get_size() == x.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  assert(cast_x != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = alpha*this->vec_[i] + beta*cast_x->vec_[i];

}

template <typename ValueType>
void HostVector<ValueType>::ScaleAdd2(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta, const BaseVector<ValueType> &y, const ValueType gamma) {

  assert(this->get_size() == x.get_size());
  assert(this->get_size() == y.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  const HostVector<ValueType> *cast_y = dynamic_cast<const HostVector<ValueType>*> (&y);
  assert(cast_x != NULL);
  assert(cast_y != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = alpha*this->vec_[i] + beta*cast_x->vec_[i] + gamma*cast_y->vec_[i];

}

#ifdef SUPPORT_MKL

template <>
void HostVector<double>::Scale(const double alpha) {

  cblas_dscal(this->get_size(), alpha, this->vec_, 1);

}

template <>
void HostVector<float>::Scale(const float alpha) {

  cblas_sscal(this->get_size(), alpha, this->vec_, 1);

}

template <>
void HostVector<int>::Scale(const int alpha) {

  LOG_INFO("No int MKL scale function");
  FATAL_ERROR(__FILE__, __LINE__);  

}


#else

template <typename ValueType>
void HostVector<ValueType>::Scale(const ValueType alpha) {

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for 
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] *= alpha ; 



}

#endif

template <typename ValueType>
void HostVector<ValueType>::PartialSum(const BaseVector<ValueType> &x) {

  assert(this->get_size() == x.get_size());

  const HostVector<double> *cast_x = dynamic_cast<const HostVector<double>*> (&x);
  assert(cast_x != NULL);

  this->vec_[0] = cast_x->vec_[0];

#pragma omp parallel for
  for (int i=1; i<this->size_; ++i)
    this->vec_[i] = cast_x->vec_[i] + cast_x->vec_[i-1];

}


#ifdef SUPPORT_MKL

template <>
double HostVector<double>::Dot(const BaseVector<double> &x) const {

  assert(this->get_size() == x.get_size());

  const HostVector<double> *cast_x = dynamic_cast<const HostVector<double>*> (&x);
  assert(cast_x != NULL);

  return cblas_ddot(this->get_size(), this->vec_, 1, cast_x->vec_, 1);
}

template <>
float HostVector<float>::Dot(const BaseVector<float> &x) const {

  assert(this->get_size() == x.get_size());

  const HostVector<float> *cast_x = dynamic_cast<const HostVector<float>*> (&x);
  assert(cast_x != NULL);

  return cblas_sdot(this->get_size(), this->vec_, 1, cast_x->vec_, 1);
}

template <>
int HostVector<int>::Dot(const BaseVector<int> &x) const {

  assert(this->get_size() == x.get_size());

  const HostVector<int> *cast_x = dynamic_cast<const HostVector<int>*> (&x);
  assert(cast_x != NULL);

  LOG_INFO("No int MKL dot function");
  FATAL_ERROR(__FILE__, __LINE__);

}


#else

template <typename ValueType>
ValueType HostVector<ValueType>::Dot(const BaseVector<ValueType> &x) const {

  assert(this->get_size() == x.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  assert(cast_x != NULL);

  ValueType dot = ValueType(0.0);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for reduction(+:dot)
  for (int i=0; i<this->size_; ++i)
    dot += this->vec_[i]*cast_x->vec_[i];

  return dot;
}

#endif


#ifdef SUPPORT_MKL

template <>
double HostVector<double>::Norm(void) const {

  return cblas_dnrm2(this->get_size(), this->vec_, 1);

}

template <>
float HostVector<float>::Norm(void) const {

  return cblas_snrm2(this->get_size(), this->vec_, 1);

}

#else 

template <typename ValueType>
ValueType HostVector<ValueType>::Norm(void) const {

  ValueType norm2 = ValueType(0.0);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for reduction(+:norm2)
  for (int i=0; i<this->size_; ++i)
    norm2 += this->vec_[i] * this->vec_[i];

  return sqrt(norm2);

}

#endif

template <typename ValueType>
ValueType HostVector<ValueType>::Reduce(void) const {

  ValueType reduce = ValueType(0.0);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for reduction(+:reduce)
  for (int i=0; i<this->size_; ++i)
    reduce += this->vec_[i];

  return reduce;

}

template <>
int HostVector<int>::Norm(void) const {

  LOG_INFO("What is int HostVector<ValueType>::Norm(void) const?");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
void HostVector<ValueType>::PointWiseMult(const BaseVector<ValueType> &x) {

  assert(this->get_size() == x.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  assert(cast_x != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = this->vec_[i]*cast_x->vec_[i];

}

template <typename ValueType>
void HostVector<ValueType>::PointWiseMult(const BaseVector<ValueType> &x, const BaseVector<ValueType> &y) {

  assert(this->get_size() == x.get_size());
  assert(this->get_size() == y.get_size());

  const HostVector<ValueType> *cast_x = dynamic_cast<const HostVector<ValueType>*> (&x);
  const HostVector<ValueType> *cast_y = dynamic_cast<const HostVector<ValueType>*> (&y);
  assert(cast_x != NULL);
  assert(cast_y != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->size_; ++i)
    this->vec_[i] = cast_y->vec_[i]*cast_x->vec_[i];

}

template <typename ValueType>
void HostVector<ValueType>::CopyFrom(const BaseVector<ValueType> &src,
                                     const int src_offset,
                                     const int dst_offset,
                                     const int size) {
  //TOOD check always for == this?
  assert(&src != this);
  assert(this->get_size() > 0);
  assert(src.  get_size() > 0);
  assert(size > 0);

  assert(src_offset + size <= src.get_size());
  assert(dst_offset + size <= this->get_size());


  const HostVector<ValueType> *cast_src = dynamic_cast<const HostVector<ValueType>*> (&src);
  assert(cast_src != NULL);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

#pragma omp parallel for
  for (int i=0; i<size; ++i)
    this->vec_[i+dst_offset] = cast_src->vec_[i+src_offset];

  
}

template <typename ValueType>
void HostVector<ValueType>::Permute(const BaseVector<int> &permutation) {
  
  assert(&permutation != NULL);
  assert(this->get_size() == permutation.get_size());

  const HostVector<int> *cast_perm = dynamic_cast<const HostVector<int>*> (&permutation) ; 
  assert(cast_perm != NULL);

  HostVector<ValueType> vec_tmp(this->local_backend_);     
  vec_tmp.Allocate(this->get_size());
  vec_tmp.CopyFrom(*this);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->get_size(); ++i)
    this->vec_[ cast_perm->vec_[i] ] = vec_tmp.vec_[i];
  
}

template <typename ValueType>
void HostVector<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {
  
  assert(&permutation != NULL);
  assert(this->get_size() == permutation.get_size());

  const HostVector<int> *cast_perm = dynamic_cast<const HostVector<int>*> (&permutation) ; 
  assert(cast_perm != NULL);

  HostVector<ValueType> vec_tmp(this->local_backend_);   
  vec_tmp.Allocate(this->get_size());
  vec_tmp.CopyFrom(*this);

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->get_size(); ++i)
    this->vec_[i] = vec_tmp.vec_[ cast_perm->vec_[i] ];
  
}

template <typename ValueType>
void HostVector<ValueType>::CopyFromPermute(const BaseVector<ValueType> &src,
                                            const BaseVector<int> &permutation) {

  assert(this != &src);

  const HostVector<ValueType> *cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src);
  const HostVector<int> *cast_perm      = dynamic_cast<const HostVector<int>*> (&permutation);
  assert(cast_perm != NULL);
  assert(cast_vec  != NULL);

  assert(cast_vec ->get_size() == this->get_size());
  assert(cast_perm->get_size() == this->get_size());

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->get_size(); ++i)
    this->vec_[ cast_perm->vec_[i] ] = cast_vec->vec_[i];
  

}

template <typename ValueType>
void HostVector<ValueType>::CopyFromPermuteBackward(const BaseVector<ValueType> &src,
                                                    const BaseVector<int> &permutation) {

  assert(this != &src);

  const HostVector<ValueType> *cast_vec = dynamic_cast<const HostVector<ValueType>*> (&src);
  const HostVector<int> *cast_perm      = dynamic_cast<const HostVector<int>*> (&permutation);
  assert(cast_perm != NULL);
  assert(cast_vec  != NULL);

  assert(cast_vec ->get_size() == this->get_size());
  assert(cast_perm->get_size() == this->get_size());

  omp_set_num_threads(this->local_backend_.OpenMP_threads);

#pragma omp parallel for
  for (int i=0; i<this->get_size(); ++i)
    this->vec_[i] = cast_vec->vec_[ cast_perm->vec_[i] ];
  

}

template <typename ValueType>
bool HostVector<ValueType>::Restriction(const BaseVector<ValueType> &vec_fine, const BaseVector<int> &map) {

  assert(this != &vec_fine);
  
  const HostVector<ValueType> *cast_vec = dynamic_cast<const HostVector<ValueType>*> (&vec_fine);
  const HostVector<int> *cast_map       = dynamic_cast<const HostVector<int>*> (&map) ;
  assert(cast_map != NULL);
  assert(cast_vec != NULL);


  assert(cast_map->get_size() == vec_fine.get_size());
  
  for (int i=0; i<vec_fine.get_size(); ++i) {

    assert(cast_map->vec_[i] < this->get_size());
    this->vec_[ cast_map->vec_[i] ] += cast_vec->vec_[i];

  }

  return true;
}

template <typename ValueType>
bool HostVector<ValueType>::Prolongation(const BaseVector<ValueType> &vec_coarse, const BaseVector<int> &map) {

  assert(this != &vec_coarse);
  
  const HostVector<ValueType> *cast_vec = dynamic_cast<const HostVector<ValueType>*> (&vec_coarse);
  const HostVector<int> *cast_map       = dynamic_cast<const HostVector<int>*> (&map) ;
  assert(cast_map != NULL);
  assert(cast_vec != NULL);


  assert(cast_map->get_size() == this->get_size());
  
  for (int i=0; i<this->get_size(); ++i) {

    assert(cast_map->vec_[i] < vec_coarse.get_size());
    this->vec_[i] += cast_vec->vec_[cast_map->vec_[i]];

  }

  return true;
}


template class HostVector<double>;
template class HostVector<float>;

template class HostVector<int>;

}
