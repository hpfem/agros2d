#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"

QMap<QString, int> Parser::quantityOrdering;
QMap<QString, bool> Parser::quantityIsNonlinear;
QMap<QString, int> Parser::functionOrdering;

class ValueGenerator
{
public:
    ValueGenerator(int initValue) : m_nextValue(initValue) {}
    QString value()
    {
        m_nextValue++;
        return QString::number(m_nextValue) + ".0"; // MSVC fix pow(int, double) doesn't work
    }

private:
    int m_nextValue;
};

ParserModuleInfo::ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType) : m_analysisType(analysisType), m_coordinateType(coordinateType),
    m_constants(field.constants()), m_volume(field.volume()), m_surface(field.surface())
{
    m_numSolutions = Agros2DGenerator::numberOfSolutions(field.general_field().analyses(), analysisType);
    m_id = QString::fromStdString(field.general_field().id());
    m_isField = true;
}

QString Parser::parsePostprocessorExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool includeVariables, bool forFilter)
{
    try
    {
        QMap<QString, QString> dict;

        // coordinates
        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            dict["x"] = "x[i]";
            dict["y"] = "y[i]";
            // surface integral
            dict["tanx"] = "e->tx[i]";
            dict["tany"] = "e->ty[i]";
            // velocity (force calculation)
            dict["velx"] = "velocity.x";
            dict["vely"] = "velocity.y";
            dict["velz"] = "velocity.z";
        }
        else
        {
            dict["r"] = "x[i]";
            dict["z"] = "y[i]";
            // surface integral
            dict["tanr"] = "e->tx[i]";
            dict["tanz"] = "e->ty[i]";
            // velocity (force calculation)
            dict["velr"] = "velocity.x";
            dict["velz"] = "velocity.y";
            dict["velphi"] = "velocity.z";
        }

        // constants
        dict["PI"] = "M_PI";
        dict["f"] = "m_fieldInfo->frequency()";
        foreach (XMLModule::constant cnst, parserModuleInfo.m_constants.constant())
            dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

        // functions
        for (int i = 1; i < parserModuleInfo.m_numSolutions + 1; i++)
        {
            dict[QString("value%1").arg(i)] = QString("value[%1][i]").arg(i-1);
            if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
            {
                dict[QString("dx%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
                dict[QString("dy%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
            }
            else
            {
                dict[QString("dr%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
                dict[QString("dz%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
            }
        }
        // eggshell
        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            dict["dxegg"] = "dudx[source_functions.size() - 1][i]";
            dict["dyegg"] = "dudy[source_functions.size() - 1][i]";
        }
        else
        {
            dict["dregg"] = "dudx[source_functions.size() - 1][i]";
            dict["dzegg"] = "dudy[source_functions.size() - 1][i]";
        }

        // variables
        if (includeVariables)
        {
            foreach (XMLModule::quantity quantity, parserModuleInfo.m_volume.quantity())
            {
                if (quantity.shortname().present())
                {
                    QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), parserModuleInfo);

                    if (nonlinearExpr.isEmpty())
                        // linear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
                    else
                        // nonlinear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                                arg(QString::fromStdString(quantity.id())).
                                arg(parsePostprocessorExpression(parserModuleInfo, nonlinearExpr, false));
                }
            }

            foreach (XMLModule::function function, parserModuleInfo.m_volume.function())
            {
                QString parameter("0");
                // todo: so far used only in Richards, where is OK
                if(QString::fromStdString(function.type()) == "function_1d")
                {
                    if(forFilter)
                        parameter = "value[0][i]";
                    else
                        parameter = "value[0]";
                }
                dict[QString::fromStdString(function.shortname())] = QString("%1->getValue(elementMarker, %2)").
                        arg(QString::fromStdString(function.shortname())).arg(parameter);
            }
        }

        LexicalAnalyser *lex = postprocessorLexicalAnalyser(parserModuleInfo);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        delete lex;

        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in module %2").arg(e.toString()).arg(parserModuleInfo.m_id).toLatin1());

        return "";
    }
}



QString Parser::parseWeakFormExpression(ParserModuleInfo parserModuleInfo, const QString &expr, bool includeVariables, bool errorCalculation)
{
    try
    {
        QMap<QString, QString> dict;

        // coordinates
        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            dict["x"] = "e->x[i]";
            dict["y"] = "e->y[i]";
            dict["tx"] = "e->tx[i]";
            dict["ty"] = "e->ty[i]";
            dict["nx"] = "e->nx[i]";
            dict["ny"] = "e->ny[i]";
        }
        else
        {
            dict["r"] = "e->x[i]";
            dict["z"] = "e->y[i]";
            dict["tr"] = "e->tx[i]";
            dict["tz"] = "e->ty[i]";
            dict["nr"] = "e->nx[i]";
            dict["nz"] = "e->ny[i]";
        }

        // constants
        dict["PI"] = "M_PI";
        dict["f"] = "this->m_markerSource->fieldInfo()->frequency()";
        foreach (XMLModule::constant cnst, parserModuleInfo.m_constants.constant())
            dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

        // area of a label
        // assumes, that this->getAreas has allways only one component (it is true at the moment, since in Agros we create one form for each label)
        dict["area"] = "this->markerVolume()";

        // functions
        // scalar field
        dict["uval"] = "u->val[i]";
        dict["vval"] = "v->val[i]";
        dict["upval"] = "u_ext[this->j]->val[i]";
        dict["uptval"] = "ext[*this->m_offsetPreviousTimeExt + this->j - this->m_offsetJ]->val[i]";
        dict["deltat"] = "Agros2D::problem()->actualTimeStepLength()";

        // vector field
        dict["uval0"] = "u->val0[i]";
        dict["uval1"] = "u->val1[i]";
        dict["ucurl"] = "u->curl[i]";
        dict["vval0"] = "v->val0[i]";
        dict["vval1"] = "v->val1[i]";
        dict["vcurl"] = "v->curl[i]";
        dict["upcurl"] = "u_ext[this->j]->curl[i]";

        dict["timedermat"] = "(*this->m_table)->matrixFormCoefficient()";
        dict["timedervec"] = "(*this->m_table)->vectorFormCoefficient(ext, this->j, this->m_markerSource->fieldInfo()->numberOfSolutions(), offset.prevSol, i)";

        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
        {
            // scalar field
            dict["udx"] = "u->dx[i]";
            dict["vdx"] = "v->dx[i]";
            dict["udy"] = "u->dy[i]";
            dict["vdy"] = "v->dy[i]";
            dict["updx"] = "u_ext[this->j]->dx[i]";
            dict["updy"] = "u_ext[this->j]->dy[i]";
        }
        else
        {
            // scalar field
            dict["udr"] = "u->dx[i]";
            dict["vdr"] = "v->dx[i]";
            dict["udz"] = "u->dy[i]";
            dict["vdz"] = "v->dy[i]";
            dict["updr"] = "u_ext[this->j]->dx[i]";
            dict["updz"] = "u_ext[this->j]->dy[i]";
        }

        for (int i = 1; i < parserModuleInfo.m_numSolutions + 1; i++)
        {
            if (errorCalculation)
            {
                // TODO: better !!!
                dict[QString("value%1").arg(i)] = QString("u->val[i]");

                if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
                {
                    dict[QString("dx%1").arg(i)] = QString("u->dx[i]");
                    dict[QString("dy%1").arg(i)] = QString("u->dy[i]");
                }
                else
                {
                    dict[QString("dr%1").arg(i)] = QString("u->dx[i]");
                    dict[QString("dz%1").arg(i)] = QString("u->dy[i]");
                }
            }
            else
            {
                dict[QString("value%1").arg(i)] = QString("u_ext[%1 + offset.forms]->val[i]").arg(i-1);
                dict[QString("timedervec%1").arg(i)] = QString("(*this->m_table)->vectorFormCoefficient(ext, %1, this->m_markerSource->fieldInfo()->numberOfSolutions(), offset.prevSol, i)").arg(i-1);

                if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
                {
                    dict[QString("dx%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dx[i]").arg(i-1);
                    dict[QString("dy%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dy[i]").arg(i-1);
                }
                else
                {
                    dict[QString("dr%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dx[i]").arg(i-1);
                    dict[QString("dz%1").arg(i)] = QString("u_ext[%1 + offset.forms]->dy[i]").arg(i-1);
                }
            }
        }

        // variables
        if (includeVariables)
        {
            foreach (XMLModule::quantity quantity, parserModuleInfo.m_volume.quantity())
            {
                if (quantity.shortname().present())
                {
                    if(errorCalculation)
                    {
                        QString dep = dependence(QString::fromStdString(quantity.id()), parserModuleInfo);
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), parserModuleInfo);

                        if (parserModuleInfo.m_linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
                        {
                            if (dep.isEmpty())
                            {
                                // linear material
                                dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                                        arg(QString::fromStdString(quantity.shortname().get()));
                            }
                            else if (dep == "time")
                            {
                                // linear boundary condition
                                // ERROR: Python expression evaluation doesn't work from weakform ("false" should be removed)
                                dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                                        arg(QString::fromStdString(quantity.shortname().get()));
                            }
                            else if (dep == "space")
                            {
                                // spacedep boundary condition
                                // ERROR: Python expression evaluation doesn't work from weakform - ERROR
                                dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(x, y))").
                                        arg(QString::fromStdString(quantity.shortname().get()));
                            }
                            else if (dep == "time-space")
                            {
                                // spacedep boundary condition
                                // ERROR: Python expression evaluation doesn't work from weakform - ERROR
                                dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(x, y))").
                                        arg(QString::fromStdString(quantity.shortname().get()));
                            }
                        }
                        else
                        {
                            // nonlinear material
                            dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberFromTable(%2)").
                                    arg(QString::fromStdString(quantity.shortname().get())).
                                    arg(parseWeakFormExpression(parserModuleInfo, nonlinearExpr, false, errorCalculation));

                            if (parserModuleInfo.m_linearityType == LinearityType_Newton)
                                dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("%1->derivativeFromTable(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(parserModuleInfo, nonlinearExpr, false, errorCalculation));
                        }
                    }
                    else{
                        // in weak forms, values replaced by ext functions
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("ext[%1 + offset.quant]->val[i]").
                                arg(quantityOrdering[QString::fromStdString(quantity.id())]);
                        if(quantityIsNonlinear[QString::fromStdString(quantity.id())])
                        {
                            dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("ext[%1 + offset.quant]->val[i]").
                                    arg(quantityOrdering[QString::fromStdString(quantity.id())] + 1);

                        }
                    }
                }
            }

            foreach (XMLModule::function function, parserModuleInfo.m_volume.function())
            {
                if(errorCalculation)
                {
                    // todo:
                }
                else{
                    // in weak forms, functions replaced by ext functions
                    dict[QString::fromStdString(function.shortname())] = QString("ext[%1 + offset.quant]->val[i]").
                            arg(functionOrdering[QString::fromStdString(function.id())]);
                }
            }


            foreach (XMLModule::quantity quantity, parserModuleInfo.m_surface.quantity())
            {
                if (quantity.shortname().present())
                {
                    QString dep = dependence(QString::fromStdString(quantity.id()), parserModuleInfo);

                    if (dep.isEmpty())
                    {
                        // linear boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                    else if (dep == "time")
                    {
                        // linear boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                    else if (dep == "space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(x, y))").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                    else if (dep == "time-space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(x, y))").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                }
            }
        }

        LexicalAnalyser *lex = weakFormLexicalAnalyser(parserModuleInfo);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        delete lex;

        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in module %2").arg(e.toString()).arg(parserModuleInfo.m_id).toLatin1());

        return "";
    }
}

QString Parser::parseWeakFormExpressionCheck(ParserModuleInfo parserModuleInfo, const QString &expr)
{
    try
    {
        QMap<QString, QString> dict;
        // TODO: remove
        ValueGenerator generator(1);

        // variables
        foreach (XMLModule::quantity quantity, parserModuleInfo.m_volume.quantity())
        {
            if (quantity.shortname().present())
            {
                QString dep = dependence(QString::fromStdString(quantity.id()), parserModuleInfo);
                QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), parserModuleInfo);

                if (parserModuleInfo.m_linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
                {
                    if (dep.isEmpty())
                    {
                        // linear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material->value(\"%1\")->number()").
                                arg(QString::fromStdString(quantity.id()));
                    }
                    else if (dep == "time")
                    {
                        // timedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                    else if (dep == "space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                    else if (dep == "time-space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                }
                else
                {
                    // nonlinear material
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    dict["d" + QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
            }
        }

        foreach (XMLModule::quantity quantity, parserModuleInfo.m_surface.quantity())
        {
            if (quantity.shortname().present())
            {
                QString dep = dependence(QString::fromStdString(quantity.id()), parserModuleInfo);

                if (dep.isEmpty())
                {
                    // linear boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = QString("boundary->value(\"%1\").number()").
                            arg(QString::fromStdString(quantity.id()));
                }
                else if (dep == "time")
                {
                    // timedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
                else if (dep == "space")
                {
                    // spacedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
                else if (dep == "time-space")
                {
                    // spacedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
            }
        }

        LexicalAnalyser *lex = weakFormLexicalAnalyser(parserModuleInfo);
        lex->setExpression(expr.isEmpty() ? "true" : expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        delete lex;

        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in module %2").arg(e.toString()).arg(parserModuleInfo.m_id).toLatin1());

        return "";
    }
}

QString Parser::nonlinearExpression(const QString &variable, ParserModuleInfo parserModuleInfo)
{
    // todo: this is slow !!!!
    // volume
    foreach (XMLModule::weakform_volume wf, parserModuleInfo.m_volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(parserModuleInfo.m_analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
                    {
                        if (quantityAnalysis.nonlinearity_planar().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                    }
                    else
                    {
                        if (quantityAnalysis.nonlinearity_axi().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                    }
                }
            }
        }
    }

    // surface
    foreach (XMLModule::weakform_surface wf, parserModuleInfo.m_surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisTypeToStringKey(parserModuleInfo.m_analysisType).toStdString())
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variable.toStdString())
                    {
                        if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
                        {
                            if (quantityAnalysis.nonlinearity_planar().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                        }
                        else
                        {
                            if (quantityAnalysis.nonlinearity_axi().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                        }
                    }
                }
            }
        }
    }

    return "";
}

QString Parser::specialFunctionNonlinearExpression(const QString &variable, ParserModuleInfo parserModuleInfo)
{
    foreach (XMLModule::weakform_volume wf, parserModuleInfo.m_volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(parserModuleInfo.m_analysisType).toStdString())
        {
            foreach (XMLModule::function_use functionUse, wf.function_use())
            {
                if (functionUse.id() == variable.toStdString())
                {
                    if (parserModuleInfo.m_coordinateType == CoordinateType_Planar)
                    {
                        if (functionUse.nonlinearity_planar().present())
                            return QString::fromStdString(functionUse.nonlinearity_planar().get());
                    }
                    else
                    {
                        if (functionUse.nonlinearity_axi().present())
                            return QString::fromStdString(functionUse.nonlinearity_axi().get());
                    }
                }
            }
        }
    }

    return "0";
}

QString Parser::dependence(const QString &variable, ParserModuleInfo parserModuleInfo)
{
    // todo: this is slow !!!!

    // volume
    foreach (XMLModule::weakform_volume wf, parserModuleInfo.m_volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(parserModuleInfo.m_analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (quantityAnalysis.dependence().present())
                        return QString::fromStdString(quantityAnalysis.dependence().get());
                }
            }
        }
    }

    // surface
    foreach (XMLModule::weakform_surface wf, parserModuleInfo.m_surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisTypeToStringKey(parserModuleInfo.m_analysisType).toStdString())
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variable.toStdString())
                    {
                        if (quantityAnalysis.dependence().present())
                            return QString::fromStdString(quantityAnalysis.dependence().get());
                    }
                }
            }
        }
    }

    return "";
}
