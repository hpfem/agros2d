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

#include "{{ID}}_extfunction.h"
#include "{{ID}}_filter.h"
#include "{{ID}}_interface.h"

#include "util.h"
#include "util/global.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "hermes2d/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

{{CLASS}}ViewScalarFilter::{{CLASS}}ViewScalarFilter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                     MultiArrayDeal *ma,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp)
    : dealii::DataPostprocessorScalar<2>("Field",  dealii::update_values | dealii::update_gradients | dealii::update_q_points),
      m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType),
      ma(ma), m_variable(variable), m_physicFieldVariableComp(physicFieldVariableComp)
{
    m_variableHash = qHash(m_variable);

    {{#SPECIAL_FUNCTION_SOURCE}}
    if(m_fieldInfo->functionUsedInAnalysis("{{SPECIAL_FUNCTION_ID}}"))
    {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));
    {{/SPECIAL_FUNCTION_SOURCE}}

    m_coordinateType = Agros2D::problem()->config()->coordinateType();
    m_labels = Agros2D::scene()->labels;
}

{{CLASS}}ViewScalarFilter::~{{CLASS}}ViewScalarFilter()
{
}

void {{CLASS}}ViewScalarFilter::compute_derived_quantities_scalar (const std::vector<double> &uh,
                                                                   const std::vector<dealii::Tensor<1,2> > &duh,
                                                                   const std::vector<dealii::Tensor<2,2> > &dduh,
                                                                   const std::vector<dealii::Point<2> > &normals,
                                                                   const std::vector<dealii::Point<2> > &evaluation_points,
                                                                   std::vector<dealii::Vector<double> > &computed_quantities) const
{
    // qDebug() << "compute_derived_quantities_scalar " << computed_quantities.size();
    // qDebug() << "uh.size() " << uh.size();
    // qDebug() << "duh.size() " << duh.size();

    dealii::Point<2> center((evaluation_points.front()[0] + evaluation_points.back()[0]) / 2.0,
            (evaluation_points.front()[1] + evaluation_points.back()[1]) / 2.0);

    // qDebug() << evaluation_points.size() << "center" << center[0] << center[1];

    std::pair<typename dealii::Triangulation<2>::active_cell_iterator, dealii::Point<2> > current_cell =
            dealii::GridTools::find_active_cell_around_point(dealii::MappingQ1<2>(), *m_fieldInfo->initialMesh().get(), center);

    // find marker
    SceneLabel *label = m_labels->at(current_cell.first->material_id() - 1);
    SceneMaterial *material = label->marker(m_fieldInfo);

    const Value *material_electrostatic_permittivity = material->valueNakedPtr(QLatin1String("electrostatic_permittivity"));
    const Value *material_electrostatic_charge_density = material->valueNakedPtr(QLatin1String("electrostatic_charge_density"));

    int i = 0;

    double solution_values[1][1];
    dealii::Tensor<1, 2> solution_grads[1][1];

    for (unsigned int k = 0; k < computed_quantities.size(); k++)
    {
        solution_values[i][0] = uh[k];
        solution_grads[i][0] = duh[k];

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}
        {{#VARIABLE_SOURCE}}
        if ((m_variableHash == {{VARIABLE_HASH}})
                && (m_coordinateType == {{COORDINATE_TYPE}})
                && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                && (m_physicFieldVariableComp == {{PHYSICFIELDVARIABLECOMP_TYPE}}))
            computed_quantities[k](0) = {{EXPRESSION}};
        {{/VARIABLE_SOURCE}}
    }
}

/*
Hermes::Hermes2D::Func<double> *{{CLASS}}ViewScalarFilter::get_pt_value(double x, double y, bool use_MeshHashGrid, Hermes::Hermes2D::Element* e)
{
    return NULL;
}

void {{CLASS}}ViewScalarFilter::precalculate(unsigned short order, unsigned short mask)
{
    Hermes::Hermes2D::Quad2D* quad = this->quads[Hermes::Hermes2D::Function<double>::cur_quad];
    int np = quad->get_num_points(order, this->get_active_element()->get_mode());
    
    for (int k = 0; k < this->solutions.size(); k++)
    {
        this->solutions[k]->set_quad_order(order, Hermes::Hermes2D::H2D_FN_DEFAULT);
        /// \todo Find out why Qt & OpenGL renders the outputs color-less if dudx, dudy, valus are 'const double*'.
        dudx[k] = const_cast<double*>(this->solutions[k]->get_dx_values());
        dudy[k] = const_cast<double*>(this->solutions[k]->get_dy_values());
        value[k] = const_cast<double*>(this->solutions[k]->get_fn_values());
    }

    this->update_refmap();

    double *x = this->refmap.get_phys_x(order);
    double *y = this->refmap.get_phys_y(order);
    Hermes::Hermes2D::Element *e = this->refmap.get_active_element();

    // set material
    SceneMaterial *material = m_labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().
                                           get_user_marker(e->marker).marker.c_str()))->marker(m_fieldInfo);

    int elementMarker = e->marker;

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}
    {{#VARIABLE_SOURCE}}
    if ((m_variableHash == {{VARIABLE_HASH}})
            && (m_coordinateType == {{COORDINATE_TYPE}})
            && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
            && (m_physicFieldVariableComp == {{PHYSICFIELDVARIABLECOMP_TYPE}}))
        for (int i = 0; i < np; i++)
            this->values[0][0][i] = {{EXPRESSION}};
    {{/VARIABLE_SOURCE}}
}

{{CLASS}}ViewScalarFilter* {{CLASS}}ViewScalarFilter::clone() const
{
    std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > slns;

    for (int i = 0; i < this->solutions.size(); i++)
        slns.push_back(this->solutions[i]->clone());

    {{CLASS}}ViewScalarFilter *filter = new {{CLASS}}ViewScalarFilter(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType, slns, m_variable, m_physicFieldVariableComp);

    return filter;
}
*/
