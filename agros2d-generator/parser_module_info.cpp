#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "generator.h"
#include "parser_module_info.h"


ParserModuleInfo::ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType) :
    analysisType(analysisType), coordinateType(coordinateType), linearityType(linearityType),
    constants(field.constants()), volume(field.volume()), surface(field.surface())
{
    numSolutions = Agros2DGenerator::numberOfSolutions(field.general_field().analyses(), analysisType);
    id = QString::fromStdString(field.general_field().id());

    Module::volumeQuantityProperties(&field, quantityOrdering, quantityIsNonlinear, functionOrdering);
}

QString ParserModuleInfo::nonlinearExpressionVolume(const QString &variable) const
{
    foreach (XMLModule::weakform_volume wf, volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (coordinateType == CoordinateType_Planar)
                    {
                        if (quantityAnalysis.nonlinearity_planar().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                    }
                    else
                    {
                        if (quantityAnalysis.nonlinearity_axi().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                    }
                }
            }
        }
    }

    return "";
}

QString ParserModuleInfo::nonlinearExpressionSurface(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_surface wf, surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variableString)
                    {
                        if (coordinateType == CoordinateType_Planar)
                        {
                            if (quantityAnalysis.nonlinearity_planar().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                        }
                        else
                        {
                            if (quantityAnalysis.nonlinearity_axi().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                        }
                    }
                }
            }
        }
    }

    return "";
}

QString ParserModuleInfo::specialFunctionNonlinearExpression(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_volume wf, volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::function_use functionUse, wf.function_use())
            {
                if (functionUse.id() == variableString)
                {
                    if (coordinateType == CoordinateType_Planar)
                    {
                        if (functionUse.nonlinearity_planar().present())
                            return QString::fromStdString(functionUse.nonlinearity_planar().get());
                    }
                    else
                    {
                        if (functionUse.nonlinearity_axi().present())
                            return QString::fromStdString(functionUse.nonlinearity_axi().get());
                    }
                }
            }
        }
    }

    return "0";
}

QString ParserModuleInfo::dependenceVolume(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_volume wf, volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variableString)
                {
                    if (quantityAnalysis.dependence().present())
                        return QString::fromStdString(quantityAnalysis.dependence().get());
                }
            }
        }
    }

    return "";
}

QString ParserModuleInfo::dependenceSurface(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_surface wf, surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variableString)
                    {
                        if (quantityAnalysis.dependence().present())
                            return QString::fromStdString(quantityAnalysis.dependence().get());
                    }
                }
            }
        }
    }

    return "";
}

