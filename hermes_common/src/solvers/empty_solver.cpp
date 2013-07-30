// This file is part of HermesCommon
//
// Copyright (c) 2009 hp-FEM group at the University of Nevada, Reno (UNR).
// Email: hpfem-group@unr.edu, home page: http://hpfem.org/.
//
// Hermes is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// Hermes is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
/*! \file empty_solver.cpp
\brief EMPTY solver interface.
*/
#include "config.h"
#include "empty_solver.h"

namespace Hermes
{
  namespace Algebra
  {
    template<typename Scalar>
    EmptyVector<Scalar>::EmptyVector() : Vector<Scalar>()
    {
      v = NULL;
      this->size = 0;
    }

    template<typename Scalar>
    EmptyVector<Scalar>::EmptyVector(unsigned int size) : Vector<Scalar>(size), v(NULL)
    {
      this->alloc(size);
    }

    template<typename Scalar>
    EmptyVector<Scalar>::~EmptyVector()
    {
      free();
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::alloc(unsigned int n)
    {
      free();
      this->size = n;
      v = new Scalar[n];
      this->zero();
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::zero()
    {
      memset(v, 0, this->size * sizeof(Scalar));
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::change_sign()
    {
      for (unsigned int i = 0; i < this->size; i++) v[i] *= -1.;
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::free()
    {
      delete [] v;
      v = NULL;
      this->size = 0;
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::set(unsigned int idx, Scalar y)
    {
      v[idx] = y;
    }

    template<>
    void EmptyVector<double>::add(unsigned int idx, double y)
    {
#pragma omp atomic
      v[idx] += y;
    }

    template<>
    void EmptyVector<std::complex<double> >::add(unsigned int idx, std::complex<double> y)
    {
#pragma omp critical(EmptyVector_add)
      v[idx] += y;
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::add(unsigned int n, unsigned int *idx, Scalar *y)
    {
      for (unsigned int i = 0; i < n; i++)
        v[idx[i]] += y[i];
    }

    template<typename Scalar>
    Scalar EmptyVector<Scalar>::get(unsigned int idx) const
    {
      return v[idx];
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::extract(Scalar *v) const
    {
      memcpy(v, this->v, this->size * sizeof(Scalar));
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::set_vector(Vector<Scalar>* vec)
    {
      assert(this->size == vec->length());
      for (unsigned int i = 0; i < this->size; i++) this->v[i] = vec->get(i);
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::set_vector(Scalar* vec)
    {
      memcpy(this->v, vec, this->size * sizeof(Scalar));
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::add_vector(Vector<Scalar>* vec)
    {
      assert(this->length() == vec->length());
      for (unsigned int i = 0; i < this->length(); i++) this->v[i] += vec->get(i);
    }

    template<typename Scalar>
    void EmptyVector<Scalar>::add_vector(Scalar* vec)
    {
      for (unsigned int i = 0; i < this->length(); i++) this->v[i] += vec[i];
    }

    template<>
    bool EmptyVector<double>::dump(FILE *file, const char *var_name, EMatrixDumpFormat fmt, char* number_format)
    {
      switch (fmt)
      {
      case DF_MATLAB_SPARSE:
        fprintf(file, "%% Size: %dx1\n%s =[\n", this->size, var_name);
        for (unsigned int i = 0; i < this->size; i++)
        {
          Hermes::Helpers::fprint_num(file, v[i], number_format);
          fprintf(file, "\n");
        }
        fprintf(file, " ];\n");
        return true;

      case DF_HERMES_BIN:
        {
          hermes_fwrite("HERMESR\001", 1, 8, file);
          int ssize = sizeof(double);
          hermes_fwrite(&ssize, sizeof(int), 1, file);
          hermes_fwrite(&this->size, sizeof(int), 1, file);
          hermes_fwrite(v, sizeof(double), this->size, file);
          return true;
        }

      case DF_HERMES_MATLAB_BIN:
        {
          hermes_fwrite(&this->size, sizeof(int), 1, file);
          hermes_fwrite(v, sizeof(double), this->size, file);
          return true;
        }

      case DF_PLAIN_ASCII:
        {
          fprintf(file, "\n");
          for (unsigned int i = 0; i < size; i++)
          {
            Hermes::Helpers::fprint_num(file, v[i], number_format);
            fprintf(file, "\n");
          }

          return true;
        }

      default:
        return false;
      }
    }

    template<>
    bool EmptyVector<std::complex<double> >::dump(FILE *file, const char *var_name, EMatrixDumpFormat fmt, char* number_format)
    {
      switch (fmt)
      {
      case DF_MATLAB_SPARSE:
        fprintf(file, "%% Size: %dx1\n%s =[\n", this->size, var_name);
        for (unsigned int i = 0; i < this->size; i++)
        {
          Hermes::Helpers::fprint_num(file, v[i], number_format);
          fprintf(file, "\n");
        }
        fprintf(file, " ];\n");
        return true;

      case DF_HERMES_BIN:
        {
          hermes_fwrite("HERMESR\001", 1, 8, file);
          int ssize = sizeof(std::complex<double>);
          hermes_fwrite(&ssize, sizeof(int), 1, file);
          hermes_fwrite(&this->size, sizeof(int), 1, file);
          hermes_fwrite(v, sizeof(std::complex<double>), this->size, file);
          return true;
        }

      case DF_PLAIN_ASCII:
        {
          fprintf(file, "\n");
          for (unsigned int i = 0; i < size; i++)
          {
            fprintf(file, "%E %E\n", v[i].real(), v[i].imag());
          }

          return true;
        }

      default:
        return false;
      }
    }

    template class HERMES_API EmptyMatrix<double>;
    template class HERMES_API EmptyMatrix<std::complex<double> >;
    template class HERMES_API EmptyVector<double>;
    template class HERMES_API EmptyVector<std::complex<double> >;
  }

  namespace Solvers
  {
    template<typename Scalar>
    EmptySolver<Scalar>::EmptySolver(EmptyMatrix<Scalar> *m, EmptyVector<Scalar> *rhs)
        : DirectSolver<Scalar>(HERMES_CREATE_STRUCTURE_FROM_SCRATCH), m(m), rhs(rhs)
    {
    }

    template<typename Scalar>
    EmptySolver<Scalar>::~EmptySolver()
    {      
    }

    template<typename Scalar>
    int EmptySolver<Scalar>::get_matrix_size()
    {
      return m->get_size();
    }   

    template<>
    void EmptySolver<double>::solve()
    {
      assert(m != NULL);
      assert(rhs != NULL);
      assert(m->get_size() == rhs->length());

      this->tick();

      if(sln != NULL)
        delete [] sln;

      sln = new double[m->get_size()];
      memset(sln, 0, m->get_size() * sizeof(double));

      this->tick();
      time = this->accumulated();
    }

    template<>
    void EmptySolver<std::complex<double> >::solve()
    {
      assert(m != NULL);
      assert(rhs != NULL);
      assert(m->get_size() == rhs->length());

      this->tick();

      if(sln)
        delete [] sln;
      sln = new std::complex<double>[m->get_size()];
      memset(sln, 0, m->get_size() * sizeof(std::complex<double>));

      this->tick();
      time = this->accumulated();
    }

    template class HERMES_API EmptySolver<double>;
    template class HERMES_API EmptySolver<std::complex<double> >;
  }
}

