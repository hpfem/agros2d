// This file is part of Hermes2D.
//
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
// Copyright 2005-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Pavel Solin <solin@unr.edu>
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

// $Id: matrix.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_MATRIX_H
#define __HERMES2D_MATRIX_H

#include "common.h"


/// Creates a new (full) matrix with m rows and n columns with entries of the type T.
/// The entries can be accessed by matrix[i][j]. To delete the matrix, just
/// do "delete matrix".
template<typename T>
T** new_matrix(int m, int n = 0)
{
  if (!n) n = m;
  T** vec = (T**) new char[sizeof(T*)*m + sizeof(T)*m*n];
  if (vec == NULL) error("Out of memory.");
  T* row = (T*) (vec + m);
  for (int i = 0; i < m; i++, row += n)
    vec[i] = row;
  return vec;
}


/// Transposes an m by n matrix. If m != n, the array matrix in fact has to be
/// a square matrix of the size max(m, n) in order for the transpose to fit inside it.
template<typename T>
void transpose(T** matrix, int m, int n)
{
  int min = std::min(m, n);
  for (int i = 0; i < min; i++)
    for (int j = i+1; j < min; j++)
      std::swap(matrix[i][j], matrix[j][i]);
    
  if (m < n)
    for (int i = 0; i < m; i++)
      for (int j = m; j < n; j++)
        matrix[j][i] = matrix[i][j];
  else if (n < m)
    for (int i = n; i < m; i++)
      for (int j = 0; j < n; j++)
        matrix[j][i] = matrix[i][j];
}


/// Changes the sign of a matrix
template<typename T>
void chsgn(T** matrix, int m, int n)
{
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
      matrix[i][j] = -matrix[i][j];
}


/// Given a matrix a[n][n], this routine replaces it by the LU decomposition of a rowwise
/// permutation of itself. a and n are input. a is output, arranged as in equation (2.3.14) above;
/// indx[n] is an output vector that records the row permutation effected by the partial
/// pivoting; d is output as +-1 depending on whether the number of row interchanges was even
/// or odd, respectively. This routine is used in combination with lubksb to solve linear equations
/// or invert a matrix.
void ludcmp(double** a, int n, int* indx, double* d);

/// Solves the set of n linear equations AX = B. Here a[n][n] is input, not as the matrix
/// A but rather as its LU decomposition, determined by the routine ludcmp. indx[n] is input
/// as the permutation vector returned by ludcmp. b[n] is input as the right-hand side vector
/// B, and returns with the solution vector X. a, n, and indx are not modified by this routine
/// and can be left in place for successive calls with different right-hand sides b. This routine takes
/// into account the possibility that b will begin with many zero elements, so it is efficient for use
/// in matrix inversion.
template<typename T>
void lubksb(double** a, int n, int* indx, T* b)
{
  int i, ip, j;
  T sum;

  for (i = 0; i < n; i++)
  {
    ip = indx[i];
    sum = b[ip];
    b[ip] = b[i];
    for (j = 0; j < i; j++) sum -= a[i][j]*b[j];
    b[i] = sum;
  }
  for (i = n-1; i >= 0; i--)
  {
    sum = b[i];
    for (j = i+1; j < n; j++) sum -= a[i][j]*b[j];
    b[i] = sum / a[i][i];
  }
}




/// Given a positive-definite symmetric matrix a[n][n], this routine constructs its Cholesky
/// decomposition, A = L*L^T . On input, only the upper triangle of a need be given; it is not
/// modified. The Cholesky factor L is returned in the lower triangle of a, except for its diagonal
/// elements which are returned in p[n].
void choldc(double **a, int n, double p[]);

/// Solves the set of n linear equations A*x = b, where a is a positive-definite symmetric matrix.
/// a[n][n] and p[n] are input as the output of the routine choldc. Only the lower
/// subdiagonal portion of a is accessed. b[n] is input as the right-hand side vector. The
/// solution vector is returned in x[n]. a, n, and p are not modified and can be left in place
/// for successive calls with different right-hand sides b. b is not modified unless you identify b and
/// x in the calling sequence, which is allowed. The right-hand side b can be complex, in which case
/// the solution x is also complex.
template<typename T>
void cholsl(double **a, int n, double p[], T b[], T x[])
{
  int i, k;
  T sum;

  for (i = 0; i < n; i++)
  {
    sum = b[i];
    k = i;
    while (--k >= 0)
      sum -= a[i][k] * x[k];
    x[i] = sum / p[i];
  }

  for (i = n-1; i >= 0; i--)
  {
    sum = x[i];
    k = i;
    while (++k < n)
      sum -= a[k][i] * x[k];
    x[i] = sum / p[i];
  }
}


#endif
