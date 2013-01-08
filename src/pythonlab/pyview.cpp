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

#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"

#include "util/constants.h"
#include "util/global.h"

void PyViewConfig::setField(char* fieldid)
{
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if (fieldInfo->fieldId() == QString(fieldid))
        {
            Agros2D::scene()->setActiveViewField(fieldInfo);
            currentPythonEngineAgros()->postHermes()->refresh();
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::problem()->fieldInfos().keys())).toStdString());
}

void PyViewConfig::setActiveTimeStep(int timeStep)
{
    if (timeStep < 0 || timeStep >= Agros2D::problem()->numTimeLevels())
        throw invalid_argument(QObject::tr("Time step must be in the range from 0 to %1.").arg(Agros2D::problem()->numTimeLevels()).toStdString());

    Agros2D::scene()->setActiveTimeStep(timeStep);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewConfig::setActiveAdaptivityStep(int adaptivityStep)
{
    if (adaptivityStep < 0 || adaptivityStep > Agros2D::scene()->activeViewField()->adaptivitySteps())
        throw invalid_argument(QObject::tr("Adaptivity step for active field (%1) must be in the range from 0 to %2.").arg(Agros2D::scene()->activeViewField()->fieldId()).arg(Agros2D::scene()->activeViewField()->adaptivitySteps()).toStdString());

    Agros2D::scene()->setActiveAdaptivityStep(adaptivityStep);
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewConfig::setActiveSolutionType(char* solutionType)
{
    if (solutionTypeStringKeys().contains(QString(solutionType)))
    {
        Agros2D::scene()->setActiveSolutionType(solutionTypeFromStringKey(QString(solutionType)));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());
}

void PyViewConfig::setGridShow(bool show)
{
    Agros2D::problem()->configView()->showGrid = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewConfig::setGridStep(double step)
{
    Agros2D::problem()->configView()->gridStep = step;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewConfig::setAxesShow(bool show)
{
    Agros2D::problem()->configView()->showAxes = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewConfig::setRulersShow(bool show)
{
    Agros2D::problem()->configView()->showRulers = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

// ****************************************************************************************************

void PyViewMesh::activate()
{
    if (Agros2D::problem()->isMeshed())
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
}

void PyViewMesh::setInitialMeshViewShow(bool show)
{
    Agros2D::problem()->configView()->showInitialMeshView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setSolutionMeshViewShow(bool show)
{
    Agros2D::problem()->configView()->showSolutionMeshView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setOrderViewShow(bool show)
{
    Agros2D::problem()->configView()->showOrderView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setOrderViewColorBar(bool show)
{
    Agros2D::problem()->configView()->showOrderColorBar = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setOrderViewLabel(bool show)
{
    Agros2D::problem()->configView()->orderLabel = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewMesh::setOrderViewPalette(char* palette)
{
    if (paletteOrderTypeStringKeys().contains(QString(palette)))
    {
        Agros2D::problem()->configView()->orderPaletteOrderType = paletteOrderTypeFromStringKey(QString(palette));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteOrderTypeStringKeys())).toStdString());
}

// ****************************************************************************************************

void PyViewPost::setScalarViewVariable(char* var)
{
    QStringList list;

    // scalar variables
    foreach (Module::LocalVariable *variable, Agros2D::scene()->activeViewField()->module()->viewScalarVariables())
    {
        list.append(variable->id());
        if (variable->id() == QString(var))
        {
            Agros2D::problem()->configView()->scalarVariable = QString(var);
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
        Agros2D::problem()->configView()->scalarVariableComp = physicFieldVariableCompFromStringKey(QString(component));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPalette(char* palette)
{
    if (paletteTypeStringKeys().contains(QString(palette)))
    {
        Agros2D::problem()->configView()->paletteType = paletteTypeFromStringKey(QString(palette));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPaletteQuality(char* quality)
{
    if (paletteQualityStringKeys().contains(QString(quality)))
    {
        Agros2D::problem()->configView()->linearizerQuality = paletteQualityFromStringKey(QString(quality));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteQualityStringKeys())).toStdString());
}

void PyViewPost::setScalarViewPaletteSteps(int steps)
{
    if (steps >= PALETTESTEPSMIN && steps <= PALETTESTEPSMAX)
    {
        Agros2D::problem()->configView()->paletteSteps = steps;
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Palette steps must be in the range from %1 to %2.").arg(PALETTESTEPSMIN).arg(PALETTESTEPSMAX).toStdString());
}

void PyViewPost::setScalarViewPaletteFilter(bool filter)
{
    Agros2D::problem()->configView()->paletteFilter = filter;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeLog(bool log)
{
    Agros2D::problem()->configView()->scalarRangeLog = log;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeBase(double base)
{
    Agros2D::problem()->configView()->scalarRangeBase = base;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewColorBar(bool show)
{
    Agros2D::problem()->configView()->showScalarColorBar = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewDecimalPlace(int place)
{
    if (place >= SCALARDECIMALPLACEMIN && place <= SCALARDECIMALPLACEMAX)
    {
        Agros2D::problem()->configView()->scalarDecimalPlace = place;
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Decimal place must be in the range from %1 to %2.").arg(SCALARDECIMALPLACEMIN).arg(SCALARDECIMALPLACEMAX).toStdString());
}

void PyViewPost::setScalarViewRangeAuto(bool autoRange)
{
    Agros2D::problem()->configView()->scalarRangeAuto = autoRange;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeMin(double min)
{
    Agros2D::problem()->configView()->scalarRangeMin = min;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setScalarViewRangeMax(double max)
{
    Agros2D::problem()->configView()->scalarRangeMax = max;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setParticleColoredByVelocity(bool colored)
{
    Agros2D::problem()->configView()->particleColorByVelocity = colored;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost::setParticleShowPoints(bool points)
{
    Agros2D::problem()->configView()->particleShowPoints = points;
    currentPythonEngineAgros()->postHermes()->refresh();
}


// ****************************************************************************************************

void PyViewPost2D::activate()
{
    if (Agros2D::problem()->isSolved())
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
}

void PyViewPost2D::setScalarViewShow(bool show)
{
    Agros2D::problem()->configView()->showScalarView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost2D::setContourShow(bool show)
{
    Agros2D::problem()->configView()->showContourView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost2D::setContourCount(int count)
{
    if (count >= CONTOURSCOUNTMIN && count <= CONTOURSCOUNTMAX)
    {
        Agros2D::problem()->configView()->contoursCount = count;
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Contour count must be in the range from %1 to %2.").arg(CONTOURSCOUNTMIN).arg(CONTOURSCOUNTMAX).toStdString());
}

void PyViewPost2D::setContourVariable(char* var)
{
    QStringList list;
    foreach (Module::LocalVariable *variable, Agros2D::scene()->activeViewField()->module()->viewScalarVariables())
    {
        if (variable->isScalar())
        {
            list.append(variable->id());

            if (variable->id() == QString(var))
            {
                Agros2D::problem()->configView()->contourVariable = QString(var);
                currentPythonEngineAgros()->postHermes()->refresh();
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorShow(bool show)
{
    Agros2D::problem()->configView()->showVectorView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost2D::setVectorCount(int count)
{
    if (count >= VECTORSCOUNTMIN && count <= VECTORSCOUNTMAX)
    {
        Agros2D::problem()->configView()->vectorCount = count;
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Vector count must be in the range from %1 to %2.").arg(VECTORSCOUNTMIN).arg(VECTORSCOUNTMAX).toStdString());
}

void PyViewPost2D::setVectorScale(double scale)
{
    if (scale >= VECTORSSCALEMIN && scale <= VECTORSSCALEMAX)
    {
        Agros2D::problem()->configView()->vectorScale = scale;
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Vector scale must be in the range from %1 to %2.").arg(VECTORSSCALEMIN).arg(VECTORSSCALEMAX).toStdString());
}

void PyViewPost2D::setVectorVariable(char* var)
{
    QStringList list;
    foreach (Module::LocalVariable *variable, Agros2D::scene()->activeViewField()->module()->viewVectorVariables())
    {
        list.append(variable->id());
        if (variable->id() == QString(var))
        {
            Agros2D::problem()->configView()->vectorVariable = QString(var);
            currentPythonEngineAgros()->postHermes()->refresh();
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorProportional(bool show)
{
    Agros2D::problem()->configView()->vectorProportional = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost2D::setVectorColor(bool show)
{
    Agros2D::problem()->configView()->vectorColor = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyViewPost2D::setParticleShow(bool show)
{
    Agros2D::problem()->configView()->showParticleView = show;
    currentPythonEngineAgros()->postHermes()->refresh();
}

// ****************************************************************************************************

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
        Agros2D::problem()->configView()->showPost3D = sceneViewPost3DModeFromStringKey(QString(mode));
        currentPythonEngineAgros()->postHermes()->refresh();
    }
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(sceneViewPost3DModeStringKeys())).toStdString());
}

// ****************************************************************************************************

void PyView::saveImageToFile(char *file, int width, int height)
{
    ErrorResult result;
    // TODO: (Franta) preprocessor
    if (currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->isChecked())
        result = currentPythonEngineAgros()->sceneViewMesh()->saveImageToFile(file, width, height);

    else if (currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->isChecked())
        result = currentPythonEngineAgros()->sceneViewPost2D()->saveImageToFile(file, width, height);

    else if (currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->isChecked())
        result = currentPythonEngineAgros()->sceneViewPost3D()->saveImageToFile(file, width, height);

    else
        result = ErrorResult(ErrorResultType_Critical, QObject::tr("Image is not saved."));

    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}
