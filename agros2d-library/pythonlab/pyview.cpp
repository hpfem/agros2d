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

void PyView::saveImageToFile(const std::string &file, int width, int height)
{
    if (!silentMode())
    {
        if (currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->isChecked())
            currentPythonEngineAgros()->sceneViewPreprocessor()->saveImageToFile(QString::fromStdString(file), width, height);
        else if (currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->isChecked())
            currentPythonEngineAgros()->sceneViewMesh()->saveImageToFile(QString::fromStdString(file), width, height);
        else if (currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->isChecked())
            currentPythonEngineAgros()->sceneViewPost2D()->saveImageToFile(QString::fromStdString(file), width, height);
        else if (currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->isChecked())
            currentPythonEngineAgros()->sceneViewPost3D()->saveImageToFile(QString::fromStdString(file), width, height);
        else if (currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->isChecked())
            currentPythonEngineAgros()->sceneViewParticleTracing()->saveImageToFile(QString::fromStdString(file), width, height);
    }
}

void PyView::zoomBestFit()
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomBestFit();
}

void PyView::zoomIn()
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomIn();
}

void PyView::zoomOut()
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomOut();
}

void PyView::zoomRegion(double x1, double y1, double x2, double y2)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

// ************************************************************************************

void PyViewConfig::setProblemSetting(ProblemSetting::Type type, bool value)
{
    Agros2D::problem()->setting()->setValue(type, value);

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

void PyViewConfig::setGridStep(double step)
{
    if (step > 0.0)
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_GridStep, step);
    else
        throw out_of_range(QObject::tr("Grid step must be positive.").toStdString());

    if (!silentMode())
    {
        Agros2D::scene()->invalidate();
        currentPythonEngineAgros()->postHermes()->refresh();
    }
}

// ************************************************************************************

void PyViewMeshAndPost::setField(const std::string &fieldId)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!Agros2D::problem()->hasField(QString::fromStdString(fieldId)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::problem()->fieldInfos().keys())).toStdString());

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

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setActiveTimeStep(int timeStep)
{
    if (!Agros2D::problem()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    if (timeStep < 0 || timeStep >= Agros2D::problem()->numTimeLevels())
        throw out_of_range(QObject::tr("Time step must be in the range from 0 to %1.").arg(Agros2D::problem()->numTimeLevels() - 1).toStdString());

    FieldInfo *fieldInfo = currentPythonEngineAgros()->postHermes()->activeViewField();
    if (!Agros2D::solutionStore()->timeLevels(fieldInfo).contains(Agros2D::problem()->timeStepToTotalTime(timeStep)))
        throw out_of_range(QObject::tr("Field '%1' does not have solution for time step %2 (%3 s).").arg(fieldInfo->fieldId()).
                           arg(timeStep).arg(Agros2D::problem()->timeStepToTotalTime(timeStep)).toStdString());

    SolutionMode solutionType = currentPythonEngineAgros()->postHermes()->activeAdaptivitySolutionType();
    int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, solutionType, timeStep);

    currentPythonEngineAgros()->postHermes()->setActiveTimeStep(timeStep);
    currentPythonEngineAgros()->postHermes()->setActiveAdaptivityStep(adaptivityStep);

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setActiveAdaptivityStep(int adaptivityStep)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (adaptivityStep < 0 || adaptivityStep >= currentPythonEngineAgros()->postHermes()->activeViewField()->adaptivitySteps())
        throw out_of_range(QObject::tr("Adaptivity step for active field (%1) must be in the range from 0 to %2.").arg(currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId()).
                           arg(currentPythonEngineAgros()->postHermes()->activeViewField()->adaptivitySteps() - 1).toStdString());

    currentPythonEngineAgros()->postHermes()->setActiveAdaptivityStep(adaptivityStep);

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setActiveSolutionType(const std::string &solutionType)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!solutionTypeStringKeys().contains(QString::fromStdString(solutionType)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

    if (currentPythonEngineAgros()->postHermes()->activeViewField()->adaptivityType() != AdaptivityType_None)
        currentPythonEngineAgros()->postHermes()->setActiveAdaptivitySolutionType(solutionTypeFromStringKey(QString::fromStdString(solutionType)));
    else
        throw logic_error(QObject::tr("Field '%1' was solved with space adaptivity.").arg(currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId()).toStdString());

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setProblemSetting(ProblemSetting::Type type, bool value)
{
    Agros2D::problem()->setting()->setValue(type, value);

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setProblemSetting(ProblemSetting::Type type, int value)
{
    Agros2D::problem()->setting()->setValue(type, value);

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMeshAndPost::setProblemSetting(ProblemSetting::Type type, double value)
{
    Agros2D::problem()->setting()->setValue(type, value);

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

// ************************************************************************************

void PyViewMesh::activate()
{
    if (Agros2D::problem()->isMeshed())
    {
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
    }
    else
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
}

void PyViewMesh::setOrderViewPalette(const std::string &palette)
{
    if (paletteOrderTypeStringKeys().contains(QString::fromStdString(palette)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_OrderPaletteOrderType, paletteOrderTypeFromStringKey(QString::fromStdString(palette)));
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteOrderTypeStringKeys())).toStdString());
}

// ************************************************************************************

void PyViewPost::setScalarViewVariable(const std::string &var)
{
    // scalar variables
    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewScalarVariables())
    {
        list.append(variable.id());
        if (variable.id() == QString::fromStdString(var))
        {
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariable, QString::fromStdString(var));
            if (!silentMode())
                currentPythonEngineAgros()->postHermes()->refresh();

            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost::setScalarViewVariableComp(const std::string &component)
{
    if (physicFieldVariableCompTypeStringKeys().contains(QString::fromStdString(component)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, physicFieldVariableCompFromStringKey(QString::fromStdString(component)));
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPalette(const std::string &palette)
{
    if (paletteTypeStringKeys().contains(QString::fromStdString(palette)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteType, paletteTypeFromStringKey(QString::fromStdString(palette)));
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPaletteQuality(const std::string &quality)
{
    if (paletteQualityStringKeys().contains(QString::fromStdString(quality)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_LinearizerQuality, paletteQualityFromStringKey(QString::fromStdString(quality)));
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteQualityStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPaletteSteps(int steps)
{
    if (steps >= PALETTESTEPSMIN && steps <= PALETTESTEPSMAX)
        setProblemSetting(ProblemSetting::View_PaletteSteps, steps);
    else
        throw invalid_argument(QObject::tr("Palette steps must be in the range from %1 to %2.").arg(PALETTESTEPSMIN).arg(PALETTESTEPSMAX).toStdString());
}

void PyViewPost::setScalarViewDecimalPlace(int place)
{
    if (place >= SCALARDECIMALPLACEMIN && place <= SCALARDECIMALPLACEMAX)
        setProblemSetting(ProblemSetting::View_ScalarDecimalPlace, place);
    else
        throw invalid_argument(QObject::tr("Decimal place must be in the range from %1 to %2.").arg(SCALARDECIMALPLACEMIN).arg(SCALARDECIMALPLACEMAX).toStdString());
}

void PyViewPost::setScalarViewRangeBase(double base)
{
    if (base > 0.0 && base != 1)
        setProblemSetting(ProblemSetting::View_ScalarRangeBase, base);
    else
        throw invalid_argument(QObject::tr("Logarithm base must be possitive and can not be equal to 1.").toStdString());
}

// ************************************************************************************

void PyViewPost2D::activate()
{
    if (Agros2D::problem()->isSolved())
    {
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
    }
    else
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

void PyViewPost2D::setContourCount(int count)
{
    if (count >= CONTOURSCOUNTMIN && count <= CONTOURSCOUNTMAX)
        setProblemSetting(ProblemSetting::View_ContoursCount, count);
    else
        throw invalid_argument(QObject::tr("Contour count must be in the range from %1 to %2.").arg(CONTOURSCOUNTMIN).arg(CONTOURSCOUNTMAX).toStdString());
}

void PyViewPost2D::setContourVariable(const std::string &var)
{
    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewScalarVariables())
    {
        if (variable.isScalar())
        {
            list.append(variable.id());

            if (variable.id() == QString::fromStdString(var))
            {
                Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContourVariable, QString::fromStdString(var));
                if (!silentMode())
                    currentPythonEngineAgros()->postHermes()->refresh();

                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorCount(int count)
{
    if (count >= VECTORSCOUNTMIN && count <= VECTORSCOUNTMAX)
        setProblemSetting(ProblemSetting::View_VectorCount, count);
    else
        throw invalid_argument(QObject::tr("Vector count must be in the range from %1 to %2.").arg(VECTORSCOUNTMIN).arg(VECTORSCOUNTMAX).toStdString());
}

void PyViewPost2D::setVectorScale(double scale)
{
    if (scale >= VECTORSSCALEMIN && scale <= VECTORSSCALEMAX)
        setProblemSetting(ProblemSetting::View_VectorScale, scale);
    else
        throw invalid_argument(QObject::tr("Vector scale must be in the range from %1 to %2.").arg(VECTORSSCALEMIN).arg(VECTORSSCALEMAX).toStdString());
}

void PyViewPost2D::setVectorVariable(const std::string &var)
{
    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewVectorVariables())
    {
        list.append(variable.id());
        if (variable.id() == QString::fromStdString(var))
        {
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorVariable, QString::fromStdString(var));
            if (!silentMode())
                currentPythonEngineAgros()->postHermes()->refresh();

            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

// ************************************************************************************

void PyViewPost3D::activate()
{
    if (Agros2D::problem()->isSolved())
    {
        if(!silentMode())
            currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->trigger();
    }
    else
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

void PyViewPost3D::setPost3DMode(const std::string &mode)
{
    if (sceneViewPost3DModeStringKeys().contains(QString::fromStdString(mode)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, sceneViewPost3DModeFromStringKey(QString::fromStdString(mode)));
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(sceneViewPost3DModeStringKeys())).toStdString());
}

// ************************************************************************************

void PyViewParticleTracing::activate()
{
    if (Agros2D::problem()->isSolved())
    {
        if (!silentMode())
        {
            currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->trigger();
            currentPythonEngineAgros()->postHermes()->refresh();
        }
    }
    else
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}
