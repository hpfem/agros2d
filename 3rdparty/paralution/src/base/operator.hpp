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


#ifndef PARALUTION_OPERATOR_HPP_
#define PARALUTION_OPERATOR_HPP_

#include "base_paralution.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

namespace paralution {

template <typename ValueType>
class GlobalVector;
template <typename ValueType>
class LocalVector;

/// Operator class defines the generic interface
/// for applying an operator (e.g. matrix, stencil)
/// from/to global and local vectors
template <typename ValueType>
class Operator : public BaseParalution<ValueType> {
  
public:

  Operator();
  virtual ~Operator();

  /// Apply the operator, out = Operator(in), where in, out are global vectors
  virtual void Apply(const GlobalVector<ValueType> &in, GlobalVector<ValueType> *out) const; 

  /// Apply and add the operator, out = out + scalar*Operator(in), where in, out are global vectors
  virtual void ApplyAdd(const GlobalVector<ValueType> &in, const ValueType scalar, 
                        GlobalVector<ValueType> *out) const; 

  /// Apply the operator, out = Operator(in), where in, out are local vectors
  virtual void Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const; 

  /// Apply and add the operator, out = out + scalar*Operator(in), where in, out are local vectors
  virtual void ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, 
                        LocalVector<ValueType> *out) const; 
  
};


}

#endif // PARALUTION_OPERTOR_HPP_

