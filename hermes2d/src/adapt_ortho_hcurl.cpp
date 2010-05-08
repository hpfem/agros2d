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
#include "shapeset_hc_all.h"
#include "quad_all.h"
#include "linsystem.h"
#include "integrals_hcurl.h"
#include "adapt_ortho_hcurl.h"
#include "traverse.h"


#ifdef H2D_COMPLEX


HcurlOrthoHP::HcurlOrthoHP(int num, ...)
{
  this->num = num;

  va_list ap;
  va_start(ap, num);
  for (int i = 0; i < num; i++)
    spaces[i] = va_arg(ap, Space*);
  va_end(ap);

  for (int i = 0; i < num; i++)
    for (int j = 0; j < num; j++)
    {
      if (i == j) {
        form[i][j] = hcurl_form<double, scalar>;
        ord[i][j]  = hcurl_form<Ord, Ord>;
      }
      else {
        form[i][j] = NULL;
        ord[i][j]  = NULL;
      }
    }

  memset(errors, 0, sizeof(errors));
  esort = NULL;
  have_errors = false;
  kappa = 1.0;
}


HcurlOrthoHP::~HcurlOrthoHP()
{
  for (int i = 0; i < num; i++)
    if (errors[i] != NULL)
      delete [] errors[i];

  if (esort != NULL) delete [] esort;
}


//// orthonormal base construction /////////////////////////////////////////////////////////////////

double** HcurlOrthoHP::obase_0[2][9];  // first component
double** HcurlOrthoHP::obase_1[2][9];  // second component
double** HcurlOrthoHP::obase_c[2][9];  // curl
int HcurlOrthoHP::basecnt[2][11];
bool HcurlOrthoHP::obase_ready = false;


void HcurlOrthoHP::calc_ortho_base()
{
  int i, j, k, l, m, ii, nb, np;
  int n, idx[220];

  HcurlShapesetGradLeg shapeset;

  // allocate the orthonormal base tables - these are simply the values of the
  // orthonormal functions in integration points; we store the basic functions
  // plus eight son cut-outs of them (4 square sons, 2 + 2 aniso quad sons and the
  // whole, i.e. 9 times)
  for (i = 0; i < 9; i++)
  {
    if (i < 4 || i >= 8) {
      obase_0[0][i] = new_matrix<double>(110, 79); // tri
      obase_1[0][i] = new_matrix<double>(110, 79); // tri
      obase_c[0][i] = new_matrix<double>(110, 79); // tri
    }
    obase_0[1][i] = new_matrix<double>(220, 121); // quad
    obase_1[1][i] = new_matrix<double>(220, 121); // quad
    obase_c[1][i] = new_matrix<double>(220, 121); // quad
  }

  // repeat for triangles and quads
  for (m = 0; m <= 1; m++)
  {
    shapeset.set_mode(m);

/*    if (m == 0) verbose("Orthonormalizing triangular basis. ");
    else verbose("Orthonormalizing quadrilateral basis. ");*/

    // obtain a list of all shape functions up to the order 9, from lowest to highest order
    n = 0;
    int nv = m ? 4 : 3;
    int num_sons = m ? 8 : 4;

    basecnt[m][0] = 0;
    for (i = 0; i <= 9; i++)
    {
      for (j = 0; j < nv; j++)
        idx[n++] = shapeset.get_edge_index(j, 0, i);

      ii = m ? H2D_MAKE_QUAD_ORDER(i, i) : i;
      nb = shapeset.get_num_bubbles(ii);
      int* bub = shapeset.get_bubble_indices(ii);
      for (j = 0; j < nb; j++)
      {
        for (k = 0; k < n; k++)
        {
          if (bub[j] == idx[k])
            break;
        }
        if (k == n) idx[n++] = bub[j];
      }
      basecnt[m][i+1] = n;
    }

    // obtain their values for integration rule 20
    g_quad_2d_std.set_mode(m);
    np = g_quad_2d_std.get_num_points(20);
    double3* pt = g_quad_2d_std.get_points(20);

    for (i = 0; i < n; i++)
      for (j = 0; j < np; j++)
      {
        obase_0[m][8][i][j] = shapeset.get_fn_value(idx[i], pt[j][0], pt[j][1], 0);
        obase_1[m][8][i][j] = shapeset.get_fn_value(idx[i], pt[j][0], pt[j][1], 1);
        obase_c[m][8][i][j] = shapeset.get_dx_value(idx[i], pt[j][0], pt[j][1], 1) -
                              shapeset.get_dy_value(idx[i], pt[j][0], pt[j][1], 0);
      }

    for (l = 0; l < num_sons; l++)
    {
      Trf* tr = (m ? quad_trf : tri_trf) + l;
      for (i = 0; i < n; i++)
        for (j = 0; j < np; j++)
        {
          double x = tr->m[0]*pt[j][0] + tr->t[0], y = tr->m[1]*pt[j][1] + tr->t[1];
          obase_0[m][l][i][j] = shapeset.get_fn_value(idx[i], x, y, 0);
          obase_1[m][l][i][j] = shapeset.get_fn_value(idx[i], x, y, 1);
          obase_c[m][l][i][j] = shapeset.get_dx_value(idx[i], x, y, 1) - shapeset.get_dy_value(idx[i], x, y, 0);
        }
    }

    // orthonormalize the basis functions in H(curl) product
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < i; j++)
      {
        double prod = 0.0;
        for (k = 0; k < np; k++)
        {
          prod += pt[k][2] * ((obase_0[m][8][i][k] * obase_0[m][8][j][k]) +
                              (obase_1[m][8][i][k] * obase_1[m][8][j][k]) +
                              (obase_c[m][8][i][k] * obase_c[m][8][j][k]));
        }
        for (l = 0; l < 9; l++)
          if (m || l < 4 || l >= 8)
            for (k = 0; k < np; k++)
            {
              obase_0[m][l][i][k] -= prod * obase_0[m][l][j][k];
              obase_1[m][l][i][k] -= prod * obase_1[m][l][j][k];
              obase_c[m][l][i][k] -= prod * obase_c[m][l][j][k];
            }
      }

      double fnorm = 0.0;
      for (k = 0; k < np; k++)
        fnorm += pt[k][2] * ( sqr(obase_0[m][8][i][k]) + sqr(obase_1[m][8][i][k]) + sqr(obase_c[m][8][i][k]) );
      fnorm = sqrt(fnorm);

      for (l = 0; l < 9; l++)
        if (m || l < 4 || l >= 8)
          for (k = 0; k < np; k++)
          {
            obase_0[m][l][i][k] /= fnorm;
            obase_1[m][l][i][k] /= fnorm;
            obase_c[m][l][i][k] /= fnorm;
          }
    }

    // check whether the basis is really orthonormal
//     verbose("Checking the orthogonal basis...");
//     for (i = 0; i < n; i++)
//     {
//       for (j = 0; j < n; j++)
//       {
//         double check = 0.0;
//         for (k = 0; k < np; k++)
//         {
//           check += pt[k][2] * ( (obase_0[m][8][i][k] * obase_0[m][8][j][k]) +
//                                 (obase_1[m][8][i][k] * obase_1[m][8][j][k]) +
//                                 (obase_c[m][8][i][k] * obase_c[m][8][j][k]) );
//         }
//         if (((i == j) && (fabs(check - 1.0) > 1e-7)) || ((i != j) && (fabs(check) > 1e-7)))
//           warn("Not orthogonal (whole): base %d times base %d = %g", i, j , check);
//         check = 0.0;
//         for(int son = 0; son < 4; son++ )
//           for (k = 0; k < np; k++)
//           {
//             check += pt[k][2] * ( (obase_0[m][son][i][k] * obase_0[m][son][j][k]) +
//                                   (obase_1[m][son][i][k] * obase_1[m][son][j][k]) +
//                                   (obase_c[m][son][i][k] * obase_c[m][son][j][k]) );
//           }
//         check *= 0.25;
//         if (((i == j) && (fabs(check - 1.0) > 1e-7)) || ((i != j) && (fabs(check) > 1e-7)))
//           warn("Not orthogonal (sons): base %d times base %d = %g", i, j , check);
//         if (m) {
//           check = 0.0;
//           for(int son = 0; son < 2; son++ )
//             for (k = 0; k < np; k++)
//             {
//               check += pt[k][2] * ( (obase_0[m][son+6][i][k] * obase_0[m][son+6][j][k]) +
//                                     (obase_1[m][son+6][i][k] * obase_1[m][son+6][j][k]) +
//                                     (obase_c[m][son+6][i][k] * obase_c[m][son+6][j][k]) );
//             }
//           check *= 0.5;
//           if (((i == j) && (fabs(check - 1.0) > 1e-7)) || ((i != j) && (fabs(check) > 1e-7)))
//             warn("Not orthogonal (aniso): base %d times base %d = %g", i, j , check);
//         }
//       }
//     }
//     verbose("done.\n");

  }

  obase_ready = true;
}


void HcurlOrthoHP::free_ortho_base()
{
  if (!obase_ready) return;

  for (int i = 0; i < 9; i++)
    for (int j = 0; j < 2; j++)
    {
      delete [] obase_0[j][i];
      delete [] obase_1[j][i];
      delete [] obase_c[j][i];
    }
  obase_ready = false;
}


//// optimal refinement search /////////////////////////////////////////////////////////////////////

void HcurlOrthoHP::calc_projection_errors(Element* e, int order, Solution* rsln,
                                   double herr[8][11], double perr[11])
{
  int i, j, k, son, s;
  int m = e->get_mode();
  double error;
  scalar prod;

  if (!obase_ready) calc_ortho_base();

  // select quadrature, obtain integration points and weights
  Quad2D* quad = &g_quad_2d_std;
  quad->set_mode(m);
  rsln->set_quad_2d(quad);
  double3* pt = quad->get_points(20);
  int np = quad->get_num_points(20);

  // everything is done on the reference domain
  // no reference mapping, no transformations
  rsln->enable_transform(false);

  // obtain reference solution values on all four refined sons
  scalar* rval0[4], *rval1[4], *rd1dx[4], *rd0dy[4];
  Element* base = rsln->get_mesh()->get_element(e->id);
  assert(!base->active);
  for (son = 0; son < 4; son++)
  {
    Element* e = base->sons[son];
    assert(e != NULL);
    rsln->set_active_element(e);
    rsln->set_quad_order(20);
    rval0[son] = rsln->get_fn_values(0);
    rval1[son] = rsln->get_fn_values(1);
    rd1dx[son] = rsln->get_dx_values(1);
    rd0dy[son] = rsln->get_dy_values(0);
  }

  // h-candidates: calculate products of the reference solution with orthonormal basis
  // functions on son elements, obtaining (partial) projections and their errors
  scalar proj_0[4][121];
  scalar proj_1[4][121];
  scalar proj_c[4][121];
  for (son = 0; son < 4; son++)
  {
    memset(proj_0[0], 0, sizeof(proj_0[0]));
    memset(proj_1[0], 0, sizeof(proj_1[0]));
    memset(proj_c[0], 0, sizeof(proj_c[0]));

    for (i = 0; i <= order; i++)  // h-candidates: max order equals to original element order
    {
      // update the projection to the current order
      for (j = basecnt[m][i]; j < basecnt[m][i+1]; j++)
      {
        for (k = 0, prod = 0.0; k < np; k++)
        {
          scalar rcurl = (rd1dx[son][k] - rd0dy[son][k]);
          scalar r0 = rval0[son][k];
          scalar r1 = rval1[son][k];
          prod += pt[k][2] * (( r0    * obase_0[m][8][j][k] ) +
                              ( r1    * obase_1[m][8][j][k] ) +
                              ( rcurl * obase_c[m][8][j][k] ) );
        }
        for (k = 0; k < np; k++)
        {
          proj_0[0][k] += obase_0[m][8][j][k] * prod;
          proj_1[0][k] += obase_1[m][8][j][k] * prod;
          proj_c[0][k] += obase_c[m][8][j][k] * prod;
        }
      }

      // calculate the H(curl) error of the projection
      for (k = 0, error = 0.0; k < np; k++)
      {
        scalar rcurl = (rd1dx[son][k] - rd0dy[son][k]);
        scalar r0 = rval0[son][k];
        scalar r1 = rval1[son][k];
        error += pt[k][2] *  ( sqr(r0    - proj_0[0][k]) +
                               sqr(r1    - proj_1[0][k]) +
                               sqr(rcurl - proj_c[0][k]) );
      }
      herr[son][i] = error;
    }
  }

  // aniso-candidates: calculate projections and their errors (only quadrilaterals)
  if (m) {
    const double mx[4] = { 2.0, 2.0, 1.0, 1.0};
    const double my[4] = { 1.0, 1.0, 2.0, 2.0};
    const int sons[4][2] = {{0,1},{3,2},{0,3},{1,2}};
    const int tr[4][2] = {{6,7},{6,7},{4,5},{4,5}};

    for (son = 0; son < 4; son++) // 2 sons for vertical split, 2 sons for horizontal split
    {
      memset(proj_0, 0, sizeof(proj_0));
      memset(proj_1, 0, sizeof(proj_1));
      memset(proj_c, 0, sizeof(proj_c));

      for (i = 0; i <= order+1; i++)  // h-candidates: max order equals to original element order+1
      {
        // update the projection to the current order
        for (j = basecnt[m][i]; j < basecnt[m][i+1]; j++)
        {
          for (s = 0, prod = 0.0; s < 2; s++)  // each son has 2 subsons (regular square sons)
          {
            for (k = 0; k < np; k++)
            {
              scalar rcurl = 2.0 * (rd1dx[sons[son][s]][k] - rd0dy[sons[son][s]][k]);
              scalar r0 = mx[son] * rval0[sons[son][s]][k];
              scalar r1 = my[son] * rval1[sons[son][s]][k];
              prod += pt[k][2] * ((r0    * obase_0[m][tr[son][s]][j][k]) +
                                  (r1    * obase_1[m][tr[son][s]][j][k]) +
                                  (rcurl * obase_c[m][tr[son][s]][j][k]));
            }
          }
          prod *= 0.5;

          for (s = 0; s < 2; s++)
            for (k = 0; k < np; k++)
            {
              proj_0[s][k] += prod * obase_0[m][tr[son][s]][j][k];
              proj_1[s][k] += prod * obase_1[m][tr[son][s]][j][k];
              proj_c[s][k] += prod * obase_c[m][tr[son][s]][j][k];
            }
        }


        // calculate the error of the projection
        for (s = 0, error = 0.0; s < 2; s++)
        {
          for (k = 0; k < np; k++)
          {
            scalar rcurl = 2.0 * (rd1dx[sons[son][s]][k] - rd0dy[sons[son][s]][k]);
            scalar r0 = mx[son] * rval0[sons[son][s]][k];
            scalar r1 = my[son] * rval1[sons[son][s]][k];
            error += pt[k][2] * (sqr(r0    - proj_0[s][k]) +
                                 sqr(r1    - proj_1[s][k]) +
                                 sqr(rcurl - proj_c[s][k]));
          }
        }
        herr[4 + son][i] = error * 0.5;
      }
    }
  }

  // p-candidates: calculate projections and their errors
  memset(proj_0, 0, sizeof(proj_0));
  memset(proj_1, 0, sizeof(proj_1));
  memset(proj_c, 0, sizeof(proj_c));

  for (i = 0; i <= std::min(order+2, 9); i++)  // p-candidate: max order = original order + 2
  {
    // update the projection to the current order
    for (j = basecnt[m][i]; j < basecnt[m][i+1]; j++)
    {
      for (son = 0, prod = 0.0; son < 4; son++)
      {
        // transformations to the quarter of the reference element
        double mm = (e->is_triangle() && son == 3) ? -2.0 : 2.0;

        for (k = 0; k < np; k++)
        {
          scalar rcurl = 4.0 * (rd1dx[son][k] - rd0dy[son][k]);
          scalar r0 = mm * rval0[son][k];
          scalar r1 = mm * rval1[son][k];
          prod += pt[k][2] * ((r0    * obase_0[m][son][j][k]) +
                              (r1    * obase_1[m][son][j][k]) +
                              (rcurl * obase_c[m][son][j][k]));
        }
      }
      prod *= 0.25;

      for (son = 0; son < 4; son++)
        for (k = 0; k < np; k++)
        {
          proj_0[son][k] += prod * obase_0[m][son][j][k];
          proj_1[son][k] += prod * obase_1[m][son][j][k];
          proj_c[son][k] += prod * obase_c[m][son][j][k];
        }
    }


    // calculate the error of the projection
    for (son = 0, error = 0.0; son < 4; son++)
    {
      double mm = (e->is_triangle() && son == 3) ? -2.0 : 2.0;

      for (k = 0; k < np; k++)
      {
        scalar rcurl = 4.0 * (rd1dx[son][k] - rd0dy[son][k]);
        scalar r0 = mm * rval0[son][k];
        scalar r1 = mm * rval1[son][k];
        error += pt[k][2] * (sqr(r0    - proj_0[son][k]) +
                             sqr(r1    - proj_1[son][k]) +
                             sqr(rcurl - proj_c[son][k]));
      }
    }
    perr[i] = error * 0.25;
  }
}


void HcurlOrthoHP::get_optimal_refinement(Element* e, int order, Solution* rsln, int& split, int p[4],
                                          bool h_only, bool iso_only, double conv_exp, int max_order)
{
  int i, j, k, n = 0;
  const int maxcand = 300;

  order = std::max(H2D_GET_H_ORDER(order), H2D_GET_V_ORDER(order));
  bool tri = e->is_triangle();

  // calculate maximal order of elements
  // linear elements = 6
  // curvilinear elements = depends on iro_cache (how curved they are)
  if (max_order == -1)
    max_order = std::min(6, (20 - e->iro_cache)/2 - 1); // default
  else
    max_order = std::min(max_order, (20 - e->iro_cache)/2 - 1); // user specified

  struct Cand
  {
    double error;
    int dofs, p[4];
    int split;
  };
  Cand cand[maxcand];

  #define make_p_cand(q) { \
    assert(n < maxcand);   \
    cand[n].split = -1; \
    cand[n].p[1] = cand[n].p[2] = cand[n].p[3] = 0; \
    cand[n++].p[0] = (q); }

  #define make_hp_cand(q0, q1, q2, q3) { \
    assert(n < maxcand);  \
    cand[n].split = 0; \
    cand[n].p[0] = (q0); \
    cand[n].p[1] = (q1); \
    cand[n].p[2] = (q2); \
    cand[n++].p[3] = (q3); }

  #define make_ani_cand(q0, q1, iso) { \
    assert(n < maxcand);  \
    cand[n].split = iso; \
    cand[n].p[2] = cand[n].p[3] = 0; \
    cand[n].p[0] = (q0); \
    cand[n++].p[1] = (q1); }\

  int first_hp;
  if (h_only)
  {
    make_p_cand(order);
    make_hp_cand(order, order, order, order);
    if ((!tri) && (!iso_only))
    {
      make_ani_cand(order, order, 1);
      make_ani_cand(order, order, 2);
    }
  }
  else {
    // prepare p-candidates
    int p0, p1 = std::min(max_order, order+2);
    for (p0 = order; p0 <= p1; p0++)
      make_p_cand(p0);

    // prepare hp-candidates
    first_hp = n;
    p0 = std::max(1, (order-1)/ 2);
    p1 = std::min(p0 + 2, order);
    int q0, q1, q2, q3;
    for (q0 = p0; q0 <= p1; q0++)
      for (q1 = p0; q1 <= p1; q1++)
        for (q2 = p0; q2 <= p1; q2++)
          for (q3 = p0; q3 <= p1; q3++)
            make_hp_cand(q0,q1,q2,q3);

    // anisotropic candidates
    // only for quadrilaterals
    // too distorted (curved) elements cannot have aniso refinement (produces even worse elements)
    if ((!tri) && (e->iro_cache < 8) && !iso_only) {
      p0 = std::max(1, 2 * (order) / 3);
      int p_max = std::min(max_order, order+1);
      p1 = std::min(p0 + 3, p_max);
      for (q0 = p0; q0 <= p1; q0++)
        for (q1 = p0; q1 <= p1; q1++) {
          if ((q0 < order+1) || (q1 < order+1)) {
            make_ani_cand(q0, q1, 1);
            make_ani_cand(q0, q1, 2);
          }
        }
    }
  }
  // calculate (partial) projection errors
  double herr[8][11], perr[11];
  calc_projection_errors(e, order, rsln, herr, perr);

  // evaluate candidates (sum partial projection errors, calculate dofs)
  double avg = 0.0;
  double dev = 0.0;
  double dev1 = 0.0;
  for (i = k = 0; i < n; i++)
  {
    Cand* c = cand + i;
    if (c->split == 0)  // isotropic split
    {
      c->error = 0.0;
      c->dofs = 0;
      for (j = 0; j < 4; j++)
      {
        c->error += herr[j][c->p[j]];// * 0.25;
        if (tri)
        {
          if (j < 3) {
            c->dofs += sqr(c->p[j] + 1); // num of bubble and bnd edges
            c->dofs += std::min(c->p[j], c->p[3]) + 1; // internal edge dofs
          }
          else
            c->dofs += 3 * (c->p[j] - 1) + (c->p[j] - 1) * (c->p[j] - 2);
        }
        else
        {
          c->dofs += 2 * sqr(c->p[j] + 1); // number of bubble and boundary edges
          c->dofs += std::min(c->p[j], c->p[j>0 ? j-1 : 3]) + 1; // number of internal edges in element
        }
      }
    }
    else if ((c->split == 1) || (c->split == 2))  // aniso splits
    {
      c->error = 0.0;
      c->dofs = 0;
      for (j = 0; j < 2 ; j++) {
        c->error += herr[ (c->split == 1) ? j+4 : j+6 ][c->p[j]];// * 0.5;
        c->dofs += 3 * (c->p[j] + 1) + 2 * c->p[j] * (c->p[j] + 1);
      }
      c->dofs += std::min(c->p[0], c->p[1]) + 1;
    }
    else // p-candidate
    {
      c->error = perr[c->p[0]];
      if (tri)
        c->dofs = (c->p[0] + 1) * (c->p[0] + 2);
      else
        c->dofs = 2 * (c->p[0] + 1) * (c->p[0] + 2); // number of bubble and boundary edges
    }
    c->error = sqrt(c->error);

     //verbose("Cand #%d: Orders %d %d %d %d, Error %g, Dofs %d", i, c->p[0],c->p[1],c->p[2],c->p[3],c->error, c->dofs);

    if (!i || c->error <= cand[0].error)
    {
      avg += log(c->error);
      dev += sqr(log(c->error));
      k++;
    }
  }
  avg /= k;  // mean
  dev /= k;  // second moment
  dev = sqrt(dev - sqr(avg));  // deviation is square root of variance

  // select an above-average candidate with the steepest error decrease
  int imax = 0;
  double score, maxscore = 0.0;
  for (i = 1; i < n; i++)
  {
    if ((log(cand[i].error) < avg + dev) && (cand[i].dofs > cand[0].dofs))
    {
      score = (log(cand[0].error) - log(cand[i].error)) /
	       //(pow(cand[i].dofs, conv_exp) - pow(cand[0].dofs, conv_exp));
               pow(cand[i].dofs - cand[0].dofs, conv_exp);
      if (score > maxscore) { maxscore = score; imax = i; }
    }
  }
  if (imax == 0) imax = first_hp;

  // return result
  split = cand[imax].split;
  memcpy(p, cand[imax].p, 4*sizeof(int));

}


//// adapt /////////////////////////////////////////////////////////////////////////////////////////
bool HcurlOrthoHP::adapt(double thr, int strat, int adapt_type, bool iso_only, int regularize,
                         double conv_exp, int max_order)
{
  if (!have_errors)
    error("Element errors have to be calculated first, see calc_error().");

  int i, j;
  Mesh* mesh[10];
  for (j = 0; j < num; j++) {
    mesh[j] = spaces[j]->get_mesh();
    rsln[j]->set_quad_2d(&g_quad_2d_std);
    rsln[j]->enable_transform(false);
  }

  bool h_only = adapt_type == 1 ? true : false;
  double err0 = 1000.0;
  double processed_error = 0.0;
  int successfully_refined = 0;
  for (i = 0; i < nact; i++)
  {
    int comp = esort[i][1];
    int id = esort[i][0];
    double err = errors[comp][id];

    // first refinement strategy:
    // refine elements until prescribed amount of error is processed
    // if more elements have similar error refine all to keep the mesh symmetric
    if ((strat == 0) && (processed_error > sqrt(thr) * total_err) && fabs((err - err0)/err0) > 1e-2) break;

    // second refinement strategy:
    // refine all elements whose error is bigger than some portion of maximal error
    if ((strat == 1) && (err < thr * errors[esort[0][1]][esort[0][0]])) break;

    Element* e;
    e = mesh[comp]->get_element(id);
    int split = 0;
    int p[4];
    int current = spaces[comp]->get_element_order(id);


    // p-adaptivity
    if (adapt_type == 2)
    {
      split = -1;
      p[0] = std::min(9, H2D_GET_H_ORDER(current) + 1);
      if (H2D_GET_H_ORDER(current) < p[0]) successfully_refined++;
    }
    // h-adaptivity
    else if ((adapt_type == 1 && iso_only) || (adapt_type == 1 && e->is_triangle()))
    {
      p[0] = p[1] = p[2] = p[3] = current;
    }
    // hp-adaptivity
    else
    {
      get_optimal_refinement(e, current, rsln[comp], split, p, h_only, iso_only, conv_exp, max_order);
      successfully_refined++;
    }

    if (split < 0)
      spaces[comp]->set_element_order(id, p[0]);
    else if (split == 0) {
      mesh[comp]->refine_element(id);
      for (j = 0; j < 4; j++)
        spaces[comp]->set_element_order(e->sons[j]->id, p[j]);
    }
    else {
      mesh[comp]->refine_element(id, split);
      for (j = 0; j < 2; j++)
        spaces[comp]->set_element_order(e->sons[ (split == 1) ? j : j+2 ]->id, p[j]);
    }

    err0 = err;
    processed_error += err;
  }

  bool done = false;
  if (successfully_refined == 0)
  {
    warn("\nNone of the elements selected for refinement could be refined.\nAdaptivity step not successful, returning 'true'.");
    done = true;
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
      parents = mesh[i]->regularize(regularize);
      spaces[i]->distribute_orders(mesh[i], parents);
      delete [] parents;
    }
  }

  for (j = 0; j < num; j++)
    rsln[j]->enable_transform(true);

  verbose("Refined %d elements.", successfully_refined);
  have_errors = false;

  return done;
}


//// error calculation /////////////////////////////////////////////////////////////////////////////

static double** cmp_err;
static int compare(const void* p1, const void* p2)
{
  const int2 (*e1) = ((const int2*) p1);
  const int2 (*e2) = ((const int2*) p2);
  return cmp_err[(*e1)[1]][(*e1)[0]] < cmp_err[(*e2)[1]][(*e2)[0]] ? 1 : -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HcurlOrthoHP::set_biform(int i, int j, biform_val_t bi_form, biform_ord_t bi_ord)
{
  if (i < 0 || i >= num || j < 0 || j >= num)
    error("Invalid equation number.");

  form[i][j] = bi_form;
  ord[i][j] = bi_ord;
}


scalar HcurlOrthoHP::eval_error(biform_val_t bi_fn, biform_ord_t bi_ord,
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
    err1->val0[i] = err1->val0[i] - v1->val0[i];
    err1->val1[i] = err1->val1[i] - v1->val1[i];
    err1->curl[i] = err1->curl[i] - v1->curl[i];
    err2->val0[i] = err2->val0[i] - v2->val0[i];
    err2->val1[i] = err2->val1[i] - v2->val1[i];
    err2->curl[i] = err2->curl[i] - v2->curl[i];
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


scalar HcurlOrthoHP::eval_norm(biform_val_t bi_fn, biform_ord_t bi_ord,
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


double HcurlOrthoHP::calc_error(MeshFunction* sln, MeshFunction* rsln)
{
  if (num != 1) error("Wrong number of solutions.");

  return calc_error_n(1, sln, rsln);
}


double HcurlOrthoHP::calc_error_2(MeshFunction* sln1, MeshFunction* sln2, MeshFunction* rsln1, MeshFunction* rsln2)
{
  if (num != 2) error("Wrong number of solutions.");

  return calc_error_n(2, sln1, sln2, rsln1, rsln2);
}


double HcurlOrthoHP::calc_error_n(int n, ...)
{
  int i, j, k;

  if (n != num) error("Wrong number of solutions.");

  // obtain solutions and bilinear forms
  va_list ap;
  va_start(ap, n);
  for (i = 0; i < n; i++) {
    sln[i] = va_arg(ap, Solution*);
    sln[i]->set_quad_2d(&g_quad_2d_std);
  }
  for (i = 0; i < n; i++) {
    rsln[i] = va_arg(ap, Solution*);
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
  memset(norms, 0, num*sizeof(double));
  double total_error = 0.0;
  if (esort != NULL) delete [] esort;
  esort = new int2[nact];

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

  Element* e;
  k = 0;
  for (i = 0; i < num; i++)
    for_all_active_elements(e, meshes[i]) {
      esort[k][0] = e->id;
      esort[k++][1] = i;
      errors[i][e->id] /= norms[i];
    }

  assert(k == nact);
  cmp_err = errors;
  qsort(esort, nact, sizeof(int2), compare);


  have_errors = true;
  total_err = total_error / total_norm;
  return sqrt(total_error / total_norm);
}



#endif
