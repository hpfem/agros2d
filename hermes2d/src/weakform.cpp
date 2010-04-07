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


#include "common.h"
#include "weakform.h"
#include "matrix.h"
#include "solution.h"
#include "forms.h"

//// interface /////////////////////////////////////////////////////////////////////////////////////

WeakForm::WeakForm(int neq, bool mat_free)
{
  this->neq = neq;
  seq = 0;
  this->is_matfree = mat_free;
}


#define init_ext \
  va_list ap; va_start(ap, nx); \
  for (int i = 0; i < nx; i++) \
    form.ext.push_back(va_arg(ap, MeshFunction*)); \
  va_end(ap)

scalar WeakForm::LiFormVol::evaluate_fn(int point_cnt, double *weights, Func<double> *values_v, Geom<double> *geometry, ExtData<scalar> *values_ext_fnc, Element* element, Shapeset* shape_set, int shape_inx)
{
  assert_msg(fn != NULL || fn_extended != NULL, "E both version of functions of LinForm are NULL");
  if (fn != NULL)
    return fn(point_cnt, weights, values_v, geometry, values_ext_fnc);
  else
    return fn_extended(point_cnt, weights, values_v, geometry, values_ext_fnc, element, shape_set, shape_inx);
}

Ord WeakForm::LiFormVol::evaluate_ord(int point_cnt, double *weights, Func<Ord> *values_v, Geom<Ord> *geometry, ExtData<Ord> *values_ext_fnc, Element* element, Shapeset* shape_set, int shape_inx)
{
  assert_msg(ord != NULL || ord_extended != NULL, "E both version of order functions of LinForm are NULL");
  if (ord != NULL)
    return ord(point_cnt, weights, values_v, geometry, values_ext_fnc);
  else
    return ord_extended(point_cnt, weights, values_v, geometry, values_ext_fnc, element, shape_set, shape_inx);
}


void WeakForm::add_biform(int i, int j, biform_val_t fn, biform_ord_t ord, SymFlag sym, int area, int nx, ...)
{
  if (i < 0 || i >= neq || j < 0 || j >= neq)
    error("Invalid equation number.");
  if (sym < -1 || sym > 1)
    error("\"sym\" must be -1, 0 or 1.");
  if (sym < 0 && i == j)
    error("Only off-diagonal forms can be antisymmetric.");
  if (area != ANY && area < 0 && -area > (int)areas.size())
    error("Invalid area number.");
  if (bfvol.size() > 100)
    warn("Large number of forms (> 100). Is this the intent?");

  BiFormVol form = { i, j, sym, area, fn, ord };
  init_ext;
  bfvol.push_back(form);
  seq++;
}

void WeakForm::add_biform_surf(int i, int j, biform_val_t fn, biform_ord_t ord, int area, int nx, ...)
{
  if (i < 0 || i >= neq || j < 0 || j >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > (int)areas.size())
    error("Invalid area number.");

  BiFormSurf form = { i, j, area, fn, ord };
  init_ext;
  bfsurf.push_back(form);
  seq++;
}

void WeakForm::add_liform(int i, liform_val_t fn, liform_ord_t ord, int area, int nx, ...)
{
  if (i < 0 || i >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > (int)areas.size())
    error("Invalid area number.");

  LiFormVol form(i, area, fn, ord);
  init_ext;
  lfvol.push_back(form);
  seq++;
}

void WeakForm::add_liform(int i, liform_val_extended_t fn_ext, liform_ord_extended_t ord_ext, int area, int nx, ...)
{
  if (i < 0 || i >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > (int)areas.size())
    error("Invalid area number.");

  LiFormVol form(i, area, fn_ext, ord_ext);
  init_ext;
  lfvol.push_back(form);
  seq++;
}

void WeakForm::add_liform_surf(int i, liform_val_t fn, liform_ord_t ord, int area, int nx, ...)
{
  if (i < 0 || i >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > (int)areas.size())
    error("Invalid area number.");

  LiFormSurf form = { i, area, fn, ord };
  init_ext;
  lfsurf.push_back(form);
  seq++;
}

void WeakForm::add_jacform(int i, int j, jacform_val_t fn, jacform_ord_t ord, SymFlag sym, int area, int nx, ...)
{
  if (i < 0 || i >= neq || j < 0 || j >= neq)
    error("Invalid equation number.");
  if (sym < -1 || sym > 1)
    error("\"sym\" must be -1, 0 or 1.");
  if (sym < 0 && i == j)
    error("Only off-diagonal forms can be antisymmetric.");
  if (area != ANY && area < 0 && -area > areas.size())
    error("Invalid area number.");
  if (jfvol.size() > 100)
    warn("Large number of forms (> 100). Is this the intent?");

  JacFormVol form = { i, j, sym, area, fn, ord };
  init_ext;
  jfvol.push_back(form);
  seq++;
}

void WeakForm::add_jacform_surf(int i, int j, jacform_val_t fn, jacform_ord_t ord, int area, int nx, ...)
{
  if (i < 0 || i >= neq || j < 0 || j >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > areas.size())
    error("Invalid area number.");

  JacFormSurf form = { i, j, area, fn, ord };
  init_ext;
  jfsurf.push_back(form);
  seq++;
}

void WeakForm::add_resform(int i, resform_val_t fn, resform_ord_t ord, int area, int nx, ...)
{
  if (i < 0 || i >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > areas.size())
    error("Invalid area number.");

  ResFormVol form = { i, area, fn, ord };
  init_ext;
  rfvol.push_back(form);
  seq++;
}

void WeakForm::add_resform_surf(int i, resform_val_t fn, resform_ord_t ord, int area, int nx, ...)
{
  if (i < 0 || i >= neq)
    error("Invalid equation number.");
  if (area != ANY && area < 0 && -area > areas.size())
    error("Invalid area number.");

  ResFormSurf form = { i, area, fn, ord };
  init_ext;
  rfsurf.push_back(form);
  seq++;
}

void WeakForm::set_ext_fns(void* fn, int nx, ...)
{
  error("not implemented yet.");
}


//// stages ////////////////////////////////////////////////////////////////////////////////////////

/// Constructs a list of assembling stages. Each stage contains a list of forms
/// that share the same meshes. Each stage is then assembled separately. This
/// improves the performance of multi-mesh assembling.
///
void WeakForm::get_stages(Space** spaces, std::vector<WeakForm::Stage>& stages, bool rhsonly)
{
  unsigned i;
  stages.clear();

  if (!rhsonly)
  {
    if (is_linear())
    {
      // process volume biforms
      for (i = 0; i < bfvol.size(); i++)
      {
        int ii = bfvol[i].i, jj = bfvol[i].j;
        Mesh* m1 = spaces[ii]->get_mesh();
        Mesh* m2 = spaces[jj]->get_mesh();
        Stage* s = find_stage(stages, ii, jj, m1, m2, bfvol[i].ext);
        s->bfvol.push_back(&bfvol[i]);
      }

      // process surface biforms
      for (i = 0; i < bfsurf.size(); i++)
      {
        int ii = bfsurf[i].i, jj = bfsurf[i].j;
        Mesh* m1 = spaces[ii]->get_mesh();
        Mesh* m2 = spaces[jj]->get_mesh();
        Stage* s = find_stage(stages, ii, jj, m1, m2, bfsurf[i].ext);
        s->bfsurf.push_back(&bfsurf[i]);
      }
    }
    else
    {
      // process volume jacforms
      for (i = 0; i < jfvol.size(); i++)
      {
        int ii = jfvol[i].i, jj = jfvol[i].j;
        Mesh* m1 = spaces[ii]->get_mesh();
        Mesh* m2 = spaces[jj]->get_mesh();
        Stage* s = find_stage(stages, ii, jj, m1, m2, jfvol[i].ext);
        s->jfvol.push_back(&jfvol[i]);
      }

      // process surface jacforms
      for (i = 0; i < jfsurf.size(); i++)
      {
        int ii = jfsurf[i].i, jj = jfsurf[i].j;
        Mesh* m1 = spaces[ii]->get_mesh();
        Mesh* m2 = spaces[jj]->get_mesh();
        Stage* s = find_stage(stages, ii, jj, m1, m2, jfsurf[i].ext);
        s->jfsurf.push_back(&jfsurf[i]);
      }
    }
  }

  if (is_linear())
  {
    // process volume liforms
    for (i = 0; i < lfvol.size(); i++) {
      int ii = lfvol[i].i;
      Mesh *m = spaces[ii]->get_mesh();
      Stage *s = find_stage(stages, ii, ii, m, m, lfvol[i].ext);
      s->lfvol.push_back(&lfvol[i]);
    }

    // process surface liforms
    for (i = 0; i < lfsurf.size(); i++) {
      int ii = lfsurf[i].i;
      Mesh *m = spaces[ii]->get_mesh();
      Stage *s = find_stage(stages, ii, ii, m, m, lfsurf[i].ext);
      s->lfsurf.push_back(&lfsurf[i]);
    }
  }
  else
  {
    // process volume res forms
    for (unsigned i = 0; i < rfvol.size(); i++) {
      int ii = rfvol[i].i;
      Mesh *m = spaces[ii]->get_mesh();
      Stage *s = find_stage(stages, ii, ii, m, m, rfvol[i].ext);
      s->rfvol.push_back(&rfvol[i]);
    }

    // process surface res forms
    for (unsigned i = 0; i < rfsurf.size(); i++) {
      int ii = rfsurf[i].i;
      Mesh *m = spaces[ii]->get_mesh();
      Stage *s = find_stage(stages, ii, ii, m, m, rfsurf[i].ext);
      s->rfsurf.push_back(&rfsurf[i]);
    }
  }

  // helper macro for iterating in a set
  #define set_for_each(myset, type) \
    for (std::set<type>::iterator it = (myset).begin(); it != (myset).end(); it++)

  // initialize the arrays meshes and fns needed by Traverse for each stage
  for (i = 0; i < stages.size(); i++)
  {
    Stage* s = &stages[i];
    set_for_each(s->idx_set, int)
    {
      s->idx.push_back(*it);
      s->meshes.push_back(spaces[*it]->get_mesh());
      s->fns.push_back(NULL);
    }
    set_for_each(s->ext_set, MeshFunction*)
    {
      s->ext.push_back(*it);
      s->meshes.push_back((*it)->get_mesh());
      s->fns.push_back(*it);
    }
    s->idx_set.clear();
    s->seq_set.clear();
    s->ext_set.clear();
  }
}


/// Finds an assembling stage with the same set of meshes as [m1, m2, ext]. If no such
/// stage can be found, a new one is created and returned.
///
WeakForm::Stage* WeakForm::find_stage(std::vector<WeakForm::Stage>& stages, int ii, int jj,
                                      Mesh* m1, Mesh* m2, std::vector<MeshFunction*>& ext)
{
  // first create a list of meshes the form uses
  std::set<unsigned> seq;
  seq.insert(m1->get_seq());
  seq.insert(m2->get_seq());
  for (unsigned i = 0; i < ext.size(); i++)
    seq.insert(ext[i]->get_mesh()->get_seq());

  // find a suitable existing stage for the form
  Stage* s = NULL;
  for (unsigned i = 0; i < stages.size(); i++)
    if (seq.size() == stages[i].seq_set.size() &&
        equal(seq.begin(), seq.end(), stages[i].seq_set.begin()))
      { s = &stages[i]; break; }

  // create a new stage if not found
  if (s == NULL)
  {
    Stage newstage;
    stages.push_back(newstage);
    s = &stages.back();
    s->seq_set = seq;
  }

  // update and return the stage
  for (unsigned int i = 0; i < ext.size(); i++)
    s->ext_set.insert(ext[i]);
  s->idx_set.insert(ii);
  s->idx_set.insert(jj);
  return s;
}


/// Returns a (neq x neq) array containing true in each element, if the corresponding
/// block of weak forms is used, and false otherwise.
///
bool** WeakForm::get_blocks()
{
  bool** blocks = new_matrix<bool>(neq, neq);
  for (int i = 0; i < neq; i++)
    for (int j = 0; j < neq; j++)
      blocks[i][j] = false;

  if (is_linear())
  {
    for (unsigned i = 0; i < bfvol.size(); i++) {
      blocks[bfvol[i].i][bfvol[i].j] = true;
      if (bfvol[i].sym)
        blocks[bfvol[i].j][bfvol[i].i] = true;
    }

    for (unsigned i = 0; i < bfsurf.size(); i++)
      blocks[bfsurf[i].i][bfsurf[i].j] = true;
  }
  else
  {
    for (unsigned i = 0; i < jfvol.size(); i++) {
      blocks[jfvol[i].i][jfvol[i].j] = true;
      if (jfvol[i].sym)
        blocks[jfvol[i].j][jfvol[i].i] = true;
    }

    for (unsigned i = 0; i < jfsurf.size(); i++)
      blocks[jfsurf[i].i][jfsurf[i].j] = true;
  }

  return blocks;
}


//// areas /////////////////////////////////////////////////////////////////////////////////////////

int WeakForm::def_area(int n, ...)
{
  Area newarea;
  va_list ap; va_start(ap, n);
  for (int i = 0; i < n; i++)
    newarea.markers.push_back(va_arg(ap, int));
  va_end(ap);

  areas.push_back(newarea);
  return -(int)areas.size();
}


bool WeakForm::is_in_area_2(int marker, int area) const
{
  if (-area > (int)areas.size()) error("Invalid area number.");
  const Area* a = &areas[-area-1];

  for (unsigned int i = 0; i < a->markers.size(); i++)
    if (a->markers[i] == marker)
      return true;

  return false;
}
