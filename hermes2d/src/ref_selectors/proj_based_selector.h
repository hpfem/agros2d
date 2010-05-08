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

#ifndef __H2D_REFINEMENT_PROJ_BASED_SELECTOR_H
#define __H2D_REFINEMENT_PROJ_BASED_SELECTOR_H

#include "optimum_selector.h"

namespace RefinementSelectors {
  typedef double SonProjectionError[H2DRS_MAX_ORDER+2][H2DRS_MAX_ORDER+2]; ///< Error of a son of a candidate for various order combinations. The maximum allowed order is H2DRS_MAX_ORDER+1.

  class H2D_API ProjBasedSelector : public OptimumSelector {
  protected:
    /// \brief Calculate various projection errors for sons of a candidates of given combination of orders. Errors are not normalized. Overloadable.
    /// \param[in] e Element which is being processed.
    /// \param[in] rsln Reference solution.
    /// \param[out] herr Errors of sons of H-candidate.
    /// \param[out] anisoerr Errors of sons of ANISO-candidates.
    /// \param[out] perr Errros of sons of P-candidates.
    virtual void calc_projection_errors(Element* e, const int max_quad_order_h, const int max_quad_order_p, const int max_quad_order_aniso, Solution* rsln, SonProjectionError herr[4], SonProjectionError anisoerr[4], SonProjectionError perr) = 0;
    virtual void evaluate_cands_error(Element* e, Solution* rsln, double* avg_error, double* dev_error); ///< Calculates error of candidates.

  public:
    ProjBasedSelector(bool iso_only, AllowedCandidates cands_allowed, double conv_exp, int max_order, Shapeset* shapeset)
	  : OptimumSelector(iso_only, cands_allowed, conv_exp, max_order, shapeset) {};
  };

}

#endif
