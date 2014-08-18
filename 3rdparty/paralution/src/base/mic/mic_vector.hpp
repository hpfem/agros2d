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


#ifndef PARALUTION_MIC_VECTOR_HPP_
#define PARALUTION_MIC_VECTOR_HPP_

#include "../base_vector.hpp"
#include "../base_matrix.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"


namespace paralution {

template <typename ValueType>
class MICAcceleratorVector : public AcceleratorVector<ValueType> {
  
public:

  MICAcceleratorVector();
  MICAcceleratorVector(const Paralution_Backend_Descriptor local_backend);
  virtual ~MICAcceleratorVector();

  virtual void info(void) const;

  virtual void Allocate(const int n);
  virtual void SetDataPtr(ValueType **ptr, const int size);
  virtual void LeaveDataPtr(ValueType **ptr);
  virtual void Clear(void);
  virtual void Zeros(void);
  virtual void Ones(void);
  virtual void SetValues(const ValueType val);

  virtual void CopyFrom(const BaseVector<ValueType> &src) ;
  virtual void CopyFrom(const BaseVector<ValueType> &src,
                        const int src_offset,
                        const int dst_offset,
                        const int size);


  virtual void CopyTo(BaseVector<ValueType> *dst) const;

  virtual void CopyFromHost(const HostVector<ValueType> &src);
  virtual void CopyToHost(HostVector<ValueType> *dst) const;

  virtual void CopyFromPermute(const BaseVector<ValueType> &src,
                               const BaseVector<int> &permutation);
  virtual void CopyFromPermuteBackward(const BaseVector<ValueType> &src,
                                       const BaseVector<int> &permutation);

  virtual void Permute(const BaseVector<int> &permutation);
  virtual void PermuteBackward(const BaseVector<int> &permutation);  


  // this = this + alpha*x
  virtual void AddScale(const BaseVector<ValueType> &x, const ValueType alpha);
  // this = alpha*this + x
  virtual void ScaleAdd(const ValueType alpha, const BaseVector<ValueType> &x);
  // this = alpha*this + x*beta
  virtual void ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta);
  virtual void ScaleAddScale(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta,
                             const int src_offset, const int dst_offset,const int size);
  // this = alpha*this + x*beta + y*gamma
  virtual void ScaleAdd2(const ValueType alpha, const BaseVector<ValueType> &x, const ValueType beta, const BaseVector<ValueType> &y, const ValueType gamma) ;
  // this = alpha*this
  virtual void Scale(const ValueType alpha);
  /// Compute partial sum
  virtual void PartialSum(const BaseVector<ValueType> &x);
  // this^T x
  virtual ValueType Dot(const BaseVector<ValueType> &x) const;
  // srqt(this^T this)
  virtual ValueType Norm(void) const;
  // reduce
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

  friend class MICAcceleratorVector<float>;
  friend class MICAcceleratorVector<double>;
  friend class MICAcceleratorVector<int>;

  friend class HostVector<ValueType>;
  friend class AcceleratorMatrix<ValueType>;

  friend class MICAcceleratorMatrixCSR<ValueType>;
  friend class MICAcceleratorMatrixMCSR<ValueType>;
  friend class MICAcceleratorMatrixBCSR<ValueType>;
  friend class MICAcceleratorMatrixCOO<ValueType>;
  friend class MICAcceleratorMatrixDIA<ValueType>;
  friend class MICAcceleratorMatrixELL<ValueType>;
  friend class MICAcceleratorMatrixDENSE<ValueType>;
  friend class MICAcceleratorMatrixHYB<ValueType>;

  friend class MICAcceleratorMatrixCOO<double>;
  friend class MICAcceleratorMatrixCOO<float>;

  friend class MICAcceleratorMatrixCSR<double>;
  friend class MICAcceleratorMatrixCSR<float>;

};




};

#endif // PARALUTION_BASE_VECTOR_HPP_
