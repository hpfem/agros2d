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
#include "{{ID}}_surfaceintegral.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

#include <deal.II/numerics/vector_tools.h>

/*
const double internal_coeff = 0.5;

class {{CLASS}}SurfaceIntegralCalculator : public Hermes::Hermes2D::PostProcessing::SurfaceIntegralCalculator<double>
{
public:
    {{CLASS}}SurfaceIntegralCalculator(const FieldInfo *fieldInfo, Hermes::Hermes2D::MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
        : Hermes::Hermes2D::PostProcessing::SurfaceIntegralCalculator<double>(source_function, number_of_integrals), m_fieldInfo(fieldInfo)
    {
    }

    {{CLASS}}SurfaceIntegralCalculator(const FieldInfo *fieldInfo, std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
        : Hermes::Hermes2D::PostProcessing::SurfaceIntegralCalculator<double>(source_functions, number_of_integrals), m_fieldInfo(fieldInfo)
    {
    }

    virtual void integral(int n, double* wt, Hermes::Hermes2D::Func<double> **fns, Hermes::Hermes2D::GeomSurf<double> *e, double* result)
    {
        int labelIdx = atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str());
        int edgeIdx = atoi(m_fieldInfo->initialMesh()->get_boundary_markers_conversion().get_user_marker(e->edge_marker).marker.c_str());

        // this was an attempt to allow proper integration of surface "flow-like" variables
        // at the present moment, we integrate twice on each inner edge and the results are added, that
        // is why there is internal_coeff = 0.5. The following three lines attempted to calculate each
        // of such integrals only once, thus internal_coeff would be = 1. It is, however, very hard
        // to determin, what is the inner or outer normal for inner edges, especially for coupled problems,
        // where each field may have holes in diffrent places, etc.

        // previous attempt
        //int labelIdxToIntegrate = Agros2D::scene()->edges->at(edgeIdx)->innerLabelIdx(m_fieldInfo);
        //if(labelIdx != labelIdxToIntegrate)
        //    return;

        // new idea: It might be possible to do it this way. We would distinguish two types of surface
        // integral quantities, ordinary and flow-like. For the first group, nothing would change. For
        // the second, we would use different threatment. They would be visible in the list in two cases
        // only:
        // 1) if only one edge is selected : than we can calculate the value correctly, but the
        //    sign would be random (although we can indicate the "outer" normal graphicaly in the postprocessor)
        //    and use convention, that, say, positive is in direction out and negative in.
        // 2) if selected edges form a circle in whose interior all labes support given field. Than it is
        //    clear what is interior and what is exterior and the value can be calculated
        // In all other cases, integral of flow-like values would not be defined / visible in the list

        SceneMaterial *material = Agros2D::scene()->labels->at(labelIdx)->marker(m_fieldInfo);

        double *x = e->x;
        double *y = e->y;

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}

        double **value = new double*[source_functions.size()];
        double **dudx = new double*[source_functions.size()];
        double **dudy = new double*[source_functions.size()];

        for (int i = 0; i < source_functions.size(); i++)
        {
            value[i] = fns[i]->val;
            dudx[i] = fns[i]->dx;
            dudy[i] = fns[i]->dy;
        }

        // expressions
        {{#VARIABLE_SOURCE}}
        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            for (int i = 0; i < n; i++)
                result[{{POSITION}}] += wt[i] * ({{EXPRESSION}});
        }
        {{/VARIABLE_SOURCE}}

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }

    virtual void order(Hermes::Hermes2D::Func<Hermes::Ord> **fns, Hermes::Ord* result)
    {
        {{#VARIABLE_SOURCE}}
        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
            result[{{POSITION}}] = Hermes::Ord(20);
        {{/VARIABLE_SOURCE}}
    }

private:
    // field info
    const FieldInfo *m_fieldInfo;
};
*/

{{CLASS}}SurfaceIntegral::{{CLASS}}SurfaceIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
    : IntegralValue(fieldInfo, timeStep, adaptivityStep, solutionType)
{
    calculate();
}

void {{CLASS}}SurfaceIntegral::calculate()
{
    int numberOfSolutions = m_fieldInfo->numberOfSolutions();

    m_values.clear();

    if (Agros2D::problem()->isSolved())
    {
        FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
        // check existence
        if (!Agros2D::solutionStore()->contains(fsid))
            return;

        MultiArrayDeal ma = Agros2D::solutionStore()->multiArrayDeal(fsid);

        // update time functions
        if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            Module::updateTimeFunctions(Agros2D::problem()->timeStepToTotalTime(m_timeStep));
        }

        dealii::QGauss<2-1> face_quadrature_formula_int(5);
        const unsigned int n_face_q_points = face_quadrature_formula_int.size();

        dealii::FEFaceValues<2> fe_face_values_int(ma.doFHandler()->get_fe(), face_quadrature_formula_int, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values);

        std::vector<dealii::Vector<double> > solution_values(n_face_q_points, dealii::Vector<double>(1));
        std::vector<std::vector<dealii::Tensor<1,2> > > solution_grads(n_face_q_points, std::vector<dealii::Tensor<1,2> > (1));

        double *x = new double[n_face_q_points];
        double *y = new double[n_face_q_points];

        for (int iFace = 0; iFace < Agros2D::scene()->edges->count(); iFace++)
        {
            SceneEdge *edge = Agros2D::scene()->edges->at(iFace);
            if (!edge->isSelected())
                continue;

            // Then start the loop over all cells, and select those cells which are close enough to the evaluation point:
            dealii::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler()->begin_active(), endc_int = ma.doFHandler()->end();
            for (; cell_int != endc_int; ++cell_int)
            {
                SceneLabel *label = Agros2D::scene()->labels->at(cell_int->material_id() - 1);
                SceneMaterial *material = label->marker(m_fieldInfo);

                {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
                {{/VARIABLE_MATERIAL}}

                // surface integration
                for (unsigned int face=0; face<dealii::GeometryInfo<2>::faces_per_cell; ++face)
                {
                    if (cell_int->face(face)->at_boundary() && cell_int->face(face)->boundary_indicator() - 1 == iFace)
                    {
                        fe_face_values_int.reinit (cell_int, face);
                        fe_face_values_int.get_function_values(*ma.solution(), solution_values);
                        fe_face_values_int.get_function_gradients(*ma.solution(), solution_grads);

                        {{#VARIABLE_SOURCE}}
                        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                        {
                            for (unsigned int i = 0; i < n_face_q_points; ++i)
                            {
                                dealii::Point<2> normal = fe_face_values_int.normal_vector(i);

                                m_values[QLatin1String("{{VARIABLE}}")] += fe_face_values_int.JxW(i) * ({{EXPRESSION}});
                            }
                        }
                        {{/VARIABLE_SOURCE}}
                    }
                }
            }
        }
    }
}
