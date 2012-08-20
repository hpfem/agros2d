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

#include "post_values.h"
#include "module.h"
#include "module_agros.h"
#include "field.h"
#include "block.h"
#include "problem.h"
#include "logview.h"

#include "util.h"
#include "scene.h"
#include "scenelabel.h"

#include "hermes2d/weakform_interface.h"
#include "solutionstore.h"


{{CLASS}}ViewScalarFilter<double>::{{CLASS}}ViewScalarFilter(FieldInfo *fieldInfo,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln,
                                           QString variable)
    : Hermes::Hermes2D::Filter<double>(sln), m_variable(variable)
{
   
}

{{CLASS}}ViewScalarFilter<double>::~{{CLASS}}ViewScalarFilter()
{
    
}

double {{CLASS}}ViewScalarFilter<double>::get_pt_value(double x, double y, int item)
{
    return 0.0;
}

void {{CLASS}}ViewScalarFilter<double>::precalculate(int order, int mask)
{
    bool isLinear = (m_fieldInfo->linearityType() == LinearityType_Linear);

    Hermes::Hermes2D::Quad2D* quad = Hermes::Hermes2D::Filter<double>::quads[Hermes::Hermes2D::Function<double>::cur_quad];
    int np = quad->get_num_points(order, this->get_active_element()->get_mode());
    node = Hermes::Hermes2D::Function<double>::new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

    for (int k = 0; k < Hermes::Hermes2D::Filter<double>::num; k++)
    {
        Hermes::Hermes2D::Filter<double>::sln[k]->set_quad_order(order, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
        Hermes::Hermes2D::Filter<double>::sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
        value[k] = Hermes::Hermes2D::Filter<double>::sln[k]->get_fn_values();
    }

    Hermes::Hermes2D::Filter<double>::update_refmap();

    double *x = Hermes::Hermes2D::MeshFunction<double>::refmap->get_phys_x(order);
    double *y = Hermes::Hermes2D::MeshFunction<double>::refmap->get_phys_y(order);
    Hermes::Hermes2D::Element *e = Hermes::Hermes2D::MeshFunction<double>::refmap->get_active_element();

    // set material
    SceneMaterial *material = Util::scene()->labels->at(atoi(Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().
                                                             get_user_marker(e->marker).marker.c_str()))->marker(m_fieldInfo);

    foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        if (isLinear || variable->expressionNonlinear().isEmpty())
            m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).number();

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < Hermes::Hermes2D::Filter<double>::num; k++)
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }

        // init nonlinear material
        setNonlinearMaterial(material);

        // parse expression
        try
        {
            node->values[0][0][i] = m_parsers[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "Scalar view: " << e.GetMsg() << std::endl;
        }
    }

    delete [] value;
    delete [] dudx;
    delete [] dudy;

    if (Hermes::Hermes2D::Function<double>::nodes->present(order))
    {
        assert(Hermes::Hermes2D::Function<double>::nodes->get(order) == Hermes::Hermes2D::Function<double>::cur_node);
        ::free(Hermes::Hermes2D::Function<double>::nodes->get(order));
    }
    Hermes::Hermes2D::Function<double>::nodes->add(node, order);
    Hermes::Hermes2D::Function<double>::cur_node = node;
}

{{CLASS}}ViewScalarFilter<double>* {{CLASS}}ViewScalarFilter<double>::clone()
{
    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> slns;

    for (int i = 0; i < this->num; i++)
        slns.push_back(this->sln[i]->clone());

    return new {{CLASS}}ViewScalarFilter(m_fieldInfo, slns, m_variable);
}

