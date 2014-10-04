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


#include "host_matrix_ell.hpp"
#include "host_matrix_csr.hpp"
#include "host_conversion.hpp"
#include "host_vector.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_set_num_threads(num);
#endif

namespace paralution {


template <typename ValueType>
HostMatrixELL<ValueType>::HostMatrixELL() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixELL<ValueType>::HostMatrixELL(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "HostMatrixELL::HostMatrixELL()",
            "constructor with local_backend");

  this->mat_.val = NULL;
  this->mat_.col = NULL;
  this->mat_.max_row = 0;

  this->set_backend(local_backend);

}

template <typename ValueType>
HostMatrixELL<ValueType>::~HostMatrixELL() {

  LOG_DEBUG(this, "HostMatrixELL::~HostMatrixELL()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void HostMatrixELL<ValueType>::info(void) const {

  LOG_INFO("HostMatrixELL<ValueType>");

}

template <typename ValueType>
void HostMatrixELL<ValueType>::Clear() {

  if (this->nnz_ > 0) {

    free_host(&this->mat_.val);
    free_host(&this->mat_.col);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row) {

  assert( nnz   >= 0);
  assert( ncol  >= 0);
  assert( nrow  >= 0);
  assert( max_row >= 0);

  if (this->nnz_ > 0)
    this->Clear();

  if (nnz > 0) {

    assert(nnz == max_row * nrow);

    allocate_host(nnz, &this->mat_.val);
    allocate_host(nnz, &this->mat_.col);

    set_to_zero_host(nnz, this->mat_.val);
    set_to_zero_host(nnz, this->mat_.col);

    this->mat_.max_row = max_row;
    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixELL<ValueType> *cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&mat)) {

    this->AllocateELL(cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_, cast_mat->mat_.max_row);

    assert((this->nnz_  == cast_mat->nnz_)  &&
           (this->nrow_ == cast_mat->nrow_) &&
           (this->ncol_ == cast_mat->ncol_));

    if (this->nnz_ > 0) {

      _set_omp_backend_threads(this->local_backend_, this->nrow_);

      int nnz  = this->nnz_;

#pragma omp parallel for
      for (int i=0; i<nnz; ++i)
        this->mat_.val[i] = cast_mat->mat_.val[i];

#pragma omp parallel for
      for (int i=0; i<nnz; ++i)
        this->mat_.col[i] = cast_mat->mat_.col[i];

    }

  } else {

    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);

  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixELL<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  if (const HostMatrixELL<ValueType> *cast_mat = dynamic_cast<const HostMatrixELL<ValueType>*> (&mat)) {

    this->CopyFrom(*cast_mat);
    return true;

  }

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    this->Clear();
    int nnz = 0;

    csr_to_ell(this->local_backend_.OpenMP_threads,
               cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_,
               cast_mat->mat_, &this->mat_, &nnz);

    this->nrow_ = cast_mat->nrow_;
    this->ncol_ = cast_mat->ncol_;
    this->nnz_ = nnz;

    return true;

  }

  return false;

}

template <typename ValueType>
void HostMatrixELL<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->nnz_ > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->ncol_);
    assert(out->get_size() == this->nrow_);

    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in);
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    _set_omp_backend_threads(this->local_backend_, this->nrow_);

#pragma omp parallel for
    for (int ai=0; ai<this->nrow_; ++ai) {
      ValueType sum = ValueType(0.0);

      for (int n=0; n<this->mat_.max_row; ++n) {

        int aj = ELL_IND(ai, n, this->nrow_, this->mat_.max_row);
        int col_aj = this->mat_.col[aj];

        if (col_aj >= 0)
          sum += this->mat_.val[aj] * cast_in->vec_[col_aj];
        else
          break;

      }

      cast_out->vec_[ai] = sum;

    }

  }

}

template <typename ValueType>
void HostMatrixELL<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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

    _set_omp_backend_threads(this->local_backend_, this->nrow_);

#pragma omp parallel for
    for (int ai=0; ai<this->nrow_; ++ai) {
      for (int n=0; n<this->mat_.max_row; ++n) {

        int aj = ELL_IND(ai, n, this->nrow_, this->mat_.max_row);
        int col_aj = this->mat_.col[aj];

        if (col_aj >= 0)
          cast_out->vec_[ai] += scalar * this->mat_.val[aj] * cast_in->vec_[col_aj];
        else
          break;

      }

    }

  }

}


template class HostMatrixELL<double>;
template class HostMatrixELL<float>;

}
