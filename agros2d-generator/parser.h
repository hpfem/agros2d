#ifndef PARSER_H
#define PARSER_H

#include "util.h"
#include "util/enums.h"

#include "../../resources_source/classes/module_xml.h"

class LexicalAnalyser;

struct ParserModuleInfo
{
    ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType);

    XMLModule::constants m_constants;
    XMLModule::volume m_volume;
    XMLModule::surface m_surface;

    int m_numSolutions;
    bool m_isField;
    QString m_id;

    AnalysisType m_analysisType;
    CoordinateType m_coordinateType;
    LinearityType m_linearityType;
};

class Parser
{
public:

    QString nonlinearExpression(const QString &variable, ParserModuleInfo parserModuleInfo);
    QString specialFunctionNonlinearExpression(const QString &variable, ParserModuleInfo parserModuleInfo);
    QString dependence(const QString &variable, ParserModuleInfo parserModuleInfo);
    QString parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool includeVariables, bool forFilter = false);

    LexicalAnalyser *weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo);
    LexicalAnalyser *postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo);

    QString parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool includeVariables = true, bool errorCalculation = false);
    QString parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr);

    // todo: do it some better way
    static QMap<QString, int> quantityOrdering;
    static QMap<QString, bool> quantityIsNonlinear;
    static QMap<QString, int> functionOrdering;

private:
    void commonLexicalAnalyser(LexicalAnalyser *lex, ParserModuleInfo parserModuleInfo);

};

#endif // PARSER_H
