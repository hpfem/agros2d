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

#ifndef __H2D_REFINEMENT_SELECTORS_H1_NONUNIFORM_HP_H
#define __H2D_REFINEMENT_SELECTORS_H1_NONUNIFORM_HP_H

#include "proj_based_selector.h"

#define H2D_NUM_MODES 2 ///< A number of modes.
#define H2DRS_GIP_ORDER 20 ///< Order of GIP used by the application.

#define H2D_FN_VALUE  0
#define H2D_FN_DX     1
#define H2D_FN_DY     2

namespace RefinementSelectors {

  class H2D_API H1NonUniformHP : public ProjBasedSelector { ///< Selector that does HP-adaptivity using non-uniform orders on quadrilateral elements.
  protected: //projection and error evaluation
    template<typename T>
    struct ValueCacheItem { ///< An item of a value cache.
      inline bool is_valid() const { return state != 0; }; ///< Returns true, if value is valid.
      inline void mark_invalid() { state = 0; }; ///< Marks a value as invalid.
      inline void mark(int new_state = 1) { state = new_state; }; ///< Sets state of a value cache.
      inline void set(T new_value) { value = new_value; }; ///< Sets a new value.
      inline T get() { return value; }; ///< Returns value.
    private:
      T value; ///< Value.
      int state; ///< State.
    };

    struct ElemProj { ///< Element projection parameters.
      int* shape_inxs; ///< Shape indices
      int num_shapes; ///< Number of shape indices.
      scalar* shape_coefs; ///< Coefficients of shape indices of a projection.
      int max_quad_order; ///< Maximum quad order of the projection.
    };
    struct ElemGIP { ///< GIP on a element.
      double3* gip_points; ///< GIP points and weights.
      int num_gip_points; ///< A number of GIP points.
      scalar** rvals; ///< Values of a reference solution at GIP.
    };
    struct ElemSubTrf { ///< Transformation from a coordinate system of a sub-element to coordinate system of an element.
      Trf* trf; ///< Transformation.
      double coef_mx, coef_my; ///< Differentials correction coefficients.
    };

    double** proj_matrices[H2DRS_MAX_ORDER+1][H2DRS_MAX_ORDER+1]; ///< An array of projection matrices. Used functions are defined through shape_inx. Index to the array is the size. All matrices are square. If record is NULL, the corresponding matrix has to be calculated.
    ValueCacheItem<scalar>* rhs_cache; ///< An array of RHS values. Valid only during evalution of proj_calc_err_son.

    double** build_projection_matrix(Shapeset& shapeset, double3* gip_points, int num_gip_points, const int* shape_inx, const int num_shapes); ///< Builds a projection matrix.
    void proj_calc_err_son(const int mode, double3* gip_points, int num_gip_points, const int num_sub, Element** sub_elems, Trf** sub_trfs, scalar*** sub_rvals, double* coefs_mx, double* coefs_my, int max_quad_order, SonProjectionError errors); ///< Calculate projection errors.
    virtual scalar evaluate_rsh_sub_element(Element* sub_elem, const ElemGIP& sub_gip, const ElemSubTrf& sub_trf, int shape_inx); ///> Evaluate a single value of the right side for a sub-element. Provided GIP are defined on a reference domain. Provided transformation will transform form a reference domain of a sub-element to a reference domain of an element.
    virtual double evaluate_error_sub_element(Element* sub_elem, const ElemGIP& sub_gip, const ElemSubTrf& sub_trf, const ElemProj& elem_proj); ///> Evaluate an error of a projection on a sub-element. Provided GIP are defined on a reference domain. Provided transformation will transform form a reference domain of a sub-element to a reference domain of an element.

    virtual void calc_projection_errors(Element* e, const int max_quad_order_h, const int max_quad_order_p, const int max_quad_order_aniso, Solution* rsln, SonProjectionError herr[4], SonProjectionError anisoerr[4], SonProjectionError perr);     ///> Overloaded. Calculate various projection errors for sons of a candidates of given combination of orders.

  protected: //shape functions
    struct ShapeInx { ///< A shape index.
      int order_h; ///< Order in H direction. Zero if the shape is just along V-direction.
      int order_v; ///< Order in H direction. Zero if the shape is just along V-direction.
      int inx; ///< Index of a shape.
      ShapeInx(int order_h, int order_v, int inx) : order_h(order_h), order_v(order_v), inx(inx) {};
    };

    static H1Shapeset default_shapeset; ///< Default shapeset.
    std::vector<ShapeInx> shape_indices[H2D_NUM_MODES]; ///< Shape indices.
    int max_shape_inx[H2D_NUM_MODES]; ///< Maximum of shape indices.
    int next_order_shape[H2D_NUM_MODES][H2DRS_MAX_ORDER+1]; ///< An index of a shape index of the next order.

    void evalute_shape_indices(const int mode); ///< Evaluates shape indices.
    void build_shape_indices(const int mode); ///< Build shape indices.

  public:
    H1NonUniformHP(bool iso_only, AllowedCandidates cands_allowed = H2DRS_CAND_HP, double conv_exp = 1.0, int max_order = H2DRS_DEFAULT_ORDER, H1Shapeset* user_shapeset = NULL);
    virtual ~H1NonUniformHP();
  };
}

#endif
