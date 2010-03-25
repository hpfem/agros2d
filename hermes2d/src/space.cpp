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
#include "space.h"
#include "matrix.h"
#include "auto_local_array.h"


Space::Space(Mesh* mesh, Shapeset* shapeset)
     : mesh(mesh), shapeset(shapeset)
{
  default_tri_order = -1;
  default_quad_order = -1;
  ndata = NULL;
  edata = NULL;
  nsize = esize = 0;
  ndata_allocated = 0;
  mesh_seq = -1;
  seq = 0;
  was_assigned = false;

  set_bc_types(NULL);
  set_bc_values((scalar (*)(int, double, double)) NULL);
  set_bc_values((scalar (*)(EdgePos*)) NULL);
}


Space::~Space()
{
  free();
}


void Space::free()
{
  free_extra_data();
  if (nsize) { ::free(ndata); nsize = 0; }
  if (esize) { ::free(edata); esize = 0; }
}


//// element orders ///////////////////////////////////////////////////////////////////////////////

void Space::resize_tables()
{
  if (nsize < mesh->get_max_node_id())
  {
    //HACK: definition of allocated size and the result number of elements
    nsize = mesh->get_max_node_id();
    if (nsize > ndata_allocated)
    {
      int prev_allocated = ndata_allocated;
      if (ndata_allocated == 0)
        ndata_allocated = 1024;
      while (ndata_allocated < nsize)
        ndata_allocated = ndata_allocated * 3 / 2;
      ndata = (NodeData*)realloc(ndata, ndata_allocated * sizeof(NodeData));
      for(int i = prev_allocated; i < ndata_allocated; i++)
        ndata[i].edge_bc_proj = NULL;
    }
  }

  if (esize < mesh->get_max_element_id())
  {
    int oldsize = esize;
    if (!esize) esize = 1024;
    while (esize < mesh->get_max_element_id()) esize = esize * 3 / 2;
    edata = (ElementData*) realloc(edata, sizeof(ElementData) * esize);
    for (int i = oldsize; i < esize; i++)
      edata[i].order = -1;
  }
}


void Space::check_order(int order)
{
  if (get_h_order(order) < 0 || get_v_order(order) < 0)
    error("order cannot be negative.");
  if (get_h_order(order) > 10 || get_v_order(order) > 10)
    error("order = %d, maximum is 10.", order);
}


void Space::set_element_order(int id, int order)
{
  if (id < 0 || id >= mesh->get_max_element_id())
    error("invalid element id.");
  check_order(order);

  resize_tables();
  if (mesh->get_element(id)->is_quad() && get_v_order(order) == 0)
     order = make_quad_order(order, order);
  edata[id].order = order;
  seq++;
}


int Space::get_element_order(int id) const
{
  if (id >= esize) return 0;
  return edata[id].order;
}


void Space::set_uniform_order(int order, int marker)
{
  resize_tables();
  check_order(order);
  int quad_order = make_quad_order(order, order);

  Element* e;
  for_all_active_elements(e, mesh)
  {
    if (marker == ANY || e->marker == marker)
    {
      ElementData* ed = &edata[e->id];
      if (e->is_triangle())
        ed->order = order;
      else
        ed->order = quad_order;
    }
  }
  seq++;
}


void Space::set_default_order(int tri_order, int quad_order)
{
  if (quad_order == 0) quad_order = make_quad_order(tri_order, tri_order);
  default_tri_order = tri_order;
  default_quad_order = quad_order;
}


void Space::copy_orders_recurrent(Element* e, int order)
{
  if (e->active)
    edata[e->id].order = order;
  else
    for (int i = 0; i < 4; i++)
      if (e->sons[i] != NULL)
        copy_orders_recurrent(e->sons[i], order);
}


void Space::copy_orders(Space* space, int inc)
{
  Element* e;
  resize_tables();
  for_all_active_elements(e, space->get_mesh())
  {
    int oo = space->get_element_order(e->id);
    if (oo < 0) error("Source space has an uninitialized order (element id = %d)", e->id);

    int mo = shapeset->get_max_order();
    int ho = std::max(1, std::min(get_h_order(oo) + inc, mo));
    int vo = std::max(1, std::min(get_v_order(oo) + inc, mo));
    oo = e->is_triangle() ? ho : make_quad_order(ho, vo);

    check_order(oo);
    copy_orders_recurrent(mesh->get_element/*sic!*/(e->id), oo);
  }
  seq++;
}


int Space::get_edge_order(Element* e, int edge)
{
  Node* en = e->en[edge];
  if (en->id >= nsize || edge >= (int)e->nvert) return 0;

  if (ndata[en->id].n == -1)
    return get_edge_order_internal(ndata[en->id].base); // constrained node
  else
    return get_edge_order_internal(en);
}


int Space::get_edge_order_internal(Node* en)
{
  assert(en->type == TYPE_EDGE);
  Element** e = en->elem;
  int o1 = 1000, o2 = 1000;
  assert(e[0] != NULL || e[1] != NULL);

  if (e[0] != NULL)
  {
    if (e[0]->is_triangle() || en == e[0]->en[0] || en == e[0]->en[2])
      o1 = get_h_order(edata[e[0]->id].order);
    else
      o1 = get_v_order(edata[e[0]->id].order);
  }

  if (e[1] != NULL)
  {
    if (e[1]->is_triangle() || en == e[1]->en[0] || en == e[1]->en[2])
      o2 = get_h_order(edata[e[1]->id].order);
    else
      o2 = get_v_order(edata[e[1]->id].order);
  }

  if (o1 == 0) return o2 == 1000 ? 0 : o2;
  if (o2 == 0) return o1 == 1000 ? 0 : o1;
  return std::min(o1, o2);
}


void Space::set_mesh(Mesh* mesh)
{
  if (this->mesh == mesh) return;
  free();
  this->mesh = mesh;
  seq++;
}


void Space::propagate_zero_orders(Element* e)
{
  set_element_order(e->id, 0);
  if (!e->active)
    for (int i = 0; i < 4; i++)
      if (e->sons[i] != NULL)
        propagate_zero_orders(e->sons[i]);
}


void Space::distribute_orders(Mesh* mesh, int* parents)
{
  int num = mesh->get_max_element_id();
  AUTOLA_OR(int, orders, num+1);
  Element* e;
  for_all_active_elements(e, mesh)
  {
    int p = get_element_order(parents[e->id]);
    if (e->is_triangle() && (get_v_order(p) != 0))
      p = std::max(get_h_order(p), get_v_order(p));
    orders[e->id] = p;
  }
  for_all_active_elements(e, mesh)
    set_element_order(e->id, orders[e->id]);

}


//// dof assignment ////////////////////////////////////////////////////////////////////////////////

int Space::assign_dofs(int first_dof, int stride)
{
  if (first_dof < 0) error("Invalid first_dof.");
  if (stride < 1)    error("Invalid stride.");

  resize_tables();

  Element* e;
  for_all_base_elements(e, mesh)
    if (get_element_order(e->id) == 0)
      propagate_zero_orders(e);

  for_all_active_elements(e, mesh)
    if (e->id >= esize || edata[e->id].order < 0)
      error("Uninitialized element order (id = #%d).", e->id);

  this->first_dof = next_dof = first_dof;
  this->stride = stride;

  assign_vertex_dofs();
  assign_edge_dofs();
  assign_bubble_dofs();

  free_extra_data();
  update_bc_dofs();
  update_constraints();
  post_assign();

  mesh_seq = mesh->get_seq();
  was_assigned = true;
  seq++;
  return get_num_dofs();
}


//// assembly lists ///////////////////////////////////////////////////////////////////////////////

void AsmList::enlarge()
{
  cap = !cap ? 256 : cap * 2;
  idx = (int*) realloc(idx, sizeof(int) * cap);
  dof = (int*) realloc(dof, sizeof(int) * cap);
  coef = (scalar*) realloc(coef, sizeof(scalar) * cap);
}


void Space::get_element_assembly_list(Element* e, AsmList* al)
{
  // some checks
  if (e->id >= esize || edata[e->id].order < 0)
    error("Uninitialized element order (id = #%d).", e->id);
  if (!is_up_to_date())
    error("The space is out of date. You need to update it with assign_dofs()"
          " any time the mesh changes.");

  // add vertex, edge and bubble functions to the assembly list
  al->clear();
  shapeset->set_mode(e->get_mode());
  for (unsigned int i = 0; i < e->nvert; i++)
    get_vertex_assembly_list(e, i, al);
  for (unsigned int i = 0; i < e->nvert; i++)
    get_edge_assembly_list_internal(e, i, al);
  get_bubble_assembly_list(e, al);
}


void Space::get_edge_assembly_list(Element* e, int edge, AsmList* al)
{
  al->clear();
  shapeset->set_mode(e->get_mode());
  get_vertex_assembly_list(e, edge, al);
  get_vertex_assembly_list(e, e->next_vert(edge), al);
  get_edge_assembly_list_internal(e, edge, al);
}


//// BC stuff /////////////////////////////////////////////////////////////////////////////////////

static int default_bc_type(int marker)
{
  return BC_NATURAL;
}

static scalar default_bc_value_by_coord(int marker, double x, double y)
{
  return 0;
}

scalar default_bc_value_by_edge(EdgePos* ep)
{
  double x, y;
  Nurbs* nurbs = ep->base->is_curved() ? ep->base->cm->nurbs[ep->edge] : NULL;
  nurbs_edge(ep->base, nurbs, ep->edge, 2.0*ep->t - 1.0, x, y);
  return ep->space->bc_value_callback_by_coord(ep->marker, x, y);
}


void Space::set_bc_types(int (*bc_type_callback)(int))
{
  if (bc_type_callback == NULL) bc_type_callback = default_bc_type;
  this->bc_type_callback = bc_type_callback;
  seq++;
}

void Space::set_bc_values(scalar (*bc_value_callback_by_coord)(int, double, double))
{
  if (bc_value_callback_by_coord == NULL) bc_value_callback_by_coord = default_bc_value_by_coord;
  this->bc_value_callback_by_coord = bc_value_callback_by_coord;
  seq++;
}

void Space::set_bc_values(scalar (*bc_value_callback_by_edge)(EdgePos*))
{
  if (bc_value_callback_by_edge == NULL) bc_value_callback_by_edge = default_bc_value_by_edge;
  this->bc_value_callback_by_edge = bc_value_callback_by_edge;
  seq++;
}


void Space::copy_callbacks(const Space* space)
{
  bc_type_callback = space->bc_type_callback;
  bc_value_callback_by_coord = space->bc_value_callback_by_coord;
  bc_value_callback_by_edge  = space->bc_value_callback_by_edge;
}


void Space::precalculate_projection_matrix(int nv, double**& mat, double*& p)
{
  int n = shapeset->get_max_order() + 1 - nv;
  mat = new_matrix<double>(n, n);
  int component = get_type() == 2 ? 1 : 0;

  Quad1DStd quad1d;
  //shapeset->set_mode(MODE_TRIANGLE);
  shapeset->set_mode(MODE_QUAD);
  for (int i = 0; i < n; i++)
  {
    for (int j = i; j < n; j++)
    {
      int o = i + j + 4;
      double2* pt = quad1d.get_points(o);
      int ii = shapeset->get_edge_index(0, 0, i + nv);
      int ij = shapeset->get_edge_index(0, 0, j + nv);
      double val = 0.0;
      for (int k = 0; k < quad1d.get_num_points(o); k++)
      {
        val += pt[k][1] * shapeset->get_fn_value(ii, pt[k][0], -1.0, component)
                        * shapeset->get_fn_value(ij, pt[k][0], -1.0, component);
      }
      mat[i][j] = val;
    }
  }

  p = new double[n];
  choldc(mat, n, p);
}


void Space::update_edge_bc(Element* e, EdgePos* ep)
{
  const int UNASSIGNED = -2;
  if (e->active)
  {
    Node* en = e->en[ep->edge];
    NodeData* nd = &ndata[en->id];
    nd->edge_bc_proj = NULL;

    if (nd->dof != UNASSIGNED && en->bnd && bc_type_callback(en->marker) == BC_ESSENTIAL)
    {
      int order = get_edge_order_internal(en);
      ep->marker = en->marker;
      nd->edge_bc_proj = get_bc_projection(ep, order);
      extra_data.push_back(nd->edge_bc_proj);

      int i = ep->edge, j = e->next_vert(i);
      ndata[e->vn[i]->id].vertex_bc_coef = nd->edge_bc_proj + 0;
      ndata[e->vn[j]->id].vertex_bc_coef = nd->edge_bc_proj + 1;
    }
  }
  else
  {
    int son1, son2;
    if (mesh->get_edge_sons(e, ep->edge, son1, son2) == 2)
    {
      double mid = (ep->lo + ep->hi) * 0.5, tmp = ep->hi;
      ep->hi = mid;
      update_edge_bc(e->sons[son1], ep);
      ep->lo = mid; ep->hi = tmp;
      update_edge_bc(e->sons[son2], ep);
    }
    else
      update_edge_bc(e->sons[son1], ep);
  }
}


void Space::update_bc_dofs()
{
  Element* e;
  for_all_base_elements(e, mesh)
  {
    for (unsigned int i = 0; i < e->nvert; i++)
    {
      int j = e->next_vert(i);
      if (e->vn[i]->bnd && e->vn[j]->bnd)
      {
        EdgePos ep = { e->vn[i]->id, e->vn[j]->id, 0, i, 0.0, 0.0, 1.0, e, this, NULL, NULL };
        update_edge_bc(e, &ep);
      }
    }
  }
}


void Space::free_extra_data()
{
  for (unsigned int i = 0; i < extra_data.size(); i++)
    delete [] (scalar*) extra_data[i];
  extra_data.clear();
}

/*void Space::dump_node_info()
{
  Node* n;
  for_all_nodes(n, mesh)
  {
    NodeData* nd = &ndata[n->id];
    if (n->type == TYPE_VERTEX)
    {
      printf("vert node id=%d ref=%d bnd=%d x=%g y=%g dof=%d n=%d ",
             n->id, n->ref, n->bnd, n->x, n->y, nd->dof, nd->n);
      if (nd->dof < 0)
        printf("coef=%g", nd->vertex_bc_coef[0]);
    }
    else
    {
      printf("edge node id=%d ref=%d bnd=%d marker=%d p1=%d p2=%d dof=%d n=%d ",
             n->id, n->ref, n->bnd, n->marker, n->p1, n->p2, nd->dof, nd->n);
      if (nd->dof < 0)
        for (int i = 0; i < nd->n; i++)
          printf("proj[%d]=%g ", i, nd->edge_bc_proj[i+2]);
    }
    printf("\n");
  }
}*/
