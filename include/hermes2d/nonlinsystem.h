// This file is part of Hermes2D.
//
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
// Copyright 2005-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Pavel Solin <solin@unr.edu>
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

// $Id: linsystem.h 1037 2008-10-01 21:32:06Z jakub $

#ifndef __HERMES2D_NONLINSYSTEM_H
#define __HERMES2D_NONLINSYSTEM_H

#include "matrix.h"
#include "function.h"

class Solution;
class MeshFunction;
  


///  
///
///
///
///
class NonlinSystem : public LinSystem 
{
public:
 
  /// Initializes the class and sets zero initial coefficient vector.
  NonlinSystem(WeakForm* wf, Solver* solver);

  /// Sets the initial coefficient vector so that it represents the given function.
  /// You can pass a pointer to a Solution or to a Filter.
  /// The projected Solution is returned in "result" (it satisfies Dirichlet BC)
  /// You can specify in which norm "fn" should be projected: 0 - L2 norm, 1 - H1 norm
  void set_ic(MeshFunction* fn, Solution* result, int proj_norm = 1);

  /// Sets the solution coefficient vector to zero
  void set_vec_zero();

  /// Adjusts the iteration coefficient. The default value for alpha is 1.
  void set_alpha(double alpha) { this->alpha = alpha; }

  /// Assembles the jacobian and the residuum vector.
  void assemble(bool rhsonly = false);

  /// Performs one Newton iteration, stores the result in the given Solutions.
  bool solve(int n, ...);
  
  /// returns the L2-norm of the residuum
  double get_residuum_l2_norm() const { return res_l2; }
  /// returns the L1-norm of the residuum  
  double get_residuum_l1_norm() const { return res_l1; }
  /// returns the L_inf-norm of the residuum
  double get_residuum_max_norm() const { return res_max; }


protected:

  double alpha;
  double res_l2, res_l1, res_max;

  friend class RefNonlinSystem;

};


#endif
