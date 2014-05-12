#ifndef PARSER_H
#define PARSER_H

#include "util.h"
#include "util/enums.h"

#include "../../resources_source/classes/module_xml.h"

class LexicalAnalyser;

struct ParserModuleInfo
{
    ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType);
    ParserModuleInfo(XMLModule::coupling coupling, XMLModule::field field, XMLModule::field fieldSource, AnalysisType analysisType, AnalysisType analysisTypeSource,
                     CoordinateType coordinateType, LinearityType linearityType, LinearityType linearityTypeSource);

    XMLModule::constants m_constants;
    XMLModule::volume m_volume;
    XMLModule::surface m_surface;

    int m_numSolutions;
    int m_numSolutionsSource;
    bool m_isField;
    QString m_id;

    AnalysisType m_analysisType;
    AnalysisType m_analysisTypeSource;
    CoordinateType m_coordinateType;
    LinearityType m_linearityType;
    LinearityType m_linearityTypeSource;

    QString nonlinearExpressionVolume(const QString &variable) const;
    QString nonlinearExpressionSurface(const QString &variable) const;
    QString specialFunctionNonlinearExpression(const QString &variable) const;
    QString dependenceVolume(const QString &variable) const;
    QString dependenceSurface(const QString &variable) const;

    QMap<QString, int> m_quantityOrdering;
    QMap<QString, bool> m_quantityIsNonlinear;
    QMap<QString, int> m_functionOrdering;
};

bool operator<(const ParserModuleInfo &sid1, const ParserModuleInfo &sid2);

class Parser;
class CouplingParser;

class ParserInstance
{
public:
    QString parse(QString expr);
    ParserInstance(ParserModuleInfo pmi);

protected:
    void addBasicWeakformTokens();
    void addCouplingWeakformTokens();
    void addPreviousSolWeakform();
    void addPreviousSolErroCalculation();
    void addPreviousSolLinearizeDependence();
    void addVolumeVariablesWeakform();//ParserModuleInfo pmiField, bool isSource);
    void addVolumeVariablesErrorCalculation();
    void addSurfaceVariables();
    void addWeakformCheckTokens();

    void addPostprocessorBasic();
    void addPostprocessorVariables();
    void addFilterVariables();

    ParserModuleInfo m_parserModuleInfo;
    QMap<QString, QString> m_dict;
};

class ParserWeakForm : public ParserInstance
{
public:
    ParserWeakForm(ParserModuleInfo pmi);
};

class ParserErrorExpression : public ParserInstance
{
public:
    ParserErrorExpression(ParserModuleInfo pmi, bool withVariables);
};

class ParserLinearizeDependence : public ParserInstance
{
public:
    ParserLinearizeDependence(ParserModuleInfo pmi);
};

class ParserWeakformCheck : public ParserInstance
{
public:
    ParserWeakformCheck(ParserModuleInfo pmi);
};

class ParserWeakFormCoupling : public ParserInstance
{
public:
    ParserWeakFormCoupling(ParserModuleInfo pmi);
};

class ParserPostprocessorExpression : public ParserInstance
{
public:
    ParserPostprocessorExpression(ParserModuleInfo pmi, bool withVariables);
};

class ParserFilterExpression : public ParserInstance
{
public:
    ParserFilterExpression(ParserModuleInfo pmi, bool withVariables);
};

class ParserCouplingWeakForm : public ParserInstance
{
public:
    ParserCouplingWeakForm(ParserModuleInfo pmiCoupling, ParserModuleInfo pmiSource, ParserModuleInfo pmiTarget);
};

class Parser
{
public:
    static QString parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    static QString parseLinearizeDependence(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr);

    static QString parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    static QString parseFilterExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);

    // todo: move to ParserModuleInfo?
    static QSharedPointer<LexicalAnalyser> weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo);
    static QSharedPointer<LexicalAnalyser> postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo);

private:
    static void commonLexicalAnalyser(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);

    static QMap<ParserModuleInfo, QSharedPointer<ParserWeakForm> > m_parserWeakFormCache;
    static QMap<ParserModuleInfo, QSharedPointer<ParserWeakformCheck> > m_parserWeakFormCheckCache;
};

#endif // PARSER_H
