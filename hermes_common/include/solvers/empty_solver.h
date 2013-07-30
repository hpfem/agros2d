// This file is part of HermesCommon
//
// Copyright (c) 2009 hp-FEM group at the University of Nevada, Reno (UNR).
// Email: hpfem-group@unr.edu, home page: http://hpfem.org/.
//
// Hermes2D is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
/*! \file empty_solver.h
\brief EMPTY solver interface.
*/
#ifndef __HERMES_COMMON_EMPTY_SOLVER_H_
#define __HERMES_COMMON_EMPTY_SOLVER_H_
#include "config.h"
#include "linear_matrix_solver.h"
#include "cs_matrix.h"

using namespace Hermes::Algebra;

namespace Hermes
{
  namespace Solvers
  {
    template <typename Scalar> class HERMES_API EmptySolver;
  }

  namespace Algebra
  {
    /// \brief This class is to be used with Empty solver only.
    template <typename Scalar>
    class HERMES_API EmptyMatrix : public CSCMatrix<Scalar>
    {
      // Friends.
      template <typename T> friend class Hermes::Solvers::EmptySolver;
      template <typename T> friend class Hermes::Solvers::CSCIterator;
      template<typename T> friend SparseMatrix<T>*  create_matrix();
    };

    /// \brief Class representing the vector for EMPTY.
    template <typename Scalar>
    class HERMES_API EmptyVector : public Vector<Scalar>
    {
    public:
      /// Default constructor.
      EmptyVector();
      /// Constructor of vector with specific size.
      /// @param[in] size size of vector
      EmptyVector(unsigned int size);
      virtual ~EmptyVector();
      virtual void alloc(unsigned int ndofs);
      virtual void free();
      virtual Scalar get(unsigned int idx) const;
      virtual void extract(Scalar *v) const;
      virtual void zero();
      virtual void change_sign();
      virtual void set(unsigned int idx, Scalar y);
      virtual void add(unsigned int idx, Scalar y);
      virtual void add(unsigned int n, unsigned int *idx, Scalar *y);
      virtual void set_vector(Vector<Scalar>* vec);
      virtual void set_vector(Scalar* vec);
      virtual void add_vector(Vector<Scalar>* vec);
      virtual void add_vector(Scalar* vec);
      virtual bool dump(FILE *file, const char *var_name, EMatrixDumpFormat fmt = DF_MATLAB_SPARSE, char* number_format = "%lf");

    protected:
      /// Empty specific data structures for storing the rhs.
      Scalar *v;
      template <typename T> friend class Hermes::Solvers::EmptySolver;
      template <typename T> friend class Hermes::Solvers::CSCIterator;
      template<typename T> friend Vector<T>* Hermes::Algebra::create_vector(bool);
    };
  }
  namespace Solvers
  {
    /// \brief Encapsulation of EMPTY linear solver.
    ///
    /// @ingroup Solvers
    template <typename Scalar>
    class HERMES_API EmptySolver : public DirectSolver<Scalar>
    {
    public:
      /// Constructor of Empty solver.
      /// @param[in] m pointer to matrix
      /// @param[in] rhs pointer to right hand side vector
      EmptySolver(EmptyMatrix<Scalar> *m, EmptyVector<Scalar> *rhs);
      virtual ~EmptySolver();
      virtual void solve();
      virtual int get_matrix_size();

      /// Matrix to solve.
      EmptyMatrix<Scalar> *m;
      /// Right hand side vector.
      EmptyVector<Scalar> *rhs;
    };
  }
}
#endif
