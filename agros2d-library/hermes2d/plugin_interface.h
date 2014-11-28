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
#include "hermes2d/field.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/solver.h"
#include "../sceneview_post.h"

#include "../../resources_source/classes/module_xml.h"

#undef signals
#include <deal.II/numerics/data_postprocessor.h>
#define signals public

class PositionInfo;

struct LocalPointValue
{
    LocalPointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    LocalPointValue(double scalar, Point vector, Material *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    const Material *material;
};

class LocalValue
{
public:
    LocalValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType), m_point(point) {}
    virtual ~LocalValue()
    {
        m_values.clear();
    }

    // point
    inline Point point() { return m_point; }

    // variables
    QMap<QString, LocalPointValue> values() const { return m_values; }

    virtual void calculate() = 0;

protected:
    // point
    Point m_point;
    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, LocalPointValue> m_values;
};

class IntegralValue
{
public:
    IntegralValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType) {}

    // variables
    inline QMap<QString, double> values() const { return m_values; }

protected:
    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, double> m_values;
};

// plugin interface
class AGROS_LIBRARY_API PluginInterface
{
public:
    inline PluginInterface() : m_module(NULL), m_coupling(NULL) {}
    virtual ~PluginInterface() {}

    virtual QString fieldId() = 0;

    inline XMLModule::field *module() const { assert(m_module); return m_module; }
    inline XMLModule::coupling *coupling() const { assert(m_coupling); return m_coupling; }

    // weak forms
    virtual SolverDeal *solverDeal(const FieldInfo *fieldInfo, int initialOrder) = 0;

    // error calculators
    // virtual Hermes::Hermes2D::ErrorCalculator<double> *errorCalculator(const FieldInfo *fieldInfo,
    //                                                                   const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType) = 0;

    // postprocessor
    // filter
    virtual dealii::DataPostprocessorScalar<2> *filter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                       MultiArray *ma,
                                                       const QString &variable,
                                                       PhysicFieldVariableComp physicFieldVariableComp) = 0;

    // local values
    virtual LocalValue *localValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point) = 0;
    // surface integrals
    virtual IntegralValue *surfaceIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // volume integrals
    virtual IntegralValue *volumeIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // force calculation
    virtual Point3 force(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                         Hermes::Hermes2D::Element *element, SceneMaterial *material, const Point3 &point, const Point3 &velocity) = 0;
    virtual bool hasForce(const FieldInfo *fieldInfo) = 0;

    // localization
    virtual QString localeName(const QString &name) = 0;
    // description
    virtual QString localeDescription() = 0;

protected:
    XMLModule::field *m_module;
    XMLModule::coupling *m_coupling;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(PluginInterface, "agros2d.PluginInterface/1.0")
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
