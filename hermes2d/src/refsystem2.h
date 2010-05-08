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

#ifndef __H2D_REFSYSTEM2_H
#define __H2D_REFSYSTEM2_H

#include "linsystem.h"

class Mesh;


class H2D_API RefNonlinSystem : public NonlinSystem
{
public:

  RefNonlinSystem(NonlinSystem* base, int order_increase = 1, int refinement = 1)
          : NonlinSystem(base->wf, base->solver)
  {
    this->base = base;
    order_inc = order_increase;
    this->refinement = refinement;
    ref_meshes = NULL;
    ref_spaces = NULL;
  }

  virtual ~RefNonlinSystem()
  {
    free_ref_data();
  }


  /// Do not call in this class
  void set_spaces(int n, ...)
  {
    error("set_spaces must not be called in RefSystem.");
  }

  /// Do not call in this class
  void set_pss(int n, ...)
  {
    error("set_pss must not be called in RefSystem.");
  }


  /// Creates reference (fine) meshes and spaces and assembles the
  /// reference system.
  void prepare()
  {
    int i, j;

    // get rid of any previous data
    free_ref_data();

    ref_meshes = new Mesh*[NonlinSystem::wf->neq];
    ref_spaces = new Space*[NonlinSystem::wf->neq];

    // copy meshes from the coarse problem, refine them
    for (i = 0; i < NonlinSystem::wf->neq; i++)
    {
      Mesh* mesh = base->spaces[i]->get_mesh();

      // check if we already have the same mesh
      for (j = 0; j < i; j++)
        if (mesh->get_seq() == base->spaces[j]->get_mesh()->get_seq())
          break;

      if (j < i) // yes
      {
        ref_meshes[i] = ref_meshes[j];
      }
      else // no, copy and refine the coarse one
      {
        Mesh* rmesh = new Mesh;
        rmesh->copy(mesh);
        if (refinement ==  1) rmesh->refine_all_elements();
        if (refinement == -1) rmesh->unrefine_all_elements();
        ref_meshes[i] = rmesh;
      }
    }

    // duplicate spaces from the coarse problem, assign reference orders and dofs
    int dofs = 0;
    for (i = 0; i < NonlinSystem::wf->neq; i++)
    {
      ref_spaces[i] = base->spaces[i]->dup(ref_meshes[i]);
      if (refinement == -1)
      {
        Element* re;
        for_all_active_elements(re, ref_meshes[i])
        {
          Mesh* mesh = base->spaces[i]->get_mesh();
          Element* e = mesh->get_element(re->id);
          int max_o = 0;
          if (e->active)
            max_o = H2D_GET_H_ORDER(base->spaces[i]->get_element_order(e->id));
          else
          {
            for (int son = 0; son < 4; son++)
            {
              if (e->sons[son] != NULL)
              {
                //max_o += H2D_GET_H_ORDER(base->spaces[i]->get_element_order(e->sons[son]->id));
                int o = H2D_GET_H_ORDER(base->spaces[i]->get_element_order(e->sons[son]->id));
                if (o > max_o) max_o = o;
              }
            }
            max_o = std::max(1, max_o);
          }
          ref_spaces[i]->set_element_order(re->id, std::max(1, ((int) max_o) + order_inc));
        }

      }
      else
        ref_spaces[i]->copy_orders(base->spaces[i], order_inc);

      dofs += ref_spaces[i]->assign_dofs(dofs);
    }

    memcpy(NonlinSystem::spaces, ref_spaces, sizeof(Space*) * NonlinSystem::wf->neq);
    memcpy(NonlinSystem::pss, base->pss, sizeof(PrecalcShapeset*) * NonlinSystem::wf->neq);
    NonlinSystem::have_spaces = true;

  }

  void assemble()
  {
    NonlinSystem::assemble();
  }


  /// Frees reference spaces and meshes. Called
  /// automatically on desctruction.
  void free_ref_data()
  {
    int i, j;

    // free reference meshes
    if (ref_meshes != NULL)
    {
      for (i = 0; i < NonlinSystem::wf->neq; i++)
      {
        for (j = 0; j < i; j++)
          if (ref_meshes[j] == ref_meshes[i])
            break;

        if (i == j) delete ref_meshes[i];
      }

      delete [] ref_meshes;
      ref_meshes = NULL;
    }

    // free reference spaces
    if (ref_spaces != NULL)
    {
      for (i = 0; i < NonlinSystem::wf->neq; i++)
        delete ref_spaces[i];

      delete [] ref_spaces;
      ref_spaces = NULL;
    }
  }

  Space* get_ref_space(int i) { return ref_spaces[i];}



protected:

  NonlinSystem* base;
  int order_inc;
  int refinement;

  Mesh**  ref_meshes;
  Space** ref_spaces;

};



#endif

