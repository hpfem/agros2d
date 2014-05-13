#ifndef PARSER_H
#define PARSER_H

#include "parser_module_info.h"

class LexicalAnalyser;

// parser interface
// allowes to parse different type of expressions
class Parser
{
public:
    // each function parse different type of expression, reusing as much code as possible
    static QString parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    static QString parseLinearizeDependence(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseCouplingWeakFormExpression(ParserModuleInfo parserModuleInfoSource, ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    static QString parseFilterExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);

    // creators of different types of lexical analysers
    // for each configuration of module creates different analyser by adding only those symbols that are neccessary (or some extra, but not all of them)
    // created analysers could be used for multiple expressions, but now are created each time (it seems that it is not the time bottleneck)
    static QSharedPointer<LexicalAnalyser> weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo);
    static QSharedPointer<LexicalAnalyser> weakFormCouplingLexicalAnalyser(ParserModuleInfo parserModuleInfoSource, ParserModuleInfo parserModuleInfo);
    static QSharedPointer<LexicalAnalyser> postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo);

private:
    static void addPreviousSolutionsLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSolutions);
    static void addSourceCouplingLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSourceSolutions);
    static void addWeakFormLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);
    static void addQuantitiesLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);
    static void addPostprocessorLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);

    // caching, not used now
//    static QMap<ParserModuleInfo, QSharedPointer<ParserInstanceWeakForm> > m_parserWeakFormCache;
//    static QMap<ParserModuleInfo, QSharedPointer<ParserInstanceWeakformCheck> > m_parserWeakFormCheckCache;
};

#endif // PARSER_H
