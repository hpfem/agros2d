#include "common.h"
#include "element_to_refine.h"

ElementToRefine& ElementToRefine::operator=(const ElementToRefine& orig) {
  id = orig.id;
  comp = orig.comp;
  split = orig.split;
  copy_orders(p, orig.p);
  copy_orders(q, orig.q);
  return *this;
}

std::ostream& operator<<(std::ostream& stream, const ElementToRefine& elem_ref) {
  stream << "id:" << elem_ref.id << ";comp:" << elem_ref.comp << "; split:" << get_refin_str(elem_ref.split) << "; orders:[";
  int num_sons = elem_ref.get_num_sons();
  for(int i = 0; i < num_sons; i++) {
    if (i > 0)
      stream << " ";
    stream << get_quad_order_str(elem_ref.p[i]);
  }
  stream << "]";
  return stream;
}
