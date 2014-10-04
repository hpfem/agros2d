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


#ifndef PARALUTION_PRECONDITIONER_MULTICOLORED_GS_HPP_
#define PARALUTION_PRECONDITIONER_MULTICOLORED_GS_HPP_

#include "../solver.hpp"
#include "preconditioner.hpp"
#include "preconditioner_multicolored.hpp"
#include "../../base/local_vector.hpp"

#include <vector>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
class MultiColoredSGS : public MultiColored<OperatorType, VectorType, ValueType> {

public:

  MultiColoredSGS();
  virtual ~MultiColoredSGS();

  virtual void Print(void) const;  

  virtual void ReBuildNumeric(void);

  /// Set the relaxation parameter for the SOR/SSOR scheme
  virtual void SetRelaxation(const ValueType omega);
  
protected:

  virtual void PostAnalyse_(void);

  virtual void SolveL_(void);
  virtual void SolveD_(void);
  virtual void SolveR_(void);
  virtual void Solve_(const VectorType &rhs,
                      VectorType *x);  

  ValueType omega_;

};

template <class OperatorType, class VectorType, typename ValueType>
class MultiColoredGS : public MultiColoredSGS<OperatorType, VectorType, ValueType> {

public:

  MultiColoredGS();
  virtual ~MultiColoredGS();

  virtual void Print(void) const;  

  
protected:

  virtual void PostAnalyse_(void);

  virtual void SolveL_(void);
  virtual void SolveD_(void);
  virtual void SolveR_(void);
  virtual void Solve_(const VectorType &rhs,
                      VectorType *x);
  
};


}

#endif // PARALUTION_PRECONDITIONER_MULTICOLORED_GS_HPP_

