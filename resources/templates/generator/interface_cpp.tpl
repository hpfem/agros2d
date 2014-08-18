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
#include "{{ID}}_extfunction.h"
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
        try
        {
            std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir() + MODULEROOT + QDir::separator() + "{{ID}}.xml").toStdString(),
                                                                             xml_schema::flags::dont_validate & xml_schema::flags::dont_initialize);
            module_module = module_xsd.release();
        }
        catch (const xml_schema::expected_element& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
            qDebug() << str;
        }
        catch (const xml_schema::expected_attribute& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
            qDebug() << str;
        }
        catch (const xml_schema::unexpected_element& e)
        {
            QString str = QString("%1: %2 instead of %3").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.encountered_name())).arg(QString::fromStdString(e.expected_name()));
            qDebug() << str;
        }
        catch (const xml_schema::unexpected_enumerator& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.enumerator()));
            qDebug() << str;
        }
        catch (const xml_schema::expected_text_content& e)
        {
            QString str = QString("%1").arg(QString::fromStdString(e.what()));
            qDebug() << str;
        }
        catch (const xml_schema::parsing& e)
        {
            QString str = QString("%1").arg(QString::fromStdString(e.what()));
            qDebug() << str;
            xml_schema::diagnostics diagnostic = e.diagnostics();
            for(int i = 0; i < diagnostic.size(); i++)
            {
                xml_schema::error err = diagnostic.at(i);
                qDebug() << QString("%1, position %2:%3, %4").arg(QString::fromStdString(err.id())).arg(err.line()).arg(err.column()).arg(QString::fromStdString(err.message()));
            }
        }
        catch (const xml_schema::exception& e)
        {
            qDebug() << QString::fromStdString(e.what());
        }
    }
    m_module = &module_module->field().get();
}

{{CLASS}}Interface::~{{CLASS}}Interface()
{
}

AgrosExtFunction *{{CLASS}}Interface::extFunction(const ProblemID problemId, QString id, bool derivative, bool linearized, const WeakFormAgros<double>* wfAgros)
{
    {{#EXT_FUNCTIONS_PART}}
    if((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeTarget == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}))
    {
        return this->{{PART_NAME}}(problemId, id, derivative, linearized, wfAgros);
    }
    {{/EXT_FUNCTIONS_PART}}
}

{{#EXT_FUNCTIONS_PART}}
AgrosExtFunction *{{CLASS}}Interface::{{PART_NAME}}(const ProblemID problemId, QString id, bool derivative, bool linearized, const WeakFormAgros<double>* wfAgros)
{
    {{#EXT_FUNCTION}}
    if((id == "{{QUANTITY_ID}}") && (derivative == {{IS_DERIVATIVE}}) && (linearized == {{IS_LINEARIZED}}))
        return new {{EXT_FUNCTION_NAME}}(Agros2D::problem()->fieldInfo(problemId.targetFieldId), wfAgros);
    {{/EXT_FUNCTION}}
    {{#VALUE_FUNCTION_SOURCE}}
    if((id == "{{VALUE_FUNCTION_ID}}") && (linearized == {{IS_LINEARIZED}}))
        return new {{VALUE_FUNCTION_FULL_NAME}}(Agros2D::problem()->fieldInfo(problemId.targetFieldId), wfAgros);
    {{/VALUE_FUNCTION_SOURCE}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    if(id == "{{SPECIAL_FUNCTION_ID}}")
        return new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(Agros2D::problem()->fieldInfo(problemId.targetFieldId), wfAgros);
    {{/SPECIAL_FUNCTION_SOURCE}}
    return NULL;
}
{{/EXT_FUNCTIONS_PART}}

MatrixFormVolAgros<double> *{{CLASS}}Interface::matrixFormVol(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Material *material)
{
       {{#VOLUME_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeTarget == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
            (form->id == "{{WEAKFORM_ID}}"))
        if ({{EXPRESSION_CHECK}})
            return new {{FUNCTION_NAME}}<double>(form->i - 1, form->j - 1, wfAgros);
    {{/VOLUME_MATRIX_SOURCE}}

    return NULL;
}

VectorFormVolAgros<double> *{{CLASS}}Interface::vectorFormVol(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Material *material)
{
    {{#VOLUME_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeTarget == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}"))
        if ({{EXPRESSION_CHECK}})
            return new {{FUNCTION_NAME}}<double>(form->i - 1 , form->j - 1, wfAgros);
    {{/VOLUME_VECTOR_SOURCE}}

    return NULL;
}

MatrixFormSurfAgros<double> *{{CLASS}}Interface::matrixFormSurf(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Boundary *boundary)
{
    {{#SURFACE_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeTarget == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (boundary->type() == "{{BOUNDARY_TYPE}}"))
        return new {{FUNCTION_NAME}}<double>(form->i - 1, form->j - 1, wfAgros);
    {{/SURFACE_MATRIX_SOURCE}}

    return NULL;
}

VectorFormSurfAgros<double> *{{CLASS}}Interface::vectorFormSurf(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Boundary *boundary)
{
    {{#SURFACE_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeTarget == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}") && (boundary->type() == "{{BOUNDARY_TYPE}}"))
        return new {{FUNCTION_NAME}}<double>(form->i - 1, form->j - 1, wfAgros);
    {{/SURFACE_VECTOR_SOURCE}}

    return NULL;
}

ExactSolutionScalarAgros<double> *{{CLASS}}Interface::exactSolution(const ProblemID problemId, FormInfo *form, Hermes::Hermes2D::MeshSharedPtr mesh)
{
    {{#EXACT_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeTarget == {{ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
         (form->id == "{{WEAKFORM_ID}}"))
        return new {{FUNCTION_NAME}}<double>(mesh);
    {{/EXACT_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::ErrorCalculator<double> *{{CLASS}}Interface::errorCalculator(const FieldInfo *fieldInfo,
                                                                               const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType)
{
    return new {{CLASS}}ErrorCalculator<double>(fieldInfo, calculator, errorType);
}

Hermes::Hermes2D::MeshFunctionSharedPtr<double> {{CLASS}}Interface::filter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                     std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp)
{
    return Hermes::Hermes2D::MeshFunctionSharedPtr<double>(new {{CLASS}}ViewScalarFilter(fieldInfo, timeStep, adaptivityStep, solutionType, sln, variable, physicFieldVariableComp));
}

LocalValue *{{CLASS}}Interface::localValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point)
{
    return new {{CLASS}}LocalValue(fieldInfo, timeStep, adaptivityStep, solutionType, point);
}

IntegralValue *{{CLASS}}Interface::surfaceIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
{
    return new {{CLASS}}SurfaceIntegral(fieldInfo, timeStep, adaptivityStep, solutionType);
}

IntegralValue *{{CLASS}}Interface::volumeIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
{
    return new {{CLASS}}VolumeIntegral(fieldInfo, timeStep, adaptivityStep, solutionType);
}

Point3 {{CLASS}}Interface::force(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                 Hermes::Hermes2D::Element *element, SceneMaterial *material,
                                 const Point3 &point, const Point3 &velocity)
{
    return force{{CLASS}}(fieldInfo, timeStep, adaptivityStep, solutionType, element, material, point, velocity);
}

bool {{CLASS}}Interface::hasForce(const FieldInfo *fieldInfo)
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


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros2d_plugin_{{ID}}, {{CLASS}}Interface)
#endif
