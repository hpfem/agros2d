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


#ifndef PARALUTION_PRECONDITIONER_BLOCKPRECOND_HPP_
#define PARALUTION_PRECONDITIONER_BLOCKPRECOND_HPP_

#include "../solver.hpp"
#include "preconditioner.hpp"
#include "../../base/local_vector.hpp"

#include <vector>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
class BlockPreconditioner : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  BlockPreconditioner();
  virtual ~BlockPreconditioner();

  virtual void Print(void) const;  
  virtual void Clear(void);  

  virtual void Set(const int n,
                   const int *size,
                   Solver<OperatorType, VectorType, ValueType> **D_solver);

  virtual void SetDiagonalSolver(void);
  virtual void SetLSolver(void);

  virtual void SetExternalLastMatrix(const OperatorType &mat);
  
  virtual void Build(void);

  virtual void Solve(const VectorType &rhs,
                     VectorType *x);


  
protected:

  // The operator decomposition
  OperatorType ***A_block_;
  OperatorType *A_last_;

  /// Keep the precond matrix in CSR or not
  bool op_mat_format_; 
  /// Precond matrix format
  unsigned int precond_mat_format_;

  VectorType **x_block_;  
  VectorType **tmp_block_;  

  int num_blocks_;
  int *block_sizes_;

  Solver<OperatorType, VectorType, ValueType> **D_solver_;


  bool diag_solve_;

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);
  

};


}

#endif // PARALUTION_PRECONDITIONER_BLOCKPRECOND_HPP_

