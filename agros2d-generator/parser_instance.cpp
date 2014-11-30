#include "parser/lex.h"
#include "parser.h"
#include "parser_instance.h"

// todo: m_parserModuleInfoSource has to be initialized by something
// todo: this is not good, but implicit initialization is not possible
ParserInstance::ParserInstance(ParserModuleInfo pmi) : m_parserModuleInfo(pmi), m_parserModuleInfoSource(pmi)
{

}

ParserInstance::ParserInstance(ParserModuleInfo pmiSource, ParserModuleInfo pmi) : m_parserModuleInfo(pmi), m_parserModuleInfoSource(pmiSource)
{

}

QString ParserInstance::parse(QString expr)
{
    try
    {
        m_lexicalAnalyser->setExpression(expr);
        QString exprCpp = m_lexicalAnalyser->replaceVariables(m_dict);

        // TODO: move from lex
        exprCpp = m_lexicalAnalyser->replaceOperatorByFunction(exprCpp);

        return exprCpp;
    }
    catch (ParserException e)
    {
        qWarning() << (QString("%1 in module %2").arg(e.toString()).arg(m_parserModuleInfo.id).toLatin1());

        return "";
    }

}

ParserInstanceWeakForm::ParserInstanceWeakForm(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens();
    addConstants(m_parserModuleInfo);
    addPreviousSolWeakform(pmi.numSolutions);
    addVolumeVariablesWeakform(pmi, false);
    addSurfaceVariables();
}

ParserInstanceErrorExpression::ParserInstanceErrorExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens();
    addConstants(m_parserModuleInfo);
    addPreviousSolErroCalculation();
    if(withVariables)
    {
        addVolumeVariablesErrorCalculation();
        addSurfaceVariables();
    }
}

ParserInstanceLinearizeDependence::ParserInstanceLinearizeDependence(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens();
    addConstants(m_parserModuleInfo);
    addPreviousSolLinearizeDependence();
}

ParserInstanceWeakformCheck::ParserInstanceWeakformCheck(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    m_lexicalAnalyser =  Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addWeakformCheckTokens();
}

ParserInstancePostprocessorExpression::ParserInstancePostprocessorExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::postprocessorLexicalAnalyser(m_parserModuleInfo);

    addPostprocessorBasic();
    addConstants(m_parserModuleInfo);

    if(withVariables)
    {
        addPostprocessorVariables();
    }
}

ParserInstanceFilterExpression::ParserInstanceFilterExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::postprocessorLexicalAnalyser(m_parserModuleInfo);

    addPostprocessorBasic();
    addConstants(m_parserModuleInfo);

    if(withVariables)
    {
        addFilterVariables();
    }
}

ParserInstanceCouplingWeakForm::ParserInstanceCouplingWeakForm(ParserModuleInfo pmiSource, ParserModuleInfo pmi)
    :ParserInstance(pmiSource, pmi)
{
    m_lexicalAnalyser = Parser::weakFormCouplingLexicalAnalyser(m_parserModuleInfoSource, m_parserModuleInfo);

    addBasicWeakformTokens();
    addConstants(m_parserModuleInfo);
    addConstants(m_parserModuleInfoSource);
    addCouplingWeakformTokens(pmiSource.numSolutions);
    addPreviousSolWeakform(pmi.numSolutions + pmiSource.numSolutions);
    addVolumeVariablesWeakform(pmi, false);
    addVolumeVariablesWeakform(pmiSource, true);
}
