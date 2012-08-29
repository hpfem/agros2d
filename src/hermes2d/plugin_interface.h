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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/marker.h"

struct PointValue
{
    PointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    PointValue(double scalar, Point vector, SceneMaterial *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    SceneMaterial *material;
};

class LocalValue
{
public:
    LocalValue(FieldInfo *fieldInfo, const Point &point)
        : m_fieldInfo(fieldInfo), m_point(point) {}

    // point
    inline Point point() { return m_point; }

    // variables
    QMap<Module::LocalVariable *, PointValue> values() const { return m_values; }

    virtual void calculate() = 0;

protected:
    // point
    Point m_point;
    // field info
    FieldInfo *m_fieldInfo;

    // variables
    QMap<Module::LocalVariable *, PointValue> m_values;
};

class IntegralValue
{
public:
    IntegralValue(FieldInfo *fieldInfo)
        : m_fieldInfo(fieldInfo) {}

    // variables
    inline QMap<Module::Integral*, double> values() const { return m_values; }

protected:
    // field info
    FieldInfo *m_fieldInfo;

    // variables
    QMap<Module::Integral*, double> m_values;
};

class LocalForceValue
{
public:
    LocalForceValue(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo) {}
    ~LocalForceValue() {}

    Point3 calculate(const Point3 &point, const Point3 &velocity = Point3()) {}

private:
    // field info
    FieldInfo *m_fieldInfo;
};

/*


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
*/

class PluginInterface
{
public:

    virtual ~PluginInterface() {}

    virtual QString fieldId() = 0;

    virtual Hermes::Hermes2D::MatrixFormVol<double> *matrixFormVol(const ProblemID problemId, int i, int j,
                                                                   const std::string &area, Hermes::Hermes2D::SymFlag sym,
                                                                   SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ ) = 0;

    virtual Hermes::Hermes2D::VectorFormVol<double> *vectorFormVol(const ProblemID problemId, int i, int j,
                                                                   const std::string &area, SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::MatrixFormSurf<double> *matrixFormSurf(const ProblemID problemId, int i, int j,
                                                                     const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::VectorFormSurf<double> *vectorFormSurf(const ProblemID problemId, int i, int j,
                                                                     const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::ExactSolutionScalar<double> *exactSolution(const ProblemID problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary) = 0;

    // postprocessor
    // filter
    virtual Hermes::Hermes2D::Filter<double> *filter(FieldInfo *fieldInfo,
                                                     Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp) = 0;

    // local values
    virtual LocalValue *localValue(FieldInfo *fieldInfo, const Point &point) = 0;
    // surface integrals
    virtual IntegralValue *surfaceIntegral(FieldInfo *fieldInfo) = 0;
    // volume integrals
    virtual IntegralValue *volumeIntegral(FieldInfo *fieldInfo) = 0;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(PluginInterface, "agros2d.PluginInterface/1.0")
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
