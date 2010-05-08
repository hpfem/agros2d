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

#ifndef __H2D_REFINEMENT_OPTIMUM_SELECTOR_H
#define __H2D_REFINEMENT_OPTIMUM_SELECTOR_H

#include <ostream>
#include "selector.h"

#define H2DRS_ASSUMED_MAX_CANDS 512 ///< A maximum estimated number of candidates. Used for purpose of reserving a space.

namespace RefinementSelectors {

  enum AllowedCandidates { ///< Candidate types allowed.
    H2DRS_CAND_HP, ///< HP-candidates.
	  H2DRS_CAND_H_ONLY, ///< H-candidates only candidates.
	  H2DRS_CAND_P_ONLY, ///< P-candidates only.
  };

  class H2D_API OptimumSelector : public Selector { ///< Selector that chooses an optimal candidates based on error decrease per a new DOF.
  public: //candidates
    struct Cand { ///< A candidate.
      double error; ///< Error of this candidate.
      int dofs;  ///< Estimated number of DOFs.
      int split; ///< Operation.
      int p[4]; ///< Encoded orders of sons. If V order is zero, V order is equal to U order.

      Cand() {};
      Cand(const int split, const int order_sons[4])
        : split(split) {
          p[0] = order_sons[0];
          p[1] = order_sons[1];
          p[2] = order_sons[2];
          p[3] = order_sons[3];
      };
      Cand(const int split, const int order_son0, const int order_son1 = 0, const int order_son2 = 0, const int order_son3 = 0)
        : split(split) {
          p[0] = order_son0;
          p[1] = order_son1;
          p[2] = order_son2;
          p[3] = order_son3;
      };
      int get_num_sons() const { ///< Returns number of sons.
        switch (split) {
          case H2D_REFINEMENT_H: return 4;
          case H2D_REFINEMENT_P: return 1;
          case H2D_REFINEMENT_ANISO_H:
          case H2D_REFINEMENT_ANISO_V:
            return 2;
          default:
            error("invalid split type %d", split);
            return -1;
            break;
        }
      }

      friend H2D_API std::ostream& operator<<(std::ostream& stream, const Cand& cand);
    };
    const std::vector<Cand>& get_candidates() const { return candidates; }; ///< Returns current candidates.

  protected: //candidates
    class OrderPermutator { ///< Permutates orders.
    protected:
      int order_h, order_v; ///< Current order
      int start_order_h, start_order_v; ///< Start orders
      int end_order_h, end_order_v; ///< End orders
      bool uniform; /// True, if order step is uniform.
      int* tgt_quad_order; ///< Target quad order to which the order is written automatically. NULL if none.
    public:
      OrderPermutator () {};
      OrderPermutator (int start_quad_order, int end_quad_order, bool uniform, int* tgt_quad_order = NULL);
      bool next(); ///< Returns false if there is not next combination.
      void reset(); ///< Resets permutator to the start order.
      int get_order_h() const { return order_h; }; ///< Returns horizontal order.
      int get_order_v() const { return order_v; }; ///< Returns vertical order.
      int get_quad_order() const { return H2D_MAKE_QUAD_ORDER(order_h, order_v); }; ///< Returns quad order.
    };

    bool iso_only; ///< True if ANISO-candidates are not allowed.
    AllowedCandidates cands_allowed; ///< Allowed candidate types.
	  double conv_exp; ///< Convergence power. Modifies difference between DOFs before they are used to calculate the score.
    std::vector<Cand> candidates; ///< A vector of candidates. The first candidate is the original element.
    void calc_cands_max_order(int* max_quad_order_h, int* max_quad_order_p, int* max_quad_order_aniso) const; ///< Calculates maximum quad orders of candidates.

    void append_candidates_split(const int start_quad_order, const int last_order, const int split, bool uniform); ///< Creates candidates of a given split-type.

    virtual void create_candidates(Element* e, int quad_order, int max_ha_quad_order, int max_p_quad_order); ///< Initializes the array of candidates and fills it with candidates. Maximum orderds for H- and ANISO-candidates are given by max_ha_quad_order and for P-candidates max_p_quad_order. In the case of ANIO-candidates, the maximum is applied only only to modified orders.
    virtual void evaluate_candidates(Element* e, Solution* rsln, double* avg_error, double* dev_error); ///< Evaluates candidates. Calculates their error and dofs. Calculates average error and sample deviation.
    virtual void select_best_candidate(Element* e, const double avg_error, const double dev_error, int* selected_cand, int* selected_h_cand); ///< Selects the best candidate and the best h-candidate.

    virtual void evaluate_cands_error(Element* e, Solution* rsln, double* avg_error, double* dev_error) = 0; ///< Calculates error of candidates.
    virtual void evaluate_cands_dof(Element* e, Solution* rsln); ///< Calculates DOFs of candidates.

  protected: //orders and their range
    int current_max_order; ///< Current maximum order.
    int current_min_order; ///< Current minimum order.

    virtual void set_current_order_range(Element* element); ///< Sets current maximum and minimum order. If the max_order is H2DRS_DEFAULT_ORDER, in the case of linear elements it uses 9 and in the case of curvilinear elements it depends on iro_cache (how curved they are).

  protected: //shapefunctions
    Shapeset *shapeset; ///< A shapeset used for projections.

  public:
    OptimumSelector(bool iso_only, AllowedCandidates cands_allowed, double conv_exp, int max_order, Shapeset* shapeset);
    virtual ~OptimumSelector() {};
    virtual bool select_refinement(Element* element, int quad_order, Solution* rsln, ElementToRefine& refinement); ///< Selects refinement.
    virtual void update_shared_mesh_orders(const Element* element, const int orig_quad_order, const int refinement, int tgt_quad_orders[H2D_MAX_ELEMENT_SONS], const int* suggested_quad_orders); ///< Updates orders of a refinement in another multimesh component which shares a mesh.
  };

  extern H2D_API std::ostream& operator<<(std::ostream& stream, const OptimumSelector::Cand& cand); ///< Flushes contents of a candidate to a stream. Useful for debug print-outs.
}

#endif
