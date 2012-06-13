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

#include "../weakform/src/weakform_factory.h"
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

// *********************************************************************************************************************************************

template <typename Scalar>
ViewScalarFilter<Scalar>::ViewScalarFilter(FieldInfo *fieldInfo,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                                           QString expression)
    : Hermes::Hermes2D::Filter<Scalar>(sln), PostprocessorValue(fieldInfo)
{
    initParser(expression);
}

template <typename Scalar>
ViewScalarFilter<Scalar>::~ViewScalarFilter()
{
    delete [] pvalue;
    delete [] pdx;
    delete [] pdy;
}

template <typename Scalar>
void ViewScalarFilter<Scalar>::initParser(const QString &expression)
{
    m_parsers.append(m_fieldInfo->module()->expressionParser());

    m_parsers[0]->SetExpr(expression.toStdString());
    m_parsers[0]->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
    m_parsers[0]->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

    // value and derivatives
    pvalue = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdx = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdy = new double[Hermes::Hermes2D::Filter<Scalar>::num];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        std::stringstream number;
        number << (k+1);

        m_parsers[0]->DefineVar("value" + number.str(), &pvalue[k]);
        m_parsers[0]->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
        m_parsers[0]->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
    }

    // set nonlinear parsers
    setNonlinearParsers();

    // fill material variables map
    foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        m_parserVariables[variable->shortname().toStdString()] = 0.0;

    // register value address
    for (QMap<std::string, double>::iterator it = m_parserVariables.begin(); it != m_parserVariables.end(); ++it)
        m_parsers[0]->DefineVar(it.key(), &it.value());
}

template <typename Scalar>
double ViewScalarFilter<Scalar>::get_pt_value(double x, double y, int item)
{
    return 0.0;
}

template <typename Scalar>
void ViewScalarFilter<Scalar>::precalculate(int order, int mask)
{
    bool isLinear = (m_fieldInfo->linearityType() == LinearityType_Linear);

    Hermes::Hermes2D::Quad2D* quad = Hermes::Hermes2D::Filter<Scalar>::quads[Hermes::Hermes2D::Function<Scalar>::cur_quad];
    int np = quad->get_num_points(order, this->get_active_element()->get_mode());
    node = Hermes::Hermes2D::Function<Scalar>::new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        Hermes::Hermes2D::Filter<Scalar>::sln[k]->set_quad_order(order, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
        Hermes::Hermes2D::Filter<Scalar>::sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
        value[k] = Hermes::Hermes2D::Filter<Scalar>::sln[k]->get_fn_values();
    }

    Hermes::Hermes2D::Filter<Scalar>::update_refmap();

    double *x = Hermes::Hermes2D::MeshFunction<Scalar>::refmap->get_phys_x(order);
    double *y = Hermes::Hermes2D::MeshFunction<Scalar>::refmap->get_phys_y(order);
    Hermes::Hermes2D::Element *e = Hermes::Hermes2D::MeshFunction<Scalar>::refmap->get_active_element();

    // set material
    SceneMaterial *material = Util::scene()->labels->at(atoi(Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().
                                                             get_user_marker(e->marker).marker.c_str()))->marker(m_fieldInfo);

    foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        if (isLinear || variable->expressionNonlinear().isEmpty())
            m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).number();

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }

        // init nonlinear material
        setNonlinearMaterial(material);

        // parse expression
        try
        {
            node->values[0][0][i] = m_parsers[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "Scalar view: " << e.GetMsg() << std::endl;
        }
    }

    delete [] value;
    delete [] dudx;
    delete [] dudy;

    if (Hermes::Hermes2D::Function<Scalar>::nodes->present(order))
    {
        assert(Hermes::Hermes2D::Function<Scalar>::nodes->get(order) == Hermes::Hermes2D::Function<Scalar>::cur_node);
        ::free(Hermes::Hermes2D::Function<Scalar>::nodes->get(order));
    }
    Hermes::Hermes2D::Function<Scalar>::nodes->add(node, order);
    Hermes::Hermes2D::Function<Scalar>::cur_node = node;
}

template <typename Scalar>
ViewScalarFilter<Scalar>* ViewScalarFilter<Scalar>::clone()
{
    Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> slns;

    for (int i = 0; i < this->num; i++)
        slns.push_back(this->sln[i]->clone());

    return new ViewScalarFilter(m_fieldInfo, slns, QString::fromStdString(m_parsers[0]->GetExpr()));
}

template class ViewScalarFilter<double>;
