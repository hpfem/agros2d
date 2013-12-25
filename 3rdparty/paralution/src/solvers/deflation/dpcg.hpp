// *************************************************************************
//
//    This code is developed and maintained by TU Delft. 
//    The deflation solver (files src/solvers/deflation/dpcg.hpp and
//    src/solvers/deflation/dpcg.cpp) are released under GNU LESSER GENERAL 
//    PUBLIC LICENSE (LGPL v3)
//
//    Copyright (C) 2013 Kees Vuik (TU Delft)
//    Delft University of Technology, the Netherlands
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as 
//    published by the Free Software Foundation, either version 3 of the 
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public 
//    License along with this program.  
//    If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#ifndef PARALUTION_DEFLATION_DPCG_HPP_
#define PARALUTION_DEFLATION_DPCG_HPP_

#include "../solver.hpp"

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
class DPCG : public IterativeLinearSolver<OperatorType, VectorType, ValueType> {
  
public:

  DPCG();
  virtual ~DPCG();

  virtual void Print(void) const;

  virtual void Build(void);
  virtual void Clear(void);

  virtual void SetNVectors(const int novecni);

protected:
  virtual void SolveNonPrecond_(const VectorType &rhs,
                                VectorType *x);

  virtual void SolvePrecond_(const VectorType &rhs,
                             VectorType *x);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);

private:

  void MakeZ_COO(void);
  void MakeZ_CSR(void);

  OperatorType L_, LT_;
  OperatorType AZ_, ZT_;
  OperatorType Z_, AZT_;
  OperatorType E_;

  VectorType r_, w_;
  VectorType p_, q_;

  VectorType Dinv_;
  VectorType hat_, intmed_;
  VectorType Qb_, Ptx_;
  VectorType LLtx_, LLtx2_;

  int novecni_;
};


}

#endif // PARALUTION_DEFLATION_DPCG_HPP_

