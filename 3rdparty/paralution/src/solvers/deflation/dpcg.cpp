// *************************************************************************
//
//    This code is developed and maintained by TU Delft. 
//    The deflation solver (files src/solvers/deflation/dpcg.hpp and
//    src/solvers/deflation/dpcg.cpp) are released under GNU LESSER GENERAL 
//    PUBLIC LICENSE (LGPL v3)
//
//    Copyright (C) 2013 Kees Vuik (TU Delft)
//    Delft University of Technology, the Netherlands
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as 
//    published by the Free Software Foundation, either version 3 of the 
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public 
//    License along with this program.  
//    If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include "dpcg.hpp"
#include "../iter_ctrl.hpp"

#include "../../base/global_matrix.hpp"
#include "../../base/local_matrix.hpp"

#include "../../base/global_stencil.hpp"
#include "../../base/local_stencil.hpp"

#include "../../base/global_vector.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"

#include <assert.h>
#include <math.h>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
DPCG<OperatorType, VectorType, ValueType>::DPCG() {

  LOG_DEBUG(this, "DPCG::DPCG()",
            "default constructor");

  this->novecni_ = 2; 

}

template <class OperatorType, class VectorType, typename ValueType>
DPCG<OperatorType, VectorType, ValueType>::~DPCG() {

  LOG_DEBUG(this, "DPCG::~DPCG()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::Print(void) const {
  
  if (this->precond_ == NULL) { 
    
    LOG_INFO("DPCG solver");
    
  } else {
    
    LOG_INFO("PDPCG solver, with preconditioner:");
    this->precond_->Print();

  }

  
}


template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("DPCG (non-precond) linear solver starts");

  } else {

    LOG_INFO("PDPCG solver starts, with preconditioner:");
    this->precond_->Print();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("DPCG (non-precond) ends");

  } else {

    LOG_INFO("PDPCG ends");

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::SetNVectors(const int novecni) {

  LOG_DEBUG(this, "DPCG::SetNVectors()",
            novecni);

  assert(novecni > 0);
  this->novecni_ = novecni;

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::MakeZ_COO(void) {

  LOG_DEBUG(this, "DPCG::MakeZ_COO()",
            "");

  int ncol = novecni_*novecni_*novecni_ - 1; 
  int nrow = this->op_->get_nrow();
  int part = nrow / (ncol+1);
  int nrows, ncols, j, k, didx_x, didx_y, didx_z;
  int s, gridarea2d, griddiminx, dom_idx_x, dom_idx_y, dom_idx_z, domarea2d, domsize;
 
  int numvecs = ncol;
  nrows= nrow;
  ncols= numvecs;	
  int xdim=64;
  part=nrows/(numvecs+1); 
  this->Z_.ConvertToCOO();
  this->Z_.AllocateCOO("Z",nrows-part,nrows,ncols);
  
  int *Z_row = NULL;
  int *Z_col = NULL;
  ValueType *Z_val = NULL;
  
  this->Z_.LeaveDataPtrCOO(&Z_row, &Z_col, &Z_val);
  
  s=xdim/novecni_;
  domsize=s*s*s;  griddiminx=xdim/s; gridarea2d=griddiminx*griddiminx; domarea2d=s*s;
  for(j=0;j<numvecs;j++){
    didx_x=(j)%griddiminx;    didx_y=(j%gridarea2d)/griddiminx; didx_z=(j)/gridarea2d;
    for(k=0;k<domsize;k++){
      dom_idx_x=(k)%s;      dom_idx_y=(k%domarea2d)/s;	dom_idx_z=(k)/domarea2d;
      Z_row[j*domsize+k]=didx_x*s+didx_y*griddiminx*domarea2d+didx_z*gridarea2d*domsize+ 
        //we are at the first ele of the right block
        //now within the block we have to position
        dom_idx_x+dom_idx_y*xdim+dom_idx_z*xdim*xdim;
      Z_col[j*domsize+k]=j; 
      Z_val[j*domsize+k] = 1.0f;
    }
  }
  
  this->Z_.SetDataPtrCOO(&Z_row, &Z_col, &Z_val, "Z", nrows-part, nrow, ncol);
  this->Z_.ConvertToCSR();
  
}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::MakeZ_CSR(void) {

  LOG_DEBUG(this, "DPCG::MakeZ_CSR()",
            "");

  int ncol = novecni_*novecni_*novecni_ - 1; 
  int nrow = this->op_->get_nrow();
  int nrows, ncols, part, i,j,column;
  int s, cntr=0;
  int didx, didy, didz;
  
  int numvecs = ncol;
  nrows= nrow;
  ncols= numvecs;	
  int xdim=64;
  part=nrows/(numvecs+1); 
  this->Z_.ConvertToCSR();
  this->Z_.AllocateCSR("Z",nrows-part,nrows,ncols);
  
  int *Z_row_offset = NULL;
  int *Z_col = NULL;
  ValueType *Z_val = NULL;
  
  this->Z_.LeaveDataPtrCSR(&Z_row_offset, &Z_col, &Z_val);
  
  s=xdim/novecni_;
  
  for(i=0,j=0;i<nrows;i++){
    // find z index
    didx=(i%xdim)/s;   didy=((i/(xdim*s))%novecni_);    didz=i/(xdim*xdim*s);
    column=didz*novecni_*novecni_+didy*novecni_+didx;
    //LOG_INFO("indices"<<didx<<" "<<didy<<" "<<didz);
    //if(didx==novecni_-1&&didy==novecni_-1&&didz==novecni_-1)
    if(column==numvecs)
      {
        Z_row_offset[i+1]=Z_row_offset[i];
        cntr++;
      }//cout<<endl;
    else
      {
        Z_col[j]=column;
        Z_val[j] = 1.0f;
        Z_row_offset[i+1]=Z_row_offset[i]+1;
        j++;
      }
  }
  //LOG_INFO("Number of non-zeros in Z are "<<j<<" nrows-part is "<<nrows-part<<"discarded"<<cntr<<" should be equal to"<<nrows-part);
  Z_row_offset[i]=nrows-part;
  
  this->Z_.SetDataPtrCSR(&Z_row_offset, &Z_col, &Z_val, "Z", nrows-part, nrow, ncol);
  
}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::Build(void) {
  
  LOG_DEBUG(this, "DPCG::Build()",
            this->build_ <<
            " #*# begin");


  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());
  assert(this->op_->get_nrow() > 0);

  int ncol = novecni_*novecni_*novecni_ - 1; 

  this->r_.CloneBackend(*this->op_);
  this->r_.Allocate("r", this->op_->get_ncol());

  this->p_.CloneBackend(*this->op_);
  this->p_.Allocate("p", this->op_->get_nrow());
  
  this->q_.CloneBackend(*this->op_);
  this->q_.Allocate("q", this->op_->get_nrow());

  this->w_.CloneBackend(*this->op_);
  this->w_.Allocate("w", this->op_->get_nrow());

  this->hat_.CloneBackend(*this->op_);
  this->hat_.Allocate("hat", ncol);

  this->intmed_.CloneBackend(*this->op_);
  this->intmed_.Allocate("intmed", ncol);

  this->Qb_.CloneBackend(*this->op_);
  this->Qb_.Allocate("Qb", this->op_->get_nrow());

  this->Ptx_.CloneBackend(*this->op_);
  this->Ptx_.Allocate("Ptx", this->op_->get_nrow());

  this->LLtx_.CloneBackend(*this->op_);
  this->LLtx_.Allocate("LLtx", this->op_->get_nrow());

  this->LLtx2_.CloneBackend(*this->op_);
  this->LLtx2_.Allocate("LLtx2", this->op_->get_nrow());

  this->Dinv_.CloneBackend(*this->op_);

  this->op_->ExtractInverseDiagonal(&this->Dinv_);

  this->L_.CloneBackend(*this->op_);
  this->LT_.CloneBackend(*this->op_);

  this->op_->ExtractL(&this->L_, false);
  this->L_.DiagonalMatrixMult(this->Dinv_);

  this->LT_.CopyFrom(this->L_);
  this->LT_.Transpose();

  //  this->MakeZ_COO();
  this->MakeZ_CSR();

  this->E_.CloneBackend(*this->op_);
  this->Z_.CloneBackend(*this->op_);
  this->ZT_.CloneBackend(*this->op_);
  this->AZ_.CloneBackend(*this->op_);
  this->AZT_.CloneBackend(*this->op_);

  this->ZT_.CopyFrom(this->Z_);
  this->ZT_.Transpose();

  this->AZ_.MatrixMult(*this->op_, this->Z_);

  this->AZT_.CopyFrom(this->AZ_);
  this->AZT_.Transpose();

  this->E_.MatrixMult(this->ZT_, this->AZ_);

  // The Invert() should be on the host
  this->E_.MoveToHost();

  this->E_.ConvertToDENSE();
  this->E_.Invert();

  // E_ goes back to the original backend
  this->E_.CloneBackend(*this->op_);

  LOG_DEBUG(this, "DPCG::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "DPCG::Clear()",
            this->build_);

  if (this->build_ == true) {

    
    this->r_.Clear();
    this->w_.Clear();
    this->p_.Clear();
    this->q_.Clear();

    this->Dinv_.Clear();
    this->hat_.Clear();
    this->intmed_.Clear();
    this->Qb_.Clear();
    this->Ptx_.Clear();
    this->LLtx_.Clear();
    this->LLtx2_.Clear();
    
    this->ZT_.Clear();
    this->E_.Clear();
    this->AZ_.Clear();
    this->L_.Clear();
    this->LT_.Clear();
    this->Z_.Clear();
    this->AZT_.Clear();
    
    this->iter_ctrl_.Clear();
    
    this->build_ = false;
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "DPCG::MoveToHostLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->r_.MoveToHost();
    this->w_.MoveToHost();
    this->p_.MoveToHost();
    this->q_.MoveToHost();

    this->Dinv_.MoveToHost();
    this->hat_.MoveToHost();
    this->intmed_.MoveToHost();
    this->Qb_.MoveToHost();
    this->Ptx_.MoveToHost();
    this->LLtx_.MoveToHost();
    this->LLtx2_.MoveToHost();
    
    this->ZT_.MoveToHost();
    this->E_.MoveToHost();
    this->AZ_.MoveToHost();
    this->L_.MoveToHost();
    this->LT_.MoveToHost();
    this->Z_.MoveToHost();
    this->AZT_.MoveToHost();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "DPCG::MoveToAcceleratorLocalData__()",
            this->build_);


  if (this->build_ == true) {

    this->r_.MoveToAccelerator();
    this->w_.MoveToAccelerator();
    this->p_.MoveToAccelerator();
    this->q_.MoveToAccelerator();

    this->Dinv_.MoveToAccelerator();
    this->hat_.MoveToAccelerator();
    this->intmed_.MoveToAccelerator();
    this->Qb_.MoveToAccelerator();
    this->Ptx_.MoveToAccelerator();
    this->LLtx_.MoveToAccelerator();
    this->LLtx2_.MoveToAccelerator();
    
    this->ZT_.MoveToAccelerator();
    this->E_.MoveToAccelerator();
    this->AZ_.MoveToAccelerator();
    this->L_.MoveToAccelerator();
    this->LT_.MoveToAccelerator();
    this->Z_.MoveToAccelerator();
    this->AZT_.MoveToAccelerator();


  }

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::SolveNonPrecond_(const VectorType &rhs,
                                                              VectorType *x) {

  LOG_DEBUG(this, "DPCG::SolveNonPrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_  == NULL);
  assert(this->build_ == true);

  const OperatorType *op = this->op_;

  VectorType *r = &this->r_;
  VectorType *p = &this->p_;
  VectorType *q = &this->q_;
  VectorType *w = &this->w_;

  VectorType *Dinv = &this->Dinv_;
  VectorType *hat = &this->hat_;
  VectorType *intmed = &this->intmed_;
  VectorType *Qb = &this->Qb_;
  VectorType *Ptx = &this->Ptx_;
  VectorType *LLtx = &this->LLtx_;
  VectorType *LLtx2 = &this->LLtx2_;

  OperatorType *ZT = &this->ZT_;
  OperatorType *E = &this->E_;
  OperatorType *AZ = &this->AZ_;
  OperatorType *L = &this->L_;
  OperatorType *LT = &this->LT_;
  OperatorType *Z = &this->Z_;
  OperatorType *AZT = &this->AZT_;
 
  ValueType alpha, beta;
  ValueType rho, rho_old;
  ValueType res_norm = 0.0, b_norm = 1.0;
  ValueType init_residual = 0.0;
  ValueType check_residual = 0.0;

  // initial residual = b - Ax
  op->Apply(*x, r); 
  r->ScaleAdd(ValueType(-1.0), rhs);

  // initial residual for the interation control
  // = |res|
  //  init_residual = this->Norm(*r);


  // Z^{T}r
  ZT->Apply(*r, hat);
  E->Apply(*hat, intmed);
  AZ->Apply(*intmed, w);

  // r = r - w
  r->AddScale(*w, ValueType(-1.0));



  // initial residual for the interation control
  // = |res| / |b|
  res_norm = this->Norm(*r);
  b_norm = this->Norm(rhs);
  init_residual = res_norm / b_norm ;

  this->iter_ctrl_.InitResidual(init_residual);

  //Apply preconditioning w=M^{-1}r
  L->Apply(*r, LLtx);
  L->Apply(*LLtx, LLtx2);
  LLtx->AddScale(*LLtx2, ValueType(-1.0));
  w->CopyFrom(*r, 0, 0, this->op_->get_nrow());

  // (I-LD^{-1}+(LD^{-1})^{2})r_{i-1}
  w->AddScale(*LLtx, ValueType(-1.0));
  w->PointWiseMult(*Dinv);
  LT->Apply(*w, LLtx);
  LT->Apply(*LLtx, LLtx2);
  w->ScaleAdd2(ValueType(1.0), *LLtx, ValueType(-1.0), *LLtx2, ValueType(1.0));

  // rho = (r,w)
  rho = r->Dot(*w);

  p->CopyFrom(*w, 0, 0, this->op_->get_nrow());

  // q = Ap
  op->Apply(*p, q);

  // Z^Tq
  ZT->Apply(*q, hat);
  E->Apply(*hat, intmed);
  AZ->Apply(*intmed, w);

  // q = q - w
  q->AddScale(*w, ValueType(-1.0));

  // alpha = rho / (p,q)
  alpha=rho/p->Dot(*q);

  // x = x + alpha * p
  x->AddScale(*p, alpha);

  // x = x + alpha * q
  r->AddScale(*q, alpha*ValueType(-1.0));

  res_norm = this->Norm(*r);
  check_residual = res_norm / b_norm ; 

  while (!this->iter_ctrl_.CheckResidual(check_residual)) {

    //Apply preconditioning w=M^{-1}r
    L->Apply(*r, LLtx);
    L->Apply(*LLtx, LLtx2);
    LLtx->AddScale(*LLtx2, ValueType(-1.0));
    w->CopyFrom(*r, 0, 0, this->op_->get_nrow());

    // (I-LD^{-1}+(LD^{-1})^{2})r_{i-1}
    w->AddScale(*LLtx, ValueType(-1.0));
    w->PointWiseMult(*Dinv);
    LT->Apply(*w, LLtx);
    LT->Apply(*LLtx, LLtx2);
    w->ScaleAdd2(ValueType(1.0), *LLtx, ValueType(-1.0), *LLtx2, ValueType(1.0));

    rho_old = rho;

    // rho = (r,w)
    rho = r->Dot(*w);

    beta = rho / rho_old;

    // p = p + beta * w
    p->ScaleAdd(beta, *w);

    // q = Ap
    op->Apply(*p, q);

    // Z^Tq
    ZT->Apply(*q, hat);
    E->Apply(*hat, intmed);
    AZ->Apply(*intmed, w);

    // q = q - w
    q->AddScale(*w, ValueType(-1.0));

    // alpha = rho / (p,q)
    alpha=rho/p->Dot(*q);

    // x = x + alpha * p
    x->AddScale(*p, alpha);

    // x = x + alpha * q
    r->AddScale(*q, alpha*ValueType(-1.0));

    res_norm = this->Norm(*r);
    check_residual = res_norm / b_norm ; 
  }

  // correct solution
  // Qb
  // Z^{T}rhs
  ZT->Apply(rhs, hat);
  // E^{-1}hat
  E->Apply(*hat, intmed);
  Z->Apply(*intmed, Qb);

  // Ptx
  // AZ^{T}x
  AZT->Apply(*x, hat);
  // E^{-1}hat
  E->Apply(*hat, intmed);
  Z->Apply(*intmed, Ptx);
  x->AddScale(*Ptx, ValueType(-1.0));
  x->AddScale(*Qb, ValueType(1.0));

  LOG_DEBUG(this, "DPCG::SolveNonPrecond_()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void DPCG<OperatorType, VectorType, ValueType>::SolvePrecond_(const VectorType &rhs,
                                                            VectorType *x) {

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_ != NULL);
  assert(this->build_ == true);

  FATAL_ERROR(__FILE__, __LINE__);

}



template class DPCG< LocalMatrix<double>, LocalVector<double>, double >;
template class DPCG< LocalMatrix<float>,  LocalVector<float>, float >;

}

