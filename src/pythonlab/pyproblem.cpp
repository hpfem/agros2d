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

#include "pythonlab/pyproblem.h"
#include "pythonlab/pythonengine_agros.h"

#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"

#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"

PyProblem::PyProblem(bool clearproblem)
{
    if (clearproblem)
        clear();
}

void PyProblem::setCoordinateType(const char *coordinateType)
{
    if (coordinateTypeStringKeys().contains(QString(coordinateType)))
        Util::problem()->config()->setCoordinateType(coordinateTypeFromStringKey(QString(coordinateType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(coordinateTypeStringKeys())).toStdString());
}

void PyProblem::setMeshType(const char *meshType)
{
    if (meshTypeStringKeys().contains(QString(meshType)))
        Util::problem()->config()->setMeshType(meshTypeFromStringKey(QString(meshType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(meshTypeStringKeys())).toStdString());
}

void PyProblem::setMatrixSolver(const char *matrixSolver)
{
    if (matrixSolverTypeStringKeys().contains(QString(matrixSolver)))
        Util::problem()->config()->setMatrixSolver(matrixSolverTypeFromStringKey(QString(matrixSolver)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(matrixSolverTypeStringKeys())).toStdString());
}

void PyProblem::setFrequency(const double frequency)
{
    if (frequency >= 0.0)
        Util::problem()->config()->setFrequency(frequency);
    else
        throw invalid_argument(QObject::tr("The frequency must be positive.").toStdString());
}

void PyProblem::setTimeStepMethod(const char *timeStepMethod)
{
    if (timeStepMethodStringKeys().contains(QString(timeStepMethod)))
        Util::problem()->config()->setTimeStepMethod((TimeStepMethod) timeStepMethodFromStringKey(QString(timeStepMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(timeStepMethodStringKeys())).toStdString());
}

void PyProblem::setTimeMethodOrder(const int timeMethodOrder)
{
    if (timeMethodOrder >= 1)
        Util::problem()->config()->setTimeOrder(timeMethodOrder);
    else
        throw invalid_argument(QObject::tr("Number of time method order must be greater then 1.").toStdString());
}

void PyProblem::setTimeMethodTolerance(const double timeMethodTolerance)
{
    if (timeMethodTolerance > 0.0)
        Util::problem()->config()->setTimeMethodTolerance(Value(QString::number(timeMethodTolerance)));
    else
        throw invalid_argument(QObject::tr("The time method tolerance must be positive.").toStdString());
}

void PyProblem::setNumConstantTimeSteps(const int timeSteps)
{
    if (timeSteps >= 1)
        Util::problem()->config()->setNumConstantTimeSteps(timeSteps);
    else
        throw invalid_argument(QObject::tr("Number of time steps must be greater then 1.").toStdString());
}

void PyProblem::setTimeTotal(const double timeTotal)
{
    if (timeTotal >= 0.0)
        Util::problem()->config()->setTimeTotal(Value(QString::number(timeTotal)));
    else
        throw invalid_argument(QObject::tr("The total time must be positive.").toStdString());
}

char *PyProblem::getCouplingType(const char *sourceField, const char *targetField)
{
    if (Util::problem()->hasCoupling(QString(sourceField),
                                     QString(targetField)))
    {
        CouplingInfo *couplingInfo = Util::problem()->couplingInfo(QString(sourceField),
                                                                   QString(targetField));

        return const_cast<char*>(couplingTypeToStringKey(couplingInfo->couplingType()).toStdString().c_str());
    }
    else
        throw invalid_argument(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(QString(sourceField)).arg(QString(targetField)).toStdString());
}

void PyProblem::setCouplingType(const char *sourceField, const char *targetField, const char *type)
{
    if (Util::problem()->hasCoupling(QString(sourceField),
                                     QString(targetField)))
    {
        CouplingInfo *couplingInfo = Util::problem()->couplingInfo(QString(sourceField),
                                                                   QString(targetField));

        if (couplingTypeStringKeys().contains(QString(type)))
            couplingInfo->setCouplingType(couplingTypeFromStringKey(QString(type)));
        else
            throw invalid_argument(QObject::tr("Invalid coupling type key. Valid keys: %1").arg(stringListToString(couplingTypeStringKeys())).toStdString());
    }
    else
        throw invalid_argument(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(QString(sourceField)).arg(QString(targetField)).toStdString());
}

void PyProblem::clear()
{
    Util::problem()->clearFieldsAndConfig();
    Util::scene()->clear();
}

void PyProblem::refresh()
{
    Util::scene()->invalidate();

    // refresh post view
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyProblem::solve()
{
    Util::scene()->invalidate();

    // trigger preprocessor
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

    Util::problem()->solve();
    if (Util::problem()->isSolved())
    {
        // trigger postprocessor
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
    }
}
