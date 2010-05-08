// This file is part of Hermes2D.
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

#ifndef __H2D_NONLINSYSTEM_H
#define __H2D_NONLINSYSTEM_H

#include "matrix.h"
#include "filter.h"
#include "views/scalar_view.h"
#include "views/order_view.h"
#include "function.h"

class Solution;
class MeshFunction;



///
///
///
///
///
class H2D_API NonlinSystem : public LinSystem
{
public:

  /// Initializes the class and sets zero initial coefficient vector.
  NonlinSystem(WeakForm* wf, Solver* solver);

  virtual void free();

  /// Sets the initial coefficient vector so that it represents the given function(s).
  /// You can pass pointer(s) to Solution(s) or to a Filter(s).
  /// The projected Solution(s) are returned in "result" (satisfy Dirichlet BC)
  /// You can specify in which norm "fn" should be projected: 0 - L2 norm, 1 - H1 norm
  void set_ic(MeshFunction* fn, Solution* result, int proj_norm = 1)
    {  set_ic_n(proj_norm, 1, fn, result);  }

  void set_ic(MeshFunction* fn1, MeshFunction* fn2, Solution* result1, Solution* result2, int proj_norm = 1)
    {  set_ic_n(proj_norm, 2, fn1, fn2, result1, result2);  }

  /// Sets the initial coefficient vector using an exact function.
  void set_ic(scalar (*exactfn)(double x, double y, scalar& dx, scalar& dy),
              Mesh* mesh, Solution* result, int proj_norm = 1)
  {
    result->set_exact(mesh, exactfn);
    set_ic_n(proj_norm, 1, result, result);
  }

  void set_ic_n(int proj_norm, int n, ...);

  /// Sets the solution coefficient vector to zero
  void set_vec_zero();

  /// Adjusts the iteration coefficient. The default value for alpha is 1.
  void set_alpha(double alpha) { this->alpha = alpha; }

  /// Assembles the jacobian and the residuum vector.
  void assemble(bool rhsonly = false);

  /// Performs one Newton iteration, stores the result in the given Solutions.
  bool solve(int n, ...);

  /// Performs complete Newton's loop for one equation
  bool solve_newton_1(Solution* u_prev, double newton_tol, int newton_max_iter,
                      Filter* f1 = NULL, Filter* f2 = NULL, Filter* f3 = NULL);
  /// Performs complete Newton's loop for two equations
  bool solve_newton_2(Solution* u_prev_1, Solution* u_prev_2, double newton_tol, int newton_max_iter,
                      Filter* f1 = NULL, Filter* f2 = NULL, Filter* f3 = NULL);
  /// Performs complete Newton's loop for two equations
  bool solve_newton_3(Solution* u_prev_1, Solution* u_prev_2, Solution* u_prev_3,
                      double newton_tol, int newton_max_iter,
                      Filter* f1 = NULL, Filter* f2 = NULL, Filter* f3 = NULL);

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
