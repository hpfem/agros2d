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

#ifndef __H2D_REFINEMENT_SELECTORS_H1_UNIFORM_HP_H
#define __H2D_REFINEMENT_SELECTORS_H1_UNIFORM_HP_H

#include "proj_based_selector.h"

namespace RefinementSelectors {
  class H2D_API H1UniformHP : public ProjBasedSelector { ///< Selector that does HP-adaptivity using uniform orders on quadrilateral elements.
  protected: //candidates
    /// \brief Calculate various projection errors for sons of a candidates of given combination of orders. Errors are not normalized. Overloadable.
    /// \param[in] e Element which is being processed.
    /// \param[in] rsln Reference solution.
    /// \param[out] herr Errors of sons of H-candidate.
    /// \param[out] anisoerr Errors of sons of ANISO-candidates.
    /// \param[out] perr Errros of sons of P-candidates.
    virtual void calc_projection_errors(Element* e, const int max_quad_order_h, const int max_quad_order_p, const int max_quad_order_aniso, Solution* rsln, SonProjectionError herr[4], SonProjectionError anisoerr[4], SonProjectionError perr);
    virtual void create_candidates(Element* e, int quad_order, int max_ha_quad_order, int max_p_quad_order); ///< Initializes the array of candidates and fills it with candidates.

  protected: //shapeset
    static H1Shapeset default_shapeset; ///< Default shapeset.

  protected: //orthonormalized base
    double3** obase[2][9]; ///< Values at GIP of orthonormalized base. first index: 0 = triangles, 1 = quads; second index: order; third index: index of a shape function; fouth index: index of GIP; fifth index: 0 = value, 1 = df/dx, 2 = df/dy
    int basecnt[2][11]; ///< Indices of the first shape functions from H1Shapeset of the given order. first index: 0 = triangles, 1 = quads; second index: order
    bool obase_ready; ///< True if orthonormalized base is not ready.

    void calc_ortho_base(); ///< Calculates orthonormalized base.
    void free_ortho_base(); ///< Deallocates allocated structures.
    int build_shape_inxs(const int mode, Shapeset *shapeset, int idx[121]); ///< Build indices of shape functions and initializes ranges. Returns number of indices.

  public:
    H1UniformHP(bool iso_only, AllowedCandidates cands_allowed = H2DRS_CAND_HP, double conv_exp = 1.0, int max_order = H2DRS_DEFAULT_ORDER, H1Shapeset* user_shapeset = NULL);
    virtual ~H1UniformHP();
    virtual void update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders); ///< Updates orders of a refinement in another multimesh component which shares a mesh.
  };
}

#endif
