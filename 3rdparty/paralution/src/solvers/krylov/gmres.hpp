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


#ifndef PARALUTION_GMRES_GMRES_HPP_
#define PARALUTION_GMRES_GMRES_HPP_

#include "../solver.hpp"

#include <vector>

namespace paralution {


template <class OperatorType, class VectorType, typename ValueType>
class GMRES : public IterativeLinearSolver<OperatorType, VectorType, ValueType> {
  
public:

  GMRES();
  virtual ~GMRES();

  virtual void Print(void) const;

  virtual void Build(void);
  virtual void Clear(void);

  /// Set the size of the Krylov-space basis
  virtual void SetBasisSize(const int size_basis);

protected:

  virtual void SolveNonPrecond_(const VectorType &rhs,
                                VectorType *x);
  virtual void SolvePrecond_(const VectorType &rhs,
                             VectorType *x);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);

  void ApplyGivensRotation_(const ValueType &c, const ValueType &s,
                                  ValueType &x,       ValueType &y) const;

  void BackSubstitute_(std::vector<ValueType> &g,
                       const std::vector<ValueType> &H,
                       int k) const;

private:

  VectorType z_, w_;
  VectorType **v_;

  int size_basis_;

};


}

#endif // PARALUTION_GMRES_GMRES_HPP_

