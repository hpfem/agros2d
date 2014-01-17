#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"


void ParserInstance::addBasicWeakformTokens()
{
    // coordinates
    if (m_parserModuleInfo.m_coordinateType == CoordinateType_Planar)
    {
        m_dict["x"] = "e->x[i]";
        m_dict["y"] = "e->y[i]";
        m_dict["tx"] = "e->tx[i]";
        m_dict["ty"] = "e->ty[i]";
        m_dict["nx"] = "e->nx[i]";
        m_dict["ny"] = "e->ny[i]";
    }
    else
    {
        m_dict["r"] = "e->x[i]";
        m_dict["z"] = "e->y[i]";
        m_dict["tr"] = "e->tx[i]";
        m_dict["tz"] = "e->ty[i]";
        m_dict["nr"] = "e->nx[i]";
        m_dict["nz"] = "e->ny[i]";
    }

    // constants
    m_dict["PI"] = "M_PI";
    m_dict["f"] = "this->m_markerSource->fieldInfo()->frequency()";
    foreach (XMLModule::constant cnst, m_parserModuleInfo.m_constants.constant())
        m_dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

    // area of a label
    // assumes, that this->getAreas has allways only one component (it is true at the moment, since in Agros we create one form for each label)
    m_dict["area"] = "this->markerVolume()";

    // functions
    // scalar field
    m_dict["uval"] = "u->val[i]";
    m_dict["vval"] = "v->val[i]";
    m_dict["upval"] = "u_ext[this->j]->val[i]";
    m_dict["uptval"] = "ext[*this->m_offsetPreviousTimeExt + this->j - this->m_offsetJ]->val[i]";
    m_dict["deltat"] = "Agros2D::problem()->actualTimeStepLength()";

    // vector field
    m_dict["uval0"] = "u->val0[i]";
    m_dict["uval1"] = "u->val1[i]";
    m_dict["ucurl"] = "u->curl[i]";
    m_dict["vval0"] = "v->val0[i]";
    m_dict["vval1"] = "v->val1[i]";
    m_dict["vcurl"] = "v->curl[i]";
    m_dict["upcurl"] = "u_ext[this->j]->curl[i]";

    m_dict["timedermat"] = "(*this->m_table)->matrixFormCoefficient()";
    m_dict["timedervec"] = "(*this->m_table)->vectorFormCoefficient(ext, this->j, this->m_markerSource->fieldInfo()->numberOfSolutions(), offset.prevSol, i)";

    if (m_parserModuleInfo.m_coordinateType == CoordinateType_Planar)
    {
        // scalar field
        m_dict["udx"] = "u->dx[i]";
        m_dict["vdx"] = "v->dx[i]";
        m_dict["udy"] = "u->dy[i]";
        m_dict["vdy"] = "v->dy[i]";
        m_dict["updx"] = "u_ext[this->j]->dx[i]";
        m_dict["updy"] = "u_ext[this->j]->dy[i]";
    }
    else
    {
        // scalar field
        m_dict["udr"] = "u->dx[i]";
        m_dict["vdr"] = "v->dx[i]";
        m_dict["udz"] = "u->dy[i]";
        m_dict["vdz"] = "v->dy[i]";
        m_dict["updr"] = "u_ext[this->j]->dx[i]";
        m_dict["updz"] = "u_ext[this->j]->dy[i]";
    }
}

void ParserInstance::addPreviousSolWeakform()
{
    for (int i = 1; i < m_parserModuleInfo.m_numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("u_ext[%1 + offset.forms]->val[i]").arg(i-1);
        m_dict[QString("timedervec%1").arg(i)] = QString("(*this->m_table)->vectorFormCoefficient(ext, %1, this->m_markerSource->fieldInfo()->numberOfSolutions(), offset.prevSol, i)").arg(i-1);

        if (m_parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dx[i]").arg(i-1);
            m_dict[QString("dy%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dy[i]").arg(i-1);
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dx[i]").arg(i-1);
            m_dict[QString("dz%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dy[i]").arg(i-1);
        }
    }
}

void ParserInstance::addPreviousSolErroCalculation()
{
    for (int i = 1; i < m_parserModuleInfo.m_numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("u->val[i]");

        if (m_parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("u->dx[i]");
            m_dict[QString("dy%1").arg(i)] = QString("u->dy[i]");
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("u->dx[i]");
            m_dict[QString("dz%1").arg(i)] = QString("u->dy[i]");
        }
    }
}

void ParserInstance::addVolumeVariablesErrorCalculation()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.m_volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceVolume(QString::fromStdString(quantity.id()));
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (m_parserModuleInfo.m_linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
            {
                if (dep.isEmpty())
                {
                    // linear material
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
                else if (dep == "time")
                {
                    // linear boundary condition
                    // ERROR: Python expression evaluation doesn't work from weakform ("false" should be removed)
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
                else if (dep == "space")
                {
                    // spacedep boundary condition
                    // ERROR: Python expression evaluation doesn't work from weakform - ERROR
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(x, y))").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
                else if (dep == "time-space")
                {
                    // spacedep boundary condition
                    // ERROR: Python expression evaluation doesn't work from weakform - ERROR
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(x, y))").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
            }
            else
            {
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberFromTable(%2)").
                        arg(QString::fromStdString(quantity.shortname().get())).
                        arg(m_moduleParser->parseErrorExpression(m_parserModuleInfo, nonlinearExpr, false));

                if (m_parserModuleInfo.m_linearityType == LinearityType_Newton)
                    m_dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("%1->derivativeFromTable(%2)").
                            arg(QString::fromStdString(quantity.shortname().get())).
                            arg(m_moduleParser->parseErrorExpression(m_parserModuleInfo, nonlinearExpr, false));
            }
        }
    }

    //todo: XMLModule::function
}

void ParserInstance::addVolumeVariablesWeakform()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.m_volume.quantity())
    {
        if (quantity.shortname().present())
        {
            // in weak forms, values replaced by ext functions
            m_dict[QString::fromStdString(quantity.shortname().get())] = QString("ext[%1 + offset.quant]->val[i]").
                    arg(m_moduleParser->quantityOrdering()[QString::fromStdString(quantity.id())]);
            if(m_moduleParser->quantityIsNonlinear()[QString::fromStdString(quantity.id())])
            {
                m_dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("ext[%1 + offset.quant]->val[i]").
                        arg(m_moduleParser->quantityOrdering()[QString::fromStdString(quantity.id())] + 1);

            }
        }
    }

    foreach (XMLModule::function function, m_parserModuleInfo.m_volume.function())
    {
        // in weak forms, functions replaced by ext functions
        m_dict[QString::fromStdString(function.shortname())] = QString("ext[%1 + offset.quant]->val[i]").
                arg(m_moduleParser->functionOrdering()[QString::fromStdString(function.id())]);
    }
}

void ParserInstance::addSurfaceVariables()
{
    // surface quantities still done the old way
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.m_surface.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceSurface(QString::fromStdString(quantity.id()));

            if (dep.isEmpty())
            {
                // linear boundary condition
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                        arg(QString::fromStdString(quantity.shortname().get()));
            }
            else if (dep == "time")
            {
                // linear boundary condition
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                        arg(QString::fromStdString(quantity.shortname().get()));
            }
            else if (dep == "space")
            {
                // spacedep boundary condition
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(x, y))").
                        arg(QString::fromStdString(quantity.shortname().get()));
            }
            else if (dep == "time-space")
            {
                // spacedep boundary condition
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(x, y))").
                        arg(QString::fromStdString(quantity.shortname().get()));
            }
        }
    }
}

ParserInstance::ParserInstance(ParserModuleInfo pmi, ModuleParser *moduleParser) : m_parserModuleInfo(pmi), m_moduleParser(moduleParser)
{

}

ParserWeakForm::ParserWeakForm(ParserModuleInfo pmi, ModuleParser *moduleParser, bool withVariables) : ParserInstance(pmi, moduleParser)
{
    addBasicWeakformTokens();
    addPreviousSolWeakform();
    if(withVariables)
    {
        addVolumeVariablesWeakform();
        addSurfaceVariables();
    }
}

ParserErrorExpression::ParserErrorExpression(ParserModuleInfo pmi, ModuleParser *moduleParser, bool withVariables) : ParserInstance(pmi, moduleParser)
{
    addBasicWeakformTokens();
    addPreviousSolErroCalculation();
    if(withVariables)
    {
        addVolumeVariablesErrorCalculation();
        addSurfaceVariables();
    }
}


QSharedPointer<LexicalAnalyser> ModuleParser::postprocessorLexicalAnalyser(ParserModuleInfo parserModuleInfo) const
{
    QSharedPointer<LexicalAnalyser> lex = QSharedPointer<LexicalAnalyser>(new LexicalAnalyser());

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

QSharedPointer<LexicalAnalyser> ModuleParser::weakFormLexicalAnalyser(ParserModuleInfo parserModuleInfo) const
{
    QSharedPointer<LexicalAnalyser> lex = QSharedPointer<LexicalAnalyser>(new LexicalAnalyser());

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


void ModuleParser::commonLexicalAnalyser(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo) const
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
