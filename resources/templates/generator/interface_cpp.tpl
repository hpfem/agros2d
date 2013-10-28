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
#include "{{ID}}_errorcalculator.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "../../resources_source/classes/module_xml.h"

static XMLModule::module *module_module = NULL;

{{CLASS}}Interface::{{CLASS}}Interface() : PluginInterface()
{    
    // xml module description
    if (!module_module)
    {
        std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir() + MODULEROOT + QDir::separator() + "{{ID}}.xml").toStdString(),
                                                                         xml_schema::flags::dont_validate & xml_schema::flags::dont_initialize);
        module_module = module_xsd.release();
    }
    m_module = module_module;
}

{{CLASS}}Interface::~{{CLASS}}Interface()
{
    // delete m_module;
}

AgrosExtFunction *{{CLASS}}Interface::extFunction(const ProblemID problemId, QString id, bool derivative, int offsetI)
{
    {{#EXT_FUNCTION}}
    if((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) && (id == "{{QUANTITY_ID}}") && (derivative == {{IS_DERIVATIVE}}))
        return new {{EXT_FUNCTION_NAME}}(Agros2D::problem()->fieldInfo(problemId.sourceFieldId), offsetI);
    {{/EXT_FUNCTION}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    if((id == "{{SPECIAL_FUNCTION_ID}}") && (problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}))
    {
        assert(derivative == false);
        AgrosExtFunction* extFunction = new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(Agros2D::problem()->fieldInfo(problemId.sourceFieldId), offsetI);
        return extFunction;
    }
    {{/SPECIAL_FUNCTION_SOURCE}}

    return NULL;
}


MatrixFormVolAgros<double> *{{CLASS}}Interface::matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material)
{
       {{#VOLUME_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
            (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}}))
        if ({{EXPRESSION_CHECK}})
            return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ);
    {{/VOLUME_MATRIX_SOURCE}}

    return NULL;
}

VectorFormVolAgros<double> *{{CLASS}}Interface::vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material, int *offsetPreviousTimeExt, int *offsetCouplingExt)
{
    {{#VOLUME_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}})))
        if ({{EXPRESSION_CHECK}})
            return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ, offsetPreviousTimeExt, offsetCouplingExt);
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

ExactSolutionScalarAgros<double> *{{CLASS}}Interface::exactSolution(const ProblemID problemId, FormInfo *form, Hermes::Hermes2D::MeshSharedPtr mesh)
{
    {{#EXACT_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.analysisTypeSource == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}})))
        return new {{FUNCTION_NAME}}<double>(mesh);
    {{/EXACT_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::ErrorCalculator<double> *{{CLASS}}Interface::errorCalculator(FieldInfo *fieldInfo,
                                                                               const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType)
{
    return new {{CLASS}}ErrorCalculator<double>(fieldInfo, calculator, errorType);
}

Hermes::Hermes2D::MeshFunctionSharedPtr<double> {{CLASS}}Interface::filter(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                     Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp)
{
    return Hermes::Hermes2D::MeshFunctionSharedPtr<double>(new {{CLASS}}ViewScalarFilter(fieldInfo, timeStep, adaptivityStep, solutionType, sln, variable, physicFieldVariableComp));
}

LocalValue *{{CLASS}}Interface::localValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point)
{
    return new {{CLASS}}LocalValue(fieldInfo, timeStep, adaptivityStep, solutionType, point);
}

IntegralValue *{{CLASS}}Interface::surfaceIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
{
    return new {{CLASS}}SurfaceIntegral(fieldInfo, timeStep, adaptivityStep, solutionType);
}

IntegralValue *{{CLASS}}Interface::volumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
{
    return new {{CLASS}}VolumeIntegral(fieldInfo, timeStep, adaptivityStep, solutionType);
}

Point3 {{CLASS}}Interface::force(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                 Hermes::Hermes2D::Element *element, SceneMaterial *material,
                                 const Point3 &point, const Point3 &velocity)
{
    return force{{CLASS}}(fieldInfo, timeStep, adaptivityStep, solutionType, element, material, point, velocity);
}

bool {{CLASS}}Interface::hasForce(FieldInfo *fieldInfo)
{
    return hasForce{{CLASS}}(fieldInfo);
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

// ***********************************************************************************************************************************

{{#SPECIAL_FUNCTION_SOURCE}}
{{SPECIAL_EXT_FUNCTION_FULL_NAME}}::{{SPECIAL_EXT_FUNCTION_FULL_NAME}}(FieldInfo* fieldInfo, int offsetI) : AgrosSpecialExtFunction(fieldInfo, offsetI, specialFunctionTypeFromStringKey("{{TYPE}}"), {{INTERPOLATION_COUNT}})
{
    m_boundLow = {{FROM}};
    m_boundHi = {{TO}};
    m_variant = "{{SELECTED_VARIANT}}";

    createConversion();

{{#PARAMETERS}}    {{PARAMETER_NAME}}_pointers = createValuePointers("{{PARAMETER_ID}}");
{{/PARAMETERS}}

    init();
}

{{SPECIAL_EXT_FUNCTION_FULL_NAME}}::~{{SPECIAL_EXT_FUNCTION_FULL_NAME}}()
{
{{#PARAMETERS}}    delete[] {{PARAMETER_NAME}}_pointers;
{{/PARAMETERS}}

}

double {{SPECIAL_EXT_FUNCTION_FULL_NAME}}::calculateValue(int hermesMarker, double h) const
{
{{#PARAMETERS}}    double {{PARAMETER_NAME}} = {{PARAMETER_NAME}}_pointers[m_conversion[hermesMarker]]->numberFromTable(h);
{{/PARAMETERS}}
    double area = m_fieldInfo->initialMesh()->get_marker_area(hermesMarker);
    if(m_useTable)
    {
        if(h < m_boundLow)
            return {{EXTRAPOLATE_LOW}};
        if(h > m_boundHi)
            return {{EXTRAPOLATE_HI}};
    }

    //if(0)
    //{}
    //{{#VARIANT}}else if (this->m_variant == QString("{{ID}}"))
        return {{EXPR}};{{/VARIANT}}
    assert(0);
}

void {{SPECIAL_EXT_FUNCTION_FULL_NAME}}::value(int n, Hermes::Hermes2D::Func<double> **u_ext, Hermes::Hermes2D::Func<double> *result, Hermes::Hermes2D::Geom<double> *e) const
{
    for(int i = 0; i < n; i++)
    {
        result->val[i] = getValue(e->elem_marker, {{DEPENDENCE}});
    }
}

{{/SPECIAL_FUNCTION_SOURCE}}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros2d_plugin_{{ID}}, {{CLASS}}Interface)
#endif
