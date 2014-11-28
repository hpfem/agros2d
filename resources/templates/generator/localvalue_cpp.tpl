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
#include "{{ID}}_localvalue.h"
#include "{{ID}}_interface.h"

#include "util.h"
#include "util/global.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>


{{CLASS}}LocalValue::{{CLASS}}LocalValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                         const Point &point)
    : LocalValue(fieldInfo, timeStep, adaptivityStep, solutionType, point)
{
    calculate();
}

void {{CLASS}}LocalValue::calculate()
{
    int numberOfSolutions = m_fieldInfo->numberOfSolutions();

    m_values.clear();

    if (Agros2D::problem()->isSolved())
    {
        try
        {
            FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
            // check existence
            if (!Agros2D::solutionStore()->contains(fsid))
                return;

            MultiArray ma = Agros2D::solutionStore()->multiArray(fsid);

            // update time functions
            if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
            {
                Module::updateTimeFunctions(Agros2D::problem()->timeStepToTotalTime(m_timeStep));
            }

            dealii::Point<2> p(m_point.x, m_point.y);
            std::pair<typename dealii::Triangulation<2>::active_cell_iterator, dealii::Point<2> > current_cell =
                    dealii::GridTools::find_active_cell_around_point(dealii::MappingQ1<2>(), *m_fieldInfo->initialMesh(), p);

            // find marker
            SceneLabel *label = Agros2D::scene()->labels->at(current_cell.first->material_id() - 1);
            SceneMaterial *material = label->marker(m_fieldInfo);

            {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
            {{/VARIABLE_MATERIAL}}

            double *solution_values = new double[numberOfSolutions];
            dealii::Tensor<1, 2> *solution_grads = new dealii::Tensor<1, 2>[numberOfSolutions];

            for (int k = 0; k < numberOfSolutions; k++)
            {
                if ((m_fieldInfo->analysisType() == AnalysisType_Transient) && m_timeStep == 0)
                {
                    // set variables
                    solution_values[k] = m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble();
                    solution_grads[k][0] = 0;
                    solution_grads[k][1] = 0;
                }
                else
                {
                    // point values
                    dealii::Functions::FEFieldFunction<2> localvalues(*ma.doFHandler(), *ma.solution());

                    // set variables
                    solution_values[k] = localvalues.value(p);
                    solution_grads[k] = localvalues.gradient(p);
                }
            }

            // expressions
            {{#VARIABLE_SOURCE}}
            if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                    && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                m_values[QLatin1String("{{VARIABLE}}")] = LocalPointValue({{EXPRESSION_SCALAR}}, Point({{EXPRESSION_VECTORX}}, {{EXPRESSION_VECTORY}}), material);
            {{/VARIABLE_SOURCE}}

            delete [] solution_values;
            delete [] solution_grads;
        }
        catch (const typename dealii::GridTools::ExcPointNotFound<2> &e)
        {
            // qDebug() << e.what();
        }
    }
}
