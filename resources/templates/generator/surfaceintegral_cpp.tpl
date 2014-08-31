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

{{CLASS}}SurfaceIntegral::{{CLASS}}SurfaceIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
    : IntegralValue(fieldInfo, timeStep, adaptivityStep, solutionType)
{
    calculate();
}

void {{CLASS}}SurfaceIntegral::calculate()
{
    m_values.clear();

    FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
    // check existence
    if (!Agros2D::solutionStore()->contains(fsid))
        return;

    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(m_fieldInfo);
            Module::updateTimeFunctions(timeLevels[m_timeStep]);
        }

        std::vector<std::string> boundaryMarkers;
        std::vector<std::string> internalMarkers;
        for (int i = 0; i < Agros2D::scene()->edges->count(); i++)
        {
            SceneEdge *edge = Agros2D::scene()->edges->at(i);
            if (edge->isSelected())
            {
                if (edge->marker(m_fieldInfo)->isNone())
                    internalMarkers.push_back(QString::number(i).toStdString());
                else
                    boundaryMarkers.push_back(QString::number(i).toStdString());
            }
        }

        if (internalMarkers.size() > 0 || boundaryMarkers.size() > 0)
        {
            {{CLASS}}SurfaceIntegralCalculator calc(m_fieldInfo, ma.solutions(), {{INTEGRAL_COUNT}});
            double *internalValues = calc.calculate(internalMarkers);
            double *boundaryValues = calc.calculate(boundaryMarkers);

            {{#VARIABLE_SOURCE}}
            if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                m_values[QLatin1String("{{VARIABLE}}")] = internal_coeff * internalValues[{{POSITION}}] + boundaryValues[{{POSITION}}];
            {{/VARIABLE_SOURCE}}

            ::free(internalValues);
            ::free(boundaryValues);
        }
    }
}
