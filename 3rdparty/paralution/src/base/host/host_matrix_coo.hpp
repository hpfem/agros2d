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


#ifndef PARALUTION_HOST_MATRIX_COO_HPP_
#define PARALUTION_HOST_MATRIX_COO_HPP_

#include "../base_vector.hpp"
#include "../base_matrix.hpp"
#include "../matrix_formats.hpp"

namespace paralution {

template <typename ValueType>
class HostMatrixCOO : public HostMatrix<ValueType> {
  
public:

  HostMatrixCOO();
  HostMatrixCOO(const Paralution_Backend_Descriptor local_backend);
  virtual ~HostMatrixCOO();

  virtual void info(void) const;
  virtual unsigned int get_mat_format(void) const { return  COO; }

  virtual void Clear(void);
  virtual void AllocateCOO(const int nnz, const int nrow, const int ncol);

  virtual void SetDataPtrCOO(int **row, int **col, ValueType **val,
                             const int nnz, const int nrow, const int ncol);
  virtual void LeaveDataPtrCOO(int **row, int **col, ValueType **val);

  virtual bool Scale(const ValueType alpha);
  virtual bool ScaleDiagonal(const ValueType alpha);
  virtual bool ScaleOffDiagonal(const ValueType alpha);
  virtual bool AddScalar(const ValueType alpha);
  virtual bool AddScalarDiagonal(const ValueType alpha);
  virtual bool AddScalarOffDiagonal(const ValueType alpha);

  virtual bool ConvertFrom(const BaseMatrix<ValueType> &mat);

  virtual bool Permute(const BaseVector<int> &permutation);
  virtual bool PermuteBackward(const BaseVector<int> &permutation);

  virtual void CopyFromCOO(const int *row, const int *col, const ValueType *val);

  virtual void CopyFrom(const BaseMatrix<ValueType> &mat);
  virtual void CopyTo(BaseMatrix<ValueType> *mat) const;

  virtual void ReadFileMTX(const std::string);
  virtual void WriteFileMTX(const std::string) const;

  virtual void Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const; 
  virtual void ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar, 
                        BaseVector<ValueType> *out) const; 
  
private:
  
  MatrixCOO<ValueType, int> mat_;

  friend class BaseVector<ValueType>;  
  friend class HostVector<ValueType>;  
  friend class HostMatrixCSR<ValueType>;
  friend class HostMatrixDIA<ValueType>;
  friend class HostMatrixELL<ValueType>;
  friend class HostMatrixHYB<ValueType>;  
  friend class HostMatrixDENSE<ValueType>;

  friend class GPUAcceleratorMatrixCOO<ValueType>;
  friend class OCLAcceleratorMatrixCOO<ValueType>;
  friend class MICAcceleratorMatrixCOO<ValueType>;

};


}

#endif // PARALUTION_HOST_MATRIX_COO_HPP_
