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



#ifndef PARALUTION_GPU_MATRIX_CSR_HPP_
#define PARALUTION_GPU_MATRIX_CSR_HPP_

#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../matrix_formats.hpp"

#include <cuda.h>
#include <cusparse_v2.h>

namespace paralution {

template <typename ValueType>
class GPUAcceleratorMatrixCSR : public GPUAcceleratorMatrix<ValueType> {
  
public:

  GPUAcceleratorMatrixCSR();
  GPUAcceleratorMatrixCSR(const Paralution_Backend_Descriptor local_backend);
  virtual ~GPUAcceleratorMatrixCSR();

  virtual void info(void) const;
  virtual unsigned int get_mat_format(void) const { return CSR; }

  virtual void Clear(void);
  virtual void Zeros(void);

  virtual void AllocateCSR(const int nnz, const int nrow, const int ncol);
  virtual void SetDataPtrCSR(int **row_offset, int **col, ValueType **val,
                             const int nnz, const int nrow, const int ncol);
  virtual void LeaveDataPtrCSR(int **row_offset, int **col, ValueType **val);

  virtual bool ConvertFrom(const BaseMatrix<ValueType> &mat);

  virtual void CopyFrom(const BaseMatrix<ValueType> &mat);
  virtual void CopyFromAsync(const BaseMatrix<ValueType> &mat);
  virtual void CopyTo(BaseMatrix<ValueType> *mat) const;
  virtual void CopyToAsync(BaseMatrix<ValueType> *mat) const;

  virtual void CopyFromHost(const HostMatrix<ValueType> &src);
  virtual void CopyFromHostAsync(const HostMatrix<ValueType> &src);
  virtual void CopyToHost(HostMatrix<ValueType> *dst) const;
  virtual void CopyToHostAsync(HostMatrix<ValueType> *dst) const;

  virtual bool Permute(const BaseVector<int> &permutation);

  virtual bool Scale(const ValueType alpha);
  virtual bool ScaleDiagonal(const ValueType alpha);
  virtual bool ScaleOffDiagonal(const ValueType alpha);
  virtual bool AddScalar(const ValueType alpha);
  virtual bool AddScalarDiagonal(const ValueType alpha);
  virtual bool AddScalarOffDiagonal(const ValueType alpha);

  virtual bool ExtractSubMatrix(const int row_offset,
                                const int col_offset,
                                const int row_size,
                                const int col_size,
                                BaseMatrix<ValueType> *mat) const;

  virtual bool ExtractDiagonal(BaseVector<ValueType> *vec_diag) const;
  virtual bool ExtractInverseDiagonal(BaseVector<ValueType> *vec_inv_diag) const;
  virtual bool ExtractL(BaseMatrix<ValueType> *L) const;
  virtual bool ExtractLDiagonal(BaseMatrix<ValueType> *L) const;

  virtual bool ExtractU(BaseMatrix<ValueType> *U) const;
  virtual bool ExtractUDiagonal(BaseMatrix<ValueType> *U) const;
  
  virtual bool MaximalIndependentSet(int &size,
                                     BaseVector<int> *permutation) const;
  virtual bool MultiColoring(int &num_colors,
                             int **size_colors,
                             BaseVector<int> *permutation) const;

  virtual bool DiagonalMatrixMult(const BaseVector<ValueType> &diag);

  virtual void SymbolicPower(const int p);

  virtual bool MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  virtual void SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  virtual void SymbolicMatMatMult(const BaseMatrix<ValueType> &src);

  virtual bool MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                         const ValueType beta, const bool structure);

  virtual bool ILU0Factorize(void);

  virtual bool ICFactorize(BaseVector<ValueType> *inv_diag = NULL);


  virtual void LUAnalyse(void);
  virtual void LUAnalyseClear(void);
  virtual bool LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 

  virtual void LLAnalyse(void);
  virtual void LLAnalyseClear(void);
  virtual bool LLSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;
  virtual bool LLSolve(const BaseVector<ValueType> &in, const BaseVector<ValueType> &inv_diag,
                       BaseVector<ValueType> *out) const;

  virtual void LAnalyse(const bool diag_unit=false);
  virtual void LAnalyseClear(void);
  virtual bool LSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 
  
  virtual void UAnalyse(const bool diag_unit=false);
  virtual void UAnalyseClear(void);
  virtual bool USolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 


  virtual bool Gershgorin(ValueType &lambda_min,
                          ValueType &lambda_max) const;

  virtual void Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 
  virtual void ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                        BaseVector<ValueType> *out) const; 

  virtual bool Compress(const ValueType drop_off);

  virtual bool Transpose(void);

private:
  
  MatrixCSR<ValueType, int> mat_;

  friend class GPUAcceleratorMatrixDIA<ValueType>;
  friend class GPUAcceleratorMatrixELL<ValueType>;
  friend class GPUAcceleratorMatrixHYB<ValueType>;

  friend class BaseVector<ValueType>;  
  friend class AcceleratorVector<ValueType>;  
  friend class GPUAcceleratorVector<ValueType>;  

  cusparseSolveAnalysisInfo_t L_mat_info_;
  cusparseSolveAnalysisInfo_t U_mat_info_;
  cusparseMatDescr_t L_mat_descr_;
  cusparseMatDescr_t U_mat_descr_;
  cusparseMatDescr_t mat_descr_;

  GPUAcceleratorVector<ValueType> *tmp_vec_;

};


}

#endif // PARALUTION_GPU_MATRIX_CSR_HPP_

