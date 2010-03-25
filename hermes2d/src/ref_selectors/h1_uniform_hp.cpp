#include "../common.h"
#include "../solution.h"
#include "../linsystem.h"
#include "../shapeset_h1_all.h"
#include "../quad_all.h"
#include "../integrals_h1.h"
#include "../element_to_refine.h"
#include "h1_uniform_hp.h"

namespace RefinementSelectors {
  H1Shapeset H1UniformHP::default_shapeset;

  H1UniformHP::H1UniformHP(bool iso_only, AllowedCandidates cands_allowed, double conv_exp, int max_order, H1Shapeset* user_shapeset)
    : ProjBasedSelector(iso_only, cands_allowed, conv_exp, max_order, user_shapeset == NULL ? &default_shapeset : user_shapeset)
    , obase_ready(false) {};

  H1UniformHP::~H1UniformHP() {
    free_ortho_base();
  }

  int H1UniformHP::build_shape_inxs(const int mode, Shapeset* shapeset, int idx[121]) {
    shapeset->set_mode(mode);

    // obtain a list of all shape functions up to the order 10, from lowest to highest order
    int n = 0;
    int nv = mode ? 4 : 3;
    for (int i = 0; i < nv; i++) //orders = 1
      idx[n++] = shapeset->get_vertex_index(i);
    basecnt[mode][0] = 0;
    basecnt[mode][1] = n;

    for (int i = 2; i <= 10; i++) //orders > 1
    {
      for (int j = 0; j < nv; j++)
        idx[n++] = shapeset->get_edge_index(j, 0, i);

      int ii = mode ? make_quad_order(i, i) : i;
      int nb = shapeset->get_num_bubbles(ii);
      int* bub = shapeset->get_bubble_indices(ii);
      for (int j = 0; j < nb; j++)
      {
        int o = shapeset->get_order(bub[j]);
        if (get_h_order(o) == i || get_v_order(o) == i)
          idx[n++] = bub[j];
      }
      basecnt[mode][i] = n;
    }

    return n;
  }

  void H1UniformHP::calc_ortho_base() {
    int i, j, k, l, m, np, r;
    int n, idx[121];

    // allocate the orthonormal base tables - these are simply the values of the
    // orthonormal functions in integration points; we store the basic functions
    // plus four son cut-outs of them (i.e. 5 times)
    for (i = 0; i < 9; i++)
    {
      if ((i < 4) || (i >= 8)) //???
        obase[0][i] = new_matrix<double3>(66, 79); // tri
      else
        obase[0][i] = NULL;
      obase[1][i] = new_matrix<double3>(121, 121); // quad
    }

    // repeat for triangles and quads
    for (m = 0; m <= 1; m++)
    {
      //build indices
      n = build_shape_inxs(m, shapeset, idx);

      // obtain their values for integration rule 20
      g_quad_2d_std.set_mode(m);
      np = g_quad_2d_std.get_num_points(20);
      double3* pt = g_quad_2d_std.get_points(20);

      for (i = 0; i < n; i++)
        for (j = 0; j < np; j++)
          for (k = 0; k < 3; k++)
            obase[m][8][i][j][k] = shapeset->get_value(k, idx[i], pt[j][0], pt[j][1], 0);

      int num_sons = m ? 8 : 4;
      for (l = 0; l < num_sons; l++)
      {
        Trf* tr = (m ? quad_trf : tri_trf) + l;
        for (i = 0; i < n; i++)
          for (j = 0; j < np; j++)
          {
            double x = tr->m[0]*pt[j][0] + tr->t[0],
                   y = tr->m[1]*pt[j][1] + tr->t[1];
            for (k = 0; k < 3; k++)
              obase[m][l][i][j][k] = shapeset->get_value(k, idx[i], x, y, 0);
          }
      }

      // orthonormalize the basis functions
      for (i = 0; i < n; i++)
      {
        for (j = 0; j < i; j++)
        {
          double prod = 0.0;
          for (k = 0; k < np; k++) {
            double sum = 0.0;
            for (r = 0; r < 3; r++)
              sum += obase[m][8][i][k][r] * obase[m][8][j][k][r];
            prod += pt[k][2] * sum;
          }

          for (l = 0; l < 9; l++)
            if (m || l < 4 || l >= 8)
              for (k = 0; k < np; k++)
                for (r = 0; r < 3; r++)
                  obase[m][l][i][k][r] -= prod * obase[m][l][j][k][r];
        }

        double norm = 0.0;
        for (k = 0; k < np; k++) {
          double sum = 0.0;
          for (r = 0; r < 3; r++)
            sum += sqr(obase[m][8][i][k][r]);
          norm += pt[k][2] * sum;
        }
        norm = sqrt(norm);

        for (l = 0; l < 9; l++)
          if (m || l < 4 || l >= 8)
            for (k = 0; k < np; k++)
              for (r = 0; r < 3; r++)
                obase[m][l][i][k][r] /= norm;
      }

      // check the orthonormal base
  /*    if (m) {
      for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
        {
          double check = 0.0;
          for(int son = 4; son < 6; son++ )
            for (k = 0; k < np; k++)
              check += pt[k][2] * (obase[m][son][i][k][0] * obase[m][son][j][k][0] +
                                   obase[m][son][i][k][1] * obase[m][son][j][k][1] +
                                   obase[m][son][i][k][2] * obase[m][son][j][k][2]);
          check *= 0.5;
          if ((i == j && fabs(check - 1.0) > 1e-8) || (i != j && fabs(check) > 1e-8))
            warn("Not orthonormal: base %d times base %d = %g", i, j , check);
        }
      }*/
    }
    obase_ready = true;
  }

  void H1UniformHP::free_ortho_base() {
    if (!obase_ready) return;

    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 2; j++)
        delete [] obase[j][i];

    obase_ready = false;
  }

  void H1UniformHP::calc_projection_errors(Element* e, const int max_quad_order_h, const int max_quad_order_p, const int max_quad_order_aniso, Solution* rsln, SonProjectionError herr[4], SonProjectionError anisoerr[4], SonProjectionError perr) {
    debug_assert(get_h_order(max_quad_order_h) <= H2DRS_MAX_ORDER && get_v_order(max_quad_order_h) <= H2DRS_MAX_ORDER, "E maximum allowed order of a son of H-candidate is %d but order (H:%d,V:%d) requested", H2DRS_MAX_ORDER, get_h_order(max_quad_order_h), get_v_order(max_quad_order_h));
    debug_assert(get_h_order(max_quad_order_p) <= H2DRS_MAX_ORDER && get_v_order(max_quad_order_p) <= H2DRS_MAX_ORDER, "E maximum allowed order of a son of P-candidate is %d but order (H:%d,V:%d) requested", H2DRS_MAX_ORDER, get_h_order(max_quad_order_p), get_v_order(max_quad_order_p));
    debug_assert(get_h_order(max_quad_order_aniso) <= H2DRS_MAX_ORDER && get_v_order(max_quad_order_aniso) <= H2DRS_MAX_ORDER, "E maximum allowed order of a son of ANISO-candidate is %d but order (H:%d,V:%d) requested", H2DRS_MAX_ORDER, get_h_order(max_quad_order_aniso), get_v_order(max_quad_order_aniso));
    debug_assert(get_h_order(max_quad_order_h) == get_v_order(max_quad_order_h), "E maximum orders (H:%d,V:%d) of H-candidate is in not uniform", get_h_order(max_quad_order_h), get_v_order(max_quad_order_h));
    debug_assert(get_h_order(max_quad_order_p) == get_v_order(max_quad_order_p), "E maximum orders (H:%d,V:%d) of P-candidate is in not uniform", get_h_order(max_quad_order_p), get_v_order(max_quad_order_p));
    debug_assert(get_h_order(max_quad_order_aniso) == get_v_order(max_quad_order_aniso), "E maximum orders (H:%d,V:%d) of ANISO-candidate is in not uniform", get_h_order(max_quad_order_aniso), get_v_order(max_quad_order_aniso));

    int i, j, s, k, r, son;
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
    // -- no reference mapping, no transformations
    rsln->enable_transform(false);

    // obtain reference solution values on all four refined sons
    scalar* rval[4][3];
    Element* base = rsln->get_mesh()->get_element(e->id);
    assert(!base->active);
    for (son = 0; son < 4; son++)
    {
      Element* e = base->sons[son];
      assert(e != NULL);
      rsln->set_active_element(e);
      rsln->set_quad_order(20);
      rval[son][0] = rsln->get_fn_values();
      rval[son][1] = rsln->get_dx_values();
      rval[son][2] = rsln->get_dy_values();
    }

    // h-cadidates: calculate products of the reference solution with orthonormal basis
    // functions on son elements, obtaining (partial) projections and their errors
    // the error is scaled by 4 (error of four sons of an element of summed togethers; error for every element is evaluted in a reference domain
    int max_order = get_h_order(max_quad_order_h);
    scalar3 proj[4][121];
    for (son = 0; son < 4; son++)
    {
      memset(proj[0], 0, sizeof(proj[0]));
      for (i = 1; i <= max_order; i++)
      {
        // update the projection to the current order
        for (j = basecnt[m][i-1]; j < basecnt[m][i]; j++)
        {
          for (k = 0, prod = 0.0; k < np; k++)
            prod += pt[k][2] * (rval[son][0][k] * obase[m][8][j][k][0] +
            rval[son][1][k] * obase[m][8][j][k][1] +
            rval[son][2][k] * obase[m][8][j][k][2]);

          for (k = 0; k < np; k++)
            for (r = 0; r < 3; r++)
              proj[0][k][r] += obase[m][8][j][k][r] * prod;
        }

        // calculate the error of the projection
        for (k = 0, error = 0.0; k < np; k++)
          error += pt[k][2] * (sqr(rval[son][0][k] - proj[0][k][0]) +
          sqr(rval[son][1][k] - proj[0][k][1]) +
          sqr(rval[son][2][k] - proj[0][k][2]));
        herr[son][i][i] = error;
      }
    }

    // aniso-candidates: calculate projections and their errors (only quadrilaterals)
    // the error is not scaled
    if (m)
    {
      const double mx[4] = { 2.0, 2.0, 1.0, 1.0};
      const double my[4] = { 1.0, 1.0, 2.0, 2.0};
      const int sons[4][2] = { {0,1}, {3,2}, {0,3}, {1,2} };
      const int tr[4][2]   = { {6,7}, {6,7}, {4,5}, {4,5} };
      max_order = get_h_order(max_quad_order_aniso);

      for (son = 0; son < 4; son++) // 2 sons for vertical split, 2 sons for horizontal split
      {
        memset(proj, 0, sizeof(proj));
        for (i = 1; i <= max_order; i++)
        {
          // update the projection to the current order
          for (j = basecnt[m][i-1]; j < basecnt[m][i]; j++)
          {
            scalar prod = 0.0;
            for (s = 0; s < 2; s++) // each son has 2 subsons (regular square sons)
              for (k = 0; k < np; k++)
                prod += pt[k][2] * ( rval[sons[son][s]][0][k]           * obase[m][tr[son][s]][j][k][0] +
                rval[sons[son][s]][1][k] * mx[son] * obase[m][tr[son][s]][j][k][1] +
                rval[sons[son][s]][2][k] * my[son] * obase[m][tr[son][s]][j][k][2]);
            prod *= 0.5; //compensate the fact that values are a sum of itegral of two elements in a ref. domain

            for (s = 0; s < 2; s++)
              for (k = 0; k < np; k++)
                for (r = 0; r < 3; r++)
                  proj[s][k][r] += prod * obase[m][tr[son][s]][j][k][r];
          }

          // calculate the error of the projection
          for (s = 0, error = 0.0; s < 2; s++)
            for (k = 0; k < np; k++)
              error += pt[k][2] * (sqr(rval[sons[son][s]][0][k]           - proj[s][k][0]) +
              sqr(rval[sons[son][s]][1][k] * mx[son] - proj[s][k][1]) +
              sqr(rval[sons[son][s]][2][k] * my[son] - proj[s][k][2]));
          anisoerr[son][i][i] = error * 0.5; //compensate the fact that values are a sum of itegral of two elements in a ref. domain
        }
      }
    }

    // p-candidates: calculate projections and their errors
    max_order = get_h_order(max_quad_order_p);
    memset(proj, 0, sizeof(proj));
    for (i = 1; i <= max_order; i++)
    {
      // update the projection to the current order
      for (j = basecnt[m][i-1]; j < basecnt[m][i]; j++)
      {
        scalar prod = 0.0;
        for (son = 0; son < 4; son++)
        {
          // (transforming to the quarter of the reference element)
          double mm = (e->is_triangle() && son == 3) ? -2.0 : 2.0;

          for (k = 0; k < np; k++)
          {
            prod += pt[k][2] * (rval[son][0][k] *      obase[m][son][j][k][0] +
              rval[son][1][k] * mm * obase[m][son][j][k][1] +
              rval[son][2][k] * mm * obase[m][son][j][k][2]);
          }
        }
        prod *= 0.25; //compensate the fact that values are a sum of itegral of four elements in a ref. domain

        for (son = 0; son < 4; son++)
          for (k = 0; k < np; k++)
            for (r = 0; r < 3; r++)
              proj[son][k][r] += prod * obase[m][son][j][k][r];
      }

      // calculate the error of the projection
      for (son = 0, error = 0.0; son < 4; son++)
      {
        double mm = (e->is_triangle() && son == 3) ? -2.0 : 2.0;

        for (k = 0; k < np; k++)
          error += pt[k][2] * (sqr(rval[son][0][k]      - proj[son][k][0]) +
          sqr(rval[son][1][k] * mm - proj[son][k][1]) +
          sqr(rval[son][2][k] * mm - proj[son][k][2]));
      }
      perr[i][i] = error * 0.25; //compensate the fact that values are a sum of itegral of four elements in a ref. domain
    }

    rsln->enable_transform(true);

  }

  void H1UniformHP::create_candidates(Element* e, int quad_order, int max_ha_quad_order, int max_p_quad_order) {
    //clear list of candidates
    candidates.clear();
    if (candidates.capacity() < H2DRS_ASSUMED_MAX_CANDS)
      candidates.reserve(H2DRS_ASSUMED_MAX_CANDS);

    int n = 0;
    int order = std::max(get_h_order(quad_order), get_v_order(quad_order));
    int max_p_order = std::max(get_h_order(max_p_quad_order), get_v_order(max_p_quad_order));
    int max_ha_order = std::max(get_h_order(max_ha_quad_order), get_v_order(max_ha_quad_order));

    bool tri = e->is_triangle();

#define make_p_cand(q) candidates.push_back(Cand(H2D_REFINEMENT_P, make_quad_order(q, q)))

#define make_hp_cand(q0, q1, q2, q3) candidates.push_back(Cand(H2D_REFINEMENT_H, \
    make_quad_order(q0, q0), make_quad_order(q1, q1), make_quad_order(q2, q2), make_quad_order(q3, q3)))

#define make_ani_cand(q0, q1, iso) candidates.push_back(Cand(iso, make_quad_order(q0, q0), make_quad_order(q1, q1)))

    if (cands_allowed == H2DRS_CAND_H_ONLY)
    {
      make_p_cand(order);
      make_hp_cand(order, order, order, order);
      if ((!tri) && (e->iro_cache < 8) && !iso_only) {
        make_ani_cand(order, order, 1);
        make_ani_cand(order, order, 2);
      }
    }
    else {
      // prepare p-candidates
      int p0, p1 = std::min(max_p_order, order+2);
      for (p0 = order; p0 <= p1; p0++)
        make_p_cand(p0);

      if (cands_allowed == H2DRS_CAND_HP) {
        //prepare hp-candidates
        p0 = std::min(std::max(current_min_order, (order+1) / 2), max_ha_order);
        p1 = std::min(std::max(current_min_order, std::min(p0 + 3, order)), max_ha_order);
        int q0, q1, q2, q3;
        for (q0 = p0; q0 <= p1; q0++)
          for (q1 = p0; q1 <= p1; q1++)
            for (q2 = p0; q2 <= p1; q2++)
              for (q3 = p0; q3 <= p1; q3++)
                make_hp_cand(q0, q1, q2, q3);

        //prepare anisotropic candidates
        //only for quadrilaterals
        //too distorted (curved) elements cannot have aniso refinement (produces even worse elements)
        if ((!tri) && (e->iro_cache < 8) && !iso_only) {
          p0 = 2 * (order+1) / 3;
          int p_max = std::min(max_ha_order, order+1);
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
    }
  }

  void H1UniformHP::update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders) {
    debug_assert(get_v_order(orig_quad_order) == 0 || get_h_order(orig_quad_order) == get_v_order(orig_quad_order), "E non-uniform orig_quad_order (H:%d,V:%d) not support for this selector", get_h_order(orig_quad_order), get_v_order(orig_quad_order));

    //update orders
    ProjBasedSelector::update_shared_mesh_orders(element, orig_quad_order, refinement, tgt_quad_orders, suggested_quad_orders);

    //enforce uniform orders
    const int num_sons = get_refin_sons(refinement);
    for(int i = 0; i < num_sons; i++) {
      int order = get_h_order(tgt_quad_orders[i]);
      if (refinement == H2D_REFINEMENT_ANISO_H)
        order = get_v_order(tgt_quad_orders[i]);
      if (element->is_triangle())
        tgt_quad_orders[i] = make_quad_order(order, 0);
      else
        tgt_quad_orders[i] = make_quad_order(order, order);
    }
  }

}
