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

#ifndef __H2D_ADAPT_H1_H
#define __H2D_ADAPT_H1_H

#include "forms.h"
#include "weakform.h"
#include "integrals_h1.h"
#include "ref_selectors/selector.h"
#include "ref_selectors/h1_nonuniform_hp.h"

#define H2D_MAX_NUM_EQUATIONS 10 ///< A maximum number of equations/spaces, etc. Currently only H1AdaptHP uses it.

/// \brief hp-adaptivity module for H1 spaces.
///
/// H1AdaptHP is a hp-adaptivity module for continuous elements.
/// Given a reference solution, it provides functions to calculate H1 or
/// energy error estimates, acts as a container for the calculated errors.
/// If not specifie by the used, this class uses the most accurate adaptivity
/// selection algorithm which is slow.
///
class H2D_API H1AdaptHP
{
public:

  /// Initializes the class. 'num' is the number of mesh-space pairs to be adapted.
  /// After 'num', exactly that many space pointers must follow.
  H1AdaptHP(int num, Space** spaces);
  H1AdaptHP(Space* space);
  H1AdaptHP(Space* space1, Space* space2);
  H1AdaptHP(Space* space1, Space* space2, Space* space3);
  virtual ~H1AdaptHP();

  /// \deprecated Old-style and type-unsafe constructor.
  H1AdaptHP(int num, ...);

  typedef scalar (*biform_val_t) (int n, double *wt, Func<scalar> *u, Func<scalar> *v, Geom<double> *e, ExtData<scalar> *);
  typedef Ord (*biform_ord_t) (int n, double *wt, Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *);

  /// Sets user defined bilinear form to calculate error. Default forms are h1 error (on diagonal).
  /// Use this function only to change it (e.g. energy error).
  void set_biform(int i, int j, biform_val_t bi_form, biform_ord_t bi_ord);

  /// Type-safe version of calc_error_n() for one solution.
  virtual double calc_error(MeshFunction* sln, MeshFunction* rsln);

  /// Type-safe version of calc_error_n() for two solutions.
  double calc_error_2(MeshFunction* sln1, MeshFunction* sln2, MeshFunction* rsln1, MeshFunction* rsln2);

  /// Calculates the error of the solution using given norms. 'n' must be the
  /// same as 'num' in the constructor. After that, n coarse solution
  /// pointers are passed, followed by n fine solution pointers.
  virtual double calc_error_n(int n, ...);

  /// Refines elements based on results from calc_error() or calc_energy_error().
  bool adapt(double thr, int strat = 0, RefinementSelectors::Selector* refinement_selector = NULL,
             int regularize = -1,
             bool same_orders = false, double to_be_processed = 0.0);

  /// Unrefines the elements with the smallest error
  void unrefine(double thr);

  /// Internal. Functions to obtain errors of individual elements.
  struct ElementReference { ///< A reference to a element.
    int id, comp;
    ElementReference() {};
    ElementReference(int id, int comp) : id(id), comp(comp) {};
  };
  double get_element_error(int component, int id) const { return errors[component][id]; }
  ElementReference*  get_sorted_elements() const { return esort; }
  int    get_total_active_elements() const { return nact; }

private: //internal constructions
  void init_instance(int num, Space** spaces); ///< Initialize instance. Used by all construtors.

protected: //adaptivity
  RefinementSelectors::H1NonUniformHP default_refin_selector; ///< A default refinement selected which is used when no refinement selector is provided.

  std::queue<ElementReference> priority_esort; ///< A list of priority elements that are processed before the next element in esort is processed.

  virtual bool ignore_element_adapt(const int inx_element, const Mesh* mesh, const Element* element) { return false; }; ///< Returns true, if an element should be ignored for purposes of adaptivity.
  virtual bool can_adapt_element(Mesh* mesh, Element* e, const int split, const int4& p, const int4& q) { return true; }; ///< Returns true, if an element can be adapted using a selected candidate.
  void fix_shared_mesh_refinements(Mesh** meshes, const int num_comps, std::vector<ElementToRefine>& elems_to_refine, AutoLocalArray2<int>& idx, RefinementSelectors::Selector* refinement_selector); ///< Fixes refinements of a mesh which is shared among multiple components of a multimesh.

  virtual void apply_refinements(Mesh** meshes, std::vector<ElementToRefine>& elems_to_refine); ///< Apply refinements.

protected:
  // spaces & solutions
  int num;
  Space* spaces[H2D_MAX_NUM_EQUATIONS];
  Solution* sln[H2D_MAX_NUM_EQUATIONS];
  Solution* rsln[H2D_MAX_NUM_EQUATIONS];

  // element error arrays
  double* errors[H2D_MAX_NUM_EQUATIONS];
  double  norms[H2D_MAX_NUM_EQUATIONS];
  bool    have_errors;
  double  total_err;
  ElementReference* esort;
  int   nact;

  // bilinear forms to calculate error
  biform_val_t form[H2D_MAX_NUM_EQUATIONS][H2D_MAX_NUM_EQUATIONS];
  biform_ord_t ord[H2D_MAX_NUM_EQUATIONS][H2D_MAX_NUM_EQUATIONS];

  // evaluation of error and norm forms
  scalar eval_error(biform_val_t bi_fn, biform_ord_t bi_ord,
                    MeshFunction *sln1, MeshFunction *sln2, MeshFunction *rsln1, MeshFunction *rsln2,
                    RefMap *rv1,        RefMap *rv2,        RefMap *rrv1,        RefMap *rrv2);

  scalar eval_norm(biform_val_t bi_fn, biform_ord_t bi_ord,
                   MeshFunction *rsln1, MeshFunction *rsln2, RefMap *rrv1, RefMap *rrv2);

  /// Builds a list of elements sorted by error descending. Assumes that H1OrthoHP::errors is initialized. Initializes H1OrthoHP::esort.
  /// \param meshes: Meshes. Indices [0,.., H1OrthoHP::num-1] contains meshes of coarse solutions, indices [H1OrthoHP::num,..,2*H1OrthoHP::num - 1] contains meshes of reference solution.
  void sort_elements_by_error(Mesh** meshes);

private:
  static double** cmp_err; ///< An helper array used to sort reference to elements.
  static int compare(const void* p1, const void* p2); ///< Compares to reference to an element according to H1OrthoHP::cmp_err.
};



#endif
