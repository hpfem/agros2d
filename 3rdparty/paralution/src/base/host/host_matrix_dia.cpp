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


#include "host_matrix_dia.hpp"
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

#ifdef SUPPORT_MKL
#include <mkl.h>
#include <mkl_spblas.h>
#endif

namespace paralution {


template <typename ValueType>
HostMatrixDIA<ValueType>::HostMatrixDIA() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostMatrixDIA<ValueType>::HostMatrixDIA(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "HostMatrixDIA::HostMatrixDIA()",
            "constructor with local_backend");

  this->mat_.val = NULL;
  this->mat_.offset = NULL;
  this->mat_.num_diag = 0;
  this->set_backend(local_backend);

}

template <typename ValueType>
HostMatrixDIA<ValueType>::~HostMatrixDIA() {

  LOG_DEBUG(this, "HostMatrixDIA::~HostMatrixDIA()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::info(void) const {

  LOG_INFO("HostMatrixDIA<ValueType>, diag = " << this->mat_.num_diag << " nnz=" << this->nnz_);

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::Clear() {

  if (this->nnz_ > 0) {

    free_host(&this->mat_.val);
    free_host(&this->mat_.offset);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;
    this->mat_.num_diag = 0;

  }

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag) {

  assert( nnz   >= 0);
  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->nnz_ > 0)
    this->Clear();

  if (nnz > 0) {

    assert( ndiag > 0);

    allocate_host(nnz, &this->mat_.val);
    allocate_host(ndiag, &this->mat_.offset);

    set_to_zero_host(nnz, mat_.val);
    set_to_zero_host(ndiag, mat_.offset);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;
    this->mat_.num_diag = ndiag;

  }

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::CopyFrom(const BaseMatrix<ValueType> &mat) {

  // copy only in the same format
  assert(this->get_mat_format() == mat.get_mat_format());

  if (const HostMatrixDIA<ValueType> *cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&mat)) {

    this->AllocateDIA(cast_mat->nnz_, cast_mat->nrow_, cast_mat->ncol_, cast_mat->mat_.num_diag);

    assert((this->nnz_  == cast_mat->nnz_)  &&
           (this->nrow_ == cast_mat->nrow_) &&
           (this->ncol_ == cast_mat->ncol_));

    if (this->nnz_ > 0) {

      _set_omp_backend_threads(this->local_backend_, this->nrow_);

#pragma omp parallel for
      for (int j=0; j<this->nnz_; ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];

      for (int j=0; j<this->mat_.num_diag; ++j)
        this->mat_.offset[j] = cast_mat->mat_.offset[j];

    }

  } else {

    // Host matrix knows only host matrices
    // -> dispatching
    mat.CopyTo(this);

  }

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::CopyTo(BaseMatrix<ValueType> *mat) const {

  mat->CopyFrom(*this);

}

template <typename ValueType>
bool HostMatrixDIA<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  if (const HostMatrixDIA<ValueType> *cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&mat)) {

    this->CopyFrom(*cast_mat);
    return true;

  }

  if (const HostMatrixCSR<ValueType> *cast_mat = dynamic_cast<const HostMatrixCSR<ValueType>*> (&mat)) {

    this->Clear();
    int nnz = 0;

    csr_to_dia(this->local_backend_.OpenMP_threads,
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
void HostMatrixDIA<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

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
    for (int i=0; i<this->nrow_; ++i) {

      ValueType sum = ValueType(0.0);

      for (int j=0; j<this->mat_.num_diag; ++j) {

        int start = 0;
        int end = this->nrow_;
        int v_offset = 0;
        int offset = this->mat_.offset[j];

        if (offset < 0) {
          start -= offset;
          v_offset = -start;
        } else {
          end -= offset;
          v_offset = offset;
        }

        if ((i >= start) && (i < end))
          sum += this->mat_.val[DIA_IND(i, j, this->nrow_, this->mat_.num_diag)] * cast_in->vec_[i+v_offset];
        else
          if (i >= end)
            break;

      }

      cast_out->vec_[i] = sum;

    }

  }

}

template <typename ValueType>
void HostMatrixDIA<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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
    for (int i=0; i<this->nrow_; ++i) {

      for (int j=0; j<this->mat_.num_diag; ++j) {

        int start = 0;
        int end = this->nrow_;
        int v_offset = 0;
        int offset = this->mat_.offset[j];

        if (offset < 0) {
          start -= offset;
          v_offset = -start;
        } else {
          end -= offset;
          v_offset = offset;
        }

        if ((i >= start) && (i < end))
          cast_out->vec_[i] += scalar*this->mat_.val[DIA_IND(i, j, this->nrow_, this->mat_.num_diag)]
                             * cast_in->vec_[i+v_offset];
        else
          if (i >= end)
            break;

      }

    }

  }

}


template class HostMatrixDIA<double>;
template class HostMatrixDIA<float>;

}
