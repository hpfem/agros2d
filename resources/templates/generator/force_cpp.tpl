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
#include "{{ID}}_force.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

bool hasForce{{CLASS}}(const FieldInfo *fieldInfo)
{
    {{#VARIABLE_SOURCE}}
    if ((fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
     && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
    {
        return true;
    }
    {{/VARIABLE_SOURCE}}

    return false;
}

Point3 force{{CLASS}}(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                      SceneMaterial *material, const Point3 &point, const Point3 &velocity)
{
    Point3 res;

    if (Agros2D::problem()->isSolved())
    {
        int numberOfSolutions = fieldInfo->numberOfSolutions();

        FieldSolutionID fsid(fieldInfo, timeStep, adaptivityStep, solutionType);
        MultiArray ma = Agros2D::solutionStore()->multiArray(fsid);

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}

        // update time functions
        if (fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(fieldInfo);
            Module::updateTimeFunctions(timeLevels[timeStep]);
        }

        // set variables
        double x = point.x;
        double y = point.y;
        dealii::Point<2> p(point.x, point.y);

        int k = 0; // only one point
        std::vector<dealii::Vector<double> > solution_values(1, dealii::Vector<double>(fieldInfo->numberOfSolutions()));
        std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(1, std::vector<dealii::Tensor<1,2> >(fieldInfo->numberOfSolutions()));

        for (int i = 0; i < numberOfSolutions; i++)
        {
            // point values
            try
            {
                if ((fieldInfo->analysisType() == AnalysisType_Transient) && timeStep == 0)
                {
                    // set variables
                    solution_values[k][i] = fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble();
                    solution_grads[k][i][0] = 0;
                    solution_grads[k][i][1] = 0;
                }
                else
                {
                    // point values
                    dealii::Functions::FEFieldFunction<2> localvalues(*ma.doFHandler(), *ma.solution());

                    // set variables
                    solution_values[k][i] = localvalues.value(p, i);
                    solution_grads[k][i] = localvalues.gradient(p, i);
                }
            }
            catch (const typename dealii::GridTools::ExcPointNotFound<2> &e)
            {
                throw AgrosException(QObject::tr("Point [%1, %2] does not lie in any element").arg(x).arg(y));

                return res;
            }
        }

        {{#VARIABLE_SOURCE}}
        if ((fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
         && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            res.x = {{EXPRESSION_X}};
            res.y = {{EXPRESSION_Y}};
            res.z = {{EXPRESSION_Z}};
        }
        {{/VARIABLE_SOURCE}}
    }

    return res;
}
