#include "util/constants.h"
#include "solver/weak_form.h"
#include "solver/module.h"
#include "solver/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"
#include "parser_instance.h"

//QMap<ParserModuleInfo, QSharedPointer<ParserInstanceWeakForm> > Parser::m_parserWeakFormCache;
//QMap<ParserModuleInfo, QSharedPointer<ParserInstanceWeakformCheck> > Parser::m_parserWeakFormCheckCache;

// to allow use of QMap for caching (not used at the moment)
bool operator<(const ParserModuleInfo &pmi1, const ParserModuleInfo &pmi2)
{
    if (pmi1.id != pmi2.id)
        return pmi1.id < pmi2.id;

    if (pmi1.analysisType != pmi2.analysisType)
        return pmi1.analysisType < pmi2.analysisType;

    if (pmi1.coordinateType != pmi2.coordinateType)
        return pmi1.coordinateType < pmi2.coordinateType;

    return pmi1.linearityType < pmi2.linearityType;
}


QString Parser::parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserInstancePostprocessorExpression parserInstance(parserModuleInfo, withVariables);
    return parserInstance.parse(expr);
}

QString Parser::parseFilterExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserInstanceFilterExpression parserInstance(parserModuleInfo, withVariables);
    return parserInstance.parse(expr);
}

QString Parser::parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr)
{
    ParserInstanceWeakForm parserInstance(parserModuleInfo);
    return parserInstance.parse(expr);
//    QSharedPointer<ParserInstanceWeakForm> parserInstance;
//    if(m_parserWeakFormCache.contains(parserModuleInfo))
//    {
//        parserInstance = m_parserWeakFormCache[parserModuleInfo];
//    }
//    else
//    {
//        parserInstance = QSharedPointer<ParserInstanceWeakForm>(new ParserInstanceWeakForm(parserModuleInfo));
//        m_parserWeakFormCache[parserModuleInfo] = parserInstance;
//    }
//    return parserInstance->parse(expr);
}

QString Parser::parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables)
{
    ParserInstanceErrorExpression parserInstance(parserModuleInfo, withVariables);
    return parserInstance.parse(expr);
}

QString Parser::parseLinearizeDependence(ParserModuleInfo parserModuleInfo, const QString &expr)
{
    ParserInstanceLinearizeDependence parserInstance(parserModuleInfo);
    return parserInstance.parse(expr);
}

QString Parser::parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr)
{
    ParserInstanceWeakformCheck parserInstance(parserModuleInfo);
    return parserInstance.parse(expr);
//    QSharedPointer<ParserInstanceWeakformCheck> parserInstance;
//    if(m_parserWeakFormCheckCache.contains(parserModuleInfo))
//    {
//        parserInstance = m_parserWeakFormCheckCache[parserModuleInfo];
//    }
//    else
//    {
//        parserInstance = QSharedPointer<ParserInstanceWeakformCheck>(new ParserInstanceWeakformCheck(parserModuleInfo));
//        m_parserWeakFormCheckCache[parserModuleInfo] = parserInstance;
//    }
//    return parserInstance->parse(expr);
}

QString Parser::parseCouplingWeakFormExpression(ParserModuleInfo parserModuleInfoSource, ParserModuleInfo parserModuleInfo, const QString &expr)
{
    ParserInstanceCouplingWeakForm parserInstance(parserModuleInfoSource, parserModuleInfo);
    return parserInstance.parse(expr);
}


QSharedPointer<LexicalAnalyser> Parser::postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo)
{
    QSharedPointer<LexicalAnalyser> lex = QSharedPointer<LexicalAnalyser>(new LexicalAnalyser());

    addPreviousSolutionsLATokens(lex, parserModuleInfo.coordinateType, parserModuleInfo.numSolutions);
    addQuantitiesLATokens(lex, parserModuleInfo);
    addPostprocessorLATokens(lex, parserModuleInfo);

    return lex;
}

QSharedPointer<LexicalAnalyser> Parser::weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo)
{
    QSharedPointer<LexicalAnalyser> lex = QSharedPointer<LexicalAnalyser>(new LexicalAnalyser());

    addPreviousSolutionsLATokens(lex, parserModuleInfo.coordinateType, parserModuleInfo.numSolutions);
    addQuantitiesLATokens(lex, parserModuleInfo);
    addWeakFormLATokens(lex, parserModuleInfo);

    return lex;
}

QSharedPointer<LexicalAnalyser> Parser::weakFormCouplingLexicalAnalyser(ParserModuleInfo parserModuleInfoSource, ParserModuleInfo parserModuleInfo)
{
    QSharedPointer<LexicalAnalyser> lex = QSharedPointer<LexicalAnalyser>(new LexicalAnalyser());

    addPreviousSolutionsLATokens(lex, parserModuleInfo.coordinateType, parserModuleInfo.numSolutions + parserModuleInfoSource.numSolutions);
    addQuantitiesLATokens(lex, parserModuleInfo);
    addQuantitiesLATokens(lex, parserModuleInfoSource);
    addWeakFormLATokens(lex, parserModuleInfo);
    addSourceCouplingLATokens(lex, parserModuleInfo.coordinateType, parserModuleInfoSource.numSolutions);

    return lex;
}

