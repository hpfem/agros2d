#include "common.h"
#include "refinement_type.h"

int get_refin_sons(const int refin_type) {
  switch(refin_type) {
    case H2D_REFINEMENT_P: return 1; break;
    case H2D_REFINEMENT_H: return 4; break;
    case H2D_REFINEMENT_ANISO_H:
    case H2D_REFINEMENT_ANISO_V: return 2; break;
    default: error("invalid refinement type %d", (int)refin_type); return -1;
  }
}

const std::string get_refin_str(const int refin_type) {
  switch(refin_type) {
    case H2D_REFINEMENT_P: return "P"; break;
    case H2D_REFINEMENT_H: return "H"; break;
    case H2D_REFINEMENT_ANISO_H: return "AnisoH"; break;
    case H2D_REFINEMENT_ANISO_V: return "AnisoV"; break;
    default:
      std::stringstream str;
      str << "Unknown(" << refin_type << ")";
      return str.str();
  }
}
