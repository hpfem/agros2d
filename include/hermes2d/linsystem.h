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

#ifndef __HERMES2D_LINSYSTEM_H
#define __HERMES2D_LINSYSTEM_H

#include "matrix.h"

class Space;
class PrecalcShapeset;
class WeakForm;
class Solver;
struct Page;


///  
///
///
///
///
class LinSystem
{
public:

  LinSystem(WeakForm* wf, Solver* solver = NULL);
  virtual ~LinSystem();

  void set_spaces(int n, ...);
  void set_pss(int n, ...);
  void copy(LinSystem* sys);

  void assemble(bool rhsonly = false);
  void assemble_rhs_only() { assemble(true); }
  bool solve(int n, ...);
  void free();

  void save_matrix_matlab(const char* filename, const char* varname = "A");
  void save_rhs_matlab(const char* filename, const char* varname = "b");
  void save_matrix_bin(const char* filename);
  void save_rhs_bin(const char* filename);

  int get_num_dofs() const { return ndofs; };
  int get_matrix_size() const;
  void get_matrix(int*& Ap, int*& Ai, scalar*& Ax, int& size) const
    { Ap = this->Ap; Ai = this->Ai; Ax = this->Ax; size = ndofs; }
  void get_rhs(scalar*& RHS, int& size) const { RHS = this->RHS; size=ndofs; }


protected:

  WeakForm* wf;
  Solver* solver;
  void* slv_ctx;

  Space** spaces;
  PrecalcShapeset** pss;

  int ndofs;
  int* Ap;      ///< row/column start indices
  int* Ai;      ///< element positions within rows/columns
  scalar* Ax;   ///< matrix values
  bool mat_row; ///< true if the matrix is row-oriented (CSR)
  bool mat_sym; ///< true if symmetric and only upper half stored

  scalar* RHS; ///< assembled right-hand side
  scalar* Dir; ///< contributions to the RHS from Dirichlet DOFs
  scalar* Vec; ///< last solution vector

  void create_matrix(bool rhsonly);
  void precalc_sparse_structure(Page** pages);
  void insert_block(scalar** mat, int* iidx, int* jidx, int ilen, int jlen);


  scalar** get_matrix_buffer(int n)
  {
    if (n <= mat_size) return buffer;
    if (buffer != NULL) delete [] buffer;
    return (buffer = new_matrix<scalar>(mat_size = n));
  }

  scalar** buffer;
  int mat_size;

  int* sp_seq;
  int num_user_pss;
  bool values_changed;
  bool struct_changed;
  bool want_dir_contrib;
  bool have_spaces;
  
  friend class RefSystem;
  
};



// can be called to set a custom order limiting table
extern void set_order_limit_table(int* tri_table, int* quad_table, int n);

// limit_order is used in integrals
extern int  g_max_order;
extern int* g_order_table;

#ifndef DEBUG_ORDER
  #define limit_order(o) \
    if (o > g_max_order) { o = g_max_order; warn_order(); } \
    o = g_order_table[o];
  #define limit_order_nowarn(o) \
    if (o > g_max_order) o = g_max_order; \
    o = g_order_table[o];
#else
  #define limit_order(o) \
    if (o > g_max_order) warn_order(); \
    o = g_max_order;
  #define limit_order_nowarn(o) \
    o = g_max_order;
#endif

extern void warn_order();
extern void update_limit_table(int mode);

    
#endif
