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

// $Id: traverse.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_TRAVERSE_H
#define __HERMES2D_TRAVERSE_H

class Mesh;
class Transformable;
struct State;
struct Rect;


struct UniData
{
  Element* e;
  uint64 idx;
};
  

/// Traverse is a multi-mesh traversal utility class. Given N meshes sharing the
/// same base mesh it walks through all (pseudo-)elements of the union of all 
/// the N meshes. 
///
class Traverse
{
public:

  void begin(int n, Mesh** meshes, Transformable** fn = NULL);
  void finish();

  Element** get_next_state(bool* bnd, EdgePos* ep);
  Element*  get_base() const { return base; }

  UniData** construct_union_mesh(Mesh* unimesh);

private:
  
  int num;
  Mesh** meshes;
  Transformable** fn;
  
  State* stack;
  int top, size;
  
  int id;
  bool tri;
  Element* base;
  int4* sons;
  uint64* subs;
  
  UniData** unidata;
  int udsize;

  State* push_state();
  void set_boundary_info(State* s, bool* bnd, EdgePos* ep);
  void union_recurrent(Rect* cr, Element** e, Rect* er, uint64* idx, Element* uni);
  uint64 init_idx(Rect* cr, Rect* er);
  
  Mesh* unimesh;
  
};



#endif
