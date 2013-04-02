#include "global.h"
#include "space_constant.h"
#include "matrix.h"
#include "shapeset/shapeset_l2_all.h"
#include "boundary_conditions/essential_boundary_conditions.h"

namespace Hermes
{
  namespace Hermes2D
  {
    template<typename Scalar>
    ConstSpace<Scalar>::ConstSpace() : Space<Scalar>()
    {
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::init(Shapeset* shapeset)
    {
        std::cout << "ConstSpace::init()" << std::endl;

        if(shapeset == NULL)
      {
        this->shapeset = new L2Shapeset;
        this->own_shapeset = true;
      }
      ldata = NULL;
      lsize = 0;

      // set uniform poly order in elements
      this->set_uniform_order_internal(1, HERMES_ANY_INT);

      // enumerate basis functions
      this->assign_dofs();
    }

    template<typename Scalar>
    ConstSpace<Scalar>::ConstSpace(MeshSharedPtr mesh, Shapeset* shapeset)
      : Space<Scalar>(mesh, shapeset, NULL)
    {
      init(shapeset);
    }

    template<typename Scalar>
    ConstSpace<Scalar>::~ConstSpace()
    {
      ::free(ldata);
      if(this->own_shapeset)
        delete this->shapeset;
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::copy(SpaceSharedPtr<Scalar> space, MeshSharedPtr new_mesh)
    {
      Space<Scalar>::copy(space, new_mesh);

      ldata = NULL;
      lsize = 0;
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::set_shapeset(Shapeset *shapeset)
    {
      if(shapeset->get_id() < 40 && shapeset->get_id() > 29)
      {
        this->shapeset = shapeset;
        this->own_shapeset = false;
      }
      else
        throw Hermes::Exceptions::Exception("Wrong shapeset type in ConstSpace<Scalar>::set_shapeset()");
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::resize_tables()
    {
      if(lsize < this->mesh->get_max_element_id())
      {
        if(!lsize) lsize = 1000;
        while (lsize < this->mesh->get_max_element_id()) lsize = lsize * 3 / 2;
        ldata = (L2Data*) realloc(ldata, sizeof(L2Data) * lsize);
      }
      Space<Scalar>::resize_tables();
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::assign_bubble_dofs()
    {
      Element* e;
      this->bubble_functions_count = 0;
      int max_marker = 0;
      for_all_active_elements(e, this->mesh)
      {
        typename Space<Scalar>::ElementData* ed = &this->edata[e->id];
        ed->bdof = this->next_dof + e->marker - 1;
        ed->n = 1; //FIXME: this function might return invalid value because retrieved bubble functions for non-uniform orders might be invalid for the given order.
        max_marker = std::max(max_marker, e->marker);
      }
      this->next_dof += max_marker;
      this->bubble_functions_count = max_marker;

    }

    template<typename Scalar>
    void ConstSpace<Scalar>::get_vertex_assembly_list(Element* e, int iv, AsmList<Scalar>* al) const
    {}

    template<typename Scalar>
    void ConstSpace<Scalar>::get_element_assembly_list(Element* e, AsmList<Scalar>* al, unsigned int first_dof) const
    {
      if(e->id >= this->esize || this->edata[e->id].order < 0)
        throw Hermes::Exceptions::Exception("Uninitialized element order (id = #%d).", e->id);
      if(!this->is_up_to_date())
        throw Hermes::Exceptions::Exception("The space is out of date. You need to update it with assign_dofs()"
        " any time the mesh changes.");

      // add bubble functions to the assembly list
      al->cnt = 0;
      get_bubble_assembly_list(e, al);
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::get_bubble_assembly_list(Element* e, AsmList<Scalar>* al) const
    {
      typename Space<Scalar>::ElementData* ed = &this->edata[e->id];
      if(!ed->n) return;

      ed->order = 1;

      int* indices = this->shapeset->get_bubble_indices(ed->order, e->get_mode());
      //assert(ed->order == 1);
      for (int i = 0, dof = ed->bdof; i < ed->n; i++, dof += this->stride)
      {
        //printf("triplet: %d, %d, %f\n", *indices, dof, 1.0);
        al->add_triplet(*indices++, dof, 1.0);
      }
    }

    template<typename Scalar>
    void ConstSpace<Scalar>::get_boundary_assembly_list_internal(Element* e, int surf_num, AsmList<Scalar>* al) const
    {
      this->get_bubble_assembly_list(e, al);
    }

    template<typename Scalar>
    Scalar* ConstSpace<Scalar>::get_bc_projection(SurfPos* surf_pos, int order, EssentialBoundaryCondition<Scalar> *bc)
    {
      throw Hermes::Exceptions::Exception("Method get_bc_projection() called from an ConstSpace.");
      return NULL;
    }

    template HERMES_API class ConstSpace<double>;
    template HERMES_API class ConstSpace<std::complex<double> >;
  }
}

