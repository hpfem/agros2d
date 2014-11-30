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

#include "{{ID}}_weakform.h"
// #include "{{ID}}_extfunction.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/bdf2.h"

#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>

{{#EXACT_SOURCE}}
template <int dim>
class Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}} : public dealii::Function<dim>
{
public:
    Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}}(SceneBoundary *boundary)
    : dealii::Function<dim>({{NUM_SOLUTIONS}})
    {
        {{#VARIABLE_SOURCE}}
        {{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
    }

    virtual ~Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}}() {}

virtual double value (const dealii::Point<dim> &p,
                      const unsigned int component) const
{
    // qDebug() << "Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}} - value";

    {{#FORM_EXPRESSION}}
    // {{EXPRESSION_ID}}
    if (component == {{ROW_INDEX}})
        return {{EXPRESSION}};{{/FORM_EXPRESSION}}
}

virtual void vector_value (const dealii::Point<dim> &p,
                           dealii::Vector<double> &values) const
{
    // qDebug() << "Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}} - vector_value";

    {{#FORM_EXPRESSION}}
    // {{EXPRESSION_ID}}
    values[{{ROW_INDEX}}] = {{EXPRESSION}};{{/FORM_EXPRESSION}}
}

virtual void value_list (const std::vector<dealii::Point<dim> > &points,
                         std::vector<double> &values,
                         const unsigned int component = 0) const
{
    // qDebug() << "Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}} - value_list";

    for (unsigned int i = 0; i < points.size(); ++i)
    {
        dealii::Point<2> p = points[i];
        {{#FORM_EXPRESSION}}
        // {{EXPRESSION_ID}}
        if (component == {{ROW_INDEX}})
            values[i] = {{EXPRESSION}};{{/FORM_EXPRESSION}}
    }
}

virtual void vector_value_list (const std::vector<dealii::Point<dim> > &points,
                                std::vector<dealii::Vector<double> > &values) const
{
    // qDebug() << "Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}} - vector_value_list";

    for (unsigned int i = 0; i < points.size(); ++i)
        vector_value(points[i], values[i]);
}

private:
{{#VARIABLE_SOURCE}}
const Value *{{VARIABLE_SHORT}};{{/VARIABLE_SOURCE}}
};
{{/EXACT_SOURCE}}

// *************************************************************************************************************************************************

/*
void SolverDeal{{CLASS}}::setup()
{

}
*/

void SolverDeal{{CLASS}}::assembleSystem()
{
    // assemble
    dealii::QGauss<2> quadrature_formula(m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + 5);
    dealii::QGauss<2-1> face_quadrature_formula(m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + 5);

    dealii::FEValues<2> fe_values (*m_fe, quadrature_formula, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);
    dealii::FEFaceValues<2> fe_face_values (*m_fe, face_quadrature_formula, dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values);

    const unsigned int dofs_per_cell = m_fe->dofs_per_cell;
    const unsigned int n_q_points = quadrature_formula.size();
    const unsigned int n_face_q_points = face_quadrature_formula.size();

    dealii::FullMatrix<double> cell_matrix (dofs_per_cell, dofs_per_cell);
    dealii::Vector<double> cell_rhs (dofs_per_cell);

    std::vector<dealii::types::global_dof_index> local_dof_indices (dofs_per_cell);

    dealii::DoFHandler<2>::active_cell_iterator cell = m_doFHandler->begin_active(), endc = m_doFHandler->end();
    for (; cell != endc; ++cell)
    {
        fe_values.reinit (cell);

        // value and grad cache
        std::vector<dealii::Vector<double> > shape_value(dofs_per_cell, dealii::Vector<double>(n_q_points));
        std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad(dofs_per_cell, std::vector<dealii::Tensor<1,2> >(n_q_points));
        std::vector<dealii::Vector<double> > shape_face_value(dofs_per_cell, dealii::Vector<double>(n_face_q_points));
        // std::vector<std::vector<dealii::Tensor<1,2> > > shape_face_grad(dofs_per_cell, std::vector<dealii::Tensor<1,2> >(n_face_q_points));

        // cache volume
        for (unsigned int i = 0; i < dofs_per_cell; ++i)
        {
            for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
            {
                shape_value[i][q_point] = fe_values.shape_value(i, q_point);
                shape_grad[i][q_point] = fe_values.shape_grad(i, q_point);
            }
        }

        // cache surface
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if (cell->face(face)->at_boundary())
            {
                fe_face_values.reinit(cell, face);

                for (unsigned int i = 0; i < dofs_per_cell; ++i)
                {
                    for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                    {
                        shape_face_value[i][q_point] = fe_face_values.shape_value(i, q_point);
                        // shape_face_grad[i][q_point] = fe_face_values.shape_grad(i, q_point);
                    }
                }
            }
        }

        // local matrix
        cell_matrix = 0;
        cell_rhs = 0;

        // materials
        for (int labelNum = 0; labelNum < Agros2D::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *material = Agros2D::scene()->labels->at(labelNum)->marker(m_fieldInfo);

            if (material != Agros2D::scene()->materials->getNone(m_fieldInfo))
            {
                // MATRIX VOLUME
                {{#VOLUME_MATRIX_SOURCE}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
                {
                    if (cell->material_id() == labelNum + 1)
                    {
                        {{#VARIABLE_SOURCE}}
                        const Value *{{VARIABLE_SHORT}} = material->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                        // matrix
                        for (unsigned int i = 0; i < dofs_per_cell; ++i)
                        {
                            const unsigned int component_i = m_fe->system_to_component_index(i).first;

                            for (unsigned int j = 0; j < dofs_per_cell; ++j)
                            {
                                const unsigned int component_j = m_fe->system_to_component_index(j).first;
                                {{#FORM_EXPRESSION}}
                                // {{EXPRESSION_ID}}
                                if (component_i == {{ROW_INDEX}} && component_j == {{COLUMN_INDEX}}) // TODO: speed up
                                {
                                    for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
                                    {
                                        const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                                        cell_matrix(i,j) += fe_values.JxW(q_point) *({{EXPRESSION}});
                                    }
                                }{{/FORM_EXPRESSION}}
                            }
                        }
                    }
                }
                {{/VOLUME_MATRIX_SOURCE}}

                // VECTOR VOLUME
                {{#VOLUME_VECTOR_SOURCE}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
                {
                    if (cell->material_id() == labelNum + 1)
                    {
                        {{#VARIABLE_SOURCE}}
                        const Value *{{VARIABLE_SHORT}} = material->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                        // rhs
                        for (unsigned int i = 0; i < dofs_per_cell; ++i)
                        {
                            const unsigned int component_i = m_fe->system_to_component_index(i).first;
                            {{#FORM_EXPRESSION}}
                            // {{EXPRESSION_ID}}
                            if (component_i == {{ROW_INDEX}}) // TODO: speed up
                            {
                                for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
                                {
                                    const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                                    cell_rhs(i) += fe_values.JxW(q_point) *({{EXPRESSION}});
                                }
                            }{{/FORM_EXPRESSION}}
                        }
                    }
                }
                {{/VOLUME_VECTOR_SOURCE}}                
            }
        }

        // boundaries
        for (int edgeNum = 0; edgeNum < Agros2D::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Agros2D::scene()->edges->at(edgeNum)->marker(m_fieldInfo);

            if (boundary != Agros2D::scene()->boundaries->getNone(m_fieldInfo))
            {
                // MATRIX SURFACE
                {{#SURFACE_MATRIX_SOURCE}}
                // {{BOUNDARY_ID}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                        && boundary->type() == "{{BOUNDARY_ID}}")
                {
                    {{#VARIABLE_SOURCE}}
                    const Value *{{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                    for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
                    {
                        if (cell->face(face)->at_boundary())
                        {
                            fe_face_values.reinit (cell, face);

                            // value and grad cache
                            std::vector<dealii::Vector<double> > shape_value = shape_face_value;
                            // std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad = shape_face_grad;

                            for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            {
                                const unsigned int component_i = m_fe->system_to_component_index(i).first;

                                for (unsigned int j = 0; j < dofs_per_cell; ++j)
                                {
                                    const unsigned int component_j = m_fe->system_to_component_index(j).first;
                                    {{#FORM_EXPRESSION}}
                                    // {{EXPRESSION_ID}}
                                    if (component_i == {{ROW_INDEX}} && component_j == {{COLUMN_INDEX}}) // TODO: speed up
                                    {
                                        for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                                        {
                                            const dealii::Point<2> p = fe_face_values.quadrature_point(q_point);
                                            cell_matrix(i,j) += fe_face_values.JxW(q_point) *({{EXPRESSION}});
                                        }
                                    }{{/FORM_EXPRESSION}}
                                }
                            }
                        }
                    }
                }
                {{/SURFACE_MATRIX_SOURCE}}

                // VECTOR SURFACE
                {{#SURFACE_VECTOR_SOURCE}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                        && boundary->type() == "{{BOUNDARY_ID}}")
                {
                    {{#VARIABLE_SOURCE}}
                    const Value *{{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                    for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
                    {
                        if (cell->face(face)->at_boundary())
                        {
                            fe_face_values.reinit(cell, face);

                            // value and grad cache
                            std::vector<dealii::Vector<double> > shape_value = shape_face_value;
                            // std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad = shape_face_grad;

                            for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            {
                                const unsigned int component_i = m_fe->system_to_component_index(i).first;
                                {{#FORM_EXPRESSION}}
                                // {{EXPRESSION_ID}}
                                if (component_i == {{ROW_INDEX}}) // TODO: speed up
                                {
                                    for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                                    {
                                        const dealii::Point<2> p = fe_face_values.quadrature_point(q_point);
                                        cell_rhs(i) += fe_face_values.JxW(q_point) *({{EXPRESSION}});
                                    }
                                }{{/FORM_EXPRESSION}}
                            }
                        }
                    }
                }
                {{/SURFACE_VECTOR_SOURCE}}
            }
        }

        // distribute local to global matrix
        cell->get_dof_indices(local_dof_indices);

        // distribute local to global system
        hanging_node_constraints.distribute_local_to_global(cell_matrix,
                                                            cell_rhs,
                                                            local_dof_indices,
                                                            system_matrix,
                                                            system_rhs);

        /*
        for (unsigned int i=0; i<dofs_per_cell; ++i)
        {
            for (unsigned int j=0; j<dofs_per_cell; ++j)
                system_matrix.add (local_dof_indices[i], local_dof_indices[j], cell_matrix(i,j));

            system_rhs(local_dof_indices[i]) += cell_rhs(i);
        }
        */
    }
}

void SolverDeal{{CLASS}}::assembleDirichlet()
{
    hanging_node_constraints.condense(system_matrix);
    hanging_node_constraints.condense(system_rhs);

    for (int i = 0; i < Agros2D::scene()->edges->count(); i++)
    {
        SceneEdge *edge = Agros2D::scene()->edges->at(i);
        SceneBoundary *boundary = edge->marker(m_fieldInfo);

        if (boundary && (!boundary->isNone()))
        {
            {{#EXACT_SOURCE}}
            // {{BOUNDARY_ID}}
            if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                    && boundary->type() == "{{BOUNDARY_ID}}")
            {
                // component mask
                std::vector<bool> mask;
                {{#FORM_EXPRESSION_MASK}}
                mask.push_back({{MASK}});{{/FORM_EXPRESSION_MASK}}
                dealii::ComponentMask component_mask(mask);

                std::map<dealii::types::global_dof_index,double> boundary_values;
                dealii::VectorTools::interpolate_boundary_values (*m_doFHandler, i+1,
                                                                  Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}}<2>(boundary),
                                                                  boundary_values,
                                                                  component_mask);
                dealii::MatrixTools::apply_boundary_values (boundary_values, system_matrix, *m_solution, system_rhs);
            }
            {{/EXACT_SOURCE}}
        }
    }
}
