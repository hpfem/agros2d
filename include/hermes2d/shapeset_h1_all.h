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

// $Id: shapeset_h1_all.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_SHAPESET_H1_ALL
#define __HERMES2D_SHAPESET_H1_ALL

// This file is a common header for all H1 shapesets.

#include "shapeset.h"


/// H1 shapeset with orthogonalized bubble functions for improved conditioning.
class H1ShapesetOrtho : public Shapeset
{
  public: H1ShapesetOrtho();
  virtual int get_id() const { return 0; }
};


/// Sven Beuchler's shape functions based on integrated Jacobi polynomials.
class H1ShapesetBeuchler : public Shapeset
{
  public: H1ShapesetBeuchler();
  virtual int get_id() const { return 1; }
};


// Experimental.
class H1ShapesetEigen : public Shapeset
{
  public: H1ShapesetEigen();
  virtual int get_id() const { return 2; }
};


/// This is the default shapeset typedef
typedef H1ShapesetBeuchler H1Shapeset;


#endif
