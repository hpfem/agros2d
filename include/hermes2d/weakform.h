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

// $Id: weakform.h 1096 2008-10-27 14:56:28Z jakub $

#ifndef __HERMES2D_WEAKFORM_H
#define __HERMES2D_WEAKFORM_H

#include "function.h"

class RefMap;
class LinSystem;
class NonlinSystem;
class Space;
class MeshFunction;
struct EdgePos;
  

// Bilinear form symmetry flag, see WeakForm::add_biform
enum SymFlag
{
  ANTISYM = -1,
  UNSYM = 0,
  SYM = 1
};


/// \brief Represents the weak formulation of a problem.
///
/// The WeakForm class represents the weak formulation of a system of linear PDEs.
/// The number of equations ("neq") in the system is fixed and is passed to the constructor.
/// The weak formulation of the system A(U,V) = L(V) has a block structure. A(U,V) is
/// a (neq x neq) matrix of bilinear forms a_mn(u,v) and L(V) is a neq-component vector
/// of linear forms l(v). U and V are the vectors of basis and test functions.
///
/// 
///
class WeakForm
{
public:
  
  WeakForm(int neq);  

  int def_area(int n, ...);

  typedef scalar (*biform_vol_t) (RealFunction*, RealFunction*, RefMap*, RefMap*);
  typedef scalar (*biform_surf_t)(RealFunction*, RealFunction*, RefMap*, RefMap*, EdgePos*);
  typedef scalar (*liform_vol_t) (RealFunction*, RefMap*);
  typedef scalar (*liform_surf_t)(RealFunction*, RefMap*, EdgePos*);

  void add_biform(int i, int j, biform_vol_t fn, SymFlag sym = UNSYM, int area = ANY, int nx = 0, ...);
  void add_biform_surf(int i, int j, biform_surf_t fn, int area = ANY, int nx = 0, ...);

  void add_liform(int i, liform_vol_t fn, int area = ANY, int nx = 0, ...);
  void add_liform_surf(int i, liform_surf_t fn, int area = ANY, int nx = 0, ...);

  void set_ext_fns(void* fn, int nx, ...);


protected:
  
  int neq;

  struct Area  {  /*std::string name;*/  std::vector<int> markers;  };

  std::vector<Area> areas;

  struct BiFormVol   {  int i, j, sym, area;  biform_vol_t  fn;  std::vector<MeshFunction*> ext;  };
  struct BiFormSurf  {  int i, j, area;       biform_surf_t fn;  std::vector<MeshFunction*> ext;  };
  struct LiFormVol   {  int i, area;          liform_vol_t  fn;  std::vector<MeshFunction*> ext;  };
  struct LiFormSurf  {  int i, area;          liform_surf_t fn;  std::vector<MeshFunction*> ext;  };
  
  std::vector<BiFormVol>  bfvol;
  std::vector<BiFormSurf> bfsurf;
  std::vector<LiFormVol>  lfvol;
  std::vector<LiFormSurf> lfsurf;


  struct Stage
  {
    std::vector<int> idx;
    std::vector<Mesh*> meshes;
    std::vector<Transformable*> fns;
    std::vector<MeshFunction*> ext;
    
    std::vector<BiFormVol*>  bfvol;
    std::vector<BiFormSurf*> bfsurf;
    std::vector<LiFormVol*>  lfvol;
    std::vector<LiFormSurf*> lfsurf;
    
    std::set<int> idx_set;
    std::set<unsigned> seq_set;
    std::set<MeshFunction*> ext_set;
  };

  void get_stages(Space** spaces, std::vector<Stage>& stages, bool rhsonly);
  bool** get_blocks();
  
  bool is_in_area(int marker, int area) const
    { return area >= 0 ? area == marker : is_in_area_2(marker, area); }
    
  bool is_sym() const { return false; /* not impl. yet */ }

  friend class LinSystem;
  friend class NonlinSystem;
  friend class RefSystem;
  friend class RefNonlinSystem;


private: 
 
  Stage* find_stage(std::vector<Stage>& stages, int ii, int jj,
                    Mesh* m1, Mesh* m2, std::vector<MeshFunction*>& ext);

  bool is_in_area_2(int marker, int area) const;

};



  //void def_ext_fn(const char* name);
  //void def_exa_fn(const char* name);
  //void def_const(const char* name, scalar value);


#endif
