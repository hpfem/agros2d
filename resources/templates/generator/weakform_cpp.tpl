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
#include "{{ID}}_extfunction.h"


#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d.h"
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
class Deal_{{FUNCTION_NAME}} : public dealii::ZeroFunction<dim>
{
public:
    Deal_{{FUNCTION_NAME}}(SceneBoundary *boundary, const unsigned int n_components = 1)
    : dealii::ZeroFunction<dim>(n_components)
    {
        {{#VARIABLE_SOURCE}}
        {{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
    }

    virtual ~Deal_{{FUNCTION_NAME}}() {}

virtual double value (const dealii::Point<dim> &p,
                      const unsigned int component) const
{
    double result = {{EXPRESSION}};
    return result;
}

virtual void vector_value (const dealii::Point<dim> &p,
                           dealii::Vector<double> &return_value) const
{
    double result = {{EXPRESSION}};
    std::fill(return_value.begin(), return_value.end(), result);
}

virtual void value_list (const std::vector<dealii::Point<dim> > &points,
                         std::vector<double> &values,
                         const unsigned int component = 0) const
{
    for (unsigned int i=0; i<points.size(); ++i)
    {
        dealii::Point<2> p = points[i];
        double result = {{EXPRESSION}};
        values[i] = result;
    }
}

virtual void vector_value_list (const std::vector<dealii::Point<dim> > &points,
                                std::vector<dealii::Vector<double> > &values) const
{
    for (unsigned int i=0; i<points.size(); ++i)
    {
        dealii::Point<2> p = points[i];
        double result = {{EXPRESSION}};
        std::fill(values[i].begin(), values[i].end(), result);
    }
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

        // local matrix
        cell_matrix = 0;
        cell_rhs = 0;

        // materials
        for (int labelNum = 0; labelNum < Agros2D::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *material = Agros2D::scene()->labels->at(labelNum)->marker(m_fieldInfo);
            assert(material);


            if (material != Agros2D::scene()->materials->getNone(m_fieldInfo))
            {
                {{#VOLUME_MATRIX_SOURCE}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
                {
                    {{#VARIABLE_SOURCE}}
                    const Value *{{VARIABLE_SHORT}} = material->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                    foreach (FormInfo form, Module::wfMatrixVolumeSeparated(m_fieldInfo->plugin()->module(), m_fieldInfo->analysisType(), m_fieldInfo->linearityType()))
                    {
                        if (cell->material_id() == labelNum + 1)
                        {
                            // matrix
                            for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            {
                                const unsigned int component_i = m_fe->system_to_component_index(i).first;

                                if ((form.i - 1) == component_i)
                                {
                                    for (unsigned int j = 0; j < dofs_per_cell; ++j)
                                    {
                                        const unsigned int component_j = m_fe->system_to_component_index(j).first;

                                        if ((form.j - 1) == component_j)
                                        {
                                            for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
                                            {
                                                const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                                                cell_matrix(i,j) += fe_values.JxW(q_point) *({{EXPRESSION}});
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                {{/VOLUME_MATRIX_SOURCE}}

                {{#VOLUME_VECTOR_SOURCE}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
                {
                    {{#VARIABLE_SOURCE}}
                    const Value *{{VARIABLE_SHORT}} = material->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                    foreach (FormInfo form, Module::wfVectorVolumeSeparated(m_fieldInfo->plugin()->module(), m_fieldInfo->analysisType(), m_fieldInfo->linearityType()))
                    {
                        if (cell->material_id() == labelNum + 1)
                        {
                            // rhs
                            for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            {
                                const unsigned int component_i = m_fe->system_to_component_index(i).first;

                                if ((form.i - 1) == component_i)
                                {
                                    for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
                                    {
                                        const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                                        cell_rhs(i) += fe_values.JxW(q_point) *({{EXPRESSION}});
                                    }
                                }
                            }
                        }
                    }
                }
                {{/VOLUME_VECTOR_SOURCE}}

                /*
                    // weak coupling
                    foreach(CouplingInfo* couplingInfo, field->couplingInfos())
                    {
                        foreach (FormInfo expression, couplingInfo->wfVectorVolumeSeparated(&couplingInfo->plugin()->coupling()->volume(),
                                                                                            couplingInfo->sourceField()->analysisType(),
                                                                                            couplingInfo->targetField()->analysisType(),
                                                                                            couplingInfo->couplingType(),
                                                                                            couplingInfo->linearityType()))
                        {
                            SceneMaterial* materialSource = Agros2D::scene()->labels->at(labelNum)->marker(couplingInfo->sourceField());
                            assert(materialSource);

                            if (materialSource != Agros2D::scene()->materials->getNone(couplingInfo->sourceField()))
                            {
                                expression.i += m_block->offset(field);
                                expression.j += m_block->offset(field);
                                registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), expression, materialSource, material, couplingInfo);
                            }
                        }

                        // todo remove code repetition
                        foreach (FormInfo expression, couplingInfo->wfMatrixVolumeSeparated(&couplingInfo->plugin()->coupling()->volume(),
                                                                                            couplingInfo->sourceField()->analysisType(),
                                                                                            couplingInfo->targetField()->analysisType(),
                                                                                            couplingInfo->couplingType(),
                                                                                            couplingInfo->linearityType()))
                        {
                            SceneMaterial* materialSource = Agros2D::scene()->labels->at(labelNum)->marker(couplingInfo->sourceField());
                            assert(materialSource);

                            if (materialSource != Agros2D::scene()->materials->getNone(couplingInfo->sourceField()))
                            {
                                expression.i += m_block->offset(field);
                                expression.j += m_block->offset(field);
                                registerFormCoupling(WeakForm_MatVol, QString::number(labelNum), expression, materialSource, material, couplingInfo);
                            }
                        }
                    }
                    */
            }
        }

        for (int edgeNum = 0; edgeNum < Agros2D::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Agros2D::scene()->edges->at(edgeNum)->marker(m_fieldInfo);
            assert(boundary);


            if (boundary != Agros2D::scene()->boundaries->getNone(m_fieldInfo))
            {
                Module::BoundaryType boundaryType = m_fieldInfo->boundaryType(boundary->type());
                foreach (FormInfo form, boundaryType.wfMatrixSurface())
                {
                    {{#SURFACE_VECTOR_SOURCE}}
                    if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
                    {
                        {{#VARIABLE_SOURCE}}
                        const Value *{{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}

                        for (unsigned int face=0; face<dealii::GeometryInfo<2>::faces_per_cell; ++face)
                        {
                            // boundary (Neumann)
                            if (cell->face(face)->at_boundary() && (cell->face(face)->boundary_indicator() == 2))
                            {
                                fe_face_values.reinit (cell, face);
                                for (unsigned int i = 0; i < dofs_per_cell; ++i)
                                {
                                    const unsigned int component_i = m_fe->system_to_component_index(i).first;

                                    if ((form.i - 1) == component_i)
                                    {
                                        for (unsigned int q_point=0; q_point < n_face_q_points; ++q_point)
                                        {
                                            const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                                            cell_rhs(i) += fe_face_values.JxW(q_point) *({{EXPRESSION}});
                                        }
                                    }
                                }
                            }
                        }
                    }
                    {{/SURFACE_VECTOR_SOURCE}}

                    /*
                    ROBIN - CHECK!!!
                    if (cell->face(face)->at_boundary() && (cell->face(face)->boundary_indicator() == 1))
                    {
                        fe_face_values.reinit(cell, face);

                        for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            for (unsigned int j = 0; j < dofs_per_cell; ++j)
                                for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                                    cell_matrix(i, j) += 1 / R_SI_1* fe_face_values.shape_value(i, q_point) * fe_face_values.shape_value(j, q_point) * fe_face_values.JxW(q_point);

                        for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                            for (unsigned int j = 0; j < dofs_per_cell; ++j)
                                cell_rhs(j) += 1/ R_SI_1 * THETA_1 * fe_face_values.shape_value(j, q_point) * fe_face_values.JxW(q_point);

                    }
                    */
                }
            }
        }

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
            {
                // system matrix
                system_matrix.add (local_dof_indices[i],
                                   local_dof_indices[j],
                                   cell_matrix(i,j));
            }

            // system rhs
            system_rhs(local_dof_indices[i]) += cell_rhs(i);
        }
        */
    }
}

void SolverDeal{{CLASS}}::assembleDirichlet()
{
    for (int i = 0; i < Agros2D::scene()->edges->count(); i++)
    {
        SceneEdge *edge = Agros2D::scene()->edges->at(i);
        SceneBoundary *boundary = edge->marker(m_fieldInfo);

        if (boundary && (!boundary->isNone()))
        {
            Module::BoundaryType boundaryType = m_fieldInfo->boundaryType(boundary->type());

            foreach (FormInfo form, boundaryType.essential())
            {
                // exact solution - Dirichlet BC
                // Hermes::Hermes2D::MeshFunctionSharedPtr<double> function = Hermes::Hermes2D::MeshFunctionSharedPtr<double>(fieldInfo->plugin()->exactSolution(problemId, &form, fieldInfo->initialMesh()));
                // static_cast<ExactSolutionScalarAgros<double> *>(function.get())->setMarkerTarget(boundary);

                // save function - boundary pairs, so thay can be easily updated in each time step;
                // m_exactSolutionFunctions[function] = boundary;

                // Hermes::Hermes2D::EssentialBoundaryCondition<double> *custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(), function);

                // this->bcs().at((form.i - 1) - 1 + this->offset(field))->add_boundary_condition(custom_form);
                //  cout << "adding BC i: " << form->i - 1 + this->offset(field) << " ( form i " << form->i << ", " << this->offset(field) << "), expression: " << form->expression << endl;

                {{#EXACT_SOURCE}}
                if ((Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
                {
                    std::map<dealii::types::global_dof_index,double> boundary_values;
                    dealii::VectorTools::interpolate_boundary_values (*m_doFHandler, i+1, Deal_{{FUNCTION_NAME}}<2>(boundary, m_fieldInfo->numberOfSolutions()), boundary_values);
                    dealii::MatrixTools::apply_boundary_values (boundary_values, system_matrix, *m_solution, system_rhs);
                }
                {{/EXACT_SOURCE}}
            }
        }
    }
}

// *************************************************************************************************************************************************

// quantities in volume weak forms:
{{#QUANTITY_INFO}}//{{QUANT_ID}} = ext[{{INDEX}} + {{OFFSET}}]
{{/QUANTITY_INFO}}

{{#VOLUME_MATRIX_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, const WeakFormAgros<double>* wfAgros)
: MatrixFormVolAgros<Scalar>(i, j, wfAgros)
{       
}


template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                        Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::GeomVol<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                           Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::GeomVol<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    //return new {{FUNCTION_NAME}}(this->i, this->j, this->m_offsetI, this->m_offsetJ);
    return new {{FUNCTION_NAME}}(*this);
}

{{/VOLUME_MATRIX_SOURCE}}

// ***********************************************************************************************************************************

{{#VOLUME_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, const WeakFormAgros<double>* wfAgros)
: VectorFormVolAgros<Scalar>(i, wfAgros), j(j)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                        Hermes::Hermes2D::GeomVol<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                           Hermes::Hermes2D::GeomVol<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    //return new {{FUNCTION_NAME}}(this->i, this->j, this->m_offsetI, this->m_offsetJ, this->m_offsetPreviousTimeExt, this->m_offsetCouplingExt);
    return new {{FUNCTION_NAME}}(*this);
}

{{/VOLUME_VECTOR_SOURCE}}

// ***********************************************************************************************************************************

{{#SURFACE_MATRIX_SOURCE}}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, const WeakFormAgros<double>* wfAgros)
: MatrixFormSurfAgros<Scalar>(i, j, wfAgros)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u, Hermes::Hermes2D::Func<double> *v,
                                        Hermes::Hermes2D::GeomSurf<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u, Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                           Hermes::Hermes2D::GeomSurf<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;

}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    //return new {{FUNCTION_NAME}}(this->i, this->j, this->m_offsetI, this->m_offsetJ);
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerTarget(const Marker *marker)
{
    FormAgrosInterface<Scalar>::setMarkerTarget(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = this->m_markerTarget->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}
{{/SURFACE_MATRIX_SOURCE}}

// ***********************************************************************************************************************************

{{#SURFACE_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, const WeakFormAgros<double>* wfAgros)

: VectorFormSurfAgros<Scalar>(i, wfAgros), j(j)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                        Hermes::Hermes2D::GeomSurf<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                           Hermes::Hermes2D::GeomSurf<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);
    Offset offset = this->m_wfAgros->offsetInfo(this->m_markerSource, this->m_markerTarget);
    for (int i = 0; i < n; i++)
    {
        // result += wt[i] * ({{EXPRESSION}});
    }
    return result;

}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    //return new {{FUNCTION_NAME}}(this->i, this->j, this->m_offsetI, this->m_offsetJ);
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerTarget(const Marker *marker)
{
    FormAgrosInterface<Scalar>::setMarkerTarget(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = this->m_markerTarget->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}
{{/SURFACE_VECTOR_SOURCE}}

// ***********************************************************************************************************************************

/*
{{#SOURCE}}template class {{FUNCTION_NAME}}<double>;
{{/SOURCE}}
*/
