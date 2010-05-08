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

#ifndef __H2D_SOLVER_UMFPACK_H
#define __H2D_SOLVER_UMFPACK_H


#include <umfpack.h>
// To use the UmfpackSolver class, you need to install the UMFPACK solver
// on your machine. For instructions, see http://www.cise.ufl.edu/research/sparse/umfpack/
//
// If UMFPACK is installed on your machine, but the compiler is unable to find
// umfpack.h, try using the -I compiler directive with a path to the UMFPACK includes.
//
// If the project compiles, but you get linker errors, make sure you are using
// -lumfpack and -lamd on the command line. You may want to point out the library
// paths to the linker using the -L directive. If you get weird Fortran-related
// error messages, try including the Fortran compatibility layer: -lgfortran
// (-lg2c in older versions of gcc)
//
// If you don't intend to use UmfpackSolver, don't include this file (umpack_solver.h)


//// helper macros /////////////////////////////////////////////////////////////////////////////////

#ifdef H2D_COMPLEX
  // macros for calling complex UMFPACK in packed-complex mode
  #define umfpack_symbolic(m, n, Ap, Ai, Ax, S, C, I) \
          umfpack_zi_symbolic(m, n, Ap, Ai, (double*) (Ax), NULL, S, C, I)
  #define umfpack_numeric(Ap, Ai, Ax, S, N, C, I) \
          umfpack_zi_numeric(Ap, Ai, (double*) (Ax), NULL, S, N, C, I)
  #define umfpack_solve(sys, Ap, Ai, Ax, X, B, N, C, I) \
          umfpack_zi_solve(sys, Ap, Ai, (double*) (Ax), NULL, (double*) (X), NULL, (double*) (B), NULL, N, C, I)
  #define umfpack_free_symbolic umfpack_zi_free_symbolic
  #define umfpack_free_numeric  umfpack_zi_free_numeric
  #define umfpack_defaults      umfpack_zi_defaults
#else
  // real case: no changes in calling UMFPACK
  #define umfpack_symbolic(m, n, Ap, Ai, Ax, S, C, I)   umfpack_di_symbolic(m, n, Ap, Ai, Ax, S, C, I)
  #define umfpack_numeric(Ap, Ai, Ax, S, N, C, I)       umfpack_di_numeric(Ap, Ai, Ax, S, N, C, I)
  #define umfpack_solve(sys, Ap, Ai, Ax, X, B, N, C, I) umfpack_di_solve(sys, Ap, Ai, Ax, X, B, N, C, I)
  #define umfpack_free_symbolic                         umfpack_di_free_symbolic
  #define umfpack_free_numeric                          umfpack_di_free_numeric
  #define umfpack_defaults                              umfpack_di_defaults
#endif


//// UmfpackSolver /////////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "solver.h"


/// \brief UMFPACK solver wrapper class
///
class UmfpackSolver : public Solver
{
public:

  UmfpackSolver() { umfpack_defaults(control_array); }

  /// Adjusts UMFPACK control parameters. See UMFPACK documentation.
  void control(int idx, double value) { control_array[idx] = value; }

  /// Retrieves detailed UMFPACK statistics. See UMFPACK documentation.
  double get_info(int idx) const { return info_array[idx]; }


protected:

  virtual bool is_row_oriented()  { return false; }
  virtual bool handles_symmetry() { return false; }

  double control_array[UMFPACK_CONTROL];
  double info_array[UMFPACK_INFO];


  struct Data
  {
    void* symbolic;
    void* numeric;
  };

  virtual void* new_context(bool sym)
  {
    Data* data = new Data;
    data->symbolic = NULL;
    data->numeric = NULL;
    return data;
  }

  virtual void free_context(void* ctx)
  {
    delete (Data*) ctx;
  }


  virtual bool analyze(void* ctx, int n, int* Ap, int* Ai, scalar* Ax, bool sym)
  {
    Data* data = (Data*) ctx;
    if (data->symbolic != NULL) umfpack_free_symbolic(&data->symbolic);
    trace("UMFPACK: analyzing matrix");
    int status = umfpack_symbolic(n, n, Ap, Ai, NULL, &data->symbolic,
                                  control_array, info_array);
    print_status(status);
    return status == UMFPACK_OK;
  }


  virtual bool factorize(void* ctx, int n, int* Ap, int* Ai, scalar* Ax, bool sym)
  {
    Data* data = (Data*) ctx;
    if (data->numeric != NULL) umfpack_free_numeric(&data->numeric);
    trace("UMFPACK: factorizing matrix");
    if (!n) return true;
    int status = umfpack_numeric(Ap, Ai, Ax, data->symbolic, &data->numeric,
                                 control_array, info_array);
    print_status(status);
    return status == UMFPACK_OK;
  }


  virtual bool solve(void* ctx, int n, int* Ap, int* Ai, scalar* Ax, bool sym,
                     scalar* RHS, scalar* vec)
  {
    Data* data = (Data*) ctx;
    trace("UMFPACK: solving system");
    if (!n) return true;
    int status = umfpack_solve(UMFPACK_A, Ap, Ai, Ax, vec, RHS, data->numeric,
                               control_array, info_array);
    print_status(status);
    return status == UMFPACK_OK;
  }


  virtual void free_data(void* ctx)
  {
    Data* data = (Data*) ctx;
    if (data->symbolic != NULL) { umfpack_free_symbolic(&data->symbolic);  data->symbolic = NULL; }
    if (data->numeric != NULL)  { umfpack_free_numeric (&data->numeric);   data->numeric  = NULL; }
  }


  virtual void print_status(int status)
  {
    switch (status)
    {
      case UMFPACK_OK:
        break;
      case UMFPACK_WARNING_singular_matrix:
        warn("UMFPACK: singular stiffness matrix!");
        break;

      case UMFPACK_ERROR_out_of_memory:           error("UMFPACK: out of memory!");
      case UMFPACK_ERROR_argument_missing:        error("UMFPACK: argument missing");
      case UMFPACK_ERROR_invalid_Symbolic_object: error("UMFPACK: invalid Symbolic object");
      case UMFPACK_ERROR_invalid_Numeric_object:  error("UMFPACK: invalid Numeric object");
      case UMFPACK_ERROR_different_pattern:       error("UMFPACK: different pattern");
      case UMFPACK_ERROR_invalid_system:          error("UMFPACK: invalid system");
      case UMFPACK_ERROR_n_nonpositive:           error("UMFPACK: n nonpositive");
      case UMFPACK_ERROR_invalid_matrix:          error("UMFPACK: invalid matrix");
      case UMFPACK_ERROR_internal_error:          error("UMFPACK: internal error");
      default:                                    error("UMFPACK: unknown error");
    }
  }

};



#endif
