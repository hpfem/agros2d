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

#ifndef __H2D_ELEMENT_TO_REFINE_H
#define __H2D_ELEMENT_TO_REFINE_H

#include "refinement_type.h"

struct H2D_API ElementToRefine { ///< A record of an element and a selected candidate.
  int id; //ID of element
  int comp; //componet
  int split; //proposed refinement
  int p[H2D_MAX_ELEMENT_SONS]; //Ecoded orders of sons. If V order is zero, V order is equal to U order.
  int q[H2D_MAX_ELEMENT_SONS]; //Encoded H orders of sons. If V order is zero, V order is equal to U order.

public:
  ElementToRefine() : id(-1), comp(-1) {};
  ElementToRefine(int id, int comp) : id(id), comp(comp), split(H2D_REFINEMENT_H) {};
  ElementToRefine(const ElementToRefine &orig) : id(orig.id), comp(orig.comp), split(orig.split) {
    copy_orders(p, orig.p);
    copy_orders(q, orig.q);
  };
  ElementToRefine& operator=(const ElementToRefine& orig);
  int get_num_sons() const { return get_refin_sons(split); }; ///< Returns a number of sons.
  static inline void copy_orders(int* dest, const int* src) { ///< Copies array of orders.
    memcpy(dest, src, sizeof(int) * H2D_MAX_ELEMENT_SONS);
  }
};

extern H2D_API std::ostream& operator<<(std::ostream& stream, const ElementToRefine& elem_ref); ///< Dumps contants of the structure. Used for debugging purposes.

#endif
