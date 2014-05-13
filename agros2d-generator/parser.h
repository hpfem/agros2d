#ifndef PARSER_H
#define PARSER_H

#include "util.h"
#include "util/enums.h"

#include "../../resources_source/classes/module_xml.h"

class LexicalAnalyser;

struct ParserModuleInfo
{
    ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType);

    XMLModule::constants constants;
    XMLModule::volume volume;
    XMLModule::surface surface;

    int numSolutions;
    QString id;

    AnalysisType analysisType;
    CoordinateType coordinateType;
    LinearityType linearityType;

    QString nonlinearExpressionVolume(const QString &variable) const;
    QString nonlinearExpressionSurface(const QString &variable) const;
    QString specialFunctionNonlinearExpression(const QString &variable) const;
    QString dependenceVolume(const QString &variable) const;
    QString dependenceSurface(const QString &variable) const;

    QMap<QString, int> quantityOrdering;
    QMap<QString, bool> quantityIsNonlinear;
    QMap<QString, int> functionOrdering;
};

bool operator<(const ParserModuleInfo &sid1, const ParserModuleInfo &sid2);

class Parser;
class CouplingParser;

class ParserInstance
{
public:
    QString parse(QString expr);
    ParserInstance(ParserModuleInfo pmi);
    ParserInstance(ParserModuleInfo pmiSource, ParserModuleInfo pmi);

protected:
    void addBasicWeakformTokens();
    void addCouplingWeakformTokens(int numSourceSolutions);
    void addPreviousSolWeakform(int numSolutions);
    void addPreviousSolErroCalculation();
    void addPreviousSolLinearizeDependence();
    void addVolumeVariablesWeakform(ParserModuleInfo pmiField, bool isSource);
    void addVolumeVariablesErrorCalculation();
    void addSurfaceVariables();
    void addWeakformCheckTokens();

    void addPostprocessorBasic();
    void addPostprocessorVariables();
    void addFilterVariables();

    ParserModuleInfo m_parserModuleInfo;
    ParserModuleInfo m_parserModuleInfoSource;
    QMap<QString, QString> m_dict;
    QSharedPointer<LexicalAnalyser> m_lexicalAnalyser;
};

class ParserInstanceWeakForm : public ParserInstance
{
public:
    ParserInstanceWeakForm(ParserModuleInfo pmi);
};

class ParserInstanceErrorExpression : public ParserInstance
{
public:
    ParserInstanceErrorExpression(ParserModuleInfo pmi, bool withVariables);
};

class ParserInstanceLinearizeDependence : public ParserInstance
{
public:
    ParserInstanceLinearizeDependence(ParserModuleInfo pmi);
};

class ParserInstanceWeakformCheck : public ParserInstance
{
public:
    ParserInstanceWeakformCheck(ParserModuleInfo pmi);
};

class ParserInstancePostprocessorExpression : public ParserInstance
{
public:
    ParserInstancePostprocessorExpression(ParserModuleInfo pmi, bool withVariables);
};

class ParserInstanceFilterExpression : public ParserInstance
{
public:
    ParserInstanceFilterExpression(ParserModuleInfo pmi, bool withVariables);
};

class ParserInstanceCouplingWeakForm : public ParserInstance
{
public:
    ParserInstanceCouplingWeakForm(ParserModuleInfo pmiSource, ParserModuleInfo pmi);
};

class Parser
{
public:
    static QString parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseErrorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    static QString parseLinearizeDependence(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr);
    static QString parseCouplingWeakFormExpression(ParserModuleInfo parserModuleInfoSource, ParserModuleInfo parserModuleInfo, const QString &expr);

    static QString parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);
    static QString parseFilterExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool withVariables = true);

    static QSharedPointer<LexicalAnalyser> weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo);
    static QSharedPointer<LexicalAnalyser> weakFormCouplingLexicalAnalyser(ParserModuleInfo parserModuleInfoSource, ParserModuleInfo parserModuleInfo);
    static QSharedPointer<LexicalAnalyser> postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo);

private:
    static void addPreviousSolutionsLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSolutions);
    static void addSourceCouplingLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSourceSolutions);
    static void addWeakFormLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);
    static void addQuantitiesLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);
    static void addPostprocessorLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo);

    static QMap<ParserModuleInfo, QSharedPointer<ParserInstanceWeakForm> > m_parserWeakFormCache;
    static QMap<ParserModuleInfo, QSharedPointer<ParserInstanceWeakformCheck> > m_parserWeakFormCheckCache;
};

#endif // PARSER_H
