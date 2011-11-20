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
#include "scene.h"
#include "util.h"
#include "hermes2d.h"
#include "module.h"
#include "module_agros.h"
#include "material.h"

// **************************************************************************

// atan2 for muparser
mu::value_type mu_atan2(mu::value_type y, mu::value_type x) { return atan2(y, x); }

ParserFormEssential::ParserFormEssential(rapidxml::xml_node<> *node, ProblemType problem_type) : expression("")
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
    j = atoi(node->first_attribute("j")->value());

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

    // atan2 for muparser
    parser->parser[0]->DefineFun("atan2", mu_atan2, false);

    // coordinates
    parser->parser[0]->DefineVar(Util::scene()->problemInfo()->labelX().toLower().toStdString(), &px);
    parser->parser[0]->DefineVar(Util::scene()->problemInfo()->labelY().toLower().toStdString(), &py);

    // current solution
    parser->parser[0]->DefineVar("uval", &puval);
    parser->parser[0]->DefineVar("ud" + Util::scene()->problemInfo()->labelX().toLower().toStdString() , &pudx);
    parser->parser[0]->DefineVar("ud" + Util::scene()->problemInfo()->labelY().toLower().toStdString(), &pudy);

    // test function
    parser->parser[0]->DefineVar("vval", &pvval);
    parser->parser[0]->DefineVar("vd" + Util::scene()->problemInfo()->labelX().toLower().toStdString(), &pvdx);
    parser->parser[0]->DefineVar("vd" + Util::scene()->problemInfo()->labelY().toLower().toStdString(), &pvdy);

    // previous solution
    parser->parser[0]->DefineVar("upval", &pupval);
    parser->parser[0]->DefineVar("upd" + Util::scene()->problemInfo()->labelX().toLower().toStdString(), &pupdx);
    parser->parser[0]->DefineVar("upd" + Util::scene()->problemInfo()->labelY().toLower().toStdString(), &pupdy);

    // solution from previous time level
    parser->parser[0]->DefineVar("uptval", &puptval);
    parser->parser[0]->DefineVar("uptd" + Util::scene()->problemInfo()->labelX().toLower().toStdString(), &puptdx);
    parser->parser[0]->DefineVar("uptd" + Util::scene()->problemInfo()->labelY().toLower().toStdString(), &puptdy);

    // time step
    parser->parser[0]->DefineVar("deltat", &pdeltat);

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
    : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j, area, sym), ParserForm(), m_material(material)
{
    initParser(material, NULL);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserMatrixFormVol<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                                Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::scene()->problemInfo()->timeStep.number();

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
        if (Util::scene()->problemInfo()->linearityType != LinearityType_Linear)
        {
            pupval = u_ext[this->i]->val[i]; //TODO PK this->j
            pupdx = u_ext[this->i]->dx[i];
            pupdy = u_ext[this->i]->dy[i];

            Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->problemInfo()->module()->material_type_variables;
            for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
            {
                Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
                parser->parser_variables[variable->shortname] = m_material->get_value(variable->id).value(pupval);
                parser->parser_variables["d" + variable->shortname] = m_material->get_value(variable->id).derivative(pupval);

                // parser->parser_variables[variable->shortname] = m_material->get_value(variable->id).value(sqrt(pupdx*pupdx + pupdy*pupdy));
                // parser->parser_variables["d" + variable->shortname] = m_material->get_value(variable->id).derivative(sqrt(pupdx*pupdx + pupdy*pupdy));

                // if (variable->shortname == "mur")
                //     qDebug() << 1.0/parser->parser_variables[variable->shortname]/(4*M_PI*1e-7);
            }
        }
        else
        {
            pupval = 0.0;
            pupdx = 0.0;
            pupdy = 0.0;
        }

        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        {
            puptval = ext->fn[this->j]->val[i];
            puptdx = ext->fn[this->j]->dx[i];
            puptdy = ext->fn[this->j]->dy[i];
        }

        try
        {
            result += wt[i] * parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserMatrixFormVol: " << parser->parser[0]->GetExpr() << " - " << e.GetMsg() << std::endl;
        }
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserMatrixFormVol<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                                   Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    return Hermes::Ord(20);
    Hermes::Ord result;

    for (int i = 0; i < n; i++)
        result += u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i];

    return result;
}

template <typename Scalar>
CustomParserVectorFormVol<Scalar>::CustomParserVectorFormVol(unsigned int i, unsigned int j,
                                                             std::string area, std::string expression,
                                                             Material *material)
    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), ParserForm(), m_material(material), j(j)
{
    initParser(material, NULL);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserVectorFormVol<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                                Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::scene()->problemInfo()->timeStep.number();

    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        // previous solution
        if (Util::scene()->problemInfo()->linearityType != LinearityType_Linear)
        {
            pupval = u_ext[this->j]->val[i];  //TODO PK this->i
            pupdx = u_ext[this->j]->dx[i];
            pupdy = u_ext[this->j]->dy[i];

            Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->problemInfo()->module()->material_type_variables;
            for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
            {
                Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
                parser->parser_variables[variable->shortname] = m_material->get_value(variable->id).value(pupval);

                // parser->parser_variables[variable->shortname] = m_material->get_value(variable->id).value(sqrt(pupdx*pupdx + pupdy*pupdy));

                // if (variable->shortname == "epsr")
                //     qDebug() << parser->parser_variables[variable->shortname];
            }
        }
        else
        {
            pupval = 0.0;
            pupdx = 0.0;
            pupdy = 0.0;
        }

        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        {
            puptval = ext->fn[this->j]->val[i];
            puptdx = ext->fn[this->j]->dx[i];
            puptdy = ext->fn[this->j]->dy[i];
        }

        try
        {
            result += wt[i] * parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserVectorFormVol: " << parser->parser[0]->GetExpr() << " - " << e.GetMsg() << std::endl;
        }
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserVectorFormVol<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                                   Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    return Hermes::Ord(20);
    Hermes::Ord result;

    for (int i = 0; i < n; i++)
        result += v->dx[i] + v->dy[i];

    return result;
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
                                                 Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::scene()->problemInfo()->timeStep.number();

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
        if (Util::scene()->problemInfo()->linearityType != LinearityType_Linear)
        {
            pupval = u_ext[this->j]->val[i];
            pupdx = u_ext[this->j]->dx[i];
            pupdy = u_ext[this->j]->dy[i];
        }
        else
        {
            pupval = 0.0;
            pupdx = 0.0;
            pupdy = 0.0;
        }

        try
        {
            result += wt[i] * parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserMatrixFormSurf: " << parser->parser[0]->GetExpr() << " - " << e.GetMsg() << std::endl;
        }
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserMatrixFormSurf<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u, Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                                    Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    return Hermes::Ord(20);
    Hermes::Ord result;

    for (int i = 0; i < n; i++)
        result += u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i];

    return result;
}

template <typename Scalar>
CustomParserVectorFormSurf<Scalar>::CustomParserVectorFormSurf(unsigned int i, unsigned int j,
                                                               std::string area, std::string expression,
                                                               Boundary *boundary)
    : Hermes::Hermes2D::VectorFormSurf<Scalar>(i, area), ParserForm(), j(j)
{
    initParser(NULL, boundary);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserVectorFormSurf<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                                 Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::scene()->problemInfo()->timeStep.number();

    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        // previous solution
        if (Util::scene()->problemInfo()->linearityType != LinearityType_Linear)
        {
            pupval = u_ext[this->j]->val[i];
            pupdx = u_ext[this->j]->dx[i];
            pupdy = u_ext[this->j]->dy[i];
        }
        else
        {
            pupval = 0.0;
            pupdx = 0.0;
            pupdy = 0.0;
        }

        try
        {
            result += wt[i] * parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserVectorFormSurf: " << parser->parser[0]->GetExpr() << " - " << e.GetMsg() << std::endl;
        }
    }

    return result;
}

template <typename Scalar>
Hermes::Ord CustomParserVectorFormSurf<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                                    Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    return Hermes::Ord(20);
    Hermes::Ord result;

    for (int i = 0; i < n; i++)
        result += v->dx[i] + v->dy[i];

    return result;
}

// **********************************************************************************************

template <typename Scalar>
CustomExactSolution<Scalar>::CustomExactSolution(Hermes::Hermes2D::Mesh *mesh, std::string expression, Boundary *boundary)
    : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), ParserForm()
{
    initParser(NULL, boundary);

    parser->parser[0]->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomExactSolution<Scalar>::value(double x, double y) const
{
    double result = 0;

    px = x;
    py = y;

    try
    {
        result = parser->parser[0]->Eval();
    }
    catch (mu::Parser::exception_type &e)
    {
        std::cout << "CustomExactSolution: " << parser->parser[0]->GetExpr() << " - " << e.GetMsg() << std::endl;
    }

    return result;
}

template <typename Scalar>
void CustomExactSolution<Scalar>::derivatives (double x, double y, Scalar& dx, Scalar& dy) const
{
}

template class CustomParserMatrixFormVol<double>;
template class CustomParserMatrixFormSurf<double>;
template class CustomParserVectorFormVol<double>;
template class CustomParserVectorFormSurf<double>;
template class CustomExactSolution<double>;
