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

    PointValue(double scalar, Point vector, Material *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    Material *material;
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

class PluginInterface
{
public:

    virtual ~PluginInterface() {}

    virtual QString fieldId() = 0;

    virtual Hermes::Hermes2D::MatrixFormVol<double> *matrixFormVol(const ProblemID problemId, int i, int j,
                                                                   Material *materialSource, Material *materialTarget, int offsetI, int offsetJ ) = 0;

    virtual Hermes::Hermes2D::VectorFormVol<double> *vectorFormVol(const ProblemID problemId, int i, int j,
                                                                   Material *materialSource, Material *materialTarget, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::MatrixFormSurf<double> *matrixFormSurf(const ProblemID problemId, int i, int j,
                                                                     Boundary *boundary, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::VectorFormSurf<double> *vectorFormSurf(const ProblemID problemId, int i, int j,
                                                                     Boundary *boundary, int offsetI, int offsetJ) = 0;

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

    // force calculation
    virtual Point3 force(FieldInfo *fieldInfo, const Point3 &point, const Point3 &velocity) = 0;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(PluginInterface, "agros2d.PluginInterface/1.0")
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
