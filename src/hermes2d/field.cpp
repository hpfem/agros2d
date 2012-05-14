#include "util.h"
#include "field.h"
#include "problem.h"
#include "scene.h"
#include "scenemarker.h"
#include "module.h"
#include "module_agros.h".h"

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
    m_module = moduleFactory(m_fieldId,
                             Util::problem()->config()->coordinateType(),
                             m_analysisType);
}

void FieldInfo::clear()
{
    m_numberOfRefinements = 1;
    m_polynomialOrder = 2;
    m_adaptivityType = AdaptivityType_None;
    m_adaptivitySteps = 0;
    m_adaptivityTolerance = 1.0;

    m_initialCondition = Value("0.0", false);

    // weakforms
    m_weakFormsType = WeakFormsType_Compiled;

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
