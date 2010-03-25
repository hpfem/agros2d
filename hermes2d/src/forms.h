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


#ifndef __HERMES2D_FORMS_H
#define __HERMES2D_FORMS_H

#include "common.h"
#include "quad.h"
#include "function.h"
#include "solution.h"
#include "refmap.h"

#define callback(a)	a<double, scalar>, a<Ord, Ord>

// Base type for orders of functions
//
// We defined a special arithmetics with this type to be able to analyze forms
// and determine the necessary integration order.  This works for forms, but it also
// works for user-defined functions.
class Ord
{
public:

	Ord() { order = 0; }
	Ord(int o) { order = o; }

	int get_order() const { return order; }
  int get_max_order() const {return 30;}

	Ord operator+(const Ord &o) { return Ord(std::max(this->order, o.order)); }
	Ord operator-(const Ord &o) { return Ord(std::max(this->order, o.order)); }
	Ord operator-(double d) { return *this; }
	Ord operator*(const Ord &o) { return Ord(this->order + o.order); }
  Ord operator/(const Ord &o) { return Ord(this->get_max_order()); }

	Ord operator/(double d) { return *this; }

	Ord operator+=(const Ord &o) { this->order = std::max(this->order, o.order); return *this; }
	bool operator<(double d) { return true; }

protected:
	int order;

};

inline Ord operator/(const scalar &a, const Ord &b) { return Ord(b.get_max_order()); }
inline Ord operator*(const scalar &a, const Ord &b) { return b; }
inline Ord operator+(const scalar &a, const Ord &b) { return b; }
inline Ord operator-(const scalar &a, const Ord &b) { return b; }
inline Ord operator-(const Ord &a) { return a; }

inline Ord pow(const Ord &a, const double &b) { return Ord((int) ceil(fabs(b)) * a.get_order()); }
inline Ord sqrt(const Ord &a) { return a; }
inline Ord sqr(const Ord &a) { return Ord(2 * a.get_order()); }
inline Ord conj(const Ord &a) { return a; }
inline Ord abs(const Ord &a) { return a; }

inline Ord atan2(const Ord &a, const Ord &b) { return Ord(a.get_max_order()); }
inline Ord atan(const Ord &a) { return Ord(a.get_max_order()); }
inline Ord sin(const Ord &a) { return Ord(a.get_max_order()); }
inline Ord cos(const Ord &a) { return Ord(a.get_max_order()); }
inline Ord log(const Ord &a) { return Ord(a.get_max_order()); }
inline Ord exp(const Ord &a) { return Ord(3 * a.get_order()); }

// Function
template<typename T>
class Func
{
public:
	int nc;							// number of components
	T *val;							// function values. If orders differ for a diffrent direction, this returns max(h_order, v_order).
	T *dx, *dy; 				// derivatives

	T *val0, *val1;				// components of function values
	T *dx0, *dx1;				// components of derivatives
	T *dy0, *dy1;

	T *curl;         		// components of curl

	Func()
  {
		val = val0 = val1 = NULL;
		dx = dx0 = dx1 = NULL;
		dy = dy0 = dy1 = NULL;
		curl = NULL;
	}

  void free_ord()  {  delete val;  }
  void free_fn()
  {
    delete [] val;
    delete [] dx;
    delete [] dy;

    delete [] val0; delete [] val1;
    delete [] dx0;  delete [] dx1;
    delete [] dy0;  delete [] dy1;
    delete [] curl;
  }
};


/// Geometry (coordinates, normals, tangents) of either an element or an edge
template<typename T>
class Geom
{
public:
  int marker;      // marker
	T *x, *y;				 // coordinates [in physical domain]
	T *nx, *ny;			 // normals [in physical domain]
	T *tx, *ty;			 // tangents [in physical domain]

	Geom()
  {
    marker = 0;
		x = y = NULL;
		nx = ny = NULL;
		tx = ty = NULL;
	}

  void free()
  {
    delete [] tx;    delete [] ty;
    delete [] nx;    delete [] ny;
  }
};

/// Init element geometry for calculating the integration order
Geom<Ord>* init_geom_ord();
/// Init element geometry for volumetric integrals
Geom<double>* init_geom_vol(RefMap *rm, const int order);
/// Init element geometry for surface integrals
Geom<double>* init_geom_surf(RefMap *rm, EdgePos* ep, const int order);


/// Init the function for calculation the integration order
Func<Ord>* init_fn_ord(const int order);
/// Init the shape function for the evaluation of the volumetric/surface integral (transformation of values)
Func<double>* init_fn(PrecalcShapeset *fu, RefMap *rm, const int order);
/// Init the mesh-function for the evaluation of the volumetric/surface integral
Func<scalar>* init_fn(MeshFunction *fu, RefMap *rm, const int order);


/// User defined data that can go to the bilinear and linear forms.
/// It also holds arbitraty number of functions, that user can use.
/// Typically, these functions are solutions from the previous time/iteration levels.
template<typename T>
class ExtData {
public:
	int nf;			  			// number of functions in 'fn' array
	Func<T>** fn;				// array of pointers to functions

	ExtData() {
		nf = 0;
		fn = NULL;
	}

  void free()
  {
    for (int i = 0; i < nf; i++)
    {
      fn[i]->free_fn();
      delete fn[i];
    }
    delete [] fn;
  }

  void free_ord()
  {
    for (int i = 0; i < nf; i++)
    {
      fn[i]->free_ord();
      delete fn[i];
    }
    delete [] fn;
  }

};

#endif
