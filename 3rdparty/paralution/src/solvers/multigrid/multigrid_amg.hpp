// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2014 Dimitar Lukarski
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************



// PARALUTION version 0.7.0 


#ifndef PARALUTION_MULTIGRID_AMG_HPP_
#define PARALUTION_MULTIGRID_AMG_HPP_

#include "../solver.hpp"
#include "multigrid.hpp"

#include <vector>

namespace paralution {

enum _interp {
  Aggregation,
  SmoothedAggregation
};

template <class OperatorType, class VectorType, typename ValueType>
class AMG : public MultiGrid<OperatorType, VectorType, ValueType> {
  
public:

  AMG();
  virtual ~AMG();

  virtual void Print(void) const;

  virtual void Build(void);
  virtual void Clear(void);

  /// Creates AMG hierarchy
  virtual void BuildHierarchy(void);

  /// Sets coupling strength
  virtual void SetCouplingStrength(const ValueType eps);
  /// Sets coarsest level for hierarchy creation
  virtual void SetCoarsestLevel(const int coarseSize);

  /// Sets flag to pass smoothers manually for each level
  virtual void SetManualSmoothers(const bool sm_manual);
  /// Sets flag to pass coarse grid solver manually
  virtual void SetManualSolver(const bool s_manual);

  /// Sets the interpolation type
  virtual void SetInterpolation(_interp interpType);
  /// Sets the relaxation parameter for smoothed aggregation
  virtual void SetInterpRelax(const ValueType relax);
  /// Sets over-interpolation parameter for aggregation
  virtual void SetOverInterp(const ValueType overInterp);

  /// Sets the smoother operator format
  virtual void SetDefaultSmootherFormat(const unsigned int op_format);
  /// Sets the operator format
  virtual void SetOperatorFormat(const unsigned int op_format);

  /// Returns the number of levels in hierarchy
  virtual int GetNumLevels();

protected:

  /// Constructs prolongation and restriction operator by smoothed aggregation
  virtual void SmoothedAggr(const OperatorType &op,
                            const LocalVector<int> &aggregates,
                            const LocalVector<int> &connections,
                            OperatorType *prolong,
                            OperatorType *restrict) const;

  /// Constructs prolongation and restriction operator by aggregation
  virtual void Aggr(const OperatorType &op,
                    const LocalVector<int> &aggregates,
                    OperatorType *prolong,
                    OperatorType *restrict) const;

  /// Constructs vector with connections
  virtual void Connect(const OperatorType &op, const ValueType eps,
                       LocalVector<int> *connections) const;

  /// Constructs vector with aggregate numbers
  virtual void Aggregate(const OperatorType &op,
                         const LocalVector<int> &connections,
                         LocalVector<int> *aggregates) const;

  /// Builds coarse operator
  virtual void CoarsenOperator(const OperatorType &restrict,
                               const OperatorType &prolong,
                               const OperatorType &op_fine,
                               OperatorType *op_coarse);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

  ValueType eps_;
  ValueType relax_;
  ValueType over_interp_;
  
  /// interpolation type for grid transfer operators
  _interp interp_type_;
  /// maximal coarse grid size
  int coarse_size_;
  /// manual smoother or not
  bool set_sm_;
  Solver<OperatorType, VectorType, ValueType> **sm_default_;
  /// manual coarse grid solver or not
  bool set_s_;
  /// true if hierarchy is built
  bool hierarchy_;

  /// smoother operator format
  unsigned int sm_format_;
  /// operator format
  unsigned int op_format_;

};


}

#endif // PARALUTION_MULTIGRID_AMG_HPP_

