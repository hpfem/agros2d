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


void PostprocessorValue::setMaterialToParsers(Material *material)
{
    parserVariables.clear();

    // set material
    foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        parserVariables[variable->shortname.toStdString()] = material->getValue(variable->id).number();

    // register value address
    for (QMap<std::string, double>::iterator itv = parserVariables.begin(); itv != parserVariables.end(); ++itv)
        foreach (mu::Parser *pars, parsers)
            pars->DefineVar(itv.key(), &itv.value());
}

PostprocessorValue::~PostprocessorValue()
{
    parserVariables.clear();
}

// *********************************************************************************************************************************************

void PostprocessorIntegralValue::initParser(QList<Module::Integral *> list)
{
    foreach (Module::Integral *integral, list)
    {
        parsers.push_back(m_fieldInfo->module()->expressionParser(integral->expression));
        m_values[integral] = 0.0;
    }
}

// *********************************************************************************************************************************************

template <typename Scalar>
ViewScalarFilter<Scalar>::ViewScalarFilter(FieldInfo *fieldInfo,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                                           QString expression)
    : Hermes::Hermes2D::Filter<Scalar>(sln), m_fieldInfo(fieldInfo)
{
    initParser(expression);
}

template <typename Scalar>
ViewScalarFilter<Scalar>::~ViewScalarFilter()
{
    delete parser;

    delete [] pvalue;
    delete [] pdx;
    delete [] pdy;
}

template <typename Scalar>
void ViewScalarFilter<Scalar>::initParser(const QString &expression)
{
    parser = m_fieldInfo->module()->expressionParser();

    parser->SetExpr(expression.toStdString());

    parser->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
    parser->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

    // value and derivatives
    pvalue = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdx = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdy = new double[Hermes::Hermes2D::Filter<Scalar>::num];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        std::stringstream number;
        number << (k+1);

        parser->DefineVar("value" + number.str(), &pvalue[k]);
        parser->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
        parser->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
    }

    // fill material variables map
    foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        parserVariables[variable->shortname.toStdString()] = 0.0;

    // register value address
    for (QMap<std::string, double>::iterator it = parserVariables.begin(); it != parserVariables.end(); ++it)
        parser->DefineVar(it.key(), &it.value());
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
    int np = quad->get_num_points(order, Hermes::Hermes2D::HERMES_MODE_TRIANGLE) + quad->get_num_points(order, Hermes::Hermes2D::HERMES_MODE_QUAD);
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
                                                             get_user_marker(e->marker).marker.c_str()))->getMarker(m_fieldInfo);
    if (isLinear)
        foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
            parserVariables[variable->shortname.toStdString()] = material->getValue(variable->id).number();

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }

        /*
        if (!isLinear)
            for (QMap<std::string, double>::iterator it = parserVariables.begin(); it != parserVariables.end(); ++it)
                parserVariables[it.key()] = material->getValue(QString::fromStdString(it.key())).value(0.0); // TODO: 0.0
        */

        // parse expression
        try
        {
            node->values[0][0][i] = parser->Eval();
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
        slns.push_back(this->sln[i]);

    return new ViewScalarFilter(m_fieldInfo, slns, QString::fromStdString(parser->GetExpr()));
}

template class ViewScalarFilter<double>;
