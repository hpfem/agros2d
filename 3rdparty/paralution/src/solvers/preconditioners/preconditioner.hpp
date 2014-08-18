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


#ifndef PARALUTION_PRECONDITIONER_HPP_
#define PARALUTION_PRECONDITIONER_HPP_

#include "../solver.hpp"

namespace paralution {

/// Base preconditioner class
template <class OperatorType, class VectorType, typename ValueType>
class Preconditioner : public Solver<OperatorType, VectorType, ValueType> {

public:

  Preconditioner();
  virtual ~Preconditioner();

  virtual void SolveZeroSol(const VectorType &rhs,
                            VectorType *x);

protected:

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;


};

//// Jacobi preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class Jacobi : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  Jacobi();
  virtual ~Jacobi();

  virtual void Print(void) const;
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);
  virtual void Build(void);
  virtual void Clear(void);

  virtual void ResetOperator(const OperatorType &op);

protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void) ;


private:

  VectorType inv_diag_entries_;

};

/// Gauss-Seidel (GS) preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class GS : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  GS();
  virtual ~GS();

  virtual void Print(void) const;
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);
  virtual void Build(void);
  virtual void Clear(void);

  virtual void ResetOperator(const OperatorType &op);

protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType GS_;

};

/// Symmetric Gauss-Seidel (SGS) preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class SGS : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  SGS();
  virtual ~SGS();

  virtual void Print(void) const;
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);
  virtual void Build(void);
  virtual void Clear(void);

  virtual void ResetOperator(const OperatorType &op);

protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType SGS_;

  VectorType diag_entries_;
  VectorType v_;

};

/// ILU preconditioner based on levels
template <class OperatorType, class VectorType, typename ValueType>
class ILU : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  ILU();
  virtual ~ILU();

  virtual void Print(void) const;  
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);

  /// Initialize ILU(p) factorization based on power (see power(q)-pattern method, 
  /// D. Lukarski "Parallel Sparse Linear Algebra for Multi-core and Many-core 
  /// Platforms - Parallel Solvers and Preconditioners", PhD Thesis, 2012, KIT) 
  /// level==true build the structure based on levels; level==false build the
  /// structure only based on the power(p+1)
  virtual void Set(const int p, const bool level=true);
  virtual void Build(void);
  virtual void Clear(void);  


protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType ILU_;
  int p_;
  bool level_;

};

/// ILUT(t,m) preconditioner based on threshold and maximum number
/// of elements per row
template <class OperatorType, class VectorType, typename ValueType>
class ILUT : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  ILUT();
  virtual ~ILUT();

  virtual void Print(void) const;  
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);

  /// ILUT with threshold
  virtual void Set(const ValueType t);

  /// ILUT with threshold and maximum number of elements per row
  virtual void Set(const ValueType t, const int maxrow);

  virtual void Build(void);
  virtual void Clear(void);  


protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType ILUT_;
  ValueType t_;
  int max_row_;

};

/// Incomplete Cholesky with no fill-ins IC0
template <class OperatorType, class VectorType, typename ValueType>
class IC : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  IC();
  virtual ~IC();

  virtual void Print(void) const;  
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);
  virtual void Build(void);
  virtual void Clear(void);  


protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);


private:

  OperatorType IC_;
  VectorType inv_diag_entries_;

};


}

#endif // PARALUTION_PRECONDITIONER_HPP_

