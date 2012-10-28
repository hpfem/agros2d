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

#include "util.h"
#include "field.h"
#include "problem.h"
#include "scene.h"
#include "scenemarker.h"
#include "module.h"
#include "module_agros.h"
#include "plugin_interface.h"

Field::Field(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

bool Field::solveInitVariables()
{
    //TODO transient
    //
    //
    //    // transient
    //    if (Util::problem()->config()->analysisType() == AnalysisType_Transient)
    //    {
    //        if (!Util::problem()->config()->timeStep.evaluate()) return false;
    //        if (!Util::problem()->config()->timeTotal.evaluate()) return false;
    //        if (!Util::problem()->config()->initialCondition.evaluate()) return false;
    //    }

    if (!Util::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if (!Util::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

FieldInfo::FieldInfo(QString fieldId, const AnalysisType analysisType)
{
    m_module = NULL;

    if (fieldId.isEmpty())
    {
        // default
        // read default field (Util::config() is not set)
        QSettings settings;
        m_fieldId = settings.value("General/DefaultPhysicField", "electrostatic").toString();

        QMap<QString, QString> modules = availableModules();
        if (!modules.keys().contains(m_fieldId))
            m_fieldId = "electrostatic";
    }
    else
    {
        m_fieldId = fieldId;
    }

    // set first analysis and read module
    if (analysisType == AnalysisType_Undefined)
    {
        QMap<AnalysisType, QString> analyses = availableAnalyses(m_fieldId);
        assert(analyses.count());

        setAnalysisType(analyses.begin().key());
    }

    clear();
}

FieldInfo::~FieldInfo()
{
    if (m_module) delete m_module;
}

void FieldInfo::setAnalysisType(const AnalysisType analysisType)
{
    m_analysisType = analysisType;

    if (m_module) delete m_module;
    m_module = new Module::ModuleAgros(m_fieldId,
                                       Util::problem()->config()->coordinateType(),
                                       m_analysisType);
}

int FieldInfo::edgeRefinement(SceneEdge *edge)
{
    QMapIterator<SceneEdge *, int> i(m_edgesRefinement);
    while (i.hasNext()) {
        i.next();
        if (i.key() == edge)
            return i.value();
    }

    return 0;
}

int FieldInfo::labelRefinement(SceneLabel *label)
{
    QMapIterator<SceneLabel *, int> i(m_labelsRefinement);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return 0;
}


int FieldInfo::labelPolynomialOrder(SceneLabel *label)
{
    QMapIterator<SceneLabel *, int> i(m_labelsPolynomialOrder);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return m_polynomialOrder;
}

void FieldInfo::clear()
{
    // mesh
    m_numberOfRefinements = 1;
    m_polynomialOrder = 2;

    m_edgesRefinement.clear();
    m_labelsRefinement.clear();

    m_labelsPolynomialOrder.clear();

    // adaptivity
    m_adaptivityType = AdaptivityType_None;
    m_adaptivitySteps = 0;
    m_adaptivityTolerance = 1.0;

    // transient
    m_initialCondition = Value("0.0", false);
    m_timeSkip = Value("0", false);

    // linearity
    m_linearityType = LinearityType_Linear;
    m_nonlinearTolerance = 1e-3;
    m_nonlinearSteps = 10;
}

QString FieldInfo::name()
{
    return m_module->name();
}

QString FieldInfo::description()
{
    return m_module->description();
}
