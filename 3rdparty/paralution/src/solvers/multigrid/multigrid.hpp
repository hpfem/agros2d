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


#ifndef PARALUTION_MULTIGRID_HPP_
#define PARALUTION_MULTIGRID_HPP_

#include "../solver.hpp"

namespace paralution {

enum _cycle {
  Vcycle = 0,
  Wcycle = 1,
  Kcycle = 2,
  Fcycle = 3
};

template <class OperatorType, class VectorType, typename ValueType>
class MultiGrid : public IterativeLinearSolver<OperatorType, VectorType, ValueType> {
  
public:

  MultiGrid();
  virtual ~MultiGrid();

  virtual void Print(void) const;

  /// disabled function
  virtual void SetPreconditioner(Solver<OperatorType, VectorType, ValueType> &precond);

  /// Set the operator for each level
  virtual void SetOperatorHierarchy(OperatorType **op);

  /// Set the smoother for each level
  virtual void SetSolver(Solver<OperatorType, VectorType, ValueType> &solver);
  
  /// Set the smoother for each level
  virtual void SetSmoother(IterativeLinearSolver<OperatorType, VectorType, ValueType> **smoother);

  /// Set the number of pre-smoothing steps
  virtual void SetSmootherPreIter(const int iter);

  /// Set the number of post-smoothing steps
  virtual void SetSmootherPostIter(const int iter);

  /// Set thre restriction method by operator for each level
  virtual void SetRestrictOperator(OperatorType **op);

  /// Set the restriction method by vector for each level
  virtual void SetRestrictVector(LocalVector<int> **op);

  /// Set the prolongation operator for each level
  virtual void SetProlongOperator(OperatorType **op);

  /// Enable/disable scaling of intergrid transfers
  virtual void SetScaling(const bool scaling);

  /// Set the MultiGrid Cycle (default: Vcycle)
  virtual void SetCycle(const _cycle cycle);

  /// Set the depth of the multigrid solver
  virtual void InitLevels(const int levels);

  /// Set the grid transfer operator type, true = OperatorType, false = LocalVector<int>
  virtual void InitOperator(const bool type);

  /// Called by default the V-cycle
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);

  virtual void Build(void);
  virtual void Clear(void);

protected:

  // Restricts from level 'level' to 'level-1'
  virtual void Restrict_(const VectorType &fine,
                         VectorType *coarse,
                         const int level);

  // Prolongs from level 'level' to 'level+1'
  virtual void Prolong_(const VectorType &coarse,
                        VectorType *fine,
                        const int level);

  void Vcycle_(const VectorType &rhs, VectorType *x);
  void Wcycle_(const VectorType &rhs, VectorType *x);
  void Fcycle_(const VectorType &rhs, VectorType *x);
  void Kcycle_(const VectorType &rhs, VectorType *x);

   /// disabled function
  virtual void SolveNonPrecond_(const VectorType &rhs,
                                VectorType *x);

  /// disabled function
  virtual void SolvePrecond_(const VectorType &rhs,
                             VectorType *x);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


  int levels_ ;
  int current_level_;
  bool operator_type_;
  bool scaling_;
  int iter_pre_smooth_;
  int iter_post_smooth_;
  int cycle_;

  ValueType res_norm_;

  OperatorType **op_level_;

  OperatorType **restrict_op_level_;
  OperatorType **prolong_op_level_;
  LocalVector<int> **restrict_vector_level_;

  VectorType **d_level_;
  VectorType **r_level_;
  VectorType **t_level_;
  VectorType **s_level_;
  VectorType **p_level_;
  VectorType **q_level_;
  VectorType **k_level_;
  VectorType **l_level_;

  Solver<OperatorType, VectorType, ValueType>  *solver_coarse_;
  IterativeLinearSolver<OperatorType, VectorType, ValueType> **smoother_level_;

};


}

#endif // PARALUTION_MULTIGRID_HPP_

