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

    sym = Hermes::Hermes2D::HERMES_NONSYM;
    if (node->first_attribute("symmetric"))
        if (atoi(node->first_attribute("symmetric")->value()))
            sym = Hermes::Hermes2D::HERMES_SYM;

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

    // coordinates
    parser->parser[0]->DefineVar("x", &px);
    parser->parser[0]->DefineVar("y", &py);

    // current solution
    parser->parser[0]->DefineVar("uval", &puval);
    parser->parser[0]->DefineVar("udx", &pudx);
    parser->parser[0]->DefineVar("udy", &pudy);

    // test function
    parser->parser[0]->DefineVar("vval", &pvval);
    parser->parser[0]->DefineVar("vdx", &pvdx);
    parser->parser[0]->DefineVar("vdy", &pvdy);

    // previous solution
    parser->parser[0]->DefineVar("upval", &pupval);
    parser->parser[0]->DefineVar("updx", &pupdx);
    parser->parser[0]->DefineVar("updy", &pupdy);

    parser->setParserVariables(material, boundary);

    for (std::map<std::string, double>::iterator it = parser->parser_variables.begin(); it != parser->parser_variables.end(); ++it)
        parser->parser[0]->DefineVar(it->first, &it->second);
}

// **********************************************************************************************

template <typename Scalar>
CustomParserMatrixFormVol<Scalar>::CustomParserMatrixFormVol(unsigned int i, unsigned int j,
                                                             std::string area,
                                                             Hermes::Hermes2D::SymFlag sym,
                                                             std::string expression,
                                                             Material *material)
    : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j, area, sym), ParserForm()
{
    initParser(material, NULL);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserMatrixFormVol<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                                Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext)
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

        // previous solution
        pupval = u_ext[this->j]->val[i];
        pupdx = u_ext[this->j]->dx[i];
        pupdy = u_ext[this->j]->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserMatrixFormVol<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                                   Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext)
{
    return Hermes::Ord(10);
}

template <typename Scalar>
CustomParserVectorFormVol<Scalar>::CustomParserVectorFormVol(unsigned int i,
                                                             std::string area, std::string expression,
                                                             Material *material)
    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), ParserForm()
{
    initParser(material, NULL);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserVectorFormVol<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                                Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        // previous solution
        pupval = u_ext[this->i]->val[i];
        pupdx = u_ext[this->i]->dx[i];
        pupdy = u_ext[this->i]->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserVectorFormVol<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                                   Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext)
{
    return Hermes::Ord(10);
}

// **********************************************************************************************

template <typename Scalar>
CustomParserMatrixFormSurf<Scalar>::CustomParserMatrixFormSurf(unsigned int i, unsigned int j,
                                                               std::string area, std::string expression,
                                                               Boundary *boundary)
    : Hermes::Hermes2D::MatrixFormSurf<Scalar>(i, j, area), ParserForm()
{
    initParser(NULL, boundary);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserMatrixFormSurf<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u, Hermes::Hermes2D::Func<double> *v,
                                                 Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext)
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

        // previous solution
        pupval = u_ext[this->j]->val[i];
        pupdx = u_ext[this->j]->dx[i];
        pupdy = u_ext[this->j]->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserMatrixFormSurf<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u, Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                                    Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext)
{
    return Hermes::Ord(10);
}

template <typename Scalar>
CustomParserVectorFormSurf<Scalar>::CustomParserVectorFormSurf(unsigned int i,
                                                               std::string area, std::string expression,
                                                               Boundary *boundary)
    : Hermes::Hermes2D::VectorFormSurf<Scalar>(i, area), ParserForm()
{
    initParser(NULL, boundary);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserVectorFormSurf<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                                 Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        // previous solution
        pupval = u_ext[this->i]->val[i];
        pupdx = u_ext[this->i]->dx[i];
        pupdy = u_ext[this->i]->dy[i];

        result += wt[i] * parser->parser[0]->Eval();
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserVectorFormSurf<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                                    Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext)
{
    return Hermes::Ord(10);
}

template class CustomParserMatrixFormVol<double>;
template class CustomParserMatrixFormSurf<double>;
template class CustomParserVectorFormVol<double>;
template class CustomParserVectorFormSurf<double>;

