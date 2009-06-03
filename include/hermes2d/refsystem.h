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

#ifndef __HERMES2D_REFSYSTEM_H
#define __HERMES2D_REFSYSTEM_H

#include "linsystem.h"

class Mesh;


///
///
///
class RefSystem : public LinSystem
{
public:
  
  RefSystem(LinSystem* base, int order_increase = 1, int refinement = 1);
  virtual ~RefSystem();

  /// Do not call in this class
  void set_spaces(int n, ...);
  /// Do not call in this class
  void set_pss(int n, ...);

  /// Creates reference (fine) meshes and spaces and assembles the 
  /// reference system.
  void assemble(bool rhsonly = false);

  /// Frees reference spaces and meshes. Called 
  /// automatically on desctruction.
  void free_ref_data();


protected:

  LinSystem* base;
  int order_inc;
  int refinement;

  Mesh**  ref_meshes;
  Space** ref_spaces;

};



#endif
