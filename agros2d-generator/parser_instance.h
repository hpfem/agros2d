#ifndef PARSER_INSTANCE_H
#define PARSER_INSTANCE_H

#include "parser_module_info.h"

// an interface class for different parser instances, used to parse different types of expressions for
// different modules and their different configurations
// include lexical analyser and dictionary of symbols specific for the given configuration
// it could be used (cached) for multiple runs of parse, but at the present moment, it is created from scratch for each parse run
// it seems, that caching does not improve the speed efficiency (the bottleneck is probably in the parsing and creation of parsed string itself)
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
    void addPreviousSolErrorCalculation();
    void addPreviousSolLinearizeDependence();
    void addVolumeVariablesWeakform(ParserModuleInfo pmiField, bool isSource);
    void addConstants(ParserModuleInfo pmiField);
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

// follow specific ParserInstance descendants for different types of expressions

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


#endif // PARSER_INSTANCE_H
