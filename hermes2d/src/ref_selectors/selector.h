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

#ifndef __H2D_REFINEMENT_SELECTOR_H
#define __H2D_REFINEMENT_SELECTOR_H

#include "../refinement_type.h"

struct ElementToRefine;
struct Element;
class Solution;

#define H2DRS_DEFAULT_ORDER -1 ///< A default order.
#define H2DRS_MAX_ORDER 9 ///< A maximum order.

namespace RefinementSelectors {
  class H2D_API Selector { ///< Basic class for selecting a refinement. All selectors has to inherit from this class.
  public:
    const int max_order; ///< A maximum allowed order.
  public:
    Selector(int max_order = H2DRS_DEFAULT_ORDER) : max_order(max_order) {};
    virtual ~Selector() {};
    virtual void reset() {}; ///< Clears internal structures of a selector.
    /// \brief Selects refinement.
    /// \param quad_order Encoded order.
    /// \param result Defined refinement. ID and comp attribute of the structure ElementToRefine should be initialized beforehand.
    /// \return False if the selector suggests to keep the element intact.
    virtual bool select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement) = 0;

    /// \brief Updates orders of a refinement in another multimesh component which shares a mesh.
    /// \param element Element which is about the be refined.
    /// \param orig_quad_order Original quad order of the element prior any updating.
    /// \param refinement Refinement of the element in the mesh.
    /// \param tgt_quad_orders Updated quad orders.
    /// \param suggested_quad_orders Suggested quad orders. If NULL, orders has to be calculated.
    virtual void update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders) = 0;
  };

  class H2D_API H1OnlyH : public Selector { ///< Selector that does only H-adaptivity.
  public:
    H1OnlyH() : Selector() {};
    virtual bool select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement); ///< Suggests refinement.
    virtual void update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders); ///< Updates orders of a refinement in another multimesh component which shares a mesh.
  };

  class H2D_API H1OnlyP : public Selector { ///< Selector that does only P-adaptivity.
  public:
    H1OnlyP(int max_order) : Selector(max_order) {};
    virtual bool select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement); ///< Suggests refinement.
    virtual void update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders); ///< Updates orders of a refinement in another multimesh component which shares a mesh.
  };
}

#endif
