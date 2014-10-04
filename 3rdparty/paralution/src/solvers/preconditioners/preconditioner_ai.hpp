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


#ifndef PARALUTION_PRECONDITIONER_AI_HPP_
#define PARALUTION_PRECONDITIONER_AI_HPP_

#include "../solver.hpp"
#include "preconditioner.hpp"

namespace paralution {

/// Approximate Inverse - Chebyshev preconditioner
/// see IEEE TRANSACTIONS ON POWER SYSTEMS, VOL. 18, NO. 4, NOVEMBER 2003;
/// A New Preconditioned Conjugate Gradient Power Flow -
/// Hasan Dag, Adam Semlyen
template <class OperatorType, class VectorType, typename ValueType>
class AIChebyshev : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  AIChebyshev();
  virtual ~AIChebyshev();

  virtual void Print(void) const;  
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);
  virtual void Set(const int p, const ValueType lambda_min, const ValueType lambda_max);
  virtual void Build(void);
  virtual void Clear(void);  


protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType AIChebyshev_;
  int p_;
  ValueType lambda_min_, lambda_max_;

};

/// Factorized Approximate Inverse preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class FSAI : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  FSAI();
  virtual ~FSAI();

  virtual void Print(void) const;
  virtual void Solve(const VectorType &rhs, VectorType *x);
  /// Initialize the FSAI with powered system matrix sparsity pattern
  virtual void Set(const int power);
  /// Initialize the FSAI with external sparsity pattern
  virtual void Set(const OperatorType &pattern);
  virtual void Build(void);
  virtual void Clear(void);

  virtual void SetPrecondMatrixFormat(const unsigned int mat_format);

protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);

private:

  OperatorType FSAI_L_;
  OperatorType FSAI_LT_;
  VectorType t_;

  int matrix_power_;

  bool external_pattern_;
  const OperatorType *matrix_pattern_;

  /// Keep the precond matrix in CSR or not
  bool op_mat_format_;
  /// Precond matrix format
  unsigned int precond_mat_format_;

};

/// SParse Approximate Inverse preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class SPAI : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  SPAI();
  virtual ~SPAI();

  virtual void Print(void) const;  
  virtual void Solve(const VectorType &rhs, VectorType *x);
  virtual void Build(void);
  virtual void Clear(void);

  virtual void SetPrecondMatrixFormat(const unsigned int mat_format);


protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType SPAI_;

  /// Keep the precond matrix in CSR or not
  bool op_mat_format_; 
  /// Precond matrix format
  unsigned int precond_mat_format_;

};


}

#endif // PARALUTION_PRECONDITIONER_AI_HPP_

