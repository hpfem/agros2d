// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "spline.h"

template<typename T>
T **new_matrix(unsigned int m, unsigned int n = 0)
{
    if (!n) n = m;
    T **vec = (T **)malloc(sizeof(T *)* m + sizeof(T)* m * n);
    memset(vec, 0, sizeof(T *)* m + sizeof(T)* m * n);
    T *row = (T *)(vec + m);
    for (unsigned int i = 0; i < m; i++, row += n) vec[i] = row;
    return vec;
}

/// Given a matrix a[n][n], this routine replaces it by the LU decomposition of a rowwise
/// permutation of itself. a and n are input. a is output, arranged as in equation (2.3.14) above;
/// indx[n] is an output vector that records the row permutation effected by the partial
/// pivoting; d is output as +-1 depending on whether the number of row interchanges was even
/// or odd, respectively. This routine is used in combination with lubksb to solve linear equations
/// or invert a matrix.
template<typename T>
void ludcmp(T **a, int n, int *indx, double *d)
{
    int i, imax = 0, j, k;
    T big, dum, sum, temp;
    T *vv = new T[n];

    *d = 1.0;
    for (i = 0; i < n; i++)
    {
        big = 0.0;
        for (j = 0; j < n; j++)
        {
            temp = a[i][j];
            if (std::abs(temp) > std::abs(big))
                big = temp;
        }
        if (big == 0.0)
        {
            delete [] vv;
            throw AgrosException("Singular matrix in routine LUDCMP!");
        }
        vv[i] = 1.0 / big;
    }
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < j; i++)
        {
            sum = a[i][j];
            for (k = 0; k < i; k++) sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
        }
        big = 0.0;
        for (i = j; i < n; i++)
        {
            sum = a[i][j];
            for (k = 0; k < j; k++) sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
            dum = vv[i] * std::abs(sum);
            if (std::abs(dum) >= std::abs(big))
            {
                big = dum;
                imax = i;
            }
        }
        if (j != imax)
        {
            for (k = 0; k < n; k++)
            {
                dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if (a[j][j] == 0.0) a[j][j] = 1.0e-20;
        if (j != n - 1)
        {
            dum = 1.0 / (a[j][j]);
            for (i = j + 1; i < n; i++) a[i][j] *= dum;
        }
    }
    delete [] vv;
}

/// Solves the set of n linear equations AX = B. Here a[n][n] is input, not as the matrix
/// A but rather as its LU decomposition, determined by the routine ludcmp. indx[n] is input
/// as the permutation vector returned by ludcmp. b[n] is input as the right-hand side vector
/// B, and returns with the solution vector X. a, n, and indx are not modified by this routine
/// and can be left in place for successive calls with different right-hand sides b. This routine takes
/// into account the possibility that b will begin with many zero elements, so it is efficient for use
/// in matrix inversion.
template<typename T, typename S>
void lubksb(T **a, int n, int *indx, S *b)
{
    int i, ip, j;
    S sum;

    for (i = 0; i < n; i++)
    {
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        for (j = 0; j < i; j++)
            sum = sum - a[i][j] * b[j];
        b[i] = sum;
    }
    for (i = n - 1; i >= 0; i--)
    {
        sum = b[i];
        for (j = i + 1; j < n; j++)
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
    }
}

CubicSpline::CubicSpline(std::vector<double> points, std::vector<double> values,
                         double bc_left, double bc_right,
                         bool first_der_left, bool first_der_right,
                         bool extrapolate_der_left, bool extrapolate_der_right) :
    points(points), values(values),
    bc_left(bc_left), bc_right(bc_right), first_der_left(first_der_left),
    first_der_right(first_der_right), extrapolate_der_left(extrapolate_der_left),
    extrapolate_der_right(extrapolate_der_right), is_const(false)
{
    for (int i = 1; i < points.size(); i++)
        if (points[i] <= points[i - 1])
            throw AgrosException("Points must be in ascending order when constructing a spline.");
}

CubicSpline::CubicSpline(double const_value) : const_value(const_value), is_const(true)
{        
}

CubicSpline::~CubicSpline()
{
    free();
}

void CubicSpline::free()
{
    coeffs.clear();
    points.clear();
    values.clear();
}

double CubicSpline::value(double x) const
{
    // For simple constant case.
    if (this->is_const)
        return const_value;
    // For general case.
    int m = -1;
    if (!this->find_interval(x, m))
    {
        // Point lies on the left of interval of definition.
        if (x <= point_left)
        {
            // Spline should be extrapolated by constant function
            // matching the value at the end.
            if (extrapolate_der_left == false)
                return value_left;
            // Spline should be extrapolated as a linear function
            // matching the derivative at the end.
            else return extrapolate_value(point_left, value_left, derivative_left, x);
        }
        // Point lies on the right of interval of definition.
        else
        {
            // Spline should be extrapolated by constant function
            // matching the value at the end.
            if (extrapolate_der_right == false)
                return value_right;
            // Spline should be extrapolated as a linear function
            // matching the derivative at the end.
            else return extrapolate_value(point_right, value_right, derivative_right, x);
        }
    }

    return get_value_from_interval(x, m);
};

double CubicSpline::derivative(double x) const
{
    // For simple constant case.
    if (this->is_const)
        return 0.0;

    // For general case.
    int m = -1;
    if (!this->find_interval(x, m))
    {
        // Point lies on the left of interval of definition.
        if (x <= point_left)
        {
            // Spline should be extrapolated by constant function
            // matching the value at the end.
            if (extrapolate_der_left == false) return 0;
            // Spline should be extrapolated as a linear function
            // matching the derivative at the end.
            else return derivative_left;
        }
        // Point lies on the right of interval of definition.
        else
        {
            // Spline should be extrapolated by constant function
            // matching the value at the end.
            if (extrapolate_der_right == false) return 0;
            // Spline should be extrapolated as a linear function
            // matching the derivative at the end.
            else return derivative_right;
        }
    }

    return get_derivative_from_interval(x, m);
};

double CubicSpline::extrapolate_value(double point_end, double value_end,
                                      double derivative_end, double x_in) const
{
    return value_end + derivative_end * (x_in - point_end);
}

double CubicSpline::get_value_from_interval(double x_in, int m) const
{
    double x2 = x_in * x_in;
    double x3 = x2 * x_in;
    return   this->coeffs[m].a + this->coeffs[m].b * x_in + this->coeffs[m].c * x2
            + this->coeffs[m].d * x3;
}

double CubicSpline::get_derivative_from_interval(double x_in, int m) const
{
    double x2 = x_in * x_in;
    return this->coeffs[m].b + 2 * this->coeffs[m].c * x_in
            + 3 * this->coeffs[m].d * x2;
}

bool CubicSpline::find_interval(double x_in, int &m) const
{
    int i_left = 0;
    int i_right = points.size() - 1;
    if (i_right < 0)
        return false;

    if (x_in < points[i_left]) return false;
    if (x_in > points[i_right]) return false;

    while (i_left + 1 < i_right)
    {
        int i_mid = (i_left + i_right) / 2;
        if (points[i_mid] < x_in) i_left = i_mid;
        else i_right = i_mid;
    }

    m = i_left;
    return true;
}

void CubicSpline::calculate_coeffs()
{
    int nelem = points.size() - 1;

    // Basic sanity checks.
    if (points.empty() || values.empty())
    {
        qDebug() << ("Empty points or values vector in CubicSpline, cancelling coefficients calculation.");
        return;
    }
    if (points.size() < 2 || values.size() < 2)
    {
        qDebug() << ("At least two points and values required in CubicSpline, cancelling coefficients calculation.");
        return;
    }
    if (points.size() != values.size())
    {
        qDebug() << ("Mismatched number of points and values in CubicSpline, cancelling coefficients calculation.");
        return;
    }

    // Check for improperly ordered or duplicated points.
    double eps = 1e-12;
    for (int i = 0; i < nelem; i++)
    {
        if (points[i + 1] < points[i] + eps)
        {
            qDebug() << ("Duplicated or improperly ordered points in CubicSpline detected, cancelling coefficients calculation.");
            return;
        }
    }

    /* START COMPUTATION */

    // Allocate matrix and rhs.
    const int n = 4 * nelem;
    double** matrix = new_matrix<double>(n, n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = 0;
        }
    }
    double *rhs = new double[n];
    for (int j = 0; j < n; j++)
    {
        rhs[j] = 0;
    }

    // Fill the rhs vector.
    for (int i = 0; i < nelem; i++)
    {
        rhs[2 * i] = values[i];
        rhs[2 * i + 1] = values[i + 1];
    }

    // Fill the matrix. Step 1 - match values at interval endpoints.
    // This will generate the first 2*nelem rows.
    for (int i = 0; i < nelem; i++) { // Loop over elements.
        double xx = points[i];
        double xx2 = xx*xx;
        double xx3 = xx2 * xx;
        matrix[2 * i][4 * i + 0] = 1;
        matrix[2 * i][4 * i + 1] = xx;
        matrix[2 * i][4 * i + 2] = xx2;
        matrix[2 * i][4 * i + 3] = xx3;
        xx = points[i + 1];
        xx2 = xx*xx;
        xx3 = xx2 * xx;
        matrix[2 * i + 1][4 * i + 0] = 1.0;
        matrix[2 * i + 1][4 * i + 1] = xx;
        matrix[2 * i + 1][4 * i + 2] = xx2;
        matrix[2 * i + 1][4 * i + 3] = xx3;
    }

    // Step 2: match first derivatives at all interior points.
    // This will generate additional n_elem-1 rows in the matrix.
    int offset = 2 * nelem;
    for (int i = 1; i < nelem; i++) { // Loop over internal points.
        double xx = points[i];
        double xx2 = xx*xx;
        matrix[offset + i - 1][4 * (i - 1) + 1] = 1;
        matrix[offset + i - 1][4 * (i - 1) + 2] = 2 * xx;
        matrix[offset + i - 1][4 * (i - 1) + 3] = 3 * xx2;
        matrix[offset + i - 1][4 * (i - 1) + 5] = -1;
        matrix[offset + i - 1][4 * (i - 1) + 6] = -2 * xx;
        matrix[offset + i - 1][4 * (i - 1) + 7] = -3 * xx2;
    }

    // Step 3: match second derivatives at all interior points.
    // This will generate additional n_elem-1 rows in the matrix.
    offset = 2 * nelem + nelem - 1;
    for (int i = 1; i < nelem; i++) { // Loop over internal points.
        double xx = points[i];
        matrix[offset + i - 1][4 * (i - 1) + 2] = 2;
        matrix[offset + i - 1][4 * (i - 1) + 3] = 6 * xx;
        matrix[offset + i - 1][4 * (i - 1) + 6] = -2;
        matrix[offset + i - 1][4 * (i - 1) + 7] = -6 * xx;
    }

    // Step 4: Additional two conditions are needed to define
    // a cubic spline. This will generate the last two rows in
    // the matrix. Setting the second derivative (curvature) at both
    // endpoints equal to zero will result into "natural cubic spline",
    // but you can also prescribe non-zero values, or decide to
    // prescribe first derivative (slope).
    // Choose just one of the following two variables to be True,
    // and state the corresponding value for the derivative.
    offset = 2 * nelem + 2 * (nelem - 1);
    double xx = points[0]; // Left end-point.
    if (first_der_left == false)
    {
        matrix[offset + 0][2] = 2;
        matrix[offset + 0][3] = 6 * xx;
        rhs[n - 2] = bc_left; // Value of the second derivative.
    }
    else
    {
        matrix[offset + 0][1] = 1;
        matrix[offset + 0][2] = 2 * xx;
        matrix[offset + 0][3] = 3 * xx*xx;
        rhs[n - 2] = bc_left; // Value of the first derivative.
    }
    xx = points[nelem]; // Right end-point.
    if (first_der_right == false)
    {
        matrix[offset + 1][n - 2] = 2;
        matrix[offset + 1][n - 1] = 6 * xx;
        rhs[n - 1] = bc_right; // Value of the second derivative.
    }
    else
    {
        matrix[offset + 1][n - 3] = 1;
        matrix[offset + 1][n - 2] = 2 * xx;
        matrix[offset + 1][n - 1] = 3 * xx*xx;
        rhs[n - 1] = bc_right; // Value of the first derivative.
    }

    // Solve the matrix problem.
    double d;
    int* perm = new int[n];
    ludcmp(matrix, n, perm, &d);
    lubksb<double>(matrix, n, perm, rhs);
    delete [] perm;

    // Copy the solution into the coeffs array.
    coeffs.clear();
    for (int i = 0; i < nelem; i++)
    {
        SplineCoeff coeff;
        coeff.a = rhs[4 * i + 0];
        coeff.b = rhs[4 * i + 1];
        coeff.c = rhs[4 * i + 2];
        coeff.d = rhs[4 * i + 3];
        coeffs.push_back(coeff);
    }

    // Define end point values and derivatives so that
    // the points[] and values[] arrays are no longer
    // needed.
    point_left = points[0];
    value_left = values[0];
    derivative_left = get_derivative_from_interval(point_left, 0);
    point_right = points[points.size() - 1];
    value_right = values[values.size() - 1];
    derivative_right = get_derivative_from_interval(point_right, points.size() - 2);

    // Free the matrix and rhs vector.
    // delete matrix;
    delete [] rhs;

    return;
}
