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

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/block.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "scenelabel.h"
#include "logview.h"

{{CLASS}}LocalValue::{{CLASS}}LocalValue(FieldInfo *fieldInfo, const Point &point)
    : LocalValue(fieldInfo, point)
{
    calculate();
}

void {{CLASS}}LocalValue::calculate()
{
    m_values.clear();

    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        m_fieldInfo->module()->updateTimeFunctions(Agros2D::problem()->timeStepToTime(Agros2D::scene()->activeTimeStep()));
    }

    if (Agros2D::problem()->isSolved())
    {
        double x = m_point.x;
        double y = m_point.y;

        Hermes::Hermes2D::Element *e = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(m_fieldInfo->initialMesh(),
                                                                                                 m_point.x, m_point.y);
        if (e)
        {
            // find marker
            SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()));
            SceneMaterial *material = label->marker(m_fieldInfo);

            double *value = new double[m_fieldInfo->module()->numberOfSolutions()];
            double *dudx = new double[m_fieldInfo->module()->numberOfSolutions()];
            double *dudy = new double[m_fieldInfo->module()->numberOfSolutions()];

            std::vector<Hermes::Hermes2D::Solution<double> *> sln(m_fieldInfo->module()->numberOfSolutions());
            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
            {
                int adaptivityStep, timeStep;
                SolutionMode solutionMode;
                if(m_fieldInfo == Agros2D::scene()->activeViewField())
                {
                    // if calculating values for active view field, use the solution that is viewed
                    timeStep = Agros2D::scene()->activeTimeStep();
                    adaptivityStep = Agros2D::scene()->activeAdaptivityStep();
                    solutionMode = Agros2D::scene()->activeSolutionType();
                }
                else
                {
                    // else use solution on nearest time step, last adaptivity step possible and if exists, reference solution
                    timeStep = Agros2D::solutionStore()->nearestTimeStep(m_fieldInfo, Agros2D::scene()->activeTimeStep());
                    adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(m_fieldInfo, SolutionMode_Normal, timeStep);
                    solutionMode = SolutionMode_Finer;
                }
                FieldSolutionID fsid(m_fieldInfo, timeStep, adaptivityStep, solutionMode);
                sln[k] = Agros2D::solutionStore()->multiArray(fsid).solutions().at(k);

                // point values
                Hermes::Hermes2D::Func<double> *values = sln[k]->get_pt_value(m_point.x, m_point.y);
                double val;
                if ((m_fieldInfo->analysisType() == AnalysisType_Transient) && Agros2D::scene()->activeTimeStep() == 0)
                    // const solution at first time step
                    val = m_fieldInfo->initialCondition().number();
                else
                    val = values->val[0];

                // set variables
                value[k] = val;
                dudx[k] = values->dx[0];
                dudy[k] = values->dy[0];
            }

            // expressions
            {{#VARIABLE_SOURCE}}
            if ((m_fieldInfo->module()->analysisType() == {{ANALYSIS_TYPE}})
                    && (m_fieldInfo->module()->coordinateType() == {{COORDINATE_TYPE}}))
                m_values[m_fieldInfo->module()->localVariable("{{VARIABLE}}")] = PointValue({{EXPRESSION_SCALAR}}, Point({{EXPRESSION_VECTORX}}, {{EXPRESSION_VECTORY}}), material);
            {{/VARIABLE_SOURCE}}

            delete [] value;
            delete [] dudx;
            delete [] dudy;
        }
    }
}
