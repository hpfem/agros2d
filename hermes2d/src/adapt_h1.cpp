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

#include "common.h"
#include "solution.h"
#include "linsystem.h"
#include "refmap.h"
#include "shapeset_h1_all.h"
#include "quad_all.h"
#include "integrals_h1.h"
#include "matrix.h"
#include "adapt_h1.h"
#include "traverse.h"
#include "norm.h"
#include "element_to_refine.h"
#include "ref_selectors/selector.h"

using namespace std;

/* constructors of attributes in the instance */
#define INST_CONS() default_refin_selector(false)

H1AdaptHP::H1AdaptHP(int num, Space** spaces) : INST_CONS() {
  init_instance(num, spaces);
}

H1AdaptHP::H1AdaptHP(Space* space) : INST_CONS() {
  init_instance(1, &space);
}

H1AdaptHP::H1AdaptHP(Space* space1, Space* space2) : INST_CONS() {
  Space* spaces[] = { space1, space2 };
  init_instance(2, spaces);
}

H1AdaptHP::H1AdaptHP(Space* space1, Space* space2, Space* space3) : INST_CONS() {
  Space* spaces[] = { space1, space2, space3 };
  init_instance(3, spaces);
}

H1AdaptHP::H1AdaptHP(int num, ...) : INST_CONS() {
  Space** spaces = new Space*[num];
  va_list ap;
  va_start(ap, num);
  for (int i = 0; i < num; i++)
    spaces[i] = va_arg(ap, Space*);
  va_end(ap);

  init_instance(num, spaces);

  delete[] spaces;
}

H1AdaptHP::~H1AdaptHP()
{
  for (int i = 0; i < num; i++)
    if (errors[i] != NULL)
      delete [] errors[i];

  if (esort != NULL) delete [] esort;
}

void H1AdaptHP::init_instance(int num, Space** spaces) {
  assert_msg(num < H2D_MAX_NUM_EQUATIONS, "Too many spaces (%d), only %d supported", num, H2D_MAX_NUM_EQUATIONS);

  this->num = num;
  for (int i = 0; i < num; i++)
    this->spaces[i] = spaces[i];

  for (int i = 0; i < num; i++)
    for (int j = 0; j < num; j++)
    {
      if (i == j) {
        form[i][j] = h1_form<double, scalar>;
        ord[i][j]  = h1_form<Ord, Ord>;
      }
      else {
        form[i][j] = NULL;
        ord[i][j]  = NULL;
      }
    }

  memset(errors, 0, sizeof(errors));
  esort = NULL;
  have_errors = false;
}

//// adapt /////////////////////////////////////////////////////////////////////////////////////////

bool H1AdaptHP::adapt(double thr, int strat, RefinementSelectors::Selector* refinement_selector,
                      int regularize,
                      bool same_orders, double to_be_processed)
{
  if (!have_errors)
    error("Element errors have to be calculated first, see calc_error().");

  //use default refinement if none is given
  if (refinement_selector == NULL)
    refinement_selector = &default_refin_selector;

  //get meshes
  int i, j, l;
  int max_id = -1;
  Mesh* meshes[H2D_MAX_NUM_EQUATIONS];
  for (j = 0; j < num; j++) {
    meshes[j] = spaces[j]->get_mesh();
    rsln[j]->set_quad_2d(&g_quad_2d_std);
    rsln[j]->enable_transform(false);
    if (meshes[j]->get_max_element_id() > max_id)
      max_id = meshes[j]->get_max_element_id();
  }

  AUTOLA2_OR(int, idx, max_id + 1, num + 1);
  for(j = 0; j < max_id; j++)
    for(l = 0; l < num; l++)
      idx[j][l] = -1; // element not refined

  double err0 = 1000.0;
  double processed_error = 0.0;

  vector<ElementToRefine> elem_inx_to_proc; //list of indices of elements that are going to be processed
  elem_inx_to_proc.reserve(nact);

  //adaptivity loop
  double error_threshod = -1; //an error threshold that breaks the adaptivity loop in a case of strategy 1
  int num_exam_elem = 0; //a number of examined elements
  int num_ignored_elem = 0; //a number of ignored elements
  int num_not_changed = 0; //a number of element that were not changed
  int num_priority_elem = 0; //a number of elements that were processed using priority queue

  int inx_regular_element = 0;
  while (inx_regular_element < nact || !priority_esort.empty())
  {
    int id, comp, inx_element;

    //get element identification
    if (priority_esort.empty()) {
      id = esort[inx_regular_element].id;
      comp = esort[inx_regular_element].comp;
      inx_element = inx_regular_element;
      inx_regular_element++;
    }
    else {
      id = priority_esort.front().id;
      comp = priority_esort.front().comp;
      inx_element = -1;
      priority_esort.pop();
      num_priority_elem++;
    }
    num_exam_elem++;

    //get info linked with the element
    double err = errors[comp][id];
    Mesh* mesh = meshes[comp];
    Element* e = mesh->get_element(id);

    if (!ignore_element_adapt(inx_element, mesh, e)) {

      //use error of the first refined element to calculate the threshold for strategy 1
      if (elem_inx_to_proc.empty())
        error_threshod = thr * err;

      //stop the loop only if processing regular elements
      if (inx_element >= 0) {

        // first refinement strategy:
        // refine elements until prescribed amount of error is processed
        // if more elements have similar error refine all to keep the mesh symmetric
        if ((strat == 0) && (processed_error > sqrt(thr) * total_err) && fabs((err - err0)/err0) > 1e-3) break;

        // second refinement strategy:
        // refine all elements whose error is bigger than some portion of maximal error
        if ((strat == 1) && (err < error_threshod)) break;

        if ((strat == 2) && (err < thr)) break;

        if ((strat == 3) &&
          ( (err < error_threshod) ||
          ( processed_error > 1.5 * to_be_processed )) ) break;
      }

      // get refinement suggestion
      ElementToRefine elem_ref(id, comp);
      int current = spaces[comp]->get_element_order(id);
      bool refined = refinement_selector->select_refinement(e, current, rsln[comp], elem_ref);

      //add to a list of elements that are going to be refined
      if (refined && can_adapt_element(mesh, e, elem_ref.split, elem_ref.p, elem_ref.q) ) {
        idx[id][comp] = (int)elem_inx_to_proc.size();
        elem_inx_to_proc.push_back(elem_ref);
        err0 = err;
        processed_error += err;
      }
      else
        num_not_changed++;
    }
    else {
      num_ignored_elem++;
    }
  }

  verbose("Examined elements: %d", num_exam_elem);
  verbose(" Elements taken from priority queue: %d", num_priority_elem);
  verbose(" Ignored elements: %d", num_ignored_elem);
  verbose(" Not changed elements: %d", num_not_changed);
  verbose(" Elements to process: %d", elem_inx_to_proc.size());
  bool done = false;
  if (num_exam_elem == 0)
    done = true;
  else if (elem_inx_to_proc.empty())
  {
    warn("None of the elements selected for refinement could be refined. Adaptivity step not successful, returning 'true'.");
    done = true;
  }

  //fix refinement if multimesh is used
  fix_shared_mesh_refinements(meshes, num, elem_inx_to_proc, idx, refinement_selector);

  //apply refinements
  apply_refinements(meshes, elem_inx_to_proc);

  if (same_orders)
  {
    Element* e;
    for (i = 0; i < num; i++)
    {
      for_all_active_elements(e, meshes[i])
      {
        int current = H2D_GET_H_ORDER(spaces[i]->get_element_order(e->id));
        for (j = 0; j < num; j++)
          if ((j != i) && (meshes[j] == meshes[i])) // components share the mesh
          {
            int o = H2D_GET_H_ORDER(spaces[j]->get_element_order(e->id));
            if (o > current) current = o;
          }
        spaces[i]->set_element_order(e->id, current);
      }
    }
  }

  // mesh regularization
  if (regularize >= 0)
  {
    if (regularize == 0)
    {
      regularize = 1;
      warn("Total mesh regularization is not supported in adaptivity. 1-irregular mesh is used instead.");
    }
    for (i = 0; i < num; i++)
    {
      int* parents;
      parents = meshes[i]->regularize(regularize);
      spaces[i]->distribute_orders(meshes[i], parents);
      delete [] parents;
    }
  }

  for (j = 0; j < num; j++)
    rsln[j]->enable_transform(true);


  verbose("Refined elements: %d", elem_inx_to_proc.size());
  have_errors = false;
  if (strat == 2 && done == true) have_errors = true; // space without changes

  return done;
}

void H1AdaptHP::fix_shared_mesh_refinements(Mesh** meshes, const int num_comps, std::vector<ElementToRefine>& elems_to_refine, AutoLocalArray2<int>& idx, RefinementSelectors::Selector* refinement_selector) {
  int num_elem_to_proc = elems_to_refine.size();
  for(int inx = 0; inx < num_elem_to_proc; inx++) {
    ElementToRefine& elem_ref = elems_to_refine[inx];
    int current_quad_order = spaces[elem_ref.comp]->get_element_order(elem_ref.id);
    Element* current_elem = meshes[elem_ref.comp]->get_element(elem_ref.id);

    //select a refinement used by all components that share a mesh which is about to be refined
    int selected_refinement = elem_ref.split;
    for (int j = 0; j < num_comps; j++)
    {
      if (selected_refinement == H2D_REFINEMENT_H) break; // iso refinement is max what can be recieved
      if (j != elem_ref.comp && meshes[j] == meshes[elem_ref.comp]) { // if a mesh is shared
        int ii = idx[elem_ref.id][j];
        if (ii >= 0) { // and the sample element is about to be refined by another compoment
          const ElementToRefine& elem_ref_ii = elems_to_refine[ii];
          if (elem_ref_ii.split != selected_refinement && elem_ref_ii.split != H2D_REFINEMENT_P) { //select more complicated refinement
            if ((elem_ref_ii.split == H2D_REFINEMENT_ANISO_H || elem_ref_ii.split == H2D_REFINEMENT_ANISO_V) && selected_refinement == H2D_REFINEMENT_P)
              selected_refinement = elem_ref_ii.split;
            else
              selected_refinement = H2D_REFINEMENT_H;
          }
        }
      }
    }

    //fix other refinements according to the selected refinement
    if (selected_refinement != H2D_REFINEMENT_P)
    {
      //get suggested orders for the selected refinement
      const int* suggested_orders = NULL;
      if (selected_refinement == H2D_REFINEMENT_H)
        suggested_orders = elem_ref.q;

      //update orders
      for (int j = 0; j < num_comps; j++) {
        if (j != elem_ref.comp && meshes[j] == meshes[elem_ref.comp]) { // if components share the mesh
          // change currently processed refinement
          if (elem_ref.split != selected_refinement) {
            elem_ref.split = selected_refinement;
            refinement_selector->update_shared_mesh_orders(current_elem, current_quad_order, elem_ref.split, elem_ref.p, suggested_orders);
          }

          // change other refinements
          int ii = idx[elem_ref.id][j];
          if (ii >= 0) {
            ElementToRefine& elem_ref_ii = elems_to_refine[ii];
            if (elem_ref_ii.split != selected_refinement) {
              elem_ref_ii.split = selected_refinement;
              refinement_selector->update_shared_mesh_orders(current_elem, current_quad_order, elem_ref_ii.split, elem_ref_ii.p, suggested_orders);
            }
          }
          else { // element (of the other comp.) not refined at all: assign refinement
            ElementToRefine elem_ref_new(elem_ref.id, j);
            elem_ref_new.split = selected_refinement;
            refinement_selector->update_shared_mesh_orders(current_elem, current_quad_order, elem_ref_new.split, elem_ref_new.p, suggested_orders);
            elems_to_refine.push_back(elem_ref_new);
          }
        }
      }
    }
  }
}

void H1AdaptHP::apply_refinements(Mesh** meshes, std::vector<ElementToRefine>& elems_to_refine)
{
  for (vector<ElementToRefine>::const_iterator elem_ref = elems_to_refine.begin(); elem_ref != elems_to_refine.end(); elem_ref++) // go over elements to be refined
  {
    Element* e;
    e = meshes[elem_ref->comp]->get_element(elem_ref->id);

    if (elem_ref->split == H2D_REFINEMENT_P)
      spaces[elem_ref->comp]->set_element_order(elem_ref->id, elem_ref->p[0]);
    else if (elem_ref->split == H2D_REFINEMENT_H) {
      if (e->active)
        meshes[elem_ref->comp]->refine_element(elem_ref->id);
      for (int j = 0; j < 4; j++)
        spaces[elem_ref->comp]->set_element_order(e->sons[j]->id, elem_ref->p[j]);
    }
    else {
      if (e->active)
        meshes[elem_ref->comp]->refine_element(elem_ref->id, elem_ref->split);
      for (int j = 0; j < 2; j++)
        spaces[elem_ref->comp]->set_element_order(e->sons[ (elem_ref->split == 1) ? j : j+2 ]->id, elem_ref->p[j]);
    }
  }
}


///// Unrefinements /////////////////////////////////////////////////////////////////////////////////

void H1AdaptHP::unrefine(double thr)
{

  if (!have_errors)
    error("Element errors have to be calculated first, see calc_error().");

  Mesh* mesh[2];
  mesh[0] = spaces[0]->get_mesh();
  mesh[1] = spaces[1]->get_mesh();


  int k = 0;
  if (mesh[0] == mesh[1]) // single mesh
  {
    Element* e;
    for_all_inactive_elements(e, mesh[0])
    {
      bool found = true;
      for (int i = 0; i < 4; i++)
        if (e->sons[i] != NULL && ((!e->sons[i]->active) || (e->sons[i]->is_curved())))
      { found = false;  break; }

      if (found)
      {
        double sum1 = 0.0, sum2 = 0.0;
        int max1 = 0, max2 = 0;
        for (int i = 0; i < 4; i++)
          if (e->sons[i] != NULL)
        {
          sum1 += errors[0][e->sons[i]->id];
          sum2 += errors[1][e->sons[i]->id];
          int oo = spaces[0]->get_element_order(e->sons[i]->id);
          if (oo > max1) max1 = oo;
          oo = spaces[1]->get_element_order(e->sons[i]->id);
          if (oo > max2) max2 = oo;
        }
        if ((sum1 < thr * errors[esort[0].comp][esort[0].id]) &&
             (sum2 < thr * errors[esort[0].comp][esort[0].id]))
        {
          mesh[0]->unrefine_element(e->id);
          mesh[1]->unrefine_element(e->id);
          errors[0][e->id] = sum1;
          errors[1][e->id] = sum2;
          spaces[0]->set_element_order(e->id, max1);
          spaces[1]->set_element_order(e->id, max2);
          k++; // number of unrefined elements
        }
      }
    }
    for_all_active_elements(e, mesh[0])
    {
      for (int i = 0; i < 2; i++)
        if (errors[i][e->id] < thr/4 * errors[esort[0].comp][esort[0].id])
      {
        int oo = H2D_GET_H_ORDER(spaces[i]->get_element_order(e->id));
        spaces[i]->set_element_order(e->id, std::max(oo - 1, 1));
        k++;
      }
    }
  }
  else // multimesh
  {
    for (int m = 0; m < 2; m++)
    {
      Element* e;
      for_all_inactive_elements(e, mesh[m])
      {
        bool found = true;
        for (int i = 0; i < 4; i++)
          if (e->sons[i] != NULL && ((!e->sons[i]->active) || (e->sons[i]->is_curved())))
        { found = false;  break; }

        if (found)
        {
          double sum = 0.0;
          int max = 0;
          for (int i = 0; i < 4; i++)
            if (e->sons[i] != NULL)
          {
            sum += errors[m][e->sons[i]->id];
            int oo = spaces[m]->get_element_order(e->sons[i]->id);
            if (oo > max) max = oo;
          }
          if ((sum < thr * errors[esort[0].comp][esort[0].id]))
          //if ((sum < 0.1 * thr))
          {
            mesh[m]->unrefine_element(e->id);
            errors[m][e->id] = sum;
            spaces[m]->set_element_order(e->id, max);
            k++; // number of unrefined elements
          }
        }
      }
      for_all_active_elements(e, mesh[m])
      {
        if (errors[m][e->id] < thr/4 * errors[esort[0].comp][esort[0].id])
        {
          int oo = H2D_GET_H_ORDER(spaces[m]->get_element_order(e->id));
          spaces[m]->set_element_order(e->id, std::max(oo - 1, 1));
          k++;
        }
      }
    }
  }
  verbose("Unrefined %d elements.", k);
  have_errors = false;
}

//// error calculation /////////////////////////////////////////////////////////////////////////////

double** H1AdaptHP::cmp_err;
int H1AdaptHP::compare(const void* p1, const void* p2) {
  const ElementReference& e1 = *((const ElementReference*)p1);
  const ElementReference& e2 = *((const ElementReference*)p2);
  return cmp_err[e1.comp][e1.id] < cmp_err[e2.comp][e2.id] ? 1 : -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void H1AdaptHP::set_biform(int i, int j, biform_val_t bi_form, biform_ord_t bi_ord)
{
  if (i < 0 || i >= num || j < 0 || j >= num)
    error("Invalid equation number.");

  form[i][j] = bi_form;
  ord[i][j] = bi_ord;
}


scalar H1AdaptHP::eval_error(biform_val_t bi_fn, biform_ord_t bi_ord,
                             MeshFunction *sln1, MeshFunction *sln2, MeshFunction *rsln1, MeshFunction *rsln2,
                             RefMap *rv1,        RefMap *rv2,        RefMap *rrv1,        RefMap *rrv2)
{
  // determine the integration order
  int inc = (rsln1->get_num_components() == 2) ? 1 : 0;
  Func<Ord>* ou = init_fn_ord(rsln1->get_fn_order() + inc);
  Func<Ord>* ov = init_fn_ord(rsln2->get_fn_order() + inc);

  double fake_wt = 1.0;
  Geom<Ord>* fake_e = init_geom_ord();
  Ord o = bi_ord(1, &fake_wt, ou, ov, fake_e, NULL);
  int order = rrv1->get_inv_ref_order();
  order += o.get_order();
  limit_order(order);

  ou->free_ord(); delete ou;
  ov->free_ord(); delete ov;
  delete fake_e;

  // eval the form
  Quad2D* quad = sln1->get_quad_2d();
  double3* pt = quad->get_points(order);
  int np = quad->get_num_points(order);

  // init geometry and jacobian*weights
  Geom<double>* e = init_geom_vol(rrv1, order);
  double* jac = rrv1->get_jacobian(order);
  double* jwt = new double[np];
  for(int i = 0; i < np; i++)
    jwt[i] = pt[i][2] * jac[i];

  // function values and values of external functions
  Func<scalar>* err1 = init_fn(sln1, rv1, order);
  Func<scalar>* err2 = init_fn(sln2, rv2, order);
  Func<scalar>* v1 = init_fn(rsln1, rrv1, order);
  Func<scalar>* v2 = init_fn(rsln2, rrv2, order);

  for (int i = 0; i < np; i++)
  {
    err1->val[i] = err1->val[i] - v1->val[i];
    err1->dx[i] = err1->dx[i] - v1->dx[i];
    err1->dy[i] = err1->dy[i] - v1->dy[i];
    err2->val[i] = err2->val[i] - v2->val[i];
    err2->dx[i] = err2->dx[i] - v2->dx[i];
    err2->dy[i] = err2->dy[i] - v2->dy[i];
  }

  scalar res = bi_fn(np, jwt, err1, err2, e, NULL);

  e->free(); delete e;
  delete [] jwt;
  err1->free_fn(); delete err1;
  err2->free_fn(); delete err2;
  v1->free_fn(); delete v1;
  v2->free_fn(); delete v2;

  return res;
}


scalar H1AdaptHP::eval_norm(biform_val_t bi_fn, biform_ord_t bi_ord,
                            MeshFunction *rsln1, MeshFunction *rsln2, RefMap *rrv1, RefMap *rrv2)
{
  // determine the integration order
  int inc = (rsln1->get_num_components() == 2) ? 1 : 0;
  Func<Ord>* ou = init_fn_ord(rsln1->get_fn_order() + inc);
  Func<Ord>* ov = init_fn_ord(rsln2->get_fn_order() + inc);

  double fake_wt = 1.0;
  Geom<Ord>* fake_e = init_geom_ord();
  Ord o = bi_ord(1, &fake_wt, ou, ov, fake_e, NULL);
  int order = rrv1->get_inv_ref_order();
  order += o.get_order();
  limit_order(order);

  ou->free_ord(); delete ou;
  ov->free_ord(); delete ov;
  delete fake_e;

  // eval the form
  Quad2D* quad = rsln1->get_quad_2d();
  double3* pt = quad->get_points(order);
  int np = quad->get_num_points(order);

  // init geometry and jacobian*weights
  Geom<double>* e = init_geom_vol(rrv1, order);
  double* jac = rrv1->get_jacobian(order);
  double* jwt = new double[np];
  for(int i = 0; i < np; i++)
    jwt[i] = pt[i][2] * jac[i];

  // function values
  Func<scalar>* v1 = init_fn(rsln1, rrv1, order);
  Func<scalar>* v2 = init_fn(rsln2, rrv2, order);

  scalar res = bi_fn(np, jwt, v1, v2, e, NULL);

  e->free(); delete e;
  delete [] jwt;
  v1->free_fn(); delete v1;
  v2->free_fn(); delete v2;

  return res;
}


double H1AdaptHP::calc_error(MeshFunction* sln, MeshFunction* rsln)
{
  if (num != 1) error("Wrong number of solutions.");

  return calc_error_n(1, sln, rsln);
}


double H1AdaptHP::calc_error_2(MeshFunction* sln1, MeshFunction* sln2, MeshFunction* rsln1, MeshFunction* rsln2)
{
  if (num != 2) error("Wrong number of solutions.");

  return calc_error_n(2, sln1, sln2, rsln1, rsln2);
}


double H1AdaptHP::calc_error_n(int n, ...)
{
  int i, j;

  if (n != num) error("Wrong number of solutions.");

  // obtain solutions and bilinear forms
  va_list ap;
  va_start(ap, n);
  for (i = 0; i < n; i++) {
    sln[i] = va_arg(ap, Solution*); //?WTF: input of calc_error, which calls calc_error_n, is a type MeshFunction* that is parent of Solution*
    sln[i]->set_quad_2d(&g_quad_2d_std);
  }
  for (i = 0; i < n; i++) {
    rsln[i] = va_arg(ap, Solution*); //?WTF: input of calc_error, which calls calc_error_n, is a type MeshFunction* that is parent of Solution*
    rsln[i]->set_quad_2d(&g_quad_2d_std);
  }
  va_end(ap);

  // prepare multi-mesh traversal and error arrays
  AUTOLA_OR(Mesh*, meshes, 2*num);
  AUTOLA_OR(Transformable*, tr, 2*num);
  Traverse trav;
  nact = 0;
  for (i = 0; i < num; i++)
  {
    meshes[i] = sln[i]->get_mesh();
    meshes[i+num] = rsln[i]->get_mesh();
    tr[i] = sln[i];
    tr[i+num] = rsln[i];

    nact += sln[i]->get_mesh()->get_num_active_elements();

    int max = meshes[i]->get_max_element_id();
    if (errors[i] != NULL) delete [] errors[i];
    errors[i] = new double[max];
    memset(errors[i], 0, sizeof(double) * max);
  }

  double total_norm = 0.0;
  AUTOLA_OR(double, norms, num);
  memset(norms, 0, norms.size);
  double total_error = 0.0;

  Element** ee;
  trav.begin(2*num, meshes, tr);
  while ((ee = trav.get_next_state(NULL, NULL)) != NULL)
  {
    for (i = 0; i < num; i++)
    {
      RefMap* rmi = sln[i]->get_refmap();
      RefMap* rrmi = rsln[i]->get_refmap();
      for (j = 0; j < num; j++)
      {
        RefMap* rmj = sln[j]->get_refmap();
        RefMap* rrmj = rsln[j]->get_refmap();
        double e, t;
        if (form[i][j] != NULL)
        {
          #ifndef H2D_COMPLEX
          e = fabs(eval_error(form[i][j], ord[i][j], sln[i], sln[j], rsln[i], rsln[j], rmi, rmj, rrmi, rrmj));
          t = fabs(eval_norm(form[i][j], ord[i][j], rsln[i], rsln[j], rrmi, rrmj));
          #else
          e = std::abs(eval_error(form[i][j], ord[i][j], sln[i], sln[j], rsln[i], rsln[j], rmi, rmj, rrmi, rrmj));
          t = std::abs(eval_norm(form[i][j], ord[i][j], rsln[i], rsln[j], rrmi, rrmj));
          #endif

          norms[i] += t;
          total_norm  += t;
          total_error += e;
          errors[i][ee[i]->id] += e;
        }
      }
    }
  }
  trav.finish();

  //prepare an ordered list of elements according to an error
  sort_elements_by_error(meshes);

  have_errors = true;
  total_err = total_error/* / total_norm*/;
  return sqrt(total_error / total_norm);
}

void H1AdaptHP::sort_elements_by_error(Mesh** meshes) {
  //allocate
  if (esort != NULL)
    delete[] esort;
  esort = new ElementReference[nact];

  //prepare indices
  Element* e;
  int inx = 0;
  for (int i = 0; i < num; i++)
    for_all_active_elements(e, meshes[i]) {
      esort[inx].id = e->id;
      esort[inx].comp = i;
      inx++;
//       errors[i][e->id] /= norms[i];
// ??? needed or not ???
// when norms of 2 components are very different it can help (microwave heating)
// navier-stokes on different meshes work only without
    }

  //sort
  assert(inx == nact);
  cmp_err = errors;
  qsort(esort, nact, sizeof(ElementReference), compare);
}
