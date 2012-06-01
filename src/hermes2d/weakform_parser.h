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

class Parser;
class Boundary;
class Material;
class FieldInfo;
class CouplingInfo;

namespace Hermes
{
namespace Hermes2D
{
    template<typename Scalar> class Func;
}
}

namespace Module
{
    struct MaterialTypeVariable;
}

struct ParserFormEssential
{
    ParserFormEssential() : i(0), expressionLinear(""), expressionNewton("") {}
    ParserFormEssential(int i, string expr_linear, string expr_newton) : i(i), expressionLinear(expr_linear), expressionNewton(expr_newton) {}

    // position
    int i;

    // expression
    std::string expressionLinear;
    std::string expressionNewton;
};

// instead of ParserFormVector and ParserFormMatrix
struct ParserFormExpression
{
    ParserFormExpression() : i(0), j(0), sym(Hermes::Hermes2D::HERMES_NONSYM), expressionLinear(""), expressionNewton("") {}
    ParserFormExpression(int i, int j, string expr_linear, string expr_newton, Hermes::Hermes2D::SymFlag sym = Hermes::Hermes2D::HERMES_NONSYM)
        : i(i), j(j), expressionLinear(expr_linear), expressionNewton(expr_newton), sym(sym) {}

    // position
    int i;
    int j;

    // symmetric flag
    Hermes::Hermes2D::SymFlag sym;

    // expression
    std::string expressionLinear;
    std::string expressionNewton;
};

const int maxSourceFieldComponents = 2;

class ParserForm
{
protected:
    // parser
    mutable mu::Parser *m_parser;
    mutable QMap<std::string, double> m_parserVariables;

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

    // coupled field
    mutable double source[maxSourceFieldComponents];
    mutable double sourcedx[maxSourceFieldComponents];
    mutable double sourcedy[maxSourceFieldComponents];

    // nonlinearities
    // coordinates
    mutable double pnlx;
    mutable double pnly;

    // current solution
    mutable double *pnlvalue;
    mutable double *pnldx;
    mutable double *pnldy;
    mutable QMap<Module::MaterialTypeVariable *, mu::Parser *> m_parsersNonlinear;

    // TODO: not good
    ParserForm(FieldInfo *fieldInfo = NULL, CouplingInfo *couplingInfo = NULL);
    ~ParserForm();

    void initParserField(const std::string &expr);
    void initParserCoupling(const std::string &expr);

    void setMaterialToParser(Material * material);
    void setMaterialsToParser(QList<Material *> materials);
    void setBoundaryToParser(Boundary * boundary);
    void setBoundariesToParser(QList<Boundary *> boundaries);

    // nonlinearities
    void setNonlinearParsers();
    void setNonlinearMaterial(Material *material, int offset, int index, double *x, double *y, Hermes::Hermes2D::Func<double> *u_ext[]) const;

protected:
    FieldInfo *m_fieldInfo;
    CouplingInfo *m_couplingInfo;

private:
    void initParser();
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
                              Material *materialSource,
                              Material *materialTarget);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                         Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                            Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;

    CustomParserMatrixFormVol<Scalar>* clone();

    LinearityType linearityType() const;

private:
    Material *m_materialSource;
    Material *m_materialTarget;
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
                              Material *materialSource,
                              Material *materialTarget);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;

    CustomParserVectorFormVol<Scalar>* clone();

    LinearityType linearityType() const;

private:
    Material *m_materialSource;
    Material *m_materialTarget;

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
