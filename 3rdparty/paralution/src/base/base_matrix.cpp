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

#include "base_matrix.hpp"
#include "base_vector.hpp"
#include "backend_manager.hpp"
#include "../utils/log.hpp"

#include <assert.h>
#include <stdlib.h>


namespace paralution {

template <typename ValueType>
BaseMatrix<ValueType>::BaseMatrix() {

  this->nrow_ = 0;
  this->ncol_ = 0;
  this->nnz_  = 0;

}

template <typename ValueType>
BaseMatrix<ValueType>::~BaseMatrix() {
}


template <typename ValueType>
void BaseMatrix<ValueType>::CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val) {

  LOG_INFO("CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a CSR matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::CopyToCSR(int *row_offsets, int *col, ValueType *val) const {

  LOG_INFO("CopyToCSR(int *row_offsets, int *col, ValueType *val) const");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a CSR matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::CopyFromCOO(const int *row, const int *col, const ValueType *val) {

  LOG_INFO("CopyFromCOO(const int *row, const int *col, const ValueType *val)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a COO matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateCSR(const int nnz, const int nrow, const int ncol) {

  LOG_INFO("AllocateCSR(const int nnz, const int nrow, const int ncol)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a CSR matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateCOO(const int nnz, const int nrow, const int ncol) {

  LOG_INFO("AllocateCOO(const int nnz, const int nrow, const int ncol)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a COO matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag) {

  LOG_INFO("AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a DIA matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row) {

  LOG_INFO("AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a ELL matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateHYB(const int ell_nnz, const int coo_nnz, const int ell_max_row,
                                        const int nrow, const int ncol) {

  LOG_INFO("AllocateHYB(const int ell_nnz, const int coo_nnz, const int ell_max_row, const int nrow, const int ncol)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a HYB matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateDENSE(const int nrow, const int ncol) {

  LOG_INFO("AllocateDENSE(const int nrow, const int ncol)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a DENSE matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AllocateMCSR(const int nnz, const int nrow, const int ncol) {

  LOG_INFO("AllocateMCSR(const int nnz, const int nrow, const int ncol)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("This is NOT a MCSR matrix");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
void BaseMatrix<ValueType>::ReadFileMTX(const std::string filename) {

  LOG_INFO("BaseMatrix<ValueType>::ReadFileMTX(const std::string)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::WriteFileMTX(const std::string filename) const {

  LOG_INFO("BaseMatrix<ValueType>::WriteFileMTX(const std::string)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
bool BaseMatrix<ValueType>::ExtractDiagonal(BaseVector<ValueType> *vec_diag) const {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::ExtractInverseDiagonal(BaseVector<ValueType> *vec_inv_diag) const {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::ExtractSubMatrix(const int row_offset,
                                             const int col_offset,
                                             const int row_size,
                                             const int col_size,
                                             BaseMatrix<ValueType> *mat) const {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::LLSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::ILU0Factorize(void) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::ILUTFactorize(const ValueType t, const int maxrow) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::IC0Factorize(void) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::Permute(const BaseVector<int> &permutation) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::PermuteBackward(const BaseVector<int> &permutation) {
  return false;
}


template <typename ValueType>
bool BaseMatrix<ValueType>::MultiColoring(int &num_colors,
                                          int **size_colors,
                                          BaseVector<int> *permutation) const {
  return false;
}

template <typename ValueType>
void BaseMatrix<ValueType>::MaximalIndependentSet(int &size,
                                                  BaseVector<int> *permutation) const {

  LOG_INFO("MaximalIndependentSet(int &size, BaseVector<int> *permutation) const");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
void BaseMatrix<ValueType>::SymbolicPower(const int p) {

  LOG_INFO("BaseMatrix<ValueType>::SymbolicPower(const int p)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &src) {

  LOG_INFO("BaseMatrix<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &src)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::ILUpFactorizeNumeric(const int p, const BaseMatrix<ValueType> &mat) {

  LOG_INFO("BaseMatrix<ValueType>::ILUpFactorizeNumeric(const int p, const BaseMatrix<ValueType> &mat)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
bool BaseMatrix<ValueType>::MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                                      const ValueType beta, const bool structure) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::Gershgorin(ValueType &lambda_min,
                                       ValueType &lambda_max) const {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::Scale(const ValueType alpha) {
  return false;
}


template <typename ValueType>
bool BaseMatrix<ValueType>::ScaleDiagonal(const ValueType alpha) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::ScaleOffDiagonal(const ValueType alpha) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::AddScalar(const ValueType alpha) {
  return false;

}

template <typename ValueType>
bool BaseMatrix<ValueType>::AddScalarDiagonal(const ValueType alpha) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::AddScalarOffDiagonal(const ValueType alpha) {
  return false;
}

template <typename ValueType>
void BaseMatrix<ValueType>::LUAnalyse(void) {

  LOG_INFO("BaseMatrix<ValueType>::LUAnalyse(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::LUAnalyseClear(void) {

  LOG_INFO("BaseMatrix<ValueType>::LUAnalyseClear(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::LLAnalyse(void) {

  LOG_INFO("BaseMatrix<ValueType>::LLAnalyse(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::LLAnalyseClear(void) {

  LOG_INFO("BaseMatrix<ValueType>::LLAnalyseClear(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::LAnalyse(const bool diag_unit) {

  LOG_INFO("BaseMatrix<ValueType>::LAnalyse(const bool diag_unit=false)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::LAnalyseClear(void) {

  LOG_INFO("BaseMatrix<ValueType>::LAnalyseClear(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
bool BaseMatrix<ValueType>::LSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}

template <typename ValueType>
void BaseMatrix<ValueType>::UAnalyse(const bool diag_unit) {

  LOG_INFO("BaseMatrix<ValueType>::UAnalyse(const bool diag_unit=false)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::UAnalyseClear(void) {

  LOG_INFO("BaseMatrix<ValueType>::UAnalyseClear(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
bool BaseMatrix<ValueType>::USolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {
  return false;
}

template <typename ValueType>
void BaseMatrix<ValueType>::NumericMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  LOG_INFO("BaseMatrix<ValueType>::NumericMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
void BaseMatrix<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {

  LOG_INFO("BaseMatrix<ValueType>::SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AMGConnect(const ValueType eps, BaseVector<int> *connections) const {

  LOG_INFO("BaseMatrix<ValueType>::AMGConnect(const ValueType eps, BaseVector<int> *connections) const");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AMGAggregate(const BaseVector<int> &connections, BaseVector<int> *aggregates) const {

  LOG_INFO("BaseMatrix<ValueType>::AMGAggregate(const std::vector<bool> &connections, BaseVector<int> *aggregates) const");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AMGSmoothedAggregation(const ValueType relax,
                                                   const BaseVector<int> &aggregates,
                                                   const BaseVector<int> &connections,
                                                         BaseMatrix<ValueType> *prolong,
                                                         BaseMatrix<ValueType> *restrict) const {

  LOG_INFO("BaseMatrix<ValueType>::AMGSmoothedAggregation(const ValueType relax, const BaseVector<int> &aggregates, const BaseVector<int> &connections, BaseMatrix<ValueType> *prolong, BaseMatrix>ValueType> *restrict) const");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::AMGAggregation(const BaseVector<int> &aggregates,
                                                 BaseMatrix<ValueType> *prolong,
                                                 BaseMatrix<ValueType> *restrict) const {

  LOG_INFO("BaseMatrix<ValueType>::AMGAggregation(const BaseVector<int> &aggregates, BaseMatrix<ValueType> *prolong, BaseMatrix>ValueType> *restrict) const");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
bool BaseMatrix<ValueType>::DiagMatMult(const BaseVector<ValueType> &diag) {
  return false;
}

template <typename ValueType>
bool BaseMatrix<ValueType>::MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B) {
  return false;
}


template <typename ValueType>
bool BaseMatrix<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {
  return false;
}



template <typename ValueType>
void BaseMatrix<ValueType>::Zeros(void) {

  LOG_INFO("BaseMatrix<ValueType>::Zeros(void)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
bool BaseMatrix<ValueType>::Compress(const ValueType drop_off) {
  return false;
}


template <typename ValueType>
bool BaseMatrix<ValueType>::Transpose(void) {
  return false;
}

template <typename ValueType>
void BaseMatrix<ValueType>::SetDataPtrCOO(int **row, int **col, ValueType **val,
                                          const int nnz, const int nrow, const int ncol) {

  LOG_INFO("BaseMatrix<ValueType>::SetDataPtrCOO(...)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)! Check the backend?");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void BaseMatrix<ValueType>::SetDataPtrCSR(int **row_offset, int **col, ValueType **val,
                                          const int nnz, const int nrow, const int ncol) {

  LOG_INFO("BaseMatrix<ValueType>::SetDataPtrCSR(...)");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)! Check the backend?");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
bool BaseMatrix<ValueType>::CreateFromMap(const BaseVector<int> &map, const int n, const int m) {
  return false;
}


  /*


template <typename ValueType>
void BaseMatrix<ValueType>::

  LOG_INFO("BaseMatrix<ValueType>::");
  LOG_INFO("Matrix format=" << _matrix_format_names[this->get_mat_format()]);
  this->info();
  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}


   */



template <typename ValueType>
void BaseMatrix<ValueType>::set_backend(const Paralution_Backend_Descriptor local_backend) {

  this->local_backend_ = local_backend;

}


  //TODO print also parameters info?

template <typename ValueType>
HostMatrix<ValueType>::HostMatrix() {
}

template <typename ValueType>
HostMatrix<ValueType>::~HostMatrix() {
}






template <typename ValueType>
AcceleratorMatrix<ValueType>::AcceleratorMatrix() {
}

template <typename ValueType>
AcceleratorMatrix<ValueType>::~AcceleratorMatrix() {
}




template <typename ValueType>
GPUAcceleratorMatrix<ValueType>::GPUAcceleratorMatrix() {
}

template <typename ValueType>
GPUAcceleratorMatrix<ValueType>::~GPUAcceleratorMatrix() {
}




template <typename ValueType>
OCLAcceleratorMatrix<ValueType>::OCLAcceleratorMatrix() {
}

template <typename ValueType>
OCLAcceleratorMatrix<ValueType>::~OCLAcceleratorMatrix() {
}



template class BaseMatrix<double>;
template class BaseMatrix<float>;

template class HostMatrix<double>;
template class HostMatrix<float>;

template class AcceleratorMatrix<double>;
template class AcceleratorMatrix<float>;

template class GPUAcceleratorMatrix<double>;
template class GPUAcceleratorMatrix<float>;

template class OCLAcceleratorMatrix<double>;
template class OCLAcceleratorMatrix<float>;

}
