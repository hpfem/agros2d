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

bool operator<(const ParserModuleInfo &sid1, const ParserModuleInfo &sid2);

class FieldParser;
class CouplingParser;

class ParserInstance
{
public:
    QString parse(QString expr);
    ParserInstance(ParserModuleInfo pmi, FieldParser *moduleParser);

protected:
    void addBasicWeakformTokens();
    void addPreviousSolWeakform();
    void addPreviousSolErroCalculation();
    void addPreviousSolLinearizeDependence();
    void addVolumeVariablesWeakform();
    void addVolumeVariablesErrorCalculation();
    void addSurfaceVariables();
    void addWeakformCheckTokens();

    void addPostprocessorBasic();
    void addPostprocessorVariables();
    void addFilterVariables();

    ParserModuleInfo m_parserModuleInfo;
    QMap<QString, QString> m_dict;
    FieldParser* m_fieldParser;
};

class ParserWeakForm : public ParserInstance
{
public:
    ParserWeakForm(ParserModuleInfo pmi, FieldParser *moduleParser, bool withVariables);
};

class ParserErrorExpression : public ParserInstance
{
public:
    ParserErrorExpression(ParserModuleInfo pmi, FieldParser *moduleParser, bool withVariables);
};

class ParserLinearizeDependence : public ParserInstance
{
public:
    ParserLinearizeDependence(ParserModuleInfo pmi, FieldParser *moduleParser);
};

class ParserWeakformCheck : public ParserInstance
{
public:
    ParserWeakformCheck(ParserModuleInfo pmi, FieldParser *moduleParser);
};

class ParserPostprocessorExpression : public ParserInstance
{
public:
    ParserPostprocessorExpression(ParserModuleInfo pmi, FieldParser *moduleParser, bool withVariables);
};

class ParserFilterExpression : public ParserInstance
{
public:
    ParserFilterExpression(ParserModuleInfo pmi, FieldParser *moduleParser, bool withVariables);
};

class ParserCouplingWeakForm : public ParserInstance
{
public:
    ParserCouplingWeakForm(ParserModuleInfo pmiCoupling, ParserModuleInfo pmiSource, ParserModuleInfo pmiTarget,
                           CouplingParser* couplingParser, FieldParser* sourceParser, FieldParser* targetParser);
};

class FieldParser
{
public:
    FieldParser(XMLModule::field* field);
    FieldParser() {}

    QString parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    QString parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    QString parseLinearizeDependence(ParserModuleInfo parserModuleInfo, const QString &expr);
    QString parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr);

    QString parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    QString parseFilterExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);

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

    QMap<ParserModuleInfo, QSharedPointer<ParserWeakForm> > m_parserWeakFormCache;
    QMap<ParserModuleInfo, QSharedPointer<ParserWeakformCheck> > m_parserWeakFormCheckCache;
};

class CouplingParser : public FieldParser
{
public:
    CouplingParser(XMLModule::coupling* coupling);
};

#endif // PARSER_H
