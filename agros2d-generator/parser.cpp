#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"

QMap<ParserModuleInfo, QSharedPointer<ParserWeakForm> > Parser::m_parserWeakFormCache;
QMap<ParserModuleInfo, QSharedPointer<ParserWeakformCheck> > Parser::m_parserWeakFormCheckCache;

bool operator<(const ParserModuleInfo &pmi1, const ParserModuleInfo &pmi2)
{
    if (pmi1.m_id != pmi2.m_id)
        return pmi1.m_id < pmi2.m_id;

    if (pmi1.m_analysisType != pmi2.m_analysisType)
        return pmi1.m_analysisType < pmi2.m_analysisType;

    if (pmi1.m_coordinateType != pmi2.m_coordinateType)
        return pmi1.m_coordinateType < pmi2.m_coordinateType;

    return pmi1.m_linearityType < pmi2.m_linearityType;
}


QString Parser::parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserPostprocessorExpression parser(parserModuleInfo, withVariables);
    return parser.parse(expr);
}

QString Parser::parseFilterExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserFilterExpression parser(parserModuleInfo, withVariables);
    return parser.parse(expr);
}

QString Parser::parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr)
{
//    ParserWeakForm parser(parserModuleInfo, withVariables);
//    return parser.parse(expr);
    QSharedPointer<ParserWeakForm> parser;
    if(m_parserWeakFormCache.contains(parserModuleInfo))
    {
        parser = m_parserWeakFormCache[parserModuleInfo];
    }
    else
    {
        parser = QSharedPointer<ParserWeakForm>(new ParserWeakForm(parserModuleInfo));
        m_parserWeakFormCache[parserModuleInfo] = parser;
    }
    return parser->parse(expr);
}

QString Parser::parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserErrorExpression parser(parserModuleInfo, withVariables);
    return parser.parse(expr);
}

QString Parser::parseLinearizeDependence(ParserModuleInfo parserModuleInfo, const QString &expr)
{
    ParserLinearizeDependence parser(parserModuleInfo);
    return parser.parse(expr);
}

QString Parser::parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr)
{
//    ParserWeakformCheck parser(parserModuleInfo);
//    return parser.parse(expr);
    QSharedPointer<ParserWeakformCheck> parser;
    if(m_parserWeakFormCheckCache.contains(parserModuleInfo))
    {
        parser = m_parserWeakFormCheckCache[parserModuleInfo];
    }
    else
    {
        parser = QSharedPointer<ParserWeakformCheck>(new ParserWeakformCheck(parserModuleInfo));
        m_parserWeakFormCheckCache[parserModuleInfo] = parser;
    }
    return parser->parse(expr);
}

QString ParserInstance::parse(QString expr)
{
    try
    {
        QSharedPointer<LexicalAnalyser> lex = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);
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

ParserModuleInfo::ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType) :
    m_analysisType(analysisType), m_coordinateType(coordinateType), m_linearityType(linearityType),
    m_constants(field.constants()), m_volume(field.volume()), m_surface(field.surface())
{
    m_numSolutions = Agros2DGenerator::numberOfSolutions(field.general_field().analyses(), analysisType);
    m_numSolutionsSource = -123456;
    m_id = QString::fromStdString(field.general_field().id());
    m_isField = true;

    Module::volumeQuantityProperties(&field, m_quantityOrdering, m_quantityIsNonlinear, m_functionOrdering);

}

ParserModuleInfo::ParserModuleInfo(XMLModule::coupling coupling, XMLModule::field field, XMLModule::field fieldSource, AnalysisType analysisType, AnalysisType analysisTypeSource, CoordinateType coordinateType, LinearityType linearityType, LinearityType linearityTypeSource) :
    m_analysisType(analysisType), m_analysisTypeSource(analysisTypeSource), m_coordinateType(coordinateType), m_linearityType(linearityType), m_linearityTypeSource(linearityTypeSource),
    m_constants(field.constants()), m_volume(field.volume()), m_surface(field.surface())
{
    m_numSolutions = Agros2DGenerator::numberOfSolutions(field.general_field().analyses(), analysisType);
    m_numSolutionsSource = Agros2DGenerator::numberOfSolutions(fieldSource.general_field().analyses(), analysisType);
    m_id = QString::fromStdString(coupling.general_coupling().id());
    m_isField = false;
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

ParserWeakForm::ParserWeakForm(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    addBasicWeakformTokens();
    addPreviousSolWeakform();
    addVolumeVariablesWeakform();//pmi, false);
    addSurfaceVariables();
}

ParserErrorExpression::ParserErrorExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    addBasicWeakformTokens();
    addPreviousSolErroCalculation();
    if(withVariables)
    {
        addVolumeVariablesErrorCalculation();
        addSurfaceVariables();
    }
}

ParserWeakFormCoupling::ParserWeakFormCoupling(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    addBasicWeakformTokens();
    addCouplingWeakformTokens();
}

ParserLinearizeDependence::ParserLinearizeDependence(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    addBasicWeakformTokens();
    addPreviousSolLinearizeDependence();
}

ParserWeakformCheck::ParserWeakformCheck(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    addWeakformCheckTokens();
}

ParserPostprocessorExpression::ParserPostprocessorExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    addPostprocessorBasic();
    if(withVariables)
    {
        addPostprocessorVariables();
    }
}

ParserFilterExpression::ParserFilterExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    addPostprocessorBasic();
    if(withVariables)
    {
        addFilterVariables();
    }
}

//ParserCouplingWeakForm::ParserCouplingWeakForm(ParserModuleInfo pmiCoupling, ParserModuleInfo pmiSource, ParserModuleInfo pmiTarget)
//    :ParserInstance(pmiCoupling)
//{
//    (pmiSource);
//}
