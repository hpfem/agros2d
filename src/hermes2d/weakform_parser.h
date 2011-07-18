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

#ifndef WEAKFORM_PARSER_H
#define WEAKFORM_PARSER_H

#include "util.h"
#include <weakform/weakform.h>
#include "boundary.h"
#include "material.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

class Parser;

struct ParserFormMatrix
{
    ParserFormMatrix() : i(0), j(0), expression("") {}
    ParserFormMatrix(rapidxml::xml_node<> *node, ProblemType problem_type);

    // position
    int i;
    int j;

    // expression
    std::string expression;
};

struct ParserFormVector
{
    ParserFormVector() : i(0), expression("") {}
    ParserFormVector(rapidxml::xml_node<> *node, ProblemType problem_type);

    // position
    int i;

    // expression
    std::string expression;
};

class ParserForm
{
public:
    Parser *parser;

    double px;
    double py;
    double puval;
    double pudx;
    double pudy;
    double pvval;
    double pvdx;
    double pvdy;

    ParserForm();
    ~ParserForm();

    void initParser(Material *material, Boundary *boundary);
};

// **********************************************************************************************

class CustomParserMatrixFormVol : public WeakForm::MatrixFormVol, public ParserForm
{
public:
    CustomParserMatrixFormVol(unsigned int i, unsigned int j,
                              std::string area, std::string expression,
                              Material *material);

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                         Geom<double> *e, ExtData<scalar> *ext);
    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                    Geom<Ord> *e, ExtData<Ord> *ext) const;
};

class CustomParserVectorFormVol : public WeakForm::VectorFormVol, public ParserForm
{
public:
    CustomParserVectorFormVol(unsigned int i,
                              std::string area, std::string expression,
                              Material *material);

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                         Geom<double> *e, ExtData<scalar> *ext);
    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                    ExtData<Ord> *ext) const;
};

// **********************************************************************************************

class CustomParserMatrixFormSurf : public WeakForm::MatrixFormSurf, public ParserForm
{
public:
    CustomParserMatrixFormSurf(unsigned int i, unsigned int j,
                              std::string area, std::string expression,
                              Boundary *boundary);

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                         Geom<double> *e, ExtData<scalar> *ext);
    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                    Geom<Ord> *e, ExtData<Ord> *ext) const;
};

class CustomParserVectorFormSurf : public WeakForm::VectorFormSurf, public ParserForm
{
public:
    CustomParserVectorFormSurf(unsigned int i,
                              std::string area, std::string expression,
                              Boundary *boundary);

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                         Geom<double> *e, ExtData<scalar> *ext);
    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v,
                      Geom<Ord> *e, ExtData<Ord> *ext) const;
};

#endif // WEAKFORM_PARSER_H
