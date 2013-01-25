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

#include <QtPlugin>

#include "{{ID}}_interface.h"
#include "{{ID}}_weakform.h"
#include "{{ID}}_filter.h"
#include "{{ID}}_force.h"
#include "{{ID}}_localvalue.h"
#include "{{ID}}_surfaceintegral.h"
#include "{{ID}}_volumeintegral.h"

#include "util.h"
#include "util/global.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

MatrixFormVolAgros<double> *{{CLASS}}Interface::matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material)
{
       {{#VOLUME_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
            (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}}))
        if (fabs((double) ({{EXPRESSION_CHECK}})) > 0.0)
            return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ);
    {{/VOLUME_MATRIX_SOURCE}}

    return NULL;
}

VectorFormVolAgros<double> *{{CLASS}}Interface::vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material)
{
    {{#VOLUME_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}})))
        if (fabs((double) ({{EXPRESSION_CHECK}})) > 0.0)
            return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ);
    {{/VOLUME_VECTOR_SOURCE}}

    return NULL;
}

MatrixFormSurfAgros<double> *{{CLASS}}Interface::matrixFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary)
{
    {{#SURFACE_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}}) && (boundary->type() == "{{BOUNDARY_TYPE}}")))
        return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ);
    {{/SURFACE_MATRIX_SOURCE}}

    return NULL;
}

VectorFormSurfAgros<double> *{{CLASS}}Interface::vectorFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary)
{
    {{#SURFACE_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}}) && (boundary->type() == "{{BOUNDARY_TYPE}}")))
        return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ);
    {{/SURFACE_VECTOR_SOURCE}}

    return NULL;
}

ExactSolutionScalarAgros<double> *{{CLASS}}Interface::exactSolution(const ProblemID problemId, FormInfo *form, Hermes::Hermes2D::Mesh *mesh)
{
    {{#EXACT_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}})))
        return new {{FUNCTION_NAME}}<double>(mesh);
    {{/EXACT_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::Filter<double> *{{CLASS}}Interface::filter(FieldInfo *fieldInfo,
                                                     Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp)
{
    return new {{CLASS}}ViewScalarFilter(fieldInfo, sln, variable, physicFieldVariableComp);
}

LocalValue *{{CLASS}}Interface::localValue(FieldInfo *fieldInfo, const Point &point)
{
    return new {{CLASS}}LocalValue(fieldInfo, point);
}

IntegralValue *{{CLASS}}Interface::surfaceIntegral(FieldInfo *fieldInfo)
{
    return new {{CLASS}}SurfaceIntegral(fieldInfo);
}

IntegralValue *{{CLASS}}Interface::volumeIntegral(FieldInfo *fieldInfo)
{
    return new {{CLASS}}VolumeIntegral(fieldInfo);
}

Point3 {{CLASS}}Interface::force(FieldInfo *fieldInfo, const SceneMaterial *material, const Point3 &point, const Point3 &velocity)
{
    return force{{CLASS}}(fieldInfo, material, point, velocity);
}

QString {{CLASS}}Interface::localeName(const QString &name)
{
   {{#NAMES}}
   if (name == "{{NAME}}")
	return tr("{{NAME}}"); 
   {{/NAMES}}
   return name;	
}

QString {{CLASS}}Interface::localeDescription()
{
    return tr("{{DESCRIPTION}}");
}

Q_EXPORT_PLUGIN2(agros2d_plugin_{{ID}}, {{CLASS}}Interface)
