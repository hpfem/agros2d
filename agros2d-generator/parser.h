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

    QString nonlinearExpressionVolume(const QString &variable) const;
    QString nonlinearExpressionSurface(const QString &variable) const;
    QString specialFunctionNonlinearExpression(const QString &variable) const;
    QString dependenceVolume(const QString &variable) const;
    QString dependenceSurface(const QString &variable) const;

};

class ModuleParser;

class ParserInstance
{
public:
    QString parse(QString expr);
    ParserInstance(ParserModuleInfo pmi, ModuleParser *moduleParser);

protected:
    void addBasicWeakformTokens();
    void addPreviousSolWeakform();
    void addPreviousSolErroCalculation();
    void addVolumeVariablesWeakform();
    void addVolumeVariablesErrorCalculation();
    void addSurfaceVariables();
    void addWeakformCheckTokens();

    ParserModuleInfo m_parserModuleInfo;
    QMap<QString, QString> m_dict;
    QSharedPointer<LexicalAnalyser> m_lex;
    ModuleParser* m_moduleParser;
};

class ParserWeakForm : public ParserInstance
{
public:
    ParserWeakForm(ParserModuleInfo pmi, ModuleParser *moduleParser, bool withVariables);
};

class ParserErrorExpression : public ParserInstance
{
public:
    ParserErrorExpression(ParserModuleInfo pmi, ModuleParser *moduleParser, bool withVariables);
};

class ModuleParser
{
public:
    ModuleParser(XMLModule::field * field);

    QString parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    QString parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);

    QString parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool includeVariables, bool forFilter = false);
    QString parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr);

    // todo: move to ParserModuleInfo?
    QSharedPointer<LexicalAnalyser> weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo) const;
    QSharedPointer<LexicalAnalyser> postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo) const;

    QMap<QString, int> quantityOrdering() const { return m_quantityOrdering; }
    QMap<QString, bool> quantityIsNonlinear() const { return m_quantityIsNonlinear; }
    QMap<QString, int> functionOrdering() const { return m_functionOrdering; }

private:
    void commonLexicalAnalyser(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo) const;

    QMap<QString, int> m_quantityOrdering;
    QMap<QString, bool> m_quantityIsNonlinear;
    QMap<QString, int> m_functionOrdering;
};

#endif // PARSER_H
