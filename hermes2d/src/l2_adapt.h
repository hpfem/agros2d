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

#ifndef __H2D_L2_ADAPT_H
#define __H2D_L2_ADAPT_H

/// Evaluation of an error between a (coarse) solution and a reference solution and adaptivity in L2 space. \ingroup g_adapt
/** The class provides functionality necessary to adaptively refine elements in L2 space.
 *  Given a reference solution and a coarse solution, it calculates error estimates
 *  and it acts as a container for the calculated errors.
 *  The class works best with a selector L2ProjBasedSelector.
 */
class H2D_API L2Adapt : public Adapt {
public:
  /// Constructor.
  /** \param[in] spaces An array of spaces. The number of spaces determines the number of components. For the best results, use instances of the class L2Space. */
  L2Adapt(const Tuple<Space*>& spaces);
};

#endif
