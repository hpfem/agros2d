#ifndef SPACE_CONSTANT_H
#define SPACE_CONSTANT_H

#include "hermes2d.h"

namespace Hermes
{
    namespace Hermes2D
    {
        /// @ingroup spaces
        /// L2Space represents a space of Scalar functions with discontinuities along
        /// mesh edges.<br>
    /// Typical usage:<br>
    /// ...<br>
    /// <br>
    /// // Initialize space.<br>
    /// int globalPolynomialOrder = 4;<br>
    /// Hermes::Hermes2D::L2Space<double> space(&mesh, &bcs, globalPolynomialOrder);<br>
        template<typename Scalar>
        class HERMES_API ConstSpace : public Space<Scalar>
        {
        public:
            ConstSpace();
            ConstSpace(MeshSharedPtr mesh,Shapeset* shapeset = NULL);

            virtual ~ConstSpace();

            virtual Scalar* get_bc_projection(SurfPos* surf_pos, int order, EssentialBoundaryCondition<Scalar> *bc);

            virtual void get_element_assembly_list(Element* e, AsmList<Scalar>* al, unsigned int first_dof = 0) const;

            /// Copy from Space instance 'space'
            virtual void copy(SpaceSharedPtr<Scalar> space, MeshSharedPtr new_mesh);
        protected:
            virtual int get_edge_order(Element* e, int edge) const {
                return H2D_MAKE_EDGE_ORDER(e->get_mode(), edge, this->edata[e->id].order);
            }

            virtual void set_shapeset(Shapeset* shapeset);

            virtual SpaceType get_type() const { return HERMES_CONST_SPACE; }

            /// Common code for the constructors.
            void init(Shapeset* shapeset);

            struct L2Data
            {
                int vdof[H2D_MAX_NUMBER_VERTICES];
                int edof[H2D_MAX_NUMBER_EDGES];
            };

            L2Data* ldata;
            int lsize;

            virtual void resize_tables();

            virtual void assign_vertex_dofs() {}
            virtual void assign_edge_dofs() {}
            virtual void assign_bubble_dofs();

            virtual void get_vertex_assembly_list(Element* e, int iv, AsmList<Scalar>* al) const;
            virtual void get_boundary_assembly_list_internal(Element* e, int surf_num, AsmList<Scalar>* al) const;
            virtual void get_bubble_assembly_list(Element* e, AsmList<Scalar>* al) const;
            template<typename T> friend class Space<T>::ReferenceSpaceCreator;
            friend class Space<Scalar>;
        };
    }
}


#endif // SPACE_CONSTANT_H
