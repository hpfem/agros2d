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

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

#include "util.h"

class Parser;
class Boundary;
class Material;
class FieldInfo;
class CouplingInfo;

struct ParserFormEssential
{
    ParserFormEssential() : i(0), expression("") {}
    ParserFormEssential(int i, std::string expr) : i(i), expression(expr) {}

    // position
    int i;

    // expression
    std::string expression;
};

// instead of ParserFormVector and ParserFormMatrix
struct ParserFormExpression
{
    ParserFormExpression() : i(0), j(0), sym(Hermes::Hermes2D::HERMES_NONSYM), expression("") {}
    ParserFormExpression(int i, int j, Hermes::Hermes2D::SymFlag sym, string expr) : i(i), j(j), sym(sym), expression(expr) {}

    // position
    int i;
    int j;

    // symmetric flag
    Hermes::Hermes2D::SymFlag sym;

    // expression
    std::string expression;   
};

const int maxSourceFieldComponents = 2;

class ParserForm
{
public:
    mutable Parser *parser;

    // coordinates
    mutable double px;
    mutable double py;

    // current solution
    mutable double puval;
    mutable double pudx;
    mutable double pudy;

    // test function
    mutable double pvval;
    mutable double pvdx;
    mutable double pvdy;

    // previous solution (in Newton method)
    mutable double pupval;
    mutable double pupdx;
    mutable double pupdy;

    // solution from the previous time level
    mutable double puptval;
    mutable double puptdx;
    mutable double puptdy;

    // time step
    mutable double pdeltat;

    //coupled field
    mutable double source[maxSourceFieldComponents];
    mutable double sourcedx[maxSourceFieldComponents];
    mutable double sourcedy[maxSourceFieldComponents];

    void initParserForm(FieldInfo *fieldInfo);
    void initParserForm(CouplingInfo *couplingInfo);
    ~ParserForm();

    void initParser(Hermes::vector<Material *> materials, Boundary *boundary);

protected:
    FieldInfo *m_fieldInfo;
    CouplingInfo *m_couplingInfo;
};

// **********************************************************************************************

// todo: number of parameters of constructors has grown due to slightly different behaviour for standard and coupling forms
// it might be done more correctly by employing polymorphism/templates, but the code might not be more readable....
template<typename Scalar>
class CustomParserMatrixFormVol : public Hermes::Hermes2D::MatrixFormVol<Scalar>, public ParserForm
{
public:
    CustomParserMatrixFormVol(unsigned int i, unsigned int j,
                              std::string area,
                              Hermes::Hermes2D::SymFlag sym,
                              std::string expression,
                              FieldInfo *fieldInfo,
                              CouplingInfo *couplingInfo,
                              Material *material1,
                              Material *material2);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                         Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                            Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;

    CustomParserMatrixFormVol<Scalar>* clone();

    LinearityType linearityType() const;

private:
    FieldInfo *m_fieldInfo;
    CouplingInfo *m_couplingInfo;
    Material *m_material1;
    Material *m_material2;
};

template<typename Scalar>
class CustomParserVectorFormVol : public Hermes::Hermes2D::VectorFormVol<Scalar>, public ParserForm
{
public:
    CustomParserVectorFormVol(unsigned int i, unsigned int j,
                              std::string area,
                              std::string expression,
                              FieldInfo *fieldInfo,
                              CouplingInfo *couplingInfo,
                              Material *material1,
                              Material *material2);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;

    CustomParserVectorFormVol<Scalar>* clone();

    LinearityType linearityType() const;

private:
    FieldInfo *m_fieldInfo;
    CouplingInfo *m_couplingInfo;
    Material *m_material1;
    Material *m_material2;

    unsigned int j;
};

// **********************************************************************************************

template<typename Scalar>
class CustomParserMatrixFormSurf : public Hermes::Hermes2D::MatrixFormSurf<Scalar>, public ParserForm
{
public:
    CustomParserMatrixFormSurf(unsigned int i, unsigned int j,
                               std::string area, std::string expression,
                               Boundary *boundary);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u, Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u, Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;

    CustomParserMatrixFormSurf<Scalar>* clone();

private:
    Boundary *m_boundary;
};

template<typename Scalar>
class CustomParserVectorFormSurf : public Hermes::Hermes2D::VectorFormSurf<Scalar>, public ParserForm
{
public:
    CustomParserVectorFormSurf(unsigned int i, unsigned int j,
                               std::string area, std::string expression,
                               Boundary *boundary);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;

    CustomParserVectorFormSurf<Scalar>* clone();

private:
    Boundary *m_boundary;

    unsigned int j;
};

// *************************************************************************************************

template<typename Scalar>
class CustomExactSolution : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>, public ParserForm
{
public:
    CustomExactSolution(Hermes::Hermes2D::Mesh *mesh, std::string expression, Boundary *boundary);

    Scalar value(double x, double y) const;
    void derivatives (double x, double y, Scalar& dx, Scalar& dy) const;

    Hermes::Ord ord (Hermes::Ord x, Hermes::Ord y) const
    {
        return Hermes::Ord(Hermes::Ord::get_max_order());
    }
};

#endif // WEAKFORM_PARSER_H
