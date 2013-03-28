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
#include "sceneview_geometry.h"
#include "sceneview_post2d.h"
#include "hermes2d/coupling.h"

PyProblem::PyProblem(bool clearProblem)
{
    if (clearProblem)
        clear();
}

void PyProblem::clear()
{
    Agros2D::scene()->clear();
}

void PyProblem::clearSolution()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    Agros2D::problem()->clearSolution();
    currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();
}

void PyProblem::refresh()
{
    Agros2D::scene()->invalidate();
    currentPythonEngineAgros()->postHermes()->refresh();
}

void PyProblem::setCoordinateType(const char *coordinateType)
{
    if (coordinateTypeStringKeys().contains(QString(coordinateType)))
        Agros2D::problem()->config()->setCoordinateType(coordinateTypeFromStringKey(QString(coordinateType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(coordinateTypeStringKeys())).toStdString());

    Agros2D::scene()->invalidate();
}

void PyProblem::setMeshType(const char *meshType)
{
    if (meshTypeStringKeys().contains(QString(meshType)))
        Agros2D::problem()->config()->setMeshType(meshTypeFromStringKey(QString(meshType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(meshTypeStringKeys())).toStdString());
}

void PyProblem::setMatrixSolver(const char *matrixSolver)
{
    if (matrixSolverTypeStringKeys().contains(QString(matrixSolver)))
        Agros2D::problem()->config()->setMatrixSolver(matrixSolverTypeFromStringKey(QString(matrixSolver)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(matrixSolverTypeStringKeys())).toStdString());
}

void PyProblem::setFrequency(const double frequency)
{
    if (frequency > 0.0)
        Agros2D::problem()->config()->setFrequency(frequency);
    else
        throw out_of_range(QObject::tr("The frequency must be positive.").toStdString());
}

void PyProblem::setTimeStepMethod(const char *timeStepMethod)
{
    if (timeStepMethodStringKeys().contains(QString(timeStepMethod)))
        Agros2D::problem()->config()->setTimeStepMethod((TimeStepMethod) timeStepMethodFromStringKey(QString(timeStepMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(timeStepMethodStringKeys())).toStdString());
}

void PyProblem::setTimeMethodOrder(const int timeMethodOrder)
{
    if (timeMethodOrder >= 1)
        Agros2D::problem()->config()->setTimeOrder(timeMethodOrder);
    else
        throw out_of_range(QObject::tr("Number of time method order must be greater then 1.").toStdString());
}

void PyProblem::setTimeMethodTolerance(const double timeMethodTolerance)
{
    if (timeMethodTolerance > 0.0)
        Agros2D::problem()->config()->setTimeMethodTolerance(timeMethodTolerance);
    else
        throw out_of_range(QObject::tr("The time method tolerance must be positive.").toStdString());
}

void PyProblem::setNumConstantTimeSteps(const int timeSteps)
{
    if (timeSteps >= 1)
        Agros2D::problem()->config()->setTimeNumConstantTimeSteps(timeSteps);
    else
        throw out_of_range(QObject::tr("Number of time steps must be greater then 1.").toStdString());
}

void PyProblem::setTimeTotal(const double timeTotal)
{
    if (timeTotal >= 0.0)
        Agros2D::problem()->config()->setTimeTotal(timeTotal);
    else
        throw out_of_range(QObject::tr("The total time must be positive.").toStdString());
}

char *PyProblem::getCouplingType(const char *sourceField, const char *targetField)
{
    checkExistingFields(QString(sourceField), QString(targetField));

    if (Agros2D::problem()->hasCoupling(QString(sourceField), QString(targetField)))
    {
        CouplingInfo *couplingInfo = Agros2D::problem()->couplingInfo(QString(sourceField), QString(targetField));
        return const_cast<char*>(couplingTypeToStringKey(couplingInfo->couplingType()).toStdString().c_str());
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(QString(sourceField)).arg(QString(targetField)).toStdString());
}

void PyProblem::setCouplingType(const char *sourceField, const char *targetField, const char *type)
{
    checkExistingFields(QString(sourceField), QString(targetField));

    if (Agros2D::problem()->hasCoupling(QString(sourceField), QString(targetField)))
    {
        CouplingInfo *couplingInfo = Agros2D::problem()->couplingInfo(QString(sourceField), QString(targetField));
        if (couplingTypeStringKeys().contains(QString(type)))
            couplingInfo->setCouplingType(couplingTypeFromStringKey(QString(type)));
        else
            throw invalid_argument(QObject::tr("Invalid coupling type key. Valid keys: %1").arg(stringListToString(couplingTypeStringKeys())).toStdString());
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(QString(sourceField)).arg(QString(targetField)).toStdString());
}

void PyProblem::checkExistingFields(QString sourceField, QString targetField)
{
    if (Agros2D::problem()->fieldInfos().isEmpty())
        throw logic_error(QObject::tr("No fields are defined.").toStdString());

    if (!Agros2D::problem()->fieldInfos().contains(sourceField))
        throw logic_error(QObject::tr("Source field '%1' is not defined.").arg(sourceField).toStdString());

    if (!Agros2D::problem()->fieldInfos().contains(targetField))
        throw logic_error(QObject::tr("Target field '%1' is not defined.").arg(targetField).toStdString());
}

void PyProblem::mesh()
{
    Agros2D::scene()->invalidate();

    Agros2D::problem()->mesh();
    if (Agros2D::problem()->isMeshed())
    {
        // trigger postprocessor
        if (!silentMode())
        {
            currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
            QApplication::processEvents();
        }
    }
    else
    {
        // trigger preprocessor
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
    }
}

void PyProblem::solve()
{
    Agros2D::scene()->invalidate();

    Agros2D::problem()->solve();
    if (Agros2D::problem()->isSolved())
    {
        // trigger postprocessor
        if (!silentMode())
        {
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
            QApplication::processEvents();
        }
    }
    else
    {
        // trigger preprocessor
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }
}

void PyProblem::solveAdaptiveStep()
{
    Agros2D::scene()->invalidate();
    Agros2D::problem()->solveAdaptiveStep();

    if (Agros2D::problem()->isSolved())
    {
        // trigger postprocessor
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
    }
    else
    {
        // trigger preprocessor
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }
}

double PyProblem::timeElapsed()
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    double time = Agros2D::problem()->timeElapsed().hour()*3600 + Agros2D::problem()->timeElapsed().minute()*60 +
                  Agros2D::problem()->timeElapsed().second() + Agros2D::problem()->timeElapsed().msec() * 1e-3;
    return time;
}

void PyProblem::timeStepsLength(vector<double> &steps)
{
    if (!Agros2D::problem()->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    for (int i = 0; i < Agros2D::problem()->timeStepLengths().size(); i++)
        steps.push_back(Agros2D::problem()->timeStepLengths().at(i));
}
