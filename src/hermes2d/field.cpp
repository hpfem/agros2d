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

    if(!Util::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if(!Util::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

FieldInfo::FieldInfo(QString fieldId)
{
    m_module = NULL;

    if (fieldId.isEmpty())
    {
        // default
        // read default field (Util::config() is not set)
        QSettings settings;
        m_fieldId = settings.value("General/DefaultPhysicField", "electrostatic").toString();

        bool check = false;
        std::map<std::string, std::string> modules = availableModules();
        for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
            if (m_fieldId.toStdString() == it->first)
            {
                check = true;
                break;
            }
        if (!check)
            m_fieldId = "electrostatic";
    }
    else
    {
        m_fieldId = fieldId;
    }

    clear();
}

FieldInfo::~FieldInfo()
{
    if (m_module) delete m_module;
}

void FieldInfo::clear()
{
    // module object
    setAnalysisType(AnalysisType_SteadyState);

    numberOfRefinements = 1;
    polynomialOrder = 2;
    adaptivityType = AdaptivityType_None;
    adaptivitySteps = 0;
    adaptivityTolerance = 1.0;

    initialCondition = Value("0.0", false);

    // weakforms
    weakFormsType = WeakFormsType_Interpreted;

    // linearity
    linearityType = LinearityType_Linear;
    nonlinearTolerance = 1e-3;
    nonlinearSteps = 10;
}

void FieldInfo::setAnalysisType(AnalysisType analysisType)
{
    m_analysisType = analysisType;

    if (m_module) delete m_module;
    m_module = moduleFactory(m_fieldId.toStdString(),
                             Util::problem()->config()->coordinateType(),
                             m_analysisType);
}

