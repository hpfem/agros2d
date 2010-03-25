#include "../common.h"
#include "../solution.h"
#include "../element_to_refine.h"
#include "selector.h"

namespace RefinementSelectors {

  bool H1OnlyH::select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement) {
    refinement.split = 0;
    refinement.p[0] = refinement.p[1] = refinement.p[2] = refinement.p[3] = quad_order;
    refinement.q[0] = refinement.q[1] = refinement.q[2] = refinement.q[3] = quad_order;
    return true;
  }

  void H1OnlyH::update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders) {
    if (suggested_quad_orders != NULL)
      for(int i = 0; i < H2D_MAX_ELEMENT_SONS; i++)
        tgt_quad_orders[i] = suggested_quad_orders[i];
    else
      for(int i = 0; i < H2D_MAX_ELEMENT_SONS; i++)
        tgt_quad_orders[i] = orig_quad_order;
  }

  bool H1OnlyP::select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement) {
    refinement.split = -1;
    refinement.p[0] = refinement.q[0] = std::min(H2DRS_MAX_ORDER, get_h_order(quad_order) + 1);
    if (get_h_order(quad_order) < refinement.p[0])
      return true;
    else
      return false;
  }

  void H1OnlyP::update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders) {
    if (suggested_quad_orders != NULL)
      tgt_quad_orders[0] = suggested_quad_orders[0];
    else
      tgt_quad_orders[0] = orig_quad_order;
#ifdef _DEBUG
    for(int i = 1; i < H2D_MAX_ELEMENT_SONS; i++)
      tgt_quad_orders[i] = 0;
#endif
  }
}
