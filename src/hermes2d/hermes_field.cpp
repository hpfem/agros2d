// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "hermes_field.h"

#include "hermes_general.h"
#include "hermes_electrostatic.h"
#include "hermes_magnetic.h"
#include "hermes_heat.h"
#include "hermes_current.h"
#include "hermes_elasticity.h"

#include "scene.h"

HermesField *hermesFieldFactory(PhysicField physicField)
{
    switch (physicField)
    {
    case PHYSICFIELD_GENERAL:
        return new HermesGeneral();
    case PHYSICFIELD_ELECTROSTATIC:
        return new HermesElectrostatic();
    case PHYSICFIELD_MAGNETIC:
        return new HermesMagnetic();
    case PHYSICFIELD_HEAT:
        return new HermesHeat();
    case PHYSICFIELD_CURRENT:
        return new HermesCurrent();
    case PHYSICFIELD_ELASTICITY:
        return new HermesElasticity();
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. hermesObjectFactory()" << endl;
        throw;
        break;
    }
}

// *********************************************************************************************************************************************

ViewScalarFilter::ViewScalarFilter(MeshFunction *sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) : Filter(sln1)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

ViewScalarFilter::ViewScalarFilter(MeshFunction *sln1, MeshFunction *sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) : Filter(sln1, sln2)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

double ViewScalarFilter::get_pt_value(double x, double y, int item)
{
    error("Not implemented");
}

void ViewScalarFilter::precalculate(int order, int mask)
{
    Quad2D* quad = quads[cur_quad];
    int np = quad->get_num_points(order);
    node = new_node(FN_VAL_0, np);

    if (sln[0])
    {
        sln[0]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[0]->get_dx_dy_values(dudx1, dudy1);
        value1 = sln[0]->get_fn_values();
    }

    if (num == 2 && sln[1])
    {
        sln[1]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[1]->get_dx_dy_values(dudx2, dudy2);
        value2 = sln[1]->get_fn_values();
    }

    update_refmap();

    x = refmap->get_phys_x(order);
    y = refmap->get_phys_y(order);
    Element *e = refmap->get_active_element();

    labelMarker = Util::scene()->labels[e->marker]->marker;

    for (int i = 0; i < np; i++)
    {
        calculateVariable(i);
    }

    replace_cur_node(node);
}
