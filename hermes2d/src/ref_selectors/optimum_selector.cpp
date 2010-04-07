#include "../common.h"
#include "../solution.h"
#include "../linsystem.h"
#include "../quad_all.h"
#include "../element_to_refine.h"
#include "optimum_selector.h"

namespace RefinementSelectors {

  std::ostream &operator<<(std::ostream &stream, const OptimumSelector::Cand& cand) {
    stream.precision(2);
    stream << "split:" << get_refin_str(cand.split);
    stream << " err:" << std::scientific << cand.error << " dofs:" << cand.dofs << " ";

    int num_sons = cand.get_num_sons();
    stream << "[";
    for(int i = 0; i < num_sons; i++) {
      if (i > 0)
        stream << " ";
      stream << get_quad_order_str(cand.p[i]);
    }
    stream << "]";
    return stream;
  }

  OptimumSelector::OrderPermutator::OrderPermutator(int start_quad_order, int end_quad_order, bool uniform, int* tgt_quad_order)
    : start_order_h(get_h_order(start_quad_order)), start_order_v(get_v_order(start_quad_order))
    , end_order_h(get_h_order(end_quad_order)), end_order_v(get_v_order(end_quad_order))
    , uniform(uniform), tgt_quad_order(tgt_quad_order) {
    assert_msg(!uniform || start_order_h == start_order_v, "E uniform orders requested but start orders (H:%d, V:%d) are not uniform", order_h, order_v);
    assert_msg(start_order_h <= end_order_h && start_order_v <= end_order_v, "E end orders (H:%d, V:%d) are below start orders (H:%d, V:%d)", end_order_h, end_order_v, start_order_h, start_order_v);
    reset();
  }

  bool OptimumSelector::OrderPermutator::next() {
    if (order_h >= end_order_h && order_v >= end_order_v)
      return false;
    else {
      if (uniform) {
        order_h++; order_v++;
      }
      else {
        order_h++;
        if (order_h > end_order_h) {
          order_h = start_order_h;
          order_v++;
        }
      }
      if (tgt_quad_order != NULL)
        *tgt_quad_order = make_quad_order(order_h, order_v);
      return true;
    }
  }

  void OptimumSelector::OrderPermutator::reset() {
    order_h = start_order_h;
    order_v = start_order_v;
    if (tgt_quad_order != NULL)
      *tgt_quad_order = make_quad_order(order_h, order_v);
  }

  OptimumSelector::OptimumSelector(bool iso_only, AllowedCandidates cands_allowed, double conv_exp, int max_order, Shapeset* shapeset)
    : Selector(max_order), iso_only(iso_only), cands_allowed(cands_allowed)
    , conv_exp(conv_exp), shapeset(shapeset) {
    assert_msg(shapeset != NULL, "E shapeset is NULL");
  }

  void OptimumSelector::set_current_order_range(Element* element) {
    current_max_order = this->max_order;
    if (current_max_order == H2DRS_DEFAULT_ORDER)
      current_max_order = (20 - element->iro_cache)/2 - 1; // default
    else
      current_max_order = std::min(current_max_order, (20 - element->iro_cache)/2 - 1); // user specified
    current_min_order = 1;
  }

  void OptimumSelector::append_candidates_split(const int start_quad_order, const int last_quad_order, const int split, bool uniform) {
    //check whether end orders are not lower than start orders
    if (get_h_order(start_quad_order) > get_h_order(last_quad_order) || get_v_order(start_quad_order) > get_v_order(last_quad_order))
      return;

    //get number of sons
    const int num_sons = get_refin_sons(split);

    //initialize orders
    int quad_orders[H2D_MAX_ELEMENT_SONS];
    OrderPermutator quad_perms[H2D_MAX_ELEMENT_SONS];
    for(int i = 0; i < num_sons; i++) {
      quad_orders[i] = start_quad_order;
      quad_perms[i] = OrderPermutator(start_quad_order, last_quad_order, uniform, &quad_orders[i]);
    }
    for(int i = num_sons; i < H2D_MAX_ELEMENT_SONS; i++)
      quad_orders[i] = 0;

    //generate permutations of orders
    bool quit = false;
    while(!quit) {
      do { //create permutation of son 0
        candidates.push_back(Cand(split, quad_orders));
      } while (quad_perms[0].next());

      //reset son 0
      quad_perms[0].reset();

      //increment orders of other sons
      int inx_son = 1;
      while (inx_son < num_sons && !quad_perms[inx_son].next()) {
        quad_perms[inx_son].reset(); //reset order of the son
        inx_son++;
      }
      if (inx_son >= num_sons)
        quit = true;
    }
  }

  void OptimumSelector::create_candidates(Element* e, int quad_order, int max_ha_quad_order, int max_p_quad_order) {
    int order_h = get_h_order(quad_order), order_v = get_v_order(quad_order);
    int max_p_order_h = get_h_order(max_p_quad_order), max_p_order_v = get_v_order(max_p_quad_order);
    int max_ha_order_h = get_h_order(max_ha_quad_order), max_ha_order_v = get_v_order(max_ha_quad_order);
    bool tri = e->is_triangle();

    //clear list of candidates
    candidates.clear();
    if (candidates.capacity() < H2DRS_ASSUMED_MAX_CANDS)
      candidates.reserve(H2DRS_ASSUMED_MAX_CANDS);

    if (cands_allowed == H2DRS_CAND_H_ONLY) {
      int quad_orders[H2D_MAX_ELEMENT_SONS] = { quad_order, 0, 0, 0};
      candidates.push_back(Cand(H2D_REFINEMENT_P, quad_orders));
      quad_orders[0] = quad_orders[1] = quad_orders[2] = quad_orders[3] = quad_order;
      candidates.push_back(Cand(H2D_REFINEMENT_H, quad_orders));
      if (!tri && (e->iro_cache < 8) && !iso_only) {
        quad_orders[2] = quad_orders[3] = 0;
        candidates.push_back(Cand(H2D_REFINEMENT_ANISO_V, quad_orders));
        candidates.push_back(Cand(H2D_REFINEMENT_ANISO_H, quad_orders));
      }
    }
    else {
      //prepare P-candidates
      int last_quad_order = make_quad_order(std::min(max_p_order_h, order_h+2), std::min(max_p_order_v, order_v+2));
      append_candidates_split(quad_order, last_quad_order, H2D_REFINEMENT_P, tri || iso_only);

      if (cands_allowed == H2DRS_CAND_HP) {
        //prepare HP-candidates
        int start_order_h = std::max(current_min_order, (order_h+1) / 2), start_order_v = std::max(current_min_order, (order_v+1) / 2);
        int start_quad_order = make_quad_order(start_order_h, start_order_v);
        last_quad_order = make_quad_order(std::min(max_ha_order_h, std::min(start_order_h + 3, order_h)), std::min(max_ha_order_v, std::min(start_order_v + 3, order_v)));
        append_candidates_split(start_quad_order, last_quad_order, H2D_REFINEMENT_H, tri || iso_only);

        //prepare ANISO-candidates
        if (!tri && !iso_only && (e->iro_cache < 8)) {
          //horizontal
          start_quad_order = make_quad_order(order_h, std::max(current_min_order, (order_v+1) / 2));
          last_quad_order = make_quad_order(order_h, std::min(max_ha_order_v, get_v_order(start_quad_order)+1));
          append_candidates_split(start_quad_order, last_quad_order, H2D_REFINEMENT_ANISO_H, false);

          //vertical
          start_quad_order = make_quad_order(std::max(current_min_order, (order_h+1) / 2), order_v);
          last_quad_order = make_quad_order(std::min(max_ha_order_h, get_h_order(start_quad_order)+1), order_v);
          append_candidates_split(start_quad_order, last_quad_order, H2D_REFINEMENT_ANISO_V, false);
        }
      }
    }
  }

  void OptimumSelector::calc_cands_max_order(int* max_quad_order_h, int* max_quad_order_p, int* max_quad_order_aniso) const {
    int order_h_cand_h = 0, order_v_cand_h = 0;
    int order_h_cand_p = 0, order_v_cand_p = 0;
    int order_h_cand_aniso = 0, order_v_cand_aniso = 0;

    std::vector<Cand>::const_iterator cand = candidates.begin();
    while (cand != candidates.end()) {
      int* order_h = NULL, * order_v = NULL;
      if (cand->split == H2D_REFINEMENT_H) { order_h = &order_h_cand_h; order_v = &order_v_cand_h; }
      else if (cand->split == H2D_REFINEMENT_P) { order_h = &order_h_cand_p; order_v = &order_v_cand_p; }
      else if (cand->split == H2D_REFINEMENT_ANISO_H || cand->split == H2D_REFINEMENT_ANISO_V) { order_h = &order_h_cand_aniso; order_v = &order_v_cand_aniso; }
      else { error("E invalid candidate type: %d", cand->split); };
      const int num_sons = cand->get_num_sons();
      for(int i = 0; i < num_sons; i++) {
        *order_h = std::max(*order_h, get_h_order(cand->p[i]));
        *order_v = std::max(*order_v, get_v_order(cand->p[i]));
      }
      cand++;
    }

    //encode
    *max_quad_order_h = make_quad_order(order_h_cand_h, order_v_cand_h);
    *max_quad_order_p = make_quad_order(order_h_cand_p, order_v_cand_p);
    *max_quad_order_aniso = make_quad_order(order_h_cand_aniso, order_v_cand_aniso);
  }

  void OptimumSelector::evaluate_cands_dof(Element* e, Solution* rsln) {
    bool tri = e->is_triangle();
#   define get_dof_bubble(__order_h, __order_v) ((__order_h - 1) * (__order_v - 1))
    for (unsigned i = 0; i < candidates.size(); i++) {
      Cand& c = candidates[i];
      if (tri) { //triangle
        switch(c.split) {
        case H2D_REFINEMENT_H:
          c.dofs = 6; //order 1
          for (int j = 0; j < 4; j++)
          {
            int order = get_h_order(c.p[j]);
            c.dofs += (order-2)*(order-1)/2;
            if (j < 3) c.dofs += std::min(order, get_h_order(c.p[3]))-1 + 2*(order-1);
          }
          break;

        case H2D_REFINEMENT_P:
          {
            int order = get_h_order(c.p[0]);
            c.dofs  = (order+1)*(order+2)/2;
          }
          break;

        default:
          assert_msg(false, "E unknown split type \"%d\" at candidate %d", c.split, i);
        }
      }
      else { //quad
        switch(c.split) {
        case H2D_REFINEMENT_H:
          c.dofs = 9; //vertex
          for (int j = 0; j < H2D_MAX_ELEMENT_SONS; j++)
          {
            int prev_j = j > 0 ? j-1 : H2D_MAX_ELEMENT_SONS-1; //neighbour
            int order_h = get_h_order(c.p[j]), order_v = get_v_order(c.p[j]);
            if ((j&1) == 0) { //shared edge is horizontal
              c.dofs += std::min(get_h_order(c.p[prev_j]), order_h)-1 + order_h-1; //the minimum order rule
              c.dofs += order_v-1; //the other edge will be added while processing of the next sons because it is shared
            }
            else { //shared edge is vertical
              c.dofs += std::min(get_v_order(c.p[prev_j]), order_v)-1 + order_v-1; //the minimum order rule
              c.dofs += order_h-1; //the other edge will be added while processing of the next sons because it is shared
            }
            c.dofs += get_dof_bubble(order_h, order_v); //bubble
          }
          break;

        case H2D_REFINEMENT_ANISO_H:
        case H2D_REFINEMENT_ANISO_V:
          {
            int order_major[2], order_minor[2];
            if (c.split == H2D_REFINEMENT_ANISO_H) {
              order_major[0] = get_h_order(c.p[0]); order_major[1] = get_h_order(c.p[1]);
              order_minor[0] = get_v_order(c.p[0]); order_minor[1] = get_v_order(c.p[1]);
            }
            else {
              order_major[0] = get_v_order(c.p[0]); order_major[1] = get_v_order(c.p[1]);
              order_minor[0] = get_h_order(c.p[0]); order_minor[1] = get_h_order(c.p[1]);
            }
            c.dofs = 6; //vertex
            c.dofs += 2 * (order_minor[0]-1 + order_minor[1]-1); //minor direction edge
            int order_shared_edge = std::min(order_major[0], order_major[1]); //the minimum order rule
            c.dofs += order_major[0]-1 + order_shared_edge-1 + order_major[1]-1; //major direction edge
            c.dofs += get_dof_bubble(order_major[0], order_minor[0]); //bubble
            c.dofs += get_dof_bubble(order_major[1], order_minor[1]); //bubble
          }
          break;

        case H2D_REFINEMENT_P:
          {
            int order_h = get_h_order(c.p[0]);
            int order_v = get_v_order(c.p[0]);
            c.dofs = 4; //vertex
            c.dofs += 2 * (order_h-1 + order_v-1); //edge
            c.dofs += (order_h-1) * (order_v-1); //bubble
          }
          break;

        default:
          assert_msg(false, "E unknown split type \"%d\" at candidate %d", c.split, i);
        }
      }
    }
  }

  void OptimumSelector::evaluate_candidates(Element* e, Solution* rsln, double* avg_error, double* dev_error) {
    evaluate_cands_error(e, rsln, avg_error, dev_error);
    evaluate_cands_dof(e, rsln);
  }

  void OptimumSelector::select_best_candidate(Element* e, const double avg_error, const double dev_error, int* selected_cand, int* selected_h_cand) {
    // select an above-average candidate with the steepest error decrease
    int imax = 0, h_imax = 0;
    double score, maxscore = 0.0, h_maxscore = 0.0;

    Cand& unrefined = candidates[0];
    for (unsigned i = 1; i < candidates.size(); i++)
    {
      Cand& cand = candidates[i];

      if ((log10(cand.error) < (avg_error + dev_error)) && (cand.dofs > unrefined.dofs))
      {
        score = (log10(unrefined.error) - log10(cand.error)) / pow(cand.dofs - unrefined.dofs, conv_exp);

        if (score > maxscore) { maxscore = score; imax = i; }
        if ((cand.split == 0) && (score > h_maxscore)) { h_maxscore = score; h_imax = i; }
      }
    }

    *selected_cand = imax;
    *selected_h_cand = h_imax;
  }

  bool OptimumSelector::select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement) {
    //make an uniform order in a case of a triangle
    int order_h = get_h_order(quad_order), order_v = get_v_order(quad_order);
    if (element->is_triangle()) {
      assert_msg(order_v == 0, "E element %d is a triangle but order_v (%d) is not zero", element->id, order_v);
      order_v = order_h;
      quad_order = make_quad_order(order_h, order_v); //in a case of a triangle, order_v is zero. Set it to order_h in order to simplify the routines.
    }

    //check validity
    assert_msg(!iso_only || order_h == order_v, "E iso_only requested but order (%d, %d) of element %d does not match", order_h, order_v);
    assert_msg(std::max(order_h, order_v) <= H2DRS_MAX_ORDER, "E given order (%d, %d) exceedes the maximum supported order %d", order_h, order_v, H2DRS_MAX_ORDER);

    //set shapeset mode
    if (element->is_triangle())
      shapeset->set_mode(MODE_TRIANGLE);
    else
      shapeset->set_mode(MODE_QUAD);

    //set orders
    set_current_order_range(element);

    //build candidates
    int inx_cand, inx_h_cand;
    create_candidates(element, quad_order
      , make_quad_order(current_max_order, current_max_order)
      , make_quad_order(current_max_order, current_max_order));
    if (candidates.size() > 1) { //there are candidates to choose from
      // evaluate candidates (sum partial projection errors, calculate dofs)
      double avg_error, dev_error;
      evaluate_candidates(element, rsln, &avg_error, &dev_error);

      //select candidate
      select_best_candidate(element, avg_error, dev_error, &inx_cand, &inx_h_cand);
    }
    else { //there is not candidate to choose from, select the original candidate
      inx_cand = 0;
      inx_h_cand = 0;
    }

    //copy result to output
    Cand& cand = candidates[inx_cand];
    Cand& cand_h = candidates[inx_h_cand];
    refinement.split = cand.split;
    ElementToRefine::copy_orders(refinement.p, cand.p);
    if (candidates[inx_h_cand].split == H2D_REFINEMENT_H) { //inx_h_cand points to a candidate which is a H-candidate: copy orders
      ElementToRefine::copy_orders(refinement.q, cand_h.p);
    }
    else { //the index is not H-candidate because not candidate was generate: so, fake orders
      int h_cand_orders[H2D_MAX_ELEMENT_SONS] = { cand_h.p[0], cand_h.p[0], cand_h.p[0], cand_h.p[0] };
      ElementToRefine::copy_orders(refinement.q, h_cand_orders);
    }

    //modify orders in a case of a triangle such that order_v is zero
    if (element->is_triangle())
    {
      for(int i = 0; i < H2D_MAX_ELEMENT_SONS; i++) {
        assert_msg(get_v_order(refinement.p[i]) == 0 || get_h_order(refinement.p[i]) == get_v_order(refinement.p[i]), "E triangle processed but the resulting order (%d, %d) of son %d is not uniform", get_h_order(refinement.p[i]), get_v_order(refinement.p[i]), i);
        refinement.p[i] = make_quad_order(get_h_order(refinement.p[i]), 0);
        assert_msg(get_v_order(refinement.q[i]) == 0 || get_h_order(refinement.q[i]) == get_v_order(refinement.q[i]), "E triangle processed but the resulting q-order (%d, %d) of son %d is not uniform", get_h_order(refinement.q[i]), get_v_order(refinement.q[i]), i);
        refinement.q[i] = make_quad_order(get_h_order(refinement.q[i]), 0);
      }
    }

    if (inx_cand == 0)
      return false;
    else
      return true;
  }

  void OptimumSelector::update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders) {
    assert_msg(refinement != H2D_REFINEMENT_P, "E P-candidate not supported for updating shared orders");
    const int num_sons = get_refin_sons(refinement);
    if (suggested_quad_orders != NULL) {
      for(int i = 0; i < num_sons; i++)
        tgt_quad_orders[i] = suggested_quad_orders[i];
    }
    else {
      //calculate new quad_orders
      int quad_order = orig_quad_order;
      if (cands_allowed != H2DRS_CAND_H_ONLY) {
        int order_h = get_h_order(quad_order), order_v = get_v_order(quad_order);
        switch(refinement) {
          case H2D_REFINEMENT_H:
            order_h = std::max(1, (order_h+1)/2);
            order_v = std::max(1, (order_v+1)/2);
            break;
          case H2D_REFINEMENT_ANISO_H:
            order_v = std::max(1, 2*(order_v+1)/3);
            break;
          case H2D_REFINEMENT_ANISO_V:
            order_h = std::max(1, 2*(order_h+1)/3);
            break;
        }
        if (element->is_triangle())
          quad_order = make_quad_order(order_h, 0);
        else
          quad_order = make_quad_order(order_h, order_v);
      }
      for(int i = 0; i < num_sons; i++)
        tgt_quad_orders[i] = quad_order;
    }
#ifdef _DEBUG
    for(int i = num_sons; i < H2D_MAX_ELEMENT_SONS; i++)
      tgt_quad_orders[i] = 0;
#endif
  }
}
