#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"

class ValueGenerator
{
public:
    ValueGenerator(int initValue) : m_nextValue(initValue) {}
    QString value()
    {
        m_nextValue++;
        return QString::number(m_nextValue) + ".0"; // MSVC fix pow(int, double) doesn't work
    }

private:
    int m_nextValue;
};


ModuleParser::ModuleParser(XMLModule::field * field)
{
    Module::volumeQuantityProperties(field, m_quantityOrdering, m_quantityIsNonlinear, m_functionOrdering);
}

QString ModuleParser::parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool includeVariables, bool forFilter)
{
    try
    {
        QMap<QString, QString> dict;

        // coordinates
        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            dict["x"] = "x[i]";
            dict["y"] = "y[i]";
            // surface integral
            dict["tanx"] = "e->tx[i]";
            dict["tany"] = "e->ty[i]";
            // velocity (force calculation)
            dict["velx"] = "velocity.x";
            dict["vely"] = "velocity.y";
            dict["velz"] = "velocity.z";
        }
        else
        {
            dict["r"] = "x[i]";
            dict["z"] = "y[i]";
            // surface integral
            dict["tanr"] = "e->tx[i]";
            dict["tanz"] = "e->ty[i]";
            // velocity (force calculation)
            dict["velr"] = "velocity.x";
            dict["velz"] = "velocity.y";
            dict["velphi"] = "velocity.z";
        }

        // constants
        dict["PI"] = "M_PI";
        dict["f"] = "m_fieldInfo->frequency()";
        foreach (XMLModule::constant cnst, parserModuleInfo.m_constants.constant())
            dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

        // functions
        for (int i = 1; i < parserModuleInfo.m_numSolutions + 1; i++)
        {
            dict[QString("value%1").arg(i)] = QString("value[%1][i]").arg(i-1);
            if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
            {
                dict[QString("dx%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
                dict[QString("dy%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
            }
            else
            {
                dict[QString("dr%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
                dict[QString("dz%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
            }
        }
        // eggshell
        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            dict["dxegg"] = "dudx[source_functions.size() - 1][i]";
            dict["dyegg"] = "dudy[source_functions.size() - 1][i]";
        }
        else
        {
            dict["dregg"] = "dudx[source_functions.size() - 1][i]";
            dict["dzegg"] = "dudy[source_functions.size() - 1][i]";
        }

        // variables
        if (includeVariables)
        {
            foreach (XMLModule::quantity quantity, parserModuleInfo.m_volume.quantity())
            {
                if (quantity.shortname().present())
                {
                    QString nonlinearExpr = parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

                    if (nonlinearExpr.isEmpty())
                        // linear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
                    else
                        // nonlinear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                                arg(QString::fromStdString(quantity.id())).
                                arg(parsePostprocessorExpression(parserModuleInfo, nonlinearExpr, false));
                }
            }

            foreach (XMLModule::function function, parserModuleInfo.m_volume.function())
            {
                QString parameter("0");
                // todo: so far used only in Richards, where is OK
                if(QString::fromStdString(function.type()) == "function_1d")
                {
                    if(forFilter)
                        parameter = "value[0][i]";
                    else
                        parameter = "value[0]";
                }
                dict[QString::fromStdString(function.shortname())] = QString("%1->getValue(elementMarker, %2)").
                        arg(QString::fromStdString(function.shortname())).arg(parameter);
            }
        }

        QSharedPointer<LexicalAnalyser> lex = postprocessorLexicalAnalyser(parserModuleInfo);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in module %2").arg(e.toString()).arg(parserModuleInfo.m_id).toLatin1());

        return "";
    }
}



QString ModuleParser::parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserWeakForm parser(parserModuleInfo, this, withVariables);
    return parser.parse(expr);
}

QString ModuleParser::parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserErrorExpression parser(parserModuleInfo, this, withVariables);
    return parser.parse(expr);
}

QString ParserInstance::parse(QString expr)
{
    try
    {
        QSharedPointer<LexicalAnalyser> lex = m_moduleParser->weakFormLexicalAnalyser(m_parserModuleInfo);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(m_dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in module %2").arg(e.toString()).arg(m_parserModuleInfo.m_id).toLatin1());

        return "";
    }

}

QString ModuleParser::parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr)
{
    try
    {
        QMap<QString, QString> dict;
        // TODO: remove
        ValueGenerator generator(1);

        // variables
        foreach (XMLModule::quantity quantity, parserModuleInfo.m_volume.quantity())
        {
            if (quantity.shortname().present())
            {
                QString dep = parserModuleInfo.dependenceVolume(QString::fromStdString(quantity.id()));
                QString nonlinearExpr = parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

                if (parserModuleInfo.m_linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
                {
                    if (dep.isEmpty())
                    {
                        // linear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material->value(\"%1\")->number()").
                                arg(QString::fromStdString(quantity.id()));
                    }
                    else if (dep == "time")
                    {
                        // timedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                    else if (dep == "space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                    else if (dep == "time-space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                }
                else
                {
                    // nonlinear material
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    dict["d" + QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
            }
        }

        foreach (XMLModule::quantity quantity, parserModuleInfo.m_surface.quantity())
        {
            if (quantity.shortname().present())
            {
                QString dep = parserModuleInfo.dependenceSurface(QString::fromStdString(quantity.id()));

                if (dep.isEmpty())
                {
                    // linear boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = QString("boundary->value(\"%1\").number()").
                            arg(QString::fromStdString(quantity.id()));
                }
                else if (dep == "time")
                {
                    // timedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
                else if (dep == "space")
                {
                    // spacedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
                else if (dep == "time-space")
                {
                    // spacedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
            }
        }

        QSharedPointer<LexicalAnalyser> lex = weakFormLexicalAnalyser(parserModuleInfo);
        lex->setExpression(expr.isEmpty() ? "true" : expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in module %2").arg(e.toString()).arg(parserModuleInfo.m_id).toLatin1());

        return "";
    }
}


ParserModuleInfo::ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType) : m_analysisType(analysisType), m_coordinateType(coordinateType),
    m_constants(field.constants()), m_volume(field.volume()), m_surface(field.surface())
{
    m_numSolutions = Agros2DGenerator::numberOfSolutions(field.general_field().analyses(), analysisType);
    m_id = QString::fromStdString(field.general_field().id());
    m_isField = true;
}

QString ParserModuleInfo::nonlinearExpressionVolume(const QString &variable) const
{
    foreach (XMLModule::weakform_volume wf, m_volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (m_coordinateType == CoordinateType_Planar)
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
    std::string analysisString = analysisTypeToStringKey(m_analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_surface wf, m_surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variableString)
                    {
                        if (m_coordinateType == CoordinateType_Planar)
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
    std::string analysisString = analysisTypeToStringKey(m_analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_volume wf, m_volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::function_use functionUse, wf.function_use())
            {
                if (functionUse.id() == variableString)
                {
                    if (m_coordinateType == CoordinateType_Planar)
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
    std::string analysisString = analysisTypeToStringKey(m_analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_volume wf, m_volume.weakforms_volume().weakform_volume())
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
    std::string analysisString = analysisTypeToStringKey(m_analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_surface wf, m_surface.weakforms_surface().weakform_surface())
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
