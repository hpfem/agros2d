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


#ifndef PARALUTION_HOST_MATRIX_DENSE_HPP_
#define PARALUTION_HOST_MATRIX_DENSE_HPP_

#include "../base_vector.hpp"
#include "../base_matrix.hpp"
#include "../matrix_formats.hpp"

namespace paralution {

template <typename ValueType>
class HostMatrixDENSE : public HostMatrix<ValueType> {

public:

  HostMatrixDENSE();
  HostMatrixDENSE(const Paralution_Backend_Descriptor local_backend);
  virtual ~HostMatrixDENSE();

  virtual void info(void) const;
  virtual unsigned int get_mat_format(void) const { return  DENSE; }

  virtual void Clear(void);
  virtual void AllocateDENSE(const int nrow, const int ncol);
  virtual void SetDataPtrDENSE(ValueType **val, const int nrow, const int ncol);
  virtual void LeaveDataPtrDENSE(ValueType **val);

  virtual bool ConvertFrom(const BaseMatrix<ValueType> &mat);

  virtual void CopyFrom(const BaseMatrix<ValueType> &mat);
  virtual void CopyTo(BaseMatrix<ValueType> *mat) const;

  virtual void Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;
  virtual void ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                        BaseVector<ValueType> *out) const;

  virtual void Householder(const int idx, ValueType &beta, BaseVector<ValueType> *vec);
  virtual void QRDecompose(void);
  virtual void QRSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;

  virtual void LUFactorize(void);
  virtual bool LUSolve(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const;

  virtual void Invert(void);

private:

  MatrixDENSE<ValueType> mat_;

  friend class BaseVector<ValueType>;
  friend class HostVector<ValueType>;
  friend class HostMatrixCSR<ValueType>;
  friend class HostMatrixCOO<ValueType>;
  friend class HostMatrixELL<ValueType>;
  friend class HostMatrixHYB<ValueType>;
  friend class HostMatrixDIA<ValueType>;

  friend class GPUAcceleratorMatrixDENSE<ValueType>;
  friend class OCLAcceleratorMatrixDENSE<ValueType>;
  friend class MICAcceleratorMatrixDENSE<ValueType>;

};


}

#endif // PARALUTION_HOST_MATRIX_DENSE_HPP_
