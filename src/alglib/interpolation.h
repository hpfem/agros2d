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
#ifndef _interpolation_pkg_h
#define _interpolation_pkg_h
#include "ap.h"
#include "alglibinternal.h"
#include "alglibmisc.h"
#include "linalg.h"
#include "solvers.h"
#include "optimization.h"
#include "specialfunctions.h"
#include "integration.h"

/////////////////////////////////////////////////////////////////////////
//
// THIS SECTION CONTAINS COMPUTATIONAL CORE DECLARATIONS (DATATYPES)
//
/////////////////////////////////////////////////////////////////////////
namespace alglib_impl
{
typedef struct
{
    ae_int_t n;
    ae_int_t nx;
    ae_int_t d;
    double r;
    ae_int_t nw;
    kdtree tree;
    ae_int_t modeltype;
    ae_matrix q;
    ae_vector xbuf;
    ae_vector tbuf;
    ae_vector rbuf;
    ae_matrix xybuf;
    ae_int_t debugsolverfailures;
    double debugworstrcond;
    double debugbestrcond;
} idwinterpolant;
typedef struct
{
    ae_int_t n;
    double sy;
    ae_vector x;
    ae_vector y;
    ae_vector w;
} barycentricinterpolant;
typedef struct
{
    ae_bool periodic;
    ae_int_t n;
    ae_int_t k;
    ae_vector x;
    ae_vector c;
} spline1dinterpolant;
typedef struct
{
    double taskrcond;
    double rmserror;
    double avgerror;
    double avgrelerror;
    double maxerror;
} polynomialfitreport;
typedef struct
{
    double taskrcond;
    ae_int_t dbest;
    double rmserror;
    double avgerror;
    double avgrelerror;
    double maxerror;
} barycentricfitreport;
typedef struct
{
    double taskrcond;
    double rmserror;
    double avgerror;
    double avgrelerror;
    double maxerror;
} spline1dfitreport;
typedef struct
{
    double taskrcond;
    double rmserror;
    double avgerror;
    double avgrelerror;
    double maxerror;
} lsfitreport;
typedef struct
{
    ae_int_t n;
    ae_int_t m;
    ae_int_t k;
    double epsf;
    double epsx;
    ae_int_t maxits;
    double stpmax;
    ae_bool xrep;
    ae_matrix taskx;
    ae_vector tasky;
    ae_vector w;
    ae_bool xupdated;
    ae_bool needf;
    ae_bool needfg;
    ae_bool needfgh;
    ae_int_t pointindex;
    ae_vector x;
    ae_vector c;
    double f;
    ae_vector g;
    ae_matrix h;
    ae_int_t repterminationtype;
    double reprmserror;
    double repavgerror;
    double repavgrelerror;
    double repmaxerror;
    minlmstate optstate;
    minlmreport optrep;
    rcommstate rstate;
} lsfitstate;
typedef struct
{
    ae_int_t n;
    ae_bool periodic;
    ae_vector p;
    spline1dinterpolant x;
    spline1dinterpolant y;
} pspline2interpolant;
typedef struct
{
    ae_int_t n;
    ae_bool periodic;
    ae_vector p;
    spline1dinterpolant x;
    spline1dinterpolant y;
    spline1dinterpolant z;
} pspline3interpolant;
typedef struct
{
    ae_int_t k;
    ae_vector c;
} spline2dinterpolant;

}

/////////////////////////////////////////////////////////////////////////
//
// THIS SECTION CONTAINS C++ INTERFACE
//
/////////////////////////////////////////////////////////////////////////
namespace alglib
{

/*************************************************************************
IDW interpolant.
*************************************************************************/
class _idwinterpolant_owner
{
public:
    _idwinterpolant_owner();
    _idwinterpolant_owner(const _idwinterpolant_owner &rhs);
    _idwinterpolant_owner& operator=(const _idwinterpolant_owner &rhs);
    virtual ~_idwinterpolant_owner();
    alglib_impl::idwinterpolant* c_ptr();
    alglib_impl::idwinterpolant* c_ptr() const;
protected:
    alglib_impl::idwinterpolant *p_struct;
};
class idwinterpolant : public _idwinterpolant_owner
{
public:
    idwinterpolant();
    idwinterpolant(const idwinterpolant &rhs);
    idwinterpolant& operator=(const idwinterpolant &rhs);
    virtual ~idwinterpolant();

};

/*************************************************************************
Barycentric interpolant.
*************************************************************************/
class _barycentricinterpolant_owner
{
public:
    _barycentricinterpolant_owner();
    _barycentricinterpolant_owner(const _barycentricinterpolant_owner &rhs);
    _barycentricinterpolant_owner& operator=(const _barycentricinterpolant_owner &rhs);
    virtual ~_barycentricinterpolant_owner();
    alglib_impl::barycentricinterpolant* c_ptr();
    alglib_impl::barycentricinterpolant* c_ptr() const;
protected:
    alglib_impl::barycentricinterpolant *p_struct;
};
class barycentricinterpolant : public _barycentricinterpolant_owner
{
public:
    barycentricinterpolant();
    barycentricinterpolant(const barycentricinterpolant &rhs);
    barycentricinterpolant& operator=(const barycentricinterpolant &rhs);
    virtual ~barycentricinterpolant();

};



/*************************************************************************
1-dimensional spline inteprolant
*************************************************************************/
class _spline1dinterpolant_owner
{
public:
    _spline1dinterpolant_owner();
    _spline1dinterpolant_owner(const _spline1dinterpolant_owner &rhs);
    _spline1dinterpolant_owner& operator=(const _spline1dinterpolant_owner &rhs);
    virtual ~_spline1dinterpolant_owner();
    alglib_impl::spline1dinterpolant* c_ptr();
    alglib_impl::spline1dinterpolant* c_ptr() const;
protected:
    alglib_impl::spline1dinterpolant *p_struct;
};
class spline1dinterpolant : public _spline1dinterpolant_owner
{
public:
    spline1dinterpolant();
    spline1dinterpolant(const spline1dinterpolant &rhs);
    spline1dinterpolant& operator=(const spline1dinterpolant &rhs);
    virtual ~spline1dinterpolant();

};

/*************************************************************************
Polynomial fitting report:
    TaskRCond       reciprocal of task's condition number
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error
*************************************************************************/
class _polynomialfitreport_owner
{
public:
    _polynomialfitreport_owner();
    _polynomialfitreport_owner(const _polynomialfitreport_owner &rhs);
    _polynomialfitreport_owner& operator=(const _polynomialfitreport_owner &rhs);
    virtual ~_polynomialfitreport_owner();
    alglib_impl::polynomialfitreport* c_ptr();
    alglib_impl::polynomialfitreport* c_ptr() const;
protected:
    alglib_impl::polynomialfitreport *p_struct;
};
class polynomialfitreport : public _polynomialfitreport_owner
{
public:
    polynomialfitreport();
    polynomialfitreport(const polynomialfitreport &rhs);
    polynomialfitreport& operator=(const polynomialfitreport &rhs);
    virtual ~polynomialfitreport();
    double &taskrcond;
    double &rmserror;
    double &avgerror;
    double &avgrelerror;
    double &maxerror;

};


/*************************************************************************
Barycentric fitting report:
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error
    TaskRCond       reciprocal of task's condition number
*************************************************************************/
class _barycentricfitreport_owner
{
public:
    _barycentricfitreport_owner();
    _barycentricfitreport_owner(const _barycentricfitreport_owner &rhs);
    _barycentricfitreport_owner& operator=(const _barycentricfitreport_owner &rhs);
    virtual ~_barycentricfitreport_owner();
    alglib_impl::barycentricfitreport* c_ptr();
    alglib_impl::barycentricfitreport* c_ptr() const;
protected:
    alglib_impl::barycentricfitreport *p_struct;
};
class barycentricfitreport : public _barycentricfitreport_owner
{
public:
    barycentricfitreport();
    barycentricfitreport(const barycentricfitreport &rhs);
    barycentricfitreport& operator=(const barycentricfitreport &rhs);
    virtual ~barycentricfitreport();
    double &taskrcond;
    ae_int_t &dbest;
    double &rmserror;
    double &avgerror;
    double &avgrelerror;
    double &maxerror;

};


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
class _spline1dfitreport_owner
{
public:
    _spline1dfitreport_owner();
    _spline1dfitreport_owner(const _spline1dfitreport_owner &rhs);
    _spline1dfitreport_owner& operator=(const _spline1dfitreport_owner &rhs);
    virtual ~_spline1dfitreport_owner();
    alglib_impl::spline1dfitreport* c_ptr();
    alglib_impl::spline1dfitreport* c_ptr() const;
protected:
    alglib_impl::spline1dfitreport *p_struct;
};
class spline1dfitreport : public _spline1dfitreport_owner
{
public:
    spline1dfitreport();
    spline1dfitreport(const spline1dfitreport &rhs);
    spline1dfitreport& operator=(const spline1dfitreport &rhs);
    virtual ~spline1dfitreport();
    double &taskrcond;
    double &rmserror;
    double &avgerror;
    double &avgrelerror;
    double &maxerror;

};


/*************************************************************************
Least squares fitting report:
    TaskRCond       reciprocal of task's condition number
    RMSError        RMS error
    AvgError        average error
    AvgRelError     average relative error (for non-zero Y[I])
    MaxError        maximum error
*************************************************************************/
class _lsfitreport_owner
{
public:
    _lsfitreport_owner();
    _lsfitreport_owner(const _lsfitreport_owner &rhs);
    _lsfitreport_owner& operator=(const _lsfitreport_owner &rhs);
    virtual ~_lsfitreport_owner();
    alglib_impl::lsfitreport* c_ptr();
    alglib_impl::lsfitreport* c_ptr() const;
protected:
    alglib_impl::lsfitreport *p_struct;
};
class lsfitreport : public _lsfitreport_owner
{
public:
    lsfitreport();
    lsfitreport(const lsfitreport &rhs);
    lsfitreport& operator=(const lsfitreport &rhs);
    virtual ~lsfitreport();
    double &taskrcond;
    double &rmserror;
    double &avgerror;
    double &avgrelerror;
    double &maxerror;

};


/*************************************************************************
Nonlinear fitter.

You should use ALGLIB functions to work with fitter.
Never try to access its fields directly!
*************************************************************************/
class _lsfitstate_owner
{
public:
    _lsfitstate_owner();
    _lsfitstate_owner(const _lsfitstate_owner &rhs);
    _lsfitstate_owner& operator=(const _lsfitstate_owner &rhs);
    virtual ~_lsfitstate_owner();
    alglib_impl::lsfitstate* c_ptr();
    alglib_impl::lsfitstate* c_ptr() const;
protected:
    alglib_impl::lsfitstate *p_struct;
};
class lsfitstate : public _lsfitstate_owner
{
public:
    lsfitstate();
    lsfitstate(const lsfitstate &rhs);
    lsfitstate& operator=(const lsfitstate &rhs);
    virtual ~lsfitstate();
    ae_bool &needf;
    ae_bool &needfg;
    ae_bool &needfgh;
    ae_bool &xupdated;
    real_1d_array c;
    double &f;
    real_1d_array g;
    real_2d_array h;
    real_1d_array x;

};

/*************************************************************************
Parametric spline inteprolant: 2-dimensional curve.

You should not try to access its members directly - use PSpline2XXXXXXXX()
functions instead.
*************************************************************************/
class _pspline2interpolant_owner
{
public:
    _pspline2interpolant_owner();
    _pspline2interpolant_owner(const _pspline2interpolant_owner &rhs);
    _pspline2interpolant_owner& operator=(const _pspline2interpolant_owner &rhs);
    virtual ~_pspline2interpolant_owner();
    alglib_impl::pspline2interpolant* c_ptr();
    alglib_impl::pspline2interpolant* c_ptr() const;
protected:
    alglib_impl::pspline2interpolant *p_struct;
};
class pspline2interpolant : public _pspline2interpolant_owner
{
public:
    pspline2interpolant();
    pspline2interpolant(const pspline2interpolant &rhs);
    pspline2interpolant& operator=(const pspline2interpolant &rhs);
    virtual ~pspline2interpolant();

};


/*************************************************************************
Parametric spline inteprolant: 3-dimensional curve.

You should not try to access its members directly - use PSpline3XXXXXXXX()
functions instead.
*************************************************************************/
class _pspline3interpolant_owner
{
public:
    _pspline3interpolant_owner();
    _pspline3interpolant_owner(const _pspline3interpolant_owner &rhs);
    _pspline3interpolant_owner& operator=(const _pspline3interpolant_owner &rhs);
    virtual ~_pspline3interpolant_owner();
    alglib_impl::pspline3interpolant* c_ptr();
    alglib_impl::pspline3interpolant* c_ptr() const;
protected:
    alglib_impl::pspline3interpolant *p_struct;
};
class pspline3interpolant : public _pspline3interpolant_owner
{
public:
    pspline3interpolant();
    pspline3interpolant(const pspline3interpolant &rhs);
    pspline3interpolant& operator=(const pspline3interpolant &rhs);
    virtual ~pspline3interpolant();

};

/*************************************************************************
2-dimensional spline inteprolant
*************************************************************************/
class _spline2dinterpolant_owner
{
public:
    _spline2dinterpolant_owner();
    _spline2dinterpolant_owner(const _spline2dinterpolant_owner &rhs);
    _spline2dinterpolant_owner& operator=(const _spline2dinterpolant_owner &rhs);
    virtual ~_spline2dinterpolant_owner();
    alglib_impl::spline2dinterpolant* c_ptr();
    alglib_impl::spline2dinterpolant* c_ptr() const;
protected:
    alglib_impl::spline2dinterpolant *p_struct;
};
class spline2dinterpolant : public _spline2dinterpolant_owner
{
public:
    spline2dinterpolant();
    spline2dinterpolant(const spline2dinterpolant &rhs);
    spline2dinterpolant& operator=(const spline2dinterpolant &rhs);
    virtual ~spline2dinterpolant();

};

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
double idwcalc(const idwinterpolant &z, const real_1d_array &x);


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
void idwbuildmodifiedshepard(const real_2d_array &xy, const ae_int_t n, const ae_int_t nx, const ae_int_t d, const ae_int_t nq, const ae_int_t nw, idwinterpolant &z);


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
void idwbuildmodifiedshepardr(const real_2d_array &xy, const ae_int_t n, const ae_int_t nx, const double r, idwinterpolant &z);


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
void idwbuildnoisy(const real_2d_array &xy, const ae_int_t n, const ae_int_t nx, const ae_int_t d, const ae_int_t nq, const ae_int_t nw, idwinterpolant &z);

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
double barycentriccalc(const barycentricinterpolant &b, const double t);


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
void barycentricdiff1(const barycentricinterpolant &b, const double t, double &f, double &df);


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
void barycentricdiff2(const barycentricinterpolant &b, const double t, double &f, double &df, double &d2f);


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
void barycentriclintransx(const barycentricinterpolant &b, const double ca, const double cb);


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
void barycentriclintransy(const barycentricinterpolant &b, const double ca, const double cb);


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
void barycentricunpack(const barycentricinterpolant &b, ae_int_t &n, real_1d_array &x, real_1d_array &y, real_1d_array &w);


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
void barycentricbuildxyw(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, barycentricinterpolant &b);


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
void barycentricbuildfloaterhormann(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t d, barycentricinterpolant &b);

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
void polynomialbar2cheb(const barycentricinterpolant &p, const double a, const double b, real_1d_array &t);


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
void polynomialcheb2bar(const real_1d_array &t, const ae_int_t n, const double a, const double b, barycentricinterpolant &p);
void polynomialcheb2bar(const real_1d_array &t, const double a, const double b, barycentricinterpolant &p);


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
void polynomialbar2pow(const barycentricinterpolant &p, const double c, const double s, real_1d_array &a);
void polynomialbar2pow(const barycentricinterpolant &p, real_1d_array &a);


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
void polynomialpow2bar(const real_1d_array &a, const ae_int_t n, const double c, const double s, barycentricinterpolant &p);
void polynomialpow2bar(const real_1d_array &a, barycentricinterpolant &p);


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
void polynomialbuild(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p);
void polynomialbuild(const real_1d_array &x, const real_1d_array &y, barycentricinterpolant &p);


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
void polynomialbuildeqdist(const double a, const double b, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p);
void polynomialbuildeqdist(const double a, const double b, const real_1d_array &y, barycentricinterpolant &p);


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
void polynomialbuildcheb1(const double a, const double b, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p);
void polynomialbuildcheb1(const double a, const double b, const real_1d_array &y, barycentricinterpolant &p);


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
void polynomialbuildcheb2(const double a, const double b, const real_1d_array &y, const ae_int_t n, barycentricinterpolant &p);
void polynomialbuildcheb2(const double a, const double b, const real_1d_array &y, barycentricinterpolant &p);


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
double polynomialcalceqdist(const double a, const double b, const real_1d_array &f, const ae_int_t n, const double t);
double polynomialcalceqdist(const double a, const double b, const real_1d_array &f, const double t);


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
double polynomialcalccheb1(const double a, const double b, const real_1d_array &f, const ae_int_t n, const double t);
double polynomialcalccheb1(const double a, const double b, const real_1d_array &f, const double t);


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
double polynomialcalccheb2(const double a, const double b, const real_1d_array &f, const ae_int_t n, const double t);
double polynomialcalccheb2(const double a, const double b, const real_1d_array &f, const double t);

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
void spline1dbuildlinear(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, spline1dinterpolant &c);
void spline1dbuildlinear(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c);


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
void spline1dbuildcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, spline1dinterpolant &c);
void spline1dbuildcubic(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c);


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
void spline1dgriddiffcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, real_1d_array &d);
void spline1dgriddiffcubic(const real_1d_array &x, const real_1d_array &y, real_1d_array &d);


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
void spline1dgriddiff2cubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, real_1d_array &d1, real_1d_array &d2);
void spline1dgriddiff2cubic(const real_1d_array &x, const real_1d_array &y, real_1d_array &d1, real_1d_array &d2);


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
void spline1dconvcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, const real_1d_array &x2, const ae_int_t n2, real_1d_array &y2);
void spline1dconvcubic(const real_1d_array &x, const real_1d_array &y, const real_1d_array &x2, real_1d_array &y2);


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
void spline1dconvdiffcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, const real_1d_array &x2, const ae_int_t n2, real_1d_array &y2, real_1d_array &d2);
void spline1dconvdiffcubic(const real_1d_array &x, const real_1d_array &y, const real_1d_array &x2, real_1d_array &y2, real_1d_array &d2);


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
void spline1dconvdiff2cubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundltype, const double boundl, const ae_int_t boundrtype, const double boundr, const real_1d_array &x2, const ae_int_t n2, real_1d_array &y2, real_1d_array &d2, real_1d_array &dd2);
void spline1dconvdiff2cubic(const real_1d_array &x, const real_1d_array &y, const real_1d_array &x2, real_1d_array &y2, real_1d_array &d2, real_1d_array &dd2);


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
void spline1dbuildcatmullrom(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t boundtype, const double tension, spline1dinterpolant &c);
void spline1dbuildcatmullrom(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c);


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
void spline1dbuildhermite(const real_1d_array &x, const real_1d_array &y, const real_1d_array &d, const ae_int_t n, spline1dinterpolant &c);
void spline1dbuildhermite(const real_1d_array &x, const real_1d_array &y, const real_1d_array &d, spline1dinterpolant &c);


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
void spline1dbuildakima(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, spline1dinterpolant &c);
void spline1dbuildakima(const real_1d_array &x, const real_1d_array &y, spline1dinterpolant &c);


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
double spline1dcalc(const spline1dinterpolant &c, const double x);


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
void spline1ddiff(const spline1dinterpolant &c, const double x, double &s, double &ds, double &d2s);


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
void spline1dunpack(const spline1dinterpolant &c, ae_int_t &n, real_2d_array &tbl);


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
void spline1dlintransx(const spline1dinterpolant &c, const double a, const double b);


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
void spline1dlintransy(const spline1dinterpolant &c, const double a, const double b);


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
double spline1dintegrate(const spline1dinterpolant &c, const double x);

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
void polynomialfit(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep);
void polynomialfit(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep);


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
void polynomialfitwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep);
void polynomialfitwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t m, ae_int_t &info, barycentricinterpolant &p, polynomialfitreport &rep);


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
void barycentricfitfloaterhormannwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, barycentricinterpolant &b, barycentricfitreport &rep);


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
void barycentricfitfloaterhormann(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, barycentricinterpolant &b, barycentricfitreport &rep);


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
void spline1dfitpenalized(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);
void spline1dfitpenalized(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);


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
void spline1dfitpenalizedw(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);
void spline1dfitpenalizedw(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t m, const double rho, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);


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
void spline1dfitcubicwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);
void spline1dfitcubicwc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);


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
void spline1dfithermitewc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const ae_int_t n, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t k, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);
void spline1dfithermitewc(const real_1d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &xc, const real_1d_array &yc, const integer_1d_array &dc, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);


/*************************************************************************
Least squares fitting by cubic spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitCubicWC().  See  Spline1DFitCubicWC() for more information
about subroutine parameters (we don't duplicate it here because of length)

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfitcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);
void spline1dfitcubic(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);


/*************************************************************************
Least squares fitting by Hermite spline.

This subroutine is "lightweight" alternative for more complex and feature-
rich Spline1DFitHermiteWC().  See Spline1DFitHermiteWC()  description  for
more information about subroutine parameters (we don't duplicate  it  here
because of length).

  -- ALGLIB PROJECT --
     Copyright 18.08.2009 by Bochkanov Sergey
*************************************************************************/
void spline1dfithermite(const real_1d_array &x, const real_1d_array &y, const ae_int_t n, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);
void spline1dfithermite(const real_1d_array &x, const real_1d_array &y, const ae_int_t m, ae_int_t &info, spline1dinterpolant &s, spline1dfitreport &rep);


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
void lsfitlinearw(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, const ae_int_t n, const ae_int_t m, ae_int_t &info, real_1d_array &c, lsfitreport &rep);
void lsfitlinearw(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep);


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
void lsfitlinearwc(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, const real_2d_array &cmatrix, const ae_int_t n, const ae_int_t m, const ae_int_t k, ae_int_t &info, real_1d_array &c, lsfitreport &rep);
void lsfitlinearwc(const real_1d_array &y, const real_1d_array &w, const real_2d_array &fmatrix, const real_2d_array &cmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep);


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
void lsfitlinear(const real_1d_array &y, const real_2d_array &fmatrix, const ae_int_t n, const ae_int_t m, ae_int_t &info, real_1d_array &c, lsfitreport &rep);
void lsfitlinear(const real_1d_array &y, const real_2d_array &fmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep);


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
void lsfitlinearc(const real_1d_array &y, const real_2d_array &fmatrix, const real_2d_array &cmatrix, const ae_int_t n, const ae_int_t m, const ae_int_t k, ae_int_t &info, real_1d_array &c, lsfitreport &rep);
void lsfitlinearc(const real_1d_array &y, const real_2d_array &fmatrix, const real_2d_array &cmatrix, ae_int_t &info, real_1d_array &c, lsfitreport &rep);


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
void lsfitcreatewf(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const double diffstep, lsfitstate &state);
void lsfitcreatewf(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const double diffstep, lsfitstate &state);


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
void lsfitcreatef(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const double diffstep, lsfitstate &state);
void lsfitcreatef(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const double diffstep, lsfitstate &state);


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
void lsfitcreatewfg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const bool cheapfg, lsfitstate &state);
void lsfitcreatewfg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const bool cheapfg, lsfitstate &state);


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
void lsfitcreatefg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, const bool cheapfg, lsfitstate &state);
void lsfitcreatefg(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const bool cheapfg, lsfitstate &state);


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
void lsfitcreatewfgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, lsfitstate &state);
void lsfitcreatewfgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &w, const real_1d_array &c, lsfitstate &state);


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
void lsfitcreatefgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, const ae_int_t n, const ae_int_t m, const ae_int_t k, lsfitstate &state);
void lsfitcreatefgh(const real_2d_array &x, const real_1d_array &y, const real_1d_array &c, lsfitstate &state);


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
void lsfitsetcond(const lsfitstate &state, const double epsf, const double epsx, const ae_int_t maxits);


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
void lsfitsetstpmax(const lsfitstate &state, const double stpmax);


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
void lsfitsetxrep(const lsfitstate &state, const bool needxrep);


/*************************************************************************
This function provides reverse communication interface
Reverse communication interface is not documented or recommended to use.
See below for functions which provide better documented API
*************************************************************************/
bool lsfititeration(const lsfitstate &state);


/*************************************************************************
This family of functions is used to launcn iterations of nonlinear fitter

These functions accept following parameters:
    state   -   algorithm state
    func    -   callback which calculates function (or merit function)
                value func at given point x
    grad    -   callback which calculates function (or merit function)
                value func and gradient grad at given point x
    hess    -   callback which calculates function (or merit function)
                value func, gradient grad and Hessian hess at given point x
    rep     -   optional callback which is called after each iteration
                can be NULL
    ptr     -   optional pointer which is passed to func/grad/hess/jac/rep
                can be NULL

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
void lsfitfit(lsfitstate &state,
    void (*func)(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr),
    void  (*rep)(const real_1d_array &c, double func, void *ptr) = NULL,
    void *ptr = NULL);
void lsfitfit(lsfitstate &state,
    void (*func)(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr),
    void (*grad)(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, void *ptr),
    void  (*rep)(const real_1d_array &c, double func, void *ptr) = NULL,
    void *ptr = NULL);
void lsfitfit(lsfitstate &state,
    void (*func)(const real_1d_array &c, const real_1d_array &x, double &func, void *ptr),
    void (*grad)(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, void *ptr),
    void (*hess)(const real_1d_array &c, const real_1d_array &x, double &func, real_1d_array &grad, real_2d_array &hess, void *ptr),
    void  (*rep)(const real_1d_array &c, double func, void *ptr) = NULL,
    void *ptr = NULL);


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
void lsfitresults(const lsfitstate &state, ae_int_t &info, real_1d_array &c, lsfitreport &rep);

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
void pspline2build(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline2interpolant &p);


/*************************************************************************
This function  builds  non-periodic 3-dimensional parametric spline  which
starts at (X[0],Y[0],Z[0]) and ends at (X[N-1],Y[N-1],Z[N-1]).

Same as PSpline2Build() function, but for 3D, so we  won't  duplicate  its
description here.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3build(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline3interpolant &p);


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
void pspline2buildperiodic(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline2interpolant &p);


/*************************************************************************
This  function  builds  periodic  3-dimensional  parametric  spline  which
starts at (X[0],Y[0],Z[0]), goes through all points to (X[N-1],Y[N-1],Z[N-1])
and then back to (X[0],Y[0],Z[0]).

Same as PSpline2Build() function, but for 3D, so we  won't  duplicate  its
description here.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3buildperiodic(const real_2d_array &xy, const ae_int_t n, const ae_int_t st, const ae_int_t pt, pspline3interpolant &p);


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
void pspline2parametervalues(const pspline2interpolant &p, ae_int_t &n, real_1d_array &t);


/*************************************************************************
This function returns vector of parameter values correspoding to points.

Same as PSpline2ParameterValues(), but for 3D.

  -- ALGLIB PROJECT --
     Copyright 28.05.2010 by Bochkanov Sergey
*************************************************************************/
void pspline3parametervalues(const pspline3interpolant &p, ae_int_t &n, real_1d_array &t);


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
void pspline2calc(const pspline2interpolant &p, const double t, double &x, double &y);


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
void pspline3calc(const pspline3interpolant &p, const double t, double &x, double &y, double &z);


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
void pspline2tangent(const pspline2interpolant &p, const double t, double &x, double &y);


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
void pspline3tangent(const pspline3interpolant &p, const double t, double &x, double &y, double &z);


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
void pspline2diff(const pspline2interpolant &p, const double t, double &x, double &dx, double &y, double &dy);


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
void pspline3diff(const pspline3interpolant &p, const double t, double &x, double &dx, double &y, double &dy, double &z, double &dz);


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
void pspline2diff2(const pspline2interpolant &p, const double t, double &x, double &dx, double &d2x, double &y, double &dy, double &d2y);


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
void pspline3diff2(const pspline3interpolant &p, const double t, double &x, double &dx, double &d2x, double &y, double &dy, double &d2y, double &z, double &dz, double &d2z);


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
double pspline2arclength(const pspline2interpolant &p, const double a, const double b);


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
double pspline3arclength(const pspline3interpolant &p, const double a, const double b);

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
void spline2dbuildbilinear(const real_1d_array &x, const real_1d_array &y, const real_2d_array &f, const ae_int_t m, const ae_int_t n, spline2dinterpolant &c);


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
void spline2dbuildbicubic(const real_1d_array &x, const real_1d_array &y, const real_2d_array &f, const ae_int_t m, const ae_int_t n, spline2dinterpolant &c);


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
double spline2dcalc(const spline2dinterpolant &c, const double x, const double y);


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
void spline2ddiff(const spline2dinterpolant &c, const double x, const double y, double &f, double &fx, double &fy, double &fxy);


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
void spline2dunpack(const spline2dinterpolant &c, ae_int_t &m, ae_int_t &n, real_2d_array &tbl);


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
void spline2dlintransxy(const spline2dinterpolant &c, const double ax, const double bx, const double ay, const double by);


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
void spline2dlintransf(const spline2dinterpolant &c, const double a, const double b);


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
void spline2dresamplebicubic(const real_2d_array &a, const ae_int_t oldheight, const ae_int_t oldwidth, real_2d_array &b, const ae_int_t newheight, const ae_int_t newwidth);


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
void spline2dresamplebilinear(const real_2d_array &a, const ae_int_t oldheight, const ae_int_t oldwidth, real_2d_array &b, const ae_int_t newheight, const ae_int_t newwidth);
}

/////////////////////////////////////////////////////////////////////////
//
// THIS SECTION CONTAINS COMPUTATIONAL CORE DECLARATIONS (FUNCTIONS)
//
/////////////////////////////////////////////////////////////////////////
namespace alglib_impl
{
double idwcalc(idwinterpolant* z,
     /* Real    */ ae_vector* x,
     ae_state *_state);
void idwbuildmodifiedshepard(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t nx,
     ae_int_t d,
     ae_int_t nq,
     ae_int_t nw,
     idwinterpolant* z,
     ae_state *_state);
void idwbuildmodifiedshepardr(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t nx,
     double r,
     idwinterpolant* z,
     ae_state *_state);
void idwbuildnoisy(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t nx,
     ae_int_t d,
     ae_int_t nq,
     ae_int_t nw,
     idwinterpolant* z,
     ae_state *_state);
ae_bool _idwinterpolant_init(idwinterpolant* p, ae_state *_state, ae_bool make_automatic);
ae_bool _idwinterpolant_init_copy(idwinterpolant* dst, idwinterpolant* src, ae_state *_state, ae_bool make_automatic);
void _idwinterpolant_clear(idwinterpolant* p);
double barycentriccalc(barycentricinterpolant* b,
     double t,
     ae_state *_state);
void barycentricdiff1(barycentricinterpolant* b,
     double t,
     double* f,
     double* df,
     ae_state *_state);
void barycentricdiff2(barycentricinterpolant* b,
     double t,
     double* f,
     double* df,
     double* d2f,
     ae_state *_state);
void barycentriclintransx(barycentricinterpolant* b,
     double ca,
     double cb,
     ae_state *_state);
void barycentriclintransy(barycentricinterpolant* b,
     double ca,
     double cb,
     ae_state *_state);
void barycentricunpack(barycentricinterpolant* b,
     ae_int_t* n,
     /* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_state *_state);
void barycentricbuildxyw(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     barycentricinterpolant* b,
     ae_state *_state);
void barycentricbuildfloaterhormann(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t d,
     barycentricinterpolant* b,
     ae_state *_state);
void barycentriccopy(barycentricinterpolant* b,
     barycentricinterpolant* b2,
     ae_state *_state);
ae_bool _barycentricinterpolant_init(barycentricinterpolant* p, ae_state *_state, ae_bool make_automatic);
ae_bool _barycentricinterpolant_init_copy(barycentricinterpolant* dst, barycentricinterpolant* src, ae_state *_state, ae_bool make_automatic);
void _barycentricinterpolant_clear(barycentricinterpolant* p);
void polynomialbar2cheb(barycentricinterpolant* p,
     double a,
     double b,
     /* Real    */ ae_vector* t,
     ae_state *_state);
void polynomialcheb2bar(/* Real    */ ae_vector* t,
     ae_int_t n,
     double a,
     double b,
     barycentricinterpolant* p,
     ae_state *_state);
void polynomialbar2pow(barycentricinterpolant* p,
     double c,
     double s,
     /* Real    */ ae_vector* a,
     ae_state *_state);
void polynomialpow2bar(/* Real    */ ae_vector* a,
     ae_int_t n,
     double c,
     double s,
     barycentricinterpolant* p,
     ae_state *_state);
void polynomialbuild(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state);
void polynomialbuildeqdist(double a,
     double b,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state);
void polynomialbuildcheb1(double a,
     double b,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state);
void polynomialbuildcheb2(double a,
     double b,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     barycentricinterpolant* p,
     ae_state *_state);
double polynomialcalceqdist(double a,
     double b,
     /* Real    */ ae_vector* f,
     ae_int_t n,
     double t,
     ae_state *_state);
double polynomialcalccheb1(double a,
     double b,
     /* Real    */ ae_vector* f,
     ae_int_t n,
     double t,
     ae_state *_state);
double polynomialcalccheb2(double a,
     double b,
     /* Real    */ ae_vector* f,
     ae_int_t n,
     double t,
     ae_state *_state);
void spline1dbuildlinear(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     spline1dinterpolant* c,
     ae_state *_state);
void spline1dbuildcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     spline1dinterpolant* c,
     ae_state *_state);
void spline1dgriddiffcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* d,
     ae_state *_state);
void spline1dgriddiff2cubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundltype,
     double boundl,
     ae_int_t boundrtype,
     double boundr,
     /* Real    */ ae_vector* d1,
     /* Real    */ ae_vector* d2,
     ae_state *_state);
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
     ae_state *_state);
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
     ae_state *_state);
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
     ae_state *_state);
void spline1dbuildcatmullrom(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t boundtype,
     double tension,
     spline1dinterpolant* c,
     ae_state *_state);
void spline1dbuildhermite(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     spline1dinterpolant* c,
     ae_state *_state);
void spline1dbuildakima(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     spline1dinterpolant* c,
     ae_state *_state);
double spline1dcalc(spline1dinterpolant* c, double x, ae_state *_state);
void spline1ddiff(spline1dinterpolant* c,
     double x,
     double* s,
     double* ds,
     double* d2s,
     ae_state *_state);
void spline1dcopy(spline1dinterpolant* c,
     spline1dinterpolant* cc,
     ae_state *_state);
void spline1dunpack(spline1dinterpolant* c,
     ae_int_t* n,
     /* Real    */ ae_matrix* tbl,
     ae_state *_state);
void spline1dlintransx(spline1dinterpolant* c,
     double a,
     double b,
     ae_state *_state);
void spline1dlintransy(spline1dinterpolant* c,
     double a,
     double b,
     ae_state *_state);
double spline1dintegrate(spline1dinterpolant* c,
     double x,
     ae_state *_state);
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
     ae_state *_state);
void heapsortdpoints(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* d,
     ae_int_t n,
     ae_state *_state);
ae_bool _spline1dinterpolant_init(spline1dinterpolant* p, ae_state *_state, ae_bool make_automatic);
ae_bool _spline1dinterpolant_init_copy(spline1dinterpolant* dst, spline1dinterpolant* src, ae_state *_state, ae_bool make_automatic);
void _spline1dinterpolant_clear(spline1dinterpolant* p);
void polynomialfit(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     barycentricinterpolant* p,
     polynomialfitreport* rep,
     ae_state *_state);
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
     ae_state *_state);
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
     ae_state *_state);
void barycentricfitfloaterhormann(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     barycentricinterpolant* b,
     barycentricfitreport* rep,
     ae_state *_state);
void spline1dfitpenalized(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     double rho,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state);
void spline1dfitpenalizedw(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     ae_int_t n,
     ae_int_t m,
     double rho,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state);
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
     ae_state *_state);
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
     ae_state *_state);
void spline1dfitcubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state);
void spline1dfithermite(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     spline1dinterpolant* s,
     spline1dfitreport* rep,
     ae_state *_state);
void lsfitlinearw(/* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_matrix* fmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state);
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
     ae_state *_state);
void lsfitlinear(/* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* fmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state);
void lsfitlinearc(/* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* fmatrix,
     /* Real    */ ae_matrix* cmatrix,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state);
void lsfitcreatewf(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     double diffstep,
     lsfitstate* state,
     ae_state *_state);
void lsfitcreatef(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     double diffstep,
     lsfitstate* state,
     ae_state *_state);
void lsfitcreatewfg(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_bool cheapfg,
     lsfitstate* state,
     ae_state *_state);
void lsfitcreatefg(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     ae_bool cheapfg,
     lsfitstate* state,
     ae_state *_state);
void lsfitcreatewfgh(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* w,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     lsfitstate* state,
     ae_state *_state);
void lsfitcreatefgh(/* Real    */ ae_matrix* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_vector* c,
     ae_int_t n,
     ae_int_t m,
     ae_int_t k,
     lsfitstate* state,
     ae_state *_state);
void lsfitsetcond(lsfitstate* state,
     double epsf,
     double epsx,
     ae_int_t maxits,
     ae_state *_state);
void lsfitsetstpmax(lsfitstate* state, double stpmax, ae_state *_state);
void lsfitsetxrep(lsfitstate* state, ae_bool needxrep, ae_state *_state);
ae_bool lsfititeration(lsfitstate* state, ae_state *_state);
void lsfitresults(lsfitstate* state,
     ae_int_t* info,
     /* Real    */ ae_vector* c,
     lsfitreport* rep,
     ae_state *_state);
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
     ae_state *_state);
ae_bool _polynomialfitreport_init(polynomialfitreport* p, ae_state *_state, ae_bool make_automatic);
ae_bool _polynomialfitreport_init_copy(polynomialfitreport* dst, polynomialfitreport* src, ae_state *_state, ae_bool make_automatic);
void _polynomialfitreport_clear(polynomialfitreport* p);
ae_bool _barycentricfitreport_init(barycentricfitreport* p, ae_state *_state, ae_bool make_automatic);
ae_bool _barycentricfitreport_init_copy(barycentricfitreport* dst, barycentricfitreport* src, ae_state *_state, ae_bool make_automatic);
void _barycentricfitreport_clear(barycentricfitreport* p);
ae_bool _spline1dfitreport_init(spline1dfitreport* p, ae_state *_state, ae_bool make_automatic);
ae_bool _spline1dfitreport_init_copy(spline1dfitreport* dst, spline1dfitreport* src, ae_state *_state, ae_bool make_automatic);
void _spline1dfitreport_clear(spline1dfitreport* p);
ae_bool _lsfitreport_init(lsfitreport* p, ae_state *_state, ae_bool make_automatic);
ae_bool _lsfitreport_init_copy(lsfitreport* dst, lsfitreport* src, ae_state *_state, ae_bool make_automatic);
void _lsfitreport_clear(lsfitreport* p);
ae_bool _lsfitstate_init(lsfitstate* p, ae_state *_state, ae_bool make_automatic);
ae_bool _lsfitstate_init_copy(lsfitstate* dst, lsfitstate* src, ae_state *_state, ae_bool make_automatic);
void _lsfitstate_clear(lsfitstate* p);
void pspline2build(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline2interpolant* p,
     ae_state *_state);
void pspline3build(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline3interpolant* p,
     ae_state *_state);
void pspline2buildperiodic(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline2interpolant* p,
     ae_state *_state);
void pspline3buildperiodic(/* Real    */ ae_matrix* xy,
     ae_int_t n,
     ae_int_t st,
     ae_int_t pt,
     pspline3interpolant* p,
     ae_state *_state);
void pspline2parametervalues(pspline2interpolant* p,
     ae_int_t* n,
     /* Real    */ ae_vector* t,
     ae_state *_state);
void pspline3parametervalues(pspline3interpolant* p,
     ae_int_t* n,
     /* Real    */ ae_vector* t,
     ae_state *_state);
void pspline2calc(pspline2interpolant* p,
     double t,
     double* x,
     double* y,
     ae_state *_state);
void pspline3calc(pspline3interpolant* p,
     double t,
     double* x,
     double* y,
     double* z,
     ae_state *_state);
void pspline2tangent(pspline2interpolant* p,
     double t,
     double* x,
     double* y,
     ae_state *_state);
void pspline3tangent(pspline3interpolant* p,
     double t,
     double* x,
     double* y,
     double* z,
     ae_state *_state);
void pspline2diff(pspline2interpolant* p,
     double t,
     double* x,
     double* dx,
     double* y,
     double* dy,
     ae_state *_state);
void pspline3diff(pspline3interpolant* p,
     double t,
     double* x,
     double* dx,
     double* y,
     double* dy,
     double* z,
     double* dz,
     ae_state *_state);
void pspline2diff2(pspline2interpolant* p,
     double t,
     double* x,
     double* dx,
     double* d2x,
     double* y,
     double* dy,
     double* d2y,
     ae_state *_state);
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
     ae_state *_state);
double pspline2arclength(pspline2interpolant* p,
     double a,
     double b,
     ae_state *_state);
double pspline3arclength(pspline3interpolant* p,
     double a,
     double b,
     ae_state *_state);
ae_bool _pspline2interpolant_init(pspline2interpolant* p, ae_state *_state, ae_bool make_automatic);
ae_bool _pspline2interpolant_init_copy(pspline2interpolant* dst, pspline2interpolant* src, ae_state *_state, ae_bool make_automatic);
void _pspline2interpolant_clear(pspline2interpolant* p);
ae_bool _pspline3interpolant_init(pspline3interpolant* p, ae_state *_state, ae_bool make_automatic);
ae_bool _pspline3interpolant_init_copy(pspline3interpolant* dst, pspline3interpolant* src, ae_state *_state, ae_bool make_automatic);
void _pspline3interpolant_clear(pspline3interpolant* p);
void spline2dbuildbilinear(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* f,
     ae_int_t m,
     ae_int_t n,
     spline2dinterpolant* c,
     ae_state *_state);
void spline2dbuildbicubic(/* Real    */ ae_vector* x,
     /* Real    */ ae_vector* y,
     /* Real    */ ae_matrix* f,
     ae_int_t m,
     ae_int_t n,
     spline2dinterpolant* c,
     ae_state *_state);
double spline2dcalc(spline2dinterpolant* c,
     double x,
     double y,
     ae_state *_state);
void spline2ddiff(spline2dinterpolant* c,
     double x,
     double y,
     double* f,
     double* fx,
     double* fy,
     double* fxy,
     ae_state *_state);
void spline2dunpack(spline2dinterpolant* c,
     ae_int_t* m,
     ae_int_t* n,
     /* Real    */ ae_matrix* tbl,
     ae_state *_state);
void spline2dlintransxy(spline2dinterpolant* c,
     double ax,
     double bx,
     double ay,
     double by,
     ae_state *_state);
void spline2dlintransf(spline2dinterpolant* c,
     double a,
     double b,
     ae_state *_state);
void spline2dcopy(spline2dinterpolant* c,
     spline2dinterpolant* cc,
     ae_state *_state);
void spline2dresamplebicubic(/* Real    */ ae_matrix* a,
     ae_int_t oldheight,
     ae_int_t oldwidth,
     /* Real    */ ae_matrix* b,
     ae_int_t newheight,
     ae_int_t newwidth,
     ae_state *_state);
void spline2dresamplebilinear(/* Real    */ ae_matrix* a,
     ae_int_t oldheight,
     ae_int_t oldwidth,
     /* Real    */ ae_matrix* b,
     ae_int_t newheight,
     ae_int_t newwidth,
     ae_state *_state);
ae_bool _spline2dinterpolant_init(spline2dinterpolant* p, ae_state *_state, ae_bool make_automatic);
ae_bool _spline2dinterpolant_init_copy(spline2dinterpolant* dst, spline2dinterpolant* src, ae_state *_state, ae_bool make_automatic);
void _spline2dinterpolant_clear(spline2dinterpolant* p);

}
#endif

