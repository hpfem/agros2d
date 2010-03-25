#include "../common.h"
#include "../solution.h"
#include "../linsystem.h"
#include "../quad_all.h"
#include "../element_to_refine.h"
#include "proj_based_selector.h"

namespace RefinementSelectors {
  void ProjBasedSelector::evaluate_cands_error(Element* e, Solution* rsln, double* avg_error, double* dev_error) {
    bool tri = e->is_triangle();

    // find range of orders
    int max_quad_order_h, max_quad_order_p, max_quad_order_aniso;
    calc_cands_max_order(&max_quad_order_h, &max_quad_order_p, &max_quad_order_aniso);

    // calculate (partial) projection errors
    SonProjectionError herr[4], anisoerr[4], perr;
    calc_projection_errors(e, max_quad_order_h, max_quad_order_p, max_quad_order_aniso, rsln, herr, anisoerr, perr);

    //evaluate errors and dofs
    double sum_err = 0.0;
    double sum_sqr_err = 0.0;
    int num_processed = 0;
    Cand& unrefined_c = candidates[0];
    for (unsigned i = 0; i < candidates.size(); i++) {
      Cand& c = candidates[i];
      if (tri) { //triangle
        switch(c.split) {
        case H2D_REFINEMENT_H:
          c.error = 0.0;
          for (int j = 0; j < H2D_MAX_ELEMENT_SONS; j++) {
            int order = get_h_order(c.p[j]);
            c.error += herr[j][order][order]; // * 0.25; // spravny vypocet chyby, ??? candidate error is composed of four sons
          }
          break;

        case H2D_REFINEMENT_P:
          {
            int order = get_h_order(c.p[0]);
            c.error = perr[order][order];
          }
          break;

        default:
          assert_msg(false, "E unknown split type \"%d\" at candidate %d", c.split, i);
        }
      }
      else { //quad
        switch(c.split) {
        case H2D_REFINEMENT_H:
          c.error = 0.0;
          for (int j = 0; j < H2D_MAX_ELEMENT_SONS; j++) {
            int order_h = get_h_order(c.p[j]), order_v = get_v_order(c.p[j]);
            c.error += herr[j][order_h][order_v]; // * 0.25; // spravny vypocet chyby, ??? candidate error is composed of four sons
          }
          break;

        case H2D_REFINEMENT_ANISO_H:
        case H2D_REFINEMENT_ANISO_V:
          {
            c.error = 0.0;
            for (int j = 0; j < 2; j++)
              c.error += anisoerr[(c.split == H2D_REFINEMENT_ANISO_H) ? j : j+2][get_h_order(c.p[j])][get_v_order(c.p[j])]; // * 0.5;  // spravny vypocet chyby, ??? average of errors on splot element (sons)
          }
          break;

        case H2D_REFINEMENT_P:
          {
            int order_h = get_h_order(c.p[0]), order_v = get_v_order(c.p[0]);
            c.error = perr[order_h][order_v];
          }
          break;

        default:
          assert_msg(false, "E unknown split type \"%d\" at candidate %d", c.split, i);
        }
      }

      c.error = sqrt(c.error);
      if (!i || c.error <= unrefined_c.error)
      {
        sum_err += log10(c.error);
        sum_sqr_err += sqr(log10(c.error));
        num_processed++;
      }
    }

    *avg_error = sum_err / num_processed;  // mean
    *dev_error = sqrt(sum_sqr_err/num_processed - sqr(*avg_error)); // deviation is square root of variance
  }
}
