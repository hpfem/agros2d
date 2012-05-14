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
#include "marker.h"
#include "hermes2d.h"
#include "module.h"
#include "coupling.h"
#include "module_agros.h"
#include "field.h"
#include "problem.h"
#include "datatable.h"
#include "scenelabel.h"

ParserForm::ParserForm(FieldInfo *fieldInfo, CouplingInfo *couplingInfo) :
    m_fieldInfo(fieldInfo), m_couplingInfo(couplingInfo)
{

}

ParserForm::~ParserForm()
{
    delete m_parser;
}

void ParserForm::initParserField(const std::string &expr)
{
    m_parser = m_fieldInfo->module()->expressionParser();
    m_parser->SetExpr(expr);
    initParser();
}

void ParserForm::initParserCoupling(const std::string &expr)
{
    m_parser = m_couplingInfo->coupling()->expressionParser();
    m_parser->SetExpr(expr);
    initParser();

    // coupling
    // FIXME: maxSourceFieldComponents not good
    for (int comp = 0; comp < maxSourceFieldComponents; comp++)
    {
        m_parser->DefineVar("source" + QString().setNum(comp).toStdString(), &source[comp]);
        m_parser->DefineVar("source" + QString().setNum(comp).toStdString() + "d" + Util::problem()->config()->labelX().toLower().toStdString(), &sourcedx[comp]);
        m_parser->DefineVar("source" + QString().setNum(comp).toStdString() + "d" + Util::problem()->config()->labelY().toLower().toStdString(), &sourcedy[comp]);
    }
}

void ParserForm::initParser()
{
    // coordinates
    m_parser->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
    m_parser->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

    // current solution
    m_parser->DefineVar("uval", &puval);
    m_parser->DefineVar("ud" + Util::problem()->config()->labelX().toLower().toStdString() , &pudx);
    m_parser->DefineVar("ud" + Util::problem()->config()->labelY().toLower().toStdString(), &pudy);

    // test function
    m_parser->DefineVar("vval", &pvval);
    m_parser->DefineVar("vd" + Util::problem()->config()->labelX().toLower().toStdString(), &pvdx);
    m_parser->DefineVar("vd" + Util::problem()->config()->labelY().toLower().toStdString(), &pvdy);

    // previous solution
    m_parser->DefineVar("upval", &pupval);
    m_parser->DefineVar("upd" + Util::problem()->config()->labelX().toLower().toStdString(), &pupdx);
    m_parser->DefineVar("upd" + Util::problem()->config()->labelY().toLower().toStdString(), &pupdy);

    // solution from previous time level
    m_parser->DefineVar("uptval", &puptval);
    m_parser->DefineVar("uptd" + Util::problem()->config()->labelX().toLower().toStdString(), &puptdx);
    m_parser->DefineVar("uptd" + Util::problem()->config()->labelY().toLower().toStdString(), &puptdy);

    // time step
    m_parser->DefineVar("deltat", &pdeltat);
}

void ParserForm::setMaterialToParser(Material *material)
{  
    QList<Module::MaterialTypeVariable *> materials = material->fieldInfo()->module()->materialTypeVariables();
    foreach (Module::MaterialTypeVariable *variable, materials)
    {
        if ((m_fieldInfo && ((m_fieldInfo->linearityType() == LinearityType_Linear) ||
                             ((m_fieldInfo->linearityType() != LinearityType_Linear) && variable->expressionNonlinear().isEmpty())))
                || m_couplingInfo)
        {
            // linear variable
            m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).number();
            m_parserVariables["d" + variable->shortname().toStdString()] = 0.0;
        }
        else
        {
            // nonlinear variable
            m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).number();
            m_parserVariables["d" + variable->shortname().toStdString()] = material->value(variable->id()).derivative(0.0);
        }
    }

    // register value address
    for (QMap<std::string, double>::iterator itv = m_parserVariables.begin(); itv != m_parserVariables.end(); ++itv)
        m_parser->DefineVar(itv.key(), &itv.value());
}

void ParserForm::setMaterialsToParser(QList<Material *> materials)
{
    foreach (Material *material, materials)
        setMaterialToParser(material);
}

void ParserForm::setBoundaryToParser(Boundary * boundary)
{
    // TODO: zkontrolovat volani value, proc u boundary neni derivace, ...
    Module::BoundaryType *boundaryType = m_fieldInfo->module()->boundaryType(boundary->getType());
    foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables())
        // linear variable
        m_parserVariables[variable->shortname().toStdString()] = boundary->value(variable->id()).number();

    // register value address
    for (QMap<std::string, double>::iterator itv = m_parserVariables.begin(); itv != m_parserVariables.end(); ++itv)
        m_parser->DefineVar(itv.key(), &itv.value());
}

void ParserForm::setBoundariesToParser(QList<Boundary *> boundaries)
{
    foreach (Boundary *boundary, boundaries)
        setBoundaryToParser(boundary);
}

void ParserForm::setNonlinearParsers()
{
    if (m_fieldInfo && (m_fieldInfo->linearityType() != LinearityType_Linear))
    {
        // value and derivatives
        pnlvalue = new double[m_fieldInfo->module()->numberOfSolutions()];
        pnldx = new double[m_fieldInfo->module()->numberOfSolutions()];
        pnldy = new double[m_fieldInfo->module()->numberOfSolutions()];

        foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        {
            if (!variable->expressionNonlinear().isEmpty())
            {
                mu::Parser *parser = m_fieldInfo->module()->expressionParser();
                parser->SetExpr(variable->expressionNonlinear().toStdString());

                parser->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &pnlx);
                parser->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &pnly);

                for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                {
                    std::stringstream number;
                    number << (k+1);

                    parser->DefineVar("value" + number.str(), &pnlvalue[k]);
                    parser->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pnldx[k]);
                    parser->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pnldy[k]);
                }

                m_parsersNonlinear[variable] = parser;
            }
        }
    }
}

void ParserForm::setNonlinearMaterial(Material *material, int offset, int index, double *x, double *y, Hermes::Hermes2D::Func<double> *u_ext[]) const
{
    if (m_fieldInfo && m_fieldInfo->linearityType() != LinearityType_Linear)
    {
        pnlx = x[index];
        pnly = y[index];

        for (int i = 0; i < m_fieldInfo->module()->numberOfSolutions(); i++)
        {
            // pnlvalue[i] = u_ext[offset + i]->val[index];
            // pnldx[i] = u_ext[offset + i]->dx[index];
            // pnldy[i] = u_ext[offset + i]->dy[index];
            pnlvalue[i] = u_ext[i]->val[index];
            pnldx[i] = u_ext[i]->dx[index];
            pnldy[i] = u_ext[i]->dy[index];
        }

        foreach (Module::MaterialTypeVariable *variable, m_fieldInfo->module()->materialTypeVariables())
        {
            if (!variable->expressionNonlinear().isEmpty())
            {
                try
                {
                    double nonlinValue = m_parsersNonlinear[variable]->Eval();
                    // qDebug() << "nonlinValue: B" << nonlinValue;

                    m_parserVariables[variable->shortname().toStdString()] = material->value(variable->id()).value(nonlinValue);
                    m_parserVariables["d" + variable->shortname().toStdString()] = material->value(variable->id()).derivative(nonlinValue);

                    // qDebug() << "mur" << m_parserVariables[variable->shortname().toStdString()] << "dmur/dB" << m_parserVariables["d" + variable->shortname().toStdString()];
                }
                catch (mu::Parser::exception_type &e)
                {
                    std::cout << "Nonlinear value '" << variable->id().toStdString() << "'): " << e.GetMsg() << std::endl;
                }
            }
        }
    }
}

// **********************************************************************************************

template <typename Scalar>
CustomParserMatrixFormVol<Scalar>::CustomParserMatrixFormVol(unsigned int i, unsigned int j,
                                                             std::string area,
                                                             Hermes::Hermes2D::SymFlag sym,
                                                             std::string expression,
                                                             FieldInfo *fieldInfo,
                                                             CouplingInfo *couplingInfo,
                                                             Material *materialSource,
                                                             Material *materialTarget)
    : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j, area, sym), ParserForm(fieldInfo, couplingInfo),
      m_materialSource(materialSource), m_materialTarget(materialTarget)
{
    // TODO: still not good
    // parser
    if (fieldInfo)
        initParserField(expression);
    else if(couplingInfo)
        initParserCoupling(expression);
    else
        assert(0);

    // set materials to the parser
    setMaterialToParser(m_materialSource);
    if (m_materialTarget)
        setMaterialToParser(m_materialTarget);

    // nonlinear parsers
    setNonlinearParsers();
}

template <typename Scalar>
Scalar CustomParserMatrixFormVol<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                                Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::problem()->config()->timeStep().number();

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
        // TODO: linear solver
        // if (this->linearityType() != LinearityType_Linear)
        if (true)
        {
            pupval = u_ext[this->i]->val[i];
            pupdx = u_ext[this->i]->dx[i];
            pupdy = u_ext[this->i]->dy[i];

            setNonlinearMaterial(m_materialSource, this->i, i, e->x, e->y, u_ext);
        }
        else
        {
            pupval = 0.0;
            pupdx = 0.0;
            pupdy = 0.0;
        }

        if (!m_materialTarget)
        {
            if (m_fieldInfo && (m_fieldInfo->analysisType() == AnalysisType_Transient))
            {
                puptval = ext->fn[this->j]->val[i];
                puptdx = ext->fn[this->j]->dx[i];
                puptdy = ext->fn[this->j]->dy[i];
            }
        }

        try
        {
            // qDebug() << "CustomParserMatrixFormVol" << m_parser->Eval();
            result += wt[i] * m_parser->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserMatrixFormVol: " << m_parser->GetExpr() << " - " << e.GetMsg() << std::endl;
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
CustomParserMatrixFormVol<Scalar>* CustomParserMatrixFormVol<Scalar>::clone()
{
    return new CustomParserMatrixFormVol(this->i, this->j, this->areas[0], (Hermes::Hermes2D::SymFlag) this->sym, m_parser->GetExpr(),
                                         this->m_fieldInfo, this->m_couplingInfo, this->m_materialSource, this->m_materialTarget);
}

template <typename Scalar>
LinearityType CustomParserMatrixFormVol<Scalar>::linearityType() const
{
    LinearityType linearityType;

    if(m_fieldInfo)
        linearityType = m_fieldInfo->linearityType();
    else
        linearityType = m_couplingInfo->linearityType();

    return linearityType;
}


template <typename Scalar>
CustomParserVectorFormVol<Scalar>::CustomParserVectorFormVol(unsigned int i, unsigned int j,
                                                             std::string area,
                                                             std::string expression,
                                                             FieldInfo *fieldInfo,
                                                             CouplingInfo *couplingInfo,
                                                             Material *materialSource,
                                                             Material *materialTarget)
    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), ParserForm(fieldInfo, couplingInfo),
      m_materialSource(materialSource), m_materialTarget(materialTarget), j(j)
{
    // TODO: still not good
    // parser
    if (fieldInfo)
        initParserField(expression);
    else if(couplingInfo)
        initParserCoupling(expression);
    else
        assert(0);

    // set materials to the parser
    setMaterialToParser(m_materialSource);
    if (m_materialTarget)
        setMaterialToParser(m_materialTarget);

    // nonlinear parsers
    setNonlinearParsers();
}

template <typename Scalar>
Scalar CustomParserVectorFormVol<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                                Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::problem()->config()->timeStep().number();

    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];

        // TODO: linear solver
        // if (this->linearityType() != LinearityType_Linear)
        if (true)
        {
            pupval = u_ext[this->j]->val[i];
            pupdx = u_ext[this->j]->dx[i];
            pupdy = u_ext[this->j]->dy[i];

            setNonlinearMaterial(m_materialSource, this->j, i, e->x, e->y, u_ext);
        }

        if (m_materialTarget)
        {
            // we have material target -> it is coupling form, without time parameter
            if (ext->get_nf() > 0)
            {
                for(int comp = 0; comp < ext->get_nf(); comp++)
                {
                    source[comp] = ext->fn[comp]->val[i];
                    sourcedx[comp] = ext->fn[comp]->dx[i];
                    sourcedy[comp] = ext->fn[comp]->dy[i];
                }
            }
        }
        else
        {
            if (m_fieldInfo && (m_fieldInfo->analysisType() == AnalysisType_Transient))
            {
                puptval = ext->fn[this->j]->val[i];
                puptdx = ext->fn[this->j]->dx[i];
                puptdy = ext->fn[this->j]->dy[i];
            }
        }

        try
        {
            result += wt[i] * m_parser->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserVectorFormVol: " << m_parser->GetExpr() << " - " << e.GetMsg() << std::endl;
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


template <typename Scalar>
CustomParserVectorFormVol<Scalar>* CustomParserVectorFormVol<Scalar>::clone()
{
    return new CustomParserVectorFormVol(this->i, this->j, this->areas[0], m_parser->GetExpr(),
                                         this->m_fieldInfo, this->m_couplingInfo, this->m_materialSource, this->m_materialTarget);
}

template <typename Scalar>
LinearityType CustomParserVectorFormVol<Scalar>::linearityType() const
{
    LinearityType linearityType;

    if(m_fieldInfo)
        linearityType = m_fieldInfo->linearityType();
    else
        linearityType = m_couplingInfo->linearityType();

    return linearityType;
}

// **********************************************************************************************

template <typename Scalar>
CustomParserMatrixFormSurf<Scalar>::CustomParserMatrixFormSurf(unsigned int i, unsigned int j,
                                                               std::string area, std::string expression,
                                                               Boundary *boundary)
    : Hermes::Hermes2D::MatrixFormSurf<Scalar>(i, j, area), ParserForm(), m_boundary(boundary)
{
    // TODO: not good
    m_fieldInfo = boundary->fieldInfo();

    // parser
    initParserField(expression);
    // set boundary to the parser
    setBoundaryToParser(boundary);

    m_parser->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserMatrixFormSurf<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u, Hermes::Hermes2D::Func<double> *v,
                                                 Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::problem()->config()->timeStep().number();

    // neighborhood material
    /*
    SceneLabel *label = Util::scene()->labels->at(atoi(Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().
                                                       get_user_marker(e->elem_marker).marker.c_str()));
    SceneMaterial *material = label->getMarker(m_fieldInfo);
    */

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

        // todo: now we use only Newton solver
        // if (m_fieldInfo->linearityType != LinearityType_Linear)
        // TODO: now we use only Newton solver
        if (true)
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
            result += wt[i] * m_parser->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserMatrixFormSurf: " << m_parser->GetExpr() << " - " << e.GetMsg() << std::endl;
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
CustomParserMatrixFormSurf<Scalar>* CustomParserMatrixFormSurf<Scalar>::clone()
{
    return new CustomParserMatrixFormSurf(this->i, this->j, this->areas[0], m_parser->GetExpr(),
                                          this->m_boundary);
}

template <typename Scalar>
CustomParserVectorFormSurf<Scalar>::CustomParserVectorFormSurf(unsigned int i, unsigned int j,
                                                               std::string area, std::string expression,
                                                               Boundary *boundary)
    : Hermes::Hermes2D::VectorFormSurf<Scalar>(i, area), j(j), ParserForm(), m_boundary(boundary)
{
    // TODO: not good
    m_fieldInfo = boundary->fieldInfo();

    // parser
    initParserField(expression);
    // set boundary to the parser
    setBoundaryToParser(boundary);

    m_parser->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomParserVectorFormSurf<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                                 Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    pdeltat = Util::problem()->config()->timeStep().number();

    for (int i = 0; i < n; i++)
    {
        px = e->x[i];
        py = e->y[i];

        pvval = v->val[i];
        pvdx = v->dx[i];
        pvdy = v->dy[i];


        // previous solution
        // if (m_fieldInfo->linearityType != LinearityType_Linear)
        // TODO: now we use only Newton solver
        if (true)
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
            result += wt[i] * m_parser->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "CustomParserVectorFormSurf: " << m_parser->GetExpr() << " - " << e.GetMsg() << std::endl;
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

template <typename Scalar>
CustomParserVectorFormSurf<Scalar>* CustomParserVectorFormSurf<Scalar>::clone()
{
    return new CustomParserVectorFormSurf(this->i, this->j, this->areas[0], m_parser->GetExpr(),
                                          this->m_boundary);
}

// **********************************************************************************************

template <typename Scalar>
CustomExactSolution<Scalar>::CustomExactSolution(Hermes::Hermes2D::Mesh *mesh, std::string expression, Boundary *boundary)
    : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh)
{
    // TODO: not good
    m_fieldInfo = boundary->fieldInfo();

    // parser
    initParserField(expression);
    // set boundary to the parser
    setBoundaryToParser(boundary);

    m_parser->SetExpr(expression);
}

template <typename Scalar>
Scalar CustomExactSolution<Scalar>::value(double x, double y) const
{
    double result = 0;

    px = x;
    py = y;

    try
    {
        result = m_parser->Eval();
    }
    catch (mu::Parser::exception_type &e)
    {
        std::cout << "CustomExactSolution: " << m_parser->GetExpr() << " - " << e.GetMsg() << std::endl;
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
