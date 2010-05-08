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


#ifndef __H2D_FEPROBLEM_H
#define __H2D_FEPROBLEM_H

#include "matrix.h"
#include "forms.h"
#include "weakform.h"
#include <map>

class Space;
class PrecalcShapeset;
class WeakForm;
class Matrix;
class SparseMatrix;
class Vector;
class Solver;

/// Finite Element problem class
///
/// This class does assembling into passed-in structures.
///
class FeProblem {
public:
  FeProblem(WeakForm *wf);
  virtual ~FeProblem();
  void free();

  void set_spaces(int n, ...);
  void set_pss(int n, ...);
  Space* get_space(int n) {  return this->spaces[n];  }
  PrecalcShapeset* get_pss(int n) {  return this->pss[n];  }

  void create(SparseMatrix *mat);
  void assemble(const Vector *x, Vector *f, Matrix *jac);

  int get_num_dofs();
  bool is_matrix_free() { return wf->is_matrix_free(); }
  void invalidate_matrix() { have_matrix = false; }

protected:
  WeakForm *wf;

  int ndofs;
  int *sp_seq;
  int wf_seq;
  Space **spaces;
  Solution **slns;
  PrecalcShapeset** pss;

  int num_user_pss;
  bool values_changed;
  bool struct_changed;
  bool have_spaces;
  bool have_matrix;
  bool is_up_to_date();

  scalar** buffer;
  int mat_size;

  scalar** get_matrix_buffer(int n)
  {
    if (n <= mat_size) return buffer;
    if (buffer != NULL) delete [] buffer;
    return (buffer = new_matrix<scalar>(mat_size = n));
  }

  ExtData<Ord>* init_ext_fns_ord(std::vector<MeshFunction *> &ext);
  ExtData<scalar>* init_ext_fns(std::vector<MeshFunction *> &ext, RefMap *rm, const int order);
  Func<double>* get_fn(PrecalcShapeset *fu, RefMap *rm, const int order);

  // Key for caching transformed function values on elements
  struct Key
  {
    int index;
    int order;
    int sub_idx;
    int shapeset_type;

    Key(int index, int order, int sub_idx, int shapeset_type)
    {
      this->index = index;
      this->order = order;
      this->sub_idx = sub_idx;
      this->shapeset_type = shapeset_type;
    }
  };

  struct Compare
  {
    bool operator()(Key a, Key b) const
    {
      if (a.index < b.index) return true;
      else if (a.index > b.index) return false;
      else
      {
        if (a.order < b.order) return true;
        else if (a.order > b.order) return false;
        else
        {
          if (a.sub_idx < b.sub_idx) return true;
          else if (a.sub_idx > b.sub_idx) return false;
          else
          {
            if (a.shapeset_type < b.shapeset_type) return true;
            else return false;
          }
        }
      }
    }
  };

  // Caching transformed values for element
  std::map<Key, Func<double>*, Compare> cache_fn;
  Geom<double>* cache_e[g_max_quad + 1 + 4];
  double* cache_jwt[g_max_quad + 1 + 4];

  void init_cache();
  void delete_cache();

  scalar eval_form(WeakForm::JacFormVol *bf, Solution *sln[], PrecalcShapeset *fu, PrecalcShapeset *fv, RefMap *ru, RefMap *rv);
  scalar eval_form(WeakForm::ResFormVol *lf, Solution *sln[], PrecalcShapeset *fv, RefMap *rv);
  scalar eval_form(WeakForm::JacFormSurf *bf, Solution *sln[], PrecalcShapeset *fu, PrecalcShapeset *fv, RefMap *ru, RefMap *rv, EdgePos* ep);
  scalar eval_form(WeakForm::ResFormSurf *lf, Solution *sln[], PrecalcShapeset *fv, RefMap *rv, EdgePos* ep);

};


///
///   Class for projecting solution, from Meshfunction obtain solution vector
///   Solution vector needed for nonlinear methods as initial guess
///
class Projection
{
public:

  Projection(int n, ...);
  ~Projection();

  void set_solver(Solver* solver);
  scalar* project();
  scalar* get_solution_vector() const { return vec; }

protected:

  int num;
  MeshFunction* slns[10];
  Space* spaces[10];
  PrecalcShapeset* pss[10];
  Solver* solver;
  scalar* vec;
};

#endif



