/*************************************************************************
Copyright (c) Sergey Bochkanov (ALGLIB project).

>>> SOURCE LICENSE >>>
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation (www.fsf.org); either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is available at
http://www.fsf.org/licensing/licenses
>>> END OF LICENSE >>>
*************************************************************************/
#include "stdafx.h"
#include "interpolation.h"

// disable some irrelevant warnings
#if (AE_COMPILER==AE_MSVC)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4702)
#pragma warning(disable:4996)
#endif
using namespace std;

/////////////////////////////////////////////////////////////////////////
//
// THIS SECTION CONTAINS IMPLEMENTATION OF C++ INTERFACE
//
/////////////////////////////////////////////////////////////////////////
namespace alglib
{


/*************************************************************************
IDW interpolant.
*************************************************************************/
_idwinterpolant_owner::_idwinterpolant_owner()
{
    p_struct = (alglib_impl::idwinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::idwinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_idwinterpolant_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_idwinterpolant_owner::_idwinterpolant_owner(const _idwinterpolant_owner &rhs)
{
    p_struct = (alglib_impl::idwinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::idwinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_idwinterpolant_init_copy(p_struct, const_cast<alglib_impl::idwinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_idwinterpolant_owner& _idwinterpolant_owner::operator=(const _idwinterpolant_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_idwinterpolant_clear(p_struct);
    if( !alglib_impl::_idwinterpolant_init_copy(p_struct, const_cast<alglib_impl::idwinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_idwinterpolant_owner::~_idwinterpolant_owner()
{
    alglib_impl::_idwinterpolant_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::idwinterpolant* _idwinterpolant_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::idwinterpolant* _idwinterpolant_owner::c_ptr() const
{
    return const_cast<alglib_impl::idwinterpolant*>(p_struct);
}
idwinterpolant::idwinterpolant() : _idwinterpolant_owner() 
{
}

idwinterpolant::idwinterpolant(const idwinterpolant &rhs):_idwinterpolant_owner(rhs) 
{
}

idwinterpolant& idwinterpolant::operator=(const idwinterpolant &rhs)
{
    if( this==&rhs )
        return *this;
    _idwinterpolant_owner::operator=(rhs);
    return *this;
}

idwinterpolant::~idwinterpolant()
{
}

/*************************************************************************
IDW interpolation

INPUT PARAMETERS:
    Z   -   IDW interpolant built with one of model building
            subroutines.
    X   -   array[0..NX-1], interpolation point

Result:
    IDW interpolant Z(X)

  -- ALGLIB --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
double idwcalc(const idwinterpolant &z, const real_1d_array &x)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::idwcalc(const_cast<alglib_impl::idwinterpolant*>(z.c_ptr()), const_cast<alglib_impl::ae_vector*>(x.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
IDW interpolant using modified Shepard method for uniform point
distributions.

INPUT PARAMETERS:
    XY  -   X and Y values, array[0..N-1,0..NX].
            First NX columns contain X-values, last column contain
            Y-values.
    N   -   number of nodes, N>0.
    NX  -   space dimension, NX>=1.
    D   -   nodal function type, either:
            * 0     constant  model.  Just  for  demonstration only, worst
                    model ever.
            * 1     linear model, least squares fitting. Simpe  model  for
                    datasets too small for quadratic models
            * 2     quadratic  model,  least  squares  fitting. Best model
                    available (if your dataset is large enough).
            * -1    "fast"  linear  model,  use  with  caution!!!   It  is
                    significantly  faster than linear/quadratic and better
                    than constant model. But it is less robust (especially
                    in the presence of noise).
    NQ  -   number of points used to calculate  nodal  functions  (ignored
            for constant models). NQ should be LARGER than:
            * max(1.5*(1+NX),2^NX+1) for linear model,
            * max(3/4*(NX+2)*(NX+1),2^NX+1) for quadratic model.
            Values less than this threshold will be silently increased.
    NW  -   number of points used to calculate weights and to interpolate.
            Required: >=2^NX+1, values less than this  threshold  will  be
            silently increased.
            Recommended value: about 2*NQ

OUTPUT PARAMETERS:
    Z   -   IDW interpolant.

NOTES:
  * best results are obtained with quadratic models, worst - with constant
    models
  * when N is large, NQ and NW must be significantly smaller than  N  both
    to obtain optimal performance and to obtain optimal accuracy. In 2  or
    3-dimensional tasks NQ=15 and NW=25 are good values to start with.
  * NQ  and  NW  may  be  greater  than  N.  In  such  cases  they will be
    automatically decreased.
  * this subroutine is always succeeds (as long as correct parameters  are
    passed).
  * see  'Multivariate  Interpolation  of Large Sets of Scattered Data' by
    Robert J. Renka for more information on this algorithm.
  * this subroutine assumes that point distribution is uniform at the small
    scales.  If  it  isn't  -  for  example,  points are concentrated along
    "lines", but "lines" distribution is uniform at the larger scale - then
    you should use IDWBuildModifiedShepardR()


  -- ALGLIB PROJECT --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
void idwbuildmodifiedshepard(const real_2d_array &xy, const ae_int_t n, const ae_int_t nx, const ae_int_t d, const ae_int_t nq, const ae_int_t nw, idwinterpolant &z)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::idwbuildmodifiedshepard(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, nx, d, nq, nw, const_cast<alglib_impl::idwinterpolant*>(z.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
IDW interpolant using modified Shepard method for non-uniform datasets.

This type of model uses  constant  nodal  functions and interpolates using
all nodes which are closer than user-specified radius R. It  may  be  used
when points distribution is non-uniform at the small scale, but it  is  at
the distances as large as R.

INPUT PARAMETERS:
    XY  -   X and Y values, array[0..N-1,0..NX].
            First NX columns contain X-values, last column contain
            Y-values.
    N   -   number of nodes, N>0.
    NX  -   space dimension, NX>=1.
    R   -   radius, R>0

OUTPUT PARAMETERS:
    Z   -   IDW interpolant.

NOTES:
* if there is less than IDWKMin points within  R-ball,  algorithm  selects
  IDWKMin closest ones, so that continuity properties of  interpolant  are
  preserved even far from points.

  -- ALGLIB PROJECT --
     Copyright 11.04.2010 by Bochkanov Sergey
*************************************************************************/
void idwbuildmodifiedshepardr(const real_2d_array &xy, const ae_int_t n, const ae_int_t nx, const double r, idwinterpolant &z)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::idwbuildmodifiedshepardr(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, nx, r, const_cast<alglib_impl::idwinterpolant*>(z.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
IDW model for noisy data.

This subroutine may be used to handle noisy data, i.e. data with noise  in
OUTPUT values.  It differs from IDWBuildModifiedShepard() in the following
aspects:
* nodal functions are not constrained to pass through  nodes:  Qi(xi)<>yi,
  i.e. we have fitting  instead  of  interpolation.
* weights which are used during least  squares fitting stage are all equal
  to 1.0 (independently of distance)
* "fast"-linear or constant nodal functions are not supported (either  not
  robust enough or too rigid)

This problem require far more complex tuning than interpolation  problems.
Below you can find some recommendations regarding this problem:
* focus on tuning NQ; it controls noise reduction. As for NW, you can just
  make it equal to 2*NQ.
* you can use cross-validation to determine optimal NQ.
* optimal NQ is a result of complex tradeoff  between  noise  level  (more
  noise = larger NQ required) and underlying  function  complexity  (given
  fixed N, larger NQ means smoothing of compex features in the data).  For
  example, NQ=N will reduce noise to the minimum level possible,  but  you
  will end up with just constant/linear/quadratic (depending on  D)  least
  squares model for the whole dataset.

INPUT PARAMETERS:
    XY  -   X and Y values, array[0..N-1,0..NX].
            First NX columns contain X-values, last column contain
            Y-values.
    N   -   number of nodes, N>0.
    NX  -   space dimension, NX>=1.
    D   -   nodal function degree, either:
            * 1     linear model, least squares fitting. Simpe  model  for
                    datasets too small for quadratic models (or  for  very
                    noisy problems).
            * 2     quadratic  model,  least  squares  fitting. Best model
                    available (if your dataset is large enough).
    NQ  -   number of points used to calculate nodal functions.  NQ should
            be  significantly   larger   than  1.5  times  the  number  of
            coefficients in a nodal function to overcome effects of noise:
            * larger than 1.5*(1+NX) for linear model,
            * larger than 3/4*(NX+2)*(NX+1) for quadratic model.
            Values less than this threshold will be silently increased.
    NW  -   number of points used to calculate weights and to interpolate.
            Required: >=2^NX+1, values less than this  threshold  will  be
            silently increased.
            Recommended value: about 2*NQ or larger

OUTPUT PARAMETERS:
    Z   -   IDW interpolant.

NOTES:
  * best results are obtained with quadratic models, linear models are not
    recommended to use unless you are pretty sure that it is what you want
  * this subroutine is always succeeds (as long as correct parameters  are
    passed).
  * see  'Multivariate  Interpolation  of Large Sets of Scattered Data' by
    Robert J. Renka for more information on this algorithm.


  -- ALGLIB PROJECT --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
void idwbuildnoisy(const real_2d_array &xy, const ae_int_t n, const ae_int_t nx, const ae_int_t d, const ae_int_t nq, const ae_int_t nw, idwinterpolant &z)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::idwbuildnoisy(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, nx, d, nq, nw, const_cast<alglib_impl::idwinterpolant*>(z.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Barycentric interpolant.
*************************************************************************/
_barycentricinterpolant_owner::_barycentricinterpolant_owner()
{
    p_struct = (alglib_impl::barycentricinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::barycentricinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_barycentricinterpolant_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_barycentricinterpolant_owner::_barycentricinterpolant_owner(const _barycentricinterpolant_owner &rhs)
{
    p_struct = (alglib_impl::barycentricinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::barycentricinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_barycentricinterpolant_init_copy(p_struct, const_cast<alglib_impl::barycentricinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_barycentricinterpolant_owner& _barycentricinterpolant_owner::operator=(const _barycentricinterpolant_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_barycentricinterpolant_clear(p_struct);
    if( !alglib_impl::_barycentricinterpolant_init_copy(p_struct, const_cast<alglib_impl::barycentricinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_barycentricinterpolant_owner::~_barycentricinterpolant_owner()
{
    alglib_impl::_barycentricinterpolant_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::barycentricinterpolant* _barycentricinterpolant_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::barycentricinterpolant* _barycentricinterpolant_owner::c_ptr() const
{
    return const_cast<alglib_impl::barycentricinterpolant*>(p_struct);
}
barycentricinterpolant::barycentricinterpolant() : _barycentricinterpolant_owner() 
{
}

barycentricinterpolant::barycentricinterpolant(const barycentricinterpolant &rhs):_barycentricinterpolant_owner(rhs) 
{
}

barycentricinterpolant& barycentricinterpolant::operator=(const barycentricinterpolant &rhs)
{
    if( this==&rhs )
        return *this;
    _barycentricinterpolant_owner::operator=(rhs);
    return *this;
}

barycentricinterpolant::~barycentricinterpolant()
{
}

/*************************************************************************
Rational interpolation using barycentric formula

F(t) = SUM(i=0,n-1,w[i]*f[i]/(t-x[i])) / SUM(i=0,n-1,w[i]/(t-x[i]))

Input parameters:
    B   -   barycentric interpolant built with one of model building
            subroutines.
    T   -   interpolation point

Result:
    barycentric interpolant F(t)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
double barycentriccalc(const barycentricinterpolant &b, const double t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::barycentriccalc(const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), t, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Differentiation of barycentric interpolant: first derivative.

Algorithm used in this subroutine is very robust and should not fail until
provided with values too close to MaxRealNumber  (usually  MaxRealNumber/N
or greater will overflow).

INPUT PARAMETERS:
    B   -   barycentric interpolant built with one of model building
            subroutines.
    T   -   interpolation point

OUTPUT PARAMETERS:
    F   -   barycentric interpolant at T
    DF  -   first derivative

NOTE


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricdiff1(const barycentricinterpolant &b, const double t, double &f, double &df)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricdiff1(const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), t, &f, &df, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Differentiation of barycentric interpolant: first/second derivatives.

INPUT PARAMETERS:
    B   -   barycentric interpolant built with one of model building
            subroutines.
    T   -   interpolation point

OUTPUT PARAMETERS:
    F   -   barycentric interpolant at T
    DF  -   first derivative
    D2F -   second derivative

NOTE: this algorithm may fail due to overflow/underflor if  used  on  data
whose values are close to MaxRealNumber or MinRealNumber.  Use more robust
BarycentricDiff1() subroutine in such cases.


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricdiff2(const barycentricinterpolant &b, const double t, double &f, double &df, double &d2f)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricdiff2(const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), t, &f, &df, &d2f, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine performs linear transformation of the argument.

INPUT PARAMETERS:
    B       -   rational interpolant in barycentric form
    CA, CB  -   transformation coefficients: x = CA*t + CB

OUTPUT PARAMETERS:
    B       -   transformed interpolant with X replaced by T

  -- ALGLIB PROJECT --
     Copyright 19.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentriclintransx(const barycentricinterpolant &b, const double ca, const double cb)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentriclintransx(const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), ca, cb, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This  subroutine   performs   linear  transformation  of  the  barycentric
interpolant.

INPUT PARAMETERS:
    B       -   rational interpolant in barycentric form
    CA, CB  -   transformation coefficients: B2(x) = CA*B(x) + CB

OUTPUT PARAMETERS:
    B       -   transformed interpolant

  -- ALGLIB PROJECT --
     Copyright 19.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentriclintransy(const barycentricinterpolant &b, const double ca, const double cb)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentriclintransy(const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), ca, cb, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Extracts X/Y/W arrays from rational interpolant

INPUT PARAMETERS:
    B   -   barycentric interpolant

OUTPUT PARAMETERS:
    N   -   nodes count, N>0
    X   -   interpolation nodes, array[0..N-1]
    F   -   function values, array[0..N-1]
    W   -   barycentric weights, array[0..N-1]

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricunpack(const barycentricinterpolant &b, ae_int_t &n, real_1d_array &x, real_1d_array &y, real_1d_array &w)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricunpack(const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), &n, const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Rational interpolant from X/Y/W arrays

F(t) = SUM(i=0,n-1,w[i]*f[i]/(t-x[i])) / SUM(i=0,n-1,w[i]/(t-x[i]))

INPUT PARAMETERS:
    X   -   interpolation nodes, array[0..N-1]
    F   -   function values, array[0..N-1]
    W   -   barycentric weights, array[0..N-1]
    N   -   nodes count, N>0

OUTPUT PARAMETERS:
    B   -   barycentric interpolant built from (X, Y, W)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricbuildxyw(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, barycentricinterpolant &b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricbuildxyw(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Rational interpolant without poles

The subroutine constructs the rational interpolating function without real
poles  (see  'Barycentric rational interpolation with no  poles  and  high
rates of approximation', Michael S. Floater. and  Kai  Hormann,  for  more
information on this subject).

Input parameters:
    X   -   interpolation nodes, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of nodes, N>0.
    D   -   order of the interpolation scheme, 0 <= D <= N-1.
            D<0 will cause an error.
            D>=N it will be replaced with D=N-1.
            if you don't know what D to choose, use small value about 3-5.

Output parameters:
    B   -   barycentric interpolant.

Note:
    this algorithm always succeeds and calculates the weights  with  close
    to machine precision.

  -- ALGLIB PROJECT --
     Copyright 17.06.2007 by Bochkanov Sergey
*************************************************************************/
void barycentricbuildfloaterhormann(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t d, barycentricinterpolant &b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricbuildfloaterhormann(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, d, const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from barycentric representation to Chebyshev basis.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    P   -   polynomial in barycentric form
    A,B -   base interval for Chebyshev polynomials (see below)
            A<>B

OUTPUT PARAMETERS
    T   -   coefficients of Chebyshev representation;
            P(x) = sum { T[i]*Ti(2*(x-A)/(B-A)-1), i=0..N-1 },
            where Ti - I-th Chebyshev polynomial.

NOTES:
    barycentric interpolant passed as P may be either polynomial  obtained
    from  polynomial  interpolation/ fitting or rational function which is
    NOT polynomial. We can't distinguish between these two cases, and this
    algorithm just tries to work assuming that P IS a polynomial.  If not,
    algorithm will return results, but they won't have any meaning.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialbar2cheb(const barycentricinterpolant &p, const double a, const double b, real_1d_array &t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbar2cheb(const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), a, b, const_cast<alglib_impl::ae_vector*>(t.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from Chebyshev basis to barycentric representation.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    T   -   coefficients of Chebyshev representation;
            P(x) = sum { T[i]*Ti(2*(x-A)/(B-A)-1), i=0..N },
            where Ti - I-th Chebyshev polynomial.
    N   -   number of coefficients:
            * if given, only leading N elements of T are used
            * if not given, automatically determined from size of T
    A,B -   base interval for Chebyshev polynomials (see above)
            A<B

OUTPUT PARAMETERS
    P   -   polynomial in barycentric form

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialcheb2bar(const real_1d_array &t, const ae_int_t n, const double a, const double b, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialcheb2bar(const_cast<alglib_impl::ae_vector*>(t.c_ptr()), n, a, b, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from Chebyshev basis to barycentric representation.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    T   -   coefficients of Chebyshev representation;
            P(x) = sum { T[i]*Ti(2*(x-A)/(B-A)-1), i=0..N },
            where Ti - I-th Chebyshev polynomial.
    N   -   number of coefficients:
            * if given, only leading N elements of T are used
            * if not given, automatically determined from size of T
    A,B -   base interval for Chebyshev polynomials (see above)
            A<B

OUTPUT PARAMETERS
    P   -   polynomial in barycentric form

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialcheb2bar(const real_1d_array &t, const double a, const double b, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = t.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialcheb2bar(const_cast<alglib_impl::ae_vector*>(t.c_ptr()), n, a, b, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from barycentric representation to power basis.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    P   -   polynomial in barycentric form
    C   -   offset (see below); 0.0 is used as default value.
    S   -   scale (see below);  1.0 is used as default value. S<>0.

OUTPUT PARAMETERS
    A   -   coefficients, P(x) = sum { A[i]*((X-C)/S)^i, i=0..N-1 }
    N   -   number of coefficients (polynomial degree plus 1)

NOTES:
1.  this function accepts offset and scale, which can be  set  to  improve
    numerical properties of polynomial. For example, if P was obtained  as
    result of interpolation on [-1,+1],  you  can  set  C=0  and  S=1  and
    represent  P  as sum of 1, x, x^2, x^3 and so on. In most cases you it
    is exactly what you need.

    However, if your interpolation model was built on [999,1001], you will
    see significant growth of numerical errors when using {1, x, x^2, x^3}
    as basis. Representing P as sum of 1, (x-1000), (x-1000)^2, (x-1000)^3
    will be better option. Such representation can be  obtained  by  using
    1000.0 as offset C and 1.0 as scale S.

2.  power basis is ill-conditioned and tricks described above can't  solve
    this problem completely. This function  will  return  coefficients  in
    any  case,  but  for  N>8  they  will  become unreliable. However, N's
    less than 5 are pretty safe.

3.  barycentric interpolant passed as P may be either polynomial  obtained
    from  polynomial  interpolation/ fitting or rational function which is
    NOT polynomial. We can't distinguish between these two cases, and this
    algorithm just tries to work assuming that P IS a polynomial.  If not,
    algorithm will return results, but they won't have any meaning.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialbar2pow(const barycentricinterpolant &p, const double c, const double s, real_1d_array &a)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbar2pow(const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), c, s, const_cast<alglib_impl::ae_vector*>(a.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from barycentric representation to power basis.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    P   -   polynomial in barycentric form
    C   -   offset (see below); 0.0 is used as default value.
    S   -   scale (see below);  1.0 is used as default value. S<>0.

OUTPUT PARAMETERS
    A   -   coefficients, P(x) = sum { A[i]*((X-C)/S)^i, i=0..N-1 }
    N   -   number of coefficients (polynomial degree plus 1)

NOTES:
1.  this function accepts offset and scale, which can be  set  to  improve
    numerical properties of polynomial. For example, if P was obtained  as
    result of interpolation on [-1,+1],  you  can  set  C=0  and  S=1  and
    represent  P  as sum of 1, x, x^2, x^3 and so on. In most cases you it
    is exactly what you need.

    However, if your interpolation model was built on [999,1001], you will
    see significant growth of numerical errors when using {1, x, x^2, x^3}
    as basis. Representing P as sum of 1, (x-1000), (x-1000)^2, (x-1000)^3
    will be better option. Such representation can be  obtained  by  using
    1000.0 as offset C and 1.0 as scale S.

2.  power basis is ill-conditioned and tricks described above can't  solve
    this problem completely. This function  will  return  coefficients  in
    any  case,  but  for  N>8  they  will  become unreliable. However, N's
    less than 5 are pretty safe.

3.  barycentric interpolant passed as P may be either polynomial  obtained
    from  polynomial  interpolation/ fitting or rational function which is
    NOT polynomial. We can't distinguish between these two cases, and this
    algorithm just tries to work assuming that P IS a polynomial.  If not,
    algorithm will return results, but they won't have any meaning.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialbar2pow(const barycentricinterpolant &p, real_1d_array &a)
{
    alglib_impl::ae_state _alglib_env_state;    
    double c;
    double s;

    c = 0;
    s = 1;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbar2pow(const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), c, s, const_cast<alglib_impl::ae_vector*>(a.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from power basis to barycentric representation.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    A   -   coefficients, P(x) = sum { A[i]*((X-C)/S)^i, i=0..N-1 }
    N   -   number of coefficients (polynomial degree plus 1)
            * if given, only leading N elements of A are used
            * if not given, automatically determined from size of A
    C   -   offset (see below); 0.0 is used as default value.
    S   -   scale (see below);  1.0 is used as default value. S<>0.

OUTPUT PARAMETERS
    P   -   polynomial in barycentric form


NOTES:
1.  this function accepts offset and scale, which can be  set  to  improve
    numerical properties of polynomial. For example, if you interpolate on
    [-1,+1],  you  can  set C=0 and S=1 and convert from sum of 1, x, x^2,
    x^3 and so on. In most cases you it is exactly what you need.

    However, if your interpolation model was built on [999,1001], you will
    see significant growth of numerical errors when using {1, x, x^2, x^3}
    as  input  basis.  Converting  from  sum  of  1, (x-1000), (x-1000)^2,
    (x-1000)^3 will be better option (you have to specify 1000.0 as offset
    C and 1.0 as scale S).

2.  power basis is ill-conditioned and tricks described above can't  solve
    this problem completely. This function  will  return barycentric model
    in any case, but for N>8 accuracy well degrade. However, N's less than
    5 are pretty safe.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialpow2bar(const real_1d_array &a, const ae_int_t n, const double c, const double s, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialpow2bar(const_cast<alglib_impl::ae_vector*>(a.c_ptr()), n, c, s, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Conversion from power basis to barycentric representation.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    A   -   coefficients, P(x) = sum { A[i]*((X-C)/S)^i, i=0..N-1 }
    N   -   number of coefficients (polynomial degree plus 1)
            * if given, only leading N elements of A are used
            * if not given, automatically determined from size of A
    C   -   offset (see below); 0.0 is used as default value.
    S   -   scale (see below);  1.0 is used as default value. S<>0.

OUTPUT PARAMETERS
    P   -   polynomial in barycentric form


NOTES:
1.  this function accepts offset and scale, which can be  set  to  improve
    numerical properties of polynomial. For example, if you interpolate on
    [-1,+1],  you  can  set C=0 and S=1 and convert from sum of 1, x, x^2,
    x^3 and so on. In most cases you it is exactly what you need.

    However, if your interpolation model was built on [999,1001], you will
    see significant growth of numerical errors when using {1, x, x^2, x^3}
    as  input  basis.  Converting  from  sum  of  1, (x-1000), (x-1000)^2,
    (x-1000)^3 will be better option (you have to specify 1000.0 as offset
    C and 1.0 as scale S).

2.  power basis is ill-conditioned and tricks described above can't  solve
    this problem completely. This function  will  return barycentric model
    in any case, but for N>8 accuracy well degrade. However, N's less than
    5 are pretty safe.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialpow2bar(const real_1d_array &a, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    double c;
    double s;

    n = a.length();
    c = 0;
    s = 1;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialpow2bar(const_cast<alglib_impl::ae_vector*>(a.c_ptr()), n, c, s, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant: generation of the model on the general grid.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    X   -   abscissas, array[0..N-1]
    Y   -   function values, array[0..N-1]
    N   -   number of points, N>=1

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuild(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuild(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant: generation of the model on the general grid.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    X   -   abscissas, array[0..N-1]
    Y   -   function values, array[0..N-1]
    N   -   number of points, N>=1

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuild(const real_1d_array &x, const real_1d_array &y, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'polynomialbuild': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuild(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant: generation of the model on equidistant grid.
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1]
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildeqdist(const double a, const double b, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuildeqdist(a, b, const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant: generation of the model on equidistant grid.
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1]
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildeqdist(const double a, const double b, const real_1d_array &y, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = y.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuildeqdist(a, b, const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant on Chebyshev grid (first kind).
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1],
            Y[I] = Y(0.5*(B+A) + 0.5*(B-A)*Cos(PI*(2*i+1)/(2*n)))
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildcheb1(const double a, const double b, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuildcheb1(a, b, const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant on Chebyshev grid (first kind).
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1],
            Y[I] = Y(0.5*(B+A) + 0.5*(B-A)*Cos(PI*(2*i+1)/(2*n)))
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildcheb1(const double a, const double b, const real_1d_array &y, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = y.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuildcheb1(a, b, const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant on Chebyshev grid (second kind).
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1],
            Y[I] = Y(0.5*(B+A) + 0.5*(B-A)*Cos(PI*i/(n-1)))
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildcheb2(const double a, const double b, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuildcheb2(a, b, const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Lagrange intepolant on Chebyshev grid (second kind).
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1],
            Y[I] = Y(0.5*(B+A) + 0.5*(B-A)*Cos(PI*i/(n-1)))
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildcheb2(const double a, const double b, const real_1d_array &y, barycentricinterpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = y.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialbuildcheb2(a, b, const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fast equidistant polynomial interpolation function with O(N) complexity

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on equidistant grid, N>=1
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use  PolynomialBuildEqDist()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalceqdist(const double a, const double b, const real_1d_array &f, const ae_int_t n, const double t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::polynomialcalceqdist(a, b, const_cast<alglib_impl::ae_vector*>(f.c_ptr()), n, t, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fast equidistant polynomial interpolation function with O(N) complexity

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on equidistant grid, N>=1
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use  PolynomialBuildEqDist()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalceqdist(const double a, const double b, const real_1d_array &f, const double t)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = f.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::polynomialcalceqdist(a, b, const_cast<alglib_impl::ae_vector*>(f.c_ptr()), n, t, &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fast polynomial interpolation function on Chebyshev points (first kind)
with O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on Chebyshev grid (first kind),
            X[i] = 0.5*(B+A) + 0.5*(B-A)*Cos(PI*(2*i+1)/(2*n))
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use  PolIntBuildCheb1()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalccheb1(const double a, const double b, const real_1d_array &f, const ae_int_t n, const double t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::polynomialcalccheb1(a, b, const_cast<alglib_impl::ae_vector*>(f.c_ptr()), n, t, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fast polynomial interpolation function on Chebyshev points (first kind)
with O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on Chebyshev grid (first kind),
            X[i] = 0.5*(B+A) + 0.5*(B-A)*Cos(PI*(2*i+1)/(2*n))
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use  PolIntBuildCheb1()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalccheb1(const double a, const double b, const real_1d_array &f, const double t)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = f.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::polynomialcalccheb1(a, b, const_cast<alglib_impl::ae_vector*>(f.c_ptr()), n, t, &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fast polynomial interpolation function on Chebyshev points (second kind)
with O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on Chebyshev grid (second kind),
            X[i] = 0.5*(B+A) + 0.5*(B-A)*Cos(PI*i/(n-1))
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use PolIntBuildCheb2()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalccheb2(const double a, const double b, const real_1d_array &f, const ae_int_t n, const double t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::polynomialcalccheb2(a, b, const_cast<alglib_impl::ae_vector*>(f.c_ptr()), n, t, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fast polynomial interpolation function on Chebyshev points (second kind)
with O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on Chebyshev grid (second kind),
            X[i] = 0.5*(B+A) + 0.5*(B-A)*Cos(PI*i/(n-1))
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use PolIntBuildCheb2()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalccheb2(const double a, const double b, const real_1d_array &f, const double t)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;

    n = f.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::polynomialcalccheb2(a, b, const_cast<alglib_impl::ae_vector*>(f.c_ptr()), n, t, &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
1-dimensional spline inteprolant
*************************************************************************/
_spline1dinterpolant_owner::_spline1dinterpolant_owner()
{
    p_struct = (alglib_impl::spline1dinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::spline1dinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_spline1dinterpolant_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_spline1dinterpolant_owner::_spline1dinterpolant_owner(const _spline1dinterpolant_owner &rhs)
{
    p_struct = (alglib_impl::spline1dinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::spline1dinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_spline1dinterpolant_init_copy(p_struct, const_cast<alglib_impl::spline1dinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_spline1dinterpolant_owner& _spline1dinterpolant_owner::operator=(const _spline1dinterpolant_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_spline1dinterpolant_clear(p_struct);
    if( !alglib_impl::_spline1dinterpolant_init_copy(p_struct, const_cast<alglib_impl::spline1dinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_spline1dinterpolant_owner::~_spline1dinterpolant_owner()
{
    alglib_impl::_spline1dinterpolant_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::spline1dinterpolant* _spline1dinterpolant_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::spline1dinterpolant* _spline1dinterpolant_owner::c_ptr() const
{
    return const_cast<alglib_impl::spline1dinterpolant*>(p_struct);
}
spline1dinterpolant::spline1dinterpolant() : _spline1dinterpolant_owner() 
{
}

spline1dinterpolant::spline1dinterpolant(const spline1dinterpolant &rhs):_spline1dinterpolant_owner(rhs) 
{
}

spline1dinterpolant& spline1dinterpolant::operator=(const spline1dinterpolant &rhs)
{
    if( this==&rhs )
        return *this;
    _spline1dinterpolant_owner::operator=(rhs);
    return *this;
}

spline1dinterpolant::~spline1dinterpolant()
{
}

/*************************************************************************
This subroutine builds linear spline interpolant

INPUT PARAMETERS:
    X   -   spline nodes, array[0..N-1]
    Y   -   function values, array[0..N-1]
    N   -   points count (optional):
            * N>=2
            * if given, only first N points are used to build spline
            * if not given, automatically detected from X/Y sizes
              (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C   -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildlinear(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildlinear(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds linear spline interpolant

INPUT PARAMETERS:
    X   -   spline nodes, array[0..N-1]
    Y   -   function values, array[0..N-1]
    N   -   points count (optional):
            * N>=2
            * if given, only first N points are used to build spline
            * if not given, automatically detected from X/Y sizes
              (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C   -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildlinear(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dbuildlinear': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildlinear(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds cubic spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1].
    Y           -   function values, array[0..N-1].

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    C           -   spline interpolant

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds cubic spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1].
    Y           -   function values, array[0..N-1].

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    C           -   spline interpolant

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildcubic(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundltype;
    double boundl;
    ae_int_t boundrtype;
    double boundr;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dbuildcubic': looks like one of arguments has wrong size");
    n = x.length();
    boundltype = 0;
    boundl = 0;
    boundrtype = 0;
    boundr = 0;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at nodes x[], it calculates and returns table of function derivatives  d[]
(calculated at the same nodes x[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   spline nodes
    Y           -   function values

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    D           -   derivative values at X[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.
Derivative values are correctly reordered on return, so  D[I]  is  always
equal to S'(X[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dgriddiffcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, real_1d_array &d)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dgriddiffcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(d.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at nodes x[], it calculates and returns table of function derivatives  d[]
(calculated at the same nodes x[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   spline nodes
    Y           -   function values

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    D           -   derivative values at X[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.
Derivative values are correctly reordered on return, so  D[I]  is  always
equal to S'(X[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dgriddiffcubic(const real_1d_array &x, const real_1d_array &y, real_1d_array &d)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundltype;
    double boundl;
    ae_int_t boundrtype;
    double boundr;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dgriddiffcubic': looks like one of arguments has wrong size");
    n = x.length();
    boundltype = 0;
    boundl = 0;
    boundrtype = 0;
    boundr = 0;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dgriddiffcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(d.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at  nodes  x[],  it  calculates  and  returns  tables  of first and second
function derivatives d1[] and d2[] (calculated at the same nodes x[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   spline nodes
    Y           -   function values

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    D1          -   S' values at X[]
    D2          -   S'' values at X[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.
Derivative values are correctly reordered on return, so  D[I]  is  always
equal to S'(X[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dgriddiff2cubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, real_1d_array &d1, real_1d_array &d2)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dgriddiff2cubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(d1.c_ptr()), const_cast<alglib_impl::ae_vector*>(d2.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at  nodes  x[],  it  calculates  and  returns  tables  of first and second
function derivatives d1[] and d2[] (calculated at the same nodes x[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   spline nodes
    Y           -   function values

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    D1          -   S' values at X[]
    D2          -   S'' values at X[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.
Derivative values are correctly reordered on return, so  D[I]  is  always
equal to S'(X[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dgriddiff2cubic(const real_1d_array &x, const real_1d_array &y, real_1d_array &d1, real_1d_array &d2)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundltype;
    double boundl;
    ae_int_t boundrtype;
    double boundr;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dgriddiff2cubic': looks like one of arguments has wrong size");
    n = x.length();
    boundltype = 0;
    boundl = 0;
    boundrtype = 0;
    boundr = 0;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dgriddiff2cubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(d1.c_ptr()), const_cast<alglib_impl::ae_vector*>(d2.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function values y2[] (calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, const real_1d_array &x2, const ae_int_t n2, real_1d_array &y2)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dconvcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(x2.c_ptr()), n2, const_cast<alglib_impl::ae_vector*>(y2.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function values y2[] (calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvcubic(const real_1d_array &x, const real_1d_array &y, const real_1d_array &x2, real_1d_array &y2)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundltype;
    double boundl;
    ae_int_t boundrtype;
    double boundr;
    ae_int_t n2;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dconvcubic': looks like one of arguments has wrong size");
    n = x.length();
    boundltype = 0;
    boundl = 0;
    boundrtype = 0;
    boundr = 0;
    n2 = x2.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dconvcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(x2.c_ptr()), n2, const_cast<alglib_impl::ae_vector*>(y2.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function values y2[] and derivatives d2[] (calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]
    D2          -   first derivatives at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiffcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, const real_1d_array &x2, const ae_int_t n2, real_1d_array &y2, real_1d_array &d2)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dconvdiffcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(x2.c_ptr()), n2, const_cast<alglib_impl::ae_vector*>(y2.c_ptr()), const_cast<alglib_impl::ae_vector*>(d2.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function values y2[] and derivatives d2[] (calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]
    D2          -   first derivatives at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiffcubic(const real_1d_array &x, const real_1d_array &y, const real_1d_array &x2, real_1d_array &y2, real_1d_array &d2)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundltype;
    double boundl;
    ae_int_t boundrtype;
    double boundr;
    ae_int_t n2;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dconvdiffcubic': looks like one of arguments has wrong size");
    n = x.length();
    boundltype = 0;
    boundl = 0;
    boundrtype = 0;
    boundr = 0;
    n2 = x2.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dconvdiffcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(x2.c_ptr()), n2, const_cast<alglib_impl::ae_vector*>(y2.c_ptr()), const_cast<alglib_impl::ae_vector*>(d2.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function  values  y2[],  first  and  second  derivatives  d2[]  and  dd2[]
(calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]
    D2          -   first derivatives at X2[]
    DD2         -   second derivatives at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiff2cubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, const real_1d_array &x2, const ae_int_t n2, real_1d_array &y2, real_1d_array &d2, real_1d_array &dd2)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dconvdiff2cubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(x2.c_ptr()), n2, const_cast<alglib_impl::ae_vector*>(y2.c_ptr()), const_cast<alglib_impl::ae_vector*>(d2.c_ptr()), const_cast<alglib_impl::ae_vector*>(dd2.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function  values  y2[],  first  and  second  derivatives  d2[]  and  dd2[]
(calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]
    D2          -   first derivatives at X2[]
    DD2         -   second derivatives at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiff2cubic(const real_1d_array &x, const real_1d_array &y, const real_1d_array &x2, real_1d_array &y2, real_1d_array &d2, real_1d_array &dd2)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundltype;
    double boundl;
    ae_int_t boundrtype;
    double boundr;
    ae_int_t n2;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dconvdiff2cubic': looks like one of arguments has wrong size");
    n = x.length();
    boundltype = 0;
    boundl = 0;
    boundrtype = 0;
    boundr = 0;
    n2 = x2.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dconvdiff2cubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundltype, boundl, boundrtype, boundr, const_cast<alglib_impl::ae_vector*>(x2.c_ptr()), n2, const_cast<alglib_impl::ae_vector*>(y2.c_ptr()), const_cast<alglib_impl::ae_vector*>(d2.c_ptr()), const_cast<alglib_impl::ae_vector*>(dd2.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds Catmull-Rom spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1].
    Y           -   function values, array[0..N-1].

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundType   -   boundary condition type:
                    * -1 for periodic boundary condition
                    *  0 for parabolically terminated spline (default)
    Tension     -   tension parameter:
                    * tension=0   corresponds to classic Catmull-Rom spline (default)
                    * 0<tension<1 corresponds to more general form - cardinal spline

OUTPUT PARAMETERS:
    C           -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildcatmullrom(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundtype, const double tension, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildcatmullrom(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundtype, tension, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds Catmull-Rom spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1].
    Y           -   function values, array[0..N-1].

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundType   -   boundary condition type:
                    * -1 for periodic boundary condition
                    *  0 for parabolically terminated spline (default)
    Tension     -   tension parameter:
                    * tension=0   corresponds to classic Catmull-Rom spline (default)
                    * 0<tension<1 corresponds to more general form - cardinal spline

OUTPUT PARAMETERS:
    C           -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildcatmullrom(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t boundtype;
    double tension;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dbuildcatmullrom': looks like one of arguments has wrong size");
    n = x.length();
    boundtype = 0;
    tension = 0;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildcatmullrom(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, boundtype, tension, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds Hermite spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1]
    Y           -   function values, array[0..N-1]
    D           -   derivatives, array[0..N-1]
    N           -   points count (optional):
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C           -   spline interpolant.


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildhermite(const real_1d_array &x, const real_1d_array &y, const real_1d_array &d, const ae_int_t n, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildhermite(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(d.c_ptr()), n, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds Hermite spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1]
    Y           -   function values, array[0..N-1]
    D           -   derivatives, array[0..N-1]
    N           -   points count (optional):
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C           -   spline interpolant.


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildhermite(const real_1d_array &x, const real_1d_array &y, const real_1d_array &d, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()) || (x.length()!=d.length()))
        throw ap_error("Error while calling 'spline1dbuildhermite': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildhermite(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(d.c_ptr()), n, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds Akima spline interpolant

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1]
    Y           -   function values, array[0..N-1]
    N           -   points count (optional):
                    * N>=5
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C           -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildakima(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildakima(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds Akima spline interpolant

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1]
    Y           -   function values, array[0..N-1]
    N           -   points count (optional):
                    * N>=5
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C           -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildakima(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dbuildakima': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dbuildakima(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine calculates the value of the spline at the given point X.

INPUT PARAMETERS:
    C   -   spline interpolant
    X   -   point

Result:
    S(x)

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
double spline1dcalc(const spline1dinterpolant &c, const double x)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::spline1dcalc(const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), x, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine differentiates the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.
    X   -   point

Result:
    S   -   S(x)
    DS  -   S'(x)
    D2S -   S''(x)

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1ddiff(const spline1dinterpolant &c, const double x, double &s, double &ds, double &d2s)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1ddiff(const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), x, &s, &ds, &d2s, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine unpacks the spline into the coefficients table.

INPUT PARAMETERS:
    C   -   spline interpolant.
    X   -   point

Result:
    Tbl -   coefficients table, unpacked format, array[0..N-2, 0..5].
            For I = 0...N-2:
                Tbl[I,0] = X[i]
                Tbl[I,1] = X[i+1]
                Tbl[I,2] = C0
                Tbl[I,3] = C1
                Tbl[I,4] = C2
                Tbl[I,5] = C3
            On [x[i], x[i+1]] spline is equals to:
                S(x) = C0 + C1*t + C2*t^2 + C3*t^3
                t = x-x[i]

  -- ALGLIB PROJECT --
     Copyright 29.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dunpack(const spline1dinterpolant &c, ae_int_t &n, real_2d_array &tbl)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dunpack(const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), &n, const_cast<alglib_impl::ae_matrix*>(tbl.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine performs linear transformation of the spline argument.

INPUT PARAMETERS:
    C   -   spline interpolant.
    A, B-   transformation coefficients: x = A*t + B
Result:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dlintransx(const spline1dinterpolant &c, const double a, const double b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dlintransx(const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), a, b, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine performs linear transformation of the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.
    A, B-   transformation coefficients: S2(x) = A*S(x) + B
Result:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dlintransy(const spline1dinterpolant &c, const double a, const double b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dlintransy(const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), a, b, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine integrates the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.
    X   -   right bound of the integration interval [a, x],
            here 'a' denotes min(x[])
Result:
    integral(S(t)dt,a,x)

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
double spline1dintegrate(const spline1dinterpolant &c, const double x)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::spline1dintegrate(const_cast<alglib_impl::spline1dinterpolant*>(c.c_ptr()), x, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Polynomial fitting report:
    TaskRCond       reciprocal of task's condition number
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error
*************************************************************************/
_polynomialfitreport_owner::_polynomialfitreport_owner()
{
    p_struct = (alglib_impl::polynomialfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::polynomialfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_polynomialfitreport_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_polynomialfitreport_owner::_polynomialfitreport_owner(const _polynomialfitreport_owner &rhs)
{
    p_struct = (alglib_impl::polynomialfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::polynomialfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_polynomialfitreport_init_copy(p_struct, const_cast<alglib_impl::polynomialfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_polynomialfitreport_owner& _polynomialfitreport_owner::operator=(const _polynomialfitreport_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_polynomialfitreport_clear(p_struct);
    if( !alglib_impl::_polynomialfitreport_init_copy(p_struct, const_cast<alglib_impl::polynomialfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_polynomialfitreport_owner::~_polynomialfitreport_owner()
{
    alglib_impl::_polynomialfitreport_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::polynomialfitreport* _polynomialfitreport_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::polynomialfitreport* _polynomialfitreport_owner::c_ptr() const
{
    return const_cast<alglib_impl::polynomialfitreport*>(p_struct);
}
polynomialfitreport::polynomialfitreport() : _polynomialfitreport_owner() ,taskrcond(p_struct->taskrcond),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

polynomialfitreport::polynomialfitreport(const polynomialfitreport &rhs):_polynomialfitreport_owner(rhs) ,taskrcond(p_struct->taskrcond),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

polynomialfitreport& polynomialfitreport::operator=(const polynomialfitreport &rhs)
{
    if( this==&rhs )
        return *this;
    _polynomialfitreport_owner::operator=(rhs);
    return *this;
}

polynomialfitreport::~polynomialfitreport()
{
}


/*************************************************************************
Barycentric fitting report:
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error
    TaskRCond       reciprocal of task's condition number
*************************************************************************/
_barycentricfitreport_owner::_barycentricfitreport_owner()
{
    p_struct = (alglib_impl::barycentricfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::barycentricfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_barycentricfitreport_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_barycentricfitreport_owner::_barycentricfitreport_owner(const _barycentricfitreport_owner &rhs)
{
    p_struct = (alglib_impl::barycentricfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::barycentricfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_barycentricfitreport_init_copy(p_struct, const_cast<alglib_impl::barycentricfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_barycentricfitreport_owner& _barycentricfitreport_owner::operator=(const _barycentricfitreport_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_barycentricfitreport_clear(p_struct);
    if( !alglib_impl::_barycentricfitreport_init_copy(p_struct, const_cast<alglib_impl::barycentricfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_barycentricfitreport_owner::~_barycentricfitreport_owner()
{
    alglib_impl::_barycentricfitreport_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::barycentricfitreport* _barycentricfitreport_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::barycentricfitreport* _barycentricfitreport_owner::c_ptr() const
{
    return const_cast<alglib_impl::barycentricfitreport*>(p_struct);
}
barycentricfitreport::barycentricfitreport() : _barycentricfitreport_owner() ,taskrcond(p_struct->taskrcond),dbest(p_struct->dbest),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

barycentricfitreport::barycentricfitreport(const barycentricfitreport &rhs):_barycentricfitreport_owner(rhs) ,taskrcond(p_struct->taskrcond),dbest(p_struct->dbest),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

barycentricfitreport& barycentricfitreport::operator=(const barycentricfitreport &rhs)
{
    if( this==&rhs )
        return *this;
    _barycentricfitreport_owner::operator=(rhs);
    return *this;
}

barycentricfitreport::~barycentricfitreport()
{
}


/*************************************************************************
Spline fitting report:
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error

Fields  below are  filled  by   obsolete    functions   (Spline1DFitCubic,
Spline1DFitHermite). Modern fitting functions do NOT fill these fields:
    TaskRCond       reciprocal of task's condition number
*************************************************************************/
_spline1dfitreport_owner::_spline1dfitreport_owner()
{
    p_struct = (alglib_impl::spline1dfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::spline1dfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_spline1dfitreport_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_spline1dfitreport_owner::_spline1dfitreport_owner(const _spline1dfitreport_owner &rhs)
{
    p_struct = (alglib_impl::spline1dfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::spline1dfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_spline1dfitreport_init_copy(p_struct, const_cast<alglib_impl::spline1dfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_spline1dfitreport_owner& _spline1dfitreport_owner::operator=(const _spline1dfitreport_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_spline1dfitreport_clear(p_struct);
    if( !alglib_impl::_spline1dfitreport_init_copy(p_struct, const_cast<alglib_impl::spline1dfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_spline1dfitreport_owner::~_spline1dfitreport_owner()
{
    alglib_impl::_spline1dfitreport_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::spline1dfitreport* _spline1dfitreport_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::spline1dfitreport* _spline1dfitreport_owner::c_ptr() const
{
    return const_cast<alglib_impl::spline1dfitreport*>(p_struct);
}
spline1dfitreport::spline1dfitreport() : _spline1dfitreport_owner() ,taskrcond(p_struct->taskrcond),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

spline1dfitreport::spline1dfitreport(const spline1dfitreport &rhs):_spline1dfitreport_owner(rhs) ,taskrcond(p_struct->taskrcond),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

spline1dfitreport& spline1dfitreport::operator=(const spline1dfitreport &rhs)
{
    if( this==&rhs )
        return *this;
    _spline1dfitreport_owner::operator=(rhs);
    return *this;
}

spline1dfitreport::~spline1dfitreport()
{
}


/*************************************************************************
Least squares fitting report:
    TaskRCond       reciprocal of task's condition number
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error
*************************************************************************/
_lsfitreport_owner::_lsfitreport_owner()
{
    p_struct = (alglib_impl::lsfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::lsfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_lsfitreport_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_lsfitreport_owner::_lsfitreport_owner(const _lsfitreport_owner &rhs)
{
    p_struct = (alglib_impl::lsfitreport*)alglib_impl::ae_malloc(sizeof(alglib_impl::lsfitreport), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_lsfitreport_init_copy(p_struct, const_cast<alglib_impl::lsfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_lsfitreport_owner& _lsfitreport_owner::operator=(const _lsfitreport_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_lsfitreport_clear(p_struct);
    if( !alglib_impl::_lsfitreport_init_copy(p_struct, const_cast<alglib_impl::lsfitreport*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_lsfitreport_owner::~_lsfitreport_owner()
{
    alglib_impl::_lsfitreport_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::lsfitreport* _lsfitreport_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::lsfitreport* _lsfitreport_owner::c_ptr() const
{
    return const_cast<alglib_impl::lsfitreport*>(p_struct);
}
lsfitreport::lsfitreport() : _lsfitreport_owner() ,taskrcond(p_struct->taskrcond),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

lsfitreport::lsfitreport(const lsfitreport &rhs):_lsfitreport_owner(rhs) ,taskrcond(p_struct->taskrcond),rmserror(p_struct->rmserror),avgerror(p_struct->avgerror),avgrelerror(p_struct->avgrelerror),maxerror(p_struct->maxerror)
{
}

lsfitreport& lsfitreport::operator=(const lsfitreport &rhs)
{
    if( this==&rhs )
        return *this;
    _lsfitreport_owner::operator=(rhs);
    return *this;
}

lsfitreport::~lsfitreport()
{
}


/*************************************************************************
Nonlinear fitter.

You should use ALGLIB functions to work with fitter.
Never try to access its fields directly!
*************************************************************************/
_lsfitstate_owner::_lsfitstate_owner()
{
    p_struct = (alglib_impl::lsfitstate*)alglib_impl::ae_malloc(sizeof(alglib_impl::lsfitstate), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_lsfitstate_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_lsfitstate_owner::_lsfitstate_owner(const _lsfitstate_owner &rhs)
{
    p_struct = (alglib_impl::lsfitstate*)alglib_impl::ae_malloc(sizeof(alglib_impl::lsfitstate), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_lsfitstate_init_copy(p_struct, const_cast<alglib_impl::lsfitstate*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_lsfitstate_owner& _lsfitstate_owner::operator=(const _lsfitstate_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_lsfitstate_clear(p_struct);
    if( !alglib_impl::_lsfitstate_init_copy(p_struct, const_cast<alglib_impl::lsfitstate*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_lsfitstate_owner::~_lsfitstate_owner()
{
    alglib_impl::_lsfitstate_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::lsfitstate* _lsfitstate_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::lsfitstate* _lsfitstate_owner::c_ptr() const
{
    return const_cast<alglib_impl::lsfitstate*>(p_struct);
}
lsfitstate::lsfitstate() : _lsfitstate_owner() ,needf(p_struct->needf),needfg(p_struct->needfg),needfgh(p_struct->needfgh),xupdated(p_struct->xupdated),c(&p_struct->c),f(p_struct->f),g(&p_struct->g),h(&p_struct->h),x(&p_struct->x)
{
}

lsfitstate::lsfitstate(const lsfitstate &rhs):_lsfitstate_owner(rhs) ,needf(p_struct->needf),needfg(p_struct->needfg),needfgh(p_struct->needfgh),xupdated(p_struct->xupdated),c(&p_struct->c),f(p_struct->f),g(&p_struct->g),h(&p_struct->h),x(&p_struct->x)
{
}

lsfitstate& lsfitstate::operator=(const lsfitstate &rhs)
{
    if( this==&rhs )
        return *this;
    _lsfitstate_owner::operator=(rhs);
    return *this;
}

lsfitstate::~lsfitstate()
{
}

/*************************************************************************
Fitting by polynomials in barycentric form. This function provides  simple
unterface for unconstrained unweighted fitting. See  PolynomialFitWC()  if
you need constrained fitting.

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO:
    PolynomialFitWC()

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0
            * if given, only leading N elements of X/Y are used
            * if not given, automatically determined from sizes of X/Y
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
    P   -   interpolant in barycentric form.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

NOTES:
    you can convert P from barycentric form  to  the  power  or  Chebyshev
    basis with PolynomialBar2Pow() or PolynomialBar2Cheb() functions  from
    POLINT subpackage.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialfit(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialfit(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), const_cast<alglib_impl::polynomialfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Fitting by polynomials in barycentric form. This function provides  simple
unterface for unconstrained unweighted fitting. See  PolynomialFitWC()  if
you need constrained fitting.

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO:
    PolynomialFitWC()

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0
            * if given, only leading N elements of X/Y are used
            * if not given, automatically determined from sizes of X/Y
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
    P   -   interpolant in barycentric form.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

NOTES:
    you can convert P from barycentric form  to  the  power  or  Chebyshev
    basis with PolynomialBar2Pow() or PolynomialBar2Cheb() functions  from
    POLINT subpackage.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialfit(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'polynomialfit': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialfit(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), const_cast<alglib_impl::polynomialfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted  fitting by polynomials in barycentric form, with constraints  on
function values or first derivatives.

Small regularizing term is used when solving constrained tasks (to improve
stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO:
    PolynomialFit()

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points, N>0.
            * if given, only leading N elements of X/Y/W are used
            * if not given, automatically determined from sizes of X/Y/W
    XC  -   points where polynomial values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that P(XC[i])=YC[i]
            * DC[i]=1   means that P'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints, 0<=K<M.
            K=0 means no constraints (XC/YC/DC are not used in such cases)
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    P   -   interpolant in barycentric form.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

NOTES:
    you can convert P from barycentric form  to  the  power  or  Chebyshev
    basis with PolynomialBar2Pow() or PolynomialBar2Cheb() functions  from
    POLINT subpackage.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* even simple constraints can be inconsistent, see  Wikipedia  article  on
  this subject: http://en.wikipedia.org/wiki/Birkhoff_interpolation
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints is NOT GUARANTEED.
* in the one special cases, however, we can  guarantee  consistency.  This
  case  is:  M>1  and constraints on the function values (NOT DERIVATIVES)

Our final recommendation is to use constraints  WHEN  AND  ONLY  when  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialfitwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialfitwc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), const_cast<alglib_impl::polynomialfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted  fitting by polynomials in barycentric form, with constraints  on
function values or first derivatives.

Small regularizing term is used when solving constrained tasks (to improve
stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO:
    PolynomialFit()

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points, N>0.
            * if given, only leading N elements of X/Y/W are used
            * if not given, automatically determined from sizes of X/Y/W
    XC  -   points where polynomial values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that P(XC[i])=YC[i]
            * DC[i]=1   means that P'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints, 0<=K<M.
            K=0 means no constraints (XC/YC/DC are not used in such cases)
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    P   -   interpolant in barycentric form.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

NOTES:
    you can convert P from barycentric form  to  the  power  or  Chebyshev
    basis with PolynomialBar2Pow() or PolynomialBar2Cheb() functions  from
    POLINT subpackage.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* even simple constraints can be inconsistent, see  Wikipedia  article  on
  this subject: http://en.wikipedia.org/wiki/Birkhoff_interpolation
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints is NOT GUARANTEED.
* in the one special cases, however, we can  guarantee  consistency.  This
  case  is:  M>1  and constraints on the function values (NOT DERIVATIVES)

Our final recommendation is to use constraints  WHEN  AND  ONLY  when  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialfitwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t k;
    if( (x.length()!=y.length()) || (x.length()!=w.length()))
        throw ap_error("Error while calling 'polynomialfitwc': looks like one of arguments has wrong size");
    if( (xc.length()!=yc.length()) || (xc.length()!=dc.length()))
        throw ap_error("Error while calling 'polynomialfitwc': looks like one of arguments has wrong size");
    n = x.length();
    k = xc.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::polynomialfitwc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::barycentricinterpolant*>(p.c_ptr()), const_cast<alglib_impl::polynomialfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weghted rational least  squares  fitting  using  Floater-Hormann  rational
functions  with  optimal  D  chosen  from  [0,9],  with  constraints   and
individual weights.

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal D (least WEIGHTED root
mean square error) is chosen.  Task  is  linear,  so  linear least squares
solver  is  used.  Complexity  of  this  computational  scheme is O(N*M^2)
(mostly dominated by the least squares solver).

SEE ALSO
* BarycentricFitFloaterHormann(), "lightweight" fitting without invididual
  weights and constraints.

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points, N>0.
    XC  -   points where function values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints, 0<=K<M.
            K=0 means no constraints (XC/YC/DC are not used in such cases)
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
                        -1 means another errors in parameters passed
                           (N<=0, for example)
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroutine doesn't calculate task's condition number for K<>0.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained barycentric interpolants:
* excessive  constraints  can  be  inconsistent.   Floater-Hormann   basis
  functions aren't as flexible as splines (although they are very smooth).
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints IS NOT GUARANTEED.
* in the several special cases, however, we CAN guarantee consistency.
* one of this cases is constraints on the function  VALUES at the interval
  boundaries. Note that consustency of the  constraints  on  the  function
  DERIVATIVES is NOT guaranteed (you can use in such cases  cubic  splines
  which are more flexible).
* another  special  case  is ONE constraint on the function value (OR, but
  not AND, derivative) anywhere in the interval

Our final recommendation is to use constraints  WHEN  AND  ONLY  WHEN  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricfitfloaterhormannwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, barycentricinterpolant &b, barycentricfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricfitfloaterhormannwc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), const_cast<alglib_impl::barycentricfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Rational least squares fitting using  Floater-Hormann  rational  functions
with optimal D chosen from [0,9].

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal  D  (least  root  mean
square error) is chosen.  Task  is  linear, so linear least squares solver
is used. Complexity  of  this  computational  scheme is  O(N*M^2)  (mostly
dominated by the least squares solver).

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0.
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricfitfloaterhormann(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, barycentricinterpolant &b, barycentricfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::barycentricfitfloaterhormann(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::barycentricinterpolant*>(b.c_ptr()), const_cast<alglib_impl::barycentricfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Rational least squares fitting using  Floater-Hormann  rational  functions
with optimal D chosen from [0,9].

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal  D  (least  root  mean
square error) is chosen.  Task  is  linear, so linear least squares solver
is used. Complexity  of  this  computational  scheme is  O(N*M^2)  (mostly
dominated by the least squares solver).

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0.
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitpenalized(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitpenalized(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, rho, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Rational least squares fitting using  Floater-Hormann  rational  functions
with optimal D chosen from [0,9].

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal  D  (least  root  mean
square error) is chosen.  Task  is  linear, so linear least squares solver
is used. Complexity  of  this  computational  scheme is  O(N*M^2)  (mostly
dominated by the least squares solver).

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0.
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitpenalized(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dfitpenalized': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitpenalized(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, rho, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted fitting by penalized cubic spline.

Equidistant grid with M nodes on [min(x,xc),max(x,xc)] is  used  to  build
basis functions. Basis functions are cubic splines with  natural  boundary
conditions. Problem is regularized by  adding non-linearity penalty to the
usual least squares penalty function:

    S(x) = arg min { LS + P }, where
    LS   = SUM { w[i]^2*(y[i] - S(x[i]))^2 } - least squares penalty
    P    = C*10^rho*integral{ S''(x)^2*dx } - non-linearity penalty
    rho  - tunable constant given by user
    C    - automatically determined scale parameter,
           makes penalty invariant with respect to scaling of X, Y, W.

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            problem.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    M   -   number of basis functions ( = number_of_nodes), M>=4.
    Rho -   regularization  constant  passed   by   user.   It   penalizes
            nonlinearity in the regression spline. It  is  logarithmically
            scaled,  i.e.  actual  value  of  regularization  constant  is
            calculated as 10^Rho. It is automatically scaled so that:
            * Rho=2.0 corresponds to moderate amount of nonlinearity
            * generally, it should be somewhere in the [-8.0,+8.0]
            If you do not want to penalize nonlineary,
            pass small Rho. Values as low as -15 should work.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD or
                           Cholesky decomposition; problem may be
                           too ill-conditioned (very rare)
    S   -   spline interpolant.
    Rep -   Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

NOTE 1: additional nodes are added to the spline outside  of  the  fitting
interval to force linearity when x<min(x,xc) or x>max(x,xc).  It  is  done
for consistency - we penalize non-linearity  at [min(x,xc),max(x,xc)],  so
it is natural to force linearity outside of this interval.

NOTE 2: function automatically sorts points,  so  caller may pass unsorted
array.

  -- ALGLIB PROJECT --
     Copyright 19.10.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dfitpenalizedw(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitpenalizedw(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, m, rho, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted fitting by penalized cubic spline.

Equidistant grid with M nodes on [min(x,xc),max(x,xc)] is  used  to  build
basis functions. Basis functions are cubic splines with  natural  boundary
conditions. Problem is regularized by  adding non-linearity penalty to the
usual least squares penalty function:

    S(x) = arg min { LS + P }, where
    LS   = SUM { w[i]^2*(y[i] - S(x[i]))^2 } - least squares penalty
    P    = C*10^rho*integral{ S''(x)^2*dx } - non-linearity penalty
    rho  - tunable constant given by user
    C    - automatically determined scale parameter,
           makes penalty invariant with respect to scaling of X, Y, W.

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            problem.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    M   -   number of basis functions ( = number_of_nodes), M>=4.
    Rho -   regularization  constant  passed   by   user.   It   penalizes
            nonlinearity in the regression spline. It  is  logarithmically
            scaled,  i.e.  actual  value  of  regularization  constant  is
            calculated as 10^Rho. It is automatically scaled so that:
            * Rho=2.0 corresponds to moderate amount of nonlinearity
            * generally, it should be somewhere in the [-8.0,+8.0]
            If you do not want to penalize nonlineary,
            pass small Rho. Values as low as -15 should work.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD or
                           Cholesky decomposition; problem may be
                           too ill-conditioned (very rare)
    S   -   spline interpolant.
    Rep -   Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

NOTE 1: additional nodes are added to the spline outside  of  the  fitting
interval to force linearity when x<min(x,xc) or x>max(x,xc).  It  is  done
for consistency - we penalize non-linearity  at [min(x,xc),max(x,xc)],  so
it is natural to force linearity outside of this interval.

NOTE 2: function automatically sorts points,  so  caller may pass unsorted
array.

  -- ALGLIB PROJECT --
     Copyright 19.10.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dfitpenalizedw(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()) || (x.length()!=w.length()))
        throw ap_error("Error while calling 'spline1dfitpenalizedw': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitpenalizedw(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, m, rho, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted fitting by cubic  spline,  with constraints on function values or
derivatives.

Equidistant grid with M-2 nodes on [min(x,xc),max(x,xc)] is  used to build
basis functions. Basis functions are cubic splines with continuous  second
derivatives  and  non-fixed first  derivatives  at  interval  ends.  Small
regularizing term is used  when  solving  constrained  tasks  (to  improve
stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO
    Spline1DFitHermiteWC()  -   fitting by Hermite splines (more flexible,
                                less smooth)
    Spline1DFitCubic()      -   "lightweight" fitting  by  cubic  splines,
                                without invididual weights and constraints

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    XC  -   points where spline values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints (optional):
            * 0<=K<M.
            * K=0 means no constraints (XC/YC/DC are not used)
            * if given, only first K elements of XC/YC/DC are used
            * if not given, automatically determined from XC/YC/DC
    M   -   number of basis functions ( = number_of_nodes+2), M>=4.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    S   -   spline interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* excessive constraints can be inconsistent. Splines are  piecewise  cubic
  functions, and it is easy to create an example, where  large  number  of
  constraints  concentrated  in  small  area will result in inconsistency.
  Just because spline is not flexible enough to satisfy all of  them.  And
  same constraints spread across the  [min(x),max(x)]  will  be  perfectly
  consistent.
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints IS NOT GUARANTEED.
* in the several special cases, however, we CAN guarantee consistency.
* one of this cases is constraints  on  the  function  values  AND/OR  its
  derivatives at the interval boundaries.
* another  special  case  is ONE constraint on the function value (OR, but
  not AND, derivative) anywhere in the interval

Our final recommendation is to use constraints  WHEN  AND  ONLY  WHEN  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.


  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubicwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitcubicwc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted fitting by cubic  spline,  with constraints on function values or
derivatives.

Equidistant grid with M-2 nodes on [min(x,xc),max(x,xc)] is  used to build
basis functions. Basis functions are cubic splines with continuous  second
derivatives  and  non-fixed first  derivatives  at  interval  ends.  Small
regularizing term is used  when  solving  constrained  tasks  (to  improve
stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO
    Spline1DFitHermiteWC()  -   fitting by Hermite splines (more flexible,
                                less smooth)
    Spline1DFitCubic()      -   "lightweight" fitting  by  cubic  splines,
                                without invididual weights and constraints

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    XC  -   points where spline values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints (optional):
            * 0<=K<M.
            * K=0 means no constraints (XC/YC/DC are not used)
            * if given, only first K elements of XC/YC/DC are used
            * if not given, automatically determined from XC/YC/DC
    M   -   number of basis functions ( = number_of_nodes+2), M>=4.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    S   -   spline interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* excessive constraints can be inconsistent. Splines are  piecewise  cubic
  functions, and it is easy to create an example, where  large  number  of
  constraints  concentrated  in  small  area will result in inconsistency.
  Just because spline is not flexible enough to satisfy all of  them.  And
  same constraints spread across the  [min(x),max(x)]  will  be  perfectly
  consistent.
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints IS NOT GUARANTEED.
* in the several special cases, however, we CAN guarantee consistency.
* one of this cases is constraints  on  the  function  values  AND/OR  its
  derivatives at the interval boundaries.
* another  special  case  is ONE constraint on the function value (OR, but
  not AND, derivative) anywhere in the interval

Our final recommendation is to use constraints  WHEN  AND  ONLY  WHEN  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.


  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubicwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t k;
    if( (x.length()!=y.length()) || (x.length()!=w.length()))
        throw ap_error("Error while calling 'spline1dfitcubicwc': looks like one of arguments has wrong size");
    if( (xc.length()!=yc.length()) || (xc.length()!=dc.length()))
        throw ap_error("Error while calling 'spline1dfitcubicwc': looks like one of arguments has wrong size");
    n = x.length();
    k = xc.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitcubicwc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted  fitting  by Hermite spline,  with constraints on function values
or first derivatives.

Equidistant grid with M nodes on [min(x,xc),max(x,xc)] is  used  to  build
basis functions. Basis functions are Hermite splines.  Small  regularizing
term is used when solving constrained tasks (to improve stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO
    Spline1DFitCubicWC()    -   fitting by Cubic splines (less flexible,
                                more smooth)
    Spline1DFitHermite()    -   "lightweight" Hermite fitting, without
                                invididual weights and constraints

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    XC  -   points where spline values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints (optional):
            * 0<=K<M.
            * K=0 means no constraints (XC/YC/DC are not used)
            * if given, only first K elements of XC/YC/DC are used
            * if not given, automatically determined from XC/YC/DC
    M   -   number of basis functions (= 2 * number of nodes),
            M>=4,
            M IS EVEN!

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
                        -2 means odd M was passed (which is not supported)
                        -1 means another errors in parameters passed
                           (N<=0, for example)
    S   -   spline interpolant.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

IMPORTANT:
    this subroitine supports only even M's


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* excessive constraints can be inconsistent. Splines are  piecewise  cubic
  functions, and it is easy to create an example, where  large  number  of
  constraints  concentrated  in  small  area will result in inconsistency.
  Just because spline is not flexible enough to satisfy all of  them.  And
  same constraints spread across the  [min(x),max(x)]  will  be  perfectly
  consistent.
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints is NOT GUARANTEED.
* in the several special cases, however, we can guarantee consistency.
* one of this cases is  M>=4  and   constraints  on   the  function  value
  (AND/OR its derivative) at the interval boundaries.
* another special case is M>=4  and  ONE  constraint on the function value
  (OR, BUT NOT AND, derivative) anywhere in [min(x),max(x)]

Our final recommendation is to use constraints  WHEN  AND  ONLY  when  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermitewc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfithermitewc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted  fitting  by Hermite spline,  with constraints on function values
or first derivatives.

Equidistant grid with M nodes on [min(x,xc),max(x,xc)] is  used  to  build
basis functions. Basis functions are Hermite splines.  Small  regularizing
term is used when solving constrained tasks (to improve stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO
    Spline1DFitCubicWC()    -   fitting by Cubic splines (less flexible,
                                more smooth)
    Spline1DFitHermite()    -   "lightweight" Hermite fitting, without
                                invididual weights and constraints

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    XC  -   points where spline values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints (optional):
            * 0<=K<M.
            * K=0 means no constraints (XC/YC/DC are not used)
            * if given, only first K elements of XC/YC/DC are used
            * if not given, automatically determined from XC/YC/DC
    M   -   number of basis functions (= 2 * number of nodes),
            M>=4,
            M IS EVEN!

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
                        -2 means odd M was passed (which is not supported)
                        -1 means another errors in parameters passed
                           (N<=0, for example)
    S   -   spline interpolant.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

IMPORTANT:
    this subroitine supports only even M's


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* excessive constraints can be inconsistent. Splines are  piecewise  cubic
  functions, and it is easy to create an example, where  large  number  of
  constraints  concentrated  in  small  area will result in inconsistency.
  Just because spline is not flexible enough to satisfy all of  them.  And
  same constraints spread across the  [min(x),max(x)]  will  be  perfectly
  consistent.
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints is NOT GUARANTEED.
* in the several special cases, however, we can guarantee consistency.
* one of this cases is  M>=4  and   constraints  on   the  function  value
  (AND/OR its derivative) at the interval boundaries.
* another special case is M>=4  and  ONE  constraint on the function value
  (OR, BUT NOT AND, derivative) anywhere in [min(x),max(x)]

Our final recommendation is to use constraints  WHEN  AND  ONLY  when  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermitewc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t k;
    if( (x.length()!=y.length()) || (x.length()!=w.length()))
        throw ap_error("Error while calling 'spline1dfithermitewc': looks like one of arguments has wrong size");
    if( (xc.length()!=yc.length()) || (xc.length()!=dc.length()))
        throw ap_error("Error while calling 'spline1dfithermitewc': looks like one of arguments has wrong size");
    n = x.length();
    k = xc.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfithermitewc(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), n, const_cast<alglib_impl::ae_vector*>(xc.c_ptr()), const_cast<alglib_impl::ae_vector*>(yc.c_ptr()), const_cast<alglib_impl::ae_vector*>(dc.c_ptr()), k, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Least squares fitting by cubic spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitCubicWC().  See  Spline1DFitCubicWC() for more information
about subroutine parameters (we don't duplicate it here because of length)

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Least squares fitting by cubic spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitCubicWC().  See  Spline1DFitCubicWC() for more information
about subroutine parameters (we don't duplicate it here because of length)

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dfitcubic': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfitcubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Least squares fitting by Hermite spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitHermiteWC().  See Spline1DFitHermiteWC()  description  for
more information about subroutine parameters (we don't duplicate  it  here
because of length).

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermite(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfithermite(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Least squares fitting by Hermite spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitHermiteWC().  See Spline1DFitHermiteWC()  description  for
more information about subroutine parameters (we don't duplicate  it  here
because of length).

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermite(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    if( (x.length()!=y.length()))
        throw ap_error("Error while calling 'spline1dfithermite': looks like one of arguments has wrong size");
    n = x.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline1dfithermite(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), n, m, &info, const_cast<alglib_impl::spline1dinterpolant*>(s.c_ptr()), const_cast<alglib_impl::spline1dfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted linear least squares fitting.

QR decomposition is used to reduce task to MxM, then triangular solver  or
SVD-based solver is used depending on condition number of the  system.  It
allows to maximize speed and retain decent accuracy.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    W       -   array[0..N-1]  Weights  corresponding to function  values.
                Each summand in square  sum  of  approximation  deviations
                from  given  values  is  multiplied  by  the   square   of
                corresponding weight.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I, J] - value of J-th basis function in I-th point.
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -1    incorrect N/M were specified
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * Rep.TaskRCond     reciprocal of condition number
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearw(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, const ae_int_t n, const ae_int_t m, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinearw(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), n, m, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted linear least squares fitting.

QR decomposition is used to reduce task to MxM, then triangular solver  or
SVD-based solver is used depending on condition number of the  system.  It
allows to maximize speed and retain decent accuracy.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    W       -   array[0..N-1]  Weights  corresponding to function  values.
                Each summand in square  sum  of  approximation  deviations
                from  given  values  is  multiplied  by  the   square   of
                corresponding weight.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I, J] - value of J-th basis function in I-th point.
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -1    incorrect N/M were specified
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * Rep.TaskRCond     reciprocal of condition number
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearw(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    if( (y.length()!=w.length()) || (y.length()!=fmatrix.rows()))
        throw ap_error("Error while calling 'lsfitlinearw': looks like one of arguments has wrong size");
    n = y.length();
    m = fmatrix.cols();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinearw(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), n, m, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted constained linear least squares fitting.

This  is  variation  of LSFitLinearW(), which searchs for min|A*x=b| given
that  K  additional  constaints  C*x=bc are satisfied. It reduces original
task to modified one: min|B*y-d| WITHOUT constraints,  then LSFitLinearW()
is called.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    W       -   array[0..N-1]  Weights  corresponding to function  values.
                Each summand in square  sum  of  approximation  deviations
                from  given  values  is  multiplied  by  the   square   of
                corresponding weight.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I,J] - value of J-th basis function in I-th point.
    CMatrix -   a table of constaints, array[0..K-1,0..M].
                I-th row of CMatrix corresponds to I-th linear constraint:
                CMatrix[I,0]*C[0] + ... + CMatrix[I,M-1]*C[M-1] = CMatrix[I,M]
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.
    K       -   number of constraints, 0 <= K < M
                K=0 corresponds to absence of constraints.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -3    either   too   many  constraints  (M   or   more),
                        degenerate  constraints   (some   constraints  are
                        repetead twice) or inconsistent  constraints  were
                        specified.
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB --
     Copyright 07.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearwc(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, const real_2d_array &cmatrix, const ae_int_t n, const ae_int_t m, const ae_int_t k, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinearwc(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), const_cast<alglib_impl::ae_matrix*>(cmatrix.c_ptr()), n, m, k, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted constained linear least squares fitting.

This  is  variation  of LSFitLinearW(), which searchs for min|A*x=b| given
that  K  additional  constaints  C*x=bc are satisfied. It reduces original
task to modified one: min|B*y-d| WITHOUT constraints,  then LSFitLinearW()
is called.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    W       -   array[0..N-1]  Weights  corresponding to function  values.
                Each summand in square  sum  of  approximation  deviations
                from  given  values  is  multiplied  by  the   square   of
                corresponding weight.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I,J] - value of J-th basis function in I-th point.
    CMatrix -   a table of constaints, array[0..K-1,0..M].
                I-th row of CMatrix corresponds to I-th linear constraint:
                CMatrix[I,0]*C[0] + ... + CMatrix[I,M-1]*C[M-1] = CMatrix[I,M]
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.
    K       -   number of constraints, 0 <= K < M
                K=0 corresponds to absence of constraints.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -3    either   too   many  constraints  (M   or   more),
                        degenerate  constraints   (some   constraints  are
                        repetead twice) or inconsistent  constraints  were
                        specified.
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB --
     Copyright 07.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearwc(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, const real_2d_array &cmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (y.length()!=w.length()) || (y.length()!=fmatrix.rows()))
        throw ap_error("Error while calling 'lsfitlinearwc': looks like one of arguments has wrong size");
    if( (fmatrix.cols()!=cmatrix.cols()-1))
        throw ap_error("Error while calling 'lsfitlinearwc': looks like one of arguments has wrong size");
    n = y.length();
    m = fmatrix.cols();
    k = cmatrix.rows();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinearwc(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), const_cast<alglib_impl::ae_matrix*>(cmatrix.c_ptr()), n, m, k, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Linear least squares fitting.

QR decomposition is used to reduce task to MxM, then triangular solver  or
SVD-based solver is used depending on condition number of the  system.  It
allows to maximize speed and retain decent accuracy.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I, J] - value of J-th basis function in I-th point.
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * Rep.TaskRCond     reciprocal of condition number
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinear(const real_1d_array &y, const real_2d_array &fmatrix, const ae_int_t n, const ae_int_t m, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinear(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), n, m, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Linear least squares fitting.

QR decomposition is used to reduce task to MxM, then triangular solver  or
SVD-based solver is used depending on condition number of the  system.  It
allows to maximize speed and retain decent accuracy.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I, J] - value of J-th basis function in I-th point.
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * Rep.TaskRCond     reciprocal of condition number
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinear(const real_1d_array &y, const real_2d_array &fmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    if( (y.length()!=fmatrix.rows()))
        throw ap_error("Error while calling 'lsfitlinear': looks like one of arguments has wrong size");
    n = y.length();
    m = fmatrix.cols();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinear(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), n, m, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Constained linear least squares fitting.

This  is  variation  of LSFitLinear(),  which searchs for min|A*x=b| given
that  K  additional  constaints  C*x=bc are satisfied. It reduces original
task to modified one: min|B*y-d| WITHOUT constraints,  then  LSFitLinear()
is called.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I,J] - value of J-th basis function in I-th point.
    CMatrix -   a table of constaints, array[0..K-1,0..M].
                I-th row of CMatrix corresponds to I-th linear constraint:
                CMatrix[I,0]*C[0] + ... + CMatrix[I,M-1]*C[M-1] = CMatrix[I,M]
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.
    K       -   number of constraints, 0 <= K < M
                K=0 corresponds to absence of constraints.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -3    either   too   many  constraints  (M   or   more),
                        degenerate  constraints   (some   constraints  are
                        repetead twice) or inconsistent  constraints  were
                        specified.
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB --
     Copyright 07.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearc(const real_1d_array &y, const real_2d_array &fmatrix, const real_2d_array &cmatrix, const ae_int_t n, const ae_int_t m, const ae_int_t k, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinearc(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), const_cast<alglib_impl::ae_matrix*>(cmatrix.c_ptr()), n, m, k, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Constained linear least squares fitting.

This  is  variation  of LSFitLinear(),  which searchs for min|A*x=b| given
that  K  additional  constaints  C*x=bc are satisfied. It reduces original
task to modified one: min|B*y-d| WITHOUT constraints,  then  LSFitLinear()
is called.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I,J] - value of J-th basis function in I-th point.
    CMatrix -   a table of constaints, array[0..K-1,0..M].
                I-th row of CMatrix corresponds to I-th linear constraint:
                CMatrix[I,0]*C[0] + ... + CMatrix[I,M-1]*C[M-1] = CMatrix[I,M]
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.
    K       -   number of constraints, 0 <= K < M
                K=0 corresponds to absence of constraints.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -3    either   too   many  constraints  (M   or   more),
                        degenerate  constraints   (some   constraints  are
                        repetead twice) or inconsistent  constraints  were
                        specified.
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB --
     Copyright 07.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearc(const real_1d_array &y, const real_2d_array &fmatrix, const real_2d_array &cmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (y.length()!=fmatrix.rows()))
        throw ap_error("Error while calling 'lsfitlinearc': looks like one of arguments has wrong size");
    if( (fmatrix.cols()!=cmatrix.cols()-1))
        throw ap_error("Error while calling 'lsfitlinearc': looks like one of arguments has wrong size");
    n = y.length();
    m = fmatrix.cols();
    k = cmatrix.rows();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitlinearc(const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_matrix*>(fmatrix.c_ptr()), const_cast<alglib_impl::ae_matrix*>(cmatrix.c_ptr()), n, m, k, &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted nonlinear least squares fitting using function values only.

Combination of numerical differentiation and secant updates is used to
obtain function Jacobian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]).

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    DiffStep-   numerical differentiation step;
                should not be very small or large;
                large = loss of accuracy
                small = growth of round-off errors

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 18.10.2008 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewf(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const double diffstep, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatewf(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, diffstep, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted nonlinear least squares fitting using function values only.

Combination of numerical differentiation and secant updates is used to
obtain function Jacobian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]).

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    DiffStep-   numerical differentiation step;
                should not be very small or large;
                large = loss of accuracy
                small = growth of round-off errors

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 18.10.2008 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewf(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const double diffstep, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (x.rows()!=y.length()) || (x.rows()!=w.length()))
        throw ap_error("Error while calling 'lsfitcreatewf': looks like one of arguments has wrong size");
    n = x.rows();
    m = x.cols();
    k = c.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatewf(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, diffstep, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Nonlinear least squares fitting using function values only.

Combination of numerical differentiation and secant updates is used to
obtain function Jacobian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (f(c,x[0])-y[0])^2 + ... + (f(c,x[n-1])-y[n-1])^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]).

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    DiffStep-   numerical differentiation step;
                should not be very small or large;
                large = loss of accuracy
                small = growth of round-off errors

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 18.10.2008 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatef(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const double diffstep, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatef(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, diffstep, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Nonlinear least squares fitting using function values only.

Combination of numerical differentiation and secant updates is used to
obtain function Jacobian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (f(c,x[0])-y[0])^2 + ... + (f(c,x[n-1])-y[n-1])^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]).

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    DiffStep-   numerical differentiation step;
                should not be very small or large;
                large = loss of accuracy
                small = growth of round-off errors

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 18.10.2008 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatef(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const double diffstep, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (x.rows()!=y.length()))
        throw ap_error("Error while calling 'lsfitcreatef': looks like one of arguments has wrong size");
    n = x.rows();
    m = x.cols();
    k = c.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatef(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, diffstep, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted nonlinear least squares fitting using gradient only.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]) and its gradient.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    CheapFG -   boolean flag, which is:
                * True  if both function and gradient calculation complexity
                        are less than O(M^2).  An improved  algorithm  can
                        be  used  which corresponds  to  FGJ  scheme  from
                        MINLM unit.
                * False otherwise.
                        Standard Jacibian-bases  Levenberg-Marquardt  algo
                        will be used (FJ scheme).

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

See also:
    LSFitResults
    LSFitCreateFG (fitting without weights)
    LSFitCreateWFGH (fitting using Hessian)
    LSFitCreateFGH (fitting using Hessian, without weights)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewfg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const bool cheapfg, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatewfg(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, cheapfg, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted nonlinear least squares fitting using gradient only.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]) and its gradient.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    CheapFG -   boolean flag, which is:
                * True  if both function and gradient calculation complexity
                        are less than O(M^2).  An improved  algorithm  can
                        be  used  which corresponds  to  FGJ  scheme  from
                        MINLM unit.
                * False otherwise.
                        Standard Jacibian-bases  Levenberg-Marquardt  algo
                        will be used (FJ scheme).

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

See also:
    LSFitResults
    LSFitCreateFG (fitting without weights)
    LSFitCreateWFGH (fitting using Hessian)
    LSFitCreateFGH (fitting using Hessian, without weights)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewfg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const bool cheapfg, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (x.rows()!=y.length()) || (x.rows()!=w.length()))
        throw ap_error("Error while calling 'lsfitcreatewfg': looks like one of arguments has wrong size");
    n = x.rows();
    m = x.cols();
    k = c.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatewfg(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, cheapfg, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Nonlinear least squares fitting using gradient only, without individual
weights.

Nonlinear task min(F(c)) is solved, where

    F(c) = ((f(c,x[0])-y[0]))^2 + ... + ((f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]) and its gradient.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    CheapFG -   boolean flag, which is:
                * True  if both function and gradient calculation complexity
                        are less than O(M^2).  An improved  algorithm  can
                        be  used  which corresponds  to  FGJ  scheme  from
                        MINLM unit.
                * False otherwise.
                        Standard Jacibian-bases  Levenberg-Marquardt  algo
                        will be used (FJ scheme).

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatefg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const bool cheapfg, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatefg(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, cheapfg, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Nonlinear least squares fitting using gradient only, without individual
weights.

Nonlinear task min(F(c)) is solved, where

    F(c) = ((f(c,x[0])-y[0]))^2 + ... + ((f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]) and its gradient.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    CheapFG -   boolean flag, which is:
                * True  if both function and gradient calculation complexity
                        are less than O(M^2).  An improved  algorithm  can
                        be  used  which corresponds  to  FGJ  scheme  from
                        MINLM unit.
                * False otherwise.
                        Standard Jacibian-bases  Levenberg-Marquardt  algo
                        will be used (FJ scheme).

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatefg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const bool cheapfg, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (x.rows()!=y.length()))
        throw ap_error("Error while calling 'lsfitcreatefg': looks like one of arguments has wrong size");
    n = x.rows();
    m = x.cols();
    k = c.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatefg(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, cheapfg, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted nonlinear least squares fitting using gradient/Hessian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses f(c,x[i]), its gradient and its Hessian.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewfgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatewfgh(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Weighted nonlinear least squares fitting using gradient/Hessian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses f(c,x[i]), its gradient and its Hessian.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewfgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (x.rows()!=y.length()) || (x.rows()!=w.length()))
        throw ap_error("Error while calling 'lsfitcreatewfgh': looks like one of arguments has wrong size");
    n = x.rows();
    m = x.cols();
    k = c.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatewfgh(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(w.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Nonlinear least squares fitting using gradient/Hessian, without individial
weights.

Nonlinear task min(F(c)) is solved, where

    F(c) = ((f(c,x[0])-y[0]))^2 + ... + ((f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses f(c,x[i]), its gradient and its Hessian.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatefgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatefgh(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Nonlinear least squares fitting using gradient/Hessian, without individial
weights.

Nonlinear task min(F(c)) is solved, where

    F(c) = ((f(c,x[0])-y[0]))^2 + ... + ((f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses f(c,x[i]), its gradient and its Hessian.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatefgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;    
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    if( (x.rows()!=y.length()))
        throw ap_error("Error while calling 'lsfitcreatefgh': looks like one of arguments has wrong size");
    n = x.rows();
    m = x.cols();
    k = c.length();
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitcreatefgh(const_cast<alglib_impl::ae_matrix*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_vector*>(c.c_ptr()), n, m, k, const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);

        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Stopping conditions for nonlinear least squares fitting.

INPUT PARAMETERS:
    State   -   structure which stores algorithm state
    EpsF    -   stopping criterion. Algorithm stops if
                |F(k+1)-F(k)| <= EpsF*max{|F(k)|, |F(k+1)|, 1}
    EpsX    -   stopping criterion. Algorithm stops if
                |X(k+1)-X(k)| <= EpsX*(1+|X(k)|)
    MaxIts  -   stopping criterion. Algorithm stops after MaxIts iterations.
                MaxIts=0 means no stopping criterion.

NOTE

Passing EpsF=0, EpsX=0 and MaxIts=0 (simultaneously) will lead to automatic
stopping criterion selection (according to the scheme used by MINLM unit).


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitsetcond(const lsfitstate &state, const double epsf, const double epsx, const ae_int_t maxits)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitsetcond(const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), epsf, epsx, maxits, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function sets maximum step length

INPUT PARAMETERS:
    State   -   structure which stores algorithm state
    StpMax  -   maximum step length, >=0. Set StpMax to 0.0,  if you don't
                want to limit step length.

Use this subroutine when you optimize target function which contains exp()
or  other  fast  growing  functions,  and optimization algorithm makes too
large  steps  which  leads  to overflow. This function allows us to reject
steps  that  are  too  large  (and  therefore  expose  us  to the possible
overflow) without actually calculating function value at the x+stp*d.

NOTE: non-zero StpMax leads to moderate  performance  degradation  because
intermediate  step  of  preconditioned L-BFGS optimization is incompatible
with limits on step size.

  -- ALGLIB --
     Copyright 02.04.2010 by Bochkanov Sergey
*************************************************************************/
void lsfitsetstpmax(const lsfitstate &state, const double stpmax)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitsetstpmax(const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), stpmax, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function turns on/off reporting.

INPUT PARAMETERS:
    State   -   structure which stores algorithm state
    NeedXRep-   whether iteration reports are needed or not

When reports are needed, State.C (current parameters) and State.F (current
value of fitting function) are reported.


  -- ALGLIB --
     Copyright 15.08.2010 by Bochkanov Sergey
*************************************************************************/
void lsfitsetxrep(const lsfitstate &state, const bool needxrep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitsetxrep(const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), needxrep, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function provides reverse communication interface
Reverse communication interface is not documented or recommended to use.
See below for functions which provide better documented API
*************************************************************************/
bool lsfititeration(const lsfitstate &state)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        ae_bool result = alglib_impl::lsfititeration(const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<bool*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}


void lsfitfit(lsfitstate &state,
    void (*func)(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr),
    void  (*rep)(const real_1d_array &c, double func, void *ptr), 
    void *ptr)
{
    alglib_impl::ae_state _alglib_env_state;
    if( func==NULL )
        throw ap_error("ALGLIB: error in 'lsfitfit()' (func is NULL)");
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        while( alglib_impl::lsfititeration(state.c_ptr(), &_alglib_env_state) )
        {
            if( state.needf )
            {
                func(state.c, state.x, state.f, ptr);
                continue;
            }
            if( state.xupdated )
            {
                if( rep!=NULL )
                    rep(state.c, state.f, ptr);
                continue;
            }
            throw ap_error("ALGLIB: error in 'lsfitfit' (some derivatives were not provided?)");
        }
        alglib_impl::ae_state_clear(&_alglib_env_state);
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}


void lsfitfit(lsfitstate &state,
    void (*func)(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr),
    void (*grad)(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, void *ptr),
    void  (*rep)(const real_1d_array &c, double func, void *ptr), 
    void *ptr)
{
    alglib_impl::ae_state _alglib_env_state;
    if( func==NULL )
        throw ap_error("ALGLIB: error in 'lsfitfit()' (func is NULL)");
    if( grad==NULL )
        throw ap_error("ALGLIB: error in 'lsfitfit()' (grad is NULL)");
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        while( alglib_impl::lsfititeration(state.c_ptr(), &_alglib_env_state) )
        {
            if( state.needf )
            {
                func(state.c, state.x, state.f, ptr);
                continue;
            }
            if( state.needfg )
            {
                grad(state.c, state.x, state.f, state.g, ptr);
                continue;
            }
            if( state.xupdated )
            {
                if( rep!=NULL )
                    rep(state.c, state.f, ptr);
                continue;
            }
            throw ap_error("ALGLIB: error in 'lsfitfit' (some derivatives were not provided?)");
        }
        alglib_impl::ae_state_clear(&_alglib_env_state);
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}


void lsfitfit(lsfitstate &state,
    void (*func)(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr),
    void (*grad)(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, void *ptr),
    void (*hess)(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, real_2d_array &hess, void *ptr),
    void  (*rep)(const real_1d_array &c, double func, void *ptr), 
    void *ptr)
{
    alglib_impl::ae_state _alglib_env_state;
    if( func==NULL )
        throw ap_error("ALGLIB: error in 'lsfitfit()' (func is NULL)");
    if( grad==NULL )
        throw ap_error("ALGLIB: error in 'lsfitfit()' (grad is NULL)");
    if( hess==NULL )
        throw ap_error("ALGLIB: error in 'lsfitfit()' (hess is NULL)");
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        while( alglib_impl::lsfititeration(state.c_ptr(), &_alglib_env_state) )
        {
            if( state.needf )
            {
                func(state.c, state.x, state.f, ptr);
                continue;
            }
            if( state.needfg )
            {
                grad(state.c, state.x, state.f, state.g, ptr);
                continue;
            }
            if( state.needfgh )
            {
                hess(state.c, state.x, state.f, state.g, state.h, ptr);
                continue;
            }
            if( state.xupdated )
            {
                if( rep!=NULL )
                    rep(state.c, state.f, ptr);
                continue;
            }
            throw ap_error("ALGLIB: error in 'lsfitfit' (some derivatives were not provided?)");
        }
        alglib_impl::ae_state_clear(&_alglib_env_state);
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}



/*************************************************************************
Nonlinear least squares fitting results.

Called after return from LSFitFit().

INPUT PARAMETERS:
    State   -   algorithm state

OUTPUT PARAMETERS:
    Info    -   completetion code:
                    *  1    relative function improvement is no more than
                            EpsF.
                    *  2    relative step is no more than EpsX.
                    *  4    gradient norm is no more than EpsG
                    *  5    MaxIts steps was taken
                    *  7    stopping conditions are too stringent,
                            further improvement is impossible
    C       -   array[0..K-1], solution
    Rep     -   optimization report. Following fields are set:
                * Rep.TerminationType completetion code:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitresults(const lsfitstate &state, ae_int_t &info, real_1d_array &c, lsfitreport &rep)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::lsfitresults(const_cast<alglib_impl::lsfitstate*>(state.c_ptr()), &info, const_cast<alglib_impl::ae_vector*>(c.c_ptr()), const_cast<alglib_impl::lsfitreport*>(rep.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Parametric spline inteprolant: 2-dimensional curve.

You should not try to access its members directly - use PSpline2XXXXXXXX()
functions instead.
*************************************************************************/
_pspline2interpolant_owner::_pspline2interpolant_owner()
{
    p_struct = (alglib_impl::pspline2interpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::pspline2interpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_pspline2interpolant_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_pspline2interpolant_owner::_pspline2interpolant_owner(const _pspline2interpolant_owner &rhs)
{
    p_struct = (alglib_impl::pspline2interpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::pspline2interpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_pspline2interpolant_init_copy(p_struct, const_cast<alglib_impl::pspline2interpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_pspline2interpolant_owner& _pspline2interpolant_owner::operator=(const _pspline2interpolant_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_pspline2interpolant_clear(p_struct);
    if( !alglib_impl::_pspline2interpolant_init_copy(p_struct, const_cast<alglib_impl::pspline2interpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_pspline2interpolant_owner::~_pspline2interpolant_owner()
{
    alglib_impl::_pspline2interpolant_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::pspline2interpolant* _pspline2interpolant_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::pspline2interpolant* _pspline2interpolant_owner::c_ptr() const
{
    return const_cast<alglib_impl::pspline2interpolant*>(p_struct);
}
pspline2interpolant::pspline2interpolant() : _pspline2interpolant_owner() 
{
}

pspline2interpolant::pspline2interpolant(const pspline2interpolant &rhs):_pspline2interpolant_owner(rhs) 
{
}

pspline2interpolant& pspline2interpolant::operator=(const pspline2interpolant &rhs)
{
    if( this==&rhs )
        return *this;
    _pspline2interpolant_owner::operator=(rhs);
    return *this;
}

pspline2interpolant::~pspline2interpolant()
{
}


/*************************************************************************
Parametric spline inteprolant: 3-dimensional curve.

You should not try to access its members directly - use PSpline3XXXXXXXX()
functions instead.
*************************************************************************/
_pspline3interpolant_owner::_pspline3interpolant_owner()
{
    p_struct = (alglib_impl::pspline3interpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::pspline3interpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_pspline3interpolant_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_pspline3interpolant_owner::_pspline3interpolant_owner(const _pspline3interpolant_owner &rhs)
{
    p_struct = (alglib_impl::pspline3interpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::pspline3interpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_pspline3interpolant_init_copy(p_struct, const_cast<alglib_impl::pspline3interpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_pspline3interpolant_owner& _pspline3interpolant_owner::operator=(const _pspline3interpolant_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_pspline3interpolant_clear(p_struct);
    if( !alglib_impl::_pspline3interpolant_init_copy(p_struct, const_cast<alglib_impl::pspline3interpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_pspline3interpolant_owner::~_pspline3interpolant_owner()
{
    alglib_impl::_pspline3interpolant_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::pspline3interpolant* _pspline3interpolant_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::pspline3interpolant* _pspline3interpolant_owner::c_ptr() const
{
    return const_cast<alglib_impl::pspline3interpolant*>(p_struct);
}
pspline3interpolant::pspline3interpolant() : _pspline3interpolant_owner() 
{
}

pspline3interpolant::pspline3interpolant(const pspline3interpolant &rhs):_pspline3interpolant_owner(rhs) 
{
}

pspline3interpolant& pspline3interpolant::operator=(const pspline3interpolant &rhs)
{
    if( this==&rhs )
        return *this;
    _pspline3interpolant_owner::operator=(rhs);
    return *this;
}

pspline3interpolant::~pspline3interpolant()
{
}

/*************************************************************************
This function  builds  non-periodic 2-dimensional parametric spline  which
starts at (X[0],Y[0]) and ends at (X[N-1],Y[N-1]).

INPUT PARAMETERS:
    XY  -   points, array[0..N-1,0..1].
            XY[I,0:1] corresponds to the Ith point.
            Order of points is important!
    N   -   points count, N>=5 for Akima splines, N>=2 for other types  of
            splines.
    ST  -   spline type:
            * 0     Akima spline
            * 1     parabolically terminated Catmull-Rom spline (Tension=0)
            * 2     parabolically terminated cubic spline
    PT  -   parameterization type:
            * 0     uniform
            * 1     chord length
            * 2     centripetal

OUTPUT PARAMETERS:
    P   -   parametric spline interpolant


NOTES:
* this function  assumes  that  there all consequent points  are distinct.
  I.e. (x0,y0)<>(x1,y1),  (x1,y1)<>(x2,y2),  (x2,y2)<>(x3,y3)  and  so on.
  However, non-consequent points may coincide, i.e. we can  have  (x0,y0)=
  =(x2,y2).

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2build(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline2interpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2build(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, st, pt, const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  builds  non-periodic 3-dimensional parametric spline  which
starts at (X[0],Y[0],Z[0]) and ends at (X[N-1],Y[N-1],Z[N-1]).

Same as PSpline2Build() function, but for 3D, so we  won't  duplicate  its
description here.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3build(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline3interpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3build(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, st, pt, const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This  function  builds  periodic  2-dimensional  parametric  spline  which
starts at (X[0],Y[0]), goes through all points to (X[N-1],Y[N-1]) and then
back to (X[0],Y[0]).

INPUT PARAMETERS:
    XY  -   points, array[0..N-1,0..1].
            XY[I,0:1] corresponds to the Ith point.
            XY[N-1,0:1] must be different from XY[0,0:1].
            Order of points is important!
    N   -   points count, N>=3 for other types of splines.
    ST  -   spline type:
            * 1     Catmull-Rom spline (Tension=0) with cyclic boundary conditions
            * 2     cubic spline with cyclic boundary conditions
    PT  -   parameterization type:
            * 0     uniform
            * 1     chord length
            * 2     centripetal

OUTPUT PARAMETERS:
    P   -   parametric spline interpolant


NOTES:
* this function  assumes  that there all consequent points  are  distinct.
  I.e. (x0,y0)<>(x1,y1), (x1,y1)<>(x2,y2),  (x2,y2)<>(x3,y3)  and  so  on.
  However, non-consequent points may coincide, i.e. we can  have  (x0,y0)=
  =(x2,y2).
* last point of sequence is NOT equal to the first  point.  You  shouldn't
  make curve "explicitly periodic" by making them equal.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2buildperiodic(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline2interpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2buildperiodic(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, st, pt, const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This  function  builds  periodic  3-dimensional  parametric  spline  which
starts at (X[0],Y[0],Z[0]), goes through all points to (X[N-1],Y[N-1],Z[N-1])
and then back to (X[0],Y[0],Z[0]).

Same as PSpline2Build() function, but for 3D, so we  won't  duplicate  its
description here.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3buildperiodic(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline3interpolant &p)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3buildperiodic(const_cast<alglib_impl::ae_matrix*>(xy.c_ptr()), n, st, pt, const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function returns vector of parameter values correspoding to points.

I.e. for P created from (X[0],Y[0])...(X[N-1],Y[N-1]) and U=TValues(P)  we
have
    (X[0],Y[0]) = PSpline2Calc(P,U[0]),
    (X[1],Y[1]) = PSpline2Calc(P,U[1]),
    (X[2],Y[2]) = PSpline2Calc(P,U[2]),
    ...

INPUT PARAMETERS:
    P   -   parametric spline interpolant

OUTPUT PARAMETERS:
    N   -   array size
    T   -   array[0..N-1]


NOTES:
* for non-periodic splines U[0]=0, U[0]<U[1]<...<U[N-1], U[N-1]=1
* for periodic splines     U[0]=0, U[0]<U[1]<...<U[N-1], U[N-1]<1

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2parametervalues(const pspline2interpolant &p, ae_int_t &n, real_1d_array &t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2parametervalues(const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), &n, const_cast<alglib_impl::ae_vector*>(t.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function returns vector of parameter values correspoding to points.

Same as PSpline2ParameterValues(), but for 3D.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3parametervalues(const pspline3interpolant &p, ae_int_t &n, real_1d_array &t)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3parametervalues(const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), &n, const_cast<alglib_impl::ae_vector*>(t.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  calculates  the value of the parametric spline for a  given
value of parameter T

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-position
    Y   -   Y-position


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2calc(const pspline2interpolant &p, const double t, double &x, double &y)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2calc(const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), t, &x, &y, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  calculates  the value of the parametric spline for a  given
value of parameter T.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-position
    Y   -   Y-position
    Z   -   Z-position


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3calc(const pspline3interpolant &p, const double t, double &x, double &y, double &z)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3calc(const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), t, &x, &y, &z, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  calculates  tangent vector for a given value of parameter T

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X    -   X-component of tangent vector (normalized)
    Y    -   Y-component of tangent vector (normalized)

NOTE:
    X^2+Y^2 is either 1 (for non-zero tangent vector) or 0.


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2tangent(const pspline2interpolant &p, const double t, double &x, double &y)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2tangent(const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), t, &x, &y, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  calculates  tangent vector for a given value of parameter T

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X    -   X-component of tangent vector (normalized)
    Y    -   Y-component of tangent vector (normalized)
    Z    -   Z-component of tangent vector (normalized)

NOTE:
    X^2+Y^2+Z^2 is either 1 (for non-zero tangent vector) or 0.


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3tangent(const pspline3interpolant &p, const double t, double &x, double &y, double &z)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3tangent(const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), t, &x, &y, &z, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function calculates derivative, i.e. it returns (dX/dT,dY/dT).

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   X-derivative
    Y   -   Y-value
    DY  -   Y-derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2diff(const pspline2interpolant &p, const double t, double &x, double &dx, double &y, double &dy)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2diff(const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), t, &x, &dx, &y, &dy, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function calculates derivative, i.e. it returns (dX/dT,dY/dT,dZ/dT).

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   X-derivative
    Y   -   Y-value
    DY  -   Y-derivative
    Z   -   Z-value
    DZ  -   Z-derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3diff(const pspline3interpolant &p, const double t, double &x, double &dx, double &y, double &dy, double &z, double &dz)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3diff(const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), t, &x, &dx, &y, &dy, &z, &dz, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function calculates first and second derivative with respect to T.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   derivative
    D2X -   second derivative
    Y   -   Y-value
    DY  -   derivative
    D2Y -   second derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2diff2(const pspline2interpolant &p, const double t, double &x, double &dx, double &d2x, double &y, double &dy, double &d2y)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline2diff2(const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), t, &x, &dx, &d2x, &y, &dy, &d2y, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function calculates first and second derivative with respect to T.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   derivative
    D2X -   second derivative
    Y   -   Y-value
    DY  -   derivative
    D2Y -   second derivative
    Z   -   Z-value
    DZ  -   derivative
    D2Z -   second derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3diff2(const pspline3interpolant &p, const double t, double &x, double &dx, double &d2x, double &y, double &dy, double &d2y, double &z, double &dz, double &d2z)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::pspline3diff2(const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), t, &x, &dx, &d2x, &y, &dy, &d2y, &z, &dz, &d2z, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  calculates  arc length, i.e. length of  curve  between  t=a
and t=b.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    A,B -   parameter values corresponding to arc ends:
            * B>A will result in positive length returned
            * B<A will result in negative length returned

RESULT:
    length of arc starting at T=A and ending at T=B.


  -- ALGLIB PROJECT --
     Copyright 30.05.2010 by Bochkanov Sergey
*************************************************************************/
double pspline2arclength(const pspline2interpolant &p, const double a, const double b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::pspline2arclength(const_cast<alglib_impl::pspline2interpolant*>(p.c_ptr()), a, b, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This function  calculates  arc length, i.e. length of  curve  between  t=a
and t=b.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    A,B -   parameter values corresponding to arc ends:
            * B>A will result in positive length returned
            * B<A will result in negative length returned

RESULT:
    length of arc starting at T=A and ending at T=B.


  -- ALGLIB PROJECT --
     Copyright 30.05.2010 by Bochkanov Sergey
*************************************************************************/
double pspline3arclength(const pspline3interpolant &p, const double a, const double b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::pspline3arclength(const_cast<alglib_impl::pspline3interpolant*>(p.c_ptr()), a, b, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
2-dimensional spline inteprolant
*************************************************************************/
_spline2dinterpolant_owner::_spline2dinterpolant_owner()
{
    p_struct = (alglib_impl::spline2dinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::spline2dinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_spline2dinterpolant_init(p_struct, NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_spline2dinterpolant_owner::_spline2dinterpolant_owner(const _spline2dinterpolant_owner &rhs)
{
    p_struct = (alglib_impl::spline2dinterpolant*)alglib_impl::ae_malloc(sizeof(alglib_impl::spline2dinterpolant), NULL);
    if( p_struct==NULL )
        throw ap_error("ALGLIB: malloc error");
    if( !alglib_impl::_spline2dinterpolant_init_copy(p_struct, const_cast<alglib_impl::spline2dinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
}

_spline2dinterpolant_owner& _spline2dinterpolant_owner::operator=(const _spline2dinterpolant_owner &rhs)
{
    if( this==&rhs )
        return *this;
    alglib_impl::_spline2dinterpolant_clear(p_struct);
    if( !alglib_impl::_spline2dinterpolant_init_copy(p_struct, const_cast<alglib_impl::spline2dinterpolant*>(rhs.p_struct), NULL, ae_false) )
        throw ap_error("ALGLIB: malloc error");
    return *this;
}

_spline2dinterpolant_owner::~_spline2dinterpolant_owner()
{
    alglib_impl::_spline2dinterpolant_clear(p_struct);
    ae_free(p_struct);
}

alglib_impl::spline2dinterpolant* _spline2dinterpolant_owner::c_ptr()
{
    return p_struct;
}

alglib_impl::spline2dinterpolant* _spline2dinterpolant_owner::c_ptr() const
{
    return const_cast<alglib_impl::spline2dinterpolant*>(p_struct);
}
spline2dinterpolant::spline2dinterpolant() : _spline2dinterpolant_owner() 
{
}

spline2dinterpolant::spline2dinterpolant(const spline2dinterpolant &rhs):_spline2dinterpolant_owner(rhs) 
{
}

spline2dinterpolant& spline2dinterpolant::operator=(const spline2dinterpolant &rhs)
{
    if( this==&rhs )
        return *this;
    _spline2dinterpolant_owner::operator=(rhs);
    return *this;
}

spline2dinterpolant::~spline2dinterpolant()
{
}

/*************************************************************************
This subroutine builds bilinear spline coefficients table.

Input parameters:
    X   -   spline abscissas, array[0..N-1]
    Y   -   spline ordinates, array[0..M-1]
    F   -   function values, array[0..M-1,0..N-1]
    M,N -   grid size, M>=2, N>=2

Output parameters:
    C   -   spline interpolant

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dbuildbilinear(const real_1d_array &x, const real_1d_array &y, const real_2d_array &f, const ae_int_t m, const ae_int_t n, spline2dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dbuildbilinear(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_matrix*>(f.c_ptr()), m, n, const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine builds bicubic spline coefficients table.

Input parameters:
    X   -   spline abscissas, array[0..N-1]
    Y   -   spline ordinates, array[0..M-1]
    F   -   function values, array[0..M-1,0..N-1]
    M,N -   grid size, M>=2, N>=2

Output parameters:
    C   -   spline interpolant

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dbuildbicubic(const real_1d_array &x, const real_1d_array &y, const real_2d_array &f, const ae_int_t m, const ae_int_t n, spline2dinterpolant &c)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dbuildbicubic(const_cast<alglib_impl::ae_vector*>(x.c_ptr()), const_cast<alglib_impl::ae_vector*>(y.c_ptr()), const_cast<alglib_impl::ae_matrix*>(f.c_ptr()), m, n, const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine calculates the value of the bilinear or bicubic spline  at
the given point X.

Input parameters:
    C   -   coefficients table.
            Built by BuildBilinearSpline or BuildBicubicSpline.
    X, Y-   point

Result:
    S(x,y)

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
double spline2dcalc(const spline2dinterpolant &c, const double x, const double y)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        double result = alglib_impl::spline2dcalc(const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), x, y, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return *(reinterpret_cast<double*>(&result));
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine calculates the value of the bilinear or bicubic spline  at
the given point X and its derivatives.

Input parameters:
    C   -   spline interpolant.
    X, Y-   point

Output parameters:
    F   -   S(x,y)
    FX  -   dS(x,y)/dX
    FY  -   dS(x,y)/dY
    FXY -   d2S(x,y)/dXdY

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
void spline2ddiff(const spline2dinterpolant &c, const double x, const double y, double &f, double &fx, double &fy, double &fxy)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2ddiff(const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), x, y, &f, &fx, &fy, &fxy, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine unpacks two-dimensional spline into the coefficients table

Input parameters:
    C   -   spline interpolant.

Result:
    M, N-   grid size (x-axis and y-axis)
    Tbl -   coefficients table, unpacked format,
            [0..(N-1)*(M-1)-1, 0..19].
            For I = 0...M-2, J=0..N-2:
                K =  I*(N-1)+J
                Tbl[K,0] = X[j]
                Tbl[K,1] = X[j+1]
                Tbl[K,2] = Y[i]
                Tbl[K,3] = Y[i+1]
                Tbl[K,4] = C00
                Tbl[K,5] = C01
                Tbl[K,6] = C02
                Tbl[K,7] = C03
                Tbl[K,8] = C10
                Tbl[K,9] = C11
                ...
                Tbl[K,19] = C33
            On each grid square spline is equals to:
                S(x) = SUM(c[i,j]*(x^i)*(y^j), i=0..3, j=0..3)
                t = x-x[j]
                u = y-y[i]

  -- ALGLIB PROJECT --
     Copyright 29.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dunpack(const spline2dinterpolant &c, ae_int_t &m, ae_int_t &n, real_2d_array &tbl)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dunpack(const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), &m, &n, const_cast<alglib_impl::ae_matrix*>(tbl.c_ptr()), &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine performs linear transformation of the spline argument.

Input parameters:
    C       -   spline interpolant
    AX, BX  -   transformation coefficients: x = A*t + B
    AY, BY  -   transformation coefficients: y = A*u + B
Result:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dlintransxy(const spline2dinterpolant &c, const double ax, const double bx, const double ay, const double by)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dlintransxy(const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), ax, bx, ay, by, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
This subroutine performs linear transformation of the spline.

Input parameters:
    C   -   spline interpolant.
    A, B-   transformation coefficients: S2(x,y) = A*S(x,y) + B

Output parameters:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dlintransf(const spline2dinterpolant &c, const double a, const double b)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dlintransf(const_cast<alglib_impl::spline2dinterpolant*>(c.c_ptr()), a, b, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Bicubic spline resampling

Input parameters:
    A           -   function values at the old grid,
                    array[0..OldHeight-1, 0..OldWidth-1]
    OldHeight   -   old grid height, OldHeight>1
    OldWidth    -   old grid width, OldWidth>1
    NewHeight   -   new grid height, NewHeight>1
    NewWidth    -   new grid width, NewWidth>1

Output parameters:
    B           -   function values at the new grid,
                    array[0..NewHeight-1, 0..NewWidth-1]

  -- ALGLIB routine --
     15 May, 2007
     Copyright by Bochkanov Sergey
*************************************************************************/
void spline2dresamplebicubic(const real_2d_array &a, const ae_int_t oldheight, const ae_int_t oldwidth, real_2d_array &b, const ae_int_t newheight, const ae_int_t newwidth)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dresamplebicubic(const_cast<alglib_impl::ae_matrix*>(a.c_ptr()), oldheight, oldwidth, const_cast<alglib_impl::ae_matrix*>(b.c_ptr()), newheight, newwidth, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}

/*************************************************************************
Bilinear spline resampling

Input parameters:
    A           -   function values at the old grid,
                    array[0..OldHeight-1, 0..OldWidth-1]
    OldHeight   -   old grid height, OldHeight>1
    OldWidth    -   old grid width, OldWidth>1
    NewHeight   -   new grid height, NewHeight>1
    NewWidth    -   new grid width, NewWidth>1

Output parameters:
    B           -   function values at the new grid,
                    array[0..NewHeight-1, 0..NewWidth-1]

  -- ALGLIB routine --
     09.07.2007
     Copyright by Bochkanov Sergey
*************************************************************************/
void spline2dresamplebilinear(const real_2d_array &a, const ae_int_t oldheight, const ae_int_t oldwidth, real_2d_array &b, const ae_int_t newheight, const ae_int_t newwidth)
{
    alglib_impl::ae_state _alglib_env_state;
    alglib_impl::ae_state_init(&_alglib_env_state);
    try
    {
        alglib_impl::spline2dresamplebilinear(const_cast<alglib_impl::ae_matrix*>(a.c_ptr()), oldheight, oldwidth, const_cast<alglib_impl::ae_matrix*>(b.c_ptr()), newheight, newwidth, &_alglib_env_state);
        alglib_impl::ae_state_clear(&_alglib_env_state);
        return;
    }
    catch(alglib_impl::ae_error_type)
    {
        throw ap_error(_alglib_env_state.error_msg);
    }
    catch(...)
    {
        throw;
    }
}
}

/////////////////////////////////////////////////////////////////////////
//
// THIS SECTION CONTAINS IMPLEMENTATION OF COMPUTATIONAL CORE
//
/////////////////////////////////////////////////////////////////////////
namespace alglib_impl
{
static double idwint_idwqfactor = 1.5;
static ae_int_t idwint_idwkmin = 5;
static double idwint_idwcalcq(idwinterpolant* z,
     /* Real    */ ae_vector* x,
     ae_int_t k,
     ae_state *_state);
static void idwint_idwinit1(ae_int_t n,
     ae_int_t nx,
     ae_int_t d,
     ae_int_t nq,
     ae_int_t nw,
     idwinterpolant* z,
     ae_state *_state);
static void idwint_idwinternalsolver(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     /* Real    */ ae_vector* temp,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* x,
     double* taskrcond,
     ae_state *_state);


static void ratint_barycentricnormalize(barycentricinterpolant* b,
     ae_state *_state);




static void spline1d_spline1dgriddiffcubicinternal(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* d,
     /* Real    */ ae_vector* a1,
     /* Real    */ ae_vector* a2,
     /* Real    */ ae_vector* a3,
     /* Real    */ ae_vector* b,
     /* Real    */ ae_vector* dt,
     ae_state *_state);
static void spline1d_heapsortpoints(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_state *_state);
static void spline1d_heapsortppoints(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Integer */ ae_vector* p,
     ae_int_t n,
     ae_state *_state);
static void spline1d_solvetridiagonal(/* Real    */ ae_vector* a,
     /* Real    */ ae_vector* b,
     /* Real    */ ae_vector* c,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     /* Real    */ ae_vector* x,
     ae_state *_state);
static void spline1d_solvecyclictridiagonal(/* Real    */ ae_vector* a,
     /* Real    */ ae_vector* b,
     /* Real    */ ae_vector* c,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     /* Real    */ ae_vector* x,
     ae_state *_state);
static double spline1d_diffthreepoint(double t,
     double x0,
     double f0,
     double x1,
     double f1,
     double x2,
     double f2,
     ae_state *_state);


static ae_int_t lsfit_rfsmax = 10;
static void lsfit_spline1dfitinternal(ae_int_t st,
     /* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state);
static void lsfit_lsfitlinearinternal(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state);
static void lsfit_lsfitclearrequestfields(lsfitstate* state,
     ae_state *_state);
static void lsfit_barycentriccalcbasis(barycentricinterpolant* b,
     double t,
     /* Real    */ ae_vector* y,
     ae_state *_state);
static void lsfit_internalchebyshevfit(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state);
static void lsfit_barycentricfitwcfixedd(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t d,
     ae_int_t* info,
     barycentricinterpolant* b,
     barycentricfitreport* rep,
     ae_state *_state);


static void pspline_pspline2par(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t pt,
     /* Real    */ ae_vector* p,
     ae_state *_state);
static void pspline_pspline3par(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t pt,
     /* Real    */ ae_vector* p,
     ae_state *_state);


static void spline2d_bicubiccalcderivatives(/* Real    */ ae_matrix* a,
     /* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t m,
     ae_int_t n,
     /* Real    */ ae_matrix* dx,
     /* Real    */ ae_matrix* dy,
     /* Real    */ ae_matrix* dxy,
     ae_state *_state);





/*************************************************************************
IDW interpolation

INPUT PARAMETERS:
    Z   -   IDW interpolant built with one of model building
            subroutines.
    X   -   array[0..NX-1], interpolation point

Result:
    IDW interpolant Z(X)

  -- ALGLIB --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
double idwcalc(idwinterpolant* z,
     /* Real    */ ae_vector* x,
     ae_state *_state)
{
    ae_int_t nx;
    ae_int_t i;
    ae_int_t k;
    double r;
    double s;
    double w;
    double v1;
    double v2;
    double d0;
    double di;
    double result;


    
    /*
     * these initializers are not really necessary,
     * but without them compiler complains about uninitialized locals
     */
    k = 0;
    
    /*
     * Query
     */
    if( z->modeltype==0 )
    {
        
        /*
         * NQ/NW-based model
         */
        nx = z->nx;
        k = kdtreequeryknn(&z->tree, x, z->nw, ae_true, _state);
        kdtreequeryresultsdistances(&z->tree, &z->rbuf, _state);
        kdtreequeryresultstags(&z->tree, &z->tbuf, _state);
    }
    if( z->modeltype==1 )
    {
        
        /*
         * R-based model
         */
        nx = z->nx;
        k = kdtreequeryrnn(&z->tree, x, z->r, ae_true, _state);
        kdtreequeryresultsdistances(&z->tree, &z->rbuf, _state);
        kdtreequeryresultstags(&z->tree, &z->tbuf, _state);
        if( k<idwint_idwkmin )
        {
            
            /*
             * we need at least IDWKMin points
             */
            k = kdtreequeryknn(&z->tree, x, idwint_idwkmin, ae_true, _state);
            kdtreequeryresultsdistances(&z->tree, &z->rbuf, _state);
            kdtreequeryresultstags(&z->tree, &z->tbuf, _state);
        }
    }
    
    /*
     * initialize weights for linear/quadratic members calculation.
     *
     * NOTE 1: weights are calculated using NORMALIZED modified
     * Shepard's formula. Original formula gives w(i) = sqr((R-di)/(R*di)),
     * where di is i-th distance, R is max(di). Modified formula have
     * following form:
     *     w_mod(i) = 1, if di=d0
     *     w_mod(i) = w(i)/w(0), if di<>d0
     *
     * NOTE 2: self-match is USED for this query
     *
     * NOTE 3: last point almost always gain zero weight, but it MUST
     * be used for fitting because sometimes it will gain NON-ZERO
     * weight - for example, when all distances are equal.
     */
    r = z->rbuf.ptr.p_double[k-1];
    d0 = z->rbuf.ptr.p_double[0];
    result = 0;
    s = 0;
    for(i=0; i<=k-1; i++)
    {
        di = z->rbuf.ptr.p_double[i];
        if( ae_fp_eq(di,d0) )
        {
            
            /*
             * distance is equal to shortest, set it 1.0
             * without explicitly calculating (which would give
             * us same result, but 'll expose us to the risk of
             * division by zero).
             */
            w = 1;
        }
        else
        {
            
            /*
             * use normalized formula
             */
            v1 = (r-di)/(r-d0);
            v2 = d0/di;
            w = ae_sqr(v1*v2, _state);
        }
        result = result+w*idwint_idwcalcq(z, x, z->tbuf.ptr.p_int[i], _state);
        s = s+w;
    }
    result = result/s;
    return result;
}


/*************************************************************************
IDW interpolant using modified Shepard method for uniform point
distributions.

INPUT PARAMETERS:
    XY  -   X and Y values, array[0..N-1,0..NX].
            First NX columns contain X-values, last column contain
            Y-values.
    N   -   number of nodes, N>0.
    NX  -   space dimension, NX>=1.
    D   -   nodal function type, either:
            * 0     constant  model.  Just  for  demonstration only, worst
                    model ever.
            * 1     linear model, least squares fitting. Simpe  model  for
                    datasets too small for quadratic models
            * 2     quadratic  model,  least  squares  fitting. Best model
                    available (if your dataset is large enough).
            * -1    "fast"  linear  model,  use  with  caution!!!   It  is
                    significantly  faster than linear/quadratic and better
                    than constant model. But it is less robust (especially
                    in the presence of noise).
    NQ  -   number of points used to calculate  nodal  functions  (ignored
            for constant models). NQ should be LARGER than:
            * max(1.5*(1+NX),2^NX+1) for linear model,
            * max(3/4*(NX+2)*(NX+1),2^NX+1) for quadratic model.
            Values less than this threshold will be silently increased.
    NW  -   number of points used to calculate weights and to interpolate.
            Required: >=2^NX+1, values less than this  threshold  will  be
            silently increased.
            Recommended value: about 2*NQ

OUTPUT PARAMETERS:
    Z   -   IDW interpolant.
    
NOTES:
  * best results are obtained with quadratic models, worst - with constant
    models
  * when N is large, NQ and NW must be significantly smaller than  N  both
    to obtain optimal performance and to obtain optimal accuracy. In 2  or
    3-dimensional tasks NQ=15 and NW=25 are good values to start with.
  * NQ  and  NW  may  be  greater  than  N.  In  such  cases  they will be
    automatically decreased.
  * this subroutine is always succeeds (as long as correct parameters  are
    passed).
  * see  'Multivariate  Interpolation  of Large Sets of Scattered Data' by
    Robert J. Renka for more information on this algorithm.
  * this subroutine assumes that point distribution is uniform at the small
    scales.  If  it  isn't  -  for  example,  points are concentrated along
    "lines", but "lines" distribution is uniform at the larger scale - then
    you should use IDWBuildModifiedShepardR()


  -- ALGLIB PROJECT --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
void idwbuildmodifiedshepard(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t nx,
     ae_int_t d,
     ae_int_t nq,
     ae_int_t nw,
     idwinterpolant* z,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    ae_int_t k;
    ae_int_t j2;
    ae_int_t j3;
    double v;
    double r;
    double s;
    double d0;
    double di;
    double v1;
    double v2;
    ae_int_t nc;
    ae_int_t offs;
    ae_vector x;
    ae_vector qrbuf;
    ae_matrix qxybuf;
    ae_vector y;
    ae_matrix fmatrix;
    ae_vector w;
    ae_vector qsol;
    ae_vector temp;
    ae_vector tags;
    ae_int_t info;
    double taskrcond;

    ae_frame_make(_state, &_frame_block);
    _idwinterpolant_clear(z);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&qrbuf, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&qxybuf, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&fmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&qsol, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&temp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tags, 0, DT_INT, _state, ae_true);

    
    /*
     * these initializers are not really necessary,
     * but without them compiler complains about uninitialized locals
     */
    nc = 0;
    
    /*
     * assertions
     */
    ae_assert(n>0, "IDWBuildModifiedShepard: N<=0!", _state);
    ae_assert(nx>=1, "IDWBuildModifiedShepard: NX<1!", _state);
    ae_assert(d>=-1&&d<=2, "IDWBuildModifiedShepard: D<>-1 and D<>0 and D<>1 and D<>2!", _state);
    
    /*
     * Correct parameters if needed
     */
    if( d==1 )
    {
        nq = ae_maxint(nq, ae_iceil(idwint_idwqfactor*(1+nx), _state)+1, _state);
        nq = ae_maxint(nq, ae_round(ae_pow(2, nx, _state), _state)+1, _state);
    }
    if( d==2 )
    {
        nq = ae_maxint(nq, ae_iceil(idwint_idwqfactor*(nx+2)*(nx+1)/2, _state)+1, _state);
        nq = ae_maxint(nq, ae_round(ae_pow(2, nx, _state), _state)+1, _state);
    }
    nw = ae_maxint(nw, ae_round(ae_pow(2, nx, _state), _state)+1, _state);
    nq = ae_minint(nq, n, _state);
    nw = ae_minint(nw, n, _state);
    
    /*
     * primary initialization of Z
     */
    idwint_idwinit1(n, nx, d, nq, nw, z, _state);
    z->modeltype = 0;
    
    /*
     * Create KD-tree
     */
    ae_vector_set_length(&tags, n, _state);
    for(i=0; i<=n-1; i++)
    {
        tags.ptr.p_int[i] = i;
    }
    kdtreebuildtagged(xy, &tags, n, nx, 1, 2, &z->tree, _state);
    
    /*
     * build nodal functions
     */
    ae_vector_set_length(&temp, nq+1, _state);
    ae_vector_set_length(&x, nx, _state);
    ae_vector_set_length(&qrbuf, nq, _state);
    ae_matrix_set_length(&qxybuf, nq, nx+1, _state);
    if( d==-1 )
    {
        ae_vector_set_length(&w, nq, _state);
    }
    if( d==1 )
    {
        ae_vector_set_length(&y, nq, _state);
        ae_vector_set_length(&w, nq, _state);
        ae_vector_set_length(&qsol, nx, _state);
        
        /*
         * NX for linear members,
         * 1 for temporary storage
         */
        ae_matrix_set_length(&fmatrix, nq, nx+1, _state);
    }
    if( d==2 )
    {
        ae_vector_set_length(&y, nq, _state);
        ae_vector_set_length(&w, nq, _state);
        ae_vector_set_length(&qsol, nx+ae_round(nx*(nx+1)*0.5, _state), _state);
        
        /*
         * NX for linear members,
         * Round(NX*(NX+1)*0.5) for quadratic model,
         * 1 for temporary storage
         */
        ae_matrix_set_length(&fmatrix, nq, nx+ae_round(nx*(nx+1)*0.5, _state)+1, _state);
    }
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * Initialize center and function value.
         * If D=0 it is all what we need
         */
        ae_v_move(&z->q.ptr.pp_double[i][0], 1, &xy->ptr.pp_double[i][0], 1, ae_v_len(0,nx));
        if( d==0 )
        {
            continue;
        }
        
        /*
         * calculate weights for linear/quadratic members calculation.
         *
         * NOTE 1: weights are calculated using NORMALIZED modified
         * Shepard's formula. Original formula is w(i) = sqr((R-di)/(R*di)),
         * where di is i-th distance, R is max(di). Modified formula have
         * following form:
         *     w_mod(i) = 1, if di=d0
         *     w_mod(i) = w(i)/w(0), if di<>d0
         *
         * NOTE 2: self-match is NOT used for this query
         *
         * NOTE 3: last point almost always gain zero weight, but it MUST
         * be used for fitting because sometimes it will gain NON-ZERO
         * weight - for example, when all distances are equal.
         */
        ae_v_move(&x.ptr.p_double[0], 1, &xy->ptr.pp_double[i][0], 1, ae_v_len(0,nx-1));
        k = kdtreequeryknn(&z->tree, &x, nq, ae_false, _state);
        kdtreequeryresultsxy(&z->tree, &qxybuf, _state);
        kdtreequeryresultsdistances(&z->tree, &qrbuf, _state);
        r = qrbuf.ptr.p_double[k-1];
        d0 = qrbuf.ptr.p_double[0];
        for(j=0; j<=k-1; j++)
        {
            di = qrbuf.ptr.p_double[j];
            if( ae_fp_eq(di,d0) )
            {
                
                /*
                 * distance is equal to shortest, set it 1.0
                 * without explicitly calculating (which would give
                 * us same result, but 'll expose us to the risk of
                 * division by zero).
                 */
                w.ptr.p_double[j] = 1;
            }
            else
            {
                
                /*
                 * use normalized formula
                 */
                v1 = (r-di)/(r-d0);
                v2 = d0/di;
                w.ptr.p_double[j] = ae_sqr(v1*v2, _state);
            }
        }
        
        /*
         * calculate linear/quadratic members
         */
        if( d==-1 )
        {
            
            /*
             * "Fast" linear nodal function calculated using
             * inverse distance weighting
             */
            for(j=0; j<=nx-1; j++)
            {
                x.ptr.p_double[j] = 0;
            }
            s = 0;
            for(j=0; j<=k-1; j++)
            {
                
                /*
                 * calculate J-th inverse distance weighted gradient:
                 *     grad_k = (y_j-y_k)*(x_j-x_k)/sqr(norm(x_j-x_k))
                 *     grad   = sum(wk*grad_k)/sum(w_k)
                 */
                v = 0;
                for(j2=0; j2<=nx-1; j2++)
                {
                    v = v+ae_sqr(qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2], _state);
                }
                
                /*
                 * Although x_j<>x_k, sqr(norm(x_j-x_k)) may be zero due to
                 * underflow. If it is, we assume than J-th gradient is zero
                 * (i.e. don't add anything)
                 */
                if( ae_fp_neq(v,0) )
                {
                    for(j2=0; j2<=nx-1; j2++)
                    {
                        x.ptr.p_double[j2] = x.ptr.p_double[j2]+w.ptr.p_double[j]*(qxybuf.ptr.pp_double[j][nx]-xy->ptr.pp_double[i][nx])*(qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2])/v;
                    }
                }
                s = s+w.ptr.p_double[j];
            }
            for(j=0; j<=nx-1; j++)
            {
                z->q.ptr.pp_double[i][nx+1+j] = x.ptr.p_double[j]/s;
            }
        }
        else
        {
            
            /*
             * Least squares models: build
             */
            if( d==1 )
            {
                
                /*
                 * Linear nodal function calculated using
                 * least squares fitting to its neighbors
                 */
                for(j=0; j<=k-1; j++)
                {
                    for(j2=0; j2<=nx-1; j2++)
                    {
                        fmatrix.ptr.pp_double[j][j2] = qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2];
                    }
                    y.ptr.p_double[j] = qxybuf.ptr.pp_double[j][nx]-xy->ptr.pp_double[i][nx];
                }
                nc = nx;
            }
            if( d==2 )
            {
                
                /*
                 * Quadratic nodal function calculated using
                 * least squares fitting to its neighbors
                 */
                for(j=0; j<=k-1; j++)
                {
                    offs = 0;
                    for(j2=0; j2<=nx-1; j2++)
                    {
                        fmatrix.ptr.pp_double[j][offs] = qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2];
                        offs = offs+1;
                    }
                    for(j2=0; j2<=nx-1; j2++)
                    {
                        for(j3=j2; j3<=nx-1; j3++)
                        {
                            fmatrix.ptr.pp_double[j][offs] = (qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2])*(qxybuf.ptr.pp_double[j][j3]-xy->ptr.pp_double[i][j3]);
                            offs = offs+1;
                        }
                    }
                    y.ptr.p_double[j] = qxybuf.ptr.pp_double[j][nx]-xy->ptr.pp_double[i][nx];
                }
                nc = nx+ae_round(nx*(nx+1)*0.5, _state);
            }
            idwint_idwinternalsolver(&y, &w, &fmatrix, &temp, k, nc, &info, &qsol, &taskrcond, _state);
            
            /*
             * Least squares models: copy results
             */
            if( info>0 )
            {
                
                /*
                 * LLS task is solved, copy results
                 */
                z->debugworstrcond = ae_minreal(z->debugworstrcond, taskrcond, _state);
                z->debugbestrcond = ae_maxreal(z->debugbestrcond, taskrcond, _state);
                for(j=0; j<=nc-1; j++)
                {
                    z->q.ptr.pp_double[i][nx+1+j] = qsol.ptr.p_double[j];
                }
            }
            else
            {
                
                /*
                 * Solver failure, very strange, but we will use
                 * zero values to handle it.
                 */
                z->debugsolverfailures = z->debugsolverfailures+1;
                for(j=0; j<=nc-1; j++)
                {
                    z->q.ptr.pp_double[i][nx+1+j] = 0;
                }
            }
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
IDW interpolant using modified Shepard method for non-uniform datasets.

This type of model uses  constant  nodal  functions and interpolates using
all nodes which are closer than user-specified radius R. It  may  be  used
when points distribution is non-uniform at the small scale, but it  is  at
the distances as large as R.

INPUT PARAMETERS:
    XY  -   X and Y values, array[0..N-1,0..NX].
            First NX columns contain X-values, last column contain
            Y-values.
    N   -   number of nodes, N>0.
    NX  -   space dimension, NX>=1.
    R   -   radius, R>0

OUTPUT PARAMETERS:
    Z   -   IDW interpolant.

NOTES:
* if there is less than IDWKMin points within  R-ball,  algorithm  selects
  IDWKMin closest ones, so that continuity properties of  interpolant  are
  preserved even far from points.

  -- ALGLIB PROJECT --
     Copyright 11.04.2010 by Bochkanov Sergey
*************************************************************************/
void idwbuildmodifiedshepardr(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t nx,
     double r,
     idwinterpolant* z,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector tags;

    ae_frame_make(_state, &_frame_block);
    _idwinterpolant_clear(z);
    ae_vector_init(&tags, 0, DT_INT, _state, ae_true);

    
    /*
     * assertions
     */
    ae_assert(n>0, "IDWBuildModifiedShepardR: N<=0!", _state);
    ae_assert(nx>=1, "IDWBuildModifiedShepardR: NX<1!", _state);
    ae_assert(ae_fp_greater(r,0), "IDWBuildModifiedShepardR: R<=0!", _state);
    
    /*
     * primary initialization of Z
     */
    idwint_idwinit1(n, nx, 0, 0, n, z, _state);
    z->modeltype = 1;
    z->r = r;
    
    /*
     * Create KD-tree
     */
    ae_vector_set_length(&tags, n, _state);
    for(i=0; i<=n-1; i++)
    {
        tags.ptr.p_int[i] = i;
    }
    kdtreebuildtagged(xy, &tags, n, nx, 1, 2, &z->tree, _state);
    
    /*
     * build nodal functions
     */
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&z->q.ptr.pp_double[i][0], 1, &xy->ptr.pp_double[i][0], 1, ae_v_len(0,nx));
    }
    ae_frame_leave(_state);
}


/*************************************************************************
IDW model for noisy data.

This subroutine may be used to handle noisy data, i.e. data with noise  in
OUTPUT values.  It differs from IDWBuildModifiedShepard() in the following
aspects:
* nodal functions are not constrained to pass through  nodes:  Qi(xi)<>yi,
  i.e. we have fitting  instead  of  interpolation.
* weights which are used during least  squares fitting stage are all equal
  to 1.0 (independently of distance)
* "fast"-linear or constant nodal functions are not supported (either  not
  robust enough or too rigid)

This problem require far more complex tuning than interpolation  problems.
Below you can find some recommendations regarding this problem:
* focus on tuning NQ; it controls noise reduction. As for NW, you can just
  make it equal to 2*NQ.
* you can use cross-validation to determine optimal NQ.
* optimal NQ is a result of complex tradeoff  between  noise  level  (more
  noise = larger NQ required) and underlying  function  complexity  (given
  fixed N, larger NQ means smoothing of compex features in the data).  For
  example, NQ=N will reduce noise to the minimum level possible,  but  you
  will end up with just constant/linear/quadratic (depending on  D)  least
  squares model for the whole dataset.

INPUT PARAMETERS:
    XY  -   X and Y values, array[0..N-1,0..NX].
            First NX columns contain X-values, last column contain
            Y-values.
    N   -   number of nodes, N>0.
    NX  -   space dimension, NX>=1.
    D   -   nodal function degree, either:
            * 1     linear model, least squares fitting. Simpe  model  for
                    datasets too small for quadratic models (or  for  very
                    noisy problems).
            * 2     quadratic  model,  least  squares  fitting. Best model
                    available (if your dataset is large enough).
    NQ  -   number of points used to calculate nodal functions.  NQ should
            be  significantly   larger   than  1.5  times  the  number  of
            coefficients in a nodal function to overcome effects of noise:
            * larger than 1.5*(1+NX) for linear model,
            * larger than 3/4*(NX+2)*(NX+1) for quadratic model.
            Values less than this threshold will be silently increased.
    NW  -   number of points used to calculate weights and to interpolate.
            Required: >=2^NX+1, values less than this  threshold  will  be
            silently increased.
            Recommended value: about 2*NQ or larger

OUTPUT PARAMETERS:
    Z   -   IDW interpolant.

NOTES:
  * best results are obtained with quadratic models, linear models are not
    recommended to use unless you are pretty sure that it is what you want
  * this subroutine is always succeeds (as long as correct parameters  are
    passed).
  * see  'Multivariate  Interpolation  of Large Sets of Scattered Data' by
    Robert J. Renka for more information on this algorithm.


  -- ALGLIB PROJECT --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
void idwbuildnoisy(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t nx,
     ae_int_t d,
     ae_int_t nq,
     ae_int_t nw,
     idwinterpolant* z,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    ae_int_t k;
    ae_int_t j2;
    ae_int_t j3;
    double v;
    ae_int_t nc;
    ae_int_t offs;
    double taskrcond;
    ae_vector x;
    ae_vector qrbuf;
    ae_matrix qxybuf;
    ae_vector y;
    ae_vector w;
    ae_matrix fmatrix;
    ae_vector qsol;
    ae_vector tags;
    ae_vector temp;
    ae_int_t info;

    ae_frame_make(_state, &_frame_block);
    _idwinterpolant_clear(z);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&qrbuf, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&qxybuf, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&fmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&qsol, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tags, 0, DT_INT, _state, ae_true);
    ae_vector_init(&temp, 0, DT_REAL, _state, ae_true);

    
    /*
     * these initializers are not really necessary,
     * but without them compiler complains about uninitialized locals
     */
    nc = 0;
    
    /*
     * assertions
     */
    ae_assert(n>0, "IDWBuildNoisy: N<=0!", _state);
    ae_assert(nx>=1, "IDWBuildNoisy: NX<1!", _state);
    ae_assert(d>=1&&d<=2, "IDWBuildNoisy: D<>1 and D<>2!", _state);
    
    /*
     * Correct parameters if needed
     */
    if( d==1 )
    {
        nq = ae_maxint(nq, ae_iceil(idwint_idwqfactor*(1+nx), _state)+1, _state);
    }
    if( d==2 )
    {
        nq = ae_maxint(nq, ae_iceil(idwint_idwqfactor*(nx+2)*(nx+1)/2, _state)+1, _state);
    }
    nw = ae_maxint(nw, ae_round(ae_pow(2, nx, _state), _state)+1, _state);
    nq = ae_minint(nq, n, _state);
    nw = ae_minint(nw, n, _state);
    
    /*
     * primary initialization of Z
     */
    idwint_idwinit1(n, nx, d, nq, nw, z, _state);
    z->modeltype = 0;
    
    /*
     * Create KD-tree
     */
    ae_vector_set_length(&tags, n, _state);
    for(i=0; i<=n-1; i++)
    {
        tags.ptr.p_int[i] = i;
    }
    kdtreebuildtagged(xy, &tags, n, nx, 1, 2, &z->tree, _state);
    
    /*
     * build nodal functions
     * (special algorithm for noisy data is used)
     */
    ae_vector_set_length(&temp, nq+1, _state);
    ae_vector_set_length(&x, nx, _state);
    ae_vector_set_length(&qrbuf, nq, _state);
    ae_matrix_set_length(&qxybuf, nq, nx+1, _state);
    if( d==1 )
    {
        ae_vector_set_length(&y, nq, _state);
        ae_vector_set_length(&w, nq, _state);
        ae_vector_set_length(&qsol, 1+nx, _state);
        
        /*
         * 1 for constant member,
         * NX for linear members,
         * 1 for temporary storage
         */
        ae_matrix_set_length(&fmatrix, nq, 1+nx+1, _state);
    }
    if( d==2 )
    {
        ae_vector_set_length(&y, nq, _state);
        ae_vector_set_length(&w, nq, _state);
        ae_vector_set_length(&qsol, 1+nx+ae_round(nx*(nx+1)*0.5, _state), _state);
        
        /*
         * 1 for constant member,
         * NX for linear members,
         * Round(NX*(NX+1)*0.5) for quadratic model,
         * 1 for temporary storage
         */
        ae_matrix_set_length(&fmatrix, nq, 1+nx+ae_round(nx*(nx+1)*0.5, _state)+1, _state);
    }
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * Initialize center.
         */
        ae_v_move(&z->q.ptr.pp_double[i][0], 1, &xy->ptr.pp_double[i][0], 1, ae_v_len(0,nx-1));
        
        /*
         * Calculate linear/quadratic members
         * using least squares fit
         * NOTE 1: all weight are equal to 1.0
         * NOTE 2: self-match is USED for this query
         */
        ae_v_move(&x.ptr.p_double[0], 1, &xy->ptr.pp_double[i][0], 1, ae_v_len(0,nx-1));
        k = kdtreequeryknn(&z->tree, &x, nq, ae_true, _state);
        kdtreequeryresultsxy(&z->tree, &qxybuf, _state);
        kdtreequeryresultsdistances(&z->tree, &qrbuf, _state);
        if( d==1 )
        {
            
            /*
             * Linear nodal function calculated using
             * least squares fitting to its neighbors
             */
            for(j=0; j<=k-1; j++)
            {
                fmatrix.ptr.pp_double[j][0] = 1.0;
                for(j2=0; j2<=nx-1; j2++)
                {
                    fmatrix.ptr.pp_double[j][1+j2] = qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2];
                }
                y.ptr.p_double[j] = qxybuf.ptr.pp_double[j][nx];
                w.ptr.p_double[j] = 1;
            }
            nc = 1+nx;
        }
        if( d==2 )
        {
            
            /*
             * Quadratic nodal function calculated using
             * least squares fitting to its neighbors
             */
            for(j=0; j<=k-1; j++)
            {
                fmatrix.ptr.pp_double[j][0] = 1;
                offs = 1;
                for(j2=0; j2<=nx-1; j2++)
                {
                    fmatrix.ptr.pp_double[j][offs] = qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2];
                    offs = offs+1;
                }
                for(j2=0; j2<=nx-1; j2++)
                {
                    for(j3=j2; j3<=nx-1; j3++)
                    {
                        fmatrix.ptr.pp_double[j][offs] = (qxybuf.ptr.pp_double[j][j2]-xy->ptr.pp_double[i][j2])*(qxybuf.ptr.pp_double[j][j3]-xy->ptr.pp_double[i][j3]);
                        offs = offs+1;
                    }
                }
                y.ptr.p_double[j] = qxybuf.ptr.pp_double[j][nx];
                w.ptr.p_double[j] = 1;
            }
            nc = 1+nx+ae_round(nx*(nx+1)*0.5, _state);
        }
        idwint_idwinternalsolver(&y, &w, &fmatrix, &temp, k, nc, &info, &qsol, &taskrcond, _state);
        
        /*
         * Least squares models: copy results
         */
        if( info>0 )
        {
            
            /*
             * LLS task is solved, copy results
             */
            z->debugworstrcond = ae_minreal(z->debugworstrcond, taskrcond, _state);
            z->debugbestrcond = ae_maxreal(z->debugbestrcond, taskrcond, _state);
            for(j=0; j<=nc-1; j++)
            {
                z->q.ptr.pp_double[i][nx+j] = qsol.ptr.p_double[j];
            }
        }
        else
        {
            
            /*
             * Solver failure, very strange, but we will use
             * zero values to handle it.
             */
            z->debugsolverfailures = z->debugsolverfailures+1;
            v = 0;
            for(j=0; j<=k-1; j++)
            {
                v = v+qxybuf.ptr.pp_double[j][nx];
            }
            z->q.ptr.pp_double[i][nx] = v/k;
            for(j=0; j<=nc-2; j++)
            {
                z->q.ptr.pp_double[i][nx+1+j] = 0;
            }
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Internal subroutine: K-th nodal function calculation

  -- ALGLIB --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
static double idwint_idwcalcq(idwinterpolant* z,
     /* Real    */ ae_vector* x,
     ae_int_t k,
     ae_state *_state)
{
    ae_int_t nx;
    ae_int_t i;
    ae_int_t j;
    ae_int_t offs;
    double result;


    nx = z->nx;
    
    /*
     * constant member
     */
    result = z->q.ptr.pp_double[k][nx];
    
    /*
     * linear members
     */
    if( z->d>=1 )
    {
        for(i=0; i<=nx-1; i++)
        {
            result = result+z->q.ptr.pp_double[k][nx+1+i]*(x->ptr.p_double[i]-z->q.ptr.pp_double[k][i]);
        }
    }
    
    /*
     * quadratic members
     */
    if( z->d>=2 )
    {
        offs = nx+1+nx;
        for(i=0; i<=nx-1; i++)
        {
            for(j=i; j<=nx-1; j++)
            {
                result = result+z->q.ptr.pp_double[k][offs]*(x->ptr.p_double[i]-z->q.ptr.pp_double[k][i])*(x->ptr.p_double[j]-z->q.ptr.pp_double[k][j]);
                offs = offs+1;
            }
        }
    }
    return result;
}


/*************************************************************************
Initialization of internal structures.

It assumes correctness of all parameters.

  -- ALGLIB --
     Copyright 02.03.2010 by Bochkanov Sergey
*************************************************************************/
static void idwint_idwinit1(ae_int_t n,
     ae_int_t nx,
     ae_int_t d,
     ae_int_t nq,
     ae_int_t nw,
     idwinterpolant* z,
     ae_state *_state)
{


    z->debugsolverfailures = 0;
    z->debugworstrcond = 1.0;
    z->debugbestrcond = 0;
    z->n = n;
    z->nx = nx;
    z->d = 0;
    if( d==1 )
    {
        z->d = 1;
    }
    if( d==2 )
    {
        z->d = 2;
    }
    if( d==-1 )
    {
        z->d = 1;
    }
    z->nw = nw;
    if( d==-1 )
    {
        ae_matrix_set_length(&z->q, n, nx+1+nx, _state);
    }
    if( d==0 )
    {
        ae_matrix_set_length(&z->q, n, nx+1, _state);
    }
    if( d==1 )
    {
        ae_matrix_set_length(&z->q, n, nx+1+nx, _state);
    }
    if( d==2 )
    {
        ae_matrix_set_length(&z->q, n, nx+1+nx+ae_round(nx*(nx+1)*0.5, _state), _state);
    }
    ae_vector_set_length(&z->tbuf, nw, _state);
    ae_vector_set_length(&z->rbuf, nw, _state);
    ae_matrix_set_length(&z->xybuf, nw, nx+1, _state);
    ae_vector_set_length(&z->xbuf, nx, _state);
}


/*************************************************************************
Linear least squares solver for small tasks.

Works faster than standard ALGLIB solver in non-degenerate cases  (due  to
absense of internal allocations and optimized row/colums).  In  degenerate
cases it calls standard solver, which results in small performance penalty
associated with preliminary steps.

INPUT PARAMETERS:
    Y           array[0..N-1]
    W           array[0..N-1]
    FMatrix     array[0..N-1,0..M], have additional column for temporary
                values
    Temp        array[0..N]
*************************************************************************/
static void idwint_idwinternalsolver(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     /* Real    */ ae_vector* temp,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* x,
     double* taskrcond,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    double v;
    double tau;
    ae_vector b;
    densesolverlsreport srep;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    _densesolverlsreport_init(&srep, _state, ae_true);

    
    /*
     * set up info
     */
    *info = 1;
    
    /*
     * prepare matrix
     */
    for(i=0; i<=n-1; i++)
    {
        fmatrix->ptr.pp_double[i][m] = y->ptr.p_double[i];
        v = w->ptr.p_double[i];
        ae_v_muld(&fmatrix->ptr.pp_double[i][0], 1, ae_v_len(0,m), v);
    }
    
    /*
     * use either fast algorithm or general algorithm
     */
    if( m<=n )
    {
        
        /*
         * QR decomposition
         * We assume that M<=N (we would have called LSFit() otherwise)
         */
        for(i=0; i<=m-1; i++)
        {
            if( i<n-1 )
            {
                ae_v_move(&temp->ptr.p_double[1], 1, &fmatrix->ptr.pp_double[i][i], fmatrix->stride, ae_v_len(1,n-i));
                generatereflection(temp, n-i, &tau, _state);
                fmatrix->ptr.pp_double[i][i] = temp->ptr.p_double[1];
                temp->ptr.p_double[1] = 1;
                for(j=i+1; j<=m; j++)
                {
                    v = ae_v_dotproduct(&fmatrix->ptr.pp_double[i][j], fmatrix->stride, &temp->ptr.p_double[1], 1, ae_v_len(i,n-1));
                    v = tau*v;
                    ae_v_subd(&fmatrix->ptr.pp_double[i][j], fmatrix->stride, &temp->ptr.p_double[1], 1, ae_v_len(i,n-1), v);
                }
            }
        }
        
        /*
         * Check condition number
         */
        *taskrcond = rmatrixtrrcondinf(fmatrix, m, ae_true, ae_false, _state);
        
        /*
         * use either fast algorithm for non-degenerate cases
         * or slow algorithm for degenerate cases
         */
        if( ae_fp_greater(*taskrcond,10000*n*ae_machineepsilon) )
        {
            
            /*
             * solve triangular system R*x = FMatrix[0:M-1,M]
             * using fast algorithm, then exit
             */
            x->ptr.p_double[m-1] = fmatrix->ptr.pp_double[m-1][m]/fmatrix->ptr.pp_double[m-1][m-1];
            for(i=m-2; i>=0; i--)
            {
                v = ae_v_dotproduct(&fmatrix->ptr.pp_double[i][i+1], 1, &x->ptr.p_double[i+1], 1, ae_v_len(i+1,m-1));
                x->ptr.p_double[i] = (fmatrix->ptr.pp_double[i][m]-v)/fmatrix->ptr.pp_double[i][i];
            }
        }
        else
        {
            
            /*
             * use more general algorithm
             */
            ae_vector_set_length(&b, m, _state);
            for(i=0; i<=m-1; i++)
            {
                for(j=0; j<=i-1; j++)
                {
                    fmatrix->ptr.pp_double[i][j] = 0.0;
                }
                b.ptr.p_double[i] = fmatrix->ptr.pp_double[i][m];
            }
            rmatrixsolvels(fmatrix, m, m, &b, 10000*ae_machineepsilon, info, &srep, x, _state);
        }
    }
    else
    {
        
        /*
         * use more general algorithm
         */
        ae_vector_set_length(&b, n, _state);
        for(i=0; i<=n-1; i++)
        {
            b.ptr.p_double[i] = fmatrix->ptr.pp_double[i][m];
        }
        rmatrixsolvels(fmatrix, n, m, &b, 10000*ae_machineepsilon, info, &srep, x, _state);
        *taskrcond = srep.r2;
    }
    ae_frame_leave(_state);
}


ae_bool _idwinterpolant_init(idwinterpolant* p, ae_state *_state, ae_bool make_automatic)
{
    if( !_kdtree_init(&p->tree, _state, make_automatic) )
        return ae_false;
    if( !ae_matrix_init(&p->q, 0, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->xbuf, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->tbuf, 0, DT_INT, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->rbuf, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_matrix_init(&p->xybuf, 0, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _idwinterpolant_init_copy(idwinterpolant* dst, idwinterpolant* src, ae_state *_state, ae_bool make_automatic)
{
    dst->n = src->n;
    dst->nx = src->nx;
    dst->d = src->d;
    dst->r = src->r;
    dst->nw = src->nw;
    if( !_kdtree_init_copy(&dst->tree, &src->tree, _state, make_automatic) )
        return ae_false;
    dst->modeltype = src->modeltype;
    if( !ae_matrix_init_copy(&dst->q, &src->q, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->xbuf, &src->xbuf, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->tbuf, &src->tbuf, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->rbuf, &src->rbuf, _state, make_automatic) )
        return ae_false;
    if( !ae_matrix_init_copy(&dst->xybuf, &src->xybuf, _state, make_automatic) )
        return ae_false;
    dst->debugsolverfailures = src->debugsolverfailures;
    dst->debugworstrcond = src->debugworstrcond;
    dst->debugbestrcond = src->debugbestrcond;
    return ae_true;
}


void _idwinterpolant_clear(idwinterpolant* p)
{
    _kdtree_clear(&p->tree);
    ae_matrix_clear(&p->q);
    ae_vector_clear(&p->xbuf);
    ae_vector_clear(&p->tbuf);
    ae_vector_clear(&p->rbuf);
    ae_matrix_clear(&p->xybuf);
}




/*************************************************************************
Rational interpolation using barycentric formula

F(t) = SUM(i=0,n-1,w[i]*f[i]/(t-x[i])) / SUM(i=0,n-1,w[i]/(t-x[i]))

Input parameters:
    B   -   barycentric interpolant built with one of model building
            subroutines.
    T   -   interpolation point

Result:
    barycentric interpolant F(t)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
double barycentriccalc(barycentricinterpolant* b,
     double t,
     ae_state *_state)
{
    double s1;
    double s2;
    double s;
    double v;
    ae_int_t i;
    double result;


    ae_assert(!ae_isinf(t, _state), "BarycentricCalc: infinite T!", _state);
    
    /*
     * special case: NaN
     */
    if( ae_isnan(t, _state) )
    {
        result = _state->v_nan;
        return result;
    }
    
    /*
     * special case: N=1
     */
    if( b->n==1 )
    {
        result = b->sy*b->y.ptr.p_double[0];
        return result;
    }
    
    /*
     * Here we assume that task is normalized, i.e.:
     * 1. abs(Y[i])<=1
     * 2. abs(W[i])<=1
     * 3. X[] is ordered
     */
    s = ae_fabs(t-b->x.ptr.p_double[0], _state);
    for(i=0; i<=b->n-1; i++)
    {
        v = b->x.ptr.p_double[i];
        if( ae_fp_eq(v,t) )
        {
            result = b->sy*b->y.ptr.p_double[i];
            return result;
        }
        v = ae_fabs(t-v, _state);
        if( ae_fp_less(v,s) )
        {
            s = v;
        }
    }
    s1 = 0;
    s2 = 0;
    for(i=0; i<=b->n-1; i++)
    {
        v = s/(t-b->x.ptr.p_double[i]);
        v = v*b->w.ptr.p_double[i];
        s1 = s1+v*b->y.ptr.p_double[i];
        s2 = s2+v;
    }
    result = b->sy*s1/s2;
    return result;
}


/*************************************************************************
Differentiation of barycentric interpolant: first derivative.

Algorithm used in this subroutine is very robust and should not fail until
provided with values too close to MaxRealNumber  (usually  MaxRealNumber/N
or greater will overflow).

INPUT PARAMETERS:
    B   -   barycentric interpolant built with one of model building
            subroutines.
    T   -   interpolation point

OUTPUT PARAMETERS:
    F   -   barycentric interpolant at T
    DF  -   first derivative
    
NOTE


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricdiff1(barycentricinterpolant* b,
     double t,
     double* f,
     double* df,
     ae_state *_state)
{
    double v;
    double vv;
    ae_int_t i;
    ae_int_t k;
    double n0;
    double n1;
    double d0;
    double d1;
    double s0;
    double s1;
    double xk;
    double xi;
    double xmin;
    double xmax;
    double xscale1;
    double xoffs1;
    double xscale2;
    double xoffs2;
    double xprev;

    *f = 0;
    *df = 0;

    ae_assert(!ae_isinf(t, _state), "BarycentricDiff1: infinite T!", _state);
    
    /*
     * special case: NaN
     */
    if( ae_isnan(t, _state) )
    {
        *f = _state->v_nan;
        *df = _state->v_nan;
        return;
    }
    
    /*
     * special case: N=1
     */
    if( b->n==1 )
    {
        *f = b->sy*b->y.ptr.p_double[0];
        *df = 0;
        return;
    }
    if( ae_fp_eq(b->sy,0) )
    {
        *f = 0;
        *df = 0;
        return;
    }
    ae_assert(ae_fp_greater(b->sy,0), "BarycentricDiff1: internal error", _state);
    
    /*
     * We assume than N>1 and B.SY>0. Find:
     * 1. pivot point (X[i] closest to T)
     * 2. width of interval containing X[i]
     */
    v = ae_fabs(b->x.ptr.p_double[0]-t, _state);
    k = 0;
    xmin = b->x.ptr.p_double[0];
    xmax = b->x.ptr.p_double[0];
    for(i=1; i<=b->n-1; i++)
    {
        vv = b->x.ptr.p_double[i];
        if( ae_fp_less(ae_fabs(vv-t, _state),v) )
        {
            v = ae_fabs(vv-t, _state);
            k = i;
        }
        xmin = ae_minreal(xmin, vv, _state);
        xmax = ae_maxreal(xmax, vv, _state);
    }
    
    /*
     * pivot point found, calculate dNumerator and dDenominator
     */
    xscale1 = 1/(xmax-xmin);
    xoffs1 = -xmin/(xmax-xmin)+1;
    xscale2 = 2;
    xoffs2 = -3;
    t = t*xscale1+xoffs1;
    t = t*xscale2+xoffs2;
    xk = b->x.ptr.p_double[k];
    xk = xk*xscale1+xoffs1;
    xk = xk*xscale2+xoffs2;
    v = t-xk;
    n0 = 0;
    n1 = 0;
    d0 = 0;
    d1 = 0;
    xprev = -2;
    for(i=0; i<=b->n-1; i++)
    {
        xi = b->x.ptr.p_double[i];
        xi = xi*xscale1+xoffs1;
        xi = xi*xscale2+xoffs2;
        ae_assert(ae_fp_greater(xi,xprev), "BarycentricDiff1: points are too close!", _state);
        xprev = xi;
        if( i!=k )
        {
            vv = ae_sqr(t-xi, _state);
            s0 = (t-xk)/(t-xi);
            s1 = (xk-xi)/vv;
        }
        else
        {
            s0 = 1;
            s1 = 0;
        }
        vv = b->w.ptr.p_double[i]*b->y.ptr.p_double[i];
        n0 = n0+s0*vv;
        n1 = n1+s1*vv;
        vv = b->w.ptr.p_double[i];
        d0 = d0+s0*vv;
        d1 = d1+s1*vv;
    }
    *f = b->sy*n0/d0;
    *df = (n1*d0-n0*d1)/ae_sqr(d0, _state);
    if( ae_fp_neq(*df,0) )
    {
        *df = ae_sign(*df, _state)*ae_exp(ae_log(ae_fabs(*df, _state), _state)+ae_log(b->sy, _state)+ae_log(xscale1, _state)+ae_log(xscale2, _state), _state);
    }
}


/*************************************************************************
Differentiation of barycentric interpolant: first/second derivatives.

INPUT PARAMETERS:
    B   -   barycentric interpolant built with one of model building
            subroutines.
    T   -   interpolation point

OUTPUT PARAMETERS:
    F   -   barycentric interpolant at T
    DF  -   first derivative
    D2F -   second derivative

NOTE: this algorithm may fail due to overflow/underflor if  used  on  data
whose values are close to MaxRealNumber or MinRealNumber.  Use more robust
BarycentricDiff1() subroutine in such cases.


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricdiff2(barycentricinterpolant* b,
     double t,
     double* f,
     double* df,
     double* d2f,
     ae_state *_state)
{
    double v;
    double vv;
    ae_int_t i;
    ae_int_t k;
    double n0;
    double n1;
    double n2;
    double d0;
    double d1;
    double d2;
    double s0;
    double s1;
    double s2;
    double xk;
    double xi;

    *f = 0;
    *df = 0;
    *d2f = 0;

    ae_assert(!ae_isinf(t, _state), "BarycentricDiff1: infinite T!", _state);
    
    /*
     * special case: NaN
     */
    if( ae_isnan(t, _state) )
    {
        *f = _state->v_nan;
        *df = _state->v_nan;
        *d2f = _state->v_nan;
        return;
    }
    
    /*
     * special case: N=1
     */
    if( b->n==1 )
    {
        *f = b->sy*b->y.ptr.p_double[0];
        *df = 0;
        *d2f = 0;
        return;
    }
    if( ae_fp_eq(b->sy,0) )
    {
        *f = 0;
        *df = 0;
        *d2f = 0;
        return;
    }
    
    /*
     * We assume than N>1 and B.SY>0. Find:
     * 1. pivot point (X[i] closest to T)
     * 2. width of interval containing X[i]
     */
    ae_assert(ae_fp_greater(b->sy,0), "BarycentricDiff: internal error", _state);
    *f = 0;
    *df = 0;
    *d2f = 0;
    v = ae_fabs(b->x.ptr.p_double[0]-t, _state);
    k = 0;
    for(i=1; i<=b->n-1; i++)
    {
        vv = b->x.ptr.p_double[i];
        if( ae_fp_less(ae_fabs(vv-t, _state),v) )
        {
            v = ae_fabs(vv-t, _state);
            k = i;
        }
    }
    
    /*
     * pivot point found, calculate dNumerator and dDenominator
     */
    xk = b->x.ptr.p_double[k];
    v = t-xk;
    n0 = 0;
    n1 = 0;
    n2 = 0;
    d0 = 0;
    d1 = 0;
    d2 = 0;
    for(i=0; i<=b->n-1; i++)
    {
        if( i!=k )
        {
            xi = b->x.ptr.p_double[i];
            vv = ae_sqr(t-xi, _state);
            s0 = (t-xk)/(t-xi);
            s1 = (xk-xi)/vv;
            s2 = -2*(xk-xi)/(vv*(t-xi));
        }
        else
        {
            s0 = 1;
            s1 = 0;
            s2 = 0;
        }
        vv = b->w.ptr.p_double[i]*b->y.ptr.p_double[i];
        n0 = n0+s0*vv;
        n1 = n1+s1*vv;
        n2 = n2+s2*vv;
        vv = b->w.ptr.p_double[i];
        d0 = d0+s0*vv;
        d1 = d1+s1*vv;
        d2 = d2+s2*vv;
    }
    *f = b->sy*n0/d0;
    *df = b->sy*(n1*d0-n0*d1)/ae_sqr(d0, _state);
    *d2f = b->sy*((n2*d0-n0*d2)*ae_sqr(d0, _state)-(n1*d0-n0*d1)*2*d0*d1)/ae_sqr(ae_sqr(d0, _state), _state);
}


/*************************************************************************
This subroutine performs linear transformation of the argument.

INPUT PARAMETERS:
    B       -   rational interpolant in barycentric form
    CA, CB  -   transformation coefficients: x = CA*t + CB

OUTPUT PARAMETERS:
    B       -   transformed interpolant with X replaced by T

  -- ALGLIB PROJECT --
     Copyright 19.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentriclintransx(barycentricinterpolant* b,
     double ca,
     double cb,
     ae_state *_state)
{
    ae_int_t i;
    ae_int_t j;
    double v;


    
    /*
     * special case, replace by constant F(CB)
     */
    if( ae_fp_eq(ca,0) )
    {
        b->sy = barycentriccalc(b, cb, _state);
        v = 1;
        for(i=0; i<=b->n-1; i++)
        {
            b->y.ptr.p_double[i] = 1;
            b->w.ptr.p_double[i] = v;
            v = -v;
        }
        return;
    }
    
    /*
     * general case: CA<>0
     */
    for(i=0; i<=b->n-1; i++)
    {
        b->x.ptr.p_double[i] = (b->x.ptr.p_double[i]-cb)/ca;
    }
    if( ae_fp_less(ca,0) )
    {
        for(i=0; i<=b->n-1; i++)
        {
            if( i<b->n-1-i )
            {
                j = b->n-1-i;
                v = b->x.ptr.p_double[i];
                b->x.ptr.p_double[i] = b->x.ptr.p_double[j];
                b->x.ptr.p_double[j] = v;
                v = b->y.ptr.p_double[i];
                b->y.ptr.p_double[i] = b->y.ptr.p_double[j];
                b->y.ptr.p_double[j] = v;
                v = b->w.ptr.p_double[i];
                b->w.ptr.p_double[i] = b->w.ptr.p_double[j];
                b->w.ptr.p_double[j] = v;
            }
            else
            {
                break;
            }
        }
    }
}


/*************************************************************************
This  subroutine   performs   linear  transformation  of  the  barycentric
interpolant.

INPUT PARAMETERS:
    B       -   rational interpolant in barycentric form
    CA, CB  -   transformation coefficients: B2(x) = CA*B(x) + CB

OUTPUT PARAMETERS:
    B       -   transformed interpolant

  -- ALGLIB PROJECT --
     Copyright 19.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentriclintransy(barycentricinterpolant* b,
     double ca,
     double cb,
     ae_state *_state)
{
    ae_int_t i;
    double v;


    for(i=0; i<=b->n-1; i++)
    {
        b->y.ptr.p_double[i] = ca*b->sy*b->y.ptr.p_double[i]+cb;
    }
    b->sy = 0;
    for(i=0; i<=b->n-1; i++)
    {
        b->sy = ae_maxreal(b->sy, ae_fabs(b->y.ptr.p_double[i], _state), _state);
    }
    if( ae_fp_greater(b->sy,0) )
    {
        v = 1/b->sy;
        ae_v_muld(&b->y.ptr.p_double[0], 1, ae_v_len(0,b->n-1), v);
    }
}


/*************************************************************************
Extracts X/Y/W arrays from rational interpolant

INPUT PARAMETERS:
    B   -   barycentric interpolant

OUTPUT PARAMETERS:
    N   -   nodes count, N>0
    X   -   interpolation nodes, array[0..N-1]
    F   -   function values, array[0..N-1]
    W   -   barycentric weights, array[0..N-1]

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricunpack(barycentricinterpolant* b,
     ae_int_t* n,
     /* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_state *_state)
{
    double v;

    *n = 0;
    ae_vector_clear(x);
    ae_vector_clear(y);
    ae_vector_clear(w);

    *n = b->n;
    ae_vector_set_length(x, *n, _state);
    ae_vector_set_length(y, *n, _state);
    ae_vector_set_length(w, *n, _state);
    v = b->sy;
    ae_v_move(&x->ptr.p_double[0], 1, &b->x.ptr.p_double[0], 1, ae_v_len(0,*n-1));
    ae_v_moved(&y->ptr.p_double[0], 1, &b->y.ptr.p_double[0], 1, ae_v_len(0,*n-1), v);
    ae_v_move(&w->ptr.p_double[0], 1, &b->w.ptr.p_double[0], 1, ae_v_len(0,*n-1));
}


/*************************************************************************
Rational interpolant from X/Y/W arrays

F(t) = SUM(i=0,n-1,w[i]*f[i]/(t-x[i])) / SUM(i=0,n-1,w[i]/(t-x[i]))

INPUT PARAMETERS:
    X   -   interpolation nodes, array[0..N-1]
    F   -   function values, array[0..N-1]
    W   -   barycentric weights, array[0..N-1]
    N   -   nodes count, N>0

OUTPUT PARAMETERS:
    B   -   barycentric interpolant built from (X, Y, W)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricbuildxyw(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     barycentricinterpolant* b,
     ae_state *_state)
{

    _barycentricinterpolant_clear(b);

    ae_assert(n>0, "BarycentricBuildXYW: incorrect N!", _state);
    
    /*
     * fill X/Y/W
     */
    ae_vector_set_length(&b->x, n, _state);
    ae_vector_set_length(&b->y, n, _state);
    ae_vector_set_length(&b->w, n, _state);
    ae_v_move(&b->x.ptr.p_double[0], 1, &x->ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_v_move(&b->y.ptr.p_double[0], 1, &y->ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_v_move(&b->w.ptr.p_double[0], 1, &w->ptr.p_double[0], 1, ae_v_len(0,n-1));
    b->n = n;
    
    /*
     * Normalize
     */
    ratint_barycentricnormalize(b, _state);
}


/*************************************************************************
Rational interpolant without poles

The subroutine constructs the rational interpolating function without real
poles  (see  'Barycentric rational interpolation with no  poles  and  high
rates of approximation', Michael S. Floater. and  Kai  Hormann,  for  more
information on this subject).

Input parameters:
    X   -   interpolation nodes, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of nodes, N>0.
    D   -   order of the interpolation scheme, 0 <= D <= N-1.
            D<0 will cause an error.
            D>=N it will be replaced with D=N-1.
            if you don't know what D to choose, use small value about 3-5.

Output parameters:
    B   -   barycentric interpolant.

Note:
    this algorithm always succeeds and calculates the weights  with  close
    to machine precision.

  -- ALGLIB PROJECT --
     Copyright 17.06.2007 by Bochkanov Sergey
*************************************************************************/
void barycentricbuildfloaterhormann(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t d,
     barycentricinterpolant* b,
     ae_state *_state)
{
    ae_frame _frame_block;
    double s0;
    double s;
    double v;
    ae_int_t i;
    ae_int_t j;
    ae_int_t k;
    ae_vector perm;
    ae_vector wtemp;
    ae_vector sortrbuf;
    ae_vector sortrbuf2;

    ae_frame_make(_state, &_frame_block);
    _barycentricinterpolant_clear(b);
    ae_vector_init(&perm, 0, DT_INT, _state, ae_true);
    ae_vector_init(&wtemp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sortrbuf, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sortrbuf2, 0, DT_REAL, _state, ae_true);

    ae_assert(n>0, "BarycentricFloaterHormann: N<=0!", _state);
    ae_assert(d>=0, "BarycentricFloaterHormann: incorrect D!", _state);
    
    /*
     * Prepare
     */
    if( d>n-1 )
    {
        d = n-1;
    }
    b->n = n;
    
    /*
     * special case: N=1
     */
    if( n==1 )
    {
        ae_vector_set_length(&b->x, n, _state);
        ae_vector_set_length(&b->y, n, _state);
        ae_vector_set_length(&b->w, n, _state);
        b->x.ptr.p_double[0] = x->ptr.p_double[0];
        b->y.ptr.p_double[0] = y->ptr.p_double[0];
        b->w.ptr.p_double[0] = 1;
        ratint_barycentricnormalize(b, _state);
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * Fill X/Y
     */
    ae_vector_set_length(&b->x, n, _state);
    ae_vector_set_length(&b->y, n, _state);
    ae_v_move(&b->x.ptr.p_double[0], 1, &x->ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_v_move(&b->y.ptr.p_double[0], 1, &y->ptr.p_double[0], 1, ae_v_len(0,n-1));
    tagsortfastr(&b->x, &b->y, &sortrbuf, &sortrbuf2, n, _state);
    
    /*
     * Calculate Wk
     */
    ae_vector_set_length(&b->w, n, _state);
    s0 = 1;
    for(k=1; k<=d; k++)
    {
        s0 = -s0;
    }
    for(k=0; k<=n-1; k++)
    {
        
        /*
         * Wk
         */
        s = 0;
        for(i=ae_maxint(k-d, 0, _state); i<=ae_minint(k, n-1-d, _state); i++)
        {
            v = 1;
            for(j=i; j<=i+d; j++)
            {
                if( j!=k )
                {
                    v = v/ae_fabs(b->x.ptr.p_double[k]-b->x.ptr.p_double[j], _state);
                }
            }
            s = s+v;
        }
        b->w.ptr.p_double[k] = s0*s;
        
        /*
         * Next S0
         */
        s0 = -s0;
    }
    
    /*
     * Normalize
     */
    ratint_barycentricnormalize(b, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Copying of the barycentric interpolant (for internal use only)

INPUT PARAMETERS:
    B   -   barycentric interpolant

OUTPUT PARAMETERS:
    B2  -   copy(B1)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentriccopy(barycentricinterpolant* b,
     barycentricinterpolant* b2,
     ae_state *_state)
{

    _barycentricinterpolant_clear(b2);

    b2->n = b->n;
    b2->sy = b->sy;
    ae_vector_set_length(&b2->x, b2->n, _state);
    ae_vector_set_length(&b2->y, b2->n, _state);
    ae_vector_set_length(&b2->w, b2->n, _state);
    ae_v_move(&b2->x.ptr.p_double[0], 1, &b->x.ptr.p_double[0], 1, ae_v_len(0,b2->n-1));
    ae_v_move(&b2->y.ptr.p_double[0], 1, &b->y.ptr.p_double[0], 1, ae_v_len(0,b2->n-1));
    ae_v_move(&b2->w.ptr.p_double[0], 1, &b->w.ptr.p_double[0], 1, ae_v_len(0,b2->n-1));
}


/*************************************************************************
Normalization of barycentric interpolant:
* B.N, B.X, B.Y and B.W are initialized
* B.SY is NOT initialized
* Y[] is normalized, scaling coefficient is stored in B.SY
* W[] is normalized, no scaling coefficient is stored
* X[] is sorted

Internal subroutine.
*************************************************************************/
static void ratint_barycentricnormalize(barycentricinterpolant* b,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector p1;
    ae_vector p2;
    ae_int_t i;
    ae_int_t j;
    ae_int_t j2;
    double v;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&p1, 0, DT_INT, _state, ae_true);
    ae_vector_init(&p2, 0, DT_INT, _state, ae_true);

    
    /*
     * Normalize task: |Y|<=1, |W|<=1, sort X[]
     */
    b->sy = 0;
    for(i=0; i<=b->n-1; i++)
    {
        b->sy = ae_maxreal(b->sy, ae_fabs(b->y.ptr.p_double[i], _state), _state);
    }
    if( ae_fp_greater(b->sy,0)&&ae_fp_greater(ae_fabs(b->sy-1, _state),10*ae_machineepsilon) )
    {
        v = 1/b->sy;
        ae_v_muld(&b->y.ptr.p_double[0], 1, ae_v_len(0,b->n-1), v);
    }
    v = 0;
    for(i=0; i<=b->n-1; i++)
    {
        v = ae_maxreal(v, ae_fabs(b->w.ptr.p_double[i], _state), _state);
    }
    if( ae_fp_greater(v,0)&&ae_fp_greater(ae_fabs(v-1, _state),10*ae_machineepsilon) )
    {
        v = 1/v;
        ae_v_muld(&b->w.ptr.p_double[0], 1, ae_v_len(0,b->n-1), v);
    }
    for(i=0; i<=b->n-2; i++)
    {
        if( ae_fp_less(b->x.ptr.p_double[i+1],b->x.ptr.p_double[i]) )
        {
            tagsort(&b->x, b->n, &p1, &p2, _state);
            for(j=0; j<=b->n-1; j++)
            {
                j2 = p2.ptr.p_int[j];
                v = b->y.ptr.p_double[j];
                b->y.ptr.p_double[j] = b->y.ptr.p_double[j2];
                b->y.ptr.p_double[j2] = v;
                v = b->w.ptr.p_double[j];
                b->w.ptr.p_double[j] = b->w.ptr.p_double[j2];
                b->w.ptr.p_double[j2] = v;
            }
            break;
        }
    }
    ae_frame_leave(_state);
}


ae_bool _barycentricinterpolant_init(barycentricinterpolant* p, ae_state *_state, ae_bool make_automatic)
{
    if( !ae_vector_init(&p->x, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->y, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->w, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _barycentricinterpolant_init_copy(barycentricinterpolant* dst, barycentricinterpolant* src, ae_state *_state, ae_bool make_automatic)
{
    dst->n = src->n;
    dst->sy = src->sy;
    if( !ae_vector_init_copy(&dst->x, &src->x, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->y, &src->y, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->w, &src->w, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


void _barycentricinterpolant_clear(barycentricinterpolant* p)
{
    ae_vector_clear(&p->x);
    ae_vector_clear(&p->y);
    ae_vector_clear(&p->w);
}




/*************************************************************************
Conversion from barycentric representation to Chebyshev basis.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    P   -   polynomial in barycentric form
    A,B -   base interval for Chebyshev polynomials (see below)
            A<>B

OUTPUT PARAMETERS
    T   -   coefficients of Chebyshev representation;
            P(x) = sum { T[i]*Ti(2*(x-A)/(B-A)-1), i=0..N-1 },
            where Ti - I-th Chebyshev polynomial.

NOTES:
    barycentric interpolant passed as P may be either polynomial  obtained
    from  polynomial  interpolation/ fitting or rational function which is
    NOT polynomial. We can't distinguish between these two cases, and this
    algorithm just tries to work assuming that P IS a polynomial.  If not,
    algorithm will return results, but they won't have any meaning.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialbar2cheb(barycentricinterpolant* p,
     double a,
     double b,
     /* Real    */ ae_vector* t,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t k;
    ae_vector vp;
    ae_vector vx;
    ae_vector tk;
    ae_vector tk1;
    double v;

    ae_frame_make(_state, &_frame_block);
    ae_vector_clear(t);
    ae_vector_init(&vp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&vx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tk, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tk1, 0, DT_REAL, _state, ae_true);

    ae_assert(ae_isfinite(a, _state), "PolynomialBar2Cheb: A is not finite!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialBar2Cheb: B is not finite!", _state);
    ae_assert(ae_fp_neq(a,b), "PolynomialBar2Cheb: A=B!", _state);
    ae_assert(p->n>0, "PolynomialBar2Cheb: P is not correctly initialized barycentric interpolant!", _state);
    
    /*
     * Calculate function values on a Chebyshev grid
     */
    ae_vector_set_length(&vp, p->n, _state);
    ae_vector_set_length(&vx, p->n, _state);
    for(i=0; i<=p->n-1; i++)
    {
        vx.ptr.p_double[i] = ae_cos(ae_pi*(i+0.5)/p->n, _state);
        vp.ptr.p_double[i] = barycentriccalc(p, 0.5*(vx.ptr.p_double[i]+1)*(b-a)+a, _state);
    }
    
    /*
     * T[0]
     */
    ae_vector_set_length(t, p->n, _state);
    v = 0;
    for(i=0; i<=p->n-1; i++)
    {
        v = v+vp.ptr.p_double[i];
    }
    t->ptr.p_double[0] = v/p->n;
    
    /*
     * other T's.
     *
     * NOTES:
     * 1. TK stores T{k} on VX, TK1 stores T{k-1} on VX
     * 2. we can do same calculations with fast DCT, but it
     *    * adds dependencies
     *    * still leaves us with O(N^2) algorithm because
     *      preparation of function values is O(N^2) process
     */
    if( p->n>1 )
    {
        ae_vector_set_length(&tk, p->n, _state);
        ae_vector_set_length(&tk1, p->n, _state);
        for(i=0; i<=p->n-1; i++)
        {
            tk.ptr.p_double[i] = vx.ptr.p_double[i];
            tk1.ptr.p_double[i] = 1;
        }
        for(k=1; k<=p->n-1; k++)
        {
            
            /*
             * calculate discrete product of function vector and TK
             */
            v = ae_v_dotproduct(&tk.ptr.p_double[0], 1, &vp.ptr.p_double[0], 1, ae_v_len(0,p->n-1));
            t->ptr.p_double[k] = v/(0.5*p->n);
            
            /*
             * Update TK and TK1
             */
            for(i=0; i<=p->n-1; i++)
            {
                v = 2*vx.ptr.p_double[i]*tk.ptr.p_double[i]-tk1.ptr.p_double[i];
                tk1.ptr.p_double[i] = tk.ptr.p_double[i];
                tk.ptr.p_double[i] = v;
            }
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Conversion from Chebyshev basis to barycentric representation.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    T   -   coefficients of Chebyshev representation;
            P(x) = sum { T[i]*Ti(2*(x-A)/(B-A)-1), i=0..N },
            where Ti - I-th Chebyshev polynomial.
    N   -   number of coefficients:
            * if given, only leading N elements of T are used
            * if not given, automatically determined from size of T
    A,B -   base interval for Chebyshev polynomials (see above)
            A<B

OUTPUT PARAMETERS
    P   -   polynomial in barycentric form

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialcheb2bar(/* Real    */ ae_vector* t,
     ae_int_t n,
     double a,
     double b,
     barycentricinterpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t k;
    ae_vector y;
    double tk;
    double tk1;
    double vx;
    double vy;
    double v;

    ae_frame_make(_state, &_frame_block);
    _barycentricinterpolant_clear(p);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);

    ae_assert(ae_isfinite(a, _state), "PolynomialBar2Cheb: A is not finite!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialBar2Cheb: B is not finite!", _state);
    ae_assert(ae_fp_neq(a,b), "PolynomialBar2Cheb: A=B!", _state);
    ae_assert(n>=1, "PolynomialBar2Cheb: N<1", _state);
    ae_assert(t->cnt>=n, "PolynomialBar2Cheb: Length(T)<N", _state);
    ae_assert(isfinitevector(t, n, _state), "PolynomialBar2Cheb: T[] contains INF or NAN", _state);
    
    /*
     * Calculate function values on a Chebyshev grid spanning [-1,+1]
     */
    ae_vector_set_length(&y, n, _state);
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * Calculate value on a grid spanning [-1,+1]
         */
        vx = ae_cos(ae_pi*(i+0.5)/n, _state);
        vy = t->ptr.p_double[0];
        tk1 = 1;
        tk = vx;
        for(k=1; k<=n-1; k++)
        {
            vy = vy+t->ptr.p_double[k]*tk;
            v = 2*vx*tk-tk1;
            tk1 = tk;
            tk = v;
        }
        y.ptr.p_double[i] = vy;
    }
    
    /*
     * Build barycentric interpolant, map grid from [-1,+1] to [A,B]
     */
    polynomialbuildcheb1(a, b, &y, n, p, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Conversion from barycentric representation to power basis.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    P   -   polynomial in barycentric form
    C   -   offset (see below); 0.0 is used as default value.
    S   -   scale (see below);  1.0 is used as default value. S<>0.

OUTPUT PARAMETERS
    A   -   coefficients, P(x) = sum { A[i]*((X-C)/S)^i, i=0..N-1 }
    N   -   number of coefficients (polynomial degree plus 1)

NOTES:
1.  this function accepts offset and scale, which can be  set  to  improve
    numerical properties of polynomial. For example, if P was obtained  as
    result of interpolation on [-1,+1],  you  can  set  C=0  and  S=1  and
    represent  P  as sum of 1, x, x^2, x^3 and so on. In most cases you it
    is exactly what you need.

    However, if your interpolation model was built on [999,1001], you will
    see significant growth of numerical errors when using {1, x, x^2, x^3}
    as basis. Representing P as sum of 1, (x-1000), (x-1000)^2, (x-1000)^3
    will be better option. Such representation can be  obtained  by  using
    1000.0 as offset C and 1.0 as scale S.

2.  power basis is ill-conditioned and tricks described above can't  solve
    this problem completely. This function  will  return  coefficients  in
    any  case,  but  for  N>8  they  will  become unreliable. However, N's
    less than 5 are pretty safe.
    
3.  barycentric interpolant passed as P may be either polynomial  obtained
    from  polynomial  interpolation/ fitting or rational function which is
    NOT polynomial. We can't distinguish between these two cases, and this
    algorithm just tries to work assuming that P IS a polynomial.  If not,
    algorithm will return results, but they won't have any meaning.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialbar2pow(barycentricinterpolant* p,
     double c,
     double s,
     /* Real    */ ae_vector* a,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t k;
    double e;
    double d;
    ae_vector vp;
    ae_vector vx;
    ae_vector tk;
    ae_vector tk1;
    ae_vector t;
    double v;

    ae_frame_make(_state, &_frame_block);
    ae_vector_clear(a);
    ae_vector_init(&vp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&vx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tk, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tk1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&t, 0, DT_REAL, _state, ae_true);

    ae_assert(ae_isfinite(c, _state), "PolynomialBar2Pow: C is not finite!", _state);
    ae_assert(ae_isfinite(s, _state), "PolynomialBar2Pow: S is not finite!", _state);
    ae_assert(ae_fp_neq(s,0), "PolynomialBar2Pow: S=0!", _state);
    ae_assert(p->n>0, "PolynomialBar2Pow: P is not correctly initialized barycentric interpolant!", _state);
    
    /*
     * Calculate function values on a Chebyshev grid
     */
    ae_vector_set_length(&vp, p->n, _state);
    ae_vector_set_length(&vx, p->n, _state);
    for(i=0; i<=p->n-1; i++)
    {
        vx.ptr.p_double[i] = ae_cos(ae_pi*(i+0.5)/p->n, _state);
        vp.ptr.p_double[i] = barycentriccalc(p, s*vx.ptr.p_double[i]+c, _state);
    }
    
    /*
     * T[0]
     */
    ae_vector_set_length(&t, p->n, _state);
    v = 0;
    for(i=0; i<=p->n-1; i++)
    {
        v = v+vp.ptr.p_double[i];
    }
    t.ptr.p_double[0] = v/p->n;
    
    /*
     * other T's.
     *
     * NOTES:
     * 1. TK stores T{k} on VX, TK1 stores T{k-1} on VX
     * 2. we can do same calculations with fast DCT, but it
     *    * adds dependencies
     *    * still leaves us with O(N^2) algorithm because
     *      preparation of function values is O(N^2) process
     */
    if( p->n>1 )
    {
        ae_vector_set_length(&tk, p->n, _state);
        ae_vector_set_length(&tk1, p->n, _state);
        for(i=0; i<=p->n-1; i++)
        {
            tk.ptr.p_double[i] = vx.ptr.p_double[i];
            tk1.ptr.p_double[i] = 1;
        }
        for(k=1; k<=p->n-1; k++)
        {
            
            /*
             * calculate discrete product of function vector and TK
             */
            v = ae_v_dotproduct(&tk.ptr.p_double[0], 1, &vp.ptr.p_double[0], 1, ae_v_len(0,p->n-1));
            t.ptr.p_double[k] = v/(0.5*p->n);
            
            /*
             * Update TK and TK1
             */
            for(i=0; i<=p->n-1; i++)
            {
                v = 2*vx.ptr.p_double[i]*tk.ptr.p_double[i]-tk1.ptr.p_double[i];
                tk1.ptr.p_double[i] = tk.ptr.p_double[i];
                tk.ptr.p_double[i] = v;
            }
        }
    }
    
    /*
     * Convert from Chebyshev basis to power basis
     */
    ae_vector_set_length(a, p->n, _state);
    for(i=0; i<=p->n-1; i++)
    {
        a->ptr.p_double[i] = 0;
    }
    d = 0;
    for(i=0; i<=p->n-1; i++)
    {
        for(k=i; k<=p->n-1; k++)
        {
            e = a->ptr.p_double[k];
            a->ptr.p_double[k] = 0;
            if( i<=1&&k==i )
            {
                a->ptr.p_double[k] = 1;
            }
            else
            {
                if( i!=0 )
                {
                    a->ptr.p_double[k] = 2*d;
                }
                if( k>i+1 )
                {
                    a->ptr.p_double[k] = a->ptr.p_double[k]-a->ptr.p_double[k-2];
                }
            }
            d = e;
        }
        d = a->ptr.p_double[i];
        e = 0;
        k = i;
        while(k<=p->n-1)
        {
            e = e+a->ptr.p_double[k]*t.ptr.p_double[k];
            k = k+2;
        }
        a->ptr.p_double[i] = e;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Conversion from power basis to barycentric representation.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    A   -   coefficients, P(x) = sum { A[i]*((X-C)/S)^i, i=0..N-1 }
    N   -   number of coefficients (polynomial degree plus 1)
            * if given, only leading N elements of A are used
            * if not given, automatically determined from size of A
    C   -   offset (see below); 0.0 is used as default value.
    S   -   scale (see below);  1.0 is used as default value. S<>0.

OUTPUT PARAMETERS
    P   -   polynomial in barycentric form


NOTES:
1.  this function accepts offset and scale, which can be  set  to  improve
    numerical properties of polynomial. For example, if you interpolate on
    [-1,+1],  you  can  set C=0 and S=1 and convert from sum of 1, x, x^2,
    x^3 and so on. In most cases you it is exactly what you need.

    However, if your interpolation model was built on [999,1001], you will
    see significant growth of numerical errors when using {1, x, x^2, x^3}
    as  input  basis.  Converting  from  sum  of  1, (x-1000), (x-1000)^2,
    (x-1000)^3 will be better option (you have to specify 1000.0 as offset
    C and 1.0 as scale S).

2.  power basis is ill-conditioned and tricks described above can't  solve
    this problem completely. This function  will  return barycentric model
    in any case, but for N>8 accuracy well degrade. However, N's less than
    5 are pretty safe.

  -- ALGLIB --
     Copyright 30.09.2010 by Bochkanov Sergey
*************************************************************************/
void polynomialpow2bar(/* Real    */ ae_vector* a,
     ae_int_t n,
     double c,
     double s,
     barycentricinterpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t k;
    ae_vector y;
    double vx;
    double vy;
    double px;

    ae_frame_make(_state, &_frame_block);
    _barycentricinterpolant_clear(p);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);

    ae_assert(ae_isfinite(c, _state), "PolynomialPow2Bar: C is not finite!", _state);
    ae_assert(ae_isfinite(s, _state), "PolynomialPow2Bar: S is not finite!", _state);
    ae_assert(ae_fp_neq(s,0), "PolynomialPow2Bar: S is zero!", _state);
    ae_assert(n>=1, "PolynomialPow2Bar: N<1", _state);
    ae_assert(a->cnt>=n, "PolynomialPow2Bar: Length(A)<N", _state);
    ae_assert(isfinitevector(a, n, _state), "PolynomialPow2Bar: A[] contains INF or NAN", _state);
    
    /*
     * Calculate function values on a Chebyshev grid spanning [-1,+1]
     */
    ae_vector_set_length(&y, n, _state);
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * Calculate value on a grid spanning [-1,+1]
         */
        vx = ae_cos(ae_pi*(i+0.5)/n, _state);
        vy = a->ptr.p_double[0];
        px = vx;
        for(k=1; k<=n-1; k++)
        {
            vy = vy+px*a->ptr.p_double[k];
            px = px*vx;
        }
        y.ptr.p_double[i] = vy;
    }
    
    /*
     * Build barycentric interpolant, map grid from [-1,+1] to [A,B]
     */
    polynomialbuildcheb1(c-s, c+s, &y, n, p, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Lagrange intepolant: generation of the model on the general grid.
This function has O(N^2) complexity.

INPUT PARAMETERS:
    X   -   abscissas, array[0..N-1]
    Y   -   function values, array[0..N-1]
    N   -   number of points, N>=1

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuild(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_int_t j;
    ae_int_t k;
    ae_vector w;
    double b;
    double a;
    double v;
    double mx;
    ae_vector sortrbuf;
    ae_vector sortrbuf2;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    _barycentricinterpolant_clear(p);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sortrbuf, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sortrbuf2, 0, DT_REAL, _state, ae_true);

    ae_assert(n>0, "PolynomialBuild: N<=0!", _state);
    ae_assert(x->cnt>=n, "PolynomialBuild: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "PolynomialBuild: Length(Y)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "PolynomialBuild: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "PolynomialBuild: Y contains infinite or NaN values!", _state);
    tagsortfastr(x, y, &sortrbuf, &sortrbuf2, n, _state);
    ae_assert(aredistinct(x, n, _state), "PolynomialBuild: at least two consequent points are too close!", _state);
    
    /*
     * calculate W[j]
     * multi-pass algorithm is used to avoid overflow
     */
    ae_vector_set_length(&w, n, _state);
    a = x->ptr.p_double[0];
    b = x->ptr.p_double[0];
    for(j=0; j<=n-1; j++)
    {
        w.ptr.p_double[j] = 1;
        a = ae_minreal(a, x->ptr.p_double[j], _state);
        b = ae_maxreal(b, x->ptr.p_double[j], _state);
    }
    for(k=0; k<=n-1; k++)
    {
        
        /*
         * W[K] is used instead of 0.0 because
         * cycle on J does not touch K-th element
         * and we MUST get maximum from ALL elements
         */
        mx = ae_fabs(w.ptr.p_double[k], _state);
        for(j=0; j<=n-1; j++)
        {
            if( j!=k )
            {
                v = (b-a)/(x->ptr.p_double[j]-x->ptr.p_double[k]);
                w.ptr.p_double[j] = w.ptr.p_double[j]*v;
                mx = ae_maxreal(mx, ae_fabs(w.ptr.p_double[j], _state), _state);
            }
        }
        if( k%5==0 )
        {
            
            /*
             * every 5-th run we renormalize W[]
             */
            v = 1/mx;
            ae_v_muld(&w.ptr.p_double[0], 1, ae_v_len(0,n-1), v);
        }
    }
    barycentricbuildxyw(x, y, &w, n, p, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Lagrange intepolant: generation of the model on equidistant grid.
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1]
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildeqdist(double a,
     double b,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector w;
    ae_vector x;
    double v;

    ae_frame_make(_state, &_frame_block);
    _barycentricinterpolant_clear(p);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);

    ae_assert(n>0, "PolynomialBuildEqDist: N<=0!", _state);
    ae_assert(y->cnt>=n, "PolynomialBuildEqDist: Length(Y)<N!", _state);
    ae_assert(ae_isfinite(a, _state), "PolynomialBuildEqDist: A is infinite or NaN!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialBuildEqDist: B is infinite or NaN!", _state);
    ae_assert(isfinitevector(y, n, _state), "PolynomialBuildEqDist: Y contains infinite or NaN values!", _state);
    ae_assert(ae_fp_neq(b,a), "PolynomialBuildEqDist: B=A!", _state);
    ae_assert(ae_fp_neq(a+(b-a)/n,a), "PolynomialBuildEqDist: B is too close to A!", _state);
    
    /*
     * Special case: N=1
     */
    if( n==1 )
    {
        ae_vector_set_length(&x, 1, _state);
        ae_vector_set_length(&w, 1, _state);
        x.ptr.p_double[0] = 0.5*(b+a);
        w.ptr.p_double[0] = 1;
        barycentricbuildxyw(&x, y, &w, 1, p, _state);
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * general case
     */
    ae_vector_set_length(&x, n, _state);
    ae_vector_set_length(&w, n, _state);
    v = 1;
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = v;
        x.ptr.p_double[i] = a+(b-a)*i/(n-1);
        v = -v*(n-1-i);
        v = v/(i+1);
    }
    barycentricbuildxyw(&x, y, &w, n, p, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Lagrange intepolant on Chebyshev grid (first kind).
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1],
            Y[I] = Y(0.5*(B+A) + 0.5*(B-A)*Cos(PI*(2*i+1)/(2*n)))
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildcheb1(double a,
     double b,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector w;
    ae_vector x;
    double v;
    double t;

    ae_frame_make(_state, &_frame_block);
    _barycentricinterpolant_clear(p);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);

    ae_assert(n>0, "PolynomialBuildCheb1: N<=0!", _state);
    ae_assert(y->cnt>=n, "PolynomialBuildCheb1: Length(Y)<N!", _state);
    ae_assert(ae_isfinite(a, _state), "PolynomialBuildCheb1: A is infinite or NaN!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialBuildCheb1: B is infinite or NaN!", _state);
    ae_assert(isfinitevector(y, n, _state), "PolynomialBuildCheb1: Y contains infinite or NaN values!", _state);
    ae_assert(ae_fp_neq(b,a), "PolynomialBuildCheb1: B=A!", _state);
    
    /*
     * Special case: N=1
     */
    if( n==1 )
    {
        ae_vector_set_length(&x, 1, _state);
        ae_vector_set_length(&w, 1, _state);
        x.ptr.p_double[0] = 0.5*(b+a);
        w.ptr.p_double[0] = 1;
        barycentricbuildxyw(&x, y, &w, 1, p, _state);
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * general case
     */
    ae_vector_set_length(&x, n, _state);
    ae_vector_set_length(&w, n, _state);
    v = 1;
    for(i=0; i<=n-1; i++)
    {
        t = ae_tan(0.5*ae_pi*(2*i+1)/(2*n), _state);
        w.ptr.p_double[i] = 2*v*t/(1+ae_sqr(t, _state));
        x.ptr.p_double[i] = 0.5*(b+a)+0.5*(b-a)*(1-ae_sqr(t, _state))/(1+ae_sqr(t, _state));
        v = -v;
    }
    barycentricbuildxyw(&x, y, &w, n, p, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Lagrange intepolant on Chebyshev grid (second kind).
This function has O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    Y   -   function values at the nodes, array[0..N-1],
            Y[I] = Y(0.5*(B+A) + 0.5*(B-A)*Cos(PI*i/(n-1)))
    N   -   number of points, N>=1
            for N=1 a constant model is constructed.

OUTPUT PARAMETERS
    P   -   barycentric model which represents Lagrange interpolant
            (see ratint unit info and BarycentricCalc() description for
            more information).

  -- ALGLIB --
     Copyright 03.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialbuildcheb2(double a,
     double b,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector w;
    ae_vector x;
    double v;

    ae_frame_make(_state, &_frame_block);
    _barycentricinterpolant_clear(p);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);

    ae_assert(n>0, "PolynomialBuildCheb2: N<=0!", _state);
    ae_assert(y->cnt>=n, "PolynomialBuildCheb2: Length(Y)<N!", _state);
    ae_assert(ae_isfinite(a, _state), "PolynomialBuildCheb2: A is infinite or NaN!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialBuildCheb2: B is infinite or NaN!", _state);
    ae_assert(ae_fp_neq(b,a), "PolynomialBuildCheb2: B=A!", _state);
    ae_assert(isfinitevector(y, n, _state), "PolynomialBuildCheb2: Y contains infinite or NaN values!", _state);
    
    /*
     * Special case: N=1
     */
    if( n==1 )
    {
        ae_vector_set_length(&x, 1, _state);
        ae_vector_set_length(&w, 1, _state);
        x.ptr.p_double[0] = 0.5*(b+a);
        w.ptr.p_double[0] = 1;
        barycentricbuildxyw(&x, y, &w, 1, p, _state);
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * general case
     */
    ae_vector_set_length(&x, n, _state);
    ae_vector_set_length(&w, n, _state);
    v = 1;
    for(i=0; i<=n-1; i++)
    {
        if( i==0||i==n-1 )
        {
            w.ptr.p_double[i] = v*0.5;
        }
        else
        {
            w.ptr.p_double[i] = v;
        }
        x.ptr.p_double[i] = 0.5*(b+a)+0.5*(b-a)*ae_cos(ae_pi*i/(n-1), _state);
        v = -v;
    }
    barycentricbuildxyw(&x, y, &w, n, p, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Fast equidistant polynomial interpolation function with O(N) complexity

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on equidistant grid, N>=1
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T
    
IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use  PolynomialBuildEqDist()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalceqdist(double a,
     double b,
     /* Real    */ ae_vector* f,
     ae_int_t n,
     double t,
     ae_state *_state)
{
    double s1;
    double s2;
    double v;
    double threshold;
    double s;
    double h;
    ae_int_t i;
    ae_int_t j;
    double w;
    double x;
    double result;


    ae_assert(n>0, "PolynomialCalcEqDist: N<=0!", _state);
    ae_assert(f->cnt>=n, "PolynomialCalcEqDist: Length(F)<N!", _state);
    ae_assert(ae_isfinite(a, _state), "PolynomialCalcEqDist: A is infinite or NaN!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialCalcEqDist: B is infinite or NaN!", _state);
    ae_assert(isfinitevector(f, n, _state), "PolynomialCalcEqDist: F contains infinite or NaN values!", _state);
    ae_assert(ae_fp_neq(b,a), "PolynomialCalcEqDist: B=A!", _state);
    ae_assert(!ae_isinf(t, _state), "PolynomialCalcEqDist: T is infinite!", _state);
    
    /*
     * Special case: T is NAN
     */
    if( ae_isnan(t, _state) )
    {
        result = _state->v_nan;
        return result;
    }
    
    /*
     * Special case: N=1
     */
    if( n==1 )
    {
        result = f->ptr.p_double[0];
        return result;
    }
    
    /*
     * First, decide: should we use "safe" formula (guarded
     * against overflow) or fast one?
     */
    threshold = ae_sqrt(ae_minrealnumber, _state);
    j = 0;
    s = t-a;
    for(i=1; i<=n-1; i++)
    {
        x = a+(double)i/(double)(n-1)*(b-a);
        if( ae_fp_less(ae_fabs(t-x, _state),ae_fabs(s, _state)) )
        {
            s = t-x;
            j = i;
        }
    }
    if( ae_fp_eq(s,0) )
    {
        result = f->ptr.p_double[j];
        return result;
    }
    if( ae_fp_greater(ae_fabs(s, _state),threshold) )
    {
        
        /*
         * use fast formula
         */
        j = -1;
        s = 1.0;
    }
    
    /*
     * Calculate using safe or fast barycentric formula
     */
    s1 = 0;
    s2 = 0;
    w = 1.0;
    h = (b-a)/(n-1);
    for(i=0; i<=n-1; i++)
    {
        if( i!=j )
        {
            v = s*w/(t-(a+i*h));
            s1 = s1+v*f->ptr.p_double[i];
            s2 = s2+v;
        }
        else
        {
            v = w;
            s1 = s1+v*f->ptr.p_double[i];
            s2 = s2+v;
        }
        w = -w*(n-1-i);
        w = w/(i+1);
    }
    result = s1/s2;
    return result;
}


/*************************************************************************
Fast polynomial interpolation function on Chebyshev points (first kind)
with O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on Chebyshev grid (first kind),
            X[i] = 0.5*(B+A) + 0.5*(B-A)*Cos(PI*(2*i+1)/(2*n))
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use  PolIntBuildCheb1()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalccheb1(double a,
     double b,
     /* Real    */ ae_vector* f,
     ae_int_t n,
     double t,
     ae_state *_state)
{
    double s1;
    double s2;
    double v;
    double threshold;
    double s;
    ae_int_t i;
    ae_int_t j;
    double a0;
    double delta;
    double alpha;
    double beta;
    double ca;
    double sa;
    double tempc;
    double temps;
    double x;
    double w;
    double p1;
    double result;


    ae_assert(n>0, "PolynomialCalcCheb1: N<=0!", _state);
    ae_assert(f->cnt>=n, "PolynomialCalcCheb1: Length(F)<N!", _state);
    ae_assert(ae_isfinite(a, _state), "PolynomialCalcCheb1: A is infinite or NaN!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialCalcCheb1: B is infinite or NaN!", _state);
    ae_assert(isfinitevector(f, n, _state), "PolynomialCalcCheb1: F contains infinite or NaN values!", _state);
    ae_assert(ae_fp_neq(b,a), "PolynomialCalcCheb1: B=A!", _state);
    ae_assert(!ae_isinf(t, _state), "PolynomialCalcCheb1: T is infinite!", _state);
    
    /*
     * Special case: T is NAN
     */
    if( ae_isnan(t, _state) )
    {
        result = _state->v_nan;
        return result;
    }
    
    /*
     * Special case: N=1
     */
    if( n==1 )
    {
        result = f->ptr.p_double[0];
        return result;
    }
    
    /*
     * Prepare information for the recurrence formula
     * used to calculate sin(pi*(2j+1)/(2n+2)) and
     * cos(pi*(2j+1)/(2n+2)):
     *
     * A0    = pi/(2n+2)
     * Delta = pi/(n+1)
     * Alpha = 2 sin^2 (Delta/2)
     * Beta  = sin(Delta)
     *
     * so that sin(..) = sin(A0+j*delta) and cos(..) = cos(A0+j*delta).
     * Then we use
     *
     * sin(x+delta) = sin(x) - (alpha*sin(x) - beta*cos(x))
     * cos(x+delta) = cos(x) - (alpha*cos(x) - beta*sin(x))
     *
     * to repeatedly calculate sin(..) and cos(..).
     */
    threshold = ae_sqrt(ae_minrealnumber, _state);
    t = (t-0.5*(a+b))/(0.5*(b-a));
    a0 = ae_pi/(2*(n-1)+2);
    delta = 2*ae_pi/(2*(n-1)+2);
    alpha = 2*ae_sqr(ae_sin(delta/2, _state), _state);
    beta = ae_sin(delta, _state);
    
    /*
     * First, decide: should we use "safe" formula (guarded
     * against overflow) or fast one?
     */
    ca = ae_cos(a0, _state);
    sa = ae_sin(a0, _state);
    j = 0;
    x = ca;
    s = t-x;
    for(i=1; i<=n-1; i++)
    {
        
        /*
         * Next X[i]
         */
        temps = sa-(alpha*sa-beta*ca);
        tempc = ca-(alpha*ca+beta*sa);
        sa = temps;
        ca = tempc;
        x = ca;
        
        /*
         * Use X[i]
         */
        if( ae_fp_less(ae_fabs(t-x, _state),ae_fabs(s, _state)) )
        {
            s = t-x;
            j = i;
        }
    }
    if( ae_fp_eq(s,0) )
    {
        result = f->ptr.p_double[j];
        return result;
    }
    if( ae_fp_greater(ae_fabs(s, _state),threshold) )
    {
        
        /*
         * use fast formula
         */
        j = -1;
        s = 1.0;
    }
    
    /*
     * Calculate using safe or fast barycentric formula
     */
    s1 = 0;
    s2 = 0;
    ca = ae_cos(a0, _state);
    sa = ae_sin(a0, _state);
    p1 = 1.0;
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * Calculate X[i], W[i]
         */
        x = ca;
        w = p1*sa;
        
        /*
         * Proceed
         */
        if( i!=j )
        {
            v = s*w/(t-x);
            s1 = s1+v*f->ptr.p_double[i];
            s2 = s2+v;
        }
        else
        {
            v = w;
            s1 = s1+v*f->ptr.p_double[i];
            s2 = s2+v;
        }
        
        /*
         * Next CA, SA, P1
         */
        temps = sa-(alpha*sa-beta*ca);
        tempc = ca-(alpha*ca+beta*sa);
        sa = temps;
        ca = tempc;
        p1 = -p1;
    }
    result = s1/s2;
    return result;
}


/*************************************************************************
Fast polynomial interpolation function on Chebyshev points (second kind)
with O(N) complexity.

INPUT PARAMETERS:
    A   -   left boundary of [A,B]
    B   -   right boundary of [A,B]
    F   -   function values, array[0..N-1]
    N   -   number of points on Chebyshev grid (second kind),
            X[i] = 0.5*(B+A) + 0.5*(B-A)*Cos(PI*i/(n-1))
            for N=1 a constant model is constructed.
    T   -   position where P(x) is calculated

RESULT
    value of the Lagrange interpolant at T

IMPORTANT
    this function provides fast interface which is not overflow-safe
    nor it is very precise.
    the best option is to use PolIntBuildCheb2()/BarycentricCalc()
    subroutines unless you are pretty sure that your data will not result
    in overflow.

  -- ALGLIB --
     Copyright 02.12.2009 by Bochkanov Sergey
*************************************************************************/
double polynomialcalccheb2(double a,
     double b,
     /* Real    */ ae_vector* f,
     ae_int_t n,
     double t,
     ae_state *_state)
{
    double s1;
    double s2;
    double v;
    double threshold;
    double s;
    ae_int_t i;
    ae_int_t j;
    double a0;
    double delta;
    double alpha;
    double beta;
    double ca;
    double sa;
    double tempc;
    double temps;
    double x;
    double w;
    double p1;
    double result;


    ae_assert(n>0, "PolynomialCalcCheb2: N<=0!", _state);
    ae_assert(f->cnt>=n, "PolynomialCalcCheb2: Length(F)<N!", _state);
    ae_assert(ae_isfinite(a, _state), "PolynomialCalcCheb2: A is infinite or NaN!", _state);
    ae_assert(ae_isfinite(b, _state), "PolynomialCalcCheb2: B is infinite or NaN!", _state);
    ae_assert(ae_fp_neq(b,a), "PolynomialCalcCheb2: B=A!", _state);
    ae_assert(isfinitevector(f, n, _state), "PolynomialCalcCheb2: F contains infinite or NaN values!", _state);
    ae_assert(!ae_isinf(t, _state), "PolynomialCalcEqDist: T is infinite!", _state);
    
    /*
     * Special case: T is NAN
     */
    if( ae_isnan(t, _state) )
    {
        result = _state->v_nan;
        return result;
    }
    
    /*
     * Special case: N=1
     */
    if( n==1 )
    {
        result = f->ptr.p_double[0];
        return result;
    }
    
    /*
     * Prepare information for the recurrence formula
     * used to calculate sin(pi*i/n) and
     * cos(pi*i/n):
     *
     * A0    = 0
     * Delta = pi/n
     * Alpha = 2 sin^2 (Delta/2)
     * Beta  = sin(Delta)
     *
     * so that sin(..) = sin(A0+j*delta) and cos(..) = cos(A0+j*delta).
     * Then we use
     *
     * sin(x+delta) = sin(x) - (alpha*sin(x) - beta*cos(x))
     * cos(x+delta) = cos(x) - (alpha*cos(x) - beta*sin(x))
     *
     * to repeatedly calculate sin(..) and cos(..).
     */
    threshold = ae_sqrt(ae_minrealnumber, _state);
    t = (t-0.5*(a+b))/(0.5*(b-a));
    a0 = 0.0;
    delta = ae_pi/(n-1);
    alpha = 2*ae_sqr(ae_sin(delta/2, _state), _state);
    beta = ae_sin(delta, _state);
    
    /*
     * First, decide: should we use "safe" formula (guarded
     * against overflow) or fast one?
     */
    ca = ae_cos(a0, _state);
    sa = ae_sin(a0, _state);
    j = 0;
    x = ca;
    s = t-x;
    for(i=1; i<=n-1; i++)
    {
        
        /*
         * Next X[i]
         */
        temps = sa-(alpha*sa-beta*ca);
        tempc = ca-(alpha*ca+beta*sa);
        sa = temps;
        ca = tempc;
        x = ca;
        
        /*
         * Use X[i]
         */
        if( ae_fp_less(ae_fabs(t-x, _state),ae_fabs(s, _state)) )
        {
            s = t-x;
            j = i;
        }
    }
    if( ae_fp_eq(s,0) )
    {
        result = f->ptr.p_double[j];
        return result;
    }
    if( ae_fp_greater(ae_fabs(s, _state),threshold) )
    {
        
        /*
         * use fast formula
         */
        j = -1;
        s = 1.0;
    }
    
    /*
     * Calculate using safe or fast barycentric formula
     */
    s1 = 0;
    s2 = 0;
    ca = ae_cos(a0, _state);
    sa = ae_sin(a0, _state);
    p1 = 1.0;
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * Calculate X[i], W[i]
         */
        x = ca;
        if( i==0||i==n-1 )
        {
            w = 0.5*p1;
        }
        else
        {
            w = 1.0*p1;
        }
        
        /*
         * Proceed
         */
        if( i!=j )
        {
            v = s*w/(t-x);
            s1 = s1+v*f->ptr.p_double[i];
            s2 = s2+v;
        }
        else
        {
            v = w;
            s1 = s1+v*f->ptr.p_double[i];
            s2 = s2+v;
        }
        
        /*
         * Next CA, SA, P1
         */
        temps = sa-(alpha*sa-beta*ca);
        tempc = ca-(alpha*ca+beta*sa);
        sa = temps;
        ca = tempc;
        p1 = -p1;
    }
    result = s1/s2;
    return result;
}




/*************************************************************************
This subroutine builds linear spline interpolant

INPUT PARAMETERS:
    X   -   spline nodes, array[0..N-1]
    Y   -   function values, array[0..N-1]
    N   -   points count (optional):
            * N>=2
            * if given, only first N points are used to build spline
            * if not given, automatically detected from X/Y sizes
              (len(X) must be equal to len(Y))
    
OUTPUT PARAMETERS:
    C   -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildlinear(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     spline1dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    _spline1dinterpolant_clear(c);

    ae_assert(n>1, "Spline1DBuildLinear: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DBuildLinear: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DBuildLinear: Length(Y)<N!", _state);
    
    /*
     * check and sort points
     */
    ae_assert(isfinitevector(x, n, _state), "Spline1DBuildLinear: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DBuildLinear: Y contains infinite or NAN values!", _state);
    spline1d_heapsortpoints(x, y, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DBuildLinear: at least two consequent points are too close!", _state);
    
    /*
     * Build
     */
    c->periodic = ae_false;
    c->n = n;
    c->k = 3;
    ae_vector_set_length(&c->x, n, _state);
    ae_vector_set_length(&c->c, 4*(n-1), _state);
    for(i=0; i<=n-1; i++)
    {
        c->x.ptr.p_double[i] = x->ptr.p_double[i];
    }
    for(i=0; i<=n-2; i++)
    {
        c->c.ptr.p_double[4*i+0] = y->ptr.p_double[i];
        c->c.ptr.p_double[4*i+1] = (y->ptr.p_double[i+1]-y->ptr.p_double[i])/(x->ptr.p_double[i+1]-x->ptr.p_double[i]);
        c->c.ptr.p_double[4*i+2] = 0;
        c->c.ptr.p_double[4*i+3] = 0;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine builds cubic spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1].
    Y           -   function values, array[0..N-1].
    
OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    C           -   spline interpolant

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     spline1dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector a1;
    ae_vector a2;
    ae_vector a3;
    ae_vector b;
    ae_vector dt;
    ae_vector d;
    ae_vector p;
    ae_int_t ylen;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    _spline1dinterpolant_clear(c);
    ae_vector_init(&a1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a3, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&p, 0, DT_INT, _state, ae_true);

    
    /*
     * check correctness of boundary conditions
     */
    ae_assert(((boundltype==-1||boundltype==0)||boundltype==1)||boundltype==2, "Spline1DBuildCubic: incorrect BoundLType!", _state);
    ae_assert(((boundrtype==-1||boundrtype==0)||boundrtype==1)||boundrtype==2, "Spline1DBuildCubic: incorrect BoundRType!", _state);
    ae_assert((boundrtype==-1&&boundltype==-1)||(boundrtype!=-1&&boundltype!=-1), "Spline1DBuildCubic: incorrect BoundLType/BoundRType!", _state);
    if( boundltype==1||boundltype==2 )
    {
        ae_assert(ae_isfinite(boundl, _state), "Spline1DBuildCubic: BoundL is infinite or NAN!", _state);
    }
    if( boundrtype==1||boundrtype==2 )
    {
        ae_assert(ae_isfinite(boundr, _state), "Spline1DBuildCubic: BoundR is infinite or NAN!", _state);
    }
    
    /*
     * check lengths of arguments
     */
    ae_assert(n>=2, "Spline1DBuildCubic: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DBuildCubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DBuildCubic: Length(Y)<N!", _state);
    
    /*
     * check and sort points
     */
    ylen = n;
    if( boundltype==-1 )
    {
        ylen = n-1;
    }
    ae_assert(isfinitevector(x, n, _state), "Spline1DBuildCubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, ylen, _state), "Spline1DBuildCubic: Y contains infinite or NAN values!", _state);
    spline1d_heapsortppoints(x, y, &p, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DBuildCubic: at least two consequent points are too close!", _state);
    
    /*
     * Now we've checked and preordered everything,
     * so we can call internal function to calculate derivatives,
     * and then build Hermite spline using these derivatives
     */
    spline1d_spline1dgriddiffcubicinternal(x, y, n, boundltype, boundl, boundrtype, boundr, &d, &a1, &a2, &a3, &b, &dt, _state);
    spline1dbuildhermite(x, y, &d, n, c, _state);
    c->periodic = boundltype==-1||boundrtype==-1;
    ae_frame_leave(_state);
}


/*************************************************************************
This function solves following problem: given table y[] of function values
at nodes x[], it calculates and returns table of function derivatives  d[]
(calculated at the same nodes x[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   spline nodes
    Y           -   function values

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    D           -   derivative values at X[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.
Derivative values are correctly reordered on return, so  D[I]  is  always
equal to S'(X[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dgriddiffcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* d,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector a1;
    ae_vector a2;
    ae_vector a3;
    ae_vector b;
    ae_vector dt;
    ae_vector p;
    ae_int_t i;
    ae_int_t ylen;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_clear(d);
    ae_vector_init(&a1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a3, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&p, 0, DT_INT, _state, ae_true);

    
    /*
     * check correctness of boundary conditions
     */
    ae_assert(((boundltype==-1||boundltype==0)||boundltype==1)||boundltype==2, "Spline1DGridDiffCubic: incorrect BoundLType!", _state);
    ae_assert(((boundrtype==-1||boundrtype==0)||boundrtype==1)||boundrtype==2, "Spline1DGridDiffCubic: incorrect BoundRType!", _state);
    ae_assert((boundrtype==-1&&boundltype==-1)||(boundrtype!=-1&&boundltype!=-1), "Spline1DGridDiffCubic: incorrect BoundLType/BoundRType!", _state);
    if( boundltype==1||boundltype==2 )
    {
        ae_assert(ae_isfinite(boundl, _state), "Spline1DGridDiffCubic: BoundL is infinite or NAN!", _state);
    }
    if( boundrtype==1||boundrtype==2 )
    {
        ae_assert(ae_isfinite(boundr, _state), "Spline1DGridDiffCubic: BoundR is infinite or NAN!", _state);
    }
    
    /*
     * check lengths of arguments
     */
    ae_assert(n>=2, "Spline1DGridDiffCubic: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DGridDiffCubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DGridDiffCubic: Length(Y)<N!", _state);
    
    /*
     * check and sort points
     */
    ylen = n;
    if( boundltype==-1 )
    {
        ylen = n-1;
    }
    ae_assert(isfinitevector(x, n, _state), "Spline1DGridDiffCubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, ylen, _state), "Spline1DGridDiffCubic: Y contains infinite or NAN values!", _state);
    spline1d_heapsortppoints(x, y, &p, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DGridDiffCubic: at least two consequent points are too close!", _state);
    
    /*
     * Now we've checked and preordered everything,
     * so we can call internal function.
     */
    spline1d_spline1dgriddiffcubicinternal(x, y, n, boundltype, boundl, boundrtype, boundr, d, &a1, &a2, &a3, &b, &dt, _state);
    
    /*
     * Remember that HeapSortPPoints() call?
     * Now we have to reorder them back.
     */
    if( dt.cnt<n )
    {
        ae_vector_set_length(&dt, n, _state);
    }
    for(i=0; i<=n-1; i++)
    {
        dt.ptr.p_double[p.ptr.p_int[i]] = d->ptr.p_double[i];
    }
    ae_v_move(&d->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_frame_leave(_state);
}


/*************************************************************************
This function solves following problem: given table y[] of function values
at  nodes  x[],  it  calculates  and  returns  tables  of first and second
function derivatives d1[] and d2[] (calculated at the same nodes x[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   spline nodes
    Y           -   function values

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)

OUTPUT PARAMETERS:
    D1          -   S' values at X[]
    D2          -   S'' values at X[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.
Derivative values are correctly reordered on return, so  D[I]  is  always
equal to S'(X[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dgriddiff2cubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* d1,
     /* Real    */ ae_vector* d2,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector a1;
    ae_vector a2;
    ae_vector a3;
    ae_vector b;
    ae_vector dt;
    ae_vector p;
    ae_int_t i;
    ae_int_t ylen;
    double delta;
    double delta2;
    double delta3;
    double s0;
    double s1;
    double s2;
    double s3;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_clear(d1);
    ae_vector_clear(d2);
    ae_vector_init(&a1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a3, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&p, 0, DT_INT, _state, ae_true);

    
    /*
     * check correctness of boundary conditions
     */
    ae_assert(((boundltype==-1||boundltype==0)||boundltype==1)||boundltype==2, "Spline1DGridDiff2Cubic: incorrect BoundLType!", _state);
    ae_assert(((boundrtype==-1||boundrtype==0)||boundrtype==1)||boundrtype==2, "Spline1DGridDiff2Cubic: incorrect BoundRType!", _state);
    ae_assert((boundrtype==-1&&boundltype==-1)||(boundrtype!=-1&&boundltype!=-1), "Spline1DGridDiff2Cubic: incorrect BoundLType/BoundRType!", _state);
    if( boundltype==1||boundltype==2 )
    {
        ae_assert(ae_isfinite(boundl, _state), "Spline1DGridDiff2Cubic: BoundL is infinite or NAN!", _state);
    }
    if( boundrtype==1||boundrtype==2 )
    {
        ae_assert(ae_isfinite(boundr, _state), "Spline1DGridDiff2Cubic: BoundR is infinite or NAN!", _state);
    }
    
    /*
     * check lengths of arguments
     */
    ae_assert(n>=2, "Spline1DGridDiff2Cubic: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DGridDiff2Cubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DGridDiff2Cubic: Length(Y)<N!", _state);
    
    /*
     * check and sort points
     */
    ylen = n;
    if( boundltype==-1 )
    {
        ylen = n-1;
    }
    ae_assert(isfinitevector(x, n, _state), "Spline1DGridDiff2Cubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, ylen, _state), "Spline1DGridDiff2Cubic: Y contains infinite or NAN values!", _state);
    spline1d_heapsortppoints(x, y, &p, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DGridDiff2Cubic: at least two consequent points are too close!", _state);
    
    /*
     * Now we've checked and preordered everything,
     * so we can call internal function.
     *
     * After this call we will calculate second derivatives
     * (manually, by converting to the power basis)
     */
    spline1d_spline1dgriddiffcubicinternal(x, y, n, boundltype, boundl, boundrtype, boundr, d1, &a1, &a2, &a3, &b, &dt, _state);
    ae_vector_set_length(d2, n, _state);
    delta = 0;
    s2 = 0;
    s3 = 0;
    for(i=0; i<=n-2; i++)
    {
        
        /*
         * We convert from Hermite basis to the power basis.
         * Si is coefficient before x^i.
         *
         * Inside this cycle we need just S2,
         * because we calculate S'' exactly at spline node,
         * (only x^2 matters at x=0), but after iterations
         * will be over, we will need other coefficients
         * to calculate spline value at the last node.
         */
        delta = x->ptr.p_double[i+1]-x->ptr.p_double[i];
        delta2 = ae_sqr(delta, _state);
        delta3 = delta*delta2;
        s0 = y->ptr.p_double[i];
        s1 = d1->ptr.p_double[i];
        s2 = (3*(y->ptr.p_double[i+1]-y->ptr.p_double[i])-2*d1->ptr.p_double[i]*delta-d1->ptr.p_double[i+1]*delta)/delta2;
        s3 = (2*(y->ptr.p_double[i]-y->ptr.p_double[i+1])+d1->ptr.p_double[i]*delta+d1->ptr.p_double[i+1]*delta)/delta3;
        d2->ptr.p_double[i] = 2*s2;
    }
    d2->ptr.p_double[n-1] = 2*s2+6*s3*delta;
    
    /*
     * Remember that HeapSortPPoints() call?
     * Now we have to reorder them back.
     */
    if( dt.cnt<n )
    {
        ae_vector_set_length(&dt, n, _state);
    }
    for(i=0; i<=n-1; i++)
    {
        dt.ptr.p_double[p.ptr.p_int[i]] = d1->ptr.p_double[i];
    }
    ae_v_move(&d1->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n-1));
    for(i=0; i<=n-1; i++)
    {
        dt.ptr.p_double[p.ptr.p_int[i]] = d2->ptr.p_double[i];
    }
    ae_v_move(&d2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_frame_leave(_state);
}


/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function values y2[] (calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* x2,
     ae_int_t n2,
     /* Real    */ ae_vector* y2,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _x2;
    ae_vector a1;
    ae_vector a2;
    ae_vector a3;
    ae_vector b;
    ae_vector d;
    ae_vector dt;
    ae_vector d1;
    ae_vector d2;
    ae_vector p;
    ae_vector p2;
    ae_int_t i;
    ae_int_t ylen;
    double t;
    double t2;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_x2, x2, _state, ae_true);
    x2 = &_x2;
    ae_vector_clear(y2);
    ae_vector_init(&a1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a3, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&p, 0, DT_INT, _state, ae_true);
    ae_vector_init(&p2, 0, DT_INT, _state, ae_true);

    
    /*
     * check correctness of boundary conditions
     */
    ae_assert(((boundltype==-1||boundltype==0)||boundltype==1)||boundltype==2, "Spline1DConvCubic: incorrect BoundLType!", _state);
    ae_assert(((boundrtype==-1||boundrtype==0)||boundrtype==1)||boundrtype==2, "Spline1DConvCubic: incorrect BoundRType!", _state);
    ae_assert((boundrtype==-1&&boundltype==-1)||(boundrtype!=-1&&boundltype!=-1), "Spline1DConvCubic: incorrect BoundLType/BoundRType!", _state);
    if( boundltype==1||boundltype==2 )
    {
        ae_assert(ae_isfinite(boundl, _state), "Spline1DConvCubic: BoundL is infinite or NAN!", _state);
    }
    if( boundrtype==1||boundrtype==2 )
    {
        ae_assert(ae_isfinite(boundr, _state), "Spline1DConvCubic: BoundR is infinite or NAN!", _state);
    }
    
    /*
     * check lengths of arguments
     */
    ae_assert(n>=2, "Spline1DConvCubic: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DConvCubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DConvCubic: Length(Y)<N!", _state);
    ae_assert(n2>=2, "Spline1DConvCubic: N2<2!", _state);
    ae_assert(x2->cnt>=n2, "Spline1DConvCubic: Length(X2)<N2!", _state);
    
    /*
     * check and sort X/Y
     */
    ylen = n;
    if( boundltype==-1 )
    {
        ylen = n-1;
    }
    ae_assert(isfinitevector(x, n, _state), "Spline1DConvCubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, ylen, _state), "Spline1DConvCubic: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(x2, n2, _state), "Spline1DConvCubic: X2 contains infinite or NAN values!", _state);
    spline1d_heapsortppoints(x, y, &p, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DConvCubic: at least two consequent points are too close!", _state);
    
    /*
     * set up DT (we will need it below)
     */
    ae_vector_set_length(&dt, ae_maxint(n, n2, _state), _state);
    
    /*
     * sort X2:
     * * use fake array DT because HeapSortPPoints() needs both integer AND real arrays
     * * if we have periodic problem, wrap points
     * * sort them, store permutation at P2
     */
    if( boundrtype==-1&&boundltype==-1 )
    {
        for(i=0; i<=n2-1; i++)
        {
            t = x2->ptr.p_double[i];
            apperiodicmap(&t, x->ptr.p_double[0], x->ptr.p_double[n-1], &t2, _state);
            x2->ptr.p_double[i] = t;
        }
    }
    spline1d_heapsortppoints(x2, &dt, &p2, n2, _state);
    
    /*
     * Now we've checked and preordered everything, so we:
     * * call internal GridDiff() function to get Hermite form of spline
     * * convert using internal Conv() function
     * * convert Y2 back to original order
     */
    spline1d_spline1dgriddiffcubicinternal(x, y, n, boundltype, boundl, boundrtype, boundr, &d, &a1, &a2, &a3, &b, &dt, _state);
    spline1dconvdiffinternal(x, y, &d, n, x2, n2, y2, ae_true, &d1, ae_false, &d2, ae_false, _state);
    ae_assert(dt.cnt>=n2, "Spline1DConvCubic: internal error!", _state);
    for(i=0; i<=n2-1; i++)
    {
        dt.ptr.p_double[p2.ptr.p_int[i]] = y2->ptr.p_double[i];
    }
    ae_v_move(&y2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n2-1));
    ae_frame_leave(_state);
}


/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function values y2[] and derivatives d2[] (calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]
    D2          -   first derivatives at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiffcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* x2,
     ae_int_t n2,
     /* Real    */ ae_vector* y2,
     /* Real    */ ae_vector* d2,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _x2;
    ae_vector a1;
    ae_vector a2;
    ae_vector a3;
    ae_vector b;
    ae_vector d;
    ae_vector dt;
    ae_vector rt1;
    ae_vector p;
    ae_vector p2;
    ae_int_t i;
    ae_int_t ylen;
    double t;
    double t2;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_x2, x2, _state, ae_true);
    x2 = &_x2;
    ae_vector_clear(y2);
    ae_vector_clear(d2);
    ae_vector_init(&a1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a3, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&rt1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&p, 0, DT_INT, _state, ae_true);
    ae_vector_init(&p2, 0, DT_INT, _state, ae_true);

    
    /*
     * check correctness of boundary conditions
     */
    ae_assert(((boundltype==-1||boundltype==0)||boundltype==1)||boundltype==2, "Spline1DConvDiffCubic: incorrect BoundLType!", _state);
    ae_assert(((boundrtype==-1||boundrtype==0)||boundrtype==1)||boundrtype==2, "Spline1DConvDiffCubic: incorrect BoundRType!", _state);
    ae_assert((boundrtype==-1&&boundltype==-1)||(boundrtype!=-1&&boundltype!=-1), "Spline1DConvDiffCubic: incorrect BoundLType/BoundRType!", _state);
    if( boundltype==1||boundltype==2 )
    {
        ae_assert(ae_isfinite(boundl, _state), "Spline1DConvDiffCubic: BoundL is infinite or NAN!", _state);
    }
    if( boundrtype==1||boundrtype==2 )
    {
        ae_assert(ae_isfinite(boundr, _state), "Spline1DConvDiffCubic: BoundR is infinite or NAN!", _state);
    }
    
    /*
     * check lengths of arguments
     */
    ae_assert(n>=2, "Spline1DConvDiffCubic: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DConvDiffCubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DConvDiffCubic: Length(Y)<N!", _state);
    ae_assert(n2>=2, "Spline1DConvDiffCubic: N2<2!", _state);
    ae_assert(x2->cnt>=n2, "Spline1DConvDiffCubic: Length(X2)<N2!", _state);
    
    /*
     * check and sort X/Y
     */
    ylen = n;
    if( boundltype==-1 )
    {
        ylen = n-1;
    }
    ae_assert(isfinitevector(x, n, _state), "Spline1DConvDiffCubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, ylen, _state), "Spline1DConvDiffCubic: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(x2, n2, _state), "Spline1DConvDiffCubic: X2 contains infinite or NAN values!", _state);
    spline1d_heapsortppoints(x, y, &p, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DConvDiffCubic: at least two consequent points are too close!", _state);
    
    /*
     * set up DT (we will need it below)
     */
    ae_vector_set_length(&dt, ae_maxint(n, n2, _state), _state);
    
    /*
     * sort X2:
     * * use fake array DT because HeapSortPPoints() needs both integer AND real arrays
     * * if we have periodic problem, wrap points
     * * sort them, store permutation at P2
     */
    if( boundrtype==-1&&boundltype==-1 )
    {
        for(i=0; i<=n2-1; i++)
        {
            t = x2->ptr.p_double[i];
            apperiodicmap(&t, x->ptr.p_double[0], x->ptr.p_double[n-1], &t2, _state);
            x2->ptr.p_double[i] = t;
        }
    }
    spline1d_heapsortppoints(x2, &dt, &p2, n2, _state);
    
    /*
     * Now we've checked and preordered everything, so we:
     * * call internal GridDiff() function to get Hermite form of spline
     * * convert using internal Conv() function
     * * convert Y2 back to original order
     */
    spline1d_spline1dgriddiffcubicinternal(x, y, n, boundltype, boundl, boundrtype, boundr, &d, &a1, &a2, &a3, &b, &dt, _state);
    spline1dconvdiffinternal(x, y, &d, n, x2, n2, y2, ae_true, d2, ae_true, &rt1, ae_false, _state);
    ae_assert(dt.cnt>=n2, "Spline1DConvDiffCubic: internal error!", _state);
    for(i=0; i<=n2-1; i++)
    {
        dt.ptr.p_double[p2.ptr.p_int[i]] = y2->ptr.p_double[i];
    }
    ae_v_move(&y2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n2-1));
    for(i=0; i<=n2-1; i++)
    {
        dt.ptr.p_double[p2.ptr.p_int[i]] = d2->ptr.p_double[i];
    }
    ae_v_move(&d2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n2-1));
    ae_frame_leave(_state);
}


/*************************************************************************
This function solves following problem: given table y[] of function values
at old nodes x[]  and new nodes  x2[],  it calculates and returns table of
function  values  y2[],  first  and  second  derivatives  d2[]  and  dd2[]
(calculated at x2[]).

This function yields same result as Spline1DBuildCubic() call followed  by
sequence of Spline1DDiff() calls, but it can be several times faster  when
called for ordered X[] and X2[].

INPUT PARAMETERS:
    X           -   old spline nodes
    Y           -   function values
    X2           -  new spline nodes

OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points from X/Y are used
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundLType  -   boundary condition type for the left boundary
    BoundL      -   left boundary condition (first or second derivative,
                    depending on the BoundLType)
    BoundRType  -   boundary condition type for the right boundary
    BoundR      -   right boundary condition (first or second derivative,
                    depending on the BoundRType)
    N2          -   new points count:
                    * N2>=2
                    * if given, only first N2 points from X2 are used
                    * if not given, automatically detected from X2 size

OUTPUT PARAMETERS:
    F2          -   function values at X2[]
    D2          -   first derivatives at X2[]
    DD2         -   second derivatives at X2[]

ORDER OF POINTS

Subroutine automatically sorts points, so caller  may pass unsorted array.
Function  values  are correctly reordered on  return, so F2[I]  is  always
equal to S(X2[I]) independently of points order.

SETTING BOUNDARY VALUES:

The BoundLType/BoundRType parameters can have the following values:
    * -1, which corresonds to the periodic (cyclic) boundary conditions.
          In this case:
          * both BoundLType and BoundRType must be equal to -1.
          * BoundL/BoundR are ignored
          * Y[last] is ignored (it is assumed to be equal to Y[first]).
    *  0, which  corresponds  to  the  parabolically   terminated  spline
          (BoundL and/or BoundR are ignored).
    *  1, which corresponds to the first derivative boundary condition
    *  2, which corresponds to the second derivative boundary condition
    *  by default, BoundType=0 is used

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiff2cubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* x2,
     ae_int_t n2,
     /* Real    */ ae_vector* y2,
     /* Real    */ ae_vector* d2,
     /* Real    */ ae_vector* dd2,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _x2;
    ae_vector a1;
    ae_vector a2;
    ae_vector a3;
    ae_vector b;
    ae_vector d;
    ae_vector dt;
    ae_vector p;
    ae_vector p2;
    ae_int_t i;
    ae_int_t ylen;
    double t;
    double t2;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_x2, x2, _state, ae_true);
    x2 = &_x2;
    ae_vector_clear(y2);
    ae_vector_clear(d2);
    ae_vector_clear(dd2);
    ae_vector_init(&a1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&a3, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&p, 0, DT_INT, _state, ae_true);
    ae_vector_init(&p2, 0, DT_INT, _state, ae_true);

    
    /*
     * check correctness of boundary conditions
     */
    ae_assert(((boundltype==-1||boundltype==0)||boundltype==1)||boundltype==2, "Spline1DConvDiff2Cubic: incorrect BoundLType!", _state);
    ae_assert(((boundrtype==-1||boundrtype==0)||boundrtype==1)||boundrtype==2, "Spline1DConvDiff2Cubic: incorrect BoundRType!", _state);
    ae_assert((boundrtype==-1&&boundltype==-1)||(boundrtype!=-1&&boundltype!=-1), "Spline1DConvDiff2Cubic: incorrect BoundLType/BoundRType!", _state);
    if( boundltype==1||boundltype==2 )
    {
        ae_assert(ae_isfinite(boundl, _state), "Spline1DConvDiff2Cubic: BoundL is infinite or NAN!", _state);
    }
    if( boundrtype==1||boundrtype==2 )
    {
        ae_assert(ae_isfinite(boundr, _state), "Spline1DConvDiff2Cubic: BoundR is infinite or NAN!", _state);
    }
    
    /*
     * check lengths of arguments
     */
    ae_assert(n>=2, "Spline1DConvDiff2Cubic: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DConvDiff2Cubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DConvDiff2Cubic: Length(Y)<N!", _state);
    ae_assert(n2>=2, "Spline1DConvDiff2Cubic: N2<2!", _state);
    ae_assert(x2->cnt>=n2, "Spline1DConvDiff2Cubic: Length(X2)<N2!", _state);
    
    /*
     * check and sort X/Y
     */
    ylen = n;
    if( boundltype==-1 )
    {
        ylen = n-1;
    }
    ae_assert(isfinitevector(x, n, _state), "Spline1DConvDiff2Cubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, ylen, _state), "Spline1DConvDiff2Cubic: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(x2, n2, _state), "Spline1DConvDiff2Cubic: X2 contains infinite or NAN values!", _state);
    spline1d_heapsortppoints(x, y, &p, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DConvDiff2Cubic: at least two consequent points are too close!", _state);
    
    /*
     * set up DT (we will need it below)
     */
    ae_vector_set_length(&dt, ae_maxint(n, n2, _state), _state);
    
    /*
     * sort X2:
     * * use fake array DT because HeapSortPPoints() needs both integer AND real arrays
     * * if we have periodic problem, wrap points
     * * sort them, store permutation at P2
     */
    if( boundrtype==-1&&boundltype==-1 )
    {
        for(i=0; i<=n2-1; i++)
        {
            t = x2->ptr.p_double[i];
            apperiodicmap(&t, x->ptr.p_double[0], x->ptr.p_double[n-1], &t2, _state);
            x2->ptr.p_double[i] = t;
        }
    }
    spline1d_heapsortppoints(x2, &dt, &p2, n2, _state);
    
    /*
     * Now we've checked and preordered everything, so we:
     * * call internal GridDiff() function to get Hermite form of spline
     * * convert using internal Conv() function
     * * convert Y2 back to original order
     */
    spline1d_spline1dgriddiffcubicinternal(x, y, n, boundltype, boundl, boundrtype, boundr, &d, &a1, &a2, &a3, &b, &dt, _state);
    spline1dconvdiffinternal(x, y, &d, n, x2, n2, y2, ae_true, d2, ae_true, dd2, ae_true, _state);
    ae_assert(dt.cnt>=n2, "Spline1DConvDiff2Cubic: internal error!", _state);
    for(i=0; i<=n2-1; i++)
    {
        dt.ptr.p_double[p2.ptr.p_int[i]] = y2->ptr.p_double[i];
    }
    ae_v_move(&y2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n2-1));
    for(i=0; i<=n2-1; i++)
    {
        dt.ptr.p_double[p2.ptr.p_int[i]] = d2->ptr.p_double[i];
    }
    ae_v_move(&d2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n2-1));
    for(i=0; i<=n2-1; i++)
    {
        dt.ptr.p_double[p2.ptr.p_int[i]] = dd2->ptr.p_double[i];
    }
    ae_v_move(&dd2->ptr.p_double[0], 1, &dt.ptr.p_double[0], 1, ae_v_len(0,n2-1));
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine builds Catmull-Rom spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1].
    Y           -   function values, array[0..N-1].
    
OPTIONAL PARAMETERS:
    N           -   points count:
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))
    BoundType   -   boundary condition type:
                    * -1 for periodic boundary condition
                    *  0 for parabolically terminated spline (default)
    Tension     -   tension parameter:
                    * tension=0   corresponds to classic Catmull-Rom spline (default)
                    * 0<tension<1 corresponds to more general form - cardinal spline

OUTPUT PARAMETERS:
    C           -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

PROBLEMS WITH PERIODIC BOUNDARY CONDITIONS:

Problems with periodic boundary conditions have Y[first_point]=Y[last_point].
However, this subroutine doesn't require you to specify equal  values  for
the first and last points - it automatically forces them  to  be  equal by
copying  Y[first_point]  (corresponds  to the leftmost,  minimal  X[])  to
Y[last_point]. However it is recommended to pass consistent values of Y[],
i.e. to make Y[first_point]=Y[last_point].

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildcatmullrom(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundtype,
     double tension,
     spline1dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector d;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    _spline1dinterpolant_clear(c);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=2, "Spline1DBuildCatmullRom: N<2!", _state);
    ae_assert(boundtype==-1||boundtype==0, "Spline1DBuildCatmullRom: incorrect BoundType!", _state);
    ae_assert(ae_fp_greater_eq(tension,0), "Spline1DBuildCatmullRom: Tension<0!", _state);
    ae_assert(ae_fp_less_eq(tension,1), "Spline1DBuildCatmullRom: Tension>1!", _state);
    ae_assert(x->cnt>=n, "Spline1DBuildCatmullRom: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DBuildCatmullRom: Length(Y)<N!", _state);
    
    /*
     * check and sort points
     */
    ae_assert(isfinitevector(x, n, _state), "Spline1DBuildCatmullRom: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DBuildCatmullRom: Y contains infinite or NAN values!", _state);
    spline1d_heapsortpoints(x, y, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DBuildCatmullRom: at least two consequent points are too close!", _state);
    
    /*
     * Special cases:
     * * N=2, parabolic terminated boundary condition on both ends
     * * N=2, periodic boundary condition
     */
    if( n==2&&boundtype==0 )
    {
        
        /*
         * Just linear spline
         */
        spline1dbuildlinear(x, y, n, c, _state);
        ae_frame_leave(_state);
        return;
    }
    if( n==2&&boundtype==-1 )
    {
        
        /*
         * Same as cubic spline with periodic conditions
         */
        spline1dbuildcubic(x, y, n, -1, 0.0, -1, 0.0, c, _state);
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * Periodic or non-periodic boundary conditions
     */
    if( boundtype==-1 )
    {
        
        /*
         * Periodic boundary conditions
         */
        y->ptr.p_double[n-1] = y->ptr.p_double[0];
        ae_vector_set_length(&d, n, _state);
        d.ptr.p_double[0] = (y->ptr.p_double[1]-y->ptr.p_double[n-2])/(2*(x->ptr.p_double[1]-x->ptr.p_double[0]+x->ptr.p_double[n-1]-x->ptr.p_double[n-2]));
        for(i=1; i<=n-2; i++)
        {
            d.ptr.p_double[i] = (1-tension)*(y->ptr.p_double[i+1]-y->ptr.p_double[i-1])/(x->ptr.p_double[i+1]-x->ptr.p_double[i-1]);
        }
        d.ptr.p_double[n-1] = d.ptr.p_double[0];
        
        /*
         * Now problem is reduced to the cubic Hermite spline
         */
        spline1dbuildhermite(x, y, &d, n, c, _state);
        c->periodic = ae_true;
    }
    else
    {
        
        /*
         * Non-periodic boundary conditions
         */
        ae_vector_set_length(&d, n, _state);
        for(i=1; i<=n-2; i++)
        {
            d.ptr.p_double[i] = (1-tension)*(y->ptr.p_double[i+1]-y->ptr.p_double[i-1])/(x->ptr.p_double[i+1]-x->ptr.p_double[i-1]);
        }
        d.ptr.p_double[0] = 2*(y->ptr.p_double[1]-y->ptr.p_double[0])/(x->ptr.p_double[1]-x->ptr.p_double[0])-d.ptr.p_double[1];
        d.ptr.p_double[n-1] = 2*(y->ptr.p_double[n-1]-y->ptr.p_double[n-2])/(x->ptr.p_double[n-1]-x->ptr.p_double[n-2])-d.ptr.p_double[n-2];
        
        /*
         * Now problem is reduced to the cubic Hermite spline
         */
        spline1dbuildhermite(x, y, &d, n, c, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine builds Hermite spline interpolant.

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1]
    Y           -   function values, array[0..N-1]
    D           -   derivatives, array[0..N-1]
    N           -   points count (optional):
                    * N>=2
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C           -   spline interpolant.


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildhermite(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     spline1dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _d;
    ae_int_t i;
    double delta;
    double delta2;
    double delta3;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_d, d, _state, ae_true);
    d = &_d;
    _spline1dinterpolant_clear(c);

    ae_assert(n>=2, "Spline1DBuildHermite: N<2!", _state);
    ae_assert(x->cnt>=n, "Spline1DBuildHermite: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DBuildHermite: Length(Y)<N!", _state);
    ae_assert(d->cnt>=n, "Spline1DBuildHermite: Length(D)<N!", _state);
    
    /*
     * check and sort points
     */
    ae_assert(isfinitevector(x, n, _state), "Spline1DBuildHermite: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DBuildHermite: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(d, n, _state), "Spline1DBuildHermite: D contains infinite or NAN values!", _state);
    heapsortdpoints(x, y, d, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DBuildHermite: at least two consequent points are too close!", _state);
    
    /*
     * Build
     */
    ae_vector_set_length(&c->x, n, _state);
    ae_vector_set_length(&c->c, 4*(n-1), _state);
    c->periodic = ae_false;
    c->k = 3;
    c->n = n;
    for(i=0; i<=n-1; i++)
    {
        c->x.ptr.p_double[i] = x->ptr.p_double[i];
    }
    for(i=0; i<=n-2; i++)
    {
        delta = x->ptr.p_double[i+1]-x->ptr.p_double[i];
        delta2 = ae_sqr(delta, _state);
        delta3 = delta*delta2;
        c->c.ptr.p_double[4*i+0] = y->ptr.p_double[i];
        c->c.ptr.p_double[4*i+1] = d->ptr.p_double[i];
        c->c.ptr.p_double[4*i+2] = (3*(y->ptr.p_double[i+1]-y->ptr.p_double[i])-2*d->ptr.p_double[i]*delta-d->ptr.p_double[i+1]*delta)/delta2;
        c->c.ptr.p_double[4*i+3] = (2*(y->ptr.p_double[i]-y->ptr.p_double[i+1])+d->ptr.p_double[i]*delta+d->ptr.p_double[i+1]*delta)/delta3;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine builds Akima spline interpolant

INPUT PARAMETERS:
    X           -   spline nodes, array[0..N-1]
    Y           -   function values, array[0..N-1]
    N           -   points count (optional):
                    * N>=5
                    * if given, only first N points are used to build spline
                    * if not given, automatically detected from X/Y sizes
                      (len(X) must be equal to len(Y))

OUTPUT PARAMETERS:
    C           -   spline interpolant


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dbuildakima(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     spline1dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_int_t i;
    ae_vector d;
    ae_vector w;
    ae_vector diff;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    _spline1dinterpolant_clear(c);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&diff, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=5, "Spline1DBuildAkima: N<5!", _state);
    ae_assert(x->cnt>=n, "Spline1DBuildAkima: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DBuildAkima: Length(Y)<N!", _state);
    
    /*
     * check and sort points
     */
    ae_assert(isfinitevector(x, n, _state), "Spline1DBuildAkima: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DBuildAkima: Y contains infinite or NAN values!", _state);
    spline1d_heapsortpoints(x, y, n, _state);
    ae_assert(aredistinct(x, n, _state), "Spline1DBuildAkima: at least two consequent points are too close!", _state);
    
    /*
     * Prepare W (weights), Diff (divided differences)
     */
    ae_vector_set_length(&w, n-1, _state);
    ae_vector_set_length(&diff, n-1, _state);
    for(i=0; i<=n-2; i++)
    {
        diff.ptr.p_double[i] = (y->ptr.p_double[i+1]-y->ptr.p_double[i])/(x->ptr.p_double[i+1]-x->ptr.p_double[i]);
    }
    for(i=1; i<=n-2; i++)
    {
        w.ptr.p_double[i] = ae_fabs(diff.ptr.p_double[i]-diff.ptr.p_double[i-1], _state);
    }
    
    /*
     * Prepare Hermite interpolation scheme
     */
    ae_vector_set_length(&d, n, _state);
    for(i=2; i<=n-3; i++)
    {
        if( ae_fp_neq(ae_fabs(w.ptr.p_double[i-1], _state)+ae_fabs(w.ptr.p_double[i+1], _state),0) )
        {
            d.ptr.p_double[i] = (w.ptr.p_double[i+1]*diff.ptr.p_double[i-1]+w.ptr.p_double[i-1]*diff.ptr.p_double[i])/(w.ptr.p_double[i+1]+w.ptr.p_double[i-1]);
        }
        else
        {
            d.ptr.p_double[i] = ((x->ptr.p_double[i+1]-x->ptr.p_double[i])*diff.ptr.p_double[i-1]+(x->ptr.p_double[i]-x->ptr.p_double[i-1])*diff.ptr.p_double[i])/(x->ptr.p_double[i+1]-x->ptr.p_double[i-1]);
        }
    }
    d.ptr.p_double[0] = spline1d_diffthreepoint(x->ptr.p_double[0], x->ptr.p_double[0], y->ptr.p_double[0], x->ptr.p_double[1], y->ptr.p_double[1], x->ptr.p_double[2], y->ptr.p_double[2], _state);
    d.ptr.p_double[1] = spline1d_diffthreepoint(x->ptr.p_double[1], x->ptr.p_double[0], y->ptr.p_double[0], x->ptr.p_double[1], y->ptr.p_double[1], x->ptr.p_double[2], y->ptr.p_double[2], _state);
    d.ptr.p_double[n-2] = spline1d_diffthreepoint(x->ptr.p_double[n-2], x->ptr.p_double[n-3], y->ptr.p_double[n-3], x->ptr.p_double[n-2], y->ptr.p_double[n-2], x->ptr.p_double[n-1], y->ptr.p_double[n-1], _state);
    d.ptr.p_double[n-1] = spline1d_diffthreepoint(x->ptr.p_double[n-1], x->ptr.p_double[n-3], y->ptr.p_double[n-3], x->ptr.p_double[n-2], y->ptr.p_double[n-2], x->ptr.p_double[n-1], y->ptr.p_double[n-1], _state);
    
    /*
     * Build Akima spline using Hermite interpolation scheme
     */
    spline1dbuildhermite(x, y, &d, n, c, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine calculates the value of the spline at the given point X.

INPUT PARAMETERS:
    C   -   spline interpolant
    X   -   point

Result:
    S(x)

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
double spline1dcalc(spline1dinterpolant* c, double x, ae_state *_state)
{
    ae_int_t l;
    ae_int_t r;
    ae_int_t m;
    double t;
    double result;


    ae_assert(c->k==3, "Spline1DCalc: internal error", _state);
    ae_assert(!ae_isinf(x, _state), "Spline1DCalc: infinite X!", _state);
    
    /*
     * special case: NaN
     */
    if( ae_isnan(x, _state) )
    {
        result = _state->v_nan;
        return result;
    }
    
    /*
     * correct if periodic
     */
    if( c->periodic )
    {
        apperiodicmap(&x, c->x.ptr.p_double[0], c->x.ptr.p_double[c->n-1], &t, _state);
    }
    
    /*
     * Binary search in the [ x[0], ..., x[n-2] ] (x[n-1] is not included)
     */
    l = 0;
    r = c->n-2+1;
    while(l!=r-1)
    {
        m = (l+r)/2;
        if( c->x.ptr.p_double[m]>=x )
        {
            r = m;
        }
        else
        {
            l = m;
        }
    }
    
    /*
     * Interpolation
     */
    x = x-c->x.ptr.p_double[l];
    m = 4*l;
    result = c->c.ptr.p_double[m]+x*(c->c.ptr.p_double[m+1]+x*(c->c.ptr.p_double[m+2]+x*c->c.ptr.p_double[m+3]));
    return result;
}


/*************************************************************************
This subroutine differentiates the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.
    X   -   point

Result:
    S   -   S(x)
    DS  -   S'(x)
    D2S -   S''(x)

  -- ALGLIB PROJECT --
     Copyright 24.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1ddiff(spline1dinterpolant* c,
     double x,
     double* s,
     double* ds,
     double* d2s,
     ae_state *_state)
{
    ae_int_t l;
    ae_int_t r;
    ae_int_t m;
    double t;

    *s = 0;
    *ds = 0;
    *d2s = 0;

    ae_assert(c->k==3, "Spline1DDiff: internal error", _state);
    ae_assert(!ae_isinf(x, _state), "Spline1DDiff: infinite X!", _state);
    
    /*
     * special case: NaN
     */
    if( ae_isnan(x, _state) )
    {
        *s = _state->v_nan;
        *ds = _state->v_nan;
        *d2s = _state->v_nan;
        return;
    }
    
    /*
     * correct if periodic
     */
    if( c->periodic )
    {
        apperiodicmap(&x, c->x.ptr.p_double[0], c->x.ptr.p_double[c->n-1], &t, _state);
    }
    
    /*
     * Binary search
     */
    l = 0;
    r = c->n-2+1;
    while(l!=r-1)
    {
        m = (l+r)/2;
        if( c->x.ptr.p_double[m]>=x )
        {
            r = m;
        }
        else
        {
            l = m;
        }
    }
    
    /*
     * Differentiation
     */
    x = x-c->x.ptr.p_double[l];
    m = 4*l;
    *s = c->c.ptr.p_double[m]+x*(c->c.ptr.p_double[m+1]+x*(c->c.ptr.p_double[m+2]+x*c->c.ptr.p_double[m+3]));
    *ds = c->c.ptr.p_double[m+1]+2*x*c->c.ptr.p_double[m+2]+3*ae_sqr(x, _state)*c->c.ptr.p_double[m+3];
    *d2s = 2*c->c.ptr.p_double[m+2]+6*x*c->c.ptr.p_double[m+3];
}


/*************************************************************************
This subroutine makes the copy of the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.

Result:
    CC  -   spline copy

  -- ALGLIB PROJECT --
     Copyright 29.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dcopy(spline1dinterpolant* c,
     spline1dinterpolant* cc,
     ae_state *_state)
{

    _spline1dinterpolant_clear(cc);

    cc->periodic = c->periodic;
    cc->n = c->n;
    cc->k = c->k;
    ae_vector_set_length(&cc->x, cc->n, _state);
    ae_v_move(&cc->x.ptr.p_double[0], 1, &c->x.ptr.p_double[0], 1, ae_v_len(0,cc->n-1));
    ae_vector_set_length(&cc->c, (cc->k+1)*(cc->n-1), _state);
    ae_v_move(&cc->c.ptr.p_double[0], 1, &c->c.ptr.p_double[0], 1, ae_v_len(0,(cc->k+1)*(cc->n-1)-1));
}


/*************************************************************************
This subroutine unpacks the spline into the coefficients table.

INPUT PARAMETERS:
    C   -   spline interpolant.
    X   -   point

Result:
    Tbl -   coefficients table, unpacked format, array[0..N-2, 0..5].
            For I = 0...N-2:
                Tbl[I,0] = X[i]
                Tbl[I,1] = X[i+1]
                Tbl[I,2] = C0
                Tbl[I,3] = C1
                Tbl[I,4] = C2
                Tbl[I,5] = C3
            On [x[i], x[i+1]] spline is equals to:
                S(x) = C0 + C1*t + C2*t^2 + C3*t^3
                t = x-x[i]

  -- ALGLIB PROJECT --
     Copyright 29.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dunpack(spline1dinterpolant* c,
     ae_int_t* n,
     /* Real    */ ae_matrix* tbl,
     ae_state *_state)
{
    ae_int_t i;
    ae_int_t j;

    *n = 0;
    ae_matrix_clear(tbl);

    ae_matrix_set_length(tbl, c->n-2+1, 2+c->k+1, _state);
    *n = c->n;
    
    /*
     * Fill
     */
    for(i=0; i<=*n-2; i++)
    {
        tbl->ptr.pp_double[i][0] = c->x.ptr.p_double[i];
        tbl->ptr.pp_double[i][1] = c->x.ptr.p_double[i+1];
        for(j=0; j<=c->k; j++)
        {
            tbl->ptr.pp_double[i][2+j] = c->c.ptr.p_double[(c->k+1)*i+j];
        }
    }
}


/*************************************************************************
This subroutine performs linear transformation of the spline argument.

INPUT PARAMETERS:
    C   -   spline interpolant.
    A, B-   transformation coefficients: x = A*t + B
Result:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dlintransx(spline1dinterpolant* c,
     double a,
     double b,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    ae_int_t n;
    double v;
    double dv;
    double d2v;
    ae_vector x;
    ae_vector y;
    ae_vector d;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&d, 0, DT_REAL, _state, ae_true);

    n = c->n;
    
    /*
     * Special case: A=0
     */
    if( ae_fp_eq(a,0) )
    {
        v = spline1dcalc(c, b, _state);
        for(i=0; i<=n-2; i++)
        {
            c->c.ptr.p_double[(c->k+1)*i] = v;
            for(j=1; j<=c->k; j++)
            {
                c->c.ptr.p_double[(c->k+1)*i+j] = 0;
            }
        }
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * General case: A<>0.
     * Unpack, X, Y, dY/dX.
     * Scale and pack again.
     */
    ae_assert(c->k==3, "Spline1DLinTransX: internal error", _state);
    ae_vector_set_length(&x, n-1+1, _state);
    ae_vector_set_length(&y, n-1+1, _state);
    ae_vector_set_length(&d, n-1+1, _state);
    for(i=0; i<=n-1; i++)
    {
        x.ptr.p_double[i] = c->x.ptr.p_double[i];
        spline1ddiff(c, x.ptr.p_double[i], &v, &dv, &d2v, _state);
        x.ptr.p_double[i] = (x.ptr.p_double[i]-b)/a;
        y.ptr.p_double[i] = v;
        d.ptr.p_double[i] = a*dv;
    }
    spline1dbuildhermite(&x, &y, &d, n, c, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine performs linear transformation of the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.
    A, B-   transformation coefficients: S2(x) = A*S(x) + B
Result:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline1dlintransy(spline1dinterpolant* c,
     double a,
     double b,
     ae_state *_state)
{
    ae_int_t i;
    ae_int_t j;
    ae_int_t n;


    n = c->n;
    for(i=0; i<=n-2; i++)
    {
        c->c.ptr.p_double[(c->k+1)*i] = a*c->c.ptr.p_double[(c->k+1)*i]+b;
        for(j=1; j<=c->k; j++)
        {
            c->c.ptr.p_double[(c->k+1)*i+j] = a*c->c.ptr.p_double[(c->k+1)*i+j];
        }
    }
}


/*************************************************************************
This subroutine integrates the spline.

INPUT PARAMETERS:
    C   -   spline interpolant.
    X   -   right bound of the integration interval [a, x],
            here 'a' denotes min(x[])
Result:
    integral(S(t)dt,a,x)

  -- ALGLIB PROJECT --
     Copyright 23.06.2007 by Bochkanov Sergey
*************************************************************************/
double spline1dintegrate(spline1dinterpolant* c,
     double x,
     ae_state *_state)
{
    ae_int_t n;
    ae_int_t i;
    ae_int_t j;
    ae_int_t l;
    ae_int_t r;
    ae_int_t m;
    double w;
    double v;
    double t;
    double intab;
    double additionalterm;
    double result;


    n = c->n;
    
    /*
     * Periodic splines require special treatment. We make
     * following transformation:
     *
     *     integral(S(t)dt,A,X) = integral(S(t)dt,A,Z)+AdditionalTerm
     *
     * here X may lie outside of [A,B], Z lies strictly in [A,B],
     * AdditionalTerm is equals to integral(S(t)dt,A,B) times some
     * integer number (may be zero).
     */
    if( c->periodic&&(ae_fp_less(x,c->x.ptr.p_double[0])||ae_fp_greater(x,c->x.ptr.p_double[c->n-1])) )
    {
        
        /*
         * compute integral(S(x)dx,A,B)
         */
        intab = 0;
        for(i=0; i<=c->n-2; i++)
        {
            w = c->x.ptr.p_double[i+1]-c->x.ptr.p_double[i];
            m = (c->k+1)*i;
            intab = intab+c->c.ptr.p_double[m]*w;
            v = w;
            for(j=1; j<=c->k; j++)
            {
                v = v*w;
                intab = intab+c->c.ptr.p_double[m+j]*v/(j+1);
            }
        }
        
        /*
         * map X into [A,B]
         */
        apperiodicmap(&x, c->x.ptr.p_double[0], c->x.ptr.p_double[c->n-1], &t, _state);
        additionalterm = t*intab;
    }
    else
    {
        additionalterm = 0;
    }
    
    /*
     * Binary search in the [ x[0], ..., x[n-2] ] (x[n-1] is not included)
     */
    l = 0;
    r = n-2+1;
    while(l!=r-1)
    {
        m = (l+r)/2;
        if( ae_fp_greater_eq(c->x.ptr.p_double[m],x) )
        {
            r = m;
        }
        else
        {
            l = m;
        }
    }
    
    /*
     * Integration
     */
    result = 0;
    for(i=0; i<=l-1; i++)
    {
        w = c->x.ptr.p_double[i+1]-c->x.ptr.p_double[i];
        m = (c->k+1)*i;
        result = result+c->c.ptr.p_double[m]*w;
        v = w;
        for(j=1; j<=c->k; j++)
        {
            v = v*w;
            result = result+c->c.ptr.p_double[m+j]*v/(j+1);
        }
    }
    w = x-c->x.ptr.p_double[l];
    m = (c->k+1)*l;
    v = w;
    result = result+c->c.ptr.p_double[m]*w;
    for(j=1; j<=c->k; j++)
    {
        v = v*w;
        result = result+c->c.ptr.p_double[m+j]*v/(j+1);
    }
    result = result+additionalterm;
    return result;
}


/*************************************************************************
Internal version of Spline1DConvDiff

Converts from Hermite spline given by grid XOld to new grid X2

INPUT PARAMETERS:
    XOld    -   old grid
    YOld    -   values at old grid
    DOld    -   first derivative at old grid
    N       -   grid size
    X2      -   new grid
    N2      -   new grid size
    Y       -   possibly preallocated output array
                (reallocate if too small)
    NeedY   -   do we need Y?
    D1      -   possibly preallocated output array
                (reallocate if too small)
    NeedD1  -   do we need D1?
    D2      -   possibly preallocated output array
                (reallocate if too small)
    NeedD2  -   do we need D1?

OUTPUT ARRAYS:
    Y       -   values, if needed
    D1      -   first derivative, if needed
    D2      -   second derivative, if needed

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dconvdiffinternal(/* Real    */ ae_vector* xold,
     /* Real    */ ae_vector* yold,
     /* Real    */ ae_vector* dold,
     ae_int_t n,
     /* Real    */ ae_vector* x2,
     ae_int_t n2,
     /* Real    */ ae_vector* y,
     ae_bool needy,
     /* Real    */ ae_vector* d1,
     ae_bool needd1,
     /* Real    */ ae_vector* d2,
     ae_bool needd2,
     ae_state *_state)
{
    ae_int_t intervalindex;
    ae_int_t pointindex;
    ae_bool havetoadvance;
    double c0;
    double c1;
    double c2;
    double c3;
    double a;
    double b;
    double w;
    double w2;
    double w3;
    double fa;
    double fb;
    double da;
    double db;
    double t;


    
    /*
     * Prepare space
     */
    if( needy&&y->cnt<n2 )
    {
        ae_vector_set_length(y, n2, _state);
    }
    if( needd1&&d1->cnt<n2 )
    {
        ae_vector_set_length(d1, n2, _state);
    }
    if( needd2&&d2->cnt<n2 )
    {
        ae_vector_set_length(d2, n2, _state);
    }
    
    /*
     * These assignments aren't actually needed
     * (variables are initialized in the loop below),
     * but without them compiler will complain about uninitialized locals
     */
    c0 = 0;
    c1 = 0;
    c2 = 0;
    c3 = 0;
    a = 0;
    b = 0;
    
    /*
     * Cycle
     */
    intervalindex = -1;
    pointindex = 0;
    for(;;)
    {
        
        /*
         * are we ready to exit?
         */
        if( pointindex>=n2 )
        {
            break;
        }
        t = x2->ptr.p_double[pointindex];
        
        /*
         * do we need to advance interval?
         */
        havetoadvance = ae_false;
        if( intervalindex==-1 )
        {
            havetoadvance = ae_true;
        }
        else
        {
            if( intervalindex<n-2 )
            {
                havetoadvance = ae_fp_greater_eq(t,b);
            }
        }
        if( havetoadvance )
        {
            intervalindex = intervalindex+1;
            a = xold->ptr.p_double[intervalindex];
            b = xold->ptr.p_double[intervalindex+1];
            w = b-a;
            w2 = w*w;
            w3 = w*w2;
            fa = yold->ptr.p_double[intervalindex];
            fb = yold->ptr.p_double[intervalindex+1];
            da = dold->ptr.p_double[intervalindex];
            db = dold->ptr.p_double[intervalindex+1];
            c0 = fa;
            c1 = da;
            c2 = (3*(fb-fa)-2*da*w-db*w)/w2;
            c3 = (2*(fa-fb)+da*w+db*w)/w3;
            continue;
        }
        
        /*
         * Calculate spline and its derivatives using power basis
         */
        t = t-a;
        if( needy )
        {
            y->ptr.p_double[pointindex] = c0+t*(c1+t*(c2+t*c3));
        }
        if( needd1 )
        {
            d1->ptr.p_double[pointindex] = c1+2*t*c2+3*t*t*c3;
        }
        if( needd2 )
        {
            d2->ptr.p_double[pointindex] = 2*c2+6*t*c3;
        }
        pointindex = pointindex+1;
    }
}


/*************************************************************************
Internal subroutine. Heap sort.
*************************************************************************/
void heapsortdpoints(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector rbuf;
    ae_vector ibuf;
    ae_vector rbuf2;
    ae_vector ibuf2;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&rbuf, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&ibuf, 0, DT_INT, _state, ae_true);
    ae_vector_init(&rbuf2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&ibuf2, 0, DT_INT, _state, ae_true);

    ae_vector_set_length(&ibuf, n, _state);
    ae_vector_set_length(&rbuf, n, _state);
    for(i=0; i<=n-1; i++)
    {
        ibuf.ptr.p_int[i] = i;
    }
    tagsortfasti(x, &ibuf, &rbuf2, &ibuf2, n, _state);
    for(i=0; i<=n-1; i++)
    {
        rbuf.ptr.p_double[i] = y->ptr.p_double[ibuf.ptr.p_int[i]];
    }
    ae_v_move(&y->ptr.p_double[0], 1, &rbuf.ptr.p_double[0], 1, ae_v_len(0,n-1));
    for(i=0; i<=n-1; i++)
    {
        rbuf.ptr.p_double[i] = d->ptr.p_double[ibuf.ptr.p_int[i]];
    }
    ae_v_move(&d->ptr.p_double[0], 1, &rbuf.ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_frame_leave(_state);
}


/*************************************************************************
Internal version of Spline1DGridDiffCubic.

Accepts pre-ordered X/Y, temporary arrays (which may be  preallocated,  if
you want to save time, or not) and output array (which may be preallocated
too).

Y is passed as var-parameter because we may need to force last element  to
be equal to the first one (if periodic boundary conditions are specified).

  -- ALGLIB PROJECT --
     Copyright 03.09.2010 by Bochkanov Sergey
*************************************************************************/
static void spline1d_spline1dgriddiffcubicinternal(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* d,
     /* Real    */ ae_vector* a1,
     /* Real    */ ae_vector* a2,
     /* Real    */ ae_vector* a3,
     /* Real    */ ae_vector* b,
     /* Real    */ ae_vector* dt,
     ae_state *_state)
{
    ae_int_t i;


    
    /*
     * allocate arrays
     */
    if( d->cnt<n )
    {
        ae_vector_set_length(d, n, _state);
    }
    if( a1->cnt<n )
    {
        ae_vector_set_length(a1, n, _state);
    }
    if( a2->cnt<n )
    {
        ae_vector_set_length(a2, n, _state);
    }
    if( a3->cnt<n )
    {
        ae_vector_set_length(a3, n, _state);
    }
    if( b->cnt<n )
    {
        ae_vector_set_length(b, n, _state);
    }
    if( dt->cnt<n )
    {
        ae_vector_set_length(dt, n, _state);
    }
    
    /*
     * Special cases:
     * * N=2, parabolic terminated boundary condition on both ends
     * * N=2, periodic boundary condition
     */
    if( (n==2&&boundltype==0)&&boundrtype==0 )
    {
        d->ptr.p_double[0] = (y->ptr.p_double[1]-y->ptr.p_double[0])/(x->ptr.p_double[1]-x->ptr.p_double[0]);
        d->ptr.p_double[1] = d->ptr.p_double[0];
        return;
    }
    if( (n==2&&boundltype==-1)&&boundrtype==-1 )
    {
        d->ptr.p_double[0] = 0;
        d->ptr.p_double[1] = 0;
        return;
    }
    
    /*
     * Periodic and non-periodic boundary conditions are
     * two separate classes
     */
    if( boundrtype==-1&&boundltype==-1 )
    {
        
        /*
         * Periodic boundary conditions
         */
        y->ptr.p_double[n-1] = y->ptr.p_double[0];
        
        /*
         * Boundary conditions at N-1 points
         * (one point less because last point is the same as first point).
         */
        a1->ptr.p_double[0] = x->ptr.p_double[1]-x->ptr.p_double[0];
        a2->ptr.p_double[0] = 2*(x->ptr.p_double[1]-x->ptr.p_double[0]+x->ptr.p_double[n-1]-x->ptr.p_double[n-2]);
        a3->ptr.p_double[0] = x->ptr.p_double[n-1]-x->ptr.p_double[n-2];
        b->ptr.p_double[0] = 3*(y->ptr.p_double[n-1]-y->ptr.p_double[n-2])/(x->ptr.p_double[n-1]-x->ptr.p_double[n-2])*(x->ptr.p_double[1]-x->ptr.p_double[0])+3*(y->ptr.p_double[1]-y->ptr.p_double[0])/(x->ptr.p_double[1]-x->ptr.p_double[0])*(x->ptr.p_double[n-1]-x->ptr.p_double[n-2]);
        for(i=1; i<=n-2; i++)
        {
            
            /*
             * Altough last point is [N-2], we use X[N-1] and Y[N-1]
             * (because of periodicity)
             */
            a1->ptr.p_double[i] = x->ptr.p_double[i+1]-x->ptr.p_double[i];
            a2->ptr.p_double[i] = 2*(x->ptr.p_double[i+1]-x->ptr.p_double[i-1]);
            a3->ptr.p_double[i] = x->ptr.p_double[i]-x->ptr.p_double[i-1];
            b->ptr.p_double[i] = 3*(y->ptr.p_double[i]-y->ptr.p_double[i-1])/(x->ptr.p_double[i]-x->ptr.p_double[i-1])*(x->ptr.p_double[i+1]-x->ptr.p_double[i])+3*(y->ptr.p_double[i+1]-y->ptr.p_double[i])/(x->ptr.p_double[i+1]-x->ptr.p_double[i])*(x->ptr.p_double[i]-x->ptr.p_double[i-1]);
        }
        
        /*
         * Solve, add last point (with index N-1)
         */
        spline1d_solvecyclictridiagonal(a1, a2, a3, b, n-1, dt, _state);
        ae_v_move(&d->ptr.p_double[0], 1, &dt->ptr.p_double[0], 1, ae_v_len(0,n-2));
        d->ptr.p_double[n-1] = d->ptr.p_double[0];
    }
    else
    {
        
        /*
         * Non-periodic boundary condition.
         * Left boundary conditions.
         */
        if( boundltype==0 )
        {
            a1->ptr.p_double[0] = 0;
            a2->ptr.p_double[0] = 1;
            a3->ptr.p_double[0] = 1;
            b->ptr.p_double[0] = 2*(y->ptr.p_double[1]-y->ptr.p_double[0])/(x->ptr.p_double[1]-x->ptr.p_double[0]);
        }
        if( boundltype==1 )
        {
            a1->ptr.p_double[0] = 0;
            a2->ptr.p_double[0] = 1;
            a3->ptr.p_double[0] = 0;
            b->ptr.p_double[0] = boundl;
        }
        if( boundltype==2 )
        {
            a1->ptr.p_double[0] = 0;
            a2->ptr.p_double[0] = 2;
            a3->ptr.p_double[0] = 1;
            b->ptr.p_double[0] = 3*(y->ptr.p_double[1]-y->ptr.p_double[0])/(x->ptr.p_double[1]-x->ptr.p_double[0])-0.5*boundl*(x->ptr.p_double[1]-x->ptr.p_double[0]);
        }
        
        /*
         * Central conditions
         */
        for(i=1; i<=n-2; i++)
        {
            a1->ptr.p_double[i] = x->ptr.p_double[i+1]-x->ptr.p_double[i];
            a2->ptr.p_double[i] = 2*(x->ptr.p_double[i+1]-x->ptr.p_double[i-1]);
            a3->ptr.p_double[i] = x->ptr.p_double[i]-x->ptr.p_double[i-1];
            b->ptr.p_double[i] = 3*(y->ptr.p_double[i]-y->ptr.p_double[i-1])/(x->ptr.p_double[i]-x->ptr.p_double[i-1])*(x->ptr.p_double[i+1]-x->ptr.p_double[i])+3*(y->ptr.p_double[i+1]-y->ptr.p_double[i])/(x->ptr.p_double[i+1]-x->ptr.p_double[i])*(x->ptr.p_double[i]-x->ptr.p_double[i-1]);
        }
        
        /*
         * Right boundary conditions
         */
        if( boundrtype==0 )
        {
            a1->ptr.p_double[n-1] = 1;
            a2->ptr.p_double[n-1] = 1;
            a3->ptr.p_double[n-1] = 0;
            b->ptr.p_double[n-1] = 2*(y->ptr.p_double[n-1]-y->ptr.p_double[n-2])/(x->ptr.p_double[n-1]-x->ptr.p_double[n-2]);
        }
        if( boundrtype==1 )
        {
            a1->ptr.p_double[n-1] = 0;
            a2->ptr.p_double[n-1] = 1;
            a3->ptr.p_double[n-1] = 0;
            b->ptr.p_double[n-1] = boundr;
        }
        if( boundrtype==2 )
        {
            a1->ptr.p_double[n-1] = 1;
            a2->ptr.p_double[n-1] = 2;
            a3->ptr.p_double[n-1] = 0;
            b->ptr.p_double[n-1] = 3*(y->ptr.p_double[n-1]-y->ptr.p_double[n-2])/(x->ptr.p_double[n-1]-x->ptr.p_double[n-2])+0.5*boundr*(x->ptr.p_double[n-1]-x->ptr.p_double[n-2]);
        }
        
        /*
         * Solve
         */
        spline1d_solvetridiagonal(a1, a2, a3, b, n, d, _state);
    }
}


/*************************************************************************
Internal subroutine. Heap sort.
*************************************************************************/
static void spline1d_heapsortpoints(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector bufx;
    ae_vector bufy;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&bufx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bufy, 0, DT_REAL, _state, ae_true);

    tagsortfastr(x, y, &bufx, &bufy, n, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Internal subroutine. Heap sort.

Accepts:
    X, Y    -   points
    P       -   empty or preallocated array
    
Returns:
    X, Y    -   sorted by X
    P       -   array of permutations; I-th position of output
                arrays X/Y contains (X[P[I]],Y[P[I]])
*************************************************************************/
static void spline1d_heapsortppoints(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Integer */ ae_vector* p,
     ae_int_t n,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector rbuf;
    ae_vector ibuf;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&rbuf, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&ibuf, 0, DT_INT, _state, ae_true);

    if( p->cnt<n )
    {
        ae_vector_set_length(p, n, _state);
    }
    ae_vector_set_length(&rbuf, n, _state);
    for(i=0; i<=n-1; i++)
    {
        p->ptr.p_int[i] = i;
    }
    tagsortfasti(x, p, &rbuf, &ibuf, n, _state);
    for(i=0; i<=n-1; i++)
    {
        rbuf.ptr.p_double[i] = y->ptr.p_double[p->ptr.p_int[i]];
    }
    ae_v_move(&y->ptr.p_double[0], 1, &rbuf.ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_frame_leave(_state);
}


/*************************************************************************
Internal subroutine. Tridiagonal solver. Solves

( B[0] C[0]                      )
( A[1] B[1] C[1]                 )
(      A[2] B[2] C[2]            )
(            ..........          ) * X = D
(            ..........          )
(           A[N-2] B[N-2] C[N-2] )
(                  A[N-1] B[N-1] )

*************************************************************************/
static void spline1d_solvetridiagonal(/* Real    */ ae_vector* a,
     /* Real    */ ae_vector* b,
     /* Real    */ ae_vector* c,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     /* Real    */ ae_vector* x,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _b;
    ae_vector _d;
    ae_int_t k;
    double t;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_b, b, _state, ae_true);
    b = &_b;
    ae_vector_init_copy(&_d, d, _state, ae_true);
    d = &_d;

    if( x->cnt<n )
    {
        ae_vector_set_length(x, n, _state);
    }
    for(k=1; k<=n-1; k++)
    {
        t = a->ptr.p_double[k]/b->ptr.p_double[k-1];
        b->ptr.p_double[k] = b->ptr.p_double[k]-t*c->ptr.p_double[k-1];
        d->ptr.p_double[k] = d->ptr.p_double[k]-t*d->ptr.p_double[k-1];
    }
    x->ptr.p_double[n-1] = d->ptr.p_double[n-1]/b->ptr.p_double[n-1];
    for(k=n-2; k>=0; k--)
    {
        x->ptr.p_double[k] = (d->ptr.p_double[k]-c->ptr.p_double[k]*x->ptr.p_double[k+1])/b->ptr.p_double[k];
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Internal subroutine. Cyclic tridiagonal solver. Solves

( B[0] C[0]                 A[0] )
( A[1] B[1] C[1]                 )
(      A[2] B[2] C[2]            )
(            ..........          ) * X = D
(            ..........          )
(           A[N-2] B[N-2] C[N-2] )
( C[N-1]           A[N-1] B[N-1] )
*************************************************************************/
static void spline1d_solvecyclictridiagonal(/* Real    */ ae_vector* a,
     /* Real    */ ae_vector* b,
     /* Real    */ ae_vector* c,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     /* Real    */ ae_vector* x,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _b;
    ae_int_t k;
    double alpha;
    double beta;
    double gamma;
    ae_vector y;
    ae_vector z;
    ae_vector u;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_b, b, _state, ae_true);
    b = &_b;
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&z, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&u, 0, DT_REAL, _state, ae_true);

    if( x->cnt<n )
    {
        ae_vector_set_length(x, n, _state);
    }
    beta = a->ptr.p_double[0];
    alpha = c->ptr.p_double[n-1];
    gamma = -b->ptr.p_double[0];
    b->ptr.p_double[0] = 2*b->ptr.p_double[0];
    b->ptr.p_double[n-1] = b->ptr.p_double[n-1]-alpha*beta/gamma;
    ae_vector_set_length(&u, n, _state);
    for(k=0; k<=n-1; k++)
    {
        u.ptr.p_double[k] = 0;
    }
    u.ptr.p_double[0] = gamma;
    u.ptr.p_double[n-1] = alpha;
    spline1d_solvetridiagonal(a, b, c, d, n, &y, _state);
    spline1d_solvetridiagonal(a, b, c, &u, n, &z, _state);
    for(k=0; k<=n-1; k++)
    {
        x->ptr.p_double[k] = y.ptr.p_double[k]-(y.ptr.p_double[0]+beta/gamma*y.ptr.p_double[n-1])/(1+z.ptr.p_double[0]+beta/gamma*z.ptr.p_double[n-1])*z.ptr.p_double[k];
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Internal subroutine. Three-point differentiation
*************************************************************************/
static double spline1d_diffthreepoint(double t,
     double x0,
     double f0,
     double x1,
     double f1,
     double x2,
     double f2,
     ae_state *_state)
{
    double a;
    double b;
    double result;


    t = t-x0;
    x1 = x1-x0;
    x2 = x2-x0;
    a = (f2-f0-x2/x1*(f1-f0))/(ae_sqr(x2, _state)-x1*x2);
    b = (f1-f0-a*ae_sqr(x1, _state))/x1;
    result = 2*a*t+b;
    return result;
}


ae_bool _spline1dinterpolant_init(spline1dinterpolant* p, ae_state *_state, ae_bool make_automatic)
{
    if( !ae_vector_init(&p->x, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->c, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _spline1dinterpolant_init_copy(spline1dinterpolant* dst, spline1dinterpolant* src, ae_state *_state, ae_bool make_automatic)
{
    dst->periodic = src->periodic;
    dst->n = src->n;
    dst->k = src->k;
    if( !ae_vector_init_copy(&dst->x, &src->x, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->c, &src->c, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


void _spline1dinterpolant_clear(spline1dinterpolant* p)
{
    ae_vector_clear(&p->x);
    ae_vector_clear(&p->c);
}




/*************************************************************************
Fitting by polynomials in barycentric form. This function provides  simple
unterface for unconstrained unweighted fitting. See  PolynomialFitWC()  if
you need constrained fitting.

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO:
    PolynomialFitWC()

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0
            * if given, only leading N elements of X/Y are used
            * if not given, automatically determined from sizes of X/Y
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
    P   -   interpolant in barycentric form.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

NOTES:
    you can convert P from barycentric form  to  the  power  or  Chebyshev
    basis with PolynomialBar2Pow() or PolynomialBar2Cheb() functions  from
    POLINT subpackage.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialfit(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     barycentricinterpolant* p,
     polynomialfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector w;
    ae_vector xc;
    ae_vector yc;
    ae_vector dc;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    _barycentricinterpolant_clear(p);
    _polynomialfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dc, 0, DT_INT, _state, ae_true);

    ae_assert(n>0, "PolynomialFit: N<=0!", _state);
    ae_assert(m>0, "PolynomialFit: M<=0!", _state);
    ae_assert(x->cnt>=n, "PolynomialFit: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "PolynomialFit: Length(Y)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "PolynomialFit: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "PolynomialFit: Y contains infinite or NaN values!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    polynomialfitwc(x, y, &w, n, &xc, &yc, &dc, 0, m, info, p, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Weighted  fitting by polynomials in barycentric form, with constraints  on
function values or first derivatives.

Small regularizing term is used when solving constrained tasks (to improve
stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO:
    PolynomialFit()

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points, N>0.
            * if given, only leading N elements of X/Y/W are used
            * if not given, automatically determined from sizes of X/Y/W
    XC  -   points where polynomial values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that P(XC[i])=YC[i]
            * DC[i]=1   means that P'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints, 0<=K<M.
            K=0 means no constraints (XC/YC/DC are not used in such cases)
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    P   -   interpolant in barycentric form.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

NOTES:
    you can convert P from barycentric form  to  the  power  or  Chebyshev
    basis with PolynomialBar2Pow() or PolynomialBar2Cheb() functions  from
    POLINT subpackage.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* even simple constraints can be inconsistent, see  Wikipedia  article  on
  this subject: http://en.wikipedia.org/wiki/Birkhoff_interpolation
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints is NOT GUARANTEED.
* in the one special cases, however, we can  guarantee  consistency.  This
  case  is:  M>1  and constraints on the function values (NOT DERIVATIVES)

Our final recommendation is to use constraints  WHEN  AND  ONLY  when  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
void polynomialfitwc(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     barycentricinterpolant* p,
     polynomialfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _w;
    ae_vector _xc;
    ae_vector _yc;
    double xa;
    double xb;
    double sa;
    double sb;
    ae_vector xoriginal;
    ae_vector yoriginal;
    ae_vector y2;
    ae_vector w2;
    ae_vector tmp;
    ae_vector tmp2;
    ae_vector bx;
    ae_vector by;
    ae_vector bw;
    ae_int_t i;
    ae_int_t j;
    double u;
    double v;
    double s;
    ae_int_t relcnt;
    lsfitreport lrep;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_w, w, _state, ae_true);
    w = &_w;
    ae_vector_init_copy(&_xc, xc, _state, ae_true);
    xc = &_xc;
    ae_vector_init_copy(&_yc, yc, _state, ae_true);
    yc = &_yc;
    *info = 0;
    _barycentricinterpolant_clear(p);
    _polynomialfitreport_clear(rep);
    ae_vector_init(&xoriginal, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yoriginal, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&by, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bw, 0, DT_REAL, _state, ae_true);
    _lsfitreport_init(&lrep, _state, ae_true);

    ae_assert(n>0, "PolynomialFitWC: N<=0!", _state);
    ae_assert(m>0, "PolynomialFitWC: M<=0!", _state);
    ae_assert(k>=0, "PolynomialFitWC: K<0!", _state);
    ae_assert(k<m, "PolynomialFitWC: K>=M!", _state);
    ae_assert(x->cnt>=n, "PolynomialFitWC: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "PolynomialFitWC: Length(Y)<N!", _state);
    ae_assert(w->cnt>=n, "PolynomialFitWC: Length(W)<N!", _state);
    ae_assert(xc->cnt>=k, "PolynomialFitWC: Length(XC)<K!", _state);
    ae_assert(yc->cnt>=k, "PolynomialFitWC: Length(YC)<K!", _state);
    ae_assert(dc->cnt>=k, "PolynomialFitWC: Length(DC)<K!", _state);
    ae_assert(isfinitevector(x, n, _state), "PolynomialFitWC: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "PolynomialFitWC: Y contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(w, n, _state), "PolynomialFitWC: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(xc, k, _state), "PolynomialFitWC: XC contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(yc, k, _state), "PolynomialFitWC: YC contains infinite or NaN values!", _state);
    for(i=0; i<=k-1; i++)
    {
        ae_assert(dc->ptr.p_int[i]==0||dc->ptr.p_int[i]==1, "PolynomialFitWC: one of DC[] is not 0 or 1!", _state);
    }
    
    /*
     * Scale X, Y, XC, YC.
     * Solve scaled problem using internal Chebyshev fitting function.
     */
    lsfitscalexy(x, y, w, n, xc, yc, dc, k, &xa, &xb, &sa, &sb, &xoriginal, &yoriginal, _state);
    lsfit_internalchebyshevfit(x, y, w, n, xc, yc, dc, k, m, info, &tmp, &lrep, _state);
    if( *info<0 )
    {
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * Generate barycentric model and scale it
     * * BX, BY store barycentric model nodes
     * * FMatrix is reused (remember - it is at least MxM, what we need)
     *
     * Model intialization is done in O(M^2). In principle, it can be
     * done in O(M*log(M)), but before it we solved task with O(N*M^2)
     * complexity, so it is only a small amount of total time spent.
     */
    ae_vector_set_length(&bx, m, _state);
    ae_vector_set_length(&by, m, _state);
    ae_vector_set_length(&bw, m, _state);
    ae_vector_set_length(&tmp2, m, _state);
    s = 1;
    for(i=0; i<=m-1; i++)
    {
        if( m!=1 )
        {
            u = ae_cos(ae_pi*i/(m-1), _state);
        }
        else
        {
            u = 0;
        }
        v = 0;
        for(j=0; j<=m-1; j++)
        {
            if( j==0 )
            {
                tmp2.ptr.p_double[j] = 1;
            }
            else
            {
                if( j==1 )
                {
                    tmp2.ptr.p_double[j] = u;
                }
                else
                {
                    tmp2.ptr.p_double[j] = 2*u*tmp2.ptr.p_double[j-1]-tmp2.ptr.p_double[j-2];
                }
            }
            v = v+tmp.ptr.p_double[j]*tmp2.ptr.p_double[j];
        }
        bx.ptr.p_double[i] = u;
        by.ptr.p_double[i] = v;
        bw.ptr.p_double[i] = s;
        if( i==0||i==m-1 )
        {
            bw.ptr.p_double[i] = 0.5*bw.ptr.p_double[i];
        }
        s = -s;
    }
    barycentricbuildxyw(&bx, &by, &bw, m, p, _state);
    barycentriclintransx(p, 2/(xb-xa), -(xa+xb)/(xb-xa), _state);
    barycentriclintransy(p, sb-sa, sa, _state);
    
    /*
     * Scale absolute errors obtained from LSFitLinearW.
     * Relative error should be calculated separately
     * (because of shifting/scaling of the task)
     */
    rep->taskrcond = lrep.taskrcond;
    rep->rmserror = lrep.rmserror*(sb-sa);
    rep->avgerror = lrep.avgerror*(sb-sa);
    rep->maxerror = lrep.maxerror*(sb-sa);
    rep->avgrelerror = 0;
    relcnt = 0;
    for(i=0; i<=n-1; i++)
    {
        if( ae_fp_neq(yoriginal.ptr.p_double[i],0) )
        {
            rep->avgrelerror = rep->avgrelerror+ae_fabs(barycentriccalc(p, xoriginal.ptr.p_double[i], _state)-yoriginal.ptr.p_double[i], _state)/ae_fabs(yoriginal.ptr.p_double[i], _state);
            relcnt = relcnt+1;
        }
    }
    if( relcnt!=0 )
    {
        rep->avgrelerror = rep->avgrelerror/relcnt;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Weghted rational least  squares  fitting  using  Floater-Hormann  rational
functions  with  optimal  D  chosen  from  [0,9],  with  constraints   and
individual weights.

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal D (least WEIGHTED root
mean square error) is chosen.  Task  is  linear,  so  linear least squares
solver  is  used.  Complexity  of  this  computational  scheme is O(N*M^2)
(mostly dominated by the least squares solver).

SEE ALSO
* BarycentricFitFloaterHormann(), "lightweight" fitting without invididual
  weights and constraints.

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points, N>0.
    XC  -   points where function values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints, 0<=K<M.
            K=0 means no constraints (XC/YC/DC are not used in such cases)
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
                        -1 means another errors in parameters passed
                           (N<=0, for example)
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroutine doesn't calculate task's condition number for K<>0.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained barycentric interpolants:
* excessive  constraints  can  be  inconsistent.   Floater-Hormann   basis
  functions aren't as flexible as splines (although they are very smooth).
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints IS NOT GUARANTEED.
* in the several special cases, however, we CAN guarantee consistency.
* one of this cases is constraints on the function  VALUES at the interval
  boundaries. Note that consustency of the  constraints  on  the  function
  DERIVATIVES is NOT guaranteed (you can use in such cases  cubic  splines
  which are more flexible).
* another  special  case  is ONE constraint on the function value (OR, but
  not AND, derivative) anywhere in the interval

Our final recommendation is to use constraints  WHEN  AND  ONLY  WHEN  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricfitfloaterhormannwc(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     barycentricinterpolant* b,
     barycentricfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t d;
    ae_int_t i;
    double wrmscur;
    double wrmsbest;
    barycentricinterpolant locb;
    barycentricfitreport locrep;
    ae_int_t locinfo;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    _barycentricinterpolant_clear(b);
    _barycentricfitreport_clear(rep);
    _barycentricinterpolant_init(&locb, _state, ae_true);
    _barycentricfitreport_init(&locrep, _state, ae_true);

    ae_assert(n>0, "BarycentricFitFloaterHormannWC: N<=0!", _state);
    ae_assert(m>0, "BarycentricFitFloaterHormannWC: M<=0!", _state);
    ae_assert(k>=0, "BarycentricFitFloaterHormannWC: K<0!", _state);
    ae_assert(k<m, "BarycentricFitFloaterHormannWC: K>=M!", _state);
    ae_assert(x->cnt>=n, "BarycentricFitFloaterHormannWC: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "BarycentricFitFloaterHormannWC: Length(Y)<N!", _state);
    ae_assert(w->cnt>=n, "BarycentricFitFloaterHormannWC: Length(W)<N!", _state);
    ae_assert(xc->cnt>=k, "BarycentricFitFloaterHormannWC: Length(XC)<K!", _state);
    ae_assert(yc->cnt>=k, "BarycentricFitFloaterHormannWC: Length(YC)<K!", _state);
    ae_assert(dc->cnt>=k, "BarycentricFitFloaterHormannWC: Length(DC)<K!", _state);
    ae_assert(isfinitevector(x, n, _state), "BarycentricFitFloaterHormannWC: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "BarycentricFitFloaterHormannWC: Y contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(w, n, _state), "BarycentricFitFloaterHormannWC: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(xc, k, _state), "BarycentricFitFloaterHormannWC: XC contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(yc, k, _state), "BarycentricFitFloaterHormannWC: YC contains infinite or NaN values!", _state);
    for(i=0; i<=k-1; i++)
    {
        ae_assert(dc->ptr.p_int[i]==0||dc->ptr.p_int[i]==1, "BarycentricFitFloaterHormannWC: one of DC[] is not 0 or 1!", _state);
    }
    
    /*
     * Find optimal D
     *
     * Info is -3 by default (degenerate constraints).
     * If LocInfo will always be equal to -3, Info will remain equal to -3.
     * If at least once LocInfo will be -4, Info will be -4.
     */
    wrmsbest = ae_maxrealnumber;
    rep->dbest = -1;
    *info = -3;
    for(d=0; d<=ae_minint(9, n-1, _state); d++)
    {
        lsfit_barycentricfitwcfixedd(x, y, w, n, xc, yc, dc, k, m, d, &locinfo, &locb, &locrep, _state);
        ae_assert((locinfo==-4||locinfo==-3)||locinfo>0, "BarycentricFitFloaterHormannWC: unexpected result from BarycentricFitWCFixedD!", _state);
        if( locinfo>0 )
        {
            
            /*
             * Calculate weghted RMS
             */
            wrmscur = 0;
            for(i=0; i<=n-1; i++)
            {
                wrmscur = wrmscur+ae_sqr(w->ptr.p_double[i]*(y->ptr.p_double[i]-barycentriccalc(&locb, x->ptr.p_double[i], _state)), _state);
            }
            wrmscur = ae_sqrt(wrmscur/n, _state);
            if( ae_fp_less(wrmscur,wrmsbest)||rep->dbest<0 )
            {
                barycentriccopy(&locb, b, _state);
                rep->dbest = d;
                *info = 1;
                rep->rmserror = locrep.rmserror;
                rep->avgerror = locrep.avgerror;
                rep->avgrelerror = locrep.avgrelerror;
                rep->maxerror = locrep.maxerror;
                rep->taskrcond = locrep.taskrcond;
                wrmsbest = wrmscur;
            }
        }
        else
        {
            if( locinfo!=-3&&*info<0 )
            {
                *info = locinfo;
            }
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Rational least squares fitting using  Floater-Hormann  rational  functions
with optimal D chosen from [0,9].

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal  D  (least  root  mean
square error) is chosen.  Task  is  linear, so linear least squares solver
is used. Complexity  of  this  computational  scheme is  O(N*M^2)  (mostly
dominated by the least squares solver).

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0.
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void barycentricfitfloaterhormann(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     barycentricinterpolant* b,
     barycentricfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector w;
    ae_vector xc;
    ae_vector yc;
    ae_vector dc;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    _barycentricinterpolant_clear(b);
    _barycentricfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dc, 0, DT_INT, _state, ae_true);

    ae_assert(n>0, "BarycentricFitFloaterHormann: N<=0!", _state);
    ae_assert(m>0, "BarycentricFitFloaterHormann: M<=0!", _state);
    ae_assert(x->cnt>=n, "BarycentricFitFloaterHormann: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "BarycentricFitFloaterHormann: Length(Y)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "BarycentricFitFloaterHormann: X contains infinite or NaN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "BarycentricFitFloaterHormann: Y contains infinite or NaN values!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    barycentricfitfloaterhormannwc(x, y, &w, n, &xc, &yc, &dc, 0, m, info, b, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Rational least squares fitting using  Floater-Hormann  rational  functions
with optimal D chosen from [0,9].

Equidistant  grid  with M node on [min(x),max(x)]  is  used to build basis
functions. Different values of D are tried, optimal  D  (least  root  mean
square error) is chosen.  Task  is  linear, so linear least squares solver
is used. Complexity  of  this  computational  scheme is  O(N*M^2)  (mostly
dominated by the least squares solver).

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    N   -   number of points, N>0.
    M   -   number of basis functions ( = number_of_nodes), M>=2.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    B   -   barycentric interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * DBest         best value of the D parameter
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitpenalized(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     double rho,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector w;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=1, "Spline1DFitPenalized: N<1!", _state);
    ae_assert(m>=4, "Spline1DFitPenalized: M<4!", _state);
    ae_assert(x->cnt>=n, "Spline1DFitPenalized: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DFitPenalized: Length(Y)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "Spline1DFitPenalized: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DFitPenalized: Y contains infinite or NAN values!", _state);
    ae_assert(ae_isfinite(rho, _state), "Spline1DFitPenalized: Rho is infinite!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    spline1dfitpenalizedw(x, y, &w, n, m, rho, info, s, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Weighted fitting by penalized cubic spline.

Equidistant grid with M nodes on [min(x,xc),max(x,xc)] is  used  to  build
basis functions. Basis functions are cubic splines with  natural  boundary
conditions. Problem is regularized by  adding non-linearity penalty to the
usual least squares penalty function:

    S(x) = arg min { LS + P }, where
    LS   = SUM { w[i]^2*(y[i] - S(x[i]))^2 } - least squares penalty
    P    = C*10^rho*integral{ S''(x)^2*dx } - non-linearity penalty
    rho  - tunable constant given by user
    C    - automatically determined scale parameter,
           makes penalty invariant with respect to scaling of X, Y, W.

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            problem.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    M   -   number of basis functions ( = number_of_nodes), M>=4.
    Rho -   regularization  constant  passed   by   user.   It   penalizes
            nonlinearity in the regression spline. It  is  logarithmically
            scaled,  i.e.  actual  value  of  regularization  constant  is
            calculated as 10^Rho. It is automatically scaled so that:
            * Rho=2.0 corresponds to moderate amount of nonlinearity
            * generally, it should be somewhere in the [-8.0,+8.0]
            If you do not want to penalize nonlineary,
            pass small Rho. Values as low as -15 should work.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD or
                           Cholesky decomposition; problem may be
                           too ill-conditioned (very rare)
    S   -   spline interpolant.
    Rep -   Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

NOTE 1: additional nodes are added to the spline outside  of  the  fitting
interval to force linearity when x<min(x,xc) or x>max(x,xc).  It  is  done
for consistency - we penalize non-linearity  at [min(x,xc),max(x,xc)],  so
it is natural to force linearity outside of this interval.

NOTE 2: function automatically sorts points,  so  caller may pass unsorted
array.

  -- ALGLIB PROJECT --
     Copyright 19.10.2010 by Bochkanov Sergey
*************************************************************************/
void spline1dfitpenalizedw(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     ae_int_t m,
     double rho,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _w;
    ae_int_t i;
    ae_int_t j;
    ae_int_t b;
    double v;
    double relcnt;
    double xa;
    double xb;
    double sa;
    double sb;
    ae_vector xoriginal;
    ae_vector yoriginal;
    double pdecay;
    double tdecay;
    ae_matrix fmatrix;
    ae_vector fcolumn;
    ae_vector y2;
    ae_vector w2;
    ae_vector xc;
    ae_vector yc;
    ae_vector dc;
    double fdmax;
    double admax;
    ae_matrix amatrix;
    ae_matrix d2matrix;
    double fa;
    double ga;
    double fb;
    double gb;
    double lambdav;
    ae_vector bx;
    ae_vector by;
    ae_vector bd1;
    ae_vector bd2;
    ae_vector tx;
    ae_vector ty;
    ae_vector td;
    spline1dinterpolant bs;
    ae_matrix nmatrix;
    ae_vector rightpart;
    fblslincgstate cgstate;
    ae_vector c;
    ae_vector tmp0;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_w, w, _state, ae_true);
    w = &_w;
    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);
    ae_vector_init(&xoriginal, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yoriginal, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&fmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&fcolumn, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dc, 0, DT_INT, _state, ae_true);
    ae_matrix_init(&amatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&d2matrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&by, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bd1, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bd2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&ty, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&td, 0, DT_REAL, _state, ae_true);
    _spline1dinterpolant_init(&bs, _state, ae_true);
    ae_matrix_init(&nmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&rightpart, 0, DT_REAL, _state, ae_true);
    _fblslincgstate_init(&cgstate, _state, ae_true);
    ae_vector_init(&c, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp0, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=1, "Spline1DFitPenalizedW: N<1!", _state);
    ae_assert(m>=4, "Spline1DFitPenalizedW: M<4!", _state);
    ae_assert(x->cnt>=n, "Spline1DFitPenalizedW: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DFitPenalizedW: Length(Y)<N!", _state);
    ae_assert(w->cnt>=n, "Spline1DFitPenalizedW: Length(W)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "Spline1DFitPenalizedW: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DFitPenalizedW: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(w, n, _state), "Spline1DFitPenalizedW: Y contains infinite or NAN values!", _state);
    ae_assert(ae_isfinite(rho, _state), "Spline1DFitPenalizedW: Rho is infinite!", _state);
    
    /*
     * Prepare LambdaV
     */
    v = -ae_log(ae_machineepsilon, _state)/ae_log(10, _state);
    if( ae_fp_less(rho,-v) )
    {
        rho = -v;
    }
    if( ae_fp_greater(rho,v) )
    {
        rho = v;
    }
    lambdav = ae_pow(10, rho, _state);
    
    /*
     * Sort X, Y, W
     */
    heapsortdpoints(x, y, w, n, _state);
    
    /*
     * Scale X, Y, XC, YC
     */
    lsfitscalexy(x, y, w, n, &xc, &yc, &dc, 0, &xa, &xb, &sa, &sb, &xoriginal, &yoriginal, _state);
    
    /*
     * Allocate space
     */
    ae_matrix_set_length(&fmatrix, n, m, _state);
    ae_matrix_set_length(&amatrix, m, m, _state);
    ae_matrix_set_length(&d2matrix, m, m, _state);
    ae_vector_set_length(&bx, m, _state);
    ae_vector_set_length(&by, m, _state);
    ae_vector_set_length(&fcolumn, n, _state);
    ae_matrix_set_length(&nmatrix, m, m, _state);
    ae_vector_set_length(&rightpart, m, _state);
    ae_vector_set_length(&tmp0, ae_maxint(m, n, _state), _state);
    ae_vector_set_length(&c, m, _state);
    
    /*
     * Fill:
     * * FMatrix by values of basis functions
     * * TmpAMatrix by second derivatives of I-th function at J-th point
     * * CMatrix by constraints
     */
    fdmax = 0;
    for(b=0; b<=m-1; b++)
    {
        
        /*
         * Prepare I-th basis function
         */
        for(j=0; j<=m-1; j++)
        {
            bx.ptr.p_double[j] = (double)(2*j)/(double)(m-1)-1;
            by.ptr.p_double[j] = 0;
        }
        by.ptr.p_double[b] = 1;
        spline1dgriddiff2cubic(&bx, &by, m, 2, 0.0, 2, 0.0, &bd1, &bd2, _state);
        spline1dbuildcubic(&bx, &by, m, 2, 0.0, 2, 0.0, &bs, _state);
        
        /*
         * Calculate B-th column of FMatrix
         * Update FDMax (maximum column norm)
         */
        spline1dconvcubic(&bx, &by, m, 2, 0.0, 2, 0.0, x, n, &fcolumn, _state);
        ae_v_move(&fmatrix.ptr.pp_double[0][b], fmatrix.stride, &fcolumn.ptr.p_double[0], 1, ae_v_len(0,n-1));
        v = 0;
        for(i=0; i<=n-1; i++)
        {
            v = v+ae_sqr(w->ptr.p_double[i]*fcolumn.ptr.p_double[i], _state);
        }
        fdmax = ae_maxreal(fdmax, v, _state);
        
        /*
         * Fill temporary with second derivatives of basis function
         */
        ae_v_move(&d2matrix.ptr.pp_double[b][0], 1, &bd2.ptr.p_double[0], 1, ae_v_len(0,m-1));
    }
    
    /*
     * * calculate penalty matrix A
     * * calculate max of diagonal elements of A
     * * calculate PDecay - coefficient before penalty matrix
     */
    for(i=0; i<=m-1; i++)
    {
        for(j=i; j<=m-1; j++)
        {
            
            /*
             * calculate integral(B_i''*B_j'') where B_i and B_j are
             * i-th and j-th basis splines.
             * B_i and B_j are piecewise linear functions.
             */
            v = 0;
            for(b=0; b<=m-2; b++)
            {
                fa = d2matrix.ptr.pp_double[i][b];
                fb = d2matrix.ptr.pp_double[i][b+1];
                ga = d2matrix.ptr.pp_double[j][b];
                gb = d2matrix.ptr.pp_double[j][b+1];
                v = v+(bx.ptr.p_double[b+1]-bx.ptr.p_double[b])*(fa*ga+(fa*(gb-ga)+ga*(fb-fa))/2+(fb-fa)*(gb-ga)/3);
            }
            amatrix.ptr.pp_double[i][j] = v;
            amatrix.ptr.pp_double[j][i] = v;
        }
    }
    admax = 0;
    for(i=0; i<=m-1; i++)
    {
        admax = ae_maxreal(admax, ae_fabs(amatrix.ptr.pp_double[i][i], _state), _state);
    }
    pdecay = lambdav*fdmax/admax;
    
    /*
     * Calculate TDecay for Tikhonov regularization
     */
    tdecay = fdmax*(1+pdecay)*10*ae_machineepsilon;
    
    /*
     * Prepare system
     *
     * NOTE: FMatrix is spoiled during this process
     */
    for(i=0; i<=n-1; i++)
    {
        v = w->ptr.p_double[i];
        ae_v_muld(&fmatrix.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
    }
    rmatrixgemm(m, m, n, 1.0, &fmatrix, 0, 0, 1, &fmatrix, 0, 0, 0, 0.0, &nmatrix, 0, 0, _state);
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=m-1; j++)
        {
            nmatrix.ptr.pp_double[i][j] = nmatrix.ptr.pp_double[i][j]+pdecay*amatrix.ptr.pp_double[i][j];
        }
    }
    for(i=0; i<=m-1; i++)
    {
        nmatrix.ptr.pp_double[i][i] = nmatrix.ptr.pp_double[i][i]+tdecay;
    }
    for(i=0; i<=m-1; i++)
    {
        rightpart.ptr.p_double[i] = 0;
    }
    for(i=0; i<=n-1; i++)
    {
        v = y->ptr.p_double[i]*w->ptr.p_double[i];
        ae_v_addd(&rightpart.ptr.p_double[0], 1, &fmatrix.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
    }
    
    /*
     * Solve system
     */
    if( !spdmatrixcholesky(&nmatrix, m, ae_true, _state) )
    {
        *info = -4;
        ae_frame_leave(_state);
        return;
    }
    fblscholeskysolve(&nmatrix, 1.0, m, ae_true, &rightpart, &tmp0, _state);
    ae_v_move(&c.ptr.p_double[0], 1, &rightpart.ptr.p_double[0], 1, ae_v_len(0,m-1));
    
    /*
     * add nodes to force linearity outside of the fitting interval
     */
    spline1dgriddiffcubic(&bx, &c, m, 2, 0.0, 2, 0.0, &bd1, _state);
    ae_vector_set_length(&tx, m+2, _state);
    ae_vector_set_length(&ty, m+2, _state);
    ae_vector_set_length(&td, m+2, _state);
    ae_v_move(&tx.ptr.p_double[1], 1, &bx.ptr.p_double[0], 1, ae_v_len(1,m));
    ae_v_move(&ty.ptr.p_double[1], 1, &rightpart.ptr.p_double[0], 1, ae_v_len(1,m));
    ae_v_move(&td.ptr.p_double[1], 1, &bd1.ptr.p_double[0], 1, ae_v_len(1,m));
    tx.ptr.p_double[0] = tx.ptr.p_double[1]-(tx.ptr.p_double[2]-tx.ptr.p_double[1]);
    ty.ptr.p_double[0] = ty.ptr.p_double[1]-td.ptr.p_double[1]*(tx.ptr.p_double[2]-tx.ptr.p_double[1]);
    td.ptr.p_double[0] = td.ptr.p_double[1];
    tx.ptr.p_double[m+1] = tx.ptr.p_double[m]+(tx.ptr.p_double[m]-tx.ptr.p_double[m-1]);
    ty.ptr.p_double[m+1] = ty.ptr.p_double[m]+td.ptr.p_double[m]*(tx.ptr.p_double[m]-tx.ptr.p_double[m-1]);
    td.ptr.p_double[m+1] = td.ptr.p_double[m];
    spline1dbuildhermite(&tx, &ty, &td, m+2, s, _state);
    spline1dlintransx(s, 2/(xb-xa), -(xa+xb)/(xb-xa), _state);
    spline1dlintransy(s, sb-sa, sa, _state);
    *info = 1;
    
    /*
     * Fill report
     */
    rep->rmserror = 0;
    rep->avgerror = 0;
    rep->avgrelerror = 0;
    rep->maxerror = 0;
    relcnt = 0;
    spline1dconvcubic(&bx, &rightpart, m, 2, 0.0, 2, 0.0, x, n, &fcolumn, _state);
    for(i=0; i<=n-1; i++)
    {
        v = (sb-sa)*fcolumn.ptr.p_double[i]+sa;
        rep->rmserror = rep->rmserror+ae_sqr(v-yoriginal.ptr.p_double[i], _state);
        rep->avgerror = rep->avgerror+ae_fabs(v-yoriginal.ptr.p_double[i], _state);
        if( ae_fp_neq(yoriginal.ptr.p_double[i],0) )
        {
            rep->avgrelerror = rep->avgrelerror+ae_fabs(v-yoriginal.ptr.p_double[i], _state)/ae_fabs(yoriginal.ptr.p_double[i], _state);
            relcnt = relcnt+1;
        }
        rep->maxerror = ae_maxreal(rep->maxerror, ae_fabs(v-yoriginal.ptr.p_double[i], _state), _state);
    }
    rep->rmserror = ae_sqrt(rep->rmserror/n, _state);
    rep->avgerror = rep->avgerror/n;
    if( ae_fp_neq(relcnt,0) )
    {
        rep->avgrelerror = rep->avgrelerror/relcnt;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Weighted fitting by cubic  spline,  with constraints on function values or
derivatives.

Equidistant grid with M-2 nodes on [min(x,xc),max(x,xc)] is  used to build
basis functions. Basis functions are cubic splines with continuous  second
derivatives  and  non-fixed first  derivatives  at  interval  ends.  Small
regularizing term is used  when  solving  constrained  tasks  (to  improve
stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO
    Spline1DFitHermiteWC()  -   fitting by Hermite splines (more flexible,
                                less smooth)
    Spline1DFitCubic()      -   "lightweight" fitting  by  cubic  splines,
                                without invididual weights and constraints

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    XC  -   points where spline values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints (optional):
            * 0<=K<M.
            * K=0 means no constraints (XC/YC/DC are not used)
            * if given, only first K elements of XC/YC/DC are used
            * if not given, automatically determined from XC/YC/DC
    M   -   number of basis functions ( = number_of_nodes+2), M>=4.

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearWC() subroutine.
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    S   -   spline interpolant.
    Rep -   report, same format as in LSFitLinearWC() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* excessive constraints can be inconsistent. Splines are  piecewise  cubic
  functions, and it is easy to create an example, where  large  number  of
  constraints  concentrated  in  small  area will result in inconsistency.
  Just because spline is not flexible enough to satisfy all of  them.  And
  same constraints spread across the  [min(x),max(x)]  will  be  perfectly
  consistent.
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints IS NOT GUARANTEED.
* in the several special cases, however, we CAN guarantee consistency.
* one of this cases is constraints  on  the  function  values  AND/OR  its
  derivatives at the interval boundaries.
* another  special  case  is ONE constraint on the function value (OR, but
  not AND, derivative) anywhere in the interval

Our final recommendation is to use constraints  WHEN  AND  ONLY  WHEN  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.


  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubicwc(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_int_t i;

    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);

    ae_assert(n>=1, "Spline1DFitCubicWC: N<1!", _state);
    ae_assert(m>=4, "Spline1DFitCubicWC: M<4!", _state);
    ae_assert(k>=0, "Spline1DFitCubicWC: K<0!", _state);
    ae_assert(k<m, "Spline1DFitCubicWC: K>=M!", _state);
    ae_assert(x->cnt>=n, "Spline1DFitCubicWC: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DFitCubicWC: Length(Y)<N!", _state);
    ae_assert(w->cnt>=n, "Spline1DFitCubicWC: Length(W)<N!", _state);
    ae_assert(xc->cnt>=k, "Spline1DFitCubicWC: Length(XC)<K!", _state);
    ae_assert(yc->cnt>=k, "Spline1DFitCubicWC: Length(YC)<K!", _state);
    ae_assert(dc->cnt>=k, "Spline1DFitCubicWC: Length(DC)<K!", _state);
    ae_assert(isfinitevector(x, n, _state), "Spline1DFitCubicWC: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DFitCubicWC: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(w, n, _state), "Spline1DFitCubicWC: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(xc, k, _state), "Spline1DFitCubicWC: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(yc, k, _state), "Spline1DFitCubicWC: Y contains infinite or NAN values!", _state);
    for(i=0; i<=k-1; i++)
    {
        ae_assert(dc->ptr.p_int[i]==0||dc->ptr.p_int[i]==1, "Spline1DFitCubicWC: DC[i] is neither 0 or 1!", _state);
    }
    lsfit_spline1dfitinternal(0, x, y, w, n, xc, yc, dc, k, m, info, s, rep, _state);
}


/*************************************************************************
Weighted  fitting  by Hermite spline,  with constraints on function values
or first derivatives.

Equidistant grid with M nodes on [min(x,xc),max(x,xc)] is  used  to  build
basis functions. Basis functions are Hermite splines.  Small  regularizing
term is used when solving constrained tasks (to improve stability).

Task is linear, so linear least squares solver is used. Complexity of this
computational scheme is O(N*M^2), mostly dominated by least squares solver

SEE ALSO
    Spline1DFitCubicWC()    -   fitting by Cubic splines (less flexible,
                                more smooth)
    Spline1DFitHermite()    -   "lightweight" Hermite fitting, without
                                invididual weights and constraints

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
            Each summand in square  sum  of  approximation deviations from
            given  values  is  multiplied  by  the square of corresponding
            weight. Fill it by 1's if you don't  want  to  solve  weighted
            task.
    N   -   number of points (optional):
            * N>0
            * if given, only first N elements of X/Y/W are processed
            * if not given, automatically determined from X/Y/W sizes
    XC  -   points where spline values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that S(XC[i])=YC[i]
            * DC[i]=1   means that S'(XC[i])=YC[i]
            SEE BELOW FOR IMPORTANT INFORMATION ON CONSTRAINTS
    K   -   number of constraints (optional):
            * 0<=K<M.
            * K=0 means no constraints (XC/YC/DC are not used)
            * if given, only first K elements of XC/YC/DC are used
            * if not given, automatically determined from XC/YC/DC
    M   -   number of basis functions (= 2 * number of nodes),
            M>=4,
            M IS EVEN!

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
                        -2 means odd M was passed (which is not supported)
                        -1 means another errors in parameters passed
                           (N<=0, for example)
    S   -   spline interpolant.
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

IMPORTANT:
    this subroitine supports only even M's


ORDER OF POINTS

Subroutine automatically sorts points, so caller may pass unsorted array.

SETTING CONSTRAINTS - DANGERS AND OPPORTUNITIES:

Setting constraints can lead  to undesired  results,  like ill-conditioned
behavior, or inconsistency being detected. From the other side,  it allows
us to improve quality of the fit. Here we summarize  our  experience  with
constrained regression splines:
* excessive constraints can be inconsistent. Splines are  piecewise  cubic
  functions, and it is easy to create an example, where  large  number  of
  constraints  concentrated  in  small  area will result in inconsistency.
  Just because spline is not flexible enough to satisfy all of  them.  And
  same constraints spread across the  [min(x),max(x)]  will  be  perfectly
  consistent.
* the more evenly constraints are spread across [min(x),max(x)],  the more
  chances that they will be consistent
* the  greater  is  M (given  fixed  constraints),  the  more chances that
  constraints will be consistent
* in the general case, consistency of constraints is NOT GUARANTEED.
* in the several special cases, however, we can guarantee consistency.
* one of this cases is  M>=4  and   constraints  on   the  function  value
  (AND/OR its derivative) at the interval boundaries.
* another special case is M>=4  and  ONE  constraint on the function value
  (OR, BUT NOT AND, derivative) anywhere in [min(x),max(x)]

Our final recommendation is to use constraints  WHEN  AND  ONLY  when  you
can't solve your task without them. Anything beyond  special  cases  given
above is not guaranteed and may result in inconsistency.

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermitewc(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_int_t i;

    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);

    ae_assert(n>=1, "Spline1DFitHermiteWC: N<1!", _state);
    ae_assert(m>=4, "Spline1DFitHermiteWC: M<4!", _state);
    ae_assert(m%2==0, "Spline1DFitHermiteWC: M is odd!", _state);
    ae_assert(k>=0, "Spline1DFitHermiteWC: K<0!", _state);
    ae_assert(k<m, "Spline1DFitHermiteWC: K>=M!", _state);
    ae_assert(x->cnt>=n, "Spline1DFitHermiteWC: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DFitHermiteWC: Length(Y)<N!", _state);
    ae_assert(w->cnt>=n, "Spline1DFitHermiteWC: Length(W)<N!", _state);
    ae_assert(xc->cnt>=k, "Spline1DFitHermiteWC: Length(XC)<K!", _state);
    ae_assert(yc->cnt>=k, "Spline1DFitHermiteWC: Length(YC)<K!", _state);
    ae_assert(dc->cnt>=k, "Spline1DFitHermiteWC: Length(DC)<K!", _state);
    ae_assert(isfinitevector(x, n, _state), "Spline1DFitHermiteWC: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DFitHermiteWC: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(w, n, _state), "Spline1DFitHermiteWC: Y contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(xc, k, _state), "Spline1DFitHermiteWC: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(yc, k, _state), "Spline1DFitHermiteWC: Y contains infinite or NAN values!", _state);
    for(i=0; i<=k-1; i++)
    {
        ae_assert(dc->ptr.p_int[i]==0||dc->ptr.p_int[i]==1, "Spline1DFitHermiteWC: DC[i] is neither 0 or 1!", _state);
    }
    lsfit_spline1dfitinternal(1, x, y, w, n, xc, yc, dc, k, m, info, s, rep, _state);
}


/*************************************************************************
Least squares fitting by cubic spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitCubicWC().  See  Spline1DFitCubicWC() for more information
about subroutine parameters (we don't duplicate it here because of length)

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector w;
    ae_vector xc;
    ae_vector yc;
    ae_vector dc;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dc, 0, DT_INT, _state, ae_true);

    ae_assert(n>=1, "Spline1DFitCubic: N<1!", _state);
    ae_assert(m>=4, "Spline1DFitCubic: M<4!", _state);
    ae_assert(x->cnt>=n, "Spline1DFitCubic: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DFitCubic: Length(Y)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "Spline1DFitCubic: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DFitCubic: Y contains infinite or NAN values!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    spline1dfitcubicwc(x, y, &w, n, &xc, &yc, &dc, 0, m, info, s, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Least squares fitting by Hermite spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitHermiteWC().  See Spline1DFitHermiteWC()  description  for
more information about subroutine parameters (we don't duplicate  it  here
because of length).

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermite(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_vector w;
    ae_vector xc;
    ae_vector yc;
    ae_vector dc;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yc, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&dc, 0, DT_INT, _state, ae_true);

    ae_assert(n>=1, "Spline1DFitHermite: N<1!", _state);
    ae_assert(m>=4, "Spline1DFitHermite: M<4!", _state);
    ae_assert(m%2==0, "Spline1DFitHermite: M is odd!", _state);
    ae_assert(x->cnt>=n, "Spline1DFitHermite: Length(X)<N!", _state);
    ae_assert(y->cnt>=n, "Spline1DFitHermite: Length(Y)<N!", _state);
    ae_assert(isfinitevector(x, n, _state), "Spline1DFitHermite: X contains infinite or NAN values!", _state);
    ae_assert(isfinitevector(y, n, _state), "Spline1DFitHermite: Y contains infinite or NAN values!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    spline1dfithermitewc(x, y, &w, n, &xc, &yc, &dc, 0, m, info, s, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Weighted linear least squares fitting.

QR decomposition is used to reduce task to MxM, then triangular solver  or
SVD-based solver is used depending on condition number of the  system.  It
allows to maximize speed and retain decent accuracy.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    W       -   array[0..N-1]  Weights  corresponding to function  values.
                Each summand in square  sum  of  approximation  deviations
                from  given  values  is  multiplied  by  the   square   of
                corresponding weight.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I, J] - value of J-th basis function in I-th point.
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -1    incorrect N/M were specified
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * Rep.TaskRCond     reciprocal of condition number
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearw(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{

    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);

    ae_assert(n>=1, "LSFitLinearW: N<1!", _state);
    ae_assert(m>=1, "LSFitLinearW: M<1!", _state);
    ae_assert(y->cnt>=n, "LSFitLinearW: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitLinearW: Y contains infinite or NaN values!", _state);
    ae_assert(w->cnt>=n, "LSFitLinearW: length(W)<N!", _state);
    ae_assert(isfinitevector(w, n, _state), "LSFitLinearW: W contains infinite or NaN values!", _state);
    ae_assert(fmatrix->rows>=n, "LSFitLinearW: rows(FMatrix)<N!", _state);
    ae_assert(fmatrix->cols>=m, "LSFitLinearW: cols(FMatrix)<M!", _state);
    ae_assert(apservisfinitematrix(fmatrix, n, m, _state), "LSFitLinearW: FMatrix contains infinite or NaN values!", _state);
    lsfit_lsfitlinearinternal(y, w, fmatrix, n, m, info, c, rep, _state);
}


/*************************************************************************
Weighted constained linear least squares fitting.

This  is  variation  of LSFitLinearW(), which searchs for min|A*x=b| given
that  K  additional  constaints  C*x=bc are satisfied. It reduces original
task to modified one: min|B*y-d| WITHOUT constraints,  then LSFitLinearW()
is called.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    W       -   array[0..N-1]  Weights  corresponding to function  values.
                Each summand in square  sum  of  approximation  deviations
                from  given  values  is  multiplied  by  the   square   of
                corresponding weight.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I,J] - value of J-th basis function in I-th point.
    CMatrix -   a table of constaints, array[0..K-1,0..M].
                I-th row of CMatrix corresponds to I-th linear constraint:
                CMatrix[I,0]*C[0] + ... + CMatrix[I,M-1]*C[M-1] = CMatrix[I,M]
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.
    K       -   number of constraints, 0 <= K < M
                K=0 corresponds to absence of constraints.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -3    either   too   many  constraints  (M   or   more),
                        degenerate  constraints   (some   constraints  are
                        repetead twice) or inconsistent  constraints  were
                        specified.
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB --
     Copyright 07.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearwc(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     /* Real    */ ae_matrix* cmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _y;
    ae_matrix _cmatrix;
    ae_int_t i;
    ae_int_t j;
    ae_vector tau;
    ae_matrix q;
    ae_matrix f2;
    ae_vector tmp;
    ae_vector c0;
    double v;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_matrix_init_copy(&_cmatrix, cmatrix, _state, ae_true);
    cmatrix = &_cmatrix;
    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);
    ae_vector_init(&tau, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&q, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&f2, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&c0, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=1, "LSFitLinearWC: N<1!", _state);
    ae_assert(m>=1, "LSFitLinearWC: M<1!", _state);
    ae_assert(k>=0, "LSFitLinearWC: K<0!", _state);
    ae_assert(y->cnt>=n, "LSFitLinearWC: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitLinearWC: Y contains infinite or NaN values!", _state);
    ae_assert(w->cnt>=n, "LSFitLinearWC: length(W)<N!", _state);
    ae_assert(isfinitevector(w, n, _state), "LSFitLinearWC: W contains infinite or NaN values!", _state);
    ae_assert(fmatrix->rows>=n, "LSFitLinearWC: rows(FMatrix)<N!", _state);
    ae_assert(fmatrix->cols>=m, "LSFitLinearWC: cols(FMatrix)<M!", _state);
    ae_assert(apservisfinitematrix(fmatrix, n, m, _state), "LSFitLinearWC: FMatrix contains infinite or NaN values!", _state);
    ae_assert(cmatrix->rows>=k, "LSFitLinearWC: rows(CMatrix)<K!", _state);
    ae_assert(cmatrix->cols>=m+1||k==0, "LSFitLinearWC: cols(CMatrix)<M+1!", _state);
    ae_assert(apservisfinitematrix(cmatrix, k, m+1, _state), "LSFitLinearWC: CMatrix contains infinite or NaN values!", _state);
    if( k>=m )
    {
        *info = -3;
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * Solve
     */
    if( k==0 )
    {
        
        /*
         * no constraints
         */
        lsfit_lsfitlinearinternal(y, w, fmatrix, n, m, info, c, rep, _state);
    }
    else
    {
        
        /*
         * First, find general form solution of constraints system:
         * * factorize C = L*Q
         * * unpack Q
         * * fill upper part of C with zeros (for RCond)
         *
         * We got C=C0+Q2'*y where Q2 is lower M-K rows of Q.
         */
        rmatrixlq(cmatrix, k, m, &tau, _state);
        rmatrixlqunpackq(cmatrix, k, m, &tau, m, &q, _state);
        for(i=0; i<=k-1; i++)
        {
            for(j=i+1; j<=m-1; j++)
            {
                cmatrix->ptr.pp_double[i][j] = 0.0;
            }
        }
        if( ae_fp_less(rmatrixlurcondinf(cmatrix, k, _state),1000*ae_machineepsilon) )
        {
            *info = -3;
            ae_frame_leave(_state);
            return;
        }
        ae_vector_set_length(&tmp, k, _state);
        for(i=0; i<=k-1; i++)
        {
            if( i>0 )
            {
                v = ae_v_dotproduct(&cmatrix->ptr.pp_double[i][0], 1, &tmp.ptr.p_double[0], 1, ae_v_len(0,i-1));
            }
            else
            {
                v = 0;
            }
            tmp.ptr.p_double[i] = (cmatrix->ptr.pp_double[i][m]-v)/cmatrix->ptr.pp_double[i][i];
        }
        ae_vector_set_length(&c0, m, _state);
        for(i=0; i<=m-1; i++)
        {
            c0.ptr.p_double[i] = 0;
        }
        for(i=0; i<=k-1; i++)
        {
            v = tmp.ptr.p_double[i];
            ae_v_addd(&c0.ptr.p_double[0], 1, &q.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
        }
        
        /*
         * Second, prepare modified matrix F2 = F*Q2' and solve modified task
         */
        ae_vector_set_length(&tmp, ae_maxint(n, m, _state)+1, _state);
        ae_matrix_set_length(&f2, n, m-k, _state);
        matrixvectormultiply(fmatrix, 0, n-1, 0, m-1, ae_false, &c0, 0, m-1, -1.0, y, 0, n-1, 1.0, _state);
        matrixmatrixmultiply(fmatrix, 0, n-1, 0, m-1, ae_false, &q, k, m-1, 0, m-1, ae_true, 1.0, &f2, 0, n-1, 0, m-k-1, 0.0, &tmp, _state);
        lsfit_lsfitlinearinternal(y, w, &f2, n, m-k, info, &tmp, rep, _state);
        rep->taskrcond = -1;
        if( *info<=0 )
        {
            ae_frame_leave(_state);
            return;
        }
        
        /*
         * then, convert back to original answer: C = C0 + Q2'*Y0
         */
        ae_vector_set_length(c, m, _state);
        ae_v_move(&c->ptr.p_double[0], 1, &c0.ptr.p_double[0], 1, ae_v_len(0,m-1));
        matrixvectormultiply(&q, k, m-1, 0, m-1, ae_true, &tmp, 0, m-k-1, 1.0, c, 0, m-1, 1.0, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Linear least squares fitting.

QR decomposition is used to reduce task to MxM, then triangular solver  or
SVD-based solver is used depending on condition number of the  system.  It
allows to maximize speed and retain decent accuracy.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I, J] - value of J-th basis function in I-th point.
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * Rep.TaskRCond     reciprocal of condition number
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinear(/* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* fmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector w;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=1, "LSFitLinear: N<1!", _state);
    ae_assert(m>=1, "LSFitLinear: M<1!", _state);
    ae_assert(y->cnt>=n, "LSFitLinear: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitLinear: Y contains infinite or NaN values!", _state);
    ae_assert(fmatrix->rows>=n, "LSFitLinear: rows(FMatrix)<N!", _state);
    ae_assert(fmatrix->cols>=m, "LSFitLinear: cols(FMatrix)<M!", _state);
    ae_assert(apservisfinitematrix(fmatrix, n, m, _state), "LSFitLinear: FMatrix contains infinite or NaN values!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    lsfit_lsfitlinearinternal(y, &w, fmatrix, n, m, info, c, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Constained linear least squares fitting.

This  is  variation  of LSFitLinear(),  which searchs for min|A*x=b| given
that  K  additional  constaints  C*x=bc are satisfied. It reduces original
task to modified one: min|B*y-d| WITHOUT constraints,  then  LSFitLinear()
is called.

INPUT PARAMETERS:
    Y       -   array[0..N-1] Function values in  N  points.
    FMatrix -   a table of basis functions values, array[0..N-1, 0..M-1].
                FMatrix[I,J] - value of J-th basis function in I-th point.
    CMatrix -   a table of constaints, array[0..K-1,0..M].
                I-th row of CMatrix corresponds to I-th linear constraint:
                CMatrix[I,0]*C[0] + ... + CMatrix[I,M-1]*C[M-1] = CMatrix[I,M]
    N       -   number of points used. N>=1.
    M       -   number of basis functions, M>=1.
    K       -   number of constraints, 0 <= K < M
                K=0 corresponds to absence of constraints.

OUTPUT PARAMETERS:
    Info    -   error code:
                * -4    internal SVD decomposition subroutine failed (very
                        rare and for degenerate systems only)
                * -3    either   too   many  constraints  (M   or   more),
                        degenerate  constraints   (some   constraints  are
                        repetead twice) or inconsistent  constraints  were
                        specified.
                *  1    task is solved
    C       -   decomposition coefficients, array[0..M-1]
    Rep     -   fitting report. Following fields are set:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB --
     Copyright 07.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitlinearc(/* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* fmatrix,
     /* Real    */ ae_matrix* cmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _y;
    ae_vector w;
    ae_int_t i;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);
    ae_vector_init(&w, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=1, "LSFitLinearC: N<1!", _state);
    ae_assert(m>=1, "LSFitLinearC: M<1!", _state);
    ae_assert(k>=0, "LSFitLinearC: K<0!", _state);
    ae_assert(y->cnt>=n, "LSFitLinearC: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitLinearC: Y contains infinite or NaN values!", _state);
    ae_assert(fmatrix->rows>=n, "LSFitLinearC: rows(FMatrix)<N!", _state);
    ae_assert(fmatrix->cols>=m, "LSFitLinearC: cols(FMatrix)<M!", _state);
    ae_assert(apservisfinitematrix(fmatrix, n, m, _state), "LSFitLinearC: FMatrix contains infinite or NaN values!", _state);
    ae_assert(cmatrix->rows>=k, "LSFitLinearC: rows(CMatrix)<K!", _state);
    ae_assert(cmatrix->cols>=m+1||k==0, "LSFitLinearC: cols(CMatrix)<M+1!", _state);
    ae_assert(apservisfinitematrix(cmatrix, k, m+1, _state), "LSFitLinearC: CMatrix contains infinite or NaN values!", _state);
    ae_vector_set_length(&w, n, _state);
    for(i=0; i<=n-1; i++)
    {
        w.ptr.p_double[i] = 1;
    }
    lsfitlinearwc(y, &w, fmatrix, cmatrix, n, m, k, info, c, rep, _state);
    ae_frame_leave(_state);
}


/*************************************************************************
Weighted nonlinear least squares fitting using function values only.

Combination of numerical differentiation and secant updates is used to
obtain function Jacobian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]).

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    DiffStep-   numerical differentiation step;
                should not be very small or large;
                large = loss of accuracy
                small = growth of round-off errors

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 18.10.2008 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewf(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     double diffstep,
     lsfitstate* state,
     ae_state *_state)
{
    ae_int_t i;

    _lsfitstate_clear(state);

    ae_assert(n>=1, "LSFitCreateWF: N<1!", _state);
    ae_assert(m>=1, "LSFitCreateWF: M<1!", _state);
    ae_assert(k>=1, "LSFitCreateWF: K<1!", _state);
    ae_assert(c->cnt>=k, "LSFitCreateWF: length(C)<K!", _state);
    ae_assert(isfinitevector(c, k, _state), "LSFitCreateWF: C contains infinite or NaN values!", _state);
    ae_assert(y->cnt>=n, "LSFitCreateWF: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitCreateWF: Y contains infinite or NaN values!", _state);
    ae_assert(w->cnt>=n, "LSFitCreateWF: length(W)<N!", _state);
    ae_assert(isfinitevector(w, n, _state), "LSFitCreateWF: W contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateWF: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateWF: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateWF: X contains infinite or NaN values!", _state);
    ae_assert(ae_isfinite(diffstep, _state), "LSFitCreateWF: DiffStep is not finite!", _state);
    ae_assert(ae_fp_greater(diffstep,0), "LSFitCreateWF: DiffStep<=0!", _state);
    state->n = n;
    state->m = m;
    state->k = k;
    lsfitsetcond(state, 0.0, 0.0, 0, _state);
    lsfitsetstpmax(state, 0.0, _state);
    lsfitsetxrep(state, ae_false, _state);
    ae_matrix_set_length(&state->taskx, n, m, _state);
    ae_vector_set_length(&state->tasky, n, _state);
    ae_vector_set_length(&state->w, n, _state);
    ae_vector_set_length(&state->c, k, _state);
    ae_vector_set_length(&state->x, m, _state);
    ae_v_move(&state->c.ptr.p_double[0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->w.ptr.p_double[0], 1, &w->ptr.p_double[0], 1, ae_v_len(0,n-1));
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&state->taskx.ptr.pp_double[i][0], 1, &x->ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
        state->tasky.ptr.p_double[i] = y->ptr.p_double[i];
    }
    minlmcreatev(k, n, &state->c, diffstep, &state->optstate, _state);
    lsfit_lsfitclearrequestfields(state, _state);
    ae_vector_set_length(&state->rstate.ia, 4+1, _state);
    ae_vector_set_length(&state->rstate.ra, 1+1, _state);
    state->rstate.stage = -1;
}


/*************************************************************************
Nonlinear least squares fitting using function values only.

Combination of numerical differentiation and secant updates is used to
obtain function Jacobian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (f(c,x[0])-y[0])^2 + ... + (f(c,x[n-1])-y[n-1])^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]).

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    DiffStep-   numerical differentiation step;
                should not be very small or large;
                large = loss of accuracy
                small = growth of round-off errors

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 18.10.2008 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatef(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     double diffstep,
     lsfitstate* state,
     ae_state *_state)
{
    ae_int_t i;

    _lsfitstate_clear(state);

    ae_assert(n>=1, "LSFitCreateFG: N<1!", _state);
    ae_assert(m>=1, "LSFitCreateFG: M<1!", _state);
    ae_assert(k>=1, "LSFitCreateFG: K<1!", _state);
    ae_assert(c->cnt>=k, "LSFitCreateFG: length(C)<K!", _state);
    ae_assert(isfinitevector(c, k, _state), "LSFitCreateFG: C contains infinite or NaN values!", _state);
    ae_assert(y->cnt>=n, "LSFitCreateFG: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitCreateFG: Y contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateFG: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateFG: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateFG: X contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateFG: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateFG: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateFG: X contains infinite or NaN values!", _state);
    ae_assert(ae_isfinite(diffstep, _state), "LSFitCreateWF: DiffStep is not finite!", _state);
    ae_assert(ae_fp_greater(diffstep,0), "LSFitCreateWF: DiffStep<=0!", _state);
    state->n = n;
    state->m = m;
    state->k = k;
    lsfitsetcond(state, 0.0, 0.0, 0, _state);
    lsfitsetstpmax(state, 0.0, _state);
    lsfitsetxrep(state, ae_false, _state);
    ae_matrix_set_length(&state->taskx, n, m, _state);
    ae_vector_set_length(&state->tasky, n, _state);
    ae_vector_set_length(&state->w, n, _state);
    ae_vector_set_length(&state->c, k, _state);
    ae_vector_set_length(&state->x, m, _state);
    ae_v_move(&state->c.ptr.p_double[0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,k-1));
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&state->taskx.ptr.pp_double[i][0], 1, &x->ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
        state->tasky.ptr.p_double[i] = y->ptr.p_double[i];
        state->w.ptr.p_double[i] = 1;
    }
    minlmcreatev(k, n, &state->c, diffstep, &state->optstate, _state);
    lsfit_lsfitclearrequestfields(state, _state);
    ae_vector_set_length(&state->rstate.ia, 4+1, _state);
    ae_vector_set_length(&state->rstate.ra, 1+1, _state);
    state->rstate.stage = -1;
}


/*************************************************************************
Weighted nonlinear least squares fitting using gradient only.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,
    
    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted
    
This subroutine uses only f(c,x[i]) and its gradient.
    
INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    CheapFG -   boolean flag, which is:
                * True  if both function and gradient calculation complexity
                        are less than O(M^2).  An improved  algorithm  can
                        be  used  which corresponds  to  FGJ  scheme  from
                        MINLM unit.
                * False otherwise.
                        Standard Jacibian-bases  Levenberg-Marquardt  algo
                        will be used (FJ scheme).

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

See also:
    LSFitResults
    LSFitCreateFG (fitting without weights)
    LSFitCreateWFGH (fitting using Hessian)
    LSFitCreateFGH (fitting using Hessian, without weights)

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewfg(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_bool cheapfg,
     lsfitstate* state,
     ae_state *_state)
{
    ae_int_t i;

    _lsfitstate_clear(state);

    ae_assert(n>=1, "LSFitCreateWFG: N<1!", _state);
    ae_assert(m>=1, "LSFitCreateWFG: M<1!", _state);
    ae_assert(k>=1, "LSFitCreateWFG: K<1!", _state);
    ae_assert(c->cnt>=k, "LSFitCreateWFG: length(C)<K!", _state);
    ae_assert(isfinitevector(c, k, _state), "LSFitCreateWFG: C contains infinite or NaN values!", _state);
    ae_assert(y->cnt>=n, "LSFitCreateWFG: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitCreateWFG: Y contains infinite or NaN values!", _state);
    ae_assert(w->cnt>=n, "LSFitCreateWFG: length(W)<N!", _state);
    ae_assert(isfinitevector(w, n, _state), "LSFitCreateWFG: W contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateWFG: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateWFG: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateWFG: X contains infinite or NaN values!", _state);
    state->n = n;
    state->m = m;
    state->k = k;
    lsfitsetcond(state, 0.0, 0.0, 0, _state);
    lsfitsetstpmax(state, 0.0, _state);
    lsfitsetxrep(state, ae_false, _state);
    ae_matrix_set_length(&state->taskx, n, m, _state);
    ae_vector_set_length(&state->tasky, n, _state);
    ae_vector_set_length(&state->w, n, _state);
    ae_vector_set_length(&state->c, k, _state);
    ae_vector_set_length(&state->x, m, _state);
    ae_vector_set_length(&state->g, k, _state);
    ae_v_move(&state->c.ptr.p_double[0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->w.ptr.p_double[0], 1, &w->ptr.p_double[0], 1, ae_v_len(0,n-1));
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&state->taskx.ptr.pp_double[i][0], 1, &x->ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
        state->tasky.ptr.p_double[i] = y->ptr.p_double[i];
    }
    if( cheapfg )
    {
        minlmcreatevgj(k, n, &state->c, &state->optstate, _state);
    }
    else
    {
        minlmcreatevj(k, n, &state->c, &state->optstate, _state);
    }
    lsfit_lsfitclearrequestfields(state, _state);
    ae_vector_set_length(&state->rstate.ia, 4+1, _state);
    ae_vector_set_length(&state->rstate.ra, 1+1, _state);
    state->rstate.stage = -1;
}


/*************************************************************************
Nonlinear least squares fitting using gradient only, without individual
weights.

Nonlinear task min(F(c)) is solved, where

    F(c) = ((f(c,x[0])-y[0]))^2 + ... + ((f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses only f(c,x[i]) and its gradient.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted
    CheapFG -   boolean flag, which is:
                * True  if both function and gradient calculation complexity
                        are less than O(M^2).  An improved  algorithm  can
                        be  used  which corresponds  to  FGJ  scheme  from
                        MINLM unit.
                * False otherwise.
                        Standard Jacibian-bases  Levenberg-Marquardt  algo
                        will be used (FJ scheme).

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatefg(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_bool cheapfg,
     lsfitstate* state,
     ae_state *_state)
{
    ae_int_t i;

    _lsfitstate_clear(state);

    ae_assert(n>=1, "LSFitCreateFG: N<1!", _state);
    ae_assert(m>=1, "LSFitCreateFG: M<1!", _state);
    ae_assert(k>=1, "LSFitCreateFG: K<1!", _state);
    ae_assert(c->cnt>=k, "LSFitCreateFG: length(C)<K!", _state);
    ae_assert(isfinitevector(c, k, _state), "LSFitCreateFG: C contains infinite or NaN values!", _state);
    ae_assert(y->cnt>=n, "LSFitCreateFG: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitCreateFG: Y contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateFG: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateFG: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateFG: X contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateFG: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateFG: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateFG: X contains infinite or NaN values!", _state);
    state->n = n;
    state->m = m;
    state->k = k;
    lsfitsetcond(state, 0.0, 0.0, 0, _state);
    lsfitsetstpmax(state, 0.0, _state);
    lsfitsetxrep(state, ae_false, _state);
    ae_matrix_set_length(&state->taskx, n, m, _state);
    ae_vector_set_length(&state->tasky, n, _state);
    ae_vector_set_length(&state->w, n, _state);
    ae_vector_set_length(&state->c, k, _state);
    ae_vector_set_length(&state->x, m, _state);
    ae_vector_set_length(&state->g, k, _state);
    ae_v_move(&state->c.ptr.p_double[0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,k-1));
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&state->taskx.ptr.pp_double[i][0], 1, &x->ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
        state->tasky.ptr.p_double[i] = y->ptr.p_double[i];
        state->w.ptr.p_double[i] = 1;
    }
    if( cheapfg )
    {
        minlmcreatevgj(k, n, &state->c, &state->optstate, _state);
    }
    else
    {
        minlmcreatevj(k, n, &state->c, &state->optstate, _state);
    }
    lsfit_lsfitclearrequestfields(state, _state);
    ae_vector_set_length(&state->rstate.ia, 4+1, _state);
    ae_vector_set_length(&state->rstate.ra, 1+1, _state);
    state->rstate.stage = -1;
}


/*************************************************************************
Weighted nonlinear least squares fitting using gradient/Hessian.

Nonlinear task min(F(c)) is solved, where

    F(c) = (w[0]*(f(c,x[0])-y[0]))^2 + ... + (w[n-1]*(f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * w is an N-dimensional vector of weight coefficients,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses f(c,x[i]), its gradient and its Hessian.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    W       -   weights, array[0..N-1]
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatewfgh(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     lsfitstate* state,
     ae_state *_state)
{
    ae_int_t i;

    _lsfitstate_clear(state);

    ae_assert(n>=1, "LSFitCreateWFGH: N<1!", _state);
    ae_assert(m>=1, "LSFitCreateWFGH: M<1!", _state);
    ae_assert(k>=1, "LSFitCreateWFGH: K<1!", _state);
    ae_assert(c->cnt>=k, "LSFitCreateWFGH: length(C)<K!", _state);
    ae_assert(isfinitevector(c, k, _state), "LSFitCreateWFGH: C contains infinite or NaN values!", _state);
    ae_assert(y->cnt>=n, "LSFitCreateWFGH: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitCreateWFGH: Y contains infinite or NaN values!", _state);
    ae_assert(w->cnt>=n, "LSFitCreateWFGH: length(W)<N!", _state);
    ae_assert(isfinitevector(w, n, _state), "LSFitCreateWFGH: W contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateWFGH: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateWFGH: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateWFGH: X contains infinite or NaN values!", _state);
    state->n = n;
    state->m = m;
    state->k = k;
    lsfitsetcond(state, 0.0, 0.0, 0, _state);
    lsfitsetstpmax(state, 0.0, _state);
    lsfitsetxrep(state, ae_false, _state);
    ae_matrix_set_length(&state->taskx, n, m, _state);
    ae_vector_set_length(&state->tasky, n, _state);
    ae_vector_set_length(&state->w, n, _state);
    ae_vector_set_length(&state->c, k, _state);
    ae_matrix_set_length(&state->h, k, k, _state);
    ae_vector_set_length(&state->x, m, _state);
    ae_vector_set_length(&state->g, k, _state);
    ae_v_move(&state->c.ptr.p_double[0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->w.ptr.p_double[0], 1, &w->ptr.p_double[0], 1, ae_v_len(0,n-1));
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&state->taskx.ptr.pp_double[i][0], 1, &x->ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
        state->tasky.ptr.p_double[i] = y->ptr.p_double[i];
    }
    minlmcreatefgh(k, &state->c, &state->optstate, _state);
    lsfit_lsfitclearrequestfields(state, _state);
    ae_vector_set_length(&state->rstate.ia, 4+1, _state);
    ae_vector_set_length(&state->rstate.ra, 1+1, _state);
    state->rstate.stage = -1;
}


/*************************************************************************
Nonlinear least squares fitting using gradient/Hessian, without individial
weights.

Nonlinear task min(F(c)) is solved, where

    F(c) = ((f(c,x[0])-y[0]))^2 + ... + ((f(c,x[n-1])-y[n-1]))^2,

    * N is a number of points,
    * M is a dimension of a space points belong to,
    * K is a dimension of a space of parameters being fitted,
    * x is a set of N points, each of them is an M-dimensional vector,
    * c is a K-dimensional vector of parameters being fitted

This subroutine uses f(c,x[i]), its gradient and its Hessian.

INPUT PARAMETERS:
    X       -   array[0..N-1,0..M-1], points (one row = one point)
    Y       -   array[0..N-1], function values.
    C       -   array[0..K-1], initial approximation to the solution,
    N       -   number of points, N>1
    M       -   dimension of space
    K       -   number of parameters being fitted

OUTPUT PARAMETERS:
    State   -   structure which stores algorithm state


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitcreatefgh(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     lsfitstate* state,
     ae_state *_state)
{
    ae_int_t i;

    _lsfitstate_clear(state);

    ae_assert(n>=1, "LSFitCreateFGH: N<1!", _state);
    ae_assert(m>=1, "LSFitCreateFGH: M<1!", _state);
    ae_assert(k>=1, "LSFitCreateFGH: K<1!", _state);
    ae_assert(c->cnt>=k, "LSFitCreateFGH: length(C)<K!", _state);
    ae_assert(isfinitevector(c, k, _state), "LSFitCreateFGH: C contains infinite or NaN values!", _state);
    ae_assert(y->cnt>=n, "LSFitCreateFGH: length(Y)<N!", _state);
    ae_assert(isfinitevector(y, n, _state), "LSFitCreateFGH: Y contains infinite or NaN values!", _state);
    ae_assert(x->rows>=n, "LSFitCreateFGH: rows(X)<N!", _state);
    ae_assert(x->cols>=m, "LSFitCreateFGH: cols(X)<M!", _state);
    ae_assert(apservisfinitematrix(x, n, m, _state), "LSFitCreateFGH: X contains infinite or NaN values!", _state);
    state->n = n;
    state->m = m;
    state->k = k;
    lsfitsetcond(state, 0.0, 0.0, 0, _state);
    lsfitsetstpmax(state, 0.0, _state);
    lsfitsetxrep(state, ae_false, _state);
    ae_matrix_set_length(&state->taskx, n, m, _state);
    ae_vector_set_length(&state->tasky, n, _state);
    ae_vector_set_length(&state->w, n, _state);
    ae_vector_set_length(&state->c, k, _state);
    ae_matrix_set_length(&state->h, k, k, _state);
    ae_vector_set_length(&state->x, m, _state);
    ae_vector_set_length(&state->g, k, _state);
    ae_v_move(&state->c.ptr.p_double[0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,k-1));
    for(i=0; i<=n-1; i++)
    {
        ae_v_move(&state->taskx.ptr.pp_double[i][0], 1, &x->ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
        state->tasky.ptr.p_double[i] = y->ptr.p_double[i];
        state->w.ptr.p_double[i] = 1;
    }
    minlmcreatefgh(k, &state->c, &state->optstate, _state);
    lsfit_lsfitclearrequestfields(state, _state);
    ae_vector_set_length(&state->rstate.ia, 4+1, _state);
    ae_vector_set_length(&state->rstate.ra, 1+1, _state);
    state->rstate.stage = -1;
}


/*************************************************************************
Stopping conditions for nonlinear least squares fitting.

INPUT PARAMETERS:
    State   -   structure which stores algorithm state
    EpsF    -   stopping criterion. Algorithm stops if
                |F(k+1)-F(k)| <= EpsF*max{|F(k)|, |F(k+1)|, 1}
    EpsX    -   stopping criterion. Algorithm stops if
                |X(k+1)-X(k)| <= EpsX*(1+|X(k)|)
    MaxIts  -   stopping criterion. Algorithm stops after MaxIts iterations.
                MaxIts=0 means no stopping criterion.

NOTE

Passing EpsF=0, EpsX=0 and MaxIts=0 (simultaneously) will lead to automatic
stopping criterion selection (according to the scheme used by MINLM unit).


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitsetcond(lsfitstate* state,
     double epsf,
     double epsx,
     ae_int_t maxits,
     ae_state *_state)
{


    ae_assert(ae_isfinite(epsf, _state), "LSFitSetCond: EpsF is not finite!", _state);
    ae_assert(ae_fp_greater_eq(epsf,0), "LSFitSetCond: negative EpsF!", _state);
    ae_assert(ae_isfinite(epsx, _state), "LSFitSetCond: EpsX is not finite!", _state);
    ae_assert(ae_fp_greater_eq(epsx,0), "LSFitSetCond: negative EpsX!", _state);
    ae_assert(maxits>=0, "LSFitSetCond: negative MaxIts!", _state);
    state->epsf = epsf;
    state->epsx = epsx;
    state->maxits = maxits;
}


/*************************************************************************
This function sets maximum step length

INPUT PARAMETERS:
    State   -   structure which stores algorithm state
    StpMax  -   maximum step length, >=0. Set StpMax to 0.0,  if you don't
                want to limit step length.

Use this subroutine when you optimize target function which contains exp()
or  other  fast  growing  functions,  and optimization algorithm makes too
large  steps  which  leads  to overflow. This function allows us to reject
steps  that  are  too  large  (and  therefore  expose  us  to the possible
overflow) without actually calculating function value at the x+stp*d.

NOTE: non-zero StpMax leads to moderate  performance  degradation  because
intermediate  step  of  preconditioned L-BFGS optimization is incompatible
with limits on step size.

  -- ALGLIB --
     Copyright 02.04.2010 by Bochkanov Sergey
*************************************************************************/
void lsfitsetstpmax(lsfitstate* state, double stpmax, ae_state *_state)
{


    ae_assert(ae_fp_greater_eq(stpmax,0), "LSFitSetStpMax: StpMax<0!", _state);
    state->stpmax = stpmax;
}


/*************************************************************************
This function turns on/off reporting.

INPUT PARAMETERS:
    State   -   structure which stores algorithm state
    NeedXRep-   whether iteration reports are needed or not
    
When reports are needed, State.C (current parameters) and State.F (current
value of fitting function) are reported.


  -- ALGLIB --
     Copyright 15.08.2010 by Bochkanov Sergey
*************************************************************************/
void lsfitsetxrep(lsfitstate* state, ae_bool needxrep, ae_state *_state)
{


    state->xrep = needxrep;
}


/*************************************************************************
NOTES:

1. this algorithm is somewhat unusual because it works with  parameterized
   function f(C,X), where X is a function argument (we  have  many  points
   which are characterized by different  argument  values),  and  C  is  a
   parameter to fit.

   For example, if we want to do linear fit by f(c0,c1,x) = c0*x+c1,  then
   x will be argument, and {c0,c1} will be parameters.
   
   It is important to understand that this algorithm finds minimum in  the
   space of function PARAMETERS (not arguments), so it  needs  derivatives
   of f() with respect to C, not X.
   
   In the example above it will need f=c0*x+c1 and {df/dc0,df/dc1} = {x,1}
   instead of {df/dx} = {c0}.

2. Callback functions accept C as the first parameter, and X as the second

3. If  state  was  created  with  LSFitCreateFG(),  algorithm  needs  just
   function   and   its   gradient,   but   if   state   was  created with
   LSFitCreateFGH(), algorithm will need function, gradient and Hessian.
   
   According  to  the  said  above,  there  ase  several  versions of this
   function, which accept different sets of callbacks.
   
   This flexibility opens way to subtle errors - you may create state with
   LSFitCreateFGH() (optimization using Hessian), but call function  which
   does not accept Hessian. So when algorithm will request Hessian,  there
   will be no callback to call. In this case exception will be thrown.
   
   Be careful to avoid such errors because there is no way to find them at
   compile time - you can see them at runtime only.

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
ae_bool lsfititeration(lsfitstate* state, ae_state *_state)
{
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    ae_int_t i;
    ae_int_t j;
    double v;
    double relcnt;
    ae_bool result;


    
    /*
     * Reverse communication preparations
     * I know it looks ugly, but it works the same way
     * anywhere from C++ to Python.
     *
     * This code initializes locals by:
     * * random values determined during code
     *   generation - on first subroutine call
     * * values from previous call - on subsequent calls
     */
    if( state->rstate.stage>=0 )
    {
        n = state->rstate.ia.ptr.p_int[0];
        m = state->rstate.ia.ptr.p_int[1];
        k = state->rstate.ia.ptr.p_int[2];
        i = state->rstate.ia.ptr.p_int[3];
        j = state->rstate.ia.ptr.p_int[4];
        v = state->rstate.ra.ptr.p_double[0];
        relcnt = state->rstate.ra.ptr.p_double[1];
    }
    else
    {
        n = -983;
        m = -989;
        k = -834;
        i = 900;
        j = -287;
        v = 364;
        relcnt = 214;
    }
    if( state->rstate.stage==0 )
    {
        goto lbl_0;
    }
    if( state->rstate.stage==1 )
    {
        goto lbl_1;
    }
    if( state->rstate.stage==2 )
    {
        goto lbl_2;
    }
    if( state->rstate.stage==3 )
    {
        goto lbl_3;
    }
    if( state->rstate.stage==4 )
    {
        goto lbl_4;
    }
    if( state->rstate.stage==5 )
    {
        goto lbl_5;
    }
    if( state->rstate.stage==6 )
    {
        goto lbl_6;
    }
    
    /*
     * Routine body
     */
    
    /*
     * init
     */
    n = state->n;
    m = state->m;
    k = state->k;
    minlmsetcond(&state->optstate, 0.0, state->epsf, state->epsx, state->maxits, _state);
    minlmsetstpmax(&state->optstate, state->stpmax, _state);
    minlmsetxrep(&state->optstate, state->xrep, _state);
    
    /*
     * Optimize
     */
lbl_7:
    if( !minlmiteration(&state->optstate, _state) )
    {
        goto lbl_8;
    }
    if( !state->optstate.needfi )
    {
        goto lbl_9;
    }
    
    /*
     * calculate f[] = wi*(f(xi,c)-yi)
     */
    i = 0;
lbl_11:
    if( i>n-1 )
    {
        goto lbl_13;
    }
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->optstate.x.ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->x.ptr.p_double[0], 1, &state->taskx.ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
    state->pointindex = i;
    lsfit_lsfitclearrequestfields(state, _state);
    state->needf = ae_true;
    state->rstate.stage = 0;
    goto lbl_rcomm;
lbl_0:
    state->needf = ae_false;
    state->optstate.fi.ptr.p_double[i] = state->w.ptr.p_double[i]*(state->f-state->tasky.ptr.p_double[i]);
    i = i+1;
    goto lbl_11;
lbl_13:
    goto lbl_7;
lbl_9:
    if( !state->optstate.needf )
    {
        goto lbl_14;
    }
    
    /*
     * calculate F = sum (wi*(f(xi,c)-yi))^2
     */
    state->optstate.f = 0;
    i = 0;
lbl_16:
    if( i>n-1 )
    {
        goto lbl_18;
    }
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->optstate.x.ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->x.ptr.p_double[0], 1, &state->taskx.ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
    state->pointindex = i;
    lsfit_lsfitclearrequestfields(state, _state);
    state->needf = ae_true;
    state->rstate.stage = 1;
    goto lbl_rcomm;
lbl_1:
    state->needf = ae_false;
    state->optstate.f = state->optstate.f+ae_sqr(state->w.ptr.p_double[i]*(state->f-state->tasky.ptr.p_double[i]), _state);
    i = i+1;
    goto lbl_16;
lbl_18:
    goto lbl_7;
lbl_14:
    if( !state->optstate.needfg )
    {
        goto lbl_19;
    }
    
    /*
     * calculate F/gradF
     */
    state->optstate.f = 0;
    for(i=0; i<=k-1; i++)
    {
        state->optstate.g.ptr.p_double[i] = 0;
    }
    i = 0;
lbl_21:
    if( i>n-1 )
    {
        goto lbl_23;
    }
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->optstate.x.ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->x.ptr.p_double[0], 1, &state->taskx.ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
    state->pointindex = i;
    lsfit_lsfitclearrequestfields(state, _state);
    state->needfg = ae_true;
    state->rstate.stage = 2;
    goto lbl_rcomm;
lbl_2:
    state->needfg = ae_false;
    state->optstate.f = state->optstate.f+ae_sqr(state->w.ptr.p_double[i]*(state->f-state->tasky.ptr.p_double[i]), _state);
    v = ae_sqr(state->w.ptr.p_double[i], _state)*2*(state->f-state->tasky.ptr.p_double[i]);
    ae_v_addd(&state->optstate.g.ptr.p_double[0], 1, &state->g.ptr.p_double[0], 1, ae_v_len(0,k-1), v);
    i = i+1;
    goto lbl_21;
lbl_23:
    goto lbl_7;
lbl_19:
    if( !state->optstate.needfij )
    {
        goto lbl_24;
    }
    
    /*
     * calculate Fi/jac(Fi)
     */
    i = 0;
lbl_26:
    if( i>n-1 )
    {
        goto lbl_28;
    }
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->optstate.x.ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->x.ptr.p_double[0], 1, &state->taskx.ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
    state->pointindex = i;
    lsfit_lsfitclearrequestfields(state, _state);
    state->needfg = ae_true;
    state->rstate.stage = 3;
    goto lbl_rcomm;
lbl_3:
    state->needfg = ae_false;
    state->optstate.fi.ptr.p_double[i] = state->w.ptr.p_double[i]*(state->f-state->tasky.ptr.p_double[i]);
    v = state->w.ptr.p_double[i];
    ae_v_moved(&state->optstate.j.ptr.pp_double[i][0], 1, &state->g.ptr.p_double[0], 1, ae_v_len(0,k-1), v);
    i = i+1;
    goto lbl_26;
lbl_28:
    goto lbl_7;
lbl_24:
    if( !state->optstate.needfgh )
    {
        goto lbl_29;
    }
    
    /*
     * calculate F/grad(F)/hess(F)
     */
    state->optstate.f = 0;
    for(i=0; i<=k-1; i++)
    {
        state->optstate.g.ptr.p_double[i] = 0;
    }
    for(i=0; i<=k-1; i++)
    {
        for(j=0; j<=k-1; j++)
        {
            state->optstate.h.ptr.pp_double[i][j] = 0;
        }
    }
    i = 0;
lbl_31:
    if( i>n-1 )
    {
        goto lbl_33;
    }
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->optstate.x.ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->x.ptr.p_double[0], 1, &state->taskx.ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
    state->pointindex = i;
    lsfit_lsfitclearrequestfields(state, _state);
    state->needfgh = ae_true;
    state->rstate.stage = 4;
    goto lbl_rcomm;
lbl_4:
    state->needfgh = ae_false;
    state->optstate.f = state->optstate.f+ae_sqr(state->w.ptr.p_double[i]*(state->f-state->tasky.ptr.p_double[i]), _state);
    v = ae_sqr(state->w.ptr.p_double[i], _state)*2*(state->f-state->tasky.ptr.p_double[i]);
    ae_v_addd(&state->optstate.g.ptr.p_double[0], 1, &state->g.ptr.p_double[0], 1, ae_v_len(0,k-1), v);
    for(j=0; j<=k-1; j++)
    {
        v = 2*ae_sqr(state->w.ptr.p_double[i], _state)*state->g.ptr.p_double[j];
        ae_v_addd(&state->optstate.h.ptr.pp_double[j][0], 1, &state->g.ptr.p_double[0], 1, ae_v_len(0,k-1), v);
        v = 2*ae_sqr(state->w.ptr.p_double[i], _state)*(state->f-state->tasky.ptr.p_double[i]);
        ae_v_addd(&state->optstate.h.ptr.pp_double[j][0], 1, &state->h.ptr.pp_double[j][0], 1, ae_v_len(0,k-1), v);
    }
    i = i+1;
    goto lbl_31;
lbl_33:
    goto lbl_7;
lbl_29:
    if( !state->optstate.xupdated )
    {
        goto lbl_34;
    }
    
    /*
     * Report new iteration
     */
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->optstate.x.ptr.p_double[0], 1, ae_v_len(0,k-1));
    state->f = state->optstate.f;
    lsfit_lsfitclearrequestfields(state, _state);
    state->xupdated = ae_true;
    state->rstate.stage = 5;
    goto lbl_rcomm;
lbl_5:
    state->xupdated = ae_false;
    goto lbl_7;
lbl_34:
    goto lbl_7;
lbl_8:
    minlmresults(&state->optstate, &state->c, &state->optrep, _state);
    state->repterminationtype = state->optrep.terminationtype;
    
    /*
     * calculate errors
     */
    if( state->repterminationtype<=0 )
    {
        goto lbl_36;
    }
    state->reprmserror = 0;
    state->repavgerror = 0;
    state->repavgrelerror = 0;
    state->repmaxerror = 0;
    relcnt = 0;
    i = 0;
lbl_38:
    if( i>n-1 )
    {
        goto lbl_40;
    }
    ae_v_move(&state->c.ptr.p_double[0], 1, &state->c.ptr.p_double[0], 1, ae_v_len(0,k-1));
    ae_v_move(&state->x.ptr.p_double[0], 1, &state->taskx.ptr.pp_double[i][0], 1, ae_v_len(0,m-1));
    state->pointindex = i;
    lsfit_lsfitclearrequestfields(state, _state);
    state->needf = ae_true;
    state->rstate.stage = 6;
    goto lbl_rcomm;
lbl_6:
    state->needf = ae_false;
    v = state->f;
    state->reprmserror = state->reprmserror+ae_sqr(v-state->tasky.ptr.p_double[i], _state);
    state->repavgerror = state->repavgerror+ae_fabs(v-state->tasky.ptr.p_double[i], _state);
    if( ae_fp_neq(state->tasky.ptr.p_double[i],0) )
    {
        state->repavgrelerror = state->repavgrelerror+ae_fabs(v-state->tasky.ptr.p_double[i], _state)/ae_fabs(state->tasky.ptr.p_double[i], _state);
        relcnt = relcnt+1;
    }
    state->repmaxerror = ae_maxreal(state->repmaxerror, ae_fabs(v-state->tasky.ptr.p_double[i], _state), _state);
    i = i+1;
    goto lbl_38;
lbl_40:
    state->reprmserror = ae_sqrt(state->reprmserror/n, _state);
    state->repavgerror = state->repavgerror/n;
    if( ae_fp_neq(relcnt,0) )
    {
        state->repavgrelerror = state->repavgrelerror/relcnt;
    }
lbl_36:
    result = ae_false;
    return result;
    
    /*
     * Saving state
     */
lbl_rcomm:
    result = ae_true;
    state->rstate.ia.ptr.p_int[0] = n;
    state->rstate.ia.ptr.p_int[1] = m;
    state->rstate.ia.ptr.p_int[2] = k;
    state->rstate.ia.ptr.p_int[3] = i;
    state->rstate.ia.ptr.p_int[4] = j;
    state->rstate.ra.ptr.p_double[0] = v;
    state->rstate.ra.ptr.p_double[1] = relcnt;
    return result;
}


/*************************************************************************
Nonlinear least squares fitting results.

Called after return from LSFitFit().

INPUT PARAMETERS:
    State   -   algorithm state

OUTPUT PARAMETERS:
    Info    -   completetion code:
                    *  1    relative function improvement is no more than
                            EpsF.
                    *  2    relative step is no more than EpsX.
                    *  4    gradient norm is no more than EpsG
                    *  5    MaxIts steps was taken
                    *  7    stopping conditions are too stringent,
                            further improvement is impossible
    C       -   array[0..K-1], solution
    Rep     -   optimization report. Following fields are set:
                * Rep.TerminationType completetion code:
                * RMSError          rms error on the (X,Y).
                * AvgError          average error on the (X,Y).
                * AvgRelError       average relative error on the non-zero Y
                * MaxError          maximum error
                                    NON-WEIGHTED ERRORS ARE CALCULATED


  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitresults(lsfitstate* state,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{

    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);

    *info = state->repterminationtype;
    if( *info>0 )
    {
        ae_vector_set_length(c, state->k, _state);
        ae_v_move(&c->ptr.p_double[0], 1, &state->c.ptr.p_double[0], 1, ae_v_len(0,state->k-1));
        rep->rmserror = state->reprmserror;
        rep->avgerror = state->repavgerror;
        rep->avgrelerror = state->repavgrelerror;
        rep->maxerror = state->repmaxerror;
    }
}


/*************************************************************************
Internal subroutine: automatic scaling for LLS tasks.
NEVER CALL IT DIRECTLY!

Maps abscissas to [-1,1], standartizes ordinates and correspondingly scales
constraints. It also scales weights so that max(W[i])=1

Transformations performed:
* X, XC         [XA,XB] => [-1,+1]
                transformation makes min(X)=-1, max(X)=+1

* Y             [SA,SB] => [0,1]
                transformation makes mean(Y)=0, stddev(Y)=1
                
* YC            transformed accordingly to SA, SB, DC[I]

  -- ALGLIB PROJECT --
     Copyright 08.09.2009 by Bochkanov Sergey
*************************************************************************/
void lsfitscalexy(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     double* xa,
     double* xb,
     double* sa,
     double* sb,
     /* Real    */ ae_vector* xoriginal,
     /* Real    */ ae_vector* yoriginal,
     ae_state *_state)
{
    double xmin;
    double xmax;
    ae_int_t i;
    double mx;

    *xa = 0;
    *xb = 0;
    *sa = 0;
    *sb = 0;
    ae_vector_clear(xoriginal);
    ae_vector_clear(yoriginal);

    ae_assert(n>=1, "LSFitScaleXY: incorrect N", _state);
    ae_assert(k>=0, "LSFitScaleXY: incorrect K", _state);
    
    /*
     * Calculate xmin/xmax.
     * Force xmin<>xmax.
     */
    xmin = x->ptr.p_double[0];
    xmax = x->ptr.p_double[0];
    for(i=1; i<=n-1; i++)
    {
        xmin = ae_minreal(xmin, x->ptr.p_double[i], _state);
        xmax = ae_maxreal(xmax, x->ptr.p_double[i], _state);
    }
    for(i=0; i<=k-1; i++)
    {
        xmin = ae_minreal(xmin, xc->ptr.p_double[i], _state);
        xmax = ae_maxreal(xmax, xc->ptr.p_double[i], _state);
    }
    if( ae_fp_eq(xmin,xmax) )
    {
        if( ae_fp_eq(xmin,0) )
        {
            xmin = -1;
            xmax = 1;
        }
        else
        {
            if( ae_fp_greater(xmin,0) )
            {
                xmin = 0.5*xmin;
            }
            else
            {
                xmax = 0.5*xmax;
            }
        }
    }
    
    /*
     * Transform abscissas: map [XA,XB] to [0,1]
     *
     * Store old X[] in XOriginal[] (it will be used
     * to calculate relative error).
     */
    ae_vector_set_length(xoriginal, n, _state);
    ae_v_move(&xoriginal->ptr.p_double[0], 1, &x->ptr.p_double[0], 1, ae_v_len(0,n-1));
    *xa = xmin;
    *xb = xmax;
    for(i=0; i<=n-1; i++)
    {
        x->ptr.p_double[i] = 2*(x->ptr.p_double[i]-0.5*(*xa+(*xb)))/(*xb-(*xa));
    }
    for(i=0; i<=k-1; i++)
    {
        ae_assert(dc->ptr.p_int[i]>=0, "LSFitScaleXY: internal error!", _state);
        xc->ptr.p_double[i] = 2*(xc->ptr.p_double[i]-0.5*(*xa+(*xb)))/(*xb-(*xa));
        yc->ptr.p_double[i] = yc->ptr.p_double[i]*ae_pow(0.5*(*xb-(*xa)), dc->ptr.p_int[i], _state);
    }
    
    /*
     * Transform function values: map [SA,SB] to [0,1]
     * SA = mean(Y),
     * SB = SA+stddev(Y).
     *
     * Store old Y[] in YOriginal[] (it will be used
     * to calculate relative error).
     */
    ae_vector_set_length(yoriginal, n, _state);
    ae_v_move(&yoriginal->ptr.p_double[0], 1, &y->ptr.p_double[0], 1, ae_v_len(0,n-1));
    *sa = 0;
    for(i=0; i<=n-1; i++)
    {
        *sa = *sa+y->ptr.p_double[i];
    }
    *sa = *sa/n;
    *sb = 0;
    for(i=0; i<=n-1; i++)
    {
        *sb = *sb+ae_sqr(y->ptr.p_double[i]-(*sa), _state);
    }
    *sb = ae_sqrt(*sb/n, _state)+(*sa);
    if( ae_fp_eq(*sb,*sa) )
    {
        *sb = 2*(*sa);
    }
    if( ae_fp_eq(*sb,*sa) )
    {
        *sb = *sa+1;
    }
    for(i=0; i<=n-1; i++)
    {
        y->ptr.p_double[i] = (y->ptr.p_double[i]-(*sa))/(*sb-(*sa));
    }
    for(i=0; i<=k-1; i++)
    {
        if( dc->ptr.p_int[i]==0 )
        {
            yc->ptr.p_double[i] = (yc->ptr.p_double[i]-(*sa))/(*sb-(*sa));
        }
        else
        {
            yc->ptr.p_double[i] = yc->ptr.p_double[i]/(*sb-(*sa));
        }
    }
    
    /*
     * Scale weights
     */
    mx = 0;
    for(i=0; i<=n-1; i++)
    {
        mx = ae_maxreal(mx, ae_fabs(w->ptr.p_double[i], _state), _state);
    }
    if( ae_fp_neq(mx,0) )
    {
        for(i=0; i<=n-1; i++)
        {
            w->ptr.p_double[i] = w->ptr.p_double[i]/mx;
        }
    }
}


/*************************************************************************
Internal spline fitting subroutine

  -- ALGLIB PROJECT --
     Copyright 08.09.2009 by Bochkanov Sergey
*************************************************************************/
static void lsfit_spline1dfitinternal(ae_int_t st,
     /* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _w;
    ae_vector _xc;
    ae_vector _yc;
    ae_matrix fmatrix;
    ae_matrix cmatrix;
    ae_vector y2;
    ae_vector w2;
    ae_vector sx;
    ae_vector sy;
    ae_vector sd;
    ae_vector tmp;
    ae_vector xoriginal;
    ae_vector yoriginal;
    lsfitreport lrep;
    double v0;
    double v1;
    double v2;
    double mx;
    spline1dinterpolant s2;
    ae_int_t i;
    ae_int_t j;
    ae_int_t relcnt;
    double xa;
    double xb;
    double sa;
    double sb;
    double bl;
    double br;
    double decay;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_w, w, _state, ae_true);
    w = &_w;
    ae_vector_init_copy(&_xc, xc, _state, ae_true);
    xc = &_xc;
    ae_vector_init_copy(&_yc, yc, _state, ae_true);
    yc = &_yc;
    *info = 0;
    _spline1dinterpolant_clear(s);
    _spline1dfitreport_clear(rep);
    ae_matrix_init(&fmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&cmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sy, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sd, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xoriginal, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yoriginal, 0, DT_REAL, _state, ae_true);
    _lsfitreport_init(&lrep, _state, ae_true);
    _spline1dinterpolant_init(&s2, _state, ae_true);

    ae_assert(st==0||st==1, "Spline1DFit: internal error!", _state);
    if( st==0&&m<4 )
    {
        *info = -1;
        ae_frame_leave(_state);
        return;
    }
    if( st==1&&m<4 )
    {
        *info = -1;
        ae_frame_leave(_state);
        return;
    }
    if( (n<1||k<0)||k>=m )
    {
        *info = -1;
        ae_frame_leave(_state);
        return;
    }
    for(i=0; i<=k-1; i++)
    {
        *info = 0;
        if( dc->ptr.p_int[i]<0 )
        {
            *info = -1;
        }
        if( dc->ptr.p_int[i]>1 )
        {
            *info = -1;
        }
        if( *info<0 )
        {
            ae_frame_leave(_state);
            return;
        }
    }
    if( st==1&&m%2!=0 )
    {
        
        /*
         * Hermite fitter must have even number of basis functions
         */
        *info = -2;
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * weight decay for correct handling of task which becomes
     * degenerate after constraints are applied
     */
    decay = 10000*ae_machineepsilon;
    
    /*
     * Scale X, Y, XC, YC
     */
    lsfitscalexy(x, y, w, n, xc, yc, dc, k, &xa, &xb, &sa, &sb, &xoriginal, &yoriginal, _state);
    
    /*
     * allocate space, initialize:
     * * SX     -   grid for basis functions
     * * SY     -   values of basis functions at grid points
     * * FMatrix-   values of basis functions at X[]
     * * CMatrix-   values (derivatives) of basis functions at XC[]
     */
    ae_vector_set_length(&y2, n+m, _state);
    ae_vector_set_length(&w2, n+m, _state);
    ae_matrix_set_length(&fmatrix, n+m, m, _state);
    if( k>0 )
    {
        ae_matrix_set_length(&cmatrix, k, m+1, _state);
    }
    if( st==0 )
    {
        
        /*
         * allocate space for cubic spline
         */
        ae_vector_set_length(&sx, m-2, _state);
        ae_vector_set_length(&sy, m-2, _state);
        for(j=0; j<=m-2-1; j++)
        {
            sx.ptr.p_double[j] = (double)(2*j)/(double)(m-2-1)-1;
        }
    }
    if( st==1 )
    {
        
        /*
         * allocate space for Hermite spline
         */
        ae_vector_set_length(&sx, m/2, _state);
        ae_vector_set_length(&sy, m/2, _state);
        ae_vector_set_length(&sd, m/2, _state);
        for(j=0; j<=m/2-1; j++)
        {
            sx.ptr.p_double[j] = (double)(2*j)/(double)(m/2-1)-1;
        }
    }
    
    /*
     * Prepare design and constraints matrices:
     * * fill constraints matrix
     * * fill first N rows of design matrix with values
     * * fill next M rows of design matrix with regularizing term
     * * append M zeros to Y
     * * append M elements, mean(abs(W)) each, to W
     */
    for(j=0; j<=m-1; j++)
    {
        
        /*
         * prepare Jth basis function
         */
        if( st==0 )
        {
            
            /*
             * cubic spline basis
             */
            for(i=0; i<=m-2-1; i++)
            {
                sy.ptr.p_double[i] = 0;
            }
            bl = 0;
            br = 0;
            if( j<m-2 )
            {
                sy.ptr.p_double[j] = 1;
            }
            if( j==m-2 )
            {
                bl = 1;
            }
            if( j==m-1 )
            {
                br = 1;
            }
            spline1dbuildcubic(&sx, &sy, m-2, 1, bl, 1, br, &s2, _state);
        }
        if( st==1 )
        {
            
            /*
             * Hermite basis
             */
            for(i=0; i<=m/2-1; i++)
            {
                sy.ptr.p_double[i] = 0;
                sd.ptr.p_double[i] = 0;
            }
            if( j%2==0 )
            {
                sy.ptr.p_double[j/2] = 1;
            }
            else
            {
                sd.ptr.p_double[j/2] = 1;
            }
            spline1dbuildhermite(&sx, &sy, &sd, m/2, &s2, _state);
        }
        
        /*
         * values at X[], XC[]
         */
        for(i=0; i<=n-1; i++)
        {
            fmatrix.ptr.pp_double[i][j] = spline1dcalc(&s2, x->ptr.p_double[i], _state);
        }
        for(i=0; i<=k-1; i++)
        {
            ae_assert(dc->ptr.p_int[i]>=0&&dc->ptr.p_int[i]<=2, "Spline1DFit: internal error!", _state);
            spline1ddiff(&s2, xc->ptr.p_double[i], &v0, &v1, &v2, _state);
            if( dc->ptr.p_int[i]==0 )
            {
                cmatrix.ptr.pp_double[i][j] = v0;
            }
            if( dc->ptr.p_int[i]==1 )
            {
                cmatrix.ptr.pp_double[i][j] = v1;
            }
            if( dc->ptr.p_int[i]==2 )
            {
                cmatrix.ptr.pp_double[i][j] = v2;
            }
        }
    }
    for(i=0; i<=k-1; i++)
    {
        cmatrix.ptr.pp_double[i][m] = yc->ptr.p_double[i];
    }
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=m-1; j++)
        {
            if( i==j )
            {
                fmatrix.ptr.pp_double[n+i][j] = decay;
            }
            else
            {
                fmatrix.ptr.pp_double[n+i][j] = 0;
            }
        }
    }
    ae_vector_set_length(&y2, n+m, _state);
    ae_vector_set_length(&w2, n+m, _state);
    ae_v_move(&y2.ptr.p_double[0], 1, &y->ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_v_move(&w2.ptr.p_double[0], 1, &w->ptr.p_double[0], 1, ae_v_len(0,n-1));
    mx = 0;
    for(i=0; i<=n-1; i++)
    {
        mx = mx+ae_fabs(w->ptr.p_double[i], _state);
    }
    mx = mx/n;
    for(i=0; i<=m-1; i++)
    {
        y2.ptr.p_double[n+i] = 0;
        w2.ptr.p_double[n+i] = mx;
    }
    
    /*
     * Solve constrained task
     */
    if( k>0 )
    {
        
        /*
         * solve using regularization
         */
        lsfitlinearwc(&y2, &w2, &fmatrix, &cmatrix, n+m, m, k, info, &tmp, &lrep, _state);
    }
    else
    {
        
        /*
         * no constraints, no regularization needed
         */
        lsfitlinearwc(y, w, &fmatrix, &cmatrix, n, m, k, info, &tmp, &lrep, _state);
    }
    if( *info<0 )
    {
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * Generate spline and scale it
     */
    if( st==0 )
    {
        
        /*
         * cubic spline basis
         */
        ae_v_move(&sy.ptr.p_double[0], 1, &tmp.ptr.p_double[0], 1, ae_v_len(0,m-2-1));
        spline1dbuildcubic(&sx, &sy, m-2, 1, tmp.ptr.p_double[m-2], 1, tmp.ptr.p_double[m-1], s, _state);
    }
    if( st==1 )
    {
        
        /*
         * Hermite basis
         */
        for(i=0; i<=m/2-1; i++)
        {
            sy.ptr.p_double[i] = tmp.ptr.p_double[2*i];
            sd.ptr.p_double[i] = tmp.ptr.p_double[2*i+1];
        }
        spline1dbuildhermite(&sx, &sy, &sd, m/2, s, _state);
    }
    spline1dlintransx(s, 2/(xb-xa), -(xa+xb)/(xb-xa), _state);
    spline1dlintransy(s, sb-sa, sa, _state);
    
    /*
     * Scale absolute errors obtained from LSFitLinearW.
     * Relative error should be calculated separately
     * (because of shifting/scaling of the task)
     */
    rep->taskrcond = lrep.taskrcond;
    rep->rmserror = lrep.rmserror*(sb-sa);
    rep->avgerror = lrep.avgerror*(sb-sa);
    rep->maxerror = lrep.maxerror*(sb-sa);
    rep->avgrelerror = 0;
    relcnt = 0;
    for(i=0; i<=n-1; i++)
    {
        if( ae_fp_neq(yoriginal.ptr.p_double[i],0) )
        {
            rep->avgrelerror = rep->avgrelerror+ae_fabs(spline1dcalc(s, xoriginal.ptr.p_double[i], _state)-yoriginal.ptr.p_double[i], _state)/ae_fabs(yoriginal.ptr.p_double[i], _state);
            relcnt = relcnt+1;
        }
    }
    if( relcnt!=0 )
    {
        rep->avgrelerror = rep->avgrelerror/relcnt;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Internal fitting subroutine
*************************************************************************/
static void lsfit_lsfitlinearinternal(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    double threshold;
    ae_matrix ft;
    ae_matrix q;
    ae_matrix l;
    ae_matrix r;
    ae_vector b;
    ae_vector wmod;
    ae_vector tau;
    ae_int_t i;
    ae_int_t j;
    double v;
    ae_vector sv;
    ae_matrix u;
    ae_matrix vt;
    ae_vector tmp;
    ae_vector utb;
    ae_vector sutb;
    ae_int_t relcnt;

    ae_frame_make(_state, &_frame_block);
    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);
    ae_matrix_init(&ft, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&q, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&l, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&r, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&b, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&wmod, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tau, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sv, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&u, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&vt, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&utb, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sutb, 0, DT_REAL, _state, ae_true);

    if( n<1||m<1 )
    {
        *info = -1;
        ae_frame_leave(_state);
        return;
    }
    *info = 1;
    threshold = ae_sqrt(ae_machineepsilon, _state);
    
    /*
     * Degenerate case, needs special handling
     */
    if( n<m )
    {
        
        /*
         * Create design matrix.
         */
        ae_matrix_set_length(&ft, n, m, _state);
        ae_vector_set_length(&b, n, _state);
        ae_vector_set_length(&wmod, n, _state);
        for(j=0; j<=n-1; j++)
        {
            v = w->ptr.p_double[j];
            ae_v_moved(&ft.ptr.pp_double[j][0], 1, &fmatrix->ptr.pp_double[j][0], 1, ae_v_len(0,m-1), v);
            b.ptr.p_double[j] = w->ptr.p_double[j]*y->ptr.p_double[j];
            wmod.ptr.p_double[j] = 1;
        }
        
        /*
         * LQ decomposition and reduction to M=N
         */
        ae_vector_set_length(c, m, _state);
        for(i=0; i<=m-1; i++)
        {
            c->ptr.p_double[i] = 0;
        }
        rep->taskrcond = 0;
        rmatrixlq(&ft, n, m, &tau, _state);
        rmatrixlqunpackq(&ft, n, m, &tau, n, &q, _state);
        rmatrixlqunpackl(&ft, n, m, &l, _state);
        lsfit_lsfitlinearinternal(&b, &wmod, &l, n, n, info, &tmp, rep, _state);
        if( *info<=0 )
        {
            ae_frame_leave(_state);
            return;
        }
        for(i=0; i<=n-1; i++)
        {
            v = tmp.ptr.p_double[i];
            ae_v_addd(&c->ptr.p_double[0], 1, &q.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
        }
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * N>=M. Generate design matrix and reduce to N=M using
     * QR decomposition.
     */
    ae_matrix_set_length(&ft, n, m, _state);
    ae_vector_set_length(&b, n, _state);
    for(j=0; j<=n-1; j++)
    {
        v = w->ptr.p_double[j];
        ae_v_moved(&ft.ptr.pp_double[j][0], 1, &fmatrix->ptr.pp_double[j][0], 1, ae_v_len(0,m-1), v);
        b.ptr.p_double[j] = w->ptr.p_double[j]*y->ptr.p_double[j];
    }
    rmatrixqr(&ft, n, m, &tau, _state);
    rmatrixqrunpackq(&ft, n, m, &tau, m, &q, _state);
    rmatrixqrunpackr(&ft, n, m, &r, _state);
    ae_vector_set_length(&tmp, m, _state);
    for(i=0; i<=m-1; i++)
    {
        tmp.ptr.p_double[i] = 0;
    }
    for(i=0; i<=n-1; i++)
    {
        v = b.ptr.p_double[i];
        ae_v_addd(&tmp.ptr.p_double[0], 1, &q.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
    }
    ae_vector_set_length(&b, m, _state);
    ae_v_move(&b.ptr.p_double[0], 1, &tmp.ptr.p_double[0], 1, ae_v_len(0,m-1));
    
    /*
     * R contains reduced MxM design upper triangular matrix,
     * B contains reduced Mx1 right part.
     *
     * Determine system condition number and decide
     * should we use triangular solver (faster) or
     * SVD-based solver (more stable).
     *
     * We can use LU-based RCond estimator for this task.
     */
    rep->taskrcond = rmatrixlurcondinf(&r, m, _state);
    if( ae_fp_greater(rep->taskrcond,threshold) )
    {
        
        /*
         * use QR-based solver
         */
        ae_vector_set_length(c, m, _state);
        c->ptr.p_double[m-1] = b.ptr.p_double[m-1]/r.ptr.pp_double[m-1][m-1];
        for(i=m-2; i>=0; i--)
        {
            v = ae_v_dotproduct(&r.ptr.pp_double[i][i+1], 1, &c->ptr.p_double[i+1], 1, ae_v_len(i+1,m-1));
            c->ptr.p_double[i] = (b.ptr.p_double[i]-v)/r.ptr.pp_double[i][i];
        }
    }
    else
    {
        
        /*
         * use SVD-based solver
         */
        if( !rmatrixsvd(&r, m, m, 1, 1, 2, &sv, &u, &vt, _state) )
        {
            *info = -4;
            ae_frame_leave(_state);
            return;
        }
        ae_vector_set_length(&utb, m, _state);
        ae_vector_set_length(&sutb, m, _state);
        for(i=0; i<=m-1; i++)
        {
            utb.ptr.p_double[i] = 0;
        }
        for(i=0; i<=m-1; i++)
        {
            v = b.ptr.p_double[i];
            ae_v_addd(&utb.ptr.p_double[0], 1, &u.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
        }
        if( ae_fp_greater(sv.ptr.p_double[0],0) )
        {
            rep->taskrcond = sv.ptr.p_double[m-1]/sv.ptr.p_double[0];
            for(i=0; i<=m-1; i++)
            {
                if( ae_fp_greater(sv.ptr.p_double[i],threshold*sv.ptr.p_double[0]) )
                {
                    sutb.ptr.p_double[i] = utb.ptr.p_double[i]/sv.ptr.p_double[i];
                }
                else
                {
                    sutb.ptr.p_double[i] = 0;
                }
            }
        }
        else
        {
            rep->taskrcond = 0;
            for(i=0; i<=m-1; i++)
            {
                sutb.ptr.p_double[i] = 0;
            }
        }
        ae_vector_set_length(c, m, _state);
        for(i=0; i<=m-1; i++)
        {
            c->ptr.p_double[i] = 0;
        }
        for(i=0; i<=m-1; i++)
        {
            v = sutb.ptr.p_double[i];
            ae_v_addd(&c->ptr.p_double[0], 1, &vt.ptr.pp_double[i][0], 1, ae_v_len(0,m-1), v);
        }
    }
    
    /*
     * calculate errors
     */
    rep->rmserror = 0;
    rep->avgerror = 0;
    rep->avgrelerror = 0;
    rep->maxerror = 0;
    relcnt = 0;
    for(i=0; i<=n-1; i++)
    {
        v = ae_v_dotproduct(&fmatrix->ptr.pp_double[i][0], 1, &c->ptr.p_double[0], 1, ae_v_len(0,m-1));
        rep->rmserror = rep->rmserror+ae_sqr(v-y->ptr.p_double[i], _state);
        rep->avgerror = rep->avgerror+ae_fabs(v-y->ptr.p_double[i], _state);
        if( ae_fp_neq(y->ptr.p_double[i],0) )
        {
            rep->avgrelerror = rep->avgrelerror+ae_fabs(v-y->ptr.p_double[i], _state)/ae_fabs(y->ptr.p_double[i], _state);
            relcnt = relcnt+1;
        }
        rep->maxerror = ae_maxreal(rep->maxerror, ae_fabs(v-y->ptr.p_double[i], _state), _state);
    }
    rep->rmserror = ae_sqrt(rep->rmserror/n, _state);
    rep->avgerror = rep->avgerror/n;
    if( relcnt!=0 )
    {
        rep->avgrelerror = rep->avgrelerror/relcnt;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Internal subroutine
*************************************************************************/
static void lsfit_lsfitclearrequestfields(lsfitstate* state,
     ae_state *_state)
{


    state->needf = ae_false;
    state->needfg = ae_false;
    state->needfgh = ae_false;
    state->xupdated = ae_false;
}


/*************************************************************************
Internal subroutine, calculates barycentric basis functions.
Used for efficient simultaneous calculation of N basis functions.

  -- ALGLIB --
     Copyright 17.08.2009 by Bochkanov Sergey
*************************************************************************/
static void lsfit_barycentriccalcbasis(barycentricinterpolant* b,
     double t,
     /* Real    */ ae_vector* y,
     ae_state *_state)
{
    double s2;
    double s;
    double v;
    ae_int_t i;
    ae_int_t j;


    
    /*
     * special case: N=1
     */
    if( b->n==1 )
    {
        y->ptr.p_double[0] = 1;
        return;
    }
    
    /*
     * Here we assume that task is normalized, i.e.:
     * 1. abs(Y[i])<=1
     * 2. abs(W[i])<=1
     * 3. X[] is ordered
     *
     * First, we decide: should we use "safe" formula (guarded
     * against overflow) or fast one?
     */
    s = ae_fabs(t-b->x.ptr.p_double[0], _state);
    for(i=0; i<=b->n-1; i++)
    {
        v = b->x.ptr.p_double[i];
        if( ae_fp_eq(v,t) )
        {
            for(j=0; j<=b->n-1; j++)
            {
                y->ptr.p_double[j] = 0;
            }
            y->ptr.p_double[i] = 1;
            return;
        }
        v = ae_fabs(t-v, _state);
        if( ae_fp_less(v,s) )
        {
            s = v;
        }
    }
    s2 = 0;
    for(i=0; i<=b->n-1; i++)
    {
        v = s/(t-b->x.ptr.p_double[i]);
        v = v*b->w.ptr.p_double[i];
        y->ptr.p_double[i] = v;
        s2 = s2+v;
    }
    v = 1/s2;
    ae_v_muld(&y->ptr.p_double[0], 1, ae_v_len(0,b->n-1), v);
}


/*************************************************************************
This is internal function for Chebyshev fitting.

It assumes that input data are normalized:
* X/XC belong to [-1,+1],
* mean(Y)=0, stddev(Y)=1.

It does not checks inputs for errors.

This function is used to fit general (shifted) Chebyshev models, power
basis models or barycentric models.

INPUT PARAMETERS:
    X   -   points, array[0..N-1].
    Y   -   function values, array[0..N-1].
    W   -   weights, array[0..N-1]
    N   -   number of points, N>0.
    XC  -   points where polynomial values/derivatives are constrained,
            array[0..K-1].
    YC  -   values of constraints, array[0..K-1]
    DC  -   array[0..K-1], types of constraints:
            * DC[i]=0   means that P(XC[i])=YC[i]
            * DC[i]=1   means that P'(XC[i])=YC[i]
    K   -   number of constraints, 0<=K<M.
            K=0 means no constraints (XC/YC/DC are not used in such cases)
    M   -   number of basis functions (= polynomial_degree + 1), M>=1

OUTPUT PARAMETERS:
    Info-   same format as in LSFitLinearW() subroutine:
            * Info>0    task is solved
            * Info<=0   an error occured:
                        -4 means inconvergence of internal SVD
                        -3 means inconsistent constraints
    C   -   interpolant in Chebyshev form; [-1,+1] is used as base interval
    Rep -   report, same format as in LSFitLinearW() subroutine.
            Following fields are set:
            * RMSError      rms error on the (X,Y).
            * AvgError      average error on the (X,Y).
            * AvgRelError   average relative error on the non-zero Y
            * MaxError      maximum error
                            NON-WEIGHTED ERRORS ARE CALCULATED

IMPORTANT:
    this subroitine doesn't calculate task's condition number for K<>0.

  -- ALGLIB PROJECT --
     Copyright 10.12.2009 by Bochkanov Sergey
*************************************************************************/
static void lsfit_internalchebyshevfit(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _xc;
    ae_vector _yc;
    ae_vector y2;
    ae_vector w2;
    ae_vector tmp;
    ae_vector tmp2;
    ae_vector tmpdiff;
    ae_vector bx;
    ae_vector by;
    ae_vector bw;
    ae_matrix fmatrix;
    ae_matrix cmatrix;
    ae_int_t i;
    ae_int_t j;
    double mx;
    double decay;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_xc, xc, _state, ae_true);
    xc = &_xc;
    ae_vector_init_copy(&_yc, yc, _state, ae_true);
    yc = &_yc;
    *info = 0;
    ae_vector_clear(c);
    _lsfitreport_clear(rep);
    ae_vector_init(&y2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmpdiff, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&by, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&bw, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&fmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&cmatrix, 0, 0, DT_REAL, _state, ae_true);

    
    /*
     * weight decay for correct handling of task which becomes
     * degenerate after constraints are applied
     */
    decay = 10000*ae_machineepsilon;
    
    /*
     * allocate space, initialize/fill:
     * * FMatrix-   values of basis functions at X[]
     * * CMatrix-   values (derivatives) of basis functions at XC[]
     * * fill constraints matrix
     * * fill first N rows of design matrix with values
     * * fill next M rows of design matrix with regularizing term
     * * append M zeros to Y
     * * append M elements, mean(abs(W)) each, to W
     */
    ae_vector_set_length(&y2, n+m, _state);
    ae_vector_set_length(&w2, n+m, _state);
    ae_vector_set_length(&tmp, m, _state);
    ae_vector_set_length(&tmpdiff, m, _state);
    ae_matrix_set_length(&fmatrix, n+m, m, _state);
    if( k>0 )
    {
        ae_matrix_set_length(&cmatrix, k, m+1, _state);
    }
    
    /*
     * Fill design matrix, Y2, W2:
     * * first N rows with basis functions for original points
     * * next M rows with decay terms
     */
    for(i=0; i<=n-1; i++)
    {
        
        /*
         * prepare Ith row
         * use Tmp for calculations to avoid multidimensional arrays overhead
         */
        for(j=0; j<=m-1; j++)
        {
            if( j==0 )
            {
                tmp.ptr.p_double[j] = 1;
            }
            else
            {
                if( j==1 )
                {
                    tmp.ptr.p_double[j] = x->ptr.p_double[i];
                }
                else
                {
                    tmp.ptr.p_double[j] = 2*x->ptr.p_double[i]*tmp.ptr.p_double[j-1]-tmp.ptr.p_double[j-2];
                }
            }
        }
        ae_v_move(&fmatrix.ptr.pp_double[i][0], 1, &tmp.ptr.p_double[0], 1, ae_v_len(0,m-1));
    }
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=m-1; j++)
        {
            if( i==j )
            {
                fmatrix.ptr.pp_double[n+i][j] = decay;
            }
            else
            {
                fmatrix.ptr.pp_double[n+i][j] = 0;
            }
        }
    }
    ae_v_move(&y2.ptr.p_double[0], 1, &y->ptr.p_double[0], 1, ae_v_len(0,n-1));
    ae_v_move(&w2.ptr.p_double[0], 1, &w->ptr.p_double[0], 1, ae_v_len(0,n-1));
    mx = 0;
    for(i=0; i<=n-1; i++)
    {
        mx = mx+ae_fabs(w->ptr.p_double[i], _state);
    }
    mx = mx/n;
    for(i=0; i<=m-1; i++)
    {
        y2.ptr.p_double[n+i] = 0;
        w2.ptr.p_double[n+i] = mx;
    }
    
    /*
     * fill constraints matrix
     */
    for(i=0; i<=k-1; i++)
    {
        
        /*
         * prepare Ith row
         * use Tmp for basis function values,
         * TmpDiff for basos function derivatives
         */
        for(j=0; j<=m-1; j++)
        {
            if( j==0 )
            {
                tmp.ptr.p_double[j] = 1;
                tmpdiff.ptr.p_double[j] = 0;
            }
            else
            {
                if( j==1 )
                {
                    tmp.ptr.p_double[j] = xc->ptr.p_double[i];
                    tmpdiff.ptr.p_double[j] = 1;
                }
                else
                {
                    tmp.ptr.p_double[j] = 2*xc->ptr.p_double[i]*tmp.ptr.p_double[j-1]-tmp.ptr.p_double[j-2];
                    tmpdiff.ptr.p_double[j] = 2*(tmp.ptr.p_double[j-1]+xc->ptr.p_double[i]*tmpdiff.ptr.p_double[j-1])-tmpdiff.ptr.p_double[j-2];
                }
            }
        }
        if( dc->ptr.p_int[i]==0 )
        {
            ae_v_move(&cmatrix.ptr.pp_double[i][0], 1, &tmp.ptr.p_double[0], 1, ae_v_len(0,m-1));
        }
        if( dc->ptr.p_int[i]==1 )
        {
            ae_v_move(&cmatrix.ptr.pp_double[i][0], 1, &tmpdiff.ptr.p_double[0], 1, ae_v_len(0,m-1));
        }
        cmatrix.ptr.pp_double[i][m] = yc->ptr.p_double[i];
    }
    
    /*
     * Solve constrained task
     */
    if( k>0 )
    {
        
        /*
         * solve using regularization
         */
        lsfitlinearwc(&y2, &w2, &fmatrix, &cmatrix, n+m, m, k, info, c, rep, _state);
    }
    else
    {
        
        /*
         * no constraints, no regularization needed
         */
        lsfitlinearwc(y, w, &fmatrix, &cmatrix, n, m, 0, info, c, rep, _state);
    }
    if( *info<0 )
    {
        ae_frame_leave(_state);
        return;
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Internal Floater-Hormann fitting subroutine for fixed D
*************************************************************************/
static void lsfit_barycentricfitwcfixedd(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     /* Real    */ ae_vector* xc,
     /* Real    */ ae_vector* yc,
     /* Integer */ ae_vector* dc,
     ae_int_t k,
     ae_int_t m,
     ae_int_t d,
     ae_int_t* info,
     barycentricinterpolant* b,
     barycentricfitreport* rep,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_vector _w;
    ae_vector _xc;
    ae_vector _yc;
    ae_matrix fmatrix;
    ae_matrix cmatrix;
    ae_vector y2;
    ae_vector w2;
    ae_vector sx;
    ae_vector sy;
    ae_vector sbf;
    ae_vector xoriginal;
    ae_vector yoriginal;
    ae_vector tmp;
    lsfitreport lrep;
    double v0;
    double v1;
    double mx;
    barycentricinterpolant b2;
    ae_int_t i;
    ae_int_t j;
    ae_int_t relcnt;
    double xa;
    double xb;
    double sa;
    double sb;
    double decay;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_vector_init_copy(&_w, w, _state, ae_true);
    w = &_w;
    ae_vector_init_copy(&_xc, xc, _state, ae_true);
    xc = &_xc;
    ae_vector_init_copy(&_yc, yc, _state, ae_true);
    yc = &_yc;
    *info = 0;
    _barycentricinterpolant_clear(b);
    _barycentricfitreport_clear(rep);
    ae_matrix_init(&fmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&cmatrix, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&w2, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sx, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sy, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&sbf, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&xoriginal, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&yoriginal, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);
    _lsfitreport_init(&lrep, _state, ae_true);
    _barycentricinterpolant_init(&b2, _state, ae_true);

    if( ((n<1||m<2)||k<0)||k>=m )
    {
        *info = -1;
        ae_frame_leave(_state);
        return;
    }
    for(i=0; i<=k-1; i++)
    {
        *info = 0;
        if( dc->ptr.p_int[i]<0 )
        {
            *info = -1;
        }
        if( dc->ptr.p_int[i]>1 )
        {
            *info = -1;
        }
        if( *info<0 )
        {
            ae_frame_leave(_state);
            return;
        }
    }
    
    /*
     * weight decay for correct handling of task which becomes
     * degenerate after constraints are applied
     */
    decay = 10000*ae_machineepsilon;
    
    /*
     * Scale X, Y, XC, YC
     */
    lsfitscalexy(x, y, w, n, xc, yc, dc, k, &xa, &xb, &sa, &sb, &xoriginal, &yoriginal, _state);
    
    /*
     * allocate space, initialize:
     * * FMatrix-   values of basis functions at X[]
     * * CMatrix-   values (derivatives) of basis functions at XC[]
     */
    ae_vector_set_length(&y2, n+m, _state);
    ae_vector_set_length(&w2, n+m, _state);
    ae_matrix_set_length(&fmatrix, n+m, m, _state);
    if( k>0 )
    {
        ae_matrix_set_length(&cmatrix, k, m+1, _state);
    }
    ae_vector_set_length(&y2, n+m, _state);
    ae_vector_set_length(&w2, n+m, _state);
    
    /*
     * Prepare design and constraints matrices:
     * * fill constraints matrix
     * * fill first N rows of design matrix with values
     * * fill next M rows of design matrix with regularizing term
     * * append M zeros to Y
     * * append M elements, mean(abs(W)) each, to W
     */
    ae_vector_set_length(&sx, m, _state);
    ae_vector_set_length(&sy, m, _state);
    ae_vector_set_length(&sbf, m, _state);
    for(j=0; j<=m-1; j++)
    {
        sx.ptr.p_double[j] = (double)(2*j)/(double)(m-1)-1;
    }
    for(i=0; i<=m-1; i++)
    {
        sy.ptr.p_double[i] = 1;
    }
    barycentricbuildfloaterhormann(&sx, &sy, m, d, &b2, _state);
    mx = 0;
    for(i=0; i<=n-1; i++)
    {
        lsfit_barycentriccalcbasis(&b2, x->ptr.p_double[i], &sbf, _state);
        ae_v_move(&fmatrix.ptr.pp_double[i][0], 1, &sbf.ptr.p_double[0], 1, ae_v_len(0,m-1));
        y2.ptr.p_double[i] = y->ptr.p_double[i];
        w2.ptr.p_double[i] = w->ptr.p_double[i];
        mx = mx+ae_fabs(w->ptr.p_double[i], _state)/n;
    }
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=m-1; j++)
        {
            if( i==j )
            {
                fmatrix.ptr.pp_double[n+i][j] = decay;
            }
            else
            {
                fmatrix.ptr.pp_double[n+i][j] = 0;
            }
        }
        y2.ptr.p_double[n+i] = 0;
        w2.ptr.p_double[n+i] = mx;
    }
    if( k>0 )
    {
        for(j=0; j<=m-1; j++)
        {
            for(i=0; i<=m-1; i++)
            {
                sy.ptr.p_double[i] = 0;
            }
            sy.ptr.p_double[j] = 1;
            barycentricbuildfloaterhormann(&sx, &sy, m, d, &b2, _state);
            for(i=0; i<=k-1; i++)
            {
                ae_assert(dc->ptr.p_int[i]>=0&&dc->ptr.p_int[i]<=1, "BarycentricFit: internal error!", _state);
                barycentricdiff1(&b2, xc->ptr.p_double[i], &v0, &v1, _state);
                if( dc->ptr.p_int[i]==0 )
                {
                    cmatrix.ptr.pp_double[i][j] = v0;
                }
                if( dc->ptr.p_int[i]==1 )
                {
                    cmatrix.ptr.pp_double[i][j] = v1;
                }
            }
        }
        for(i=0; i<=k-1; i++)
        {
            cmatrix.ptr.pp_double[i][m] = yc->ptr.p_double[i];
        }
    }
    
    /*
     * Solve constrained task
     */
    if( k>0 )
    {
        
        /*
         * solve using regularization
         */
        lsfitlinearwc(&y2, &w2, &fmatrix, &cmatrix, n+m, m, k, info, &tmp, &lrep, _state);
    }
    else
    {
        
        /*
         * no constraints, no regularization needed
         */
        lsfitlinearwc(y, w, &fmatrix, &cmatrix, n, m, k, info, &tmp, &lrep, _state);
    }
    if( *info<0 )
    {
        ae_frame_leave(_state);
        return;
    }
    
    /*
     * Generate interpolant and scale it
     */
    ae_v_move(&sy.ptr.p_double[0], 1, &tmp.ptr.p_double[0], 1, ae_v_len(0,m-1));
    barycentricbuildfloaterhormann(&sx, &sy, m, d, b, _state);
    barycentriclintransx(b, 2/(xb-xa), -(xa+xb)/(xb-xa), _state);
    barycentriclintransy(b, sb-sa, sa, _state);
    
    /*
     * Scale absolute errors obtained from LSFitLinearW.
     * Relative error should be calculated separately
     * (because of shifting/scaling of the task)
     */
    rep->taskrcond = lrep.taskrcond;
    rep->rmserror = lrep.rmserror*(sb-sa);
    rep->avgerror = lrep.avgerror*(sb-sa);
    rep->maxerror = lrep.maxerror*(sb-sa);
    rep->avgrelerror = 0;
    relcnt = 0;
    for(i=0; i<=n-1; i++)
    {
        if( ae_fp_neq(yoriginal.ptr.p_double[i],0) )
        {
            rep->avgrelerror = rep->avgrelerror+ae_fabs(barycentriccalc(b, xoriginal.ptr.p_double[i], _state)-yoriginal.ptr.p_double[i], _state)/ae_fabs(yoriginal.ptr.p_double[i], _state);
            relcnt = relcnt+1;
        }
    }
    if( relcnt!=0 )
    {
        rep->avgrelerror = rep->avgrelerror/relcnt;
    }
    ae_frame_leave(_state);
}


ae_bool _polynomialfitreport_init(polynomialfitreport* p, ae_state *_state, ae_bool make_automatic)
{
    return ae_true;
}


ae_bool _polynomialfitreport_init_copy(polynomialfitreport* dst, polynomialfitreport* src, ae_state *_state, ae_bool make_automatic)
{
    dst->taskrcond = src->taskrcond;
    dst->rmserror = src->rmserror;
    dst->avgerror = src->avgerror;
    dst->avgrelerror = src->avgrelerror;
    dst->maxerror = src->maxerror;
    return ae_true;
}


void _polynomialfitreport_clear(polynomialfitreport* p)
{
}


ae_bool _barycentricfitreport_init(barycentricfitreport* p, ae_state *_state, ae_bool make_automatic)
{
    return ae_true;
}


ae_bool _barycentricfitreport_init_copy(barycentricfitreport* dst, barycentricfitreport* src, ae_state *_state, ae_bool make_automatic)
{
    dst->taskrcond = src->taskrcond;
    dst->dbest = src->dbest;
    dst->rmserror = src->rmserror;
    dst->avgerror = src->avgerror;
    dst->avgrelerror = src->avgrelerror;
    dst->maxerror = src->maxerror;
    return ae_true;
}


void _barycentricfitreport_clear(barycentricfitreport* p)
{
}


ae_bool _spline1dfitreport_init(spline1dfitreport* p, ae_state *_state, ae_bool make_automatic)
{
    return ae_true;
}


ae_bool _spline1dfitreport_init_copy(spline1dfitreport* dst, spline1dfitreport* src, ae_state *_state, ae_bool make_automatic)
{
    dst->taskrcond = src->taskrcond;
    dst->rmserror = src->rmserror;
    dst->avgerror = src->avgerror;
    dst->avgrelerror = src->avgrelerror;
    dst->maxerror = src->maxerror;
    return ae_true;
}


void _spline1dfitreport_clear(spline1dfitreport* p)
{
}


ae_bool _lsfitreport_init(lsfitreport* p, ae_state *_state, ae_bool make_automatic)
{
    return ae_true;
}


ae_bool _lsfitreport_init_copy(lsfitreport* dst, lsfitreport* src, ae_state *_state, ae_bool make_automatic)
{
    dst->taskrcond = src->taskrcond;
    dst->rmserror = src->rmserror;
    dst->avgerror = src->avgerror;
    dst->avgrelerror = src->avgrelerror;
    dst->maxerror = src->maxerror;
    return ae_true;
}


void _lsfitreport_clear(lsfitreport* p)
{
}


ae_bool _lsfitstate_init(lsfitstate* p, ae_state *_state, ae_bool make_automatic)
{
    if( !ae_matrix_init(&p->taskx, 0, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->tasky, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->w, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->x, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->c, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init(&p->g, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !ae_matrix_init(&p->h, 0, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !_minlmstate_init(&p->optstate, _state, make_automatic) )
        return ae_false;
    if( !_minlmreport_init(&p->optrep, _state, make_automatic) )
        return ae_false;
    if( !_rcommstate_init(&p->rstate, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _lsfitstate_init_copy(lsfitstate* dst, lsfitstate* src, ae_state *_state, ae_bool make_automatic)
{
    dst->n = src->n;
    dst->m = src->m;
    dst->k = src->k;
    dst->epsf = src->epsf;
    dst->epsx = src->epsx;
    dst->maxits = src->maxits;
    dst->stpmax = src->stpmax;
    dst->xrep = src->xrep;
    if( !ae_matrix_init_copy(&dst->taskx, &src->taskx, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->tasky, &src->tasky, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->w, &src->w, _state, make_automatic) )
        return ae_false;
    dst->xupdated = src->xupdated;
    dst->needf = src->needf;
    dst->needfg = src->needfg;
    dst->needfgh = src->needfgh;
    dst->pointindex = src->pointindex;
    if( !ae_vector_init_copy(&dst->x, &src->x, _state, make_automatic) )
        return ae_false;
    if( !ae_vector_init_copy(&dst->c, &src->c, _state, make_automatic) )
        return ae_false;
    dst->f = src->f;
    if( !ae_vector_init_copy(&dst->g, &src->g, _state, make_automatic) )
        return ae_false;
    if( !ae_matrix_init_copy(&dst->h, &src->h, _state, make_automatic) )
        return ae_false;
    dst->repterminationtype = src->repterminationtype;
    dst->reprmserror = src->reprmserror;
    dst->repavgerror = src->repavgerror;
    dst->repavgrelerror = src->repavgrelerror;
    dst->repmaxerror = src->repmaxerror;
    if( !_minlmstate_init_copy(&dst->optstate, &src->optstate, _state, make_automatic) )
        return ae_false;
    if( !_minlmreport_init_copy(&dst->optrep, &src->optrep, _state, make_automatic) )
        return ae_false;
    if( !_rcommstate_init_copy(&dst->rstate, &src->rstate, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


void _lsfitstate_clear(lsfitstate* p)
{
    ae_matrix_clear(&p->taskx);
    ae_vector_clear(&p->tasky);
    ae_vector_clear(&p->w);
    ae_vector_clear(&p->x);
    ae_vector_clear(&p->c);
    ae_vector_clear(&p->g);
    ae_matrix_clear(&p->h);
    _minlmstate_clear(&p->optstate);
    _minlmreport_clear(&p->optrep);
    _rcommstate_clear(&p->rstate);
}




/*************************************************************************
This function  builds  non-periodic 2-dimensional parametric spline  which
starts at (X[0],Y[0]) and ends at (X[N-1],Y[N-1]).

INPUT PARAMETERS:
    XY  -   points, array[0..N-1,0..1].
            XY[I,0:1] corresponds to the Ith point.
            Order of points is important!
    N   -   points count, N>=5 for Akima splines, N>=2 for other types  of
            splines.
    ST  -   spline type:
            * 0     Akima spline
            * 1     parabolically terminated Catmull-Rom spline (Tension=0)
            * 2     parabolically terminated cubic spline
    PT  -   parameterization type:
            * 0     uniform
            * 1     chord length
            * 2     centripetal

OUTPUT PARAMETERS:
    P   -   parametric spline interpolant


NOTES:
* this function  assumes  that  there all consequent points  are distinct.
  I.e. (x0,y0)<>(x1,y1),  (x1,y1)<>(x2,y2),  (x2,y2)<>(x3,y3)  and  so on.
  However, non-consequent points may coincide, i.e. we can  have  (x0,y0)=
  =(x2,y2).

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2build(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline2interpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_matrix _xy;
    ae_vector tmp;

    ae_frame_make(_state, &_frame_block);
    ae_matrix_init_copy(&_xy, xy, _state, ae_true);
    xy = &_xy;
    _pspline2interpolant_clear(p);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);

    ae_assert(st>=0&&st<=2, "PSpline2Build: incorrect spline type!", _state);
    ae_assert(pt>=0&&pt<=2, "PSpline2Build: incorrect parameterization type!", _state);
    if( st==0 )
    {
        ae_assert(n>=5, "PSpline2Build: N<5 (minimum value for Akima splines)!", _state);
    }
    else
    {
        ae_assert(n>=2, "PSpline2Build: N<2!", _state);
    }
    
    /*
     * Prepare
     */
    p->n = n;
    p->periodic = ae_false;
    ae_vector_set_length(&tmp, n, _state);
    
    /*
     * Build parameterization, check that all parameters are distinct
     */
    pspline_pspline2par(xy, n, pt, &p->p, _state);
    ae_assert(aredistinct(&p->p, n, _state), "PSpline2Build: consequent points are too close!", _state);
    
    /*
     * Build splines
     */
    if( st==0 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
        spline1dbuildakima(&p->p, &tmp, n, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
        spline1dbuildakima(&p->p, &tmp, n, &p->y, _state);
    }
    if( st==1 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcatmullrom(&p->p, &tmp, n, 0, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcatmullrom(&p->p, &tmp, n, 0, 0.0, &p->y, _state);
    }
    if( st==2 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcubic(&p->p, &tmp, n, 0, 0.0, 0, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcubic(&p->p, &tmp, n, 0, 0.0, 0, 0.0, &p->y, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This function  builds  non-periodic 3-dimensional parametric spline  which
starts at (X[0],Y[0],Z[0]) and ends at (X[N-1],Y[N-1],Z[N-1]).

Same as PSpline2Build() function, but for 3D, so we  won't  duplicate  its
description here.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3build(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline3interpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_matrix _xy;
    ae_vector tmp;

    ae_frame_make(_state, &_frame_block);
    ae_matrix_init_copy(&_xy, xy, _state, ae_true);
    xy = &_xy;
    _pspline3interpolant_clear(p);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);

    ae_assert(st>=0&&st<=2, "PSpline3Build: incorrect spline type!", _state);
    ae_assert(pt>=0&&pt<=2, "PSpline3Build: incorrect parameterization type!", _state);
    if( st==0 )
    {
        ae_assert(n>=5, "PSpline3Build: N<5 (minimum value for Akima splines)!", _state);
    }
    else
    {
        ae_assert(n>=2, "PSpline3Build: N<2!", _state);
    }
    
    /*
     * Prepare
     */
    p->n = n;
    p->periodic = ae_false;
    ae_vector_set_length(&tmp, n, _state);
    
    /*
     * Build parameterization, check that all parameters are distinct
     */
    pspline_pspline3par(xy, n, pt, &p->p, _state);
    ae_assert(aredistinct(&p->p, n, _state), "PSpline3Build: consequent points are too close!", _state);
    
    /*
     * Build splines
     */
    if( st==0 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
        spline1dbuildakima(&p->p, &tmp, n, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
        spline1dbuildakima(&p->p, &tmp, n, &p->y, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][2], xy->stride, ae_v_len(0,n-1));
        spline1dbuildakima(&p->p, &tmp, n, &p->z, _state);
    }
    if( st==1 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcatmullrom(&p->p, &tmp, n, 0, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcatmullrom(&p->p, &tmp, n, 0, 0.0, &p->y, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][2], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcatmullrom(&p->p, &tmp, n, 0, 0.0, &p->z, _state);
    }
    if( st==2 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcubic(&p->p, &tmp, n, 0, 0.0, 0, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcubic(&p->p, &tmp, n, 0, 0.0, 0, 0.0, &p->y, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xy->ptr.pp_double[0][2], xy->stride, ae_v_len(0,n-1));
        spline1dbuildcubic(&p->p, &tmp, n, 0, 0.0, 0, 0.0, &p->z, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This  function  builds  periodic  2-dimensional  parametric  spline  which
starts at (X[0],Y[0]), goes through all points to (X[N-1],Y[N-1]) and then
back to (X[0],Y[0]).

INPUT PARAMETERS:
    XY  -   points, array[0..N-1,0..1].
            XY[I,0:1] corresponds to the Ith point.
            XY[N-1,0:1] must be different from XY[0,0:1].
            Order of points is important!
    N   -   points count, N>=3 for other types of splines.
    ST  -   spline type:
            * 1     Catmull-Rom spline (Tension=0) with cyclic boundary conditions
            * 2     cubic spline with cyclic boundary conditions
    PT  -   parameterization type:
            * 0     uniform
            * 1     chord length
            * 2     centripetal

OUTPUT PARAMETERS:
    P   -   parametric spline interpolant


NOTES:
* this function  assumes  that there all consequent points  are  distinct.
  I.e. (x0,y0)<>(x1,y1), (x1,y1)<>(x2,y2),  (x2,y2)<>(x3,y3)  and  so  on.
  However, non-consequent points may coincide, i.e. we can  have  (x0,y0)=
  =(x2,y2).
* last point of sequence is NOT equal to the first  point.  You  shouldn't
  make curve "explicitly periodic" by making them equal.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2buildperiodic(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline2interpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_matrix _xy;
    ae_matrix xyp;
    ae_vector tmp;

    ae_frame_make(_state, &_frame_block);
    ae_matrix_init_copy(&_xy, xy, _state, ae_true);
    xy = &_xy;
    _pspline2interpolant_clear(p);
    ae_matrix_init(&xyp, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);

    ae_assert(st>=1&&st<=2, "PSpline2BuildPeriodic: incorrect spline type!", _state);
    ae_assert(pt>=0&&pt<=2, "PSpline2BuildPeriodic: incorrect parameterization type!", _state);
    ae_assert(n>=3, "PSpline2BuildPeriodic: N<3!", _state);
    
    /*
     * Prepare
     */
    p->n = n;
    p->periodic = ae_true;
    ae_vector_set_length(&tmp, n+1, _state);
    ae_matrix_set_length(&xyp, n+1, 2, _state);
    ae_v_move(&xyp.ptr.pp_double[0][0], xyp.stride, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
    ae_v_move(&xyp.ptr.pp_double[0][1], xyp.stride, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
    ae_v_move(&xyp.ptr.pp_double[n][0], 1, &xy->ptr.pp_double[0][0], 1, ae_v_len(0,1));
    
    /*
     * Build parameterization, check that all parameters are distinct
     */
    pspline_pspline2par(&xyp, n+1, pt, &p->p, _state);
    ae_assert(aredistinct(&p->p, n+1, _state), "PSpline2BuildPeriodic: consequent (or first and last) points are too close!", _state);
    
    /*
     * Build splines
     */
    if( st==1 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][0], xyp.stride, ae_v_len(0,n));
        spline1dbuildcatmullrom(&p->p, &tmp, n+1, -1, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][1], xyp.stride, ae_v_len(0,n));
        spline1dbuildcatmullrom(&p->p, &tmp, n+1, -1, 0.0, &p->y, _state);
    }
    if( st==2 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][0], xyp.stride, ae_v_len(0,n));
        spline1dbuildcubic(&p->p, &tmp, n+1, -1, 0.0, -1, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][1], xyp.stride, ae_v_len(0,n));
        spline1dbuildcubic(&p->p, &tmp, n+1, -1, 0.0, -1, 0.0, &p->y, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This  function  builds  periodic  3-dimensional  parametric  spline  which
starts at (X[0],Y[0],Z[0]), goes through all points to (X[N-1],Y[N-1],Z[N-1])
and then back to (X[0],Y[0],Z[0]).

Same as PSpline2Build() function, but for 3D, so we  won't  duplicate  its
description here.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3buildperiodic(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline3interpolant* p,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_matrix _xy;
    ae_matrix xyp;
    ae_vector tmp;

    ae_frame_make(_state, &_frame_block);
    ae_matrix_init_copy(&_xy, xy, _state, ae_true);
    xy = &_xy;
    _pspline3interpolant_clear(p);
    ae_matrix_init(&xyp, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&tmp, 0, DT_REAL, _state, ae_true);

    ae_assert(st>=1&&st<=2, "PSpline3BuildPeriodic: incorrect spline type!", _state);
    ae_assert(pt>=0&&pt<=2, "PSpline3BuildPeriodic: incorrect parameterization type!", _state);
    ae_assert(n>=3, "PSpline3BuildPeriodic: N<3!", _state);
    
    /*
     * Prepare
     */
    p->n = n;
    p->periodic = ae_true;
    ae_vector_set_length(&tmp, n+1, _state);
    ae_matrix_set_length(&xyp, n+1, 3, _state);
    ae_v_move(&xyp.ptr.pp_double[0][0], xyp.stride, &xy->ptr.pp_double[0][0], xy->stride, ae_v_len(0,n-1));
    ae_v_move(&xyp.ptr.pp_double[0][1], xyp.stride, &xy->ptr.pp_double[0][1], xy->stride, ae_v_len(0,n-1));
    ae_v_move(&xyp.ptr.pp_double[0][2], xyp.stride, &xy->ptr.pp_double[0][2], xy->stride, ae_v_len(0,n-1));
    ae_v_move(&xyp.ptr.pp_double[n][0], 1, &xy->ptr.pp_double[0][0], 1, ae_v_len(0,2));
    
    /*
     * Build parameterization, check that all parameters are distinct
     */
    pspline_pspline3par(&xyp, n+1, pt, &p->p, _state);
    ae_assert(aredistinct(&p->p, n+1, _state), "PSplineBuild2Periodic: consequent (or first and last) points are too close!", _state);
    
    /*
     * Build splines
     */
    if( st==1 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][0], xyp.stride, ae_v_len(0,n));
        spline1dbuildcatmullrom(&p->p, &tmp, n+1, -1, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][1], xyp.stride, ae_v_len(0,n));
        spline1dbuildcatmullrom(&p->p, &tmp, n+1, -1, 0.0, &p->y, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][2], xyp.stride, ae_v_len(0,n));
        spline1dbuildcatmullrom(&p->p, &tmp, n+1, -1, 0.0, &p->z, _state);
    }
    if( st==2 )
    {
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][0], xyp.stride, ae_v_len(0,n));
        spline1dbuildcubic(&p->p, &tmp, n+1, -1, 0.0, -1, 0.0, &p->x, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][1], xyp.stride, ae_v_len(0,n));
        spline1dbuildcubic(&p->p, &tmp, n+1, -1, 0.0, -1, 0.0, &p->y, _state);
        ae_v_move(&tmp.ptr.p_double[0], 1, &xyp.ptr.pp_double[0][2], xyp.stride, ae_v_len(0,n));
        spline1dbuildcubic(&p->p, &tmp, n+1, -1, 0.0, -1, 0.0, &p->z, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This function returns vector of parameter values correspoding to points.

I.e. for P created from (X[0],Y[0])...(X[N-1],Y[N-1]) and U=TValues(P)  we
have
    (X[0],Y[0]) = PSpline2Calc(P,U[0]),
    (X[1],Y[1]) = PSpline2Calc(P,U[1]),
    (X[2],Y[2]) = PSpline2Calc(P,U[2]),
    ...

INPUT PARAMETERS:
    P   -   parametric spline interpolant

OUTPUT PARAMETERS:
    N   -   array size
    T   -   array[0..N-1]


NOTES:
* for non-periodic splines U[0]=0, U[0]<U[1]<...<U[N-1], U[N-1]=1
* for periodic splines     U[0]=0, U[0]<U[1]<...<U[N-1], U[N-1]<1

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2parametervalues(pspline2interpolant* p,
     ae_int_t* n,
     /* Real    */ ae_vector* t,
     ae_state *_state)
{

    *n = 0;
    ae_vector_clear(t);

    ae_assert(p->n>=2, "PSpline2ParameterValues: internal error!", _state);
    *n = p->n;
    ae_vector_set_length(t, *n, _state);
    ae_v_move(&t->ptr.p_double[0], 1, &p->p.ptr.p_double[0], 1, ae_v_len(0,*n-1));
    t->ptr.p_double[0] = 0;
    if( !p->periodic )
    {
        t->ptr.p_double[*n-1] = 1;
    }
}


/*************************************************************************
This function returns vector of parameter values correspoding to points.

Same as PSpline2ParameterValues(), but for 3D.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3parametervalues(pspline3interpolant* p,
     ae_int_t* n,
     /* Real    */ ae_vector* t,
     ae_state *_state)
{

    *n = 0;
    ae_vector_clear(t);

    ae_assert(p->n>=2, "PSpline3ParameterValues: internal error!", _state);
    *n = p->n;
    ae_vector_set_length(t, *n, _state);
    ae_v_move(&t->ptr.p_double[0], 1, &p->p.ptr.p_double[0], 1, ae_v_len(0,*n-1));
    t->ptr.p_double[0] = 0;
    if( !p->periodic )
    {
        t->ptr.p_double[*n-1] = 1;
    }
}


/*************************************************************************
This function  calculates  the value of the parametric spline for a  given
value of parameter T

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-position
    Y   -   Y-position


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2calc(pspline2interpolant* p,
     double t,
     double* x,
     double* y,
     ae_state *_state)
{

    *x = 0;
    *y = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    *x = spline1dcalc(&p->x, t, _state);
    *y = spline1dcalc(&p->y, t, _state);
}


/*************************************************************************
This function  calculates  the value of the parametric spline for a  given
value of parameter T.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-position
    Y   -   Y-position
    Z   -   Z-position


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3calc(pspline3interpolant* p,
     double t,
     double* x,
     double* y,
     double* z,
     ae_state *_state)
{

    *x = 0;
    *y = 0;
    *z = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    *x = spline1dcalc(&p->x, t, _state);
    *y = spline1dcalc(&p->y, t, _state);
    *z = spline1dcalc(&p->z, t, _state);
}


/*************************************************************************
This function  calculates  tangent vector for a given value of parameter T

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X    -   X-component of tangent vector (normalized)
    Y    -   Y-component of tangent vector (normalized)
    
NOTE:
    X^2+Y^2 is either 1 (for non-zero tangent vector) or 0.


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2tangent(pspline2interpolant* p,
     double t,
     double* x,
     double* y,
     ae_state *_state)
{
    double v;
    double v0;
    double v1;

    *x = 0;
    *y = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    pspline2diff(p, t, &v0, x, &v1, y, _state);
    if( ae_fp_neq(*x,0)||ae_fp_neq(*y,0) )
    {
        
        /*
         * this code is a bit more complex than X^2+Y^2 to avoid
         * overflow for large values of X and Y.
         */
        v = safepythag2(*x, *y, _state);
        *x = *x/v;
        *y = *y/v;
    }
}


/*************************************************************************
This function  calculates  tangent vector for a given value of parameter T

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X    -   X-component of tangent vector (normalized)
    Y    -   Y-component of tangent vector (normalized)
    Z    -   Z-component of tangent vector (normalized)

NOTE:
    X^2+Y^2+Z^2 is either 1 (for non-zero tangent vector) or 0.


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3tangent(pspline3interpolant* p,
     double t,
     double* x,
     double* y,
     double* z,
     ae_state *_state)
{
    double v;
    double v0;
    double v1;
    double v2;

    *x = 0;
    *y = 0;
    *z = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    pspline3diff(p, t, &v0, x, &v1, y, &v2, z, _state);
    if( (ae_fp_neq(*x,0)||ae_fp_neq(*y,0))||ae_fp_neq(*z,0) )
    {
        v = safepythag3(*x, *y, *z, _state);
        *x = *x/v;
        *y = *y/v;
        *z = *z/v;
    }
}


/*************************************************************************
This function calculates derivative, i.e. it returns (dX/dT,dY/dT).

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   X-derivative
    Y   -   Y-value
    DY  -   Y-derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2diff(pspline2interpolant* p,
     double t,
     double* x,
     double* dx,
     double* y,
     double* dy,
     ae_state *_state)
{
    double d2s;

    *x = 0;
    *dx = 0;
    *y = 0;
    *dy = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    spline1ddiff(&p->x, t, x, dx, &d2s, _state);
    spline1ddiff(&p->y, t, y, dy, &d2s, _state);
}


/*************************************************************************
This function calculates derivative, i.e. it returns (dX/dT,dY/dT,dZ/dT).

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   X-derivative
    Y   -   Y-value
    DY  -   Y-derivative
    Z   -   Z-value
    DZ  -   Z-derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3diff(pspline3interpolant* p,
     double t,
     double* x,
     double* dx,
     double* y,
     double* dy,
     double* z,
     double* dz,
     ae_state *_state)
{
    double d2s;

    *x = 0;
    *dx = 0;
    *y = 0;
    *dy = 0;
    *z = 0;
    *dz = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    spline1ddiff(&p->x, t, x, dx, &d2s, _state);
    spline1ddiff(&p->y, t, y, dy, &d2s, _state);
    spline1ddiff(&p->z, t, z, dz, &d2s, _state);
}


/*************************************************************************
This function calculates first and second derivative with respect to T.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   derivative
    D2X -   second derivative
    Y   -   Y-value
    DY  -   derivative
    D2Y -   second derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline2diff2(pspline2interpolant* p,
     double t,
     double* x,
     double* dx,
     double* d2x,
     double* y,
     double* dy,
     double* d2y,
     ae_state *_state)
{

    *x = 0;
    *dx = 0;
    *d2x = 0;
    *y = 0;
    *dy = 0;
    *d2y = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    spline1ddiff(&p->x, t, x, dx, d2x, _state);
    spline1ddiff(&p->y, t, y, dy, d2y, _state);
}


/*************************************************************************
This function calculates first and second derivative with respect to T.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    T   -   point:
            * T in [0,1] corresponds to interval spanned by points
            * for non-periodic splines T<0 (or T>1) correspond to parts of
              the curve before the first (after the last) point
            * for periodic splines T<0 (or T>1) are projected  into  [0,1]
              by making T=T-floor(T).

OUTPUT PARAMETERS:
    X   -   X-value
    DX  -   derivative
    D2X -   second derivative
    Y   -   Y-value
    DY  -   derivative
    D2Y -   second derivative
    Z   -   Z-value
    DZ  -   derivative
    D2Z -   second derivative


  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3diff2(pspline3interpolant* p,
     double t,
     double* x,
     double* dx,
     double* d2x,
     double* y,
     double* dy,
     double* d2y,
     double* z,
     double* dz,
     double* d2z,
     ae_state *_state)
{

    *x = 0;
    *dx = 0;
    *d2x = 0;
    *y = 0;
    *dy = 0;
    *d2y = 0;
    *z = 0;
    *dz = 0;
    *d2z = 0;

    if( p->periodic )
    {
        t = t-ae_ifloor(t, _state);
    }
    spline1ddiff(&p->x, t, x, dx, d2x, _state);
    spline1ddiff(&p->y, t, y, dy, d2y, _state);
    spline1ddiff(&p->z, t, z, dz, d2z, _state);
}


/*************************************************************************
This function  calculates  arc length, i.e. length of  curve  between  t=a
and t=b.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    A,B -   parameter values corresponding to arc ends:
            * B>A will result in positive length returned
            * B<A will result in negative length returned

RESULT:
    length of arc starting at T=A and ending at T=B.


  -- ALGLIB PROJECT --
     Copyright 30.05.2010 by Bochkanov Sergey
*************************************************************************/
double pspline2arclength(pspline2interpolant* p,
     double a,
     double b,
     ae_state *_state)
{
    ae_frame _frame_block;
    autogkstate state;
    autogkreport rep;
    double sx;
    double dsx;
    double d2sx;
    double sy;
    double dsy;
    double d2sy;
    double result;

    ae_frame_make(_state, &_frame_block);
    _autogkstate_init(&state, _state, ae_true);
    _autogkreport_init(&rep, _state, ae_true);

    autogksmooth(a, b, &state, _state);
    while(autogkiteration(&state, _state))
    {
        spline1ddiff(&p->x, state.x, &sx, &dsx, &d2sx, _state);
        spline1ddiff(&p->y, state.x, &sy, &dsy, &d2sy, _state);
        state.f = safepythag2(dsx, dsy, _state);
    }
    autogkresults(&state, &result, &rep, _state);
    ae_assert(rep.terminationtype>0, "PSpline2ArcLength: internal error!", _state);
    ae_frame_leave(_state);
    return result;
}


/*************************************************************************
This function  calculates  arc length, i.e. length of  curve  between  t=a
and t=b.

INPUT PARAMETERS:
    P   -   parametric spline interpolant
    A,B -   parameter values corresponding to arc ends:
            * B>A will result in positive length returned
            * B<A will result in negative length returned

RESULT:
    length of arc starting at T=A and ending at T=B.


  -- ALGLIB PROJECT --
     Copyright 30.05.2010 by Bochkanov Sergey
*************************************************************************/
double pspline3arclength(pspline3interpolant* p,
     double a,
     double b,
     ae_state *_state)
{
    ae_frame _frame_block;
    autogkstate state;
    autogkreport rep;
    double sx;
    double dsx;
    double d2sx;
    double sy;
    double dsy;
    double d2sy;
    double sz;
    double dsz;
    double d2sz;
    double result;

    ae_frame_make(_state, &_frame_block);
    _autogkstate_init(&state, _state, ae_true);
    _autogkreport_init(&rep, _state, ae_true);

    autogksmooth(a, b, &state, _state);
    while(autogkiteration(&state, _state))
    {
        spline1ddiff(&p->x, state.x, &sx, &dsx, &d2sx, _state);
        spline1ddiff(&p->y, state.x, &sy, &dsy, &d2sy, _state);
        spline1ddiff(&p->z, state.x, &sz, &dsz, &d2sz, _state);
        state.f = safepythag3(dsx, dsy, dsz, _state);
    }
    autogkresults(&state, &result, &rep, _state);
    ae_assert(rep.terminationtype>0, "PSpline3ArcLength: internal error!", _state);
    ae_frame_leave(_state);
    return result;
}


/*************************************************************************
Builds non-periodic parameterization for 2-dimensional spline
*************************************************************************/
static void pspline_pspline2par(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t pt,
     /* Real    */ ae_vector* p,
     ae_state *_state)
{
    double v;
    ae_int_t i;

    ae_vector_clear(p);

    ae_assert(pt>=0&&pt<=2, "PSpline2Par: internal error!", _state);
    
    /*
     * Build parameterization:
     * * fill by non-normalized values
     * * normalize them so we have P[0]=0, P[N-1]=1.
     */
    ae_vector_set_length(p, n, _state);
    if( pt==0 )
    {
        for(i=0; i<=n-1; i++)
        {
            p->ptr.p_double[i] = i;
        }
    }
    if( pt==1 )
    {
        p->ptr.p_double[0] = 0;
        for(i=1; i<=n-1; i++)
        {
            p->ptr.p_double[i] = p->ptr.p_double[i-1]+safepythag2(xy->ptr.pp_double[i][0]-xy->ptr.pp_double[i-1][0], xy->ptr.pp_double[i][1]-xy->ptr.pp_double[i-1][1], _state);
        }
    }
    if( pt==2 )
    {
        p->ptr.p_double[0] = 0;
        for(i=1; i<=n-1; i++)
        {
            p->ptr.p_double[i] = p->ptr.p_double[i-1]+ae_sqrt(safepythag2(xy->ptr.pp_double[i][0]-xy->ptr.pp_double[i-1][0], xy->ptr.pp_double[i][1]-xy->ptr.pp_double[i-1][1], _state), _state);
        }
    }
    v = 1/p->ptr.p_double[n-1];
    ae_v_muld(&p->ptr.p_double[0], 1, ae_v_len(0,n-1), v);
}


/*************************************************************************
Builds non-periodic parameterization for 3-dimensional spline
*************************************************************************/
static void pspline_pspline3par(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t pt,
     /* Real    */ ae_vector* p,
     ae_state *_state)
{
    double v;
    ae_int_t i;

    ae_vector_clear(p);

    ae_assert(pt>=0&&pt<=2, "PSpline3Par: internal error!", _state);
    
    /*
     * Build parameterization:
     * * fill by non-normalized values
     * * normalize them so we have P[0]=0, P[N-1]=1.
     */
    ae_vector_set_length(p, n, _state);
    if( pt==0 )
    {
        for(i=0; i<=n-1; i++)
        {
            p->ptr.p_double[i] = i;
        }
    }
    if( pt==1 )
    {
        p->ptr.p_double[0] = 0;
        for(i=1; i<=n-1; i++)
        {
            p->ptr.p_double[i] = p->ptr.p_double[i-1]+safepythag3(xy->ptr.pp_double[i][0]-xy->ptr.pp_double[i-1][0], xy->ptr.pp_double[i][1]-xy->ptr.pp_double[i-1][1], xy->ptr.pp_double[i][2]-xy->ptr.pp_double[i-1][2], _state);
        }
    }
    if( pt==2 )
    {
        p->ptr.p_double[0] = 0;
        for(i=1; i<=n-1; i++)
        {
            p->ptr.p_double[i] = p->ptr.p_double[i-1]+ae_sqrt(safepythag3(xy->ptr.pp_double[i][0]-xy->ptr.pp_double[i-1][0], xy->ptr.pp_double[i][1]-xy->ptr.pp_double[i-1][1], xy->ptr.pp_double[i][2]-xy->ptr.pp_double[i-1][2], _state), _state);
        }
    }
    v = 1/p->ptr.p_double[n-1];
    ae_v_muld(&p->ptr.p_double[0], 1, ae_v_len(0,n-1), v);
}


ae_bool _pspline2interpolant_init(pspline2interpolant* p, ae_state *_state, ae_bool make_automatic)
{
    if( !ae_vector_init(&p->p, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init(&p->x, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init(&p->y, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _pspline2interpolant_init_copy(pspline2interpolant* dst, pspline2interpolant* src, ae_state *_state, ae_bool make_automatic)
{
    dst->n = src->n;
    dst->periodic = src->periodic;
    if( !ae_vector_init_copy(&dst->p, &src->p, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init_copy(&dst->x, &src->x, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init_copy(&dst->y, &src->y, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


void _pspline2interpolant_clear(pspline2interpolant* p)
{
    ae_vector_clear(&p->p);
    _spline1dinterpolant_clear(&p->x);
    _spline1dinterpolant_clear(&p->y);
}


ae_bool _pspline3interpolant_init(pspline3interpolant* p, ae_state *_state, ae_bool make_automatic)
{
    if( !ae_vector_init(&p->p, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init(&p->x, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init(&p->y, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init(&p->z, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _pspline3interpolant_init_copy(pspline3interpolant* dst, pspline3interpolant* src, ae_state *_state, ae_bool make_automatic)
{
    dst->n = src->n;
    dst->periodic = src->periodic;
    if( !ae_vector_init_copy(&dst->p, &src->p, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init_copy(&dst->x, &src->x, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init_copy(&dst->y, &src->y, _state, make_automatic) )
        return ae_false;
    if( !_spline1dinterpolant_init_copy(&dst->z, &src->z, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


void _pspline3interpolant_clear(pspline3interpolant* p)
{
    ae_vector_clear(&p->p);
    _spline1dinterpolant_clear(&p->x);
    _spline1dinterpolant_clear(&p->y);
    _spline1dinterpolant_clear(&p->z);
}




/*************************************************************************
This subroutine builds bilinear spline coefficients table.

Input parameters:
    X   -   spline abscissas, array[0..N-1]
    Y   -   spline ordinates, array[0..M-1]
    F   -   function values, array[0..M-1,0..N-1]
    M,N -   grid size, M>=2, N>=2

Output parameters:
    C   -   spline interpolant

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dbuildbilinear(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* f,
     ae_int_t m,
     ae_int_t n,
     spline2dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_matrix _f;
    ae_int_t i;
    ae_int_t j;
    ae_int_t k;
    ae_int_t tblsize;
    ae_int_t shift;
    double t;
    ae_matrix dx;
    ae_matrix dy;
    ae_matrix dxy;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_matrix_init_copy(&_f, f, _state, ae_true);
    f = &_f;
    _spline2dinterpolant_clear(c);
    ae_matrix_init(&dx, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&dy, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&dxy, 0, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=2&&m>=2, "Spline2DBuildBilinear: N<2 or M<2!", _state);
    
    /*
     * Sort points
     */
    for(j=0; j<=n-1; j++)
    {
        k = j;
        for(i=j+1; i<=n-1; i++)
        {
            if( ae_fp_less(x->ptr.p_double[i],x->ptr.p_double[k]) )
            {
                k = i;
            }
        }
        if( k!=j )
        {
            for(i=0; i<=m-1; i++)
            {
                t = f->ptr.pp_double[i][j];
                f->ptr.pp_double[i][j] = f->ptr.pp_double[i][k];
                f->ptr.pp_double[i][k] = t;
            }
            t = x->ptr.p_double[j];
            x->ptr.p_double[j] = x->ptr.p_double[k];
            x->ptr.p_double[k] = t;
        }
    }
    for(i=0; i<=m-1; i++)
    {
        k = i;
        for(j=i+1; j<=m-1; j++)
        {
            if( ae_fp_less(y->ptr.p_double[j],y->ptr.p_double[k]) )
            {
                k = j;
            }
        }
        if( k!=i )
        {
            for(j=0; j<=n-1; j++)
            {
                t = f->ptr.pp_double[i][j];
                f->ptr.pp_double[i][j] = f->ptr.pp_double[k][j];
                f->ptr.pp_double[k][j] = t;
            }
            t = y->ptr.p_double[i];
            y->ptr.p_double[i] = y->ptr.p_double[k];
            y->ptr.p_double[k] = t;
        }
    }
    
    /*
     * Fill C:
     *  C[0]            -   length(C)
     *  C[1]            -   type(C):
     *                      -1 = bilinear interpolant
     *                      -3 = general cubic spline
     *                           (see BuildBicubicSpline)
     *  C[2]:
     *      N (x count)
     *  C[3]:
     *      M (y count)
     *  C[4]...C[4+N-1]:
     *      x[i], i = 0...N-1
     *  C[4+N]...C[4+N+M-1]:
     *      y[i], i = 0...M-1
     *  C[4+N+M]...C[4+N+M+(N*M-1)]:
     *      f(i,j) table. f(0,0), f(0, 1), f(0,2) and so on...
     */
    c->k = 1;
    tblsize = 4+n+m+n*m;
    ae_vector_set_length(&c->c, tblsize-1+1, _state);
    c->c.ptr.p_double[0] = tblsize;
    c->c.ptr.p_double[1] = -1;
    c->c.ptr.p_double[2] = n;
    c->c.ptr.p_double[3] = m;
    for(i=0; i<=n-1; i++)
    {
        c->c.ptr.p_double[4+i] = x->ptr.p_double[i];
    }
    for(i=0; i<=m-1; i++)
    {
        c->c.ptr.p_double[4+n+i] = y->ptr.p_double[i];
    }
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=n-1; j++)
        {
            shift = i*n+j;
            c->c.ptr.p_double[4+n+m+shift] = f->ptr.pp_double[i][j];
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine builds bicubic spline coefficients table.

Input parameters:
    X   -   spline abscissas, array[0..N-1]
    Y   -   spline ordinates, array[0..M-1]
    F   -   function values, array[0..M-1,0..N-1]
    M,N -   grid size, M>=2, N>=2

Output parameters:
    C   -   spline interpolant

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dbuildbicubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* f,
     ae_int_t m,
     ae_int_t n,
     spline2dinterpolant* c,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_vector _x;
    ae_vector _y;
    ae_matrix _f;
    ae_int_t i;
    ae_int_t j;
    ae_int_t k;
    ae_int_t tblsize;
    ae_int_t shift;
    double t;
    ae_matrix dx;
    ae_matrix dy;
    ae_matrix dxy;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init_copy(&_x, x, _state, ae_true);
    x = &_x;
    ae_vector_init_copy(&_y, y, _state, ae_true);
    y = &_y;
    ae_matrix_init_copy(&_f, f, _state, ae_true);
    f = &_f;
    _spline2dinterpolant_clear(c);
    ae_matrix_init(&dx, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&dy, 0, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&dxy, 0, 0, DT_REAL, _state, ae_true);

    ae_assert(n>=2&&m>=2, "BuildBicubicSpline: N<2 or M<2!", _state);
    
    /*
     * Sort points
     */
    for(j=0; j<=n-1; j++)
    {
        k = j;
        for(i=j+1; i<=n-1; i++)
        {
            if( ae_fp_less(x->ptr.p_double[i],x->ptr.p_double[k]) )
            {
                k = i;
            }
        }
        if( k!=j )
        {
            for(i=0; i<=m-1; i++)
            {
                t = f->ptr.pp_double[i][j];
                f->ptr.pp_double[i][j] = f->ptr.pp_double[i][k];
                f->ptr.pp_double[i][k] = t;
            }
            t = x->ptr.p_double[j];
            x->ptr.p_double[j] = x->ptr.p_double[k];
            x->ptr.p_double[k] = t;
        }
    }
    for(i=0; i<=m-1; i++)
    {
        k = i;
        for(j=i+1; j<=m-1; j++)
        {
            if( ae_fp_less(y->ptr.p_double[j],y->ptr.p_double[k]) )
            {
                k = j;
            }
        }
        if( k!=i )
        {
            for(j=0; j<=n-1; j++)
            {
                t = f->ptr.pp_double[i][j];
                f->ptr.pp_double[i][j] = f->ptr.pp_double[k][j];
                f->ptr.pp_double[k][j] = t;
            }
            t = y->ptr.p_double[i];
            y->ptr.p_double[i] = y->ptr.p_double[k];
            y->ptr.p_double[k] = t;
        }
    }
    
    /*
     * Fill C:
     *  C[0]            -   length(C)
     *  C[1]            -   type(C):
     *                      -1 = bilinear interpolant
     *                           (see BuildBilinearInterpolant)
     *                      -3 = general cubic spline
     *  C[2]:
     *      N (x count)
     *  C[3]:
     *      M (y count)
     *  C[4]...C[4+N-1]:
     *      x[i], i = 0...N-1
     *  C[4+N]...C[4+N+M-1]:
     *      y[i], i = 0...M-1
     *  C[4+N+M]...C[4+N+M+(N*M-1)]:
     *      f(i,j) table. f(0,0), f(0, 1), f(0,2) and so on...
     *  C[4+N+M+N*M]...C[4+N+M+(2*N*M-1)]:
     *      df(i,j)/dx table.
     *  C[4+N+M+2*N*M]...C[4+N+M+(3*N*M-1)]:
     *      df(i,j)/dy table.
     *  C[4+N+M+3*N*M]...C[4+N+M+(4*N*M-1)]:
     *      d2f(i,j)/dxdy table.
     */
    c->k = 3;
    tblsize = 4+n+m+4*n*m;
    ae_vector_set_length(&c->c, tblsize-1+1, _state);
    c->c.ptr.p_double[0] = tblsize;
    c->c.ptr.p_double[1] = -3;
    c->c.ptr.p_double[2] = n;
    c->c.ptr.p_double[3] = m;
    for(i=0; i<=n-1; i++)
    {
        c->c.ptr.p_double[4+i] = x->ptr.p_double[i];
    }
    for(i=0; i<=m-1; i++)
    {
        c->c.ptr.p_double[4+n+i] = y->ptr.p_double[i];
    }
    spline2d_bicubiccalcderivatives(f, x, y, m, n, &dx, &dy, &dxy, _state);
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=n-1; j++)
        {
            shift = i*n+j;
            c->c.ptr.p_double[4+n+m+shift] = f->ptr.pp_double[i][j];
            c->c.ptr.p_double[4+n+m+n*m+shift] = dx.ptr.pp_double[i][j];
            c->c.ptr.p_double[4+n+m+2*n*m+shift] = dy.ptr.pp_double[i][j];
            c->c.ptr.p_double[4+n+m+3*n*m+shift] = dxy.ptr.pp_double[i][j];
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine calculates the value of the bilinear or bicubic spline  at
the given point X.

Input parameters:
    C   -   coefficients table.
            Built by BuildBilinearSpline or BuildBicubicSpline.
    X, Y-   point

Result:
    S(x,y)

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
double spline2dcalc(spline2dinterpolant* c,
     double x,
     double y,
     ae_state *_state)
{
    double v;
    double vx;
    double vy;
    double vxy;
    double result;


    spline2ddiff(c, x, y, &v, &vx, &vy, &vxy, _state);
    result = v;
    return result;
}


/*************************************************************************
This subroutine calculates the value of the bilinear or bicubic spline  at
the given point X and its derivatives.

Input parameters:
    C   -   spline interpolant.
    X, Y-   point

Output parameters:
    F   -   S(x,y)
    FX  -   dS(x,y)/dX
    FY  -   dS(x,y)/dY
    FXY -   d2S(x,y)/dXdY

  -- ALGLIB PROJECT --
     Copyright 05.07.2007 by Bochkanov Sergey
*************************************************************************/
void spline2ddiff(spline2dinterpolant* c,
     double x,
     double y,
     double* f,
     double* fx,
     double* fy,
     double* fxy,
     ae_state *_state)
{
    ae_int_t n;
    ae_int_t m;
    double t;
    double dt;
    double u;
    double du;
    ae_int_t ix;
    ae_int_t iy;
    ae_int_t l;
    ae_int_t r;
    ae_int_t h;
    ae_int_t shift1;
    ae_int_t s1;
    ae_int_t s2;
    ae_int_t s3;
    ae_int_t s4;
    ae_int_t sf;
    ae_int_t sfx;
    ae_int_t sfy;
    ae_int_t sfxy;
    double y1;
    double y2;
    double y3;
    double y4;
    double v;
    double t0;
    double t1;
    double t2;
    double t3;
    double u0;
    double u1;
    double u2;
    double u3;

    *f = 0;
    *fx = 0;
    *fy = 0;
    *fxy = 0;

    ae_assert(ae_round(c->c.ptr.p_double[1], _state)==-1||ae_round(c->c.ptr.p_double[1], _state)==-3, "Spline2DDiff: incorrect C!", _state);
    n = ae_round(c->c.ptr.p_double[2], _state);
    m = ae_round(c->c.ptr.p_double[3], _state);
    
    /*
     * Binary search in the [ x[0], ..., x[n-2] ] (x[n-1] is not included)
     */
    l = 4;
    r = 4+n-2+1;
    while(l!=r-1)
    {
        h = (l+r)/2;
        if( ae_fp_greater_eq(c->c.ptr.p_double[h],x) )
        {
            r = h;
        }
        else
        {
            l = h;
        }
    }
    t = (x-c->c.ptr.p_double[l])/(c->c.ptr.p_double[l+1]-c->c.ptr.p_double[l]);
    dt = 1.0/(c->c.ptr.p_double[l+1]-c->c.ptr.p_double[l]);
    ix = l-4;
    
    /*
     * Binary search in the [ y[0], ..., y[m-2] ] (y[m-1] is not included)
     */
    l = 4+n;
    r = 4+n+(m-2)+1;
    while(l!=r-1)
    {
        h = (l+r)/2;
        if( ae_fp_greater_eq(c->c.ptr.p_double[h],y) )
        {
            r = h;
        }
        else
        {
            l = h;
        }
    }
    u = (y-c->c.ptr.p_double[l])/(c->c.ptr.p_double[l+1]-c->c.ptr.p_double[l]);
    du = 1.0/(c->c.ptr.p_double[l+1]-c->c.ptr.p_double[l]);
    iy = l-(4+n);
    
    /*
     * Prepare F, dF/dX, dF/dY, d2F/dXdY
     */
    *f = 0;
    *fx = 0;
    *fy = 0;
    *fxy = 0;
    
    /*
     * Bilinear interpolation
     */
    if( ae_round(c->c.ptr.p_double[1], _state)==-1 )
    {
        shift1 = 4+n+m;
        y1 = c->c.ptr.p_double[shift1+n*iy+ix];
        y2 = c->c.ptr.p_double[shift1+n*iy+(ix+1)];
        y3 = c->c.ptr.p_double[shift1+n*(iy+1)+(ix+1)];
        y4 = c->c.ptr.p_double[shift1+n*(iy+1)+ix];
        *f = (1-t)*(1-u)*y1+t*(1-u)*y2+t*u*y3+(1-t)*u*y4;
        *fx = (-(1-u)*y1+(1-u)*y2+u*y3-u*y4)*dt;
        *fy = (-(1-t)*y1-t*y2+t*y3+(1-t)*y4)*du;
        *fxy = (y1-y2+y3-y4)*du*dt;
        return;
    }
    
    /*
     * Bicubic interpolation
     */
    if( ae_round(c->c.ptr.p_double[1], _state)==-3 )
    {
        
        /*
         * Prepare info
         */
        t0 = 1;
        t1 = t;
        t2 = ae_sqr(t, _state);
        t3 = t*t2;
        u0 = 1;
        u1 = u;
        u2 = ae_sqr(u, _state);
        u3 = u*u2;
        sf = 4+n+m;
        sfx = 4+n+m+n*m;
        sfy = 4+n+m+2*n*m;
        sfxy = 4+n+m+3*n*m;
        s1 = n*iy+ix;
        s2 = n*iy+(ix+1);
        s3 = n*(iy+1)+(ix+1);
        s4 = n*(iy+1)+ix;
        
        /*
         * Calculate
         */
        v = 1*c->c.ptr.p_double[sf+s1];
        *f = *f+v*t0*u0;
        v = 1*c->c.ptr.p_double[sfy+s1]/du;
        *f = *f+v*t0*u1;
        *fy = *fy+1*v*t0*u0*du;
        v = -3*c->c.ptr.p_double[sf+s1]+3*c->c.ptr.p_double[sf+s4]-2*c->c.ptr.p_double[sfy+s1]/du-1*c->c.ptr.p_double[sfy+s4]/du;
        *f = *f+v*t0*u2;
        *fy = *fy+2*v*t0*u1*du;
        v = 2*c->c.ptr.p_double[sf+s1]-2*c->c.ptr.p_double[sf+s4]+1*c->c.ptr.p_double[sfy+s1]/du+1*c->c.ptr.p_double[sfy+s4]/du;
        *f = *f+v*t0*u3;
        *fy = *fy+3*v*t0*u2*du;
        v = 1*c->c.ptr.p_double[sfx+s1]/dt;
        *f = *f+v*t1*u0;
        *fx = *fx+1*v*t0*u0*dt;
        v = 1*c->c.ptr.p_double[sfxy+s1]/(dt*du);
        *f = *f+v*t1*u1;
        *fx = *fx+1*v*t0*u1*dt;
        *fy = *fy+1*v*t1*u0*du;
        *fxy = *fxy+1*v*t0*u0*dt*du;
        v = -3*c->c.ptr.p_double[sfx+s1]/dt+3*c->c.ptr.p_double[sfx+s4]/dt-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
        *f = *f+v*t1*u2;
        *fx = *fx+1*v*t0*u2*dt;
        *fy = *fy+2*v*t1*u1*du;
        *fxy = *fxy+2*v*t0*u1*dt*du;
        v = 2*c->c.ptr.p_double[sfx+s1]/dt-2*c->c.ptr.p_double[sfx+s4]/dt+1*c->c.ptr.p_double[sfxy+s1]/(dt*du)+1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
        *f = *f+v*t1*u3;
        *fx = *fx+1*v*t0*u3*dt;
        *fy = *fy+3*v*t1*u2*du;
        *fxy = *fxy+3*v*t0*u2*dt*du;
        v = -3*c->c.ptr.p_double[sf+s1]+3*c->c.ptr.p_double[sf+s2]-2*c->c.ptr.p_double[sfx+s1]/dt-1*c->c.ptr.p_double[sfx+s2]/dt;
        *f = *f+v*t2*u0;
        *fx = *fx+2*v*t1*u0*dt;
        v = -3*c->c.ptr.p_double[sfy+s1]/du+3*c->c.ptr.p_double[sfy+s2]/du-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-1*c->c.ptr.p_double[sfxy+s2]/(dt*du);
        *f = *f+v*t2*u1;
        *fx = *fx+2*v*t1*u1*dt;
        *fy = *fy+1*v*t2*u0*du;
        *fxy = *fxy+2*v*t1*u0*dt*du;
        v = 9*c->c.ptr.p_double[sf+s1]-9*c->c.ptr.p_double[sf+s2]+9*c->c.ptr.p_double[sf+s3]-9*c->c.ptr.p_double[sf+s4]+6*c->c.ptr.p_double[sfx+s1]/dt+3*c->c.ptr.p_double[sfx+s2]/dt-3*c->c.ptr.p_double[sfx+s3]/dt-6*c->c.ptr.p_double[sfx+s4]/dt+6*c->c.ptr.p_double[sfy+s1]/du-6*c->c.ptr.p_double[sfy+s2]/du-3*c->c.ptr.p_double[sfy+s3]/du+3*c->c.ptr.p_double[sfy+s4]/du+4*c->c.ptr.p_double[sfxy+s1]/(dt*du)+2*c->c.ptr.p_double[sfxy+s2]/(dt*du)+1*c->c.ptr.p_double[sfxy+s3]/(dt*du)+2*c->c.ptr.p_double[sfxy+s4]/(dt*du);
        *f = *f+v*t2*u2;
        *fx = *fx+2*v*t1*u2*dt;
        *fy = *fy+2*v*t2*u1*du;
        *fxy = *fxy+4*v*t1*u1*dt*du;
        v = -6*c->c.ptr.p_double[sf+s1]+6*c->c.ptr.p_double[sf+s2]-6*c->c.ptr.p_double[sf+s3]+6*c->c.ptr.p_double[sf+s4]-4*c->c.ptr.p_double[sfx+s1]/dt-2*c->c.ptr.p_double[sfx+s2]/dt+2*c->c.ptr.p_double[sfx+s3]/dt+4*c->c.ptr.p_double[sfx+s4]/dt-3*c->c.ptr.p_double[sfy+s1]/du+3*c->c.ptr.p_double[sfy+s2]/du+3*c->c.ptr.p_double[sfy+s3]/du-3*c->c.ptr.p_double[sfy+s4]/du-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-1*c->c.ptr.p_double[sfxy+s2]/(dt*du)-1*c->c.ptr.p_double[sfxy+s3]/(dt*du)-2*c->c.ptr.p_double[sfxy+s4]/(dt*du);
        *f = *f+v*t2*u3;
        *fx = *fx+2*v*t1*u3*dt;
        *fy = *fy+3*v*t2*u2*du;
        *fxy = *fxy+6*v*t1*u2*dt*du;
        v = 2*c->c.ptr.p_double[sf+s1]-2*c->c.ptr.p_double[sf+s2]+1*c->c.ptr.p_double[sfx+s1]/dt+1*c->c.ptr.p_double[sfx+s2]/dt;
        *f = *f+v*t3*u0;
        *fx = *fx+3*v*t2*u0*dt;
        v = 2*c->c.ptr.p_double[sfy+s1]/du-2*c->c.ptr.p_double[sfy+s2]/du+1*c->c.ptr.p_double[sfxy+s1]/(dt*du)+1*c->c.ptr.p_double[sfxy+s2]/(dt*du);
        *f = *f+v*t3*u1;
        *fx = *fx+3*v*t2*u1*dt;
        *fy = *fy+1*v*t3*u0*du;
        *fxy = *fxy+3*v*t2*u0*dt*du;
        v = -6*c->c.ptr.p_double[sf+s1]+6*c->c.ptr.p_double[sf+s2]-6*c->c.ptr.p_double[sf+s3]+6*c->c.ptr.p_double[sf+s4]-3*c->c.ptr.p_double[sfx+s1]/dt-3*c->c.ptr.p_double[sfx+s2]/dt+3*c->c.ptr.p_double[sfx+s3]/dt+3*c->c.ptr.p_double[sfx+s4]/dt-4*c->c.ptr.p_double[sfy+s1]/du+4*c->c.ptr.p_double[sfy+s2]/du+2*c->c.ptr.p_double[sfy+s3]/du-2*c->c.ptr.p_double[sfy+s4]/du-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-2*c->c.ptr.p_double[sfxy+s2]/(dt*du)-1*c->c.ptr.p_double[sfxy+s3]/(dt*du)-1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
        *f = *f+v*t3*u2;
        *fx = *fx+3*v*t2*u2*dt;
        *fy = *fy+2*v*t3*u1*du;
        *fxy = *fxy+6*v*t2*u1*dt*du;
        v = 4*c->c.ptr.p_double[sf+s1]-4*c->c.ptr.p_double[sf+s2]+4*c->c.ptr.p_double[sf+s3]-4*c->c.ptr.p_double[sf+s4]+2*c->c.ptr.p_double[sfx+s1]/dt+2*c->c.ptr.p_double[sfx+s2]/dt-2*c->c.ptr.p_double[sfx+s3]/dt-2*c->c.ptr.p_double[sfx+s4]/dt+2*c->c.ptr.p_double[sfy+s1]/du-2*c->c.ptr.p_double[sfy+s2]/du-2*c->c.ptr.p_double[sfy+s3]/du+2*c->c.ptr.p_double[sfy+s4]/du+1*c->c.ptr.p_double[sfxy+s1]/(dt*du)+1*c->c.ptr.p_double[sfxy+s2]/(dt*du)+1*c->c.ptr.p_double[sfxy+s3]/(dt*du)+1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
        *f = *f+v*t3*u3;
        *fx = *fx+3*v*t2*u3*dt;
        *fy = *fy+3*v*t3*u2*du;
        *fxy = *fxy+9*v*t2*u2*dt*du;
        return;
    }
}


/*************************************************************************
This subroutine unpacks two-dimensional spline into the coefficients table

Input parameters:
    C   -   spline interpolant.

Result:
    M, N-   grid size (x-axis and y-axis)
    Tbl -   coefficients table, unpacked format,
            [0..(N-1)*(M-1)-1, 0..19].
            For I = 0...M-2, J=0..N-2:
                K =  I*(N-1)+J
                Tbl[K,0] = X[j]
                Tbl[K,1] = X[j+1]
                Tbl[K,2] = Y[i]
                Tbl[K,3] = Y[i+1]
                Tbl[K,4] = C00
                Tbl[K,5] = C01
                Tbl[K,6] = C02
                Tbl[K,7] = C03
                Tbl[K,8] = C10
                Tbl[K,9] = C11
                ...
                Tbl[K,19] = C33
            On each grid square spline is equals to:
                S(x) = SUM(c[i,j]*(x^i)*(y^j), i=0..3, j=0..3)
                t = x-x[j]
                u = y-y[i]

  -- ALGLIB PROJECT --
     Copyright 29.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dunpack(spline2dinterpolant* c,
     ae_int_t* m,
     ae_int_t* n,
     /* Real    */ ae_matrix* tbl,
     ae_state *_state)
{
    ae_int_t i;
    ae_int_t j;
    ae_int_t ci;
    ae_int_t cj;
    ae_int_t k;
    ae_int_t p;
    ae_int_t shift;
    ae_int_t s1;
    ae_int_t s2;
    ae_int_t s3;
    ae_int_t s4;
    ae_int_t sf;
    ae_int_t sfx;
    ae_int_t sfy;
    ae_int_t sfxy;
    double y1;
    double y2;
    double y3;
    double y4;
    double dt;
    double du;

    *m = 0;
    *n = 0;
    ae_matrix_clear(tbl);

    ae_assert(ae_round(c->c.ptr.p_double[1], _state)==-3||ae_round(c->c.ptr.p_double[1], _state)==-1, "SplineUnpack2D: incorrect C!", _state);
    *n = ae_round(c->c.ptr.p_double[2], _state);
    *m = ae_round(c->c.ptr.p_double[3], _state);
    ae_matrix_set_length(tbl, (*n-1)*(*m-1)-1+1, 19+1, _state);
    
    /*
     * Fill
     */
    for(i=0; i<=*m-2; i++)
    {
        for(j=0; j<=*n-2; j++)
        {
            p = i*(*n-1)+j;
            tbl->ptr.pp_double[p][0] = c->c.ptr.p_double[4+j];
            tbl->ptr.pp_double[p][1] = c->c.ptr.p_double[4+j+1];
            tbl->ptr.pp_double[p][2] = c->c.ptr.p_double[4+(*n)+i];
            tbl->ptr.pp_double[p][3] = c->c.ptr.p_double[4+(*n)+i+1];
            dt = 1/(tbl->ptr.pp_double[p][1]-tbl->ptr.pp_double[p][0]);
            du = 1/(tbl->ptr.pp_double[p][3]-tbl->ptr.pp_double[p][2]);
            
            /*
             * Bilinear interpolation
             */
            if( ae_round(c->c.ptr.p_double[1], _state)==-1 )
            {
                for(k=4; k<=19; k++)
                {
                    tbl->ptr.pp_double[p][k] = 0;
                }
                shift = 4+(*n)+(*m);
                y1 = c->c.ptr.p_double[shift+*n*i+j];
                y2 = c->c.ptr.p_double[shift+*n*i+(j+1)];
                y3 = c->c.ptr.p_double[shift+*n*(i+1)+(j+1)];
                y4 = c->c.ptr.p_double[shift+*n*(i+1)+j];
                tbl->ptr.pp_double[p][4] = y1;
                tbl->ptr.pp_double[p][4+1*4+0] = y2-y1;
                tbl->ptr.pp_double[p][4+0*4+1] = y4-y1;
                tbl->ptr.pp_double[p][4+1*4+1] = y3-y2-y4+y1;
            }
            
            /*
             * Bicubic interpolation
             */
            if( ae_round(c->c.ptr.p_double[1], _state)==-3 )
            {
                sf = 4+(*n)+(*m);
                sfx = 4+(*n)+(*m)+*n*(*m);
                sfy = 4+(*n)+(*m)+2*(*n)*(*m);
                sfxy = 4+(*n)+(*m)+3*(*n)*(*m);
                s1 = *n*i+j;
                s2 = *n*i+(j+1);
                s3 = *n*(i+1)+(j+1);
                s4 = *n*(i+1)+j;
                tbl->ptr.pp_double[p][4+0*4+0] = 1*c->c.ptr.p_double[sf+s1];
                tbl->ptr.pp_double[p][4+0*4+1] = 1*c->c.ptr.p_double[sfy+s1]/du;
                tbl->ptr.pp_double[p][4+0*4+2] = -3*c->c.ptr.p_double[sf+s1]+3*c->c.ptr.p_double[sf+s4]-2*c->c.ptr.p_double[sfy+s1]/du-1*c->c.ptr.p_double[sfy+s4]/du;
                tbl->ptr.pp_double[p][4+0*4+3] = 2*c->c.ptr.p_double[sf+s1]-2*c->c.ptr.p_double[sf+s4]+1*c->c.ptr.p_double[sfy+s1]/du+1*c->c.ptr.p_double[sfy+s4]/du;
                tbl->ptr.pp_double[p][4+1*4+0] = 1*c->c.ptr.p_double[sfx+s1]/dt;
                tbl->ptr.pp_double[p][4+1*4+1] = 1*c->c.ptr.p_double[sfxy+s1]/(dt*du);
                tbl->ptr.pp_double[p][4+1*4+2] = -3*c->c.ptr.p_double[sfx+s1]/dt+3*c->c.ptr.p_double[sfx+s4]/dt-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
                tbl->ptr.pp_double[p][4+1*4+3] = 2*c->c.ptr.p_double[sfx+s1]/dt-2*c->c.ptr.p_double[sfx+s4]/dt+1*c->c.ptr.p_double[sfxy+s1]/(dt*du)+1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
                tbl->ptr.pp_double[p][4+2*4+0] = -3*c->c.ptr.p_double[sf+s1]+3*c->c.ptr.p_double[sf+s2]-2*c->c.ptr.p_double[sfx+s1]/dt-1*c->c.ptr.p_double[sfx+s2]/dt;
                tbl->ptr.pp_double[p][4+2*4+1] = -3*c->c.ptr.p_double[sfy+s1]/du+3*c->c.ptr.p_double[sfy+s2]/du-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-1*c->c.ptr.p_double[sfxy+s2]/(dt*du);
                tbl->ptr.pp_double[p][4+2*4+2] = 9*c->c.ptr.p_double[sf+s1]-9*c->c.ptr.p_double[sf+s2]+9*c->c.ptr.p_double[sf+s3]-9*c->c.ptr.p_double[sf+s4]+6*c->c.ptr.p_double[sfx+s1]/dt+3*c->c.ptr.p_double[sfx+s2]/dt-3*c->c.ptr.p_double[sfx+s3]/dt-6*c->c.ptr.p_double[sfx+s4]/dt+6*c->c.ptr.p_double[sfy+s1]/du-6*c->c.ptr.p_double[sfy+s2]/du-3*c->c.ptr.p_double[sfy+s3]/du+3*c->c.ptr.p_double[sfy+s4]/du+4*c->c.ptr.p_double[sfxy+s1]/(dt*du)+2*c->c.ptr.p_double[sfxy+s2]/(dt*du)+1*c->c.ptr.p_double[sfxy+s3]/(dt*du)+2*c->c.ptr.p_double[sfxy+s4]/(dt*du);
                tbl->ptr.pp_double[p][4+2*4+3] = -6*c->c.ptr.p_double[sf+s1]+6*c->c.ptr.p_double[sf+s2]-6*c->c.ptr.p_double[sf+s3]+6*c->c.ptr.p_double[sf+s4]-4*c->c.ptr.p_double[sfx+s1]/dt-2*c->c.ptr.p_double[sfx+s2]/dt+2*c->c.ptr.p_double[sfx+s3]/dt+4*c->c.ptr.p_double[sfx+s4]/dt-3*c->c.ptr.p_double[sfy+s1]/du+3*c->c.ptr.p_double[sfy+s2]/du+3*c->c.ptr.p_double[sfy+s3]/du-3*c->c.ptr.p_double[sfy+s4]/du-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-1*c->c.ptr.p_double[sfxy+s2]/(dt*du)-1*c->c.ptr.p_double[sfxy+s3]/(dt*du)-2*c->c.ptr.p_double[sfxy+s4]/(dt*du);
                tbl->ptr.pp_double[p][4+3*4+0] = 2*c->c.ptr.p_double[sf+s1]-2*c->c.ptr.p_double[sf+s2]+1*c->c.ptr.p_double[sfx+s1]/dt+1*c->c.ptr.p_double[sfx+s2]/dt;
                tbl->ptr.pp_double[p][4+3*4+1] = 2*c->c.ptr.p_double[sfy+s1]/du-2*c->c.ptr.p_double[sfy+s2]/du+1*c->c.ptr.p_double[sfxy+s1]/(dt*du)+1*c->c.ptr.p_double[sfxy+s2]/(dt*du);
                tbl->ptr.pp_double[p][4+3*4+2] = -6*c->c.ptr.p_double[sf+s1]+6*c->c.ptr.p_double[sf+s2]-6*c->c.ptr.p_double[sf+s3]+6*c->c.ptr.p_double[sf+s4]-3*c->c.ptr.p_double[sfx+s1]/dt-3*c->c.ptr.p_double[sfx+s2]/dt+3*c->c.ptr.p_double[sfx+s3]/dt+3*c->c.ptr.p_double[sfx+s4]/dt-4*c->c.ptr.p_double[sfy+s1]/du+4*c->c.ptr.p_double[sfy+s2]/du+2*c->c.ptr.p_double[sfy+s3]/du-2*c->c.ptr.p_double[sfy+s4]/du-2*c->c.ptr.p_double[sfxy+s1]/(dt*du)-2*c->c.ptr.p_double[sfxy+s2]/(dt*du)-1*c->c.ptr.p_double[sfxy+s3]/(dt*du)-1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
                tbl->ptr.pp_double[p][4+3*4+3] = 4*c->c.ptr.p_double[sf+s1]-4*c->c.ptr.p_double[sf+s2]+4*c->c.ptr.p_double[sf+s3]-4*c->c.ptr.p_double[sf+s4]+2*c->c.ptr.p_double[sfx+s1]/dt+2*c->c.ptr.p_double[sfx+s2]/dt-2*c->c.ptr.p_double[sfx+s3]/dt-2*c->c.ptr.p_double[sfx+s4]/dt+2*c->c.ptr.p_double[sfy+s1]/du-2*c->c.ptr.p_double[sfy+s2]/du-2*c->c.ptr.p_double[sfy+s3]/du+2*c->c.ptr.p_double[sfy+s4]/du+1*c->c.ptr.p_double[sfxy+s1]/(dt*du)+1*c->c.ptr.p_double[sfxy+s2]/(dt*du)+1*c->c.ptr.p_double[sfxy+s3]/(dt*du)+1*c->c.ptr.p_double[sfxy+s4]/(dt*du);
            }
            
            /*
             * Rescale Cij
             */
            for(ci=0; ci<=3; ci++)
            {
                for(cj=0; cj<=3; cj++)
                {
                    tbl->ptr.pp_double[p][4+ci*4+cj] = tbl->ptr.pp_double[p][4+ci*4+cj]*ae_pow(dt, ci, _state)*ae_pow(du, cj, _state);
                }
            }
        }
    }
}


/*************************************************************************
This subroutine performs linear transformation of the spline argument.

Input parameters:
    C       -   spline interpolant
    AX, BX  -   transformation coefficients: x = A*t + B
    AY, BY  -   transformation coefficients: y = A*u + B
Result:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dlintransxy(spline2dinterpolant* c,
     double ax,
     double bx,
     double ay,
     double by,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    ae_int_t n;
    ae_int_t m;
    double v;
    ae_vector x;
    ae_vector y;
    ae_matrix f;
    ae_int_t typec;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&f, 0, 0, DT_REAL, _state, ae_true);

    typec = ae_round(c->c.ptr.p_double[1], _state);
    ae_assert(typec==-3||typec==-1, "Spline2DLinTransXY: incorrect C!", _state);
    n = ae_round(c->c.ptr.p_double[2], _state);
    m = ae_round(c->c.ptr.p_double[3], _state);
    ae_vector_set_length(&x, n-1+1, _state);
    ae_vector_set_length(&y, m-1+1, _state);
    ae_matrix_set_length(&f, m-1+1, n-1+1, _state);
    for(j=0; j<=n-1; j++)
    {
        x.ptr.p_double[j] = c->c.ptr.p_double[4+j];
    }
    for(i=0; i<=m-1; i++)
    {
        y.ptr.p_double[i] = c->c.ptr.p_double[4+n+i];
    }
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=n-1; j++)
        {
            f.ptr.pp_double[i][j] = c->c.ptr.p_double[4+n+m+i*n+j];
        }
    }
    
    /*
     * Special case: AX=0 or AY=0
     */
    if( ae_fp_eq(ax,0) )
    {
        for(i=0; i<=m-1; i++)
        {
            v = spline2dcalc(c, bx, y.ptr.p_double[i], _state);
            for(j=0; j<=n-1; j++)
            {
                f.ptr.pp_double[i][j] = v;
            }
        }
        if( typec==-3 )
        {
            spline2dbuildbicubic(&x, &y, &f, m, n, c, _state);
        }
        if( typec==-1 )
        {
            spline2dbuildbilinear(&x, &y, &f, m, n, c, _state);
        }
        ax = 1;
        bx = 0;
    }
    if( ae_fp_eq(ay,0) )
    {
        for(j=0; j<=n-1; j++)
        {
            v = spline2dcalc(c, x.ptr.p_double[j], by, _state);
            for(i=0; i<=m-1; i++)
            {
                f.ptr.pp_double[i][j] = v;
            }
        }
        if( typec==-3 )
        {
            spline2dbuildbicubic(&x, &y, &f, m, n, c, _state);
        }
        if( typec==-1 )
        {
            spline2dbuildbilinear(&x, &y, &f, m, n, c, _state);
        }
        ay = 1;
        by = 0;
    }
    
    /*
     * General case: AX<>0, AY<>0
     * Unpack, scale and pack again.
     */
    for(j=0; j<=n-1; j++)
    {
        x.ptr.p_double[j] = (x.ptr.p_double[j]-bx)/ax;
    }
    for(i=0; i<=m-1; i++)
    {
        y.ptr.p_double[i] = (y.ptr.p_double[i]-by)/ay;
    }
    if( typec==-3 )
    {
        spline2dbuildbicubic(&x, &y, &f, m, n, c, _state);
    }
    if( typec==-1 )
    {
        spline2dbuildbilinear(&x, &y, &f, m, n, c, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine performs linear transformation of the spline.

Input parameters:
    C   -   spline interpolant.
    A, B-   transformation coefficients: S2(x,y) = A*S(x,y) + B
    
Output parameters:
    C   -   transformed spline

  -- ALGLIB PROJECT --
     Copyright 30.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dlintransf(spline2dinterpolant* c,
     double a,
     double b,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    ae_int_t n;
    ae_int_t m;
    ae_vector x;
    ae_vector y;
    ae_matrix f;
    ae_int_t typec;

    ae_frame_make(_state, &_frame_block);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    ae_matrix_init(&f, 0, 0, DT_REAL, _state, ae_true);

    typec = ae_round(c->c.ptr.p_double[1], _state);
    ae_assert(typec==-3||typec==-1, "Spline2DLinTransXY: incorrect C!", _state);
    n = ae_round(c->c.ptr.p_double[2], _state);
    m = ae_round(c->c.ptr.p_double[3], _state);
    ae_vector_set_length(&x, n-1+1, _state);
    ae_vector_set_length(&y, m-1+1, _state);
    ae_matrix_set_length(&f, m-1+1, n-1+1, _state);
    for(j=0; j<=n-1; j++)
    {
        x.ptr.p_double[j] = c->c.ptr.p_double[4+j];
    }
    for(i=0; i<=m-1; i++)
    {
        y.ptr.p_double[i] = c->c.ptr.p_double[4+n+i];
    }
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=n-1; j++)
        {
            f.ptr.pp_double[i][j] = a*c->c.ptr.p_double[4+n+m+i*n+j]+b;
        }
    }
    if( typec==-3 )
    {
        spline2dbuildbicubic(&x, &y, &f, m, n, c, _state);
    }
    if( typec==-1 )
    {
        spline2dbuildbilinear(&x, &y, &f, m, n, c, _state);
    }
    ae_frame_leave(_state);
}


/*************************************************************************
This subroutine makes the copy of the spline model.

Input parameters:
    C   -   spline interpolant

Output parameters:
    CC  -   spline copy

  -- ALGLIB PROJECT --
     Copyright 29.06.2007 by Bochkanov Sergey
*************************************************************************/
void spline2dcopy(spline2dinterpolant* c,
     spline2dinterpolant* cc,
     ae_state *_state)
{
    ae_int_t n;

    _spline2dinterpolant_clear(cc);

    ae_assert(c->k==1||c->k==3, "Spline2DCopy: incorrect C!", _state);
    cc->k = c->k;
    n = ae_round(c->c.ptr.p_double[0], _state);
    ae_vector_set_length(&cc->c, n, _state);
    ae_v_move(&cc->c.ptr.p_double[0], 1, &c->c.ptr.p_double[0], 1, ae_v_len(0,n-1));
}


/*************************************************************************
Bicubic spline resampling

Input parameters:
    A           -   function values at the old grid,
                    array[0..OldHeight-1, 0..OldWidth-1]
    OldHeight   -   old grid height, OldHeight>1
    OldWidth    -   old grid width, OldWidth>1
    NewHeight   -   new grid height, NewHeight>1
    NewWidth    -   new grid width, NewWidth>1
    
Output parameters:
    B           -   function values at the new grid,
                    array[0..NewHeight-1, 0..NewWidth-1]

  -- ALGLIB routine --
     15 May, 2007
     Copyright by Bochkanov Sergey
*************************************************************************/
void spline2dresamplebicubic(/* Real    */ ae_matrix* a,
     ae_int_t oldheight,
     ae_int_t oldwidth,
     /* Real    */ ae_matrix* b,
     ae_int_t newheight,
     ae_int_t newwidth,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_matrix buf;
    ae_vector x;
    ae_vector y;
    spline1dinterpolant c;
    ae_int_t i;
    ae_int_t j;
    ae_int_t mw;
    ae_int_t mh;

    ae_frame_make(_state, &_frame_block);
    ae_matrix_clear(b);
    ae_matrix_init(&buf, 0, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&x, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&y, 0, DT_REAL, _state, ae_true);
    _spline1dinterpolant_init(&c, _state, ae_true);

    ae_assert(oldwidth>1&&oldheight>1, "Spline2DResampleBicubic: width/height less than 1", _state);
    ae_assert(newwidth>1&&newheight>1, "Spline2DResampleBicubic: width/height less than 1", _state);
    
    /*
     * Prepare
     */
    mw = ae_maxint(oldwidth, newwidth, _state);
    mh = ae_maxint(oldheight, newheight, _state);
    ae_matrix_set_length(b, newheight-1+1, newwidth-1+1, _state);
    ae_matrix_set_length(&buf, oldheight-1+1, newwidth-1+1, _state);
    ae_vector_set_length(&x, ae_maxint(mw, mh, _state)-1+1, _state);
    ae_vector_set_length(&y, ae_maxint(mw, mh, _state)-1+1, _state);
    
    /*
     * Horizontal interpolation
     */
    for(i=0; i<=oldheight-1; i++)
    {
        
        /*
         * Fill X, Y
         */
        for(j=0; j<=oldwidth-1; j++)
        {
            x.ptr.p_double[j] = (double)j/(double)(oldwidth-1);
            y.ptr.p_double[j] = a->ptr.pp_double[i][j];
        }
        
        /*
         * Interpolate and place result into temporary matrix
         */
        spline1dbuildcubic(&x, &y, oldwidth, 0, 0.0, 0, 0.0, &c, _state);
        for(j=0; j<=newwidth-1; j++)
        {
            buf.ptr.pp_double[i][j] = spline1dcalc(&c, (double)j/(double)(newwidth-1), _state);
        }
    }
    
    /*
     * Vertical interpolation
     */
    for(j=0; j<=newwidth-1; j++)
    {
        
        /*
         * Fill X, Y
         */
        for(i=0; i<=oldheight-1; i++)
        {
            x.ptr.p_double[i] = (double)i/(double)(oldheight-1);
            y.ptr.p_double[i] = buf.ptr.pp_double[i][j];
        }
        
        /*
         * Interpolate and place result into B
         */
        spline1dbuildcubic(&x, &y, oldheight, 0, 0.0, 0, 0.0, &c, _state);
        for(i=0; i<=newheight-1; i++)
        {
            b->ptr.pp_double[i][j] = spline1dcalc(&c, (double)i/(double)(newheight-1), _state);
        }
    }
    ae_frame_leave(_state);
}


/*************************************************************************
Bilinear spline resampling

Input parameters:
    A           -   function values at the old grid,
                    array[0..OldHeight-1, 0..OldWidth-1]
    OldHeight   -   old grid height, OldHeight>1
    OldWidth    -   old grid width, OldWidth>1
    NewHeight   -   new grid height, NewHeight>1
    NewWidth    -   new grid width, NewWidth>1

Output parameters:
    B           -   function values at the new grid,
                    array[0..NewHeight-1, 0..NewWidth-1]

  -- ALGLIB routine --
     09.07.2007
     Copyright by Bochkanov Sergey
*************************************************************************/
void spline2dresamplebilinear(/* Real    */ ae_matrix* a,
     ae_int_t oldheight,
     ae_int_t oldwidth,
     /* Real    */ ae_matrix* b,
     ae_int_t newheight,
     ae_int_t newwidth,
     ae_state *_state)
{
    ae_int_t i;
    ae_int_t j;
    ae_int_t l;
    ae_int_t c;
    double t;
    double u;

    ae_matrix_clear(b);

    ae_matrix_set_length(b, newheight-1+1, newwidth-1+1, _state);
    for(i=0; i<=newheight-1; i++)
    {
        for(j=0; j<=newwidth-1; j++)
        {
            l = i*(oldheight-1)/(newheight-1);
            if( l==oldheight-1 )
            {
                l = oldheight-2;
            }
            u = (double)i/(double)(newheight-1)*(oldheight-1)-l;
            c = j*(oldwidth-1)/(newwidth-1);
            if( c==oldwidth-1 )
            {
                c = oldwidth-2;
            }
            t = (double)(j*(oldwidth-1))/(double)(newwidth-1)-c;
            b->ptr.pp_double[i][j] = (1-t)*(1-u)*a->ptr.pp_double[l][c]+t*(1-u)*a->ptr.pp_double[l][c+1]+t*u*a->ptr.pp_double[l+1][c+1]+(1-t)*u*a->ptr.pp_double[l+1][c];
        }
    }
}


/*************************************************************************
Internal subroutine.
Calculation of the first derivatives and the cross-derivative.
*************************************************************************/
static void spline2d_bicubiccalcderivatives(/* Real    */ ae_matrix* a,
     /* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t m,
     ae_int_t n,
     /* Real    */ ae_matrix* dx,
     /* Real    */ ae_matrix* dy,
     /* Real    */ ae_matrix* dxy,
     ae_state *_state)
{
    ae_frame _frame_block;
    ae_int_t i;
    ae_int_t j;
    ae_vector xt;
    ae_vector ft;
    double s;
    double ds;
    double d2s;
    spline1dinterpolant c;

    ae_frame_make(_state, &_frame_block);
    ae_matrix_clear(dx);
    ae_matrix_clear(dy);
    ae_matrix_clear(dxy);
    ae_vector_init(&xt, 0, DT_REAL, _state, ae_true);
    ae_vector_init(&ft, 0, DT_REAL, _state, ae_true);
    _spline1dinterpolant_init(&c, _state, ae_true);

    ae_matrix_set_length(dx, m, n, _state);
    ae_matrix_set_length(dy, m, n, _state);
    ae_matrix_set_length(dxy, m, n, _state);
    
    /*
     * dF/dX
     */
    ae_vector_set_length(&xt, n, _state);
    ae_vector_set_length(&ft, n, _state);
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=n-1; j++)
        {
            xt.ptr.p_double[j] = x->ptr.p_double[j];
            ft.ptr.p_double[j] = a->ptr.pp_double[i][j];
        }
        spline1dbuildcubic(&xt, &ft, n, 0, 0.0, 0, 0.0, &c, _state);
        for(j=0; j<=n-1; j++)
        {
            spline1ddiff(&c, x->ptr.p_double[j], &s, &ds, &d2s, _state);
            dx->ptr.pp_double[i][j] = ds;
        }
    }
    
    /*
     * dF/dY
     */
    ae_vector_set_length(&xt, m, _state);
    ae_vector_set_length(&ft, m, _state);
    for(j=0; j<=n-1; j++)
    {
        for(i=0; i<=m-1; i++)
        {
            xt.ptr.p_double[i] = y->ptr.p_double[i];
            ft.ptr.p_double[i] = a->ptr.pp_double[i][j];
        }
        spline1dbuildcubic(&xt, &ft, m, 0, 0.0, 0, 0.0, &c, _state);
        for(i=0; i<=m-1; i++)
        {
            spline1ddiff(&c, y->ptr.p_double[i], &s, &ds, &d2s, _state);
            dy->ptr.pp_double[i][j] = ds;
        }
    }
    
    /*
     * d2F/dXdY
     */
    ae_vector_set_length(&xt, n, _state);
    ae_vector_set_length(&ft, n, _state);
    for(i=0; i<=m-1; i++)
    {
        for(j=0; j<=n-1; j++)
        {
            xt.ptr.p_double[j] = x->ptr.p_double[j];
            ft.ptr.p_double[j] = dy->ptr.pp_double[i][j];
        }
        spline1dbuildcubic(&xt, &ft, n, 0, 0.0, 0, 0.0, &c, _state);
        for(j=0; j<=n-1; j++)
        {
            spline1ddiff(&c, x->ptr.p_double[j], &s, &ds, &d2s, _state);
            dxy->ptr.pp_double[i][j] = ds;
        }
    }
    ae_frame_leave(_state);
}


ae_bool _spline2dinterpolant_init(spline2dinterpolant* p, ae_state *_state, ae_bool make_automatic)
{
    if( !ae_vector_init(&p->c, 0, DT_REAL, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


ae_bool _spline2dinterpolant_init_copy(spline2dinterpolant* dst, spline2dinterpolant* src, ae_state *_state, ae_bool make_automatic)
{
    dst->k = src->k;
    if( !ae_vector_init_copy(&dst->c, &src->c, _state, make_automatic) )
        return ae_false;
    return ae_true;
}


void _spline2dinterpolant_clear(spline2dinterpolant* p)
{
    ae_vector_clear(&p->c);
}



}

