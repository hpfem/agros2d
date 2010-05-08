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

#ifndef __H2D_ADAPT_ORTHO_H1_H
#define __H2D_ADAPT_ORTHO_H1_H

#include "forms.h"
#include "weakform.h"
#include "integrals_h1.h"

/// \brief hp-adaptivity module for H1 spaces.
///
/// H1OrthoHP is a fast hp-adaptivity module for continuous elements.
/// Given a reference solution, it provides functions to calculate H1 or
/// energy error estimates, acts as a container for the calculated errors
/// and uses the "ortho" hp-adaptivty selector by default which is fast.
/// This classes is provided only for backward compatibility only
/// and should not be used in new projects.
///
class H2D_API H1OrthoHP : public H1AdaptHP
{
public:

  /// Initializes the class. 'num' is the number of mesh-space pairs to be adapted.
  /// After 'num', exactly that many space pointers must be used. The rest of space
  /// pointers is ignored.
  /// These constructors are due to backward compatibility since the original class
  /// used variable number of parameters instead of an array or any another similar
  /// structure. The maximum number of variables it taken from the size of the array
  /// H1AdaptHP::space.
  H1OrthoHP(int num, Space* space1, Space* space2 = NULL, Space* space3 = NULL, Space* space4 = NULL, Space* space5 = NULL, Space* space6 = NULL, Space* space7 = NULL, Space* space8 = NULL, Space* space9 = NULL, Space* space10 = NULL);

  /// Refines elements based on results from calc_error() or calc_energy_error(). (deprecated version)
  bool adapt(double thr, int strat = 0, int adapt_type = 0, bool iso_only = false, int regularize = -1,
             double conv_exp = 1.0, int max_order = -1, bool same_orders = false, double to_be_processed = 0.0);
};


#endif
