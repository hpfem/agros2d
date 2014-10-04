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


#include "host_matrix_dense.hpp"
#include "host_matrix_csr.hpp"
#include "host_conversion.hpp"
#include "host_vector.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>
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
HostMatrixDENSE<ValueType>::HostMatrixDENSE() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixDENSE<ValueType>::HostMatrixDENSE(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "HostMatrixDENSE::HostMatrixDENSE()",
            "constructor with local_backend");

  this->mat_.val = NULL;
  this->set_backend(local_backend);

}

template <typename ValueType>
HostMatrixDENSE<ValueType>::~HostMatrixDENSE() {

  LOG_DEBUG(this, "HostMatrixDENSE::~HostMatrixDENSE()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::info(void) const {

  LOG_INFO("HostMatrixDENSE<ValueType>");

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::Clear() {

  if (this->nnz_ > 0) {

    free_host(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::AllocateDENSE(const int nrow, const int ncol) {

  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->nnz_ > 0)
    this->Clear();

  if (nrow*ncol > 0) {

    allocate_host(nrow*ncol, &this->mat_.val);
    set_to_zero_host(nrow*ncol, mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nrow*ncol;

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::SetDataPtrDENSE(ValueType **val, const int nrow, const int ncol) {

  assert(*val != NULL);
  assert(nrow > 0);
  assert(ncol > 0);

  this->Clear();

  this->nrow_ = nrow;
  this->ncol_ = ncol;
  this->nnz_  = nrow*ncol;

  this->mat_.val = *val;

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::LeaveDataPtrDENSE(ValueType **val) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);

  *val = this->mat_.val;

  this->mat_.val = NULL;

  this->nrow_ = 0;
  this->ncol_ = 0;
  this->nnz_  = 0;

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixDENSE<ValueType> *cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&mat)) {

    this->AllocateDENSE(cast_mat->nrow_, cast_mat->ncol_);

    assert((this->nnz_  == cast_mat->nnz_)  &&
           (this->nrow_ == cast_mat->nrow_) &&
           (this->ncol_ == cast_mat->ncol_) );

    if (this->nnz_ > 0) {

      _set_omp_backend_threads(this->local_backend_, this->nnz_);

#pragma omp parallel for
      for (int j=0; j<this->nnz_; ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];

    }

  } else {

    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixDENSE<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  if (const HostMatrixDENSE<ValueType> *cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&mat)) {

    this->CopyFrom(*cast_mat);
    return true;

  }

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    this->Clear();

    csr_to_dense(this->local_backend_.OpenMP_threads,
                 cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_, cast_mat->mat_, &this->mat_);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_ = this->nrow_ * this->ncol_;

    return true;

  }

  return false;

}

#ifdef SUPPORT_MKL

template <>
void HostMatrixDENSE<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in);
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  double alpha = double(1.0);
  double beta = double(0.0);
  int nrow = this->nrow_;
  int ncol = this->ncol_;

  cblas_dgemv(CblasRowMajor, CblasNoTrans,
              nrow, ncol,
              alpha, this->mat_.val,
              nrow,
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);

}

template <>
void HostMatrixDENSE<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<float> *cast_in = dynamic_cast<const HostVector<float>*> (&in);
  HostVector<float> *cast_out      = dynamic_cast<      HostVector<float>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  float alpha = float(1.0);
  float beta = float(0.0);
  int nrow = this->nrow_;
  int ncol = this->ncol_;

  cblas_sgemv(CblasRowMajor, CblasNoTrans,
              nrow, ncol,
              alpha, this->mat_.val,
              nrow,
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);

}

#else

template <typename ValueType>
void HostMatrixDENSE<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in);
  HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  _set_omp_backend_threads(this->local_backend_, this->nnz_);

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai) {
    cast_out->vec_[ai] = ValueType(0.0);
      for (int aj=0; aj<this->ncol_; ++aj)
        cast_out->vec_[ai] += this->mat_.val[DENSE_IND(ai,aj,this->nrow_,this->ncol_)] * cast_in->vec_[aj];
  }

}

#endif

#ifdef SUPPORT_MKL

template <>
void HostMatrixDENSE<double>::ApplyAdd(const BaseVector<double> &in, const double scalar,
                                       BaseVector<double> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<double> *cast_in = dynamic_cast<const HostVector<double>*> (&in);
  HostVector<double> *cast_out      = dynamic_cast<      HostVector<double>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  double beta = double(1.0);
  int nrow = this->nrow_;
  int ncol = this->ncol_;

  cblas_dgemv(CblasRowMajor, CblasNoTrans,
              nrow, ncol,
              scalar, this->mat_.val,
              nrow,
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);

}

template <>
void HostMatrixDENSE<float>::ApplyAdd(const BaseVector<float> &in, const float scalar,
                                      BaseVector<float> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->ncol_);
  assert(out->get_size() == this->nrow_);

  const HostVector<float> *cast_in = dynamic_cast<const HostVector<float>*> (&in);
  HostVector<float> *cast_out      = dynamic_cast<      HostVector<float>*> (out);

  assert(cast_in != NULL);
  assert(cast_out!= NULL);

  float beta = float(1.0);
  int nrow = this->nrow_;
  int ncol = this->ncol_;

  cblas_sgemv(CblasRowMajor, CblasNoTrans,
              nrow, ncol,
              scalar, this->mat_.val,
              nrow,
              cast_in->vec_, 1, beta,
              cast_out->vec_, 1);

}

#else

template <typename ValueType>
void HostMatrixDENSE<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                          BaseVector<ValueType> *out) const {

  if (this->nnz_ > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->ncol_);
    assert(out->get_size() == this->nrow_);

    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in);
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

  _set_omp_backend_threads(this->local_backend_, this->nnz_);

#pragma omp parallel for
  for (int ai=0; ai<this->nrow_; ++ai)
    for (int aj=0; aj<this->ncol_; ++aj)
      cast_out->vec_[ai] += scalar * this->mat_.val[DENSE_IND(ai,aj,this->nrow_,this->ncol_)] * cast_in->vec_[aj];

  }

}

#endif

template <typename ValueType>
void HostMatrixDENSE<ValueType>::Householder(const int idx, ValueType &beta, BaseVector<ValueType> *vec) {

  HostVector<ValueType> *cast_vec = dynamic_cast<HostVector<ValueType>*> (vec);
  assert(cast_vec != NULL);
  assert(cast_vec->get_size() >= this->nrow_-idx);

  ValueType s  = 0.0;
  ValueType mu;

  for (int i=0; i<this->nrow_-idx; ++i)
    cast_vec->vec_[i] = this->mat_.val[DENSE_IND(idx, i+idx, this->nrow_, this->ncol_)];

  ValueType y1 = cast_vec->vec_[0];

  for (int i=1; i<this->nrow_-idx; ++i)
    s += cast_vec->vec_[i] * cast_vec->vec_[i];

  cast_vec->vec_[0] = 1.0;

  if (s == 0.0) {

    beta = 0.0;

  } else {

    mu = sqrt(y1 * y1 + s);

    if (y1 <= 0.0)
      cast_vec->vec_[0] = y1 - mu;
    else
      cast_vec->vec_[0] = -s / (y1 + mu);

    ValueType y0sq = cast_vec->vec_[0] * cast_vec->vec_[0];
    beta = 2 * y0sq / (s + y0sq);

    ValueType norm = cast_vec->vec_[0];
    for (int i=0; i<this->nrow_-idx; ++i)
      cast_vec->vec_[i] /= norm;

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::QRDecompose(void) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);

  int size = (this->nrow_ < this->ncol_) ? this->nrow_ : this->ncol_;
  ValueType beta;
  HostVector<ValueType> v(this->local_backend_);
  v.Allocate(this->nrow_);

  for (int i=0; i<size; ++i) {

    this->Householder(i, beta, &v);

    if (beta != 0.0) {

      for (int aj=i; aj<this->ncol_; ++aj) {
        ValueType sum = 0.0;
        for (int ai=i; ai<this->nrow_; ++ai)
          sum += v.vec_[ai-i] * this->mat_.val[DENSE_IND(ai, aj, this->nrow_, this->ncol_)];
        sum *= beta;

        for (int ai=i; ai<this->nrow_; ++ai)
          this->mat_.val[DENSE_IND(ai, aj, this->nrow_, this->ncol_)] -= sum * v.vec_[ai-i];

      }

      for (int k=i+1; k<this->nrow_; ++k)
        this->mat_.val[DENSE_IND(k, i, this->nrow_, this->ncol_)] = v.vec_[k-i];

    }

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::QRSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->nrow_);
  assert(out->get_size() == this->ncol_);

  HostVector<ValueType> *cast_out = dynamic_cast<HostVector<ValueType>*>(out);

  assert(cast_out!= NULL);

  HostVector<ValueType> copy_in(this->local_backend_);
  copy_in.CopyFrom(in);

  int size = (this->nrow_ < this->ncol_) ? this->nrow_ : this->ncol_;

  ValueType *v = NULL;
  allocate_host(this->nrow_, &v);

  // Apply Q^T on copy_in
  v[0] = 1.0;
  for (int i=0; i<size; ++i) {

    ValueType sum = 1.0;
    for (int j=1; j<this->nrow_-i; ++j) {
      v[j] = this->mat_.val[DENSE_IND(j+i,i,this->nrow_, this->ncol_)];
      sum += v[j] * v[j];
    }
    sum = ValueType(2.0) / sum;

    if (sum != 2.0) {

      ValueType sum2 = 0.0;
      for (int j=0; j<this->nrow_-i; ++j)
        sum2 += v[j] * copy_in.vec_[j+i];

      for (int j=0; j<this->nrow_-i; ++j)
        copy_in.vec_[j+i] -= sum * sum2 * v[j];

    }

  }

  free_host(&v);

  // Backsolve Rx = Q^T b
  for (int i=size-1; i>=0; --i) {

    ValueType sum = 0.0;
    for (int j=i+1; j<this->ncol_; ++j)
      sum += this->mat_.val[DENSE_IND(i, j, this->nrow_, this->ncol_)] * cast_out->vec_[j];

    cast_out->vec_[i] = (copy_in.vec_[i] - sum) / this->mat_.val[DENSE_IND(i, i, this->nrow_, this->ncol_)];

  }

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::Invert(void) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);
  assert(this->nrow_ == this->ncol_);

  ValueType *val = NULL;
  allocate_host(this->nrow_ * this->ncol_, &val);

  this->QRDecompose();

#pragma omp parallel for
  for (int i=0; i<this->nrow_; ++i) {

    HostVector<ValueType> sol(this->local_backend_);
    HostVector<ValueType> rhs(this->local_backend_);
    sol.Allocate(this->nrow_);
    rhs.Allocate(this->nrow_);

    rhs.vec_[i] = ValueType(1.0);

    this->QRSolve(rhs, &sol);

    for (int j=0; j<this->ncol_; ++j)
      val[DENSE_IND(j, i, this->nrow_, this->ncol_)] = sol.vec_[j];

  }

  free_host(&this->mat_.val);
  this->mat_.val = val;

}

template <typename ValueType>
void HostMatrixDENSE<ValueType>::LUFactorize(void) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);
  assert(this->nrow_ == this->ncol_);

  for (int i=0; i<this->nrow_-1; ++i)
    for (int j=i+1; j<this->nrow_; ++j) {

      this->mat_.val[DENSE_IND(j, i, this->nrow_, this->ncol_)] /=
      this->mat_.val[DENSE_IND(i, i, this->nrow_, this->ncol_)];

      for (int k=i+1; k<this->ncol_; ++k)
        this->mat_.val[DENSE_IND(j, k, this->nrow_, this->ncol_)] -=
        this->mat_.val[DENSE_IND(j, i, this->nrow_, this->ncol_)] *
        this->mat_.val[DENSE_IND(i, k, this->nrow_, this->ncol_)];

    }

}

template <typename ValueType>
bool HostMatrixDENSE<ValueType>::LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  assert(in.  get_size() >= 0);
  assert(out->get_size() >= 0);
  assert(in.  get_size() == this->nrow_);
  assert(out->get_size() == this->ncol_);

  HostVector<ValueType> *cast_out = dynamic_cast<HostVector<ValueType>*>(out);
  const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*>(&in);

  assert(cast_out!= NULL);

  // fill solution vector
  for (int i=0; i<this->nrow_; ++i)
    cast_out->vec_[i] = cast_in->vec_[i];

  // forward sweeps
  for (int i=0; i<this->nrow_-1; ++i) {
    for (int j=i+1; j<this->nrow_; ++j)
      cast_out->vec_[j] -= cast_out->vec_[i] * this->mat_.val[DENSE_IND(j, i, this->nrow_, this->ncol_)];
  }

  // backward sweeps
  for (int i=this->nrow_-1; i>=0; --i) {
    cast_out->vec_[i] /= this->mat_.val[DENSE_IND(i, i, this->nrow_, this->ncol_)];
    for (int j=0; j<i; ++j)
      cast_out->vec_[j] -= cast_out->vec_[i] * this->mat_.val[DENSE_IND(j, i, this->nrow_, this->ncol_)];
  }

  return true;

}


template class HostMatrixDENSE<double>;
template class HostMatrixDENSE<float>;

}
