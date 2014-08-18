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


#ifndef PARALUTION_PRECONDITIONER_MULTICOLORED_ILU_HPP_
#define PARALUTION_PRECONDITIONER_MULTICOLORED_ILU_HPP_

#include "../solver.hpp"
#include "preconditioner.hpp"
#include "preconditioner_multicolored.hpp"
#include "../../base/local_vector.hpp"

#include <vector>

namespace paralution {

/// ILU(p,q) preconditioner (see power(q)-pattern method, D. Lukarski "Parallel Sparse Linear 
/// Algebra for Multi-core and Many-core Platforms - Parallel Solvers and Preconditioners", 
/// PhD Thesis, 2012, KIT) 
template <class OperatorType, class VectorType, typename ValueType>
class MultiColoredILU : public MultiColored<OperatorType, VectorType, ValueType> {

public:

  MultiColoredILU();
  virtual ~MultiColoredILU();

  virtual void Print(void) const;  

  virtual void ReBuildNumeric(void);
  
  /// Initialize a multi-colored ILU(p,p+1) preconditioner
  virtual void Set(const int p);

  /// Initialize a multi-colored ILU(p,q) preconditioner;
  /// level==true will perform the factorization with levels; 
  /// level==false will perform the factorization only on the power(q)-pattern 
  virtual void Set(const int p, const int q, const bool level=true);

  
protected:

  virtual void Build_Analyser_(void); 
  virtual void Factorize_(void);
  virtual void PostAnalyse_(void);

  virtual void SolveL_(void);
  virtual void SolveD_(void);
  virtual void SolveR_(void);
  virtual void Solve_(const VectorType &rhs,
                      VectorType *x);

  int q_;
  int p_;
  bool level_;
  int nnz_;
  
};


}

#endif // PARALUTION_PRECONDITIONER_MULTICOLORED_ILU_HPP_

