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


#ifndef PARALUTION_GLOBAL_VECTOR_HPP_
#define PARALUTION_GLOBAL_VECTOR_HPP_

#include "vector.hpp"

namespace paralution {

template <typename ValueType>
class LocalMatrix;
template <typename ValueType>
class GlobalMatrix;
template <typename ValueType>
class LocalStencil;
template <typename ValueType>
class GlobalStencil;


// Global vector
template <typename ValueType>
class GlobalVector : public Vector<ValueType> {
  
public:

  GlobalVector();
  virtual ~GlobalVector();

  virtual void MoveToAccelerator(void);
  virtual void MoveToHost(void);

  virtual void info(void) const ;   
  virtual int get_size(void) const;

  virtual void Allocate(std::string name, const unsigned int size);
  virtual void Clear(void);
  virtual void Zeros(void);
  virtual void Ones(void);
  virtual void SetValues(const ValueType val);
  virtual void SetRandom(const ValueType a = -1.0, const ValueType b = 1.0, const int seed = 0);
  void CloneFrom(const GlobalVector<ValueType> &src);

  // Accessing operator - only for host data
  ValueType& operator[](const unsigned int i);
  const ValueType& operator[](const unsigned int i) const;

  virtual void CopyFrom(const GlobalVector<ValueType> &src);
  virtual void ReadFileASCII(const std::string filename);
  virtual void WriteFileASCII(const std::string filename) const;
  virtual void ReadFileBinary(const std::string filename);
  virtual void WriteFileBinary(const std::string filename) const;

  // this = this + alpha*x
  virtual void AddScale(const GlobalVector<ValueType> &x, const ValueType alpha);
  // this = alpha*this + x
  virtual void ScaleAdd(const ValueType alpha, const GlobalVector<ValueType> &x);
  // this = alpha*this
  virtual void Scale(const ValueType alpha);
  // this^T x
  virtual ValueType Dot(const GlobalVector<ValueType> &x) const;
  // srqt(this^T this)
  virtual ValueType Norm(void) const;
  // reduce
  virtual ValueType Reduce(void) const;
  // L1 norm, sum(|this|)
  virtual ValueType Asum(void) const;
  // Amax, max(|this|)
  virtual int Amax(ValueType &value) const;
  // point-wise multiplication
  virtual void PointWiseMult(const GlobalVector<ValueType> &x);
  virtual void PointWiseMult(const GlobalVector<ValueType> &x, const GlobalVector<ValueType> &y);

protected:

  virtual bool is_host(void) const {return true;};
  virtual bool is_accel(void) const {return true;};

private:

  int local_size_;
  int global_size_;

  LocalVector<ValueType> vector_interior_;
  LocalVector<ValueType> vector_ghost_;

  friend class LocalMatrix<ValueType>;  
  friend class GlobalMatrix<ValueType>;  
  friend class LocalStencil<ValueType>;  
  friend class GlobalStencil<ValueType>;  

};


}

#endif // PARALUTION_GLOBAL_VECTOR_HPP_

