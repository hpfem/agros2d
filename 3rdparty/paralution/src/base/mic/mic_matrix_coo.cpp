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

#include "mic_matrix_csr.hpp"
#include "mic_matrix_coo.hpp"
#include "mic_matrix_dia.hpp"
#include "mic_matrix_ell.hpp"
#include "mic_matrix_hyb.hpp"
#include "mic_matrix_mcsr.hpp"
#include "mic_matrix_bcsr.hpp"
#include "mic_matrix_dense.hpp"
#include "mic_vector.hpp"
#include "../host/host_matrix_coo.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "mic_utils.hpp"
#include "mic_allocate_free.hpp"
#include "mic_matrix_coo_kernel.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>



namespace paralution {

template <typename ValueType>
MICAcceleratorMatrixCOO<ValueType>::MICAcceleratorMatrixCOO() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
MICAcceleratorMatrixCOO<ValueType>::MICAcceleratorMatrixCOO(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.row = NULL;  
  this->mat_.col = NULL;  
  this->mat_.val = NULL;
  this->set_backend(local_backend); 

}


template <typename ValueType>
MICAcceleratorMatrixCOO<ValueType>::~MICAcceleratorMatrixCOO() {

  this->Clear();

}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::info(void) const {

  LOG_INFO("MICAcceleratorMatrixCOO<ValueType>");

}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::AllocateCOO(const int nnz, const int nrow, const int ncol) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    allocate_mic(nnz, &this->mat_.row);
    allocate_mic(nnz, &this->mat_.col);
    allocate_mic(nnz, &this->mat_.val);
 
    set_to_zero_mic(nnz, this->mat_.row);
    set_to_zero_mic(nnz, this->mat_.col);
    set_to_zero_mic(nnz, this->mat_.val);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;

  }

}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_mic(&this->mat_.row);
    free_mic(&this->mat_.col);
    free_mic(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }


}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to MIC copy
  if ((cast_mat = dynamic_cast<const HostMatrixCOO<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

  if (this->get_nnz() > 0) {

      assert((this->get_nnz()  == src.get_nnz())  &&
             (this->get_nrow() == src.get_nrow()) &&
             (this->get_ncol() == src.get_ncol()) );

      
      // TODO
      copy_to_mic(cast_mat->mat_.row, this->mat_.row, this->get_nnz());
      copy_to_mic(cast_mat->mat_.col, this->mat_.col, this->get_nnz());
      copy_to_mic(cast_mat->mat_.val, this->mat_.val, this->get_nnz());
      
      /*
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.row[j] = cast_mat->mat_.row[j];
      
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.col[j] = cast_mat->mat_.col[j];
      
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];
      */

    }
    
  } else {
    
    LOG_INFO("Error unsupported MIC matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixCOO<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // MIC to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixCOO<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol() );

  if (this->get_nnz() > 0) {

      assert((this->get_nnz()  == dst->get_nnz())  &&
             (this->get_nrow() == dst->get_nrow()) &&
             (this->get_ncol() == dst->get_ncol()) );

      // TODO

      copy_to_host(this->mat_.row, cast_mat->mat_.row, this->get_nnz());
      copy_to_host(this->mat_.col, cast_mat->mat_.col, this->get_nnz());
      copy_to_host(this->mat_.val, cast_mat->mat_.val, this->get_nnz());

      /*
      for (int j=0; j<this->get_nnz(); ++j)
        cast_mat->mat_.row[j] = this->mat_.row[j] ;
      
      for (int j=0; j<this->get_nnz(); ++j)
        cast_mat->mat_.col[j] = this->mat_.col[j] ;
      
      for (int j=0; j<this->get_nnz(); ++j)
        cast_mat->mat_.val[j] = this->mat_.val[j] ;
      */

    }
    
  } else {
    
    LOG_INFO("Error unsupported MIC matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const MICAcceleratorMatrixCOO<ValueType> *mic_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // MIC to MIC copy
  if ((mic_cast_mat = dynamic_cast<const MICAcceleratorMatrixCOO<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateCOO(src.get_nnz(), src.get_nrow(), src.get_ncol() );

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      // TODO
      copy_mic_mic(mic_cast_mat->mat_.row, this->mat_.row, this->get_nnz());
      copy_mic_mic(mic_cast_mat->mat_.col, this->mat_.col, this->get_nnz());
      copy_mic_mic(mic_cast_mat->mat_.val, this->mat_.val, this->get_nnz());

      /*
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.row[j] = mic_cast_mat->mat_.row[j];
      
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.col[j] = mic_cast_mat->mat_.col[j];
      
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.val[j] = mic_cast_mat->mat_.val[j];
      */
    }

  } else {

    //CPU to MIC
    if ((host_cast_mat = dynamic_cast<const HostMatrix<ValueType>*> (&src)) != NULL) {
      
      this->CopyFromHost(*host_cast_mat);
      
    } else {
      
      LOG_INFO("Error unsupported MIC matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  MICAcceleratorMatrixCOO<ValueType> *mic_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // MIC to MIC copy
  if ((mic_cast_mat = dynamic_cast<MICAcceleratorMatrixCOO<ValueType>*> (dst)) != NULL) {

    mic_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    mic_cast_mat->AllocateCOO(dst->get_nnz(), dst->get_nrow(), dst->get_ncol() );

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

      // TODO
      copy_mic_mic(this->mat_.row, mic_cast_mat->mat_.row, this->get_nnz());
      copy_mic_mic(this->mat_.col, mic_cast_mat->mat_.col, this->get_nnz());
      copy_mic_mic(this->mat_.val, mic_cast_mat->mat_.val, this->get_nnz());


      /*
      for (int j=0; j<this->get_nnz(); ++j)
        mic_cast_mat->mat_.row[j] = this->mat_.row[j] ;
      
      for (int j=0; j<this->get_nnz(); ++j)
        mic_cast_mat->mat_.col[j] = this->mat_.col[j] ;
      
      for (int j=0; j<this->get_nnz(); ++j)
        mic_cast_mat->mat_.val[j] = this->mat_.val[j] ;
      */
    }
    
  } else {

    //MIC to CPU
    if ((host_cast_mat = dynamic_cast<HostMatrix<ValueType>*> (dst)) != NULL) {
      
      this->CopyToHost(host_cast_mat);

    } else {
      
      LOG_INFO("Error unsupported MIC matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }


}


template <typename ValueType>
bool MICAcceleratorMatrixCOO<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  return false;

}

// TODO
// MKL

template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const MICAcceleratorVector<ValueType> *cast_in = dynamic_cast<const MICAcceleratorVector<ValueType>*> (&in) ; 
    MICAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      MICAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    spmv_coo(this->mat_.row,
	     this->mat_.col,
	     this->mat_.val,
	     this->get_nrow(),
	     this->get_nnz(),
	     cast_in->vec_,
	     cast_out->vec_);

  }

}


template <typename ValueType>
void MICAcceleratorMatrixCOO<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                                  BaseVector<ValueType> *out) const {
  
  if (this->get_nnz() > 0) {
    
    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const MICAcceleratorVector<ValueType> *cast_in = dynamic_cast<const MICAcceleratorVector<ValueType>*> (&in) ; 
    MICAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      MICAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    spmv_add_coo(this->mat_.row,
		 this->mat_.col,
		 this->mat_.val,
		 this->get_nrow(),
		 this->get_nnz(),
		 scalar,
		 cast_in->vec_,
		 cast_out->vec_);

  }

}


template <typename ValueType>
bool MICAcceleratorMatrixCOO<ValueType>::Permute(const BaseVector<int> &permutation) {

  assert(&permutation != NULL);

  // symmetric permutation only
  assert( (permutation.get_size() == this->get_nrow()) &&
          (permutation.get_size() == this->get_ncol()) );

  if (this->get_nnz() > 0) {

    const MICAcceleratorVector<int> *cast_perm = dynamic_cast<const MICAcceleratorVector<int>*> (&permutation) ;   
    assert(cast_perm != NULL);
    
    MICAcceleratorMatrixCOO<ValueType> src(this->local_backend_);       
    src.AllocateCOO(this->get_nnz(), this->get_nrow(), this->get_ncol());
    src.CopyFrom(*this);

    //  omp_set_num_threads(this->local_backend_.OpenMP_threads);  

  // TODO

#pragma omp parallel for      
  for (int i=0; i<this->get_nnz(); ++i) {
 
    this->mat_.row[i] = cast_perm->vec_[ src.mat_.row[i] ];
    this->mat_.col[i] = cast_perm->vec_[ src.mat_.col[i] ];

  }

  }

  return true;

}


template <typename ValueType>
bool MICAcceleratorMatrixCOO<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {

  return false;

}


template class MICAcceleratorMatrixCOO<double>;
template class MICAcceleratorMatrixCOO<float>;

}
