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

// #include "{{ID}}_extfunction.h"
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
                                                     MultiArray *ma,
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

// only one component
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

    int numberOfSolutions = m_fieldInfo->numberOfSolutions();

    dealii::Point<2> center((evaluation_points.front()[0] + evaluation_points.back()[0]) / 2.0,
            (evaluation_points.front()[1] + evaluation_points.back()[1]) / 2.0);

    // qDebug() << evaluation_points.size() << "center" << center[0] << center[1];

    std::pair<typename dealii::Triangulation<2>::active_cell_iterator, dealii::Point<2> > current_cell =
            dealii::GridTools::find_active_cell_around_point(dealii::MappingQ1<2>(), *m_fieldInfo->initialMesh(), center);

    // find marker
    SceneLabel *label = m_labels->at(current_cell.first->material_id() - 1);
    SceneMaterial *material = label->marker(m_fieldInfo);

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}

    std::vector<dealii::Vector<double> > solution_values(computed_quantities.size(), dealii::Vector<double>(numberOfSolutions));
    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(computed_quantities.size(), std::vector<dealii::Tensor<1,2> >(numberOfSolutions));

    for (unsigned int k = 0; k < computed_quantities.size(); k++)
    {
        dealii::Point<2> p = evaluation_points[k];

        solution_values[k][0] = uh[k];
        solution_grads[k][0] = duh[k];

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

// multiple components
void {{CLASS}}ViewScalarFilter::compute_derived_quantities_vector (const std::vector<dealii::Vector<double> > &uh,
                                                                   const std::vector<std::vector<dealii::Tensor<1,2> > > &duh,
                                                                   const std::vector<std::vector<dealii::Tensor<2,2> > > &dduh,
                                                                   const std::vector<dealii::Point<2> > &normals,
                                                                   const std::vector<dealii::Point<2> > &evaluation_points,
                                                                   std::vector<dealii::Vector<double> > &computed_quantities) const
{
    // qDebug() << "compute_derived_quantities_vector " << computed_quantities.size();
    // qDebug() << "uh.size() " << uh.size();
    // qDebug() << "duh.size() " << duh.size();

    int numberOfSolutions = m_fieldInfo->numberOfSolutions();

    dealii::Point<2> center((evaluation_points.front()[0] + evaluation_points.back()[0]) / 2.0,
            (evaluation_points.front()[1] + evaluation_points.back()[1]) / 2.0);

    // qDebug() << evaluation_points.size() << "center" << center[0] << center[1];

    std::pair<typename dealii::Triangulation<2>::active_cell_iterator, dealii::Point<2> > current_cell =
            dealii::GridTools::find_active_cell_around_point(dealii::MappingQ1<2>(), *m_fieldInfo->initialMesh(), center);

    // find marker
    SceneLabel *label = m_labels->at(current_cell.first->material_id() - 1);
    SceneMaterial *material = label->marker(m_fieldInfo);

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}

    std::vector<dealii::Vector<double> > solution_values(computed_quantities.size(), dealii::Vector<double>(numberOfSolutions));
    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(computed_quantities.size(), std::vector<dealii::Tensor<1,2> >(numberOfSolutions));

    for (unsigned int k = 0; k < computed_quantities.size(); k++)
    {
        dealii::Point<2> p = evaluation_points[k];

        for (int i = 0; i < numberOfSolutions; i++)
        {
            solution_values[k][i] = uh[k][i];
            solution_grads[k][i] = duh[k][i];
        }

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
