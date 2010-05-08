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
#include "filter.h"
#include "traverse.h"


//// Filter ////////////////////////////////////////////////////////////////////////////////////////

Filter::Filter(MeshFunction* sln1) : MeshFunction()
{
  num = 1;
  sln[0] = sln1;
  init();
}

Filter::Filter(MeshFunction* sln1, MeshFunction* sln2) : MeshFunction()
{
  num = 2;
  sln[0] = sln1;  sln[1] = sln2;
  init();
}

Filter::Filter(MeshFunction* sln1, MeshFunction* sln2, MeshFunction* sln3) : MeshFunction()
{
  num = 3;
  sln[0] = sln1;  sln[1] = sln2;  sln[2] = sln3;
  init();
}

Filter::Filter(MeshFunction* sln1, MeshFunction* sln2, MeshFunction* sln3, MeshFunction* sln4) : MeshFunction()
{
  num = 4;
  sln[0] = sln1;  sln[1] = sln2;  sln[2] = sln3;  sln[3] = sln4;
  init();
}


void Filter::init()
{
  // create the array sln_nodup, which is the array sln with duplicities removed
  /*sln_nodup[0] = sln[0]; nnd = 1;
  if (num > 1 && sln[1] != sln[0]) sln_nodup[nnd++] = sln[1];
  if (num > 2 && sln[2] != sln[0] && sln[2] != sln[1]) sln_nodup[nnd++] = sln[2];*/

  // construct the union mesh, if necessary
  Mesh* meshes[4] = {              sln[0]->get_mesh(),
                      (num >= 2) ? sln[1]->get_mesh() : NULL,
                      (num >= 3) ? sln[2]->get_mesh() : NULL,
                      (num >= 4) ? sln[3]->get_mesh() : NULL };

  mesh = meshes[0];
  unimesh = false;

  for (int i = 1; i < num; i++)
    if (meshes[i]->get_seq() != mesh->get_seq())
      { unimesh = true; break; }

  if (unimesh)
  {
    Traverse trav;
    trav.begin(num, meshes);
    mesh = new Mesh;
    unidata = trav.construct_union_mesh(mesh);
    trav.finish();
  }

  // misc init
  num_components = 1;
  order = 0;
  memset(tables, 0, sizeof(tables));
  memset(sln_sub, 0, sizeof(sln_sub));
  set_quad_2d(&g_quad_2d_std);
}


Filter::~Filter()
{
  free();
  if (unimesh)
  {
    delete mesh;
    for (int i = 0; i < num; i++)
      delete [] unidata[i];
    delete [] unidata;
  }
}


void Filter::set_quad_2d(Quad2D* quad_2d)
{
  MeshFunction::set_quad_2d(quad_2d);
  for (int i = 0; i < num; i++)
    sln[i]->set_quad_2d(quad_2d); // nodup
}


void Filter::set_active_element(Element* e)
{
  MeshFunction::set_active_element(e);
  if (!unimesh)
  {
    for (int i = 0; i < num; i++)
      sln[i]->set_active_element(e); // nodup
    memset(sln_sub, 0, sizeof(sln_sub));
  }
  else
  {
    for (int i = 0; i < num; i++) {
      sln[i]->set_active_element(unidata[i][e->id].e);
      sln[i]->set_transform(unidata[i][e->id].idx);
      sln_sub[i] = sln[i]->get_transform();
    }
  }

  if (tables[cur_quad] != NULL) free_sub_tables(&(tables[cur_quad]));
  sub_tables = &(tables[cur_quad]);
  update_nodes_ptr();

  order = 20; // fixme
}


void Filter::free()
{
  for (int i = 0; i < 4; i++)
    if (tables[i] != NULL)
      free_sub_tables(&(tables[i]));
}


void Filter::reinit()
{
  free();
  init();
}


void Filter::push_transform(int son)
{
  MeshFunction::push_transform(son);
  for (int i = 0; i < num; i++)
  {
    // sln_sub[i] contains the value sln[i]->sub_idx, which the Filter thinks
    // the solution has, or at least had last time. If the filter graph is
    // cyclic, it could happen that some solutions would get pushed the transform
    // more than once. This mechanism prevents it. If the filter sees that the
    // solution already has a different sub_idx than it thinks it should have,
    // it assumes someone else has already pushed the correct transform. This
    // is actually the case for cyclic filter graphs and filters used in multi-mesh
    // computation.

    if (sln[i]->get_transform() == sln_sub[i])
      sln[i]->push_transform(son);
    sln_sub[i] = sln[i]->get_transform();
  }
}


void Filter::pop_transform()
{
  MeshFunction::pop_transform();
  for (int i = 0; i < num; i++)
  {
    if (sln[i]->get_transform() == sln_sub[i])
      sln[i]->pop_transform();
    sln_sub[i] = sln[i]->get_transform();
  }
}


//// SimpleFilter //////////////////////////////////////////////////////////////////////////////////

SimpleFilter::SimpleFilter(void (*filter_fn)(int n, scalar* val1, scalar* result),
                           MeshFunction* sln1, int item1)
            : Filter(sln1)
{
  item[0] = item1;
  filter_fn_1 = filter_fn;
  init_components();
}

SimpleFilter::SimpleFilter(void (*filter_fn)(int n, scalar* val1, scalar* val2, scalar* result),
                           MeshFunction* sln1, MeshFunction* sln2, int item1, int item2)
            : Filter(sln1, sln2)
{
  item[0] = item1;
  item[1] = item2;
  filter_fn_2 = filter_fn;
  init_components();
}

SimpleFilter::SimpleFilter(void (*filter_fn)(int n, scalar* val1, scalar* val2, scalar* val3, scalar* result),
                           MeshFunction* sln1, MeshFunction* sln2, MeshFunction* sln3, int item1, int item2, int item3)
            : Filter(sln1, sln2, sln3)
{
  item[0] = item1;
  item[1] = item2;
  item[2] = item3;
  filter_fn_3 = filter_fn;
  init_components();
}

SimpleFilter::SimpleFilter(void (*filter_fn)(int n, scalar* val1, scalar* val2, scalar* val3, scalar* val4, scalar* result),
                           MeshFunction* sln1, MeshFunction* sln2, MeshFunction* sln3, MeshFunction* sln4,
                           int item1, int item2, int item3, int item4)
            : Filter(sln1, sln2, sln3, sln4)
{
  item[0] = item1;
  item[1] = item2;
  item[2] = item3;
  item[3] = item4;
  filter_fn_4 = filter_fn;
  init_components();
}


void SimpleFilter::init_components()
{
  bool vec1 = false, vec2 = false;
  for (int i = 0; i < num; i++)
  {
    if (sln[i]->get_num_components() > 1) vec1 = true;
    if ((item[i] & H2D_FN_COMPONENT_0) && (item[i] & H2D_FN_COMPONENT_1)) vec2 = true;
    if (sln[i]->get_num_components() == 1) item[i] &= H2D_FN_COMPONENT_0;
  }
  num_components = (vec1 && vec2) ? 2 : 1;
}


void SimpleFilter::precalculate(int order, int mask)
{
  if (mask & (H2D_FN_DX | H2D_FN_DY | H2D_FN_DXX | H2D_FN_DYY | H2D_FN_DXY))
    error("Filter not defined for derivatives.");

  Quad2D* quad = quads[cur_quad];
  int np = quad->get_num_points(order);
  Node* node = new_node(H2D_FN_VAL, np);

  // precalculate all solutions
  for (int i = 0; i < num; i++)
    sln[i]->set_quad_order(order, item[i]);

  for (int j = 0; j < num_components; j++)
  {
    // obtain corresponding tables
    scalar* tab[4];
    for (int i = 0; i < num; i++)
    {
      int a = 0, b = 0, mask = item[i];
      if (mask >= 0x40) { a = 1; mask >>= 6; }
      while (!(mask & 1)) { mask >>= 1; b++; }
      tab[i] = sln[i]->get_values(num_components == 1 ? a : j, b);
      if (tab[i] == NULL) error("Value of 'item%d' is incorrect in filter definition.", i+1);
    }

    // apply the filter
    switch (num)
    {
      case 1: filter_fn_1(np, tab[0], node->values[j][0]); break;
      case 2: filter_fn_2(np, tab[0], tab[1], node->values[j][0]); break;
      case 3: filter_fn_3(np, tab[0], tab[1], tab[2], node->values[j][0]); break;
      case 4: filter_fn_4(np, tab[0], tab[1], tab[2], tab[3], node->values[j][0]); break;
      default: assert(0);
    }
  }

  // remove the old node and attach the new one
  replace_cur_node(node);
}

scalar SimpleFilter::get_pt_value(double x, double y, int it)
{
  if (it & (H2D_FN_DX | H2D_FN_DY | H2D_FN_DXX | H2D_FN_DYY | H2D_FN_DXY))
    error("Filter not defined for derivatives.");
  scalar val[4];
  for (int i = 0; i < num; i++)
    val[i] = sln[i]->get_pt_value(x, y, item[i]);

  scalar result;

  switch (num)
  {
    case 1: filter_fn_1(1, &val[0], &result); break;
    case 2: filter_fn_2(1, &val[0], &val[1], &result); break;
    case 3: filter_fn_3(1, &val[0], &val[1], &val[2], &result); break;
    case 4: filter_fn_4(1, &val[0], &val[1], &val[2], &val[3], &result); break;
    default: assert(0);
  }

  return result;
}

//// DXDYFilter ////////////////////////////////////////////////////////////////////////////////////

DXDYFilter::DXDYFilter(filter_fn_1_t fn, MeshFunction* sln1)
          : Filter(sln1)
{
  filter_fn_1 = fn;
  init_components();
}

DXDYFilter::DXDYFilter(filter_fn_2_t fn, MeshFunction* sln1, MeshFunction* sln2)
          : Filter(sln1, sln2)
{
  filter_fn_2 = fn;
  init_components();
}

DXDYFilter::DXDYFilter(filter_fn_3_t fn, MeshFunction* sln1, MeshFunction* sln2, MeshFunction* sln3)
          : Filter(sln1, sln2, sln3)
{
  filter_fn_3 = fn;
  init_components();
}

DXDYFilter::DXDYFilter(filter_fn_4_t fn, MeshFunction* sln1, MeshFunction* sln2, MeshFunction* sln3, MeshFunction* sln4)
          : Filter(sln1, sln2, sln3, sln4)
{
  filter_fn_4 = fn;
  init_components();
}


void DXDYFilter::init_components()
{
  num_components = sln[0]->get_num_components();
  for (int i = 1; i < num; i++)
    if (sln[i]->get_num_components() != num_components)
      error("Filter: Solutions do not have the same number of components!");
}


void DXDYFilter::precalculate(int order, int mask)
{
  Quad2D* quad = quads[cur_quad];
  int np = quad->get_num_points(order);
  Node* node = new_node(H2D_FN_DEFAULT, np);

  // precalculate all solutions
  for (int i = 0; i < num; i++)
    sln[i]->set_quad_order(order, H2D_FN_DEFAULT);

  for (int j = 0; j < num_components; j++)
  {
    // obtain solution tables
    scalar *val[4], *dx[4], *dy[4];
    for (int i = 0; i < num; i++)
    {
      val[i] = sln[i]->get_fn_values(j);
      dx[i]  = sln[i]->get_dx_values(j);
      dy[i]  = sln[i]->get_dy_values(j);
    }

    // apply the filter
    switch (num)
    {
      case 1:
        filter_fn_1(np, val[0], dx[0], dy[0],
                    node->values[j][0], node->values[j][1], node->values[j][2]);
        break;
      case 2:
        filter_fn_2(np, val[0], dx[0], dy[0], val[1], dx[1], dy[1],
                    node->values[j][0], node->values[j][1], node->values[j][2]);
        break;
      case 3:
        filter_fn_3(np, val[0], dx[0], dy[0], val[1], dx[1], dy[1], val[2], dx[2], dy[2],
                    node->values[j][0], node->values[j][1], node->values[j][2]);
        break;
      case 4:
        filter_fn_4(np, val[0], dx[0], dy[0], val[1], dx[1], dy[1], val[2], dx[2], dy[2], val[3], dx[3], dy[3],
                    node->values[j][0], node->values[j][1], node->values[j][2]);
        break;
      default:
        assert(0);
    }
  }

  // remove the old node and attach the new one
  replace_cur_node(node);
}


//// predefined simple filters /////////////////////////////////////////////////////////////////////

static void magnitude_fn_2(int n, scalar* v1, scalar* v2, scalar* result)
{
  for (int i = 0; i < n; i++)
    result[i] = sqrt(sqr(v1[i]) + sqr(v2[i]));
}

MagFilter::MagFilter(MeshFunction* sln1, MeshFunction* sln2, int item1, int item2)
         : SimpleFilter(magnitude_fn_2, sln1, sln2, item1, item2) {}

MagFilter::MagFilter(MeshFunction* sln1, int item1)
         : SimpleFilter(magnitude_fn_2, sln1, sln1, item1 & H2D_FN_COMPONENT_0, item1 & H2D_FN_COMPONENT_1)
{
  if (sln1->get_num_components() < 2)
    error("The single-argument constructor is intended for vector-valued solutions.");
}


static void difference_fn_2(int n, scalar* v1, scalar* v2, scalar* result)
{
  for (int i = 0; i < n; i++)
    result[i] = v1[i] - v2[i];
}

DiffFilter::DiffFilter(MeshFunction* sln1, MeshFunction* sln2, int item1, int item2)
          : SimpleFilter(difference_fn_2, sln1, sln2, item1, item2) {}


static void sum_fn_2(int n, scalar* v1, scalar* v2, scalar* result)
{
  for (int i = 0; i < n; i++)
    result[i] = v1[i] + v2[i];
}

SumFilter::SumFilter(MeshFunction* sln1, MeshFunction* sln2, int item1, int item2)
          : SimpleFilter(sum_fn_2, sln1, sln2, item1, item2) {}


static void square_fn_1(int n, scalar* v1, scalar* result)
{
#ifdef H2D_COMPLEX
  for (int i = 0; i < n; i++)
    result[i] = std::norm(v1[i]);
#else
  for (int i = 0; i < n; i++)
    result[i] = sqr(v1[i]);
#endif
}

SquareFilter::SquareFilter(MeshFunction* sln1, int item1)
          : SimpleFilter(square_fn_1, sln1, item1) {}


static void real_part_fn_1(int n, scalar* v1, scalar* result)
{
#ifndef H2D_COMPLEX
  memcpy(result, v1, sizeof(scalar) * n);
#else
  for (int i = 0; i < n; i++)
    result[i] = v1[i].real();
#endif
}

RealFilter::RealFilter(MeshFunction* sln1, int item1)
          : SimpleFilter(real_part_fn_1, sln1, item1) {}


static void imag_part_fn_1(int n, scalar* v1, scalar* result)
{
#ifndef H2D_COMPLEX
  memset(result, 0, sizeof(scalar) * n);
#else
  for (int i = 0; i < n; i++)
    result[i] = v1[i].imag();
#endif
}

ImagFilter::ImagFilter(MeshFunction* sln1, int item1)
          : SimpleFilter(imag_part_fn_1, sln1, item1) {}


static void abs_fn_1(int n, scalar* v1, scalar* result)
{
#ifndef H2D_COMPLEX
  for (int i = 0; i < n; i++)
    result[i] = fabs(v1[i]);
#else
  for (int i = 0; i < n; i++)
    result[i] = sqrt(sqr(v1[i].real()) + sqr(v1[i].imag()));
#endif
}

AbsFilter::AbsFilter(MeshFunction* sln1, int item1)
          : SimpleFilter(abs_fn_1, sln1, item1) {}


static void angle_fn_1(int n, scalar* v1, scalar* result)
{
#ifndef H2D_COMPLEX
  for (int i = 0; i < n; i++)
    result[i] = 0.0;
#else
  for (int i = 0; i < n; i++)
    result[i] = atan2( v1[i].imag(), v1[i].real() );
#endif
}

AngleFilter::AngleFilter(MeshFunction* sln1, int item1)
  : SimpleFilter(angle_fn_1, sln1, item1) {}


//// VonMisesFilter ////////////////////////////////////////////////////////////////////////////////

#ifndef H2D_COMPLEX
  #define getval(exp) (exp)
#else
  #define getval(exp) (exp.real())
#endif


void VonMisesFilter::precalculate(int order, int mask)
{
  if (mask & (H2D_FN_DX | H2D_FN_DY | H2D_FN_DXX | H2D_FN_DYY | H2D_FN_DXY))
    error("VonMisesFilter not defined for derivatives.");

  Quad2D* quad = quads[cur_quad];
  int np = quad->get_num_points(order);
  Node* node = new_node(H2D_FN_VAL_0, np);

  sln[0]->set_quad_order(order, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
  sln[1]->set_quad_order(order, H2D_FN_DX | H2D_FN_DY);

  scalar *dudx, *dudy, *dvdx, *dvdy;
  sln[0]->get_dx_dy_values(dudx, dudy);
  sln[1]->get_dx_dy_values(dvdx, dvdy);
  scalar *uval = sln[0]->get_fn_values();
  update_refmap();
  double *x = refmap->get_phys_x(order);


  for (int i = 0; i < np; i++)
  {
    // stress tensor
    double tz = lambda*(getval(dudx[i]) + getval(dvdy[i]));
    double tx = tz + 2*mu*getval(dudx[i]);
    double ty = tz + 2*mu*getval(dvdy[i]);
    if (cyl) tz += 2*mu*getval(uval[i]) / x[i];
    double txy = mu*(getval(dudy[i]) + getval(dvdx[i]));

    // Von Mises stress
    node->values[0][0][i] = 1.0/sqrt(2.0) * sqrt(sqr(tx - ty) + sqr(ty - tz) + sqr(tz - tx) + 6*sqr(txy));
  }

  // remove the old node and attach the new one
  replace_cur_node(node);
}


VonMisesFilter::VonMisesFilter(MeshFunction* sln1, MeshFunction* sln2, double lambda, double mu,
                               int cyl, int item1, int item2)
       : Filter(sln1, sln2)
{
  this->mu = mu;
  this->lambda = lambda;
  this->cyl = cyl;
  this->item1 = item1;
  this->item2 = item2;
}

//// LinearFilter //////////////////////////////////////////////////////////////////////////////////


void LinearFilter::precalculate(int order, int mask)
{
  Quad2D* quad = quads[cur_quad];
  int np = quad->get_num_points(order);
  Node* node = new_node(H2D_FN_DEFAULT, np);

  // precalculate all solutions
  for (int i = 0; i < num; i++)
    sln[i]->set_quad_order(order);

  for (int j = 0; j < num_components; j++)
  {
    // obtain solution tables
    scalar *val[4], *dx[4], *dy[4];
    for (int i = 0; i < num; i++)
    {
      val[i] = sln[i]->get_fn_values(j);
      dx[i]  = sln[i]->get_dx_values(j);
      dy[i]  = sln[i]->get_dy_values(j);
    }
    if (num == 2)
      for (int i = 0; i < np; i++)
      {
        node->values[j][0][i] = tau_frac * (val[1][i] - val[0][i]) + val[1][i];
        node->values[j][1][i] = tau_frac * (dx[1][i]  - dx[0][i])  + dx[1][i];
        node->values[j][2][i] = tau_frac * (dy[1][i]  - dy[0][i])  + dy[1][i];
      }
    else
      for (int i = 0; i < np; i++)
      {
        node->values[j][0][i] = val[0][i];
        node->values[j][1][i] = dx[0][i];
        node->values[j][2][i] = dy[0][i];
      }

  }
  // remove the old node and attach the new one
  replace_cur_node(node);
}


LinearFilter::LinearFilter(MeshFunction* old)
          : Filter(old)
 {
   init_components();
 }

LinearFilter::LinearFilter(MeshFunction* older, MeshFunction* old, double tau_frac)
          : Filter(older, old)
 {
   this->tau_frac = tau_frac;
   init_components();
 }


void LinearFilter::init_components()
{
  num_components = sln[0]->get_num_components();
  for (int i = 1; i < num; i++)
    if (sln[i]->get_num_components() != num_components)
      error("Filter: Solutions do not have the same number of components!");
}


void LinearFilter::set_active_element(Element* e)
{
  Filter::set_active_element(e);

  order = 0;
  for (int i = 0; i < num; i++)
  {
    int o = sln[i]->get_fn_order();
    if (o > order) order = o;
  }
}
