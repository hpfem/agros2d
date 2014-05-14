#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"
#include "parser_instance.h"

//********************************************************************************************************
// In this file are all methods of Parser and ParserInstance, that handle creation of lexical analyser and dictionary of symbols
// defining symbols and creation of lexical analyser is done in static methods of Parser
// creation of dictionary is done in ParserInstance
// This is probably not ideal state
//********************************************************************************************************

void ParserInstance::addBasicWeakformTokens()
{
    // coordinates
    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
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
    m_dict["f"] = "this->m_markerTarget->fieldInfo()->frequency()";

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
    m_dict["timedervec"] = "(*this->m_table)->vectorFormCoefficient(ext, this->j, this->m_markerTarget->fieldInfo()->numberOfSolutions(), offset.prevSol, i)";

    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
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

void ParserInstance::addConstants(ParserModuleInfo pmiField)
{
    foreach (XMLModule::constant cnst, pmiField.constants.constant())
    {
        m_dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());
    }
}

void ParserInstance::addCouplingWeakformTokens(int numSourceSolutions)
{
    for (int i = 1; i < numSourceSolutions + 1; i++)
    {
        m_dict[QString("source%1").arg(i)] = QString("ext[%1 + offset.sourcePrevSol]->val[i]").arg(i-1);
        if(m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("source%1dx").arg(i)] = QString("ext[%1 + offset.sourcePrevSol]->dx[i]").arg(i-1);
            m_dict[QString("source%1dy").arg(i)] = QString("ext[%1 + offset.sourcePrevSol]->dy[i]").arg(i-1);
        }
        else
        {
            m_dict[QString("source%1dr").arg(i)] = QString("ext[%1 + offset.sourcePrevSol]->dx[i]").arg(i-1);
            m_dict[QString("source%1dz").arg(i)] = QString("ext[%1 + offset.sourcePrevSol]->dy[i]").arg(i-1);
        }
    }
}


void ParserInstance::addPreviousSolWeakform(int numSolutions)
{
    for (int i = 1; i < numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("u_ext[%1 + offset.forms]->val[i]").arg(i-1);
        m_dict[QString("timedervec%1").arg(i)] = QString("(*this->m_table)->vectorFormCoefficient(ext, %1, this->m_markerTarget->fieldInfo()->numberOfSolutions(), offset.prevSol, i)").arg(i-1);

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
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
    for (int i = 1; i < m_parserModuleInfo.numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("u->val[i]");

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
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

// for linearized variant adding ext with index of allready solved field
// used for nonlinear source term in the case of weak coupling
void ParserInstance::addPreviousSolLinearizeDependence()
{
    for (int i = 1; i < m_parserModuleInfo.numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("ext[%1 + offset.prevSol]->val[i]").arg(i-1);

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dx[i]").arg(i-1);
            m_dict[QString("dy%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dy[i]").arg(i-1);
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dx[i]").arg(i-1);
            m_dict[QString("dz%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dy[i]").arg(i-1);
        }
    }
}

void ParserInstance::addVolumeVariablesErrorCalculation()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceVolume(QString::fromStdString(quantity.id()));
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (m_parserModuleInfo.linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
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
                        arg(Parser::parseErrorExpression(m_parserModuleInfo, nonlinearExpr, false));

                if (m_parserModuleInfo.linearityType == LinearityType_Newton)
                    m_dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("%1->derivativeFromTable(%2)").
                            arg(QString::fromStdString(quantity.shortname().get())).
                            arg(Parser::parseErrorExpression(m_parserModuleInfo, nonlinearExpr, false));
            }
        }
    }

    //todo: XMLModule::function
}

void ParserInstance::addVolumeVariablesWeakform(ParserModuleInfo pmiField, bool isSource)
{
    QString offsetQuant("offset.quant");
    if(isSource)
        offsetQuant = "offset.sourceQuant";

    foreach (XMLModule::quantity quantity, pmiField.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            // in weak forms, values replaced by ext functions
            m_dict[QString::fromStdString(quantity.shortname().get())] = QString("ext[%1 + %2]->val[i]").
                    arg(pmiField.quantityOrdering[QString::fromStdString(quantity.id())]).
                    arg(offsetQuant);
            if(pmiField.quantityIsNonlinear[QString::fromStdString(quantity.id())])
            {
                m_dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("ext[%1 + %2]->val[i]").
                        arg(pmiField.quantityOrdering[QString::fromStdString(quantity.id())] + 1).
                        arg(offsetQuant);

            }
        }
    }

    foreach (XMLModule::function function, pmiField.volume.function())
    {
        // in weak forms, functions replaced by ext functions
        m_dict[QString::fromStdString(function.shortname())] = QString("ext[%1 + %2]->val[i]").
                arg(pmiField.functionOrdering[QString::fromStdString(function.id())]).
                arg(offsetQuant);
    }
}

void ParserInstance::addSurfaceVariables()
{
    // surface quantities still done the old way
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.surface.quantity())
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

void ParserInstance::addWeakformCheckTokens()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceVolume(QString::fromStdString(quantity.id()));
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (m_parserModuleInfo.linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
            {
                if (dep.isEmpty())
                {
                    // linear material
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material->value(\"%1\")->number()").
                            arg(QString::fromStdString(quantity.id()));
                }
                else
                {
                    m_dict[QString::fromStdString(quantity.shortname().get())] = "1.";
                }
            }
            else
            {
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = "1.";
                m_dict["d" + QString::fromStdString(quantity.shortname().get())] = "1.";
            }
        }
    }

    foreach (XMLModule::quantity quantity, m_parserModuleInfo.surface.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceSurface(QString::fromStdString(quantity.id()));

            if (dep.isEmpty())
            {
                // linear boundary condition
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("boundary->value(\"%1\").number()").
                        arg(QString::fromStdString(quantity.id()));
            }
            else
            {
                m_dict[QString::fromStdString(quantity.shortname().get())] = "1.";
            }
        }
    }

}

void ParserInstance::addPostprocessorBasic()
{
    // coordinates
    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        m_dict["x"] = "x[i]";
        m_dict["y"] = "y[i]";
        // surface integral
        m_dict["tanx"] = "e->tx[i]";
        m_dict["tany"] = "e->ty[i]";
        // velocity (force calculation)
        m_dict["velx"] = "velocity.x";
        m_dict["vely"] = "velocity.y";
        m_dict["velz"] = "velocity.z";
    }
    else
    {
        m_dict["r"] = "x[i]";
        m_dict["z"] = "y[i]";
        // surface integral
        m_dict["tanr"] = "e->tx[i]";
        m_dict["tanz"] = "e->ty[i]";
        // velocity (force calculation)
        m_dict["velr"] = "velocity.x";
        m_dict["velz"] = "velocity.y";
        m_dict["velphi"] = "velocity.z";
    }

    // constants
    m_dict["PI"] = "M_PI";
    m_dict["f"] = "m_fieldInfo->frequency()";
    foreach (XMLModule::constant cnst, m_parserModuleInfo.constants.constant())
        m_dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

    // functions
    for (int i = 1; i < m_parserModuleInfo.numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("value[%1][i]").arg(i-1);
        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
            m_dict[QString("dy%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
            m_dict[QString("dz%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
        }
    }
    // eggshell
    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        m_dict["dxegg"] = "dudx[source_functions.size() - 1][i]";
        m_dict["dyegg"] = "dudy[source_functions.size() - 1][i]";
    }
    else
    {
        m_dict["dregg"] = "dudx[source_functions.size() - 1][i]";
        m_dict["dzegg"] = "dudy[source_functions.size() - 1][i]";
    }

}

void ParserInstance::addPostprocessorVariables()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (nonlinearExpr.isEmpty())
                // linear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
            else
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                        arg(QString::fromStdString(quantity.id())).
                        arg(Parser::parsePostprocessorExpression(m_parserModuleInfo, nonlinearExpr, false));
        }
    }

    foreach (XMLModule::function function, m_parserModuleInfo.volume.function())
    {
        QString parameter("0");
        // todo: so far used only in Richards, where is OK
        if(QString::fromStdString(function.type()) == "function_1d")
            parameter = "value[0]";

        m_dict[QString::fromStdString(function.shortname())] = QString("%1->getValue(elementMarker, %2)").
                arg(QString::fromStdString(function.shortname())).arg(parameter);
    }

}

void ParserInstance::addFilterVariables()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (nonlinearExpr.isEmpty())
                // linear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
            else
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                        arg(QString::fromStdString(quantity.id())).
                        arg(Parser::parseFilterExpression(m_parserModuleInfo, nonlinearExpr, false));
        }
    }

    foreach (XMLModule::function function, m_parserModuleInfo.volume.function())
    {
        QString parameter("0");
        // todo: so far used only in Richards, where is OK
        if(QString::fromStdString(function.type()) == "function_1d")
            parameter = "value[0][i]";

        m_dict[QString::fromStdString(function.shortname())] = QString("%1->getValue(elementMarker, %2)").
                arg(QString::fromStdString(function.shortname())).arg(parameter);
    }

}


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------


void Parser::addPreviousSolutionsLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSolutions)
{
    for (int i = 1; i < numSolutions + 1; i++)
    {
        lex->addVariable(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
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
}

void Parser::addSourceCouplingLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSourceSolutions)
{
    for (int i = 1; i < numSourceSolutions + 1; i++)
    {
        lex->addVariable(QString("source%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex->addVariable(QString("source%1dx").arg(i));
            lex->addVariable(QString("source%1dy").arg(i));
        }
        else
        {
            lex->addVariable(QString("source%1dr").arg(i));
            lex->addVariable(QString("source%1dz").arg(i));
        }
    }

}


void Parser::addQuantitiesLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo)
{
    // constants
    lex->addVariable("PI");
    lex->addVariable("f");

    foreach (XMLModule::constant cnst, parserModuleInfo.constants.constant())
        lex->addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::function function, parserModuleInfo.volume.function())
    {
        lex->addVariable(QString::fromStdString(function.shortname()));
    }

    foreach (XMLModule::quantity quantity, parserModuleInfo.surface.quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

}


void Parser::addWeakFormLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo)
{
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
    if (parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        // scalar field
        lex->addVariable("udx");
        lex->addVariable("vdx");
        lex->addVariable("udy");
        lex->addVariable("vdy");
        lex->addVariable("updx");
        lex->addVariable("updy");

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

        lex->addVariable(QString("r"));
        lex->addVariable(QString("z"));
    }

    if (parserModuleInfo.analysisType == AnalysisType_Transient)
    {
        lex->addVariable("deltat");
        lex->addVariable("timedermat");
        lex->addVariable("timedervec");
    }

}

void Parser::addPostprocessorLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo)
{
    // coordinates
    if (parserModuleInfo.coordinateType == CoordinateType_Planar)
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

}
