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


#ifndef PARALUTION_PRECONDITIONER_AS_HPP_
#define PARALUTION_PRECONDITIONER_AS_HPP_

#include "preconditioner.hpp"

namespace paralution {

/// AS preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class AS : public Preconditioner<OperatorType, VectorType, ValueType> {

public:

  AS();
  virtual ~AS();

  virtual void Print(void) const;
  virtual void Set(const int nb, const int overlap,
                   Solver<OperatorType, VectorType, ValueType> **preconds);
  
  virtual void Solve(const VectorType &rhs,
                     VectorType *x);


  virtual void Build(void);
  virtual void Clear(void);

protected:

  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void) ;

  int num_blocks_;
  int overlap_;
  int *pos_;
  int *sizes_; // with overlap

  Solver<OperatorType, VectorType, ValueType> **local_precond_;

  OperatorType **local_mat_;
  VectorType **r_;
  VectorType **z_;
  VectorType weight_;

};

/// AS preconditioner
template <class OperatorType, class VectorType, typename ValueType>
class RAS : public AS<OperatorType, VectorType, ValueType> {

public:

  RAS();
  virtual ~RAS();

  virtual void Print(void) const;

  virtual void Solve(const VectorType &rhs,
                     VectorType *x);

};

};

#endif // PARALUTION_PRECONDITIONER_AS_HPP_
