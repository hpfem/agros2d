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


#ifndef PARALUTION_MIXED_PRECISION_HPP_
#define PARALUTION_MIXED_PRECISION_HPP_

#include "solver.hpp"

namespace paralution {

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
class MixedPrecisionDC : public IterativeLinearSolver<OperatorTypeH, VectorTypeH, ValueTypeH> {
  
public:

  MixedPrecisionDC();
  virtual ~MixedPrecisionDC();

  virtual void Print(void) const;

  void Set(Solver<OperatorTypeL, VectorTypeL, ValueTypeL> &Solver_L);

  virtual void Build(void);
  virtual void Clear(void);

protected:

  virtual void SolveNonPrecond_(const VectorTypeH &rhs,
                                VectorTypeH *x);
  virtual void SolvePrecond_(const VectorTypeH &rhs,
                             VectorTypeH *x);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);

private:

  Solver<OperatorTypeL, VectorTypeL, ValueTypeL> *Solver_L_;

  VectorTypeH r_h_;
  VectorTypeL r_l_;

  VectorTypeH *x_h_;
  VectorTypeL d_l_;
  VectorTypeH d_h_;

  const OperatorTypeH *op_h_;
  OperatorTypeL *op_l_;

};


}

#endif // PARALUTION_MIXED_PRECISION_HPP_

