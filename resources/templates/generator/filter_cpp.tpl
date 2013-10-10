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

#include "{{ID}}_filter.h"
#include "{{ID}}_interface.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "hermes2d/plugin_interface.h"


{{CLASS}}ViewScalarFilter::{{CLASS}}ViewScalarFilter(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                                           const QString &variable,
                                           PhysicFieldVariableComp physicFieldVariableComp)
    : Hermes::Hermes2D::Filter<double>(sln), m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType),
      m_variable(variable), m_physicFieldVariableComp(physicFieldVariableComp)
{
    m_variableHash = qHash(m_variable);
}

Hermes::Hermes2D::Func<double> *{{CLASS}}ViewScalarFilter::get_pt_value(double x, double y, bool use_MeshHashGrid, Hermes::Hermes2D::Element* e)
{
    return NULL;
}

void {{CLASS}}ViewScalarFilter::precalculate(int order, int mask)
{
    Hermes::Hermes2D::Quad2D* quad = this->quads[Hermes::Hermes2D::Function<double>::cur_quad];
    int np = quad->get_num_points(order, this->get_active_element()->get_mode());
    Hermes::Hermes2D::Function<double>::Node* node = this->new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[this->num];
    double **dudx = new double*[this->num];
    double **dudy = new double*[this->num];

    for (int k = 0; k < this->num; k++)
    {
        this->sln[k]->set_quad_order(order, Hermes::Hermes2D::H2D_FN_DEFAULT);
        dudx[k] = this->sln[k]->get_dx_values();
        dudy[k] = this->sln[k]->get_dy_values();
        value[k] = this->sln[k]->get_fn_values();
    }

    this->update_refmap();

    double *x = this->refmap->get_phys_x(order);
    double *y = this->refmap->get_phys_y(order);
    Hermes::Hermes2D::Element *e = this->refmap->get_active_element();

    // set material
    SceneMaterial *material = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().
                                                             get_user_marker(e->marker).marker.c_str()))->marker(m_fieldInfo);

    double area = m_fieldInfo->initialMesh()->get_marker_area(e->marker);

    {{#VARIABLE_MATERIAL}}Value *material_{{MATERIAL_VARIABLE}} = &material->value(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    {{SPECIAL_FUNCTION_FULL_NAME}}<double> {{SPECIAL_FUNCTION_NAME}};{{#PARAMETERS}}
    {{SPECIAL_FUNCTION_NAME}}.{{PARAMETER_NAME}} = material_{{PARAMETER_FULL_NAME}}->number(); {{/PARAMETERS}}
    {{SPECIAL_FUNCTION_NAME}}.setVariant("{{SELECTED_VARIANT}}");
    {{SPECIAL_FUNCTION_NAME}}.setType(specialFunctionTypeFromStringKey("{{TYPE}}"));
    {{SPECIAL_FUNCTION_NAME}}.setBounds({{FROM}}, {{TO}}, {{EXTRAPOLATE_LOW_PRESENT}}, {{EXTRAPOLATE_HI_PRESENT}});
    {{SPECIAL_FUNCTION_NAME}}.setArea(area);
    {{SPECIAL_FUNCTION_NAME}}.setUseInterpolation(false);
    {{/SPECIAL_FUNCTION_SOURCE}}

    {{#VARIABLE_SOURCE}}
    if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}})
            && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
            && (m_physicFieldVariableComp == {{PHYSICFIELDVARIABLECOMP_TYPE}})
            && (m_variableHash == {{VARIABLE_HASH}}))
        for (int i = 0; i < np; i++)
            node->values[0][0][i] = {{EXPRESSION}};
    {{/VARIABLE_SOURCE}}

    delete [] value;
    delete [] dudx;
    delete [] dudy;

    if(this->nodes->present(order))
    {
      assert(this->nodes->get(order) == this->cur_node);
      ::free(this->nodes->get(order));
    }
    this->nodes->add(node, order);
    this->cur_node = node;
}

{{CLASS}}ViewScalarFilter* {{CLASS}}ViewScalarFilter::clone() const
{
    Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > slns;

    for (int i = 0; i < this->num; i++)
        slns.push_back(this->sln[i]->clone());

    {{CLASS}}ViewScalarFilter *filter = new {{CLASS}}ViewScalarFilter(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType, slns, m_variable, m_physicFieldVariableComp);

    return filter;
}

