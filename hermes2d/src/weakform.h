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


#ifndef __H2D_WEAKFORM_H
#define __H2D_WEAKFORM_H

#include "function.h"

class RefMap;
class LinSystem;
class NonlinSystem;
class Space;
class MeshFunction;
struct EdgePos;
class Ord;

struct Element;
class Shapeset;
template<typename T> class Func;
template<typename T> class Geom;
template<typename T> class ExtData;


// Bilinear form symmetry flag, see WeakForm::add_biform
enum SymFlag
{
  H2D_ANTISYM = -1,
  H2D_UNSYM = 0,
  H2D_SYM = 1
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
class H2D_API WeakForm
{
public:

  WeakForm(int neq, bool mat_free = false);

  int def_area(int n, ...);

  // linear case
  typedef scalar (*biform_val_t) (int n, double *wt, Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<scalar> *);
  typedef Ord (*biform_ord_t) (int n, double *wt, Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *);
  typedef scalar (*liform_val_t)(int n, double *wt, Func<double> *v, Geom<double> *e, ExtData<scalar> *v_ext_fnc);
  typedef Ord (*liform_ord_t)(int n, double *wt, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *v_ext_fnc);
  typedef scalar (*liform_val_extended_t)(int n, double *wt, Func<double> *v, Geom<double> *e, ExtData<scalar> *v_ext_fnc, Element* element, Shapeset* shape_set, int shape_inx);
  typedef Ord (*liform_ord_extended_t)(int n, double *wt, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *v_ext_fnc, Element* element, Shapeset* shape_set, int shape_inx);

  // nonlinear case
  typedef scalar (*jacform_val_t)(int n, double *wt, Func<scalar> *u[], Func<double> *vi, Func<double> *vj, Geom<double> *e, ExtData<scalar> *);
  typedef Ord (*jacform_ord_t)(int n, double *wt, Func<Ord> *u[], Func<Ord> *vi, Func<Ord> *vj, Geom<Ord> *e, ExtData<Ord> *);
  typedef scalar (*resform_val_t)(int n, double *wt, Func<scalar> *u[], Func<double> *vi, Geom<double> *e, ExtData<scalar> *);
  typedef Ord (*resform_ord_t)(int n, double *wt, Func<Ord> *u[], Func<Ord> *vi, Geom<Ord> *e, ExtData<Ord> *);

  // linear case
  void add_biform(int i, int j, biform_val_t fn, biform_ord_t ord, SymFlag sym = H2D_UNSYM, int area = H2D_ANY, int nx = 0, ...);
  void add_biform_surf(int i, int j, biform_val_t fn, biform_ord_t ord, int area = H2D_ANY, int nx = 0, ...);
  void add_liform(int i, liform_val_t fn, liform_ord_t ord, int area = H2D_ANY, int nx = 0, ...);
  void add_liform_surf(int i, liform_val_t fn, liform_ord_t ord, int area = H2D_ANY, int nx = 0, ...);
  void add_liform(int i, liform_val_extended_t fn, liform_ord_extended_t ord, int area = H2D_ANY, int nx = 0, ...);

  // nonlinear case
  void add_jacform(int i, int j, jacform_val_t fn, jacform_ord_t ord, SymFlag sym = H2D_UNSYM, int area = H2D_ANY, int nx = 0, ...);
  void add_jacform_surf(int i, int j, jacform_val_t fn, jacform_ord_t ord, int area = H2D_ANY, int nx = 0, ...);
  void add_resform(int i, resform_val_t fn, resform_ord_t ord, int area = H2D_ANY, int nx = 0, ...);
  void add_resform_surf(int i, resform_val_t fn, resform_ord_t ord, int area = H2D_ANY, int nx = 0, ...);

  void set_ext_fns(void* fn, int nx, ...);

  /// Internal. Used by LinSystem to detect changes in the weakform.
  int get_seq() const { return seq; }

  bool is_matrix_free() { return is_matfree; }


protected:

  int neq;
  int seq;
  bool is_matfree;

  struct Area  {  /*std::string name;*/  std::vector<int> markers;  };

  H2D_API_USED_STL_VECTOR(Area);
  std::vector<Area> areas;
  H2D_API_USED_STL_VECTOR(MeshFunction*);

  // linear case
  struct BiFormVol   {  int i, j, sym, area;  biform_val_t  fn;  biform_ord_t  ord;  std::vector<MeshFunction*> ext;  };
  struct BiFormSurf  {  int i, j, area;       biform_val_t  fn;  biform_ord_t  ord;  std::vector<MeshFunction*> ext;  };
  struct LiFormVol   {
    int i, area;
    std::vector<MeshFunction*> ext;

  private:
    liform_val_t fn;
    liform_ord_t ord;
    liform_val_extended_t fn_extended;
    liform_ord_extended_t ord_extended;

  public:
    scalar evaluate_fn(int point_cnt, double *weights, Func<double> *values_v, Geom<double> *geometry, ExtData<scalar> *values_ext_fnc, Element* element, Shapeset* shape_set, int shape_inx); ///< Evaluate value of the user defined function.
    Ord evaluate_ord(int point_cnt, double *weights, Func<Ord> *values_v, Geom<Ord> *geometry, ExtData<Ord> *values_ext_fnc, Element* element, Shapeset* shape_set, int shape_inx); ///< Evaluate order of the user defined function.

    LiFormVol() {};
    LiFormVol(int i, int area, liform_val_t fn, liform_ord_t ord) : i(i), area(area), fn(fn), ord(ord), fn_extended(NULL), ord_extended(NULL) {};
    LiFormVol(int i, int area, liform_val_extended_t fn_extended, liform_ord_extended_t ord_extended) : i(i), area(area), fn(NULL), ord(NULL), fn_extended(fn_extended), ord_extended(ord_extended) {};
  };
  struct LiFormSurf  {  int i, area;          liform_val_t  fn;  liform_ord_t  ord;  std::vector<MeshFunction*> ext;  };

  // nonlinear case
  struct JacFormVol  {  int i, j, sym, area;  jacform_val_t fn;  jacform_ord_t ord;  std::vector<MeshFunction *> ext; };
  struct JacFormSurf {  int i, j, area;       jacform_val_t fn;  jacform_ord_t ord;  std::vector<MeshFunction *> ext; };
  struct ResFormVol  {  int i, area;          resform_val_t fn;  resform_ord_t ord;  std::vector<MeshFunction *> ext; };
  struct ResFormSurf {  int i, area;          resform_val_t fn;  resform_ord_t ord;  std::vector<MeshFunction *> ext; };

  // linear case
  H2D_API_USED_STL_VECTOR(BiFormVol);
  std::vector<BiFormVol>  bfvol;
  H2D_API_USED_STL_VECTOR(BiFormSurf);
  std::vector<BiFormSurf> bfsurf;
  H2D_API_USED_STL_VECTOR(LiFormVol);
  std::vector<LiFormVol>  lfvol;
  H2D_API_USED_STL_VECTOR(LiFormSurf);
  std::vector<LiFormSurf> lfsurf;

  // nonlinear case
  std::vector<JacFormVol>  jfvol;
  std::vector<JacFormSurf> jfsurf;
  std::vector<ResFormVol>  rfvol;
  std::vector<ResFormSurf> rfsurf;

  struct Stage
  {
    std::vector<int> idx;
    std::vector<Mesh*> meshes;
    std::vector<Transformable*> fns;
    std::vector<MeshFunction*> ext;

    // linear case
    std::vector<BiFormVol*>  bfvol;
    std::vector<BiFormSurf*> bfsurf;
    std::vector<LiFormVol*>  lfvol;
    std::vector<LiFormSurf*> lfsurf;

    // nonlinear case
    std::vector<JacFormVol *>  jfvol;
    std::vector<JacFormSurf *> jfsurf;
    std::vector<ResFormVol *>  rfvol;
    std::vector<ResFormSurf *> rfsurf;

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
  friend class FeProblem;
  friend class Precond;


private:

  Stage* find_stage(std::vector<Stage>& stages, int ii, int jj,
                    Mesh* m1, Mesh* m2, std::vector<MeshFunction*>& ext);

  bool is_in_area_2(int marker, int area) const;

  // FIXME: pretty dumb to test this in such a way
  bool is_linear() {
    return bfvol.size() > 0 || bfsurf.size() > 0 || lfvol.size() > 0 || lfsurf.size() > 0;
  }

};



  //void def_ext_fn(const char* name);
  //void def_exa_fn(const char* name);
  //void def_const(const char* name, scalar value);


#endif
