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

#include "{{ID}}_volumeintegral.h"
#include "{{ID}}_interface.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

class {{CLASS}}VolumetricIntegralEggShellCalculator : public Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>
{
public:
{{CLASS}}VolumetricIntegralEggShellCalculator(FieldInfo *fieldInfo, Hermes::Hermes2D::MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
    : Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>(source_function, number_of_integrals), m_fieldInfo(fieldInfo)
{
}

{{CLASS}}VolumetricIntegralEggShellCalculator(FieldInfo *fieldInfo, Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
    : Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>(source_functions, number_of_integrals), m_fieldInfo(fieldInfo)
{
}

virtual void integral(int n, double* wt, Hermes::Hermes2D::Func<double> **fns, Hermes::Hermes2D::Geom<double> *e, double* result)
{
    SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str()));
    SceneMaterial *material = label->marker(m_fieldInfo);

    double *x = e->x;
    double *y = e->y;

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    {{SPECIAL_EXT_FUNCTION_FULL_NAME}} {{SPECIAL_FUNCTION_NAME}}(m_fieldInfo, 0);{{/SPECIAL_FUNCTION_SOURCE}}

    // functions
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
    {{#VARIABLE_SOURCE_EGGSHELL}}
    if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
    {
        for (int i = 0; i < n; i++)
            result[{{POSITION}}] += wt[i] * ({{EXPRESSION}});
    }
    {{/VARIABLE_SOURCE_EGGSHELL}}

    delete [] value;
    delete [] dudx;
    delete [] dudy;
}

virtual void order(Hermes::Hermes2D::Func<Hermes::Ord> **fns, Hermes::Ord* result)
{
    {{#VARIABLE_SOURCE_EGGSHELL}}
    if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        result[{{POSITION}}] = Hermes::Ord(20);
    {{/VARIABLE_SOURCE_EGGSHELL}}
}

private:
// field info
FieldInfo *m_fieldInfo;
};

class {{CLASS}}VolumetricIntegralCalculator : public Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>
{
public:
{{CLASS}}VolumetricIntegralCalculator(FieldInfo *fieldInfo, Hermes::Hermes2D::MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
    : Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>(source_function, number_of_integrals), m_fieldInfo(fieldInfo)
{
}

{{CLASS}}VolumetricIntegralCalculator(FieldInfo *fieldInfo, Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
    : Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>(source_functions, number_of_integrals), m_fieldInfo(fieldInfo)
{
}

virtual void integral(int n, double* wt, Hermes::Hermes2D::Func<double> **fns, Hermes::Hermes2D::Geom<double> *e, double* result)
{
    SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str()));
    SceneMaterial *material = label->marker(m_fieldInfo);

    double *x = e->x;
    double *y = e->y;
    int elementMarker = e->elem_marker;

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    {{SPECIAL_EXT_FUNCTION_FULL_NAME}} {{SPECIAL_FUNCTION_NAME}}(m_fieldInfo, 0);{{/SPECIAL_FUNCTION_SOURCE}}

    // functions
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
FieldInfo *m_fieldInfo;
};

{{CLASS}}VolumeIntegral::{{CLASS}}VolumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
    : IntegralValue(fieldInfo, timeStep, adaptivityStep, solutionType)
{
    calculate();
}

void {{CLASS}}VolumeIntegral::calculate()
{
    m_values.clear();

    FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(m_fieldInfo);
            Module::updateTimeFunctions(timeLevels[m_timeStep]);
        }

        Hermes::vector<std::string> markers;
        Hermes::vector<std::string> markersInverted;
        for (int i = 0; i < Agros2D::scene()->labels->count(); i++)
        {
            SceneLabel *label = Agros2D::scene()->labels->at(i);
            if (label->isSelected())
            {
                markers.push_back(QString::number(i).toStdString());
            }
            else
            {
                markersInverted.push_back(QString::number(i).toStdString());
            }
        }

        if (markers.size() > 0)
        {
            {{CLASS}}VolumetricIntegralCalculator calc(m_fieldInfo, ma.solutions(), {{INTEGRAL_COUNT}});
            double *values = calc.calculate(markers);

            {{#VARIABLE_SOURCE}}
            if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                m_values[QLatin1String("{{VARIABLE}}")] = values[{{POSITION}}];
            {{/VARIABLE_SOURCE}}

            ::free(values);
        }

        if ({{INTEGRAL_COUNT_EGGSHELL}} > 0 && markers.size() > 0 && markersInverted.size() > 0)
        {
            Hermes::Hermes2D::MeshSharedPtr eggShellMesh = Hermes::Hermes2D::EggShell::get_egg_shell(ma.solutions().at(0)->get_mesh(), markers, 3);
            Hermes::Hermes2D::MeshFunctionSharedPtr<double> eggShell(new Hermes::Hermes2D::ExactSolutionEggShell(eggShellMesh, 3));

            Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > slns;
            for (int i = 0; i < ma.solutions().size(); i++)
                slns.push_back(ma.solutions().at(i));
            slns.push_back(eggShell);

            {{CLASS}}VolumetricIntegralEggShellCalculator calcEggShell(m_fieldInfo, slns, {{INTEGRAL_COUNT_EGGSHELL}});
            double *valuesEggShell = calcEggShell.calculate(markersInverted);

            {{#VARIABLE_SOURCE_EGGSHELL}}
            if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                m_values[QLatin1String("{{VARIABLE}}")] = valuesEggShell[{{POSITION}}];
            {{/VARIABLE_SOURCE_EGGSHELL}}

            ::free(valuesEggShell);
        }
    }
}
