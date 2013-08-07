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

#include "pythonlab/pyview.h"

#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"

#include "hermes2d/module.h"
#include "hermes2d/solutionstore.h"

#include "util/constants.h"
#include "util/global.h"

SceneViewCommon* PyView::currentSceneViewMode()
{
    if (currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->isChecked())
        return currentPythonEngineAgros()->sceneViewMesh();
    else if (currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->isChecked())
        return currentPythonEngineAgros()->sceneViewPost2D();
    else if (currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->isChecked())
        return currentPythonEngineAgros()->sceneViewPost3D();
    else if (currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->isChecked())
        return currentPythonEngineAgros()->sceneViewParticleTracing();

    // default
    return currentPythonEngineAgros()->sceneViewPreprocessor();
}

void PyView::saveImageToFile(const std::string &file, int width, int height)
{
    if (!silentMode())
        currentSceneViewMode()->saveImageToFile(QString::fromStdString(file), width, height);
}

void PyView::zoomBestFit()
{
    if (!silentMode())
        currentSceneViewMode()->doZoomBestFit();
}

void PyView::zoomIn()
{
    if (!silentMode())
        currentSceneViewMode()->doZoomIn();
}

void PyView::zoomOut()
{
    if (!silentMode())
        currentSceneViewMode()->doZoomOut();
}

void PyView::zoomRegion(double x1, double y1, double x2, double y2)
{
    if (!silentMode())
        currentSceneViewMode()->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

// ************************************************************************************

void PyViewConfig::setFontFamily(ProblemSetting::Type type, const std::string &family)
{
    if (silentMode())
        return;

    QStringList filter;
    filter << "*.ttf";
    QStringList list = QDir(datadir() + "/resources/fonts").entryList(filter);

    foreach (QString fileName, list)
        list.replaceInStrings(fileName, QFileInfo(fileName).baseName());

    foreach (QString fileName, list)
    {
        if (fileName.toStdString() == family)
        {
            Agros2D::problem()->setting()->setValue(type, QString::fromStdString(family));
            return;
        }

    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

// ************************************************************************************

void PyViewMeshAndPost::setActiveTimeStep(int timeStep)
{
    if (!Agros2D::problem()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    if (timeStep < 0 || timeStep >= Agros2D::problem()->numTimeLevels())
        throw out_of_range(QObject::tr("Time step must be in the range from 0 to %1.").arg(Agros2D::problem()->numTimeLevels() - 1).toStdString());

    if (silentMode())
        return;

    FieldInfo *fieldInfo = currentPythonEngineAgros()->postHermes()->activeViewField();
    if (!Agros2D::solutionStore()->timeLevels(fieldInfo).contains(Agros2D::problem()->timeStepToTotalTime(timeStep)))
        throw out_of_range(QObject::tr("Field '%1' does not have solution for time step %2 (%3 s).").arg(fieldInfo->fieldId()).
                           arg(timeStep).arg(Agros2D::problem()->timeStepToTotalTime(timeStep)).toStdString());

    SolutionMode solutionType = currentPythonEngineAgros()->postHermes()->activeAdaptivitySolutionType();
    int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, solutionType, timeStep);

    currentPythonEngineAgros()->postHermes()->setActiveTimeStep(timeStep);
    currentPythonEngineAgros()->postHermes()->setActiveAdaptivityStep(adaptivityStep);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setActiveAdaptivityStep(int adaptivityStep)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (adaptivityStep < 0 || adaptivityStep >= currentPythonEngineAgros()->postHermes()->activeViewField()->value(FieldInfo::AdaptivitySteps).toInt())
        throw out_of_range(QObject::tr("Adaptivity step for active field (%1) must be in the range from 0 to %2.").arg(currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId()).
                           arg(currentPythonEngineAgros()->postHermes()->activeViewField()->value(FieldInfo::AdaptivitySteps).toInt() - 1).toStdString());

    if (!silentMode())
    {
        currentPythonEngineAgros()->postHermes()->setActiveAdaptivityStep(adaptivityStep);
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

void PyViewMeshAndPost::setActiveSolutionType(const std::string &solutionType)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!solutionTypeStringKeys().contains(QString::fromStdString(solutionType)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

    if (currentPythonEngineAgros()->postHermes()->activeViewField()->adaptivityType() == AdaptivityType_None)
        throw logic_error(QObject::tr("Field '%1' was not solved with space adaptivity.").
                          arg(currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId()).toStdString());

    if (!silentMode())
    {
        currentPythonEngineAgros()->postHermes()->setActiveAdaptivitySolutionType(solutionTypeFromStringKey(QString::fromStdString(solutionType)));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

// ************************************************************************************

void PyViewMesh::checkExistingMesh()
{
    if (!Agros2D::problem()->isMeshed() && !currentPythonEngineAgros()->isScriptRunning())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
}

void PyViewMesh::setProblemSetting(ProblemSetting::Type type, bool value)
{
    checkExistingMesh();

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(type, value);
}

void PyViewMesh::activate()
{
    checkExistingMesh();

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

void PyViewMesh::setField(const std::string &fieldId)
{
    checkExistingMesh();

    if (!Agros2D::problem()->hasField(QString::fromStdString(fieldId)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::problem()->fieldInfos().keys())).toStdString());

    if (silentMode())
        return;

    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(QString::fromStdString(fieldId));

    currentPythonEngineAgros()->postHermes()->setActiveViewField(fieldInfo);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setOrderViewPalette(const std::string &palette)
{
    checkExistingMesh();

    if (!paletteOrderTypeStringKeys().contains(QString::fromStdString(palette)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteOrderTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_OrderPaletteOrderType, paletteOrderTypeFromStringKey(QString::fromStdString(palette)));
}

// ************************************************************************************

void PyViewPost::checkExistingSolution()
{
    if (!Agros2D::problem()->isSolved() && !currentPythonEngineAgros()->isScriptRunning())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

void PyViewPost::setProblemSetting(ProblemSetting::Type type, bool value)
{
    checkExistingSolution();

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(type, value);
}

void PyViewPost::setField(const std::string &fieldId)
{
    checkExistingSolution();

    if (!Agros2D::problem()->hasField(QString::fromStdString(fieldId)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::problem()->fieldInfos().keys())).toStdString());

    if (silentMode())
        return;

    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(QString::fromStdString(fieldId));
    SolutionMode solutionType = currentPythonEngineAgros()->postHermes()->activeAdaptivitySolutionType();
    int timeStep = currentPythonEngineAgros()->postHermes()->activeTimeStep();

    if (fieldInfo->adaptivityType() == AdaptivityType_None)
        solutionType = SolutionMode_Normal;

    if (!Agros2D::solutionStore()->timeLevels(fieldInfo).contains(Agros2D::problem()->timeStepToTotalTime(timeStep)))
        timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo, solutionType);

    // set last adaptivity step (keeping of previous step can be misleading)
    int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, solutionType, timeStep);

    currentPythonEngineAgros()->postHermes()->setActiveViewField(fieldInfo);
    currentPythonEngineAgros()->postHermes()->setActiveTimeStep(timeStep);
    currentPythonEngineAgros()->postHermes()->setActiveAdaptivityStep(adaptivityStep);
    currentPythonEngineAgros()->postHermes()->setActiveAdaptivitySolutionType(solutionType);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewVariable(const std::string &var)
{
    checkExistingSolution();

    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewScalarVariables())
    {
        list.append(variable.id());
        if (variable.id() == QString::fromStdString(var))
        {
            if (!silentMode())
            {
                Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariable, QString::fromStdString(var));
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost::setScalarViewVariableComp(const std::string &component)
{
    checkExistingSolution();

    if (!physicFieldVariableCompTypeStringKeys().contains(QString::fromStdString(component)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, physicFieldVariableCompFromStringKey(QString::fromStdString(component)));
}

void PyViewPost::setScalarViewPalette(const std::string &palette)
{
    checkExistingSolution();

    if (!paletteTypeStringKeys().contains(QString::fromStdString(palette)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteType, paletteTypeFromStringKey(QString::fromStdString(palette)));
}

void PyViewPost::setScalarViewPaletteQuality(const std::string &quality)
{
    checkExistingSolution();

    if (!paletteQualityStringKeys().contains(QString::fromStdString(quality)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteQualityStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_LinearizerQuality, paletteQualityFromStringKey(QString::fromStdString(quality)));
}

// ************************************************************************************

void PyViewPost2D::activate()
{
    checkExistingSolution();

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

void PyViewPost2D::setContourVariable(const std::string &var)
{
    checkExistingSolution();

    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewScalarVariables())
    {
        if (variable.isScalar())
        {
            list.append(variable.id());

            if (variable.id() == QString::fromStdString(var))
            {
                if (!silentMode())
                {
                    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContourVariable, QString::fromStdString(var));
                    return;
                }
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorVariable(const std::string &var)
{
    checkExistingSolution();

    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewVectorVariables())
    {
        list.append(variable.id());
        if (variable.id() == QString::fromStdString(var))
        {
            if (!silentMode())
            {
                Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorVariable, QString::fromStdString(var));
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorType(const std::string &type)
{
    checkExistingSolution();

    if (!vectorTypeStringKeys().contains(QString::fromStdString(type)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(vectorTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorType, vectorTypeFromStringKey(QString::fromStdString(type)));
}

void PyViewPost2D::setVectorCenter(const std::string &center)
{
    checkExistingSolution();

    if (!vectorCenterStringKeys().contains(QString::fromStdString(center)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(vectorCenterStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorCenter, vectorCenterFromStringKey(QString::fromStdString(center)));
}

// ************************************************************************************

void PyViewPost3D::activate()
{
    checkExistingSolution();

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->trigger();
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

void PyViewPost3D::setPost3DMode(const std::string &mode)
{
    if (!sceneViewPost3DModeStringKeys().contains(QString::fromStdString(mode)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(sceneViewPost3DModeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, sceneViewPost3DModeFromStringKey(QString::fromStdString(mode)));
}

// ************************************************************************************

void PyViewParticleTracing::activate()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->trigger();
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}
