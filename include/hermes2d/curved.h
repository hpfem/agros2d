// This file is part of Hermes2D.
//
// Copyright 2006-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
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

// $Id: curved.h 1112 2008-11-06 15:50:21Z jakub $

#ifndef __HERMES2D_CURVED_H
#define __HERMES2D_CURVED_H

#include "common.h"

struct Element;


/// \brief Represents one NURBS curve.
///
/// The structure Nurbs defines one curved edge, or, more precisely, 
/// the control points and other data for one NURBS curve.
///
/// A good introduction to NURBS curves can be found <a href=
/// "http://devworld.apple.com/dev/techsupport/develop/issue25/schneider.html">
/// here</a>.
///
struct Nurbs
{
  Nurbs() { ref = 0; twin = false; };
  void unref();
  
  int degree;  ///< curve degree (2=quadratic, etc.)
  int np;      ///< number of control points
  double3* pt; ///< control points and their weights
  int nk;      ///< knot vector length
  double* kv;  ///< knot vector
  int ref;     ///< reference counter (the structure is deleted when this reaches zero)
  bool twin;   ///< true on internal curved edges for the second (artificial) Nurbs
  bool arc;     ///< true if this is in fact a circular arc 
  double angle; ///< arc angle
};


/// CurvMap is a structure storing complete information on the curved edges of 
/// an element. There are two variants of this structure. The first if for 
/// top-level (master mesh) elements. 
///
struct CurvMap
{
  CurvMap() { coefs = NULL; };
  CurvMap(CurvMap* cm);
  ~CurvMap();

  // this structure defines a curved mapping of an element; it has two
  // modes, depending on the value of 'toplevel'
  bool toplevel;
  union
  {
    // if toplevel=true, this structure belongs to a base mesh element
    // and the array 'nurbs' points to (up to four) NURBS curved edges
    Nurbs* nurbs[4];
    struct
    {
      // if toplevel=false, this structure belongs to a refined element
      // and 'parent' points to the base mesh element CurvMap structure;
      Element* parent;
      uint64 part;
    };
  };

  // current polynomial degree of the refmap approximation
  int order;

  // finally here are the coefficients of the higher-order basis functions
  // that constitute the projected reference mapping:
  int nc; // number of coefficients (todo: mozna spis polyn. rad zobrazeni)
  double2* coefs; // array of the coefficients

  // tohle se zavola pro kazdy krivocary element kdyz vznikne anebo
  // bude potreba prepocitat koeficienty pro jiny rad (order);
  // 'e' je pointer na element ke kteremu tato CurvMap patri;
  // prvni co udela, ze smaze stare 'coefs', pokud nejsou NULL,
  // pote zprojektuje nove koeficienty
  void update_refmap_coefs(Element* e);
  
  void get_mid_edge_points(Element* e, double2* pt, int n);
  
};


void nurbs_edge(Element* e, Nurbs* nurbs, int edge, double t, double& x, double& y);


#endif
