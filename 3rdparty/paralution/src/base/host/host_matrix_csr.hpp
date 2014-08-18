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


#ifndef PARALUTION_HOST_MATRIX_CSR_HPP_
#define PARALUTION_HOST_MATRIX_CSR_HPP_

#include "../base_vector.hpp"
#include "../base_matrix.hpp"
#include "../matrix_formats.hpp"

namespace paralution {

template <typename ValueType>
class HostMatrixCSR : public HostMatrix<ValueType> {
  
public:

  HostMatrixCSR();
  HostMatrixCSR(const Paralution_Backend_Descriptor local_backend);
  virtual ~HostMatrixCSR();

  virtual void info(void) const;
  virtual unsigned int get_mat_format(void) const {  return CSR; }

  virtual bool Check(void) const;
  virtual void AllocateCSR(const int nnz, const int nrow, const int ncol);
  virtual void SetDataPtrCSR(int **row_offset, int **col, ValueType **val,
                             const int nnz, const int nrow, const int ncol);
  virtual void LeaveDataPtrCSR(int **row_offset, int **col, ValueType **val);

  virtual void Clear(void);
  virtual void Zeros(void);

  virtual void Assemble(const int *i, const int *j, const ValueType *v,
                        const int size, const int n, const int m,
                        int **pp_assembly_rank,
                        int **pp_assembly_irank,
                        int **pp_assembly_loop_start,
                        int **pp_assembly_loop_end,
                        int &nThreads);
  virtual void AssembleUpdate(const ValueType *v,
                              const int *assembly_rank,
                              const int *assembly_irank,
                              const int *assembly_loop_start,
                              const int *assembly_loop_end,
                              const int nThreads);

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
  virtual bool ExtractU(BaseMatrix<ValueType> *U) const;
  virtual bool ExtractUDiagonal(BaseMatrix<ValueType> *U) const;
  virtual bool ExtractL(BaseMatrix<ValueType> *L) const;
  virtual bool ExtractLDiagonal(BaseMatrix<ValueType> *L) const;
 
  virtual bool MultiColoring(int &num_colors,
                             int **size_colors,
                             BaseVector<int> *permutation) const;

  virtual bool MaximalIndependentSet(int &size,
                                     BaseVector<int> *permutation) const;
  
  virtual void ZeroBlockPermutation(int &size,
                                    BaseVector<int> *permutation) const;


  virtual void SymbolicPower(const int p);

  virtual void SymbolicMatMatMult(const BaseMatrix<ValueType> &src);
  virtual bool MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  virtual void SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  virtual void NumericMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);

  virtual bool DiagonalMatrixMult(const BaseVector<ValueType> &diag);

  virtual bool MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                         const ValueType beta, const bool structure);

  virtual bool Permute(const BaseVector<int> &permutation);

  virtual bool CMK(BaseVector<int> *permutation) const;
  virtual bool RCMK(BaseVector<int> *permutation) const;

  virtual bool ConvertFrom(const BaseMatrix<ValueType> &mat);

  virtual void CopyFrom(const BaseMatrix<ValueType> &mat);

  virtual void CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val);
  virtual void CopyToCSR(int *row_offsets, int *col, ValueType *val) const;

  virtual void CopyTo(BaseMatrix<ValueType> *mat) const;

  virtual void ReadFileCSR(const std::string);
  virtual void WriteFileCSR(const std::string) const;

  virtual bool CreateFromMap(const BaseVector<int> &map, const int n, const int m);

  virtual bool ICFactorize(BaseVector<ValueType> *inv_diag);

  virtual bool ILU0Factorize(void);
  virtual void ILUpFactorizeNumeric(const int p, const BaseMatrix<ValueType> &mat);
  virtual bool ILUTFactorize(const ValueType t, const int maxrow);

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

  virtual void AMGConnect(const ValueType eps, BaseVector<int> *connections) const;
  virtual void AMGAggregate(const BaseVector<int> &connections, BaseVector<int> *aggregates) const;
  virtual void AMGSmoothedAggregation(const ValueType relax,
                                      const BaseVector<int> &aggregates,
                                      const BaseVector<int> &connections,
                                            BaseMatrix<ValueType> *prolong,
                                            BaseMatrix<ValueType> *restrict) const;
  virtual void AMGAggregation(const BaseVector<int> &aggregates,
                                    BaseMatrix<ValueType> *prolong,
                                    BaseMatrix<ValueType> *restrict) const;

  virtual bool FSAI(const int power, const BaseMatrix<ValueType> *pattern);
  virtual void SPAI(void);

private:

  MatrixCSR<ValueType, int> mat_;

  friend class BaseVector<ValueType>;
  friend class HostVector<ValueType>;
  friend class HostMatrixCOO<ValueType>;
  friend class HostMatrixDIA<ValueType>;
  friend class HostMatrixELL<ValueType>;
  friend class HostMatrixHYB<ValueType>;
  friend class HostMatrixDENSE<ValueType>;
  friend class HostMatrixMCSR<ValueType>;
  friend class HostMatrixBCSR<ValueType>;

  friend class GPUAcceleratorMatrixCSR<ValueType>;
  friend class OCLAcceleratorMatrixCSR<ValueType>;
  friend class MICAcceleratorMatrixCSR<ValueType>;

#ifdef SUPPORT_MKL

  double *mkl_tmp_vec_;

#endif

  bool L_diag_unit_;
  bool U_diag_unit_;

};


}

#endif // PARALUTION_HOST_MATRIX_CSR_HPP_
