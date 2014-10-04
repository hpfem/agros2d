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


#ifndef PARALUTION_GLOBAL_MATRIX_HPP_
#define PARALUTION_GLOBAL_MATRIX_HPP_

#include "operator.hpp"
#include "global_vector.hpp"


namespace paralution {

template <typename ValueType>
class GlobalVector;
template <typename ValueType>
class LocalVector;

template <typename ValueType>
class LocalMatrix;


// Global Matrix
template <typename ValueType>
class GlobalMatrix : public Operator<ValueType> {
  
public:

  GlobalMatrix();
  virtual ~GlobalMatrix();

  virtual int get_nrow(void) const { return 0;};
  virtual int get_ncol(void) const { return 0;};


  virtual void CopyFrom(const GlobalMatrix<ValueType>&);
  
  virtual void Apply(const GlobalVector<ValueType> &in, GlobalVector<ValueType> *out) const; 
  virtual void ApplyAdd(const GlobalVector<ValueType> &in, const ValueType scalar, 
                        GlobalVector<ValueType> *out) const; 

protected:

  virtual bool is_host(void) const {return true;};
  virtual bool is_accel(void) const {return true;};

private:

  int local_nrow_, local_ncol_, local_nnz_;
  int global_nrow_, global_ncol_, global_nnz_;

  friend class GlobalVector<ValueType>;  
  friend class LocalMatrix<ValueType>;  
  friend class LocalVector<ValueType>;  
  
};


}

#endif // PARALUTION_GLOBAL_MATRIX_HPP_

