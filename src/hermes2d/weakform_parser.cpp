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

#include "weakform_parser.h"

#include "util.h"
#include "hermes2d.h"
#include "module.h"

// **************************************************************************

ParserFormMatrix::ParserFormMatrix(rapidxml::xml_node<> *node, ProblemType problem_type)
{
    i = atoi(node->first_attribute("i")->value());
    j = atoi(node->first_attribute("j")->value());

    sym = HERMES_NONSYM;
    if (node->first_attribute("symmetric"))
        if (atoi(node->first_attribute("symmetric")->value()))
            sym = HERMES_SYM;

    if (problem_type == ProblemType_Planar)
    {
        if (node->first_attribute("planar"))
            expression = node->first_attribute("planar")->value();
    }
    else
    {
        if (node->first_attribute("axi"))
            expression = node->first_attribute("axi")->value();
    }
}

ParserFormVector::ParserFormVector(rapidxml::xml_node<> *node, ProblemType problem_type)
{
    i = atoi(node->first_attribute("i")->value());

    if (problem_type == ProblemType_Planar)
    {
        if (node->first_attribute("planar"))
            expression = node->first_attribute("planar")->value();
    }
    else
    {
        if (node->first_attribute("axi"))
            expression = node->first_attribute("axi")->value();
    }
}

ParserForm::ParserForm()
{
    parser = new Parser();
}

ParserForm::~ParserForm()
{
    delete parser;
}

void ParserForm::initParser(Material *material, Boundary *boundary)
{
    parser->parser.push_back(Util::scene()->problemInfo()->module()->get_parser());


    parser->parser[0]->DefineVar("x", &px);
    parser->parser[0]->DefineVar("y", &py);

    parser->parser[0]->DefineVar("uval", &puval);
    parser->parser[0]->DefineVar("udx", &pudx);
    parser->parser[0]->DefineVar("udy", &pudy);

    parser->parser[0]->DefineVar("vval", &pvval);
    parser->parser[0]->DefineVar("vdx", &pvdx);
    parser->parser[0]->DefineVar("vdy", &pvdy);

    parser->setParserVariables(material, boundary);

    for (std::map<std::string, double>::iterator it = parser->parser_variables.begin(); it != parser->parser_variables.end(); ++it)
        parser->parser[0]->DefineVar(it->first, &it->second);
}

// **********************************************************************************************

CustomParserMatrixFormVol::CustomParserMatrixFormVol(unsigned int i, unsigned int j,
                                                     std::string area,
                                                     SymFlag sym,
                                                     std::string expression,
                                                     Material *material)
    : WeakForm::MatrixFormVol(i, j, sym, area), ParserForm()
{
    initParser(material, NULL);

    parser->parser[0]->SetExpr(expression);
}

scalar CustomParserMatrixFormVol::value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                                        Geom<double> *e, ExtData<scalar> *ext)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        puval = u->val[i];
        pudx = u->dx[i];
        pudy = u->dy[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        // result += wt[i] * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]);
        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

Ord CustomParserMatrixFormVol::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                                   Geom<Ord> *e, ExtData<Ord> *ext) const
{
    return Ord(6);
}

CustomParserVectorFormVol::CustomParserVectorFormVol(unsigned int i,
                                                     std::string area, std::string expression,
                                                     Material *material)
    : WeakForm::VectorFormVol(i, area), ParserForm()
{
    initParser(material, NULL);

    parser->parser[0]->SetExpr(expression);
}

scalar CustomParserVectorFormVol::value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                                        Geom<double> *e, ExtData<scalar> *ext)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

Ord CustomParserVectorFormVol::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                                   ExtData<Ord> *ext) const
{
    return Ord(6);
}

// **********************************************************************************************

CustomParserMatrixFormSurf::CustomParserMatrixFormSurf(unsigned int i, unsigned int j,
                                                     std::string area, std::string expression,
                                                     Boundary *boundary)
    : WeakForm::MatrixFormSurf(i, j, area), ParserForm()
{
    initParser(NULL, boundary);

    parser->parser[0]->SetExpr(expression);
}

scalar CustomParserMatrixFormSurf::value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                                         Geom<double> *e, ExtData<scalar> *ext)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        puval = u->val[i];
        pudx = u->dx[i];
        pudy = u->dy[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

Ord CustomParserMatrixFormSurf::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                                    Geom<Ord> *e, ExtData<Ord> *ext) const
{
    return Ord(6);
}

CustomParserVectorFormSurf::CustomParserVectorFormSurf(unsigned int i,
                                                     std::string area, std::string expression,
                                                     Boundary *boundary)
    : WeakForm::VectorFormSurf(i, area), ParserForm()
{
    initParser(NULL, boundary);

    parser->parser[0]->SetExpr(expression);
}

scalar CustomParserVectorFormSurf::value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                                         Geom<double> *e, ExtData<scalar> *ext)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

Ord CustomParserVectorFormSurf::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v,
                                    Geom<Ord> *e, ExtData<Ord> *ext) const
{
    return Ord(10);
}
