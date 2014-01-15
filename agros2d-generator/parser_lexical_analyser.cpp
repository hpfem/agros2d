#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"


LexicalAnalyser *Parser::postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo)
{
    LexicalAnalyser *lex = new LexicalAnalyser();

    // coordinates
    if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
    {
        lex->addVariable("tanx");
        lex->addVariable("tany");
        lex->addVariable("velx");
        lex->addVariable("vely");
        lex->addVariable("velz");
        lex->addVariable("x");
        lex->addVariable("y");
        lex->addVariable("tx");
        lex->addVariable("ty");
        lex->addVariable("nx");
        lex->addVariable("ny");
    }
    else
    {
        lex->addVariable("tanr");
        lex->addVariable("tanz");
        lex->addVariable("velr");
        lex->addVariable("velz");
        lex->addVariable("velphi");
        lex->addVariable("r");
        lex->addVariable("z");
        lex->addVariable("tr");
        lex->addVariable("tz");
        lex->addVariable("nr");
        lex->addVariable("nz");
    }

    // marker area
    lex->addVariable("area");

    return lex;
}

//-----------------------------------------------------------------------------------------

LexicalAnalyser *Parser::weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo)
{
    LexicalAnalyser *lex = new LexicalAnalyser();

    // scalar field
    lex->addVariable("uval");
    lex->addVariable("upval");
    lex->addVariable("vval");

    // vector field
    lex->addVariable("val0");
    lex->addVariable("val1");
    lex->addVariable("ucurl");
    lex->addVariable("vcurl");
    lex->addVariable("upcurl");

    // coordinates
    if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
    {
        // scalar field
        lex->addVariable("udx");
        lex->addVariable("vdx");
        lex->addVariable("udy");
        lex->addVariable("vdy");
        lex->addVariable("updx");
        lex->addVariable("updy");

        // vector field
        lex->addVariable("dx0");
        lex->addVariable("dx1");
        lex->addVariable("dy0");
        lex->addVariable("dy1");

        lex->addVariable(QString("x"));
        lex->addVariable(QString("y"));
    }
    else
    {
        // scalar field
        lex->addVariable("udr");
        lex->addVariable("vdr");
        lex->addVariable("udz");
        lex->addVariable("vdz");
        lex->addVariable("updr");
        lex->addVariable("updz");

        // vector field
        lex->addVariable("dr0");
        lex->addVariable("dr1");
        lex->addVariable("dz0");
        lex->addVariable("dz1");

        lex->addVariable(QString("r"));
        lex->addVariable(QString("z"));
    }

    if (parserModuleInfo.m_analysisType == AnalysisType_Transient)
    {
        lex->addVariable("deltat");
        lex->addVariable("timedermat");
        lex->addVariable("timedervec");
    }

    commonLexicalAnalyser(lex, parserModuleInfo);

    return lex;
}


void Parser::commonLexicalAnalyser(LexicalAnalyser *lex, ParserModuleInfo parserModuleInfo)
{
    int numOfSol = parserModuleInfo.m_numSolutions;

    // functions
    for (int i = 1; i < numOfSol + 1; i++)
    {
        lex->addVariable(QString("value%1").arg(i));
        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            lex->addVariable(QString("dx%1").arg(i));
            lex->addVariable(QString("dy%1").arg(i));
        }
        else
        {
            lex->addVariable(QString("dr%1").arg(i));
            lex->addVariable(QString("dz%1").arg(i));
        }
    }

    // constants
    lex->addVariable("PI");
    lex->addVariable("f");
    foreach (XMLModule::constant cnst, parserModuleInfo.m_constants.constant())
        lex->addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, parserModuleInfo.m_volume.quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::quantity quantity, parserModuleInfo.m_surface.quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }
}
