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

// $Id: norm.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_NORM_H
#define __HERMES2D_NORM_H

#include "solution.h"
#include "refmap.h"

double calc_error(double (*fn)(MeshFunction*, MeshFunction*, RefMap*, RefMap*), MeshFunction* sln1, MeshFunction* sln2);
double calc_norm(double (*fn)(MeshFunction*, RefMap*), MeshFunction* sln);


double l2_error(MeshFunction* sln1, MeshFunction* sln2);
double l2_norm(MeshFunction* sln);
 
double h1_error(MeshFunction* sln1, MeshFunction* sln2);
double h1_norm(MeshFunction* sln);

#ifdef COMPLEX

double hcurl_error(MeshFunction* sln1, MeshFunction* sln2);
double hcurl_norm(MeshFunction* sln);

#endif
 
#endif
