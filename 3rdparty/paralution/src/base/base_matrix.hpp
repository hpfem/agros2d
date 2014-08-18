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


#ifndef PARALUTION_BASE_MATRIX_HPP_
#define PARALUTION_BASE_MATRIX_HPP_

#include "base_vector.hpp"
#include "matrix_formats.hpp"
#include <vector>

namespace paralution {
  
// Forward declartions
template <typename ValueType>
class HostMatrix;
template <typename ValueType>
class HostMatrixCSR;
template <typename ValueType>
class HostMatrixCOO;
template <typename ValueType>
class HostMatrixDIA;
template <typename ValueType>
class HostMatrixELL;
template <typename ValueType>
class HostMatrixHYB;
template <typename ValueType>
class HostMatrixDENSE;
template <typename ValueType>
class HostMatrixMCSR;
template <typename ValueType>
class HostMatrixBCSR;

template <typename ValueType>
class AcceleratorMatrix;

template <typename ValueType>
class GPUAcceleratorMatrix;
template <typename ValueType>
class GPUAcceleratorMatrixCSR;
template <typename ValueType>
class GPUAcceleratorMatrixMCSR;
template <typename ValueType>
class GPUAcceleratorMatrixBCSR;
template <typename ValueType>
class GPUAcceleratorMatrixCOO;
template <typename ValueType>
class GPUAcceleratorMatrixDIA;
template <typename ValueType>
class GPUAcceleratorMatrixELL;
template <typename ValueType>
class GPUAcceleratorMatrixHYB;
template <typename ValueType>
class GPUAcceleratorMatrixDENSE;

template <typename ValueType>
class OCLAcceleratorMatrix;
template <typename ValueType>
class OCLAcceleratorMatrixCSR;
template <typename ValueType>
class OCLAcceleratorMatrixMCSR;
template <typename ValueType>
class OCLAcceleratorMatrixBCSR;
template <typename ValueType>
class OCLAcceleratorMatrixCOO;
template <typename ValueType>
class OCLAcceleratorMatrixDIA;
template <typename ValueType>
class OCLAcceleratorMatrixELL;
template <typename ValueType>
class OCLAcceleratorMatrixHYB;
template <typename ValueType>
class OCLAcceleratorMatrixDENSE;

template <typename ValueType>
class MICAcceleratorMatrix;
template <typename ValueType>
class MICAcceleratorMatrixCSR;
template <typename ValueType>
class MICAcceleratorMatrixMCSR;
template <typename ValueType>
class MICAcceleratorMatrixBCSR;
template <typename ValueType>
class MICAcceleratorMatrixCOO;
template <typename ValueType>
class MICAcceleratorMatrixDIA;
template <typename ValueType>
class MICAcceleratorMatrixELL;
template <typename ValueType>
class MICAcceleratorMatrixHYB;
template <typename ValueType>
class MICAcceleratorMatrixDENSE;

/// Base class for all host/accelerator matrices
template <typename ValueType>
class BaseMatrix {
  
public:

  BaseMatrix();
  virtual ~BaseMatrix();

  /// Return the number of rows in the matrix
  int get_nrow(void) const;
  /// Return the number of columns in the matrix
  int get_ncol(void) const;
  /// Return the non-zeros of the matrix
  int get_nnz(void) const;
  /// Shows simple info about the object
  virtual void info(void) const = 0;
  /// Return the matrix format id (see matrix_formats.hpp)
  virtual unsigned int get_mat_format(void) const = 0 ;
  /// Copy the backend descriptor information
  virtual void set_backend(const Paralution_Backend_Descriptor local_backend);

  virtual bool Check(void) const;

  /// Allocate CSR Matrix
  virtual void AllocateCSR(const int nnz, const int nrow, const int ncol);
  /// Allocate MCSR Matrix
  virtual void AllocateMCSR(const int nnz, const int nrow, const int ncol);
  /// Allocate COO Matrix
  virtual void AllocateCOO(const int nnz, const int nrow, const int ncol);
  /// Allocate DIA Matrix
  virtual void AllocateDIA(const int nnz, const int nrow, const int ncol, const int ndiag);
  /// Allocate ELL Matrix
  virtual void AllocateELL(const int nnz, const int nrow, const int ncol, const int max_row);
  /// Allocate HYB Matrix
  virtual void AllocateHYB(const int ell_nnz, const int coo_nnz, const int ell_max_row, 
                           const int nrow, const int ncol);
  /// Allocate DENSE Matrix
  virtual void AllocateDENSE(const int nrow, const int ncol);

  /// Initialize a COO matrix on the Host with externally allocated data
  virtual void SetDataPtrCOO(int **row, int **col, ValueType **val,
                             const int nnz, const int nrow, const int ncol);
  /// Leave a COO matrix to Host pointers
  virtual void LeaveDataPtrCOO(int **row, int **col, ValueType **val);

  /// Initialize a CSR matrix on the Host with externally allocated data
  virtual void SetDataPtrCSR(int **row_offset, int **col, ValueType **val,
                             const int nnz, const int nrow, const int ncol);
  /// Leave a CSR matrix to Host pointers
  virtual void LeaveDataPtrCSR(int **row_offset, int **col, ValueType **val);

  /// Initialize a DENSE matrix on the Host with externally allocated data
  virtual void SetDataPtrDENSE(ValueType **val, const int nrow, const int ncol);
  /// Leave a DENSE matrix to Host pointers
  virtual void LeaveDataPtrDENSE(ValueType **val);

  /// Clear (free) the matrix
  virtual void Clear(void) = 0;

  /// Set all the values to zero
  virtual void Zeros(void);

  /// Assembling
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

  /// Scale all values
  virtual bool Scale(const ValueType alpha);
  /// Scale the diagonal entries of the matrix with alpha
  virtual bool ScaleDiagonal(const ValueType alpha);
  /// Scale the off-diagonal entries of the matrix with alpha
  virtual bool ScaleOffDiagonal(const ValueType alpha);
  /// Add alpha to all values
  virtual bool AddScalar(const ValueType alpha);
  /// Add alpha to the diagonal entries of the matrix
  virtual bool AddScalarDiagonal(const ValueType alpha);
  /// Add alpha to the off-diagonal entries of the matrix
  virtual bool AddScalarOffDiagonal(const ValueType alpha);

  /// Extrat a sub-matrix with row/col_offset and row/col_size
  virtual bool ExtractSubMatrix(const int row_offset,
                                const int col_offset,
                                const int row_size,
                                const int col_size,
                                BaseMatrix<ValueType> *mat) const;
  
  /// Extract the diagonal values of the matrix into a LocalVector
  virtual bool ExtractDiagonal(BaseVector<ValueType> *vec_diag) const;
  /// Extract the inverse (reciprocal) diagonal values of the matrix into a LocalVector
  virtual bool ExtractInverseDiagonal(BaseVector<ValueType> *vec_inv_diag) const;
  /// Extract the upper triangular matrix
  virtual bool ExtractU(BaseMatrix<ValueType> *U) const;
  /// Extract the upper triangular matrix including diagonal
  virtual bool ExtractUDiagonal(BaseMatrix<ValueType> *U) const;
  /// Extract the lower triangular matrix
  virtual bool ExtractL(BaseMatrix<ValueType> *L) const;
  /// Extract the lower triangular matrix including diagonal
  virtual bool ExtractLDiagonal(BaseMatrix<ValueType> *L) const;

  /// Perform (forward) permutation of the matrix
  virtual bool Permute(const BaseVector<int> &permutation);

  /// Perform (backward) permutation of the matrix
  virtual bool PermuteBackward(const BaseVector<int> &permutation);

  /// Create permutation vector for CMK reordering of the matrix
  virtual bool CMK(BaseVector<int> *permutation) const;
  /// Create permutation vector for reverse CMK reordering of the matrix
  virtual bool RCMK(BaseVector<int> *permutation) const;

  /// Perform multi-coloring decomposition of the matrix; Returns number of 
  /// colors, the corresponding sizes (the array is allocated in the function) 
  /// and the permutation
  virtual bool MultiColoring(int &num_colors,
                             int **size_colors,
                             BaseVector<int> *permutation) const;

  /// Perform maximal independent set decomposition of the matrix; Returns the 
  /// size of the maximal independent set and the corresponding permutation
  virtual bool MaximalIndependentSet(int &size,
                                     BaseVector<int> *permutation) const;

  /// Return a permutation for saddle-point problems (zero diagonal entries),
  /// where all zero diagonal elements are mapped to the last block;
  /// the return size is the size of the first block
  virtual void ZeroBlockPermutation(int &size,
                                    BaseVector<int> *permutation) const;
  
  /// Convert the matrix from another matrix (with different structure)
  virtual bool ConvertFrom(const BaseMatrix<ValueType> &mat) = 0;

  /// Copy from another matrix
  virtual void CopyFrom(const BaseMatrix<ValueType> &mat) = 0;

  /// Copy to another matrix
  virtual void CopyTo(BaseMatrix<ValueType> *mat) const = 0;

  /// Async copy from another matrix
  virtual void CopyFromAsync(const BaseMatrix<ValueType> &mat);

  /// Copy to another matrix
  virtual void CopyToAsync(BaseMatrix<ValueType> *mat) const;

  /// Copy from CSR array (the matrix has to be allocated)
  virtual void CopyFromCSR(const int *row_offsets, const int *col, const ValueType *val);

  /// Copy to CSR array (the arrays have to be allocated)
  virtual void CopyToCSR(int *row_offsets, int *col, ValueType *val) const;

  /// Copy from COO array (the matrix has to be allocated)
  virtual void CopyFromCOO(const int *row, const int *col, const ValueType *val);

  /// Create a restriction matrix operator based on an int vector map
  virtual bool CreateFromMap(const BaseVector<int> &map, const int n, const int m);

  /// Read matrix from MTX (Matrix Market Format) file
  virtual void ReadFileMTX(const std::string filename);
  /// Write matrix to MTX (Matrix Market Format) file
  virtual void WriteFileMTX(const std::string filename) const;

  /// Read matrix from CSR (PARALUTION binary format) file
  virtual void ReadFileCSR(const std::string filename);
  /// Write matrix to CSR (PARALUTION binary format) file
  virtual void WriteFileCSR(const std::string filename) const;

  /// Perform symbolic computation (structure only) of |this|^p
  virtual void SymbolicPower(const int p);

  /// Perform symbolic matrix-matrix multiplication (i.e. determine the structure), 
  /// this = this*src
  virtual void SymbolicMatMatMult(const BaseMatrix<ValueType> &src);
  /// Multiply two matrices, this = A * B
  virtual bool MatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  /// Perform symbolic matrix-matrix multiplication (i.e. determine the structure), 
  /// this = A*B
  virtual void SymbolicMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  /// Perform numerical matrix-matrix multiplication (i.e. value computation), 
  /// this = A*B
  virtual void NumericMatMatMult(const BaseMatrix<ValueType> &A, const BaseMatrix<ValueType> &B);
  /// Multiply the matrix with diagonal matrix (stored in LocalVector), 
  /// this=this*diag
  virtual bool DiagonalMatrixMult(const BaseVector<ValueType> &diag);
  /// Perform matrix addition, this = alpha*this + beta*mat; 
  /// if structure==false the structure of the matrix is not changed, 
  /// if structure==true new data structure is computed
  virtual bool MatrixAdd(const BaseMatrix<ValueType> &mat, const ValueType alpha, 
                         const ValueType beta, const bool structure);

  /// Perform ILU(0) factorization
  virtual bool ILU0Factorize(void);
  /// Perform LU factorization
  virtual void LUFactorize(void);
  /// Perform ILU(t,m) factorization based on threshold and maximum
  /// number of elements per row
  virtual bool ILUTFactorize(const ValueType t, const int maxrow);
  /// Perform ILU(p) factorization based on power (see power(q)-pattern method, D. Lukarski 
  ///  "Parallel Sparse Linear Algebra for Multi-core and Many-core Platforms - Parallel Solvers and 
  /// Preconditioners", PhD Thesis, 2012, KIT) 
  virtual void ILUpFactorizeNumeric(const int p, const BaseMatrix<ValueType> &mat);

  /// Perform IC(0) factorization
  virtual bool ICFactorize(BaseVector<ValueType> *inv_diag);

  /// Analyse the structure (level-scheduling)
  virtual void LUAnalyse(void);
  /// Delete the analysed data (see LUAnalyse)
  virtual void LUAnalyseClear(void);
  /// Solve LU out = in; if level-scheduling algorithm is provided then the graph 
  /// traversing is performed in parallel
  virtual bool LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 

  /// Analyse the structure (level-scheduling)
  virtual void LLAnalyse(void);
  /// Delete the analysed data (see LLAnalyse)
  virtual void LLAnalyseClear(void);
  /// Solve LL^T out = in; if level-scheduling algorithm is provided then the graph 
  // traversing is performed in parallel
  virtual bool LLSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;
  virtual bool LLSolve(const BaseVector<ValueType> &in, const BaseVector<ValueType> &inv_diag,
                       BaseVector<ValueType> *out) const;

  /// Analyse the structure (level-scheduling) L-part
  /// diag_unit == true the diag is 1;
  /// diag_unit == false the diag is 0;
  virtual void LAnalyse(const bool diag_unit=false);
  /// Delete the analysed data (see LAnalyse) L-party
  virtual void LAnalyseClear(void);
  /// Solve L out = in; if level-scheduling algorithm is provided then the 
  /// graph traversing is performed in parallel
  virtual bool LSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;

  /// Analyse the structure (level-scheduling) U-part;
  /// diag_unit == true the diag is 1;
  /// diag_unit == false the diag is 0;
  virtual void UAnalyse(const bool diag_unit=false);
  /// Delete the analysed data (see UAnalyse) U-party
  virtual void UAnalyseClear(void);
  /// Solve U out = in; if level-scheduling algorithm is provided then the 
  /// graph traversing is performed in parallel
  virtual bool USolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 

  /// Compute Householder vector
  virtual void Householder(const int idx, ValueType &beta, BaseVector<ValueType> *vec);
  /// QR Decomposition
  virtual void QRDecompose(void);
  /// Solve QR out = in
  virtual void QRSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;

  /// Invert this
  virtual void Invert(void);

  /// Compute the spectrum approximation with Gershgorin circles theorem
  virtual bool Gershgorin(ValueType &lambda_min,
                          ValueType &lambda_max) const;

  /// Apply the matrix to vector, out = this*in;
  virtual void Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const = 0; 
  /// Apply and add the matrix to vector, out = out + scalar*this*in;
  virtual void ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                        BaseVector<ValueType> *out) const = 0; 

  /// Delete all entries abs(a_ij) <= drop_off;
  /// the diagonal elements are never deleted
  virtual bool Compress(const ValueType drop_off);

  /// Transpose the matrix
  virtual bool Transpose(void);

  // TODO needs some return value - bool vector about connections?
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

  /// Factorized Sparse Approximate Inverse assembly for given system
  /// matrix power pattern or external sparsity pattern
  virtual bool FSAI(const int power, const BaseMatrix<ValueType> *pattern);

  /// SParse Approximate Inverse assembly for given system matrix pattern
  virtual void SPAI(void);

protected:
  
  /// Number of rows
  int nrow_;
  /// Number of columns
  int ncol_;
  /// Number of non-zero elements
  int nnz_;  

  /// Backend descriptor (local copy)
  Paralution_Backend_Descriptor local_backend_;

  friend class BaseVector<ValueType>;
  friend class HostVector<ValueType>;
  friend class AcceleratorVector<ValueType>;
  friend class GPUAcceleratorVector<ValueType>;
  friend class OCLAcceleratorVector<ValueType>;
  friend class MICAcceleratorVector<ValueType>;
  
};


template <typename ValueType>
class HostMatrix : public BaseMatrix<ValueType> {
  
public:

  HostMatrix();
  virtual ~HostMatrix();
  
};


template <typename ValueType>
class AcceleratorMatrix : public BaseMatrix<ValueType> {
  
public:
  AcceleratorMatrix();
  virtual ~AcceleratorMatrix();

  /// Copy (accelerator matrix) from host matrix
  virtual void CopyFromHost(const HostMatrix<ValueType> &src) = 0;

  /// Async copy (accelerator matrix) from host matrix
  virtual void CopyFromHostAsync(const HostMatrix<ValueType> &src);

  /// Copy (accelerator matrix) to host matrix
  virtual void CopyToHost(HostMatrix<ValueType> *dst) const = 0;

  /// Async opy (accelerator matrix) to host matrix
  virtual void CopyToHostAsync(HostMatrix<ValueType> *dst) const;

};



template <typename ValueType>
class GPUAcceleratorMatrix : public AcceleratorMatrix<ValueType> {

public: 

  GPUAcceleratorMatrix();
  virtual ~GPUAcceleratorMatrix();

};


template <typename ValueType>
class OCLAcceleratorMatrix : public AcceleratorMatrix<ValueType> {

public:

  OCLAcceleratorMatrix();
  virtual ~OCLAcceleratorMatrix();

};

template <typename ValueType>
class MICAcceleratorMatrix : public AcceleratorMatrix<ValueType> {

public: 

  MICAcceleratorMatrix();
  virtual ~MICAcceleratorMatrix();

};

}

#endif // PARALUTION_BASE_MATRIX_HPP_
