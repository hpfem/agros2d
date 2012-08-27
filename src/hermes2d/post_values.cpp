// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "post_values.h"
#include "module.h"
#include "module_agros.h"
#include "field.h"
#include "block.h"
#include "problem.h"
#include "logview.h"

#include "util.h"
#include "scene.h"
#include "scenelabel.h"

#include "hermes2d/weakform_interface.h"
#include "solutionstore.h"

#include <muParserDef.h>

PostprocessorValue::~PostprocessorValue()
{
    // delete parsers
    foreach (mu::Parser *parser, m_parsers)
        delete parser;
    m_parsers.clear();

    // delete parsers
    foreach (mu::Parser *parser, m_parsersNonlinear.values())
        delete parser;
    m_parsersNonlinear.clear();

    m_parserVariables.clear();
}

void PostprocessorValue::setMaterialToParsers(Material *material)
{
    m_parserVariables.clear();

    // set material
    foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        if (m_parserVariables.keys().indexOf(variable->shortname().toStdString()) == -1)
            if ((m_fieldInfo->linearityType() == LinearityType_Linear) ||
                    ((m_fieldInfo->linearityType() != LinearityType_Linear) && (m_parsers.length() > 0) && variable->expressionNonlinear().isEmpty()))
                // linear variable
                m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).number();
            else
                // nonlinear variable
                m_parserVariables[variable->shortname().toStdString()] = 0.0;

    // register value address
    for (QMap<std::string, double>::iterator itv = m_parserVariables.begin(); itv != m_parserVariables.end(); ++itv)
        foreach (mu::Parser *pars, m_parsers)
            pars->DefineVar(itv.key(), &itv.value());
}

void PostprocessorValue::setNonlinearParsers()
{
    if ((m_fieldInfo->linearityType() != LinearityType_Linear) && (m_parsers.length() > 0))
    {
        foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        {
            if (!variable->expressionNonlinear().isEmpty())
            {
                mu::Parser *parser = m_fieldInfo->module()->expressionParser();
                parser->SetExpr(variable->expressionNonlinear().toStdString());

                // get variables
                for (std::map<std::string, double *>::const_iterator item = m_parsers[0]->GetVar().begin(); item != m_parsers[0]->GetVar().end(); ++item)
                    parser->DefineVar(item->first, item->second);

                m_parsersNonlinear[variable] = parser;
            }
        }
    }
}

void PostprocessorValue::setNonlinearMaterial(Material *material)
{
    if (m_fieldInfo->linearityType() != LinearityType_Linear)
    {
        foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        {
            if (!variable->expressionNonlinear().isEmpty())
            {
                try
                {
                    double nonlinValue = m_parsersNonlinear[variable]->Eval();
                    m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).value(nonlinValue);
                }
                catch (mu::Parser::exception_type &e)
                {
                    std::cout << "Nonlinear value '" << variable->id().toStdString() << "'): " << e.GetMsg() << std::endl;
                }
            }
        }
    }
}

// *********************************************************************************************************************************************

void PostprocessorIntegralValue::initParser(QList<Module::Integral *> list)
{
    foreach (Module::Integral *integral, list)
    {
        m_parsers.push_back(m_fieldInfo->module()->expressionParser(integral->expression()));
        m_values[integral] = 0.0;
    }
}

// *********************************************************************************************************************************************

LocalForceValue::LocalForceValue(FieldInfo *fieldInfo) : PostprocessorValue(fieldInfo)
{
    pvalue = new double[m_fieldInfo->module()->numberOfSolutions()];
    pdx = new double[m_fieldInfo->module()->numberOfSolutions()];
    pdy = new double[m_fieldInfo->module()->numberOfSolutions()];

    // parser variables
    m_parsers.push_back(addParser(m_fieldInfo->module()->force().compX()));
    m_parsers.push_back(addParser(m_fieldInfo->module()->force().compY()));
    m_parsers.push_back(addParser(m_fieldInfo->module()->force().compZ()));
}

LocalForceValue::~LocalForceValue()
{
    delete [] pvalue;
    delete [] pdx;
    delete [] pdy;
}

mu::Parser *LocalForceValue::addParser(const QString &exp)
{
    mu::Parser *parser = m_fieldInfo->module()->expressionParser();
    parser->SetExpr(exp.toStdString());

    parser->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
    parser->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

    parser->DefineVar("velx", &pvelx);
    parser->DefineVar("vely", &pvely);
    parser->DefineVar("velz", &pvelz);

    for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
    {
        std::stringstream number;
        number << (k+1);

        parser->DefineVar("value" + number.str(), &pvalue[k]);
        parser->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
        parser->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
    }

    return parser;
}

Point3 LocalForceValue::calculate(const Point3 &point, const Point3 &velocity)
{
    Point3 res;

    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());
        m_fieldInfo->module()->updateTimeFunctions(timeLevels[Util::scene()->activeTimeStep()]);
    }

    if (Util::problem()->isSolved())
    {
        int index = findElementInMesh(Util::problem()->meshInitial(m_fieldInfo), Point(point.x, point.y));
        if (index != -1)
        {
            // find marker
            Hermes::Hermes2D::Element *e = Util::problem()->meshInitial(m_fieldInfo)->get_element_fast(index);
            SceneLabel *label = Util::scene()->labels->at(atoi(Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()));
            SceneMaterial *material = label->marker(m_fieldInfo);

            // set variables
            px = point.x;
            py = point.y;

            pvelx = velocity.x;
            pvely = velocity.y;
            pvelz = velocity.z;

            // set material variables
            setMaterialToParsers(material);

            // add nonlinear parsers
            setNonlinearParsers();

            std::vector<Hermes::Hermes2D::Solution<double> *> sln(m_fieldInfo->module()->numberOfSolutions());
            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
            {
                FieldSolutionID fsid(m_fieldInfo, Util::scene()->activeTimeStep(), Util::scene()->activeAdaptivityStep(), Util::scene()->activeSolutionType());
                sln[k] = Util::solutionStore()->multiSolution(fsid).component(k).sln.data();

                double value;
                if ((m_fieldInfo->analysisType() == AnalysisType_Transient) && Util::scene()->activeTimeStep() == 0)
                    // const solution at first time step
                    value = m_fieldInfo->initialCondition().number();
                else
                    value = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_VAL_0);

                Point derivative;
                derivative.x = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_DX_0);
                derivative.y = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_DY_0);

                // set variables
                pvalue[k] = value;
                pdx[k] = derivative.x;
                pdy[k] = derivative.y;
            }

            // init nonlinear material
            setNonlinearMaterial(material);

            try
            {
                res.x = m_parsers[0]->Eval();
            }
            catch (mu::Parser::exception_type &e)
            {
                qDebug() << "Local force value X: " << QString::fromStdString(m_parsers[0]->GetExpr()) << " - " << QString::fromStdString(e.GetMsg());
            }
            try
            {
                res.y = m_parsers[1]->Eval();
            }
            catch (mu::Parser::exception_type &e)
            {
                qDebug() << "Local force value Y: " << QString::fromStdString(m_parsers[1]->GetExpr()) << " - " << QString::fromStdString(e.GetMsg());
            }
            try
            {
                res.z = m_parsers[2]->Eval();
            }
            catch (mu::Parser::exception_type &e)
            {
                qDebug() << "Local force value Z: " << QString::fromStdString(m_parsers[2]->GetExpr()) << " - " << QString::fromStdString(e.GetMsg());
            }
        }
    }

    return res;
}

