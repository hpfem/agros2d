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

#include "{{ID}}_force.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/block.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include "util.h"
#include "scene.h"
#include "scenelabel.h"
#include "logview.h"

#include "hermes2d/plugin_interface.h"

Point3 force{{CLASS}}(FieldInfo *fieldInfo, const SceneMaterial *material, const Point3 &point, const Point3 &velocity)
{
    Point3 res;

    if (Util::problem()->isSolved())
    {
        // update time functions
        if (fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());
            fieldInfo->module()->updateTimeFunctions(timeLevels[Util::scene()->activeTimeStep()]);
        }

        // set variables
        double x = point.x;
        double y = point.y;

        double *value = new double[fieldInfo->module()->numberOfSolutions()];
        double *dudx = new double[fieldInfo->module()->numberOfSolutions()];
        double *dudy = new double[fieldInfo->module()->numberOfSolutions()];

        std::vector<Hermes::Hermes2D::Solution<double> *> sln(fieldInfo->module()->numberOfSolutions());
        for (int k = 0; k < fieldInfo->module()->numberOfSolutions(); k++)
        {
            // todo: do it better! - I could use reference solution. This way I ignore selected active adaptivity step and solution mode
            FieldSolutionID fsid(fieldInfo, Util::scene()->activeTimeStep(), Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal, Util::scene()->activeTimeStep()), SolutionMode_Normal);
            sln[k] = Util::solutionStore()->multiSolution(fsid).component(k).sln.data();

            // point values
            Hermes::Hermes2D::Func<double> *values = sln[k]->get_pt_value(point.x, point.y);
            double val;
            if ((fieldInfo->analysisType() == AnalysisType_Transient) && Util::scene()->activeTimeStep() == 0)
                // const solution at first time step
                val = fieldInfo->initialCondition().number();
            else
                val = values->val[0];

            // set variables
            value[k] = val;
            dudx[k] = values->dx[0];
            dudy[k] = values->dy[0];
        }

        {{#VARIABLE_SOURCE}}
        if ((fieldInfo->module()->analysisType() == {{ANALYSIS_TYPE}})
         && (fieldInfo->module()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            res.x = {{EXPRESSION_X}};
            res.y = {{EXPRESSION_Y}};
            res.z = {{EXPRESSION_Z}};
        }
        {{/VARIABLE_SOURCE}}

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }

    return res;
}
