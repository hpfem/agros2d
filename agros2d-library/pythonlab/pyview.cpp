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
#include "pythonlab/pythonengine_agros.h"

#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"

#include "hermes2d/module.h"
#include "hermes2d/solutionstore.h"

#include "util/constants.h"
#include "util/global.h"

void PyView::saveImageToFile(const char *file, int width, int height)
{
    if (!silentMode())
    {
        if (currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->isChecked())
            currentPythonEngineAgros()->sceneViewPreprocessor()->saveImageToFile(file, width, height);
        else if (currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->isChecked())
            currentPythonEngineAgros()->sceneViewMesh()->saveImageToFile(file, width, height);
        else if (currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->isChecked())
            currentPythonEngineAgros()->sceneViewPost2D()->saveImageToFile(file, width, height);
        else if (currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->isChecked())
            currentPythonEngineAgros()->sceneViewPost3D()->saveImageToFile(file, width, height);
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

void PyViewConfig::set(ProblemSetting::Type type, QVariant value)
{
    switch(value.type())
    {
    case QVariant::Bool:
        Agros2D::problem()->setting()->setValue(type, value.toBool());
        break;
    case QVariant::Double:
        Agros2D::problem()->setting()->setValue(type, value.toDouble());
        break;
    }

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewConfig::setGridStep(double step)
{
    if (step > 0.0)
        set(ProblemSetting::View_GridStep, step);
    else
        throw out_of_range(QObject::tr("Grid step must be positive.").toStdString());
}

// ************************************************************************************

void PyViewMeshAndPost::setField(const char* fieldid)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!Agros2D::problem()->hasField(fieldid))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::problem()->fieldInfos().keys())).toStdString());

    currentPythonEngineAgros()->postHermes()->setActiveViewField(Agros2D::problem()->fieldInfo(fieldid));
    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

char* PyViewMeshAndPost::getField()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    return const_cast<char*>(currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId().toStdString().c_str());
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

    currentPythonEngineAgros()->postHermes()->setActiveTimeStep(timeStep);
    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

int PyViewMeshAndPost::getActiveTimeStep()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    return currentPythonEngineAgros()->postHermes()->activeTimeStep();
}

int PyViewMeshAndPost::getActiveAdaptivityStep()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    return currentPythonEngineAgros()->postHermes()->activeAdaptivityStep();
}

void PyViewMeshAndPost::setActiveAdaptivityStep(int adaptivityStep)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (adaptivityStep < 0 || adaptivityStep >= currentPythonEngineAgros()->postHermes()->activeViewField()->adaptivitySteps())
        throw out_of_range(QObject::tr("Adaptivity step for active field (%1) must be in the range from 0 to %2.").arg(currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId()).arg(currentPythonEngineAgros()->postHermes()->activeViewField()->adaptivitySteps() - 1).toStdString());

    currentPythonEngineAgros()->postHermes()->setActiveAdaptivityStep(adaptivityStep);
    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

char* PyViewMeshAndPost::getActiveSolutionType()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    return const_cast<char*>(solutionTypeToStringKey(currentPythonEngineAgros()->postHermes()->activeAdaptivitySolutionType()).toStdString().c_str());
}

void PyViewMeshAndPost::setActiveSolutionType(const char *solutionType)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!solutionTypeStringKeys().contains(QString(solutionType)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

    currentPythonEngineAgros()->postHermes()->setActiveAdaptivitySolutionType(solutionTypeFromStringKey(QString(solutionType)));
    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

// ************************************************************************************

void PyViewMesh::activate()
{
    if (Agros2D::problem()->isMeshed() && !silentMode())
        currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
}

void PyViewMesh::set(ProblemSetting::Type type, QVariant value)
{
    if (!Agros2D::problem()->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());

    switch(value.type())
    {
    case QVariant::Bool:
        Agros2D::problem()->setting()->setValue(type, value.toBool());
        break;
    }

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setOrderViewPalette(char* palette)
{
    if (paletteOrderTypeStringKeys().contains(QString(palette)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_OrderPaletteOrderType, paletteOrderTypeFromStringKey(QString(palette)));
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteOrderTypeStringKeys())).toStdString());
}

// ************************************************************************************

void PyViewPost::setScalarViewVariable(char* var)
{
    QStringList list;

    // scalar variables
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewScalarVariables())
    {
        list.append(variable.id());
        if (variable.id() == QString(var))
        {
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariable, QString(var));
            currentPythonEngineAgros()->postHermes()->refresh();
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost::setScalarViewVariableComp(char* component)
{
    if (physicFieldVariableCompTypeStringKeys().contains(QString(component)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, physicFieldVariableCompFromStringKey(QString(component)));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPalette(char* palette)
{
    if (paletteTypeStringKeys().contains(QString(palette)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteType, paletteTypeFromStringKey(QString(palette)));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPaletteQuality(char* quality)
{
    if (paletteQualityStringKeys().contains(QString(quality)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_LinearizerQuality, paletteQualityFromStringKey(QString(quality)));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteQualityStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPaletteSteps(int steps)
{
    if (steps >= PALETTESTEPSMIN && steps <= PALETTESTEPSMAX)
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteSteps, steps);
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Palette steps must be in the range from %1 to %2.").arg(PALETTESTEPSMIN).arg(PALETTESTEPSMAX).toStdString());
}

void PyViewPost::setScalarViewPaletteFilter(bool filter)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteFilter, filter);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeLog(bool log)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeLog, log);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeBase(double base)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeBase, base);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewColorBar(bool show)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowScalarColorBar, show);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewDecimalPlace(int place)
{
    if (place >= SCALARDECIMALPLACEMIN && place <= SCALARDECIMALPLACEMAX)
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarDecimalPlace, place);
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Decimal place must be in the range from %1 to %2.").arg(SCALARDECIMALPLACEMIN).arg(SCALARDECIMALPLACEMAX).toStdString());
}

void PyViewPost::setScalarViewRangeAuto(bool autoRange)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeAuto, autoRange);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeMin(double min)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeMin, min);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeMax(double max)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeMax, max);
    currentPythonEngineAgros()->postHermes()->refresh();
}

// ************************************************************************************

void PyViewPost2D::activate()
{
    if (Agros2D::problem()->isSolved() && !silentMode())
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
}

void PyViewPost2D::set(ProblemSetting::Type type, QVariant value)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    switch(value.type())
    {
    case QVariant::Bool:
        Agros2D::problem()->setting()->setValue(type, value.toBool());
        break;
    }

    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost2D::setContourCount(int count)
{
    if (count >= CONTOURSCOUNTMIN && count <= CONTOURSCOUNTMAX)
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContoursCount, count);
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Contour count must be in the range from %1 to %2.").arg(CONTOURSCOUNTMIN).arg(CONTOURSCOUNTMAX).toStdString());
}

void PyViewPost2D::setContourVariable(char* var)
{
    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewScalarVariables())
    {
        if (variable.isScalar())
        {
            list.append(variable.id());

            if (variable.id() == QString(var))
            {
                Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContourVariable, QString(var));
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
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorCount, count);
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Vector count must be in the range from %1 to %2.").arg(VECTORSCOUNTMIN).arg(VECTORSCOUNTMAX).toStdString());
}

void PyViewPost2D::setVectorScale(double scale)
{
    if (scale >= VECTORSSCALEMIN && scale <= VECTORSSCALEMAX)
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorScale, scale);
        if (!silentMode())
            currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Vector scale must be in the range from %1 to %2.").arg(VECTORSSCALEMIN).arg(VECTORSSCALEMAX).toStdString());
}

void PyViewPost2D::setVectorVariable(char* var)
{
    QStringList list;
    foreach (Module::LocalVariable variable, currentPythonEngineAgros()->postHermes()->activeViewField()->viewVectorVariables())
    {
        list.append(variable.id());
        if (variable.id() == QString(var))
        {
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorVariable, QString(var));
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
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->trigger();
}

void PyViewPost3D::setPost3DMode(char* mode)
{
    if (sceneViewPost3DModeStringKeys().contains(QString(mode)))
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, sceneViewPost3DModeFromStringKey(QString(mode)));
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
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->trigger();
}

/*
void PyView{Post2DParticleTracing::setParticleShow(bool show)
{
    Agros2D::problem()->configView()->showParticleView = show;
    if (!silentMode())
        currentPythonEngineAgros()->postHermes()->refresh();
}
*/
