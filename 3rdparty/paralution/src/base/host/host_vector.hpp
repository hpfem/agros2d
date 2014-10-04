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


#ifndef PARALUTION_HOST_VECTOR_HPP_
#define PARALUTION_HOST_VECTOR_HPP_

#include "../base_vector.hpp"
#include "../base_matrix.hpp"

namespace paralution {

template <typename ValueType>
class LocalVector;

template <typename ValueType>
class HostVector : public BaseVector<ValueType> {

public:

  HostVector();
  HostVector(const Paralution_Backend_Descriptor local_backend);
  virtual ~HostVector();

  virtual void info(void) const;

  virtual bool Check(void) const;
  virtual void Allocate(const int n);
  virtual void SetDataPtr(ValueType **ptr, const int size);
  virtual void LeaveDataPtr(ValueType **ptr);
  virtual void Clear(void);
  virtual void Zeros(void);
  virtual void Ones(void);
  virtual void SetValues(const ValueType val);
  virtual void SetRandom(const ValueType a, const ValueType b, const int seed);
  virtual void Assemble(const int *i, const ValueType *v,
                        int size, const int n);

  virtual void CopyFrom(const BaseVector<ValueType> &vec);
  virtual void CopyFromFloat(const BaseVector<float> &vec);
  virtual void CopyFromDouble(const BaseVector<double> &vec);
  virtual void CopyTo(BaseVector<ValueType> *vec) const;
  virtual void CopyFrom(const BaseVector<ValueType> &src,
                        const int src_offset,
                        const int dst_offset,
                        const int size);
  virtual void CopyFromPermute(const BaseVector<ValueType> &src,
                               const BaseVector<int> &permutation);
  virtual void CopyFromPermuteBackward(const BaseVector<ValueType> &src,
                                       const BaseVector<int> &permutation);

  virtual void Permute(const BaseVector<int> &permutation);
  virtual void PermuteBackward(const BaseVector<int> &permutation);

  virtual bool Restriction(const BaseVector<ValueType> &vec_fine, const BaseVector<int> &map);
  virtual bool Prolongation(const BaseVector<ValueType> &vec_coarse, const BaseVector<int> &map);

  /// Read vector from ASCII file
  void ReadFileASCII(const std::string filename);
  /// Write vector to ASCII file
  void WriteFileASCII(const std::string filename) const;
  /// Read vector from binary file
  void ReadFileBinary(const std::string filename);
  /// Write vector to binary file
  void WriteFileBinary(const std::string filename) const;

  // this = this + alpha*x
  virtual void AddScale(const BaseVector<ValueType> &x, const ValueType alpha);
  // this = alpha*this + x
  virtual void ScaleAdd(const ValueType alpha, const BaseVector<ValueType> &x);
  // this = alpha*this + x*beta
  virtual void ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta);
  virtual void ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta,
                             const int src_offset, const int dst_offset,const int size);
  // this = alpha*this + x*beta + y*gamma
  virtual void ScaleAdd2(const ValueType alpha, const BaseVector<ValueType> &x,
                         const ValueType beta, const BaseVector<ValueType> &y,
                         const ValueType gamma);
  // this = alpha*this
  virtual void Scale(const ValueType alpha);
  // Compute partial sum
  virtual void PartialSum(const BaseVector<ValueType> &x);
  // this^T x
  virtual ValueType Dot(const BaseVector<ValueType> &x) const;
  // srqt(this^T this)
  virtual ValueType Norm(void) const;
  // reduce vector
  virtual ValueType Reduce(void) const;
  // Compute sum of absolute values of this
  virtual ValueType Asum(void) const;
  // Compute absolute value of this
  virtual int Amax(ValueType &value) const;
  // point-wise multiplication
  virtual void PointWiseMult(const BaseVector<ValueType> &x);
  virtual void PointWiseMult(const BaseVector<ValueType> &x, const BaseVector<ValueType> &y);

private:

  ValueType *vec_;

  // for [] operator in LocalVector
  friend class LocalVector<ValueType>;

  friend class HostVector<double>;
  friend class HostVector<float>;
  friend class HostVector<int>;

  friend class HostMatrix<ValueType>;
  friend class HostMatrixCSR<ValueType>;
  friend class HostMatrixCOO<ValueType>;
  friend class HostMatrixDIA<ValueType>;
  friend class HostMatrixELL<ValueType>;
  friend class HostMatrixHYB<ValueType>;
  friend class HostMatrixDENSE<ValueType>;
  friend class HostMatrixMCSR<ValueType>;
  friend class HostMatrixBCSR<ValueType>;

  friend class HostMatrixCOO<float>;
  friend class HostMatrixCOO<double>;

  friend class HostMatrixCSR<double>;
  friend class HostMatrixCSR<float>;

  friend class GPUAcceleratorVector<ValueType>;
  friend class OCLAcceleratorVector<ValueType>;
  friend class MICAcceleratorVector<ValueType>;

};


}

#endif // PARALUTION_HOST_VECTOR_HPP_
