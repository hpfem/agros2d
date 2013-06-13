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

#include "{{ID}}_localvalue.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

{{CLASS}}LocalValue::{{CLASS}}LocalValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                         const Point &point)
    : LocalValue(fieldInfo, timeStep, adaptivityStep, solutionType, point)
{
    calculate();
}

void {{CLASS}}LocalValue::calculate()
{
    int numberOfSolutions = m_fieldInfo->numberOfSolutions();

    m_values.clear();

    FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
       Module::updateTimeFunctions(Agros2D::problem()->timeStepToTotalTime(m_timeStep));
    }

    if (Agros2D::problem()->isSolved())
    {
        double x = m_point.x;
        double y = m_point.y;

        Hermes::Hermes2D::Element *e = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(false, m_fieldInfo->initialMesh(),
                                                                                                 m_point.x, m_point.y);
        if (e)
        {
            // find marker
            SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()));
            SceneMaterial *material = label->marker(m_fieldInfo);

            {{#VARIABLE_MATERIAL}}Value *material_{{MATERIAL_VARIABLE}} = &material->value("{{MATERIAL_VARIABLE}}");
            {{/VARIABLE_MATERIAL}}

            double *value = new double[numberOfSolutions];
            double *dudx = new double[numberOfSolutions];
            double *dudy = new double[numberOfSolutions];

            for (int k = 0; k < numberOfSolutions; k++)
            {
                if ((m_fieldInfo->analysisType() == AnalysisType_Transient) && m_timeStep == 0)
                {

                    // set variables
                    value[k] = m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble();
                    dudx[k] = 0;
                    dudy[k] = 0;
                }
                else
                {
                    // point values
                    Hermes::Hermes2D::Func<double> *values = ma.solutions().at(k)->get_pt_value(m_point.x, m_point.y, true);

                    // set variables
                    value[k] = values->val[0];
                    dudx[k] = values->dx[0];
                    dudy[k] = values->dy[0];

                    values->free_fn();
                    values->free_ord();
                    delete values;
                }
            }

            // expressions
            {{#VARIABLE_SOURCE}}
            if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                    && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                m_values["{{VARIABLE}}"] = PointValue({{EXPRESSION_SCALAR}}, Point({{EXPRESSION_VECTORX}}, {{EXPRESSION_VECTORY}}), material);
            {{/VARIABLE_SOURCE}}

            delete [] value;
            delete [] dudx;
            delete [] dudy;
        }
    }
}
