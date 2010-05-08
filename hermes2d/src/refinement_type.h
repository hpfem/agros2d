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

#ifndef __H2D_REFINEMENT_TYPE_H
#define __H2D_REFINEMENT_TYPE_H

#define H2D_MAX_ELEMENT_SONS 4 ///< A maximum number of sons generated through refinement

#define H2D_REFINEMENT_P -1
#define H2D_REFINEMENT_H 0
#define H2D_REFINEMENT_ANISO_H 1
#define H2D_REFINEMENT_ANISO_V 2

extern H2D_API int get_refin_sons(const int refin_type); ///< Returns number of sons.
extern H2D_API const std::string get_refin_str(const int refin_type); ///< Returns name of a refinement type value. Used for debugging purposes.

#endif
