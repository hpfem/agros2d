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

#include "pythonlab/pyfield.h"
#include "pythonlab/pythonengine_agros.h"

#include "hermes2d/plugin_interface.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "scenemarker.h"
#include "sceneview_post2d.h"

PyField::PyField(char *fieldId)
{
    QMap<QString, QString> modules = Module::availableModules();

    if (modules.keys().contains(QString(fieldId)))
        if (Agros2D::problem()->hasField(QString(fieldId)))
        {
            m_fieldInfo = Agros2D::problem()->fieldInfo(fieldId);
        }
        else
        {
            try{
                m_fieldInfo = new FieldInfo(fieldId);
            }
            catch(AgrosPluginException& e)
            {
                throw invalid_argument(QObject::tr("Invalid field id. Plugin %1 cannot be loaded").arg(QString(fieldId)).toStdString());
            }

            Agros2D::problem()->addField(fieldInfo());
        }
    else
        throw invalid_argument(QObject::tr("Invalid field id. Valid keys: %1").arg(stringListToString(modules.keys())).toStdString());
}

FieldInfo *PyField::fieldInfo()
{
    return m_fieldInfo;
}

void PyField::setAnalysisType(const char *analysisType)
{
    if (m_fieldInfo->analyses().contains(analysisTypeFromStringKey(QString(analysisType))))
    {
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setAnalysisType(analysisTypeFromStringKey(QString(analysisType)));
    }
    else
    {
        QStringList list;
        QList<AnalysisType> analyses = m_fieldInfo->analyses().keys();
        foreach (AnalysisType analysis, analyses)
            list.append(analysisTypeToStringKey(analysis));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyField::setNumberOfRefinements(const int numberOfRefinements)
{
    if (numberOfRefinements >= 0 && numberOfRefinements <= 5)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setNumberOfRefinements(numberOfRefinements);
    else
        throw out_of_range(QObject::tr("Number of refinements is out of range (0 - 5).").toStdString());
}

void PyField::setPolynomialOrder(const int polynomialOrder)
{
    if (polynomialOrder > 0 && polynomialOrder <= 10)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setPolynomialOrder(polynomialOrder);
    else
        throw out_of_range(QObject::tr("Polynomial order is out of range (1 - 10).").toStdString());
}

void PyField::setLinearityType(const char *linearityType)
{
    if (linearityTypeStringKeys().contains(QString(linearityType)))
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setLinearityType(linearityTypeFromStringKey(QString(linearityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(linearityTypeStringKeys())).toStdString());
}

void PyField::setNonlinearTolerance(const double nonlinearTolerance)
{
    if (nonlinearTolerance > 0.0)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setNonlinearTolerance(nonlinearTolerance);
    else
        throw out_of_range(QObject::tr("Nonlinearity tolerance must be positive.").toStdString());
}

void PyField::setNonlinearSteps(const int nonlinearSteps)
{
    if (nonlinearSteps >= 1)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setNonlinearSteps(nonlinearSteps);
    else
        throw out_of_range(QObject::tr("Nonlinearity steps must be higher than 1.").toStdString());
}

void PyField::setDampingCoeff(const double dampingCoeff)
{
    if ((dampingCoeff <= 1) && (dampingCoeff > 0))
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setNewtonDampingCoeff(dampingCoeff);
    else
        throw out_of_range(QObject::tr("Damping coefficient must be between 0 and 1 .").toStdString());
}

void PyField::setAutomaticDamping(const bool automaticDamping)
{
    Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setNewtonAutomaticDamping(automaticDamping);
}

void PyField::setDampingNumberToIncrease(const int dampingNumberToIncrease)
{
    if ((dampingNumberToIncrease <= 5) && (dampingNumberToIncrease >= 1))
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setNewtonDampingNumberToIncrease(dampingNumberToIncrease);
    else
        throw out_of_range(QObject::tr("Number of steps needed to increase the damping coefficient must be between 1 and 5 .").toStdString());
}

void PyField::setPicardAndersonAcceleration(const bool acceleration)
{
    Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setPicardAndersonAcceleration(acceleration);
}

void PyField::setPicardAndersonBeta(const double beta)
{
    if ((beta <= 1) && (beta > 0))
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setPicardAndersonBeta(beta);
    else
        throw out_of_range(QObject::tr("Anderson coefficient must be between 0 and 1 .").toStdString());
}

void PyField::setPicardAndersonNumberOfLastVectors(const int number)
{
    if ((number <= 5) && (number >= 1))
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setPicardAndersonNumberOfLastVectors(number);
    else
        throw out_of_range(QObject::tr("Number of last vector must be between 1 and 5 .").toStdString());
}

void PyField::setAdaptivityType(const char *adaptivityType)
{
    if (adaptivityTypeStringKeys().contains(QString(adaptivityType)))
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivityType(adaptivityTypeFromStringKey(QString(adaptivityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityTypeStringKeys())).toStdString());
}

void PyField::setAdaptivityTolerance(const double adaptivityTolerance)
{
    if (adaptivityTolerance > 0.0)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivityTolerance(adaptivityTolerance);
    else
        throw out_of_range(QObject::tr("Adaptivity tolerance must be positive.").toStdString());
}

void PyField::setAdaptivitySteps(const int adaptivitySteps)
{
    if (adaptivitySteps >= 1)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivitySteps(adaptivitySteps);
    else
        throw out_of_range(QObject::tr("Adaptivity steps must be higher than 1.").toStdString());
}

void PyField::setAdaptivityBackSteps(const int adaptivityBackSteps)
{
    if (adaptivityBackSteps >= 0)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivityBackSteps(adaptivityBackSteps);
    else
        throw out_of_range(QObject::tr("Adaptivity back steps must be positive.").toStdString());
}

void PyField::setAdaptivityRedoneEach(const int adaptivityRedoneEach)
{
    if (adaptivityRedoneEach >= 1)
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivityRedoneEach(adaptivityRedoneEach);
    else
        throw out_of_range(QObject::tr("Adaptivity back steps must be higher than 1.").toStdString());
}

void PyField::setInitialCondition(const double initialCondition)
{
    Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setInitialCondition(initialCondition);
}

void PyField::setTimeSkip(const double timeSkip)
{
    if (timeSkip >= 0 && timeSkip <= Agros2D::problem()->config()->timeTotal().number())
        Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->setTimeSkip(timeSkip);
    else
        throw out_of_range(QObject::tr("Time skip is out of range (0 - %1).").arg(Agros2D::problem()->config()->timeTotal().number()).toStdString());
}

void PyField::addBoundary(const char *name, const char *type, map<char*, double> parameters, map<char *, char *> expressions)
{
    // check boundaries with same name
    foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(Agros2D::problem()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (boundary->name() == name)
            throw invalid_argument(QObject::tr("Boundary condition '%1' already exists.").arg(QString(name)).toStdString());
    }

    if (!Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->boundaryTypeContains(QString(type)))
        throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(type).toStdString());

    Module::BoundaryType boundaryType = Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->boundaryType(QString(type));

    // browse boundary parameters
    QHash<QString, Value> values;
    for (map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == QString(QString((*i).first)))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    values[variable.id()] = Value(m_fieldInfo,
                                                  (*i).second,
                                                  vector<double>(),
                                                  vector<double>());
                else
                    values[variable.id()] = Value(m_fieldInfo,
                                                  expressions[(*i).first],
                            vector<double>(),
                            vector<double>());
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString((*i).first)).toStdString());
    }

    Agros2D::scene()->addBoundary(new SceneBoundary(fieldInfo(), name, type, values));
}

void PyField::setBoundary(const char *name, const char *type, map<char*, double> parameters, map<char *, char *> expressions)
{
    SceneBoundary *sceneBoundary = Agros2D::scene()->getBoundary(fieldInfo(), QString(name));
    if (sceneBoundary == NULL)
        throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(name).toStdString());

    // browse boundary types
    foreach (Module::BoundaryType boundaryType, sceneBoundary->fieldInfo()->boundaryTypes())
    {
        if (QString(type) == boundaryType.id())
        {
            sceneBoundary->setType(QString(type));
            break;
        }
        else
            throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(type).toStdString());
    }

    // browse boundary parameters
    Module::BoundaryType boundaryType = Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->boundaryType(sceneBoundary->type());
    for (map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == QString(QString((*i).first)))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    sceneBoundary->setValue(QString((*i).first), Value(m_fieldInfo,
                                                                       (*i).second,
                                                                       vector<double>(),
                                                                       vector<double>()));
                else
                    sceneBoundary->setValue(QString((*i).first), Value(m_fieldInfo,
                                                                       expressions[(*i).first],
                                            vector<double>(),
                                            vector<double>()));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString((*i).first)).toStdString());
    }
}

void PyField::removeBoundary(char *name)
{
    Agros2D::scene()->removeBoundary(Agros2D::scene()->getBoundary(fieldInfo(), QString(name)));
}

void PyField::addMaterial(char *name, map<char*, double> parameters,
                          map<char*, char* > expressions,
                          map<char*, vector<double> > nonlin_x,
                          map<char*, vector<double> > nonlin_y)
{
    // check materials with same name
    foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(Agros2D::problem()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (material->name() == name)
            throw invalid_argument(QObject::tr("Material '%1' already exists.").arg(QString(name)).toStdString());
    }

    // browse material parameters
    QHash<QString, Value> values;
    for (std::map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        QList<Module::MaterialTypeVariable> materials = Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->materialTypeVariables();

        bool assigned = false;
        foreach (Module::MaterialTypeVariable variable, materials)
        {
            if (variable.id() == QString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x[(*i).first].size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y[(*i).first].size() : 0);
                if (lenx != leny)
                    if (lenx > leny)
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

                assigned = true;
                if (expressions.count((*i).first) == 0)
                    values[variable.id()] = Value(m_fieldInfo,
                                                  (*i).second,
                                                  (lenx > 0) ? nonlin_x[(*i).first] : vector<double>(),
                                                  (leny > 0) ? nonlin_y[(*i).first] : vector<double>());
                else
                    values[variable.id()] = Value(m_fieldInfo,
                                                  expressions[(*i).first],
                            (lenx > 0) ? nonlin_x[(*i).first] : vector<double>(),
                        (leny > 0) ? nonlin_y[(*i).first] : vector<double>());
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString((*i).first)).toStdString());
    }

    Agros2D::scene()->addMaterial(new SceneMaterial(fieldInfo(), QString(name), values));
}

void PyField::setMaterial(char *name, map<char*, double> parameters,
                          map<char*, char* > expressions,
                          map<char*, vector<double> > nonlin_x,
                          map<char*, vector<double> > nonlin_y)
{
    SceneMaterial *sceneMaterial = Agros2D::scene()->getMaterial(fieldInfo(), QString(name));

    if (sceneMaterial == NULL)
        throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(name).toStdString());

    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
    {
        QList<Module::MaterialTypeVariable> materialVariables = Agros2D::problem()->fieldInfo(m_fieldInfo->fieldId())->materialTypeVariables();

        bool assigned = false;
        foreach (Module::MaterialTypeVariable variable, materialVariables)
        {
            if (variable.id() == QString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x[(*i).first].size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y[(*i).first].size() : 0);
                if (lenx != leny)
                    if (lenx > leny)
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

                assigned = true;
                if (expressions.count((*i).first) == 0)
                    sceneMaterial->setValue(QString((*i).first), Value(m_fieldInfo,
                                                                       (*i).second,
                                                                       (lenx > 0) ? nonlin_x[(*i).first] : vector<double>(),
                                                                       (leny > 0) ? nonlin_y[(*i).first] : vector<double>()));
                else
                    sceneMaterial->setValue(QString((*i).first), Value(m_fieldInfo,
                                                                       expressions[(*i).first],
                                            (lenx > 0) ? nonlin_x[(*i).first] : vector<double>(),
                                            (leny > 0) ? nonlin_y[(*i).first] : vector<double>()));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString((*i).first)).toStdString());
    }
}

void PyField::removeMaterial(char *name)
{
    Agros2D::scene()->removeMaterial(Agros2D::scene()->getMaterial(fieldInfo(), QString(name)));
}

void PyField::localValues(const double x, const double y, int timeStep, int adaptivityStep,
                          const char *solutionType, map<std::string, double> &results)
{
    map<std::string, double> values;

    if (Agros2D::problem()->isSolved())
    {
        // set mode
        if (!silentMode())
        {
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
            currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeNothing->trigger();
        }

        Point point(x, y);

        if (!solutionTypeStringKeys().contains(QString(solutionType)))
            throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

        SolutionMode solutionMode = solutionTypeFromStringKey(QString(solutionType));

        // FIXME: (Franta) solutionMode have to be tested, but not here
        // FIXME: (Franta) finer solution is nor supported yet
        if (solutionMode != SolutionMode_Normal && fieldInfo()->adaptivityType() == AdaptivityType_None)
            throw logic_error(QObject::tr("Reference solution doesn't exist.").toStdString());

        if (timeStep == -1)
            timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo(), solutionMode);
        else if (timeStep < 0 || timeStep > Agros2D::problem()->numTimeLevels() - 1)
            throw out_of_range(QObject::tr("Time step is out of range (0 - %1).").arg(Agros2D::problem()->numTimeLevels()-1).toStdString());

        if (adaptivityStep == -1)
            adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo(), solutionMode, timeStep);
        else if (adaptivityStep < 0 || adaptivityStep > fieldInfo()->adaptivitySteps()-1)
                throw out_of_range(QObject::tr("Adaptivity step is out of range. (0 to %2).").arg(fieldInfo()->adaptivitySteps()-1).toStdString());

        LocalValue *value = fieldInfo()->plugin()->localValue(fieldInfo(), timeStep, adaptivityStep, solutionMode, point);
        QMapIterator<QString, PointValue> it(value->values());
        while (it.hasNext())
        {
            it.next();

            Module::LocalVariable variable = fieldInfo()->localVariable(it.key());

            if (variable.isScalar())
            {
                values[variable.shortname().toStdString()] = it.value().scalar;
            }
            else
            {
                values[variable.shortname().toStdString()] = it.value().vector.magnitude();
                values[variable.shortname().toStdString() + Agros2D::problem()->config()->labelX().toLower().toStdString()] = it.value().vector.x;
                values[variable.shortname().toStdString() + Agros2D::problem()->config()->labelY().toLower().toStdString()] = it.value().vector.y;
            }
        }
        delete value;
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::surfaceIntegrals(vector<int> edges, int timeStep, int adaptivityStep,
                               const char *solutionType, map<std::string, double> &results)
{
    map<std::string, double> values;

    if (Agros2D::problem()->isSolved())
    {
        // set mode
        if (!silentMode())
        {
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
            currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeSurfaceIntegral->trigger();
        }
        Agros2D::scene()->selectNone();

        if (!edges.empty())
        {
            for (vector<int>::iterator it = edges.begin(); it != edges.end(); ++it)
            {
                if ((*it >= 0) && (*it < Agros2D::scene()->edges->length()))
                {
                    Agros2D::scene()->edges->at(*it)->setSelected(true);
                }
                else
                {
                    throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::scene()->edges->length()-1).toStdString());
                    results = values;
                    return;
                }
            }

            if (!silentMode())
                currentPythonEngineAgros()->sceneViewPost2D()->updateGL();
        }
        else
        {
            Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
        }

        if (!solutionTypeStringKeys().contains(QString(solutionType)))
            throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

        SolutionMode solutionMode = solutionTypeFromStringKey(QString(solutionType));

        // FIXME: (Franta) solutionMode have to be tested, but not here
        // FIXME: (Franta) finer solution is nor supported yet
        if (solutionMode != SolutionMode_Normal && fieldInfo()->adaptivityType() == AdaptivityType_None)
            throw logic_error(QObject::tr("Reference solution doesn't exist.").toStdString());

        if (timeStep == -1)
            timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo(), solutionMode);
        else if (timeStep < 0 || timeStep > Agros2D::problem()->numTimeLevels() - 1)
            throw out_of_range(QObject::tr("Time step is out of range (0 - %1).").arg(Agros2D::problem()->numTimeLevels()-1).toStdString());

        if (adaptivityStep == -1)
            adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo(), solutionMode, timeStep);
        else if (adaptivityStep < 0 || adaptivityStep > fieldInfo()->adaptivitySteps()-1)
                throw out_of_range(QObject::tr("Adaptivity step is out of range. (0 to %2).").arg(fieldInfo()->adaptivitySteps()-1).toStdString());

        IntegralValue *integral = fieldInfo()->plugin()->surfaceIntegral(fieldInfo(), timeStep, adaptivityStep, solutionMode);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();

            Module::Integral integral = fieldInfo()->surfaceIntegral(it.key());

            values[integral.shortname().toStdString()] = it.value();
        }
        delete integral;
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::volumeIntegrals(vector<int> labels, int timeStep, int adaptivityStep,
                              const char *solutionType, map<std::string, double> &results)
{
    map<std::string, double> values;

    if (Agros2D::problem()->isSolved())
    {
        // set mode
        if (!silentMode())
        {
            currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
            currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeVolumeIntegral->trigger();
        }
        Agros2D::scene()->selectNone();

        if (!labels.empty())
        {
            for (vector<int>::iterator it = labels.begin(); it != labels.end(); ++it)
            {
                if ((*it >= 0) && (*it < Agros2D::scene()->labels->length()))
                {
                    if (Agros2D::scene()->labels->at(*it)->marker(m_fieldInfo) != Agros2D::scene()->materials->getNone(m_fieldInfo))
                    {
                        Agros2D::scene()->labels->at(*it)->setSelected(true);
                    }
                    else
                    {
                        throw out_of_range(QObject::tr("Label with index '%1' is 'none'.").arg(*it).toStdString());
                    }
                }
                else
                {
                    throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::scene()->labels->length()-1).toStdString());
                    results = values;
                    return;
                }
            }

            if (!silentMode())
                currentPythonEngineAgros()->sceneViewPost2D()->updateGL();
        }
        else
        {
            Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
        }

        if (!solutionTypeStringKeys().contains(QString(solutionType)))
            throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

        SolutionMode solutionMode = solutionTypeFromStringKey(QString(solutionType));

        // FIXME: (Franta) solutionMode have to be tested, but not here
        // FIXME: (Franta) finer solution is nor supported yet
        if (solutionMode != SolutionMode_Normal && fieldInfo()->adaptivityType() == AdaptivityType_None)
            throw logic_error(QObject::tr("Reference solution doesn't exist.").toStdString());

        if (timeStep == -1)
            timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo(), solutionMode);
        else if (timeStep < 0 || timeStep > Agros2D::problem()->numTimeLevels() - 1)
            throw out_of_range(QObject::tr("Time step is out of range (0 - %1).").arg(Agros2D::problem()->numTimeLevels()-1).toStdString());

        if (adaptivityStep == -1)
            adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo(), solutionMode, timeStep);
        else if (adaptivityStep < 0 || adaptivityStep > fieldInfo()->adaptivitySteps()-1)
                throw out_of_range(QObject::tr("Adaptivity step is out of range. (0 to %1).").arg(fieldInfo()->adaptivitySteps()-1).toStdString());

        IntegralValue *integral = fieldInfo()->plugin()->volumeIntegral(fieldInfo(), timeStep, adaptivityStep, solutionMode);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();

            Module::Integral integral = fieldInfo()->volumeIntegral(it.key());

            values[integral.shortname().toStdString()] = it.value();

        }
        delete integral;
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::initialMeshParameters(map<std::string, int> &parameters)
{
    if (!Agros2D::problem()->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());

    parameters["nodes"] = fieldInfo()->initialMesh()->get_num_vertex_nodes();
    parameters["elements"] = fieldInfo()->initialMesh()->get_num_active_elements();

    if (Agros2D::problem()->isSolved())
    {
        int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo(), SolutionMode_Normal);
        MultiArray<double> msa = Agros2D::solutionStore()->multiArray(FieldSolutionID(fieldInfo(), timeStep, 0, SolutionMode_Normal));

        parameters["dofs"] = Hermes::Hermes2D::Space<double>::get_num_dofs(msa.spacesConst());
    }
}

void PyField::solutionMeshParameters(map<std::string, int> &parameters)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (Agros2D::problem()->fieldInfo(fieldInfo()->fieldId())->adaptivityType() != AdaptivityType_None)
    {
        int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo(), SolutionMode_Normal);
        int adaptiveStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo(), SolutionMode_Normal);
        MultiArray<double> msa = Agros2D::solutionStore()->multiArray(FieldSolutionID(fieldInfo(), timeStep, adaptiveStep, SolutionMode_Normal));

        parameters["nodes"] = msa.solutions().at(0)->get_mesh()->get_num_vertex_nodes();
        parameters["elements"] = msa.solutions().at(0)->get_mesh()->get_num_active_elements();
        parameters["dofs"] = Hermes::Hermes2D::Space<double>::get_num_dofs(msa.spacesConst());
    }
    else
        initialMeshParameters(parameters);
}

void PyField::adaptivityInfo(vector<double> &error, vector<int> &dofs)
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (Agros2D::problem()->fieldInfo(fieldInfo()->fieldId())->adaptivityType() != AdaptivityType_None)
    {
        int timeStep = Agros2D::solutionStore()->timeLevels(fieldInfo()).count() - 1;
        int adaptiveSteps = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo(), SolutionMode_Normal) + 1;

        for (int i = 0; i < adaptiveSteps; i++)
        {
            SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(fieldInfo(), timeStep, i, SolutionMode_Normal)); // TODO: (Franta) wrapper
            error.push_back(runTime.adaptivity_error);
            dofs.push_back(runTime.DOFs);
        }
    }
    else
        throw logic_error(QObject::tr("Solution is not adaptive.").toStdString());
}
