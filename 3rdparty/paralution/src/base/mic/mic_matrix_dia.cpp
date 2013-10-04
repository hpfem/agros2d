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
#include "../host/host_matrix_dia.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "mic_utils.hpp"
#include "mic_allocate_free.hpp"
#include "mic_matrix_dia_kernel.hpp"
#include "../matrix_formats_ind.hpp"

#include <assert.h>


namespace paralution {

template <typename ValueType>
MICAcceleratorMatrixDIA<ValueType>::MICAcceleratorMatrixDIA() {

  // no default constructors
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
MICAcceleratorMatrixDIA<ValueType>::MICAcceleratorMatrixDIA(const Paralution_Backend_Descriptor local_backend) {

  this->mat_.val = NULL;
  this->mat_.offset = NULL;  
  this->mat_.num_diag = 0 ;
  this->set_backend(local_backend); 

}


template <typename ValueType>
MICAcceleratorMatrixDIA<ValueType>::~MICAcceleratorMatrixDIA() {

  this->Clear();

}

template <typename ValueType>
void MICAcceleratorMatrixDIA<ValueType>::info(void) const {

  LOG_INFO("MICAcceleratorMatrixDIA<ValueType> diag=" << this->get_ndiag() << " nnz=" << this->get_nnz() );

}

template <typename ValueType>
void MICAcceleratorMatrixDIA<ValueType>::AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag) {

  assert(nnz >= 0);
  assert(ncol >= 0);
  assert(nrow >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nnz > 0) {

    assert(ndiag > 0);


    allocate_mic(nnz, &this->mat_.val);
    allocate_mic(ndiag, &this->mat_.offset);
 
    set_to_zero_mic(nnz, mat_.val);
    
    set_to_zero_mic(ndiag, mat_.offset);

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nnz;
    this->mat_.num_diag = ndiag;

  }

}

template <typename ValueType>
void MICAcceleratorMatrixDIA<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_mic(&this->mat_.val);
    free_mic(&this->mat_.offset);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;
    this->mat_.num_diag = 0 ;

  }


}

template <typename ValueType>
void MICAcceleratorMatrixDIA<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixDIA<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to MIC copy
  if ((cast_mat = dynamic_cast<const HostMatrixDIA<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDIA(cast_mat->get_nnz(), cast_mat->get_nrow(), cast_mat->get_ncol(), cast_mat->get_ndiag());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      copy_to_mic(cast_mat->mat_.val, this->mat_.val, this->get_nnz());
      copy_to_mic(cast_mat->mat_.offset, this->mat_.offset, this->mat_.num_diag);

      /*
      // TODO

      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.val[j] = cast_mat->mat_.val[j];
      
      for (int j=0; j<this->mat_.num_diag; ++j)
        this->mat_.offset[j] = cast_mat->mat_.offset[j];
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
void MICAcceleratorMatrixDIA<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixDIA<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // MIC to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixDIA<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateDIA(this->get_nnz(), this->get_nrow(), this->get_ncol(), this->get_ndiag());

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) {

      copy_to_host(this->mat_.val, cast_mat->mat_.val, this->get_nnz());
      copy_to_host(this->mat_.offset, cast_mat->mat_.offset, this->mat_.num_diag);

      /*
      // TODO

      for (int j=0; j<this->get_nnz(); ++j)
        cast_mat->mat_.val[j] = this->mat_.val[j];
      
      for (int j=0; j<this->mat_.num_diag; ++j)
        cast_mat->mat_.offset[j] = this->mat_.offset[j];
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
void MICAcceleratorMatrixDIA<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const MICAcceleratorMatrixDIA<ValueType> *mic_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // MIC to MIC copy
  if ((mic_cast_mat = dynamic_cast<const MICAcceleratorMatrixDIA<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDIA(mic_cast_mat->get_nnz(), mic_cast_mat->get_nrow(), mic_cast_mat->get_ncol(), mic_cast_mat->get_ndiag());

    assert((this->get_nnz()  == src.get_nnz())  &&
	   (this->get_nrow() == src.get_nrow()) &&
	   (this->get_ncol() == src.get_ncol()) );

    if (this->get_nnz() > 0) {

      copy_mic_mic(mic_cast_mat->mat_.val, this->mat_.val, this->get_nnz());
      copy_mic_mic(mic_cast_mat->mat_.offset, this->mat_.offset, this->mat_.num_diag);

      /*
      // TODO
      for (int j=0; j<this->get_nnz(); ++j)
        this->mat_.val[j] = mic_cast_mat->mat_.val[j];
      
      for (int j=0; j<this->mat_.num_diag; ++j)
        this->mat_.offset[j] = mic_cast_mat->mat_.offset[j];
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
void MICAcceleratorMatrixDIA<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  MICAcceleratorMatrixDIA<ValueType> *mic_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // MIC to MIC copy
  if ((mic_cast_mat = dynamic_cast<MICAcceleratorMatrixDIA<ValueType>*> (dst)) != NULL) {

    mic_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    mic_cast_mat->AllocateDIA(mic_cast_mat->get_nnz(), mic_cast_mat->get_nrow(), mic_cast_mat->get_ncol(), mic_cast_mat->get_ndiag());

    assert((this->get_nnz()  == dst->get_nnz())  &&
	   (this->get_nrow() == dst->get_nrow()) &&
	   (this->get_ncol() == dst->get_ncol()) );

    if (this->get_nnz() > 0) { 

      copy_mic_mic(this->mat_.val, mic_cast_mat->mat_.val, this->get_nnz());
      copy_mic_mic(this->mat_.offset, mic_cast_mat->mat_.offset, this->mat_.num_diag);

      /*
      // TODO

      for (int j=0; j<this->get_nnz(); ++j)
        mic_cast_mat->mat_.val[j] = this->mat_.val[j];
      
      for (int j=0; j<this->mat_.num_diag; ++j)
        mic_cast_mat->mat_.offset[j] = this->mat_.offset[j];

      FATAL_ERROR(__FILE__, __LINE__);
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
bool MICAcceleratorMatrixDIA<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const MICAcceleratorMatrixDIA<ValueType>   *cast_mat_dia;
  
  if ((cast_mat_dia = dynamic_cast<const MICAcceleratorMatrixDIA<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_dia);
      return true;

  }

  return false;

}

template <typename ValueType>
void MICAcceleratorMatrixDIA<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if (this->get_nnz() > 0) {
    
    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());
    
    
    const MICAcceleratorVector<ValueType> *cast_in = dynamic_cast<const MICAcceleratorVector<ValueType>*> (&in) ; 
    MICAcceleratorVector<ValueType> *cast_out      = dynamic_cast<      MICAcceleratorVector<ValueType>*> (out) ; 
    
    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    spmv_dia(this->mat_.offset,
	     this->mat_.val,
	     this->get_nrow(),
	     this->get_ndiag(),
	     cast_in->vec_,
	     cast_out->vec_);

  }
}

template <typename ValueType>
void MICAcceleratorMatrixDIA<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
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

   spmv_add_dia(this->mat_.offset,
		this->mat_.val,
		this->get_nrow(),
		this->get_ndiag(),
		scalar,
		cast_in->vec_,
		cast_out->vec_);
   
  }
}


template class MICAcceleratorMatrixDIA<double>;
template class MICAcceleratorMatrixDIA<float>;

}
