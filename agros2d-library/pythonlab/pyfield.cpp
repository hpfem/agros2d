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
#include "sceneview_post2d.h"

PyField::PyField(std::string fieldId)
{
    QMap<QString, QString> modules = Module::availableModules();
    QString id = QString::fromStdString(fieldId);

    if (modules.keys().contains(id))
    {
        if (Agros2D::problem()->hasField(id))
        {
            m_fieldInfo = Agros2D::problem()->fieldInfo(id);
        }
        else
        {
            try{
                m_fieldInfo = new FieldInfo(id);
            }
            catch(AgrosPluginException& e)
            {
                throw invalid_argument(QObject::tr("Invalid field id. Plugin %1 cannot be loaded").arg(id).toStdString());
            }

            Agros2D::problem()->addField(m_fieldInfo);
        }
    }
    else
    {
        throw invalid_argument(QObject::tr("Invalid field id. Valid keys: %1").arg(stringListToString(modules.keys())).toStdString());
    }
}

void PyField::setAnalysisType(const std::string &analysisType)
{
    if (m_fieldInfo->analyses().contains(analysisTypeFromStringKey(QString::fromStdString(analysisType))))
    {
        m_fieldInfo->setAnalysisType(analysisTypeFromStringKey(QString::fromStdString(analysisType)));
    }
    else
    {
        QStringList list;
        foreach (AnalysisType analysis, m_fieldInfo->analyses().keys())
            list.append(analysisTypeToStringKey(analysis));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyField::setNumberOfRefinements(int numberOfRefinements)
{
    if (numberOfRefinements >= 0 && numberOfRefinements <= 5)
        m_fieldInfo->setValue(FieldInfo::SpaceNumberOfRefinements, numberOfRefinements);
    else
        throw out_of_range(QObject::tr("Number of refinements is out of range (0 - 5).").toStdString());
}

void PyField::setPolynomialOrder(int polynomialOrder)
{
    if (polynomialOrder > 0 && polynomialOrder <= 10)
        m_fieldInfo->setValue(FieldInfo::SpacePolynomialOrder, polynomialOrder);
    else
        throw out_of_range(QObject::tr("Polynomial order is out of range (1 - 10).").toStdString());
}

void PyField::setLinearityType(const std::string &linearityType)
{
    if (linearityTypeStringKeys().contains(QString::fromStdString(linearityType)))
        m_fieldInfo->setLinearityType(linearityTypeFromStringKey(QString::fromStdString(linearityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(linearityTypeStringKeys())).toStdString());
}

void PyField::setNonlinearConvergenceMeasurement(const std::string &nonlinearConvergenceMeasurement)
{
    if (nonlinearSolverConvergenceMeasurementStringKeys().contains(QString::fromStdString(nonlinearConvergenceMeasurement)))
        m_fieldInfo->setValue(FieldInfo::NonlinearConvergenceMeasurement, (Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType) nonlinearSolverConvergenceMeasurementFromStringKey(QString::fromStdString(nonlinearConvergenceMeasurement)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(nonlinearSolverConvergenceMeasurementStringKeys())).toStdString());
}

void PyField::setNonlinearTolerance(double nonlinearTolerance)
{
    if (nonlinearTolerance > 0.0)
        m_fieldInfo->setValue(FieldInfo::NonlinearTolerance, nonlinearTolerance);
    else
        throw out_of_range(QObject::tr("Nonlinearity tolerance must be positive.").toStdString());
}

void PyField::setNonlinearSteps(int nonlinearSteps)
{
    if (nonlinearSteps >= 1 && nonlinearSteps <= 100)
        m_fieldInfo->setValue(FieldInfo::NonlinearSteps, nonlinearSteps);
    else
        throw out_of_range(QObject::tr("Nonlinearity steps is out of range (1 - 100).").toStdString());
}

void PyField::setNewtonDampingCoeff(double dampingCoeff)
{
    if (dampingCoeff > 0 && dampingCoeff <= 1)
        m_fieldInfo->setValue(FieldInfo::NewtonDampingCoeff, dampingCoeff);
    else
        throw out_of_range(QObject::tr("Newton damping coefficient is out of range (0 - 1).").toStdString());
}

void PyField::setNewtonDampingType(std::string dampingType)
{
    if (dampingTypeStringKeys().contains(QString::fromStdString(dampingType)))
        m_fieldInfo->setValue(FieldInfo::NewtonDampingType, (DampingType) dampingTypeFromStringKey(QString::fromStdString(dampingType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(dampingTypeStringKeys())).toStdString());
}

void PyField::setNewtonReuseJacobian(bool reuse)
{
    m_fieldInfo->setValue(FieldInfo::NewtonReuseJacobian, reuse);
}

void PyField::setNewtonDampingNumberToIncrease(int dampingNumberToIncrease)
{
    if (dampingNumberToIncrease >= 1 && dampingNumberToIncrease <= 5)
        m_fieldInfo->setValue(FieldInfo::NewtonStepsToIncreaseDF, dampingNumberToIncrease);
    else
        throw out_of_range(QObject::tr("Number of steps needed to increase the damping coefficient is out of range (1 - 5).").toStdString());
}

void PyField::setNewtonSufficientImprovementFactorForJacobianReuse(double sufficientImprovementFactorJacobian)
{
    if (sufficientImprovementFactorJacobian >= 0.00001 && sufficientImprovementFactorJacobian <= 1.0)
        m_fieldInfo->setValue(FieldInfo::NewtonSufImprovForJacobianReuse, sufficientImprovementFactorJacobian);
    else
        throw out_of_range(QObject::tr("Sufficient improvement factor for Jacobian reuse is out of range (0.00001 - 1.0).").toStdString());
}

void PyField::setNewtonSufficientImprovementFactor(double sufficientImprovementFactor)
{
    if (sufficientImprovementFactor >= 0.00001 && sufficientImprovementFactor <= 100.0)
        m_fieldInfo->setValue(FieldInfo::NewtonSufImprov, sufficientImprovementFactor);
    else
        throw out_of_range(QObject::tr("Sufficient improvement factor is out of range (0.00001 - 100).").toStdString());
}

void PyField::setNewtonMaximumStepsWithReusedJacobian(int maximumStepsWithReusedJacobian)
{
    if (maximumStepsWithReusedJacobian >= 1 && maximumStepsWithReusedJacobian <= 100)
        m_fieldInfo->setValue(FieldInfo::NewtonMaxStepsReuseJacobian, maximumStepsWithReusedJacobian);
    else
        throw out_of_range(QObject::tr("Maximum steps with reused Jacobian is out of range (1 - 100).").toStdString());
}

void PyField::setPicardAndersonAcceleration(bool acceleration)
{
    m_fieldInfo->setValue(FieldInfo::PicardAndersonAcceleration, acceleration);
}

void PyField::setPicardAndersonBeta(double beta)
{
    if (beta > 0 && beta <= 1)
        m_fieldInfo->setValue(FieldInfo::PicardAndersonBeta, beta);
    else
        throw out_of_range(QObject::tr("Anderson coefficient is out of range (0 - 1).").toStdString());
}

void PyField::setPicardAndersonNumberOfLastVectors(int number)
{
    if (number >= 1 && number <= 5)
        m_fieldInfo->setValue(FieldInfo::PicardAndersonNumberOfLastVectors, number);
    else
        throw out_of_range(QObject::tr("Number of last vector is out of range (1 - 5).").toStdString());
}

void PyField::setAdaptivityType(const std::string &adaptivityType)
{
    if (adaptivityTypeStringKeys().contains(QString::fromStdString(adaptivityType)))
        m_fieldInfo->setAdaptivityType((AdaptivityType) adaptivityTypeFromStringKey(QString::fromStdString(adaptivityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityTypeStringKeys())).toStdString());
}

void PyField::setMatrixSolver(const std::string &matrixSolver)
{
    if (matrixSolverTypeStringKeys().contains(QString::fromStdString(matrixSolver)))
        m_fieldInfo->setMatrixSolver(matrixSolverTypeFromStringKey(QString::fromStdString(matrixSolver)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(matrixSolverTypeStringKeys())).toStdString());
}

void PyField::setLinearSolverAbsoluteTolerance(double absoluteTolerance)
{
    if (absoluteTolerance > 0.0)
        m_fieldInfo->setValue(FieldInfo::LinearSolverIterToleranceAbsolute, absoluteTolerance);
    else
        throw out_of_range(QObject::tr("Absolute tolerance must be positive.").toStdString());
}

void PyField::setLinearSolverIterations(int numberOfIterations)
{
    if (numberOfIterations >= 1 && numberOfIterations <= 10000)
        m_fieldInfo->setValue(FieldInfo::LinearSolverIterIters, numberOfIterations);
    else
        throw out_of_range(QObject::tr("Linear solver iterations is out of range (1 - 10000).").toStdString());
}


void PyField::setLinearSolverMethod(const std::string &linearSolverMethod)
{
    if (iterLinearSolverMethodStringKeys().contains(QString::fromStdString(linearSolverMethod)))
        m_fieldInfo->setValue(FieldInfo::LinearSolverIterMethod,
                              (Hermes::Solvers::IterativeParalutionLinearMatrixSolver<double>::ParalutionSolverType) iterLinearSolverMethodFromStringKey(QString::fromStdString(linearSolverMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(iterLinearSolverMethodStringKeys())).toStdString());
}

void PyField::setLinearSolverPreconditioner(const std::string &linearSolverPreconditioner)
{
    if (iterLinearSolverPreconditionerTypeStringKeys().contains(QString::fromStdString(linearSolverPreconditioner)))
        m_fieldInfo->setValue(FieldInfo::LinearSolverIterPreconditioner,
                              (Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType) iterLinearSolverPreconditionerTypeFromStringKey(QString::fromStdString(linearSolverPreconditioner)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(iterLinearSolverPreconditionerTypeStringKeys())).toStdString());
}

void PyField::setAdaptivityTolerance(double adaptivityTolerance)
{
    if (adaptivityTolerance > 0.0)
        m_fieldInfo->setValue(FieldInfo::AdaptivityTolerance, adaptivityTolerance);
    else
        throw out_of_range(QObject::tr("Adaptivity tolerance must be positive.").toStdString());
}

void PyField::setAdaptivitySteps(int adaptivitySteps)
{
    if (adaptivitySteps >= 1 && adaptivitySteps <= 100)
        m_fieldInfo->setValue(FieldInfo::AdaptivitySteps, adaptivitySteps);
    else
        throw out_of_range(QObject::tr("Adaptivity steps is out of range (1 - 100).").toStdString());
}

void PyField::setAdaptivityThreshold(double adaptivityThreshold)
{
    if (adaptivityThreshold > 0.0)
        m_fieldInfo->setValue(FieldInfo::AdaptivityThreshold, adaptivityThreshold);
    else
        throw out_of_range(QObject::tr("Adaptivity threshold must be positive.").toStdString());
}

void PyField::setAdaptivityStoppingCriterion(const std::string &adaptivityStoppingCriterion)
{
    if (adaptivityStoppingCriterionTypeStringKeys().contains(QString::fromStdString(adaptivityStoppingCriterion)))
        m_fieldInfo->setValue(FieldInfo::AdaptivityStoppingCriterion, (AdaptivityStoppingCriterionType) adaptivityStoppingCriterionFromStringKey(QString::fromStdString(adaptivityStoppingCriterion)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityStoppingCriterionTypeStringKeys())).toStdString());
}

void PyField::setAdaptivityNormType(const std::string &adaptivityNormType)
{
    if (adaptivityNormTypeStringKeys().contains(QString::fromStdString(adaptivityNormType)))
        m_fieldInfo->setValue(FieldInfo::AdaptivityProjNormType, (Hermes::Hermes2D::NormType) adaptivityNormTypeFromStringKey(QString::fromStdString(adaptivityNormType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityNormTypeStringKeys())).toStdString());
}

void PyField::setAdaptivityAnisotropic(bool adaptivityAnisotropic)
{
    m_fieldInfo->setValue(FieldInfo::AdaptivityUseAniso, adaptivityAnisotropic);
}

void PyField::setAdaptivityFinerReference(bool adaptivityFinerReference)
{
    m_fieldInfo->setValue(FieldInfo::AdaptivityFinerReference, adaptivityFinerReference);
}

void PyField::setAdaptivityBackSteps(int adaptivityBackSteps)
{
    if (adaptivityBackSteps >= 0 && adaptivityBackSteps <= 100)
        m_fieldInfo->setValue(FieldInfo::AdaptivityTransientBackSteps, adaptivityBackSteps);
    else
        throw out_of_range(QObject::tr("Adaptivity back steps is out of range (0 - 100).").toStdString());
}

void PyField::setAdaptivityRedoneEach(int adaptivityRedoneEach)
{
    if (adaptivityRedoneEach >= 1 && adaptivityRedoneEach <= 100)
        m_fieldInfo->setValue(FieldInfo::AdaptivityTransientRedoneEach, adaptivityRedoneEach);
    else
        throw out_of_range(QObject::tr("Adaptivity back steps is out of range (0 - 100).").toStdString());
}

void PyField::setInitialCondition(double initialCondition)
{
    m_fieldInfo->setValue(FieldInfo::TransientInitialCondition, initialCondition);
}

void PyField::setTimeSkip(double timeSkip)
{
    if (timeSkip >= 0)
        m_fieldInfo->setValue(FieldInfo::TransientTimeSkip, timeSkip);
    else
        throw out_of_range(QObject::tr("Time skip must be greater than or equal to zero.").toStdString());
}

void PyField::addBoundary(const std::string &name, const std::string &type,
                          const map<std::string, double> &parameters,
                          const map<std::string, std::string> &expressions)
{
    // check boundaries with same name
    foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(m_fieldInfo->fieldId()).items())
    {
        if (boundary->name() == QString::fromStdString(name))
            throw invalid_argument(QObject::tr("Boundary condition '%1' already exists.").arg(QString::fromStdString(name)).toStdString());
    }

    if (!m_fieldInfo->boundaryTypeContains(QString::fromStdString(type)))
        throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(QString::fromStdString(type)).toStdString());

    Module::BoundaryType boundaryType = m_fieldInfo->boundaryType(QString::fromStdString(type));

    // browse boundary parameters
    QMap<QString, Value> values;
    for (map<std::string, double>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    values[variable.id()] = Value((*i).second);
                else
                    values[variable.id()] = Value(QString::fromStdString(expressions.at((*i).first)));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }

    Agros2D::scene()->addBoundary(new SceneBoundary(m_fieldInfo, QString::fromStdString(name), QString::fromStdString(type), values));
}

void PyField::modifyBoundary(const std::string &name, const std::string &type,
                             const map<std::string, double> &parameters,
                             const map<std::string, std::string> &expressions)
{
    SceneBoundary *sceneBoundary = Agros2D::scene()->getBoundary(m_fieldInfo, QString::fromStdString(name));
    if (sceneBoundary == NULL)
        throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    // browse boundary types
    foreach (Module::BoundaryType boundaryType, sceneBoundary->fieldInfo()->boundaryTypes())
    {
        if (QString::fromStdString(type) == boundaryType.id())
        {
            sceneBoundary->setType(QString::fromStdString(type));
            break;
        }
        else
            throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(QString::fromStdString(type)).toStdString());
    }

    // browse boundary parameters
    Module::BoundaryType boundaryType = m_fieldInfo->boundaryType(sceneBoundary->type());
    for (map<std::string, double>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    sceneBoundary->setValue(QString::fromStdString((*i).first), Value((*i).second));
                else
                    sceneBoundary->setValue(QString::fromStdString((*i).first), Value(QString::fromStdString(expressions.at((*i).first))));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }
}

void PyField::removeBoundary(const std::string &name)
{
    Agros2D::scene()->removeBoundary(Agros2D::scene()->getBoundary(m_fieldInfo, QString::fromStdString(name)));
}

void PyField::addMaterial(const std::string &name, const map<std::string, double> &parameters,
                          const map<std::string, std::string> &expressions,
                          const map<std::string, vector<double> > &nonlin_x,
                          const map<std::string, vector<double> > &nonlin_y,
                          const map<string, map<string, string> > &settings_map)
{
    // check materials with same name
    foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(m_fieldInfo->fieldId()).items())
    {
        if (material->name() == QString::fromStdString(name))
            throw invalid_argument(QObject::tr("Material '%1' already exists.").arg(QString::fromStdString(name)).toStdString());
    }

    // browse material parameters
    QList<Module::MaterialTypeVariable> variables = m_fieldInfo->materialTypeVariables();
    QMap<QString, Value> values;
    for (map<std::string, double>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::MaterialTypeVariable variable, variables)
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x.at((*i).first).size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y.at((*i).first).size() : 0);
                if (lenx != leny)
                    if (lenx > leny)
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

                DataTableType dataTableType = DataTableType_PiecewiseLinear;
                bool splineFirstDerivatives = true;
                bool extrapolateConstant = true;

                if (settings_map.find((*i).first) != settings_map.end())
                {
                    map<string, string> settings = settings_map.at((*i).first);
                    for (map<std::string, string>::const_iterator is = settings.begin(); is != settings.end(); ++is)
                    {
                        if (QString::fromStdString((*is).first) == "interpolation")
                        {
                            dataTableType = dataTableTypeFromStringKey(QString::fromStdString((*is).second));
                            assert(dataTableType != DataTableType_Undefined);
                        }

                        if (QString::fromStdString((*is).first) == "extrapolation")
                        {
                            if (QString::fromStdString((*is).second) == "constant")
                                extrapolateConstant = true;
                            else if (QString::fromStdString((*is).second) == "linear")
                                extrapolateConstant = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'constant' or 'linear'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }

                        if (QString::fromStdString((*is).first) == "derivative_at_endpoints")
                        {
                            if (QString::fromStdString((*is).second) == "first")
                                splineFirstDerivatives = true;
                            else if (QString::fromStdString((*is).second) == "second")
                                splineFirstDerivatives = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'first' or 'second'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }
                    }
                }

                assigned = true;

                try
                {
                    if (expressions.count((*i).first) == 0)
                    {
                        values[variable.id()] = Value((*i).second,
                                                      (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                      (leny > 0) ? nonlin_y.at((*i).first) : vector<double>(),
                                                      dataTableType, splineFirstDerivatives, extrapolateConstant);
                    }
                    else
                    {
                        values[variable.id()] = Value(QString::fromStdString(expressions.at((*i).first)),
                                                      (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                      (leny > 0) ? nonlin_y.at((*i).first) : vector<double>(),
                                                      dataTableType, splineFirstDerivatives, extrapolateConstant);
                    }
                }
                catch (AgrosException e)
                {
                    throw invalid_argument(e.toString().toStdString());
                }

                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }

    Agros2D::scene()->addMaterial(new SceneMaterial(m_fieldInfo, QString::fromStdString(name), values));
}

void PyField::modifyMaterial(const std::string &name, const map<std::string, double> &parameters,
                             const map<std::string, std::string> &expressions,
                             const map<std::string, vector<double> > &nonlin_x,
                             const map<std::string, vector<double> > &nonlin_y,
                             const map<string, map<string, string> > &settings_map)
{
    SceneMaterial *sceneMaterial = Agros2D::scene()->getMaterial(m_fieldInfo, QString::fromStdString(name));

    if (sceneMaterial == NULL)
        throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    for (map<std::string, double>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        QList<Module::MaterialTypeVariable> materialVariables = m_fieldInfo->materialTypeVariables();

        bool assigned = false;
        foreach (Module::MaterialTypeVariable variable, materialVariables)
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x.at((*i).first).size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y.at((*i).first).size() : 0);
                if (lenx != leny)
                    if (lenx > leny)
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

                DataTableType dataTableType = DataTableType_PiecewiseLinear;
                bool splineFirstDerivatives = true;
                bool extrapolateConstant = true;

                if (settings_map.find((*i).first) != settings_map.end())
                {
                    map<string, string> settings = settings_map.at((*i).first);
                    for (map<std::string, string>::const_iterator is = settings.begin(); is != settings.end(); ++is)
                    {
                        if (QString::fromStdString((*is).first) == "interpolation")
                        {
                            dataTableType = dataTableTypeFromStringKey(QString::fromStdString((*is).second));
                            assert(dataTableType != DataTableType_Undefined);
                        }

                        if (QString::fromStdString((*is).first) == "extrapolation")
                        {
                            if (QString::fromStdString((*is).second) == "constant")
                                extrapolateConstant = true;
                            else if (QString::fromStdString((*is).second) == "linear")
                                extrapolateConstant = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'constant' or 'linear'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }

                        if (QString::fromStdString((*is).first) == "derivative_at_endpoints")
                        {
                            if (QString::fromStdString((*is).second) == "first")
                                splineFirstDerivatives = true;
                            else if (QString::fromStdString((*is).second) == "second")
                                splineFirstDerivatives = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'first' or 'second'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }
                    }
                }

                assigned = true;

                try
                {
                    if (expressions.count((*i).first) == 0)
                    {
                        sceneMaterial->setValue(QString::fromStdString((*i).first), Value((*i).second,
                                                                                          (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                                                          (leny > 0) ? nonlin_y.at((*i).first) : vector<double>()));
                    }
                    else
                    {
                        sceneMaterial->setValue(QString::fromStdString((*i).first), Value(QString::fromStdString(expressions.at((*i).first)),
                                                                                          (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                                                          (leny > 0) ? nonlin_y.at((*i).first) : vector<double>()));
                    }
                }
                catch (AgrosException e)
                {
                    throw invalid_argument(e.toString().toStdString());
                }

                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }
}

void PyField::removeMaterial(const std::string &name)
{
    Agros2D::scene()->removeMaterial(Agros2D::scene()->getMaterial(m_fieldInfo, QString::fromStdString(name)));
}

void PyField::localValues(double x, double y, int timeStep, int adaptivityStep,
                          const std::string &solutionType, map<std::string, double> &results) const
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

        SolutionMode solutionMode = getSolutionMode(QString::fromStdString(solutionType));

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep, solutionMode);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep, solutionMode);

        LocalValue *value = m_fieldInfo->plugin()->localValue(m_fieldInfo, timeStep, adaptivityStep, solutionMode, point);
        QMapIterator<QString, PointValue> it(value->values());
        while (it.hasNext())
        {
            it.next();

            Module::LocalVariable variable = m_fieldInfo->localVariable(it.key());

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

void PyField::surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                               const std::string &solutionType, map<std::string, double> &results) const
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
            for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
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

        SolutionMode solutionMode = getSolutionMode(QString::fromStdString(solutionType));

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep, solutionMode);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep, solutionMode);

        IntegralValue *integral = m_fieldInfo->plugin()->surfaceIntegral(m_fieldInfo, timeStep, adaptivityStep, solutionMode);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();

            Module::Integral integral = m_fieldInfo->surfaceIntegral(it.key());

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

void PyField::volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                              const std::string &solutionType, map<std::string, double> &results) const
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
            for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
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

        SolutionMode solutionMode = getSolutionMode(QString::fromStdString(solutionType));

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep, solutionMode);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep, solutionMode);

        IntegralValue *integral = m_fieldInfo->plugin()->volumeIntegral(m_fieldInfo, timeStep, adaptivityStep, solutionMode);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();

            Module::Integral integral = m_fieldInfo->volumeIntegral(it.key());

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

void PyField::initialMeshInfo(map<std::string, int> &info) const
{
    if (!Agros2D::problem()->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());

    info["nodes"] = m_fieldInfo->initialMesh()->get_num_vertex_nodes();
    info["elements"] = m_fieldInfo->initialMesh()->get_num_active_elements();

    if (Agros2D::problem()->isSolved())
    {
        MultiArray<double> msa = Agros2D::solutionStore()->multiArray(FieldSolutionID(m_fieldInfo, 0, 0, SolutionMode_Normal));
        info["dofs"] = Hermes::Hermes2D::Space<double>::get_num_dofs(msa.spaces());
    }
}

void PyField::solutionMeshInfo(int timeStep, int adaptivityStep, const std::string &solutionType, map<std::string, int> &info) const
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    SolutionMode solutionMode = getSolutionMode(QString::fromStdString(solutionType));

    // set time and adaptivity step if -1 (default parameter - last steps), check steps
    timeStep = getTimeStep(timeStep, solutionMode);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep, solutionMode);

    // TODO: (Franta) time and adaptivity step in gui vs. implementation
    MultiArray<double> msa = Agros2D::solutionStore()->multiArray(FieldSolutionID(m_fieldInfo, timeStep, adaptivityStep, solutionMode));

    info["nodes"] = msa.solutions().at(0)->get_mesh()->get_num_vertex_nodes();
    info["elements"] = msa.solutions().at(0)->get_mesh()->get_num_active_elements();
    info["dofs"] = Hermes::Hermes2D::Space<double>::get_num_dofs(msa.spaces());
}

void PyField::solverInfo(int timeStep, int adaptivityStep, const std::string &solutionType, vector<double> &residual, vector<double> &dampingCoeff, int &jacobianCalculations) const
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    SolutionMode solutionMode = getSolutionMode(QString::fromStdString(solutionType));

    // step if -1 (default parameter - last steps)
    timeStep = getTimeStep(timeStep, solutionMode);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep, solutionMode);

    SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(m_fieldInfo, timeStep, adaptivityStep, solutionMode));

    for (int i = 0; i < runTime.newtonResidual().size(); i++)
        residual.push_back(runTime.newtonResidual().at(i));

    for (int i = 0; i < runTime.newtonDamping().size(); i++)
        dampingCoeff.push_back(runTime.newtonDamping().at(i));

    jacobianCalculations = runTime.jacobianCalculations();
}

void PyField::adaptivityInfo(int timeStep, const std::string &solutionType, vector<double> &error, vector<int> &dofs) const
{
    if (!Agros2D::problem()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (m_fieldInfo->adaptivityType() == AdaptivityType_None)
        throw logic_error(QObject::tr("Solution is not adaptive.").toStdString());

    SolutionMode solutionMode = getSolutionMode(QString::fromStdString(solutionType));

    // set time step if -1 (default parameter - last steps)
    timeStep = getTimeStep(timeStep, solutionMode);

    int adaptivitySteps = Agros2D::solutionStore()->lastAdaptiveStep(m_fieldInfo, solutionMode) + 1;
    for (int i = 0; i < adaptivitySteps; i++)
    {
        SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(m_fieldInfo, timeStep, i, solutionMode));
        error.push_back(runTime.adaptivityError());
        dofs.push_back(runTime.DOFs());
    }
}

SolutionMode PyField::getSolutionMode(const QString &solutionType) const
{
    if (!solutionTypeStringKeys().contains(solutionType))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());

    if (solutionType == solutionTypeToStringKey(SolutionMode_Reference) && m_fieldInfo->adaptivityType() == AdaptivityType_None)
        throw logic_error(QObject::tr("Reference solution does not exist.").toStdString());

    return solutionTypeFromStringKey(solutionType);
}

int PyField::getTimeStep(int timeStep, SolutionMode solutionMode) const
{
    if (timeStep == -1)
        timeStep = Agros2D::solutionStore()->lastTimeStep(m_fieldInfo, solutionMode);
    else if (timeStep < 0 || timeStep > Agros2D::problem()->numTimeLevels() - 1)
        throw out_of_range(QObject::tr("Time step is out of range (0 - %1).").arg(Agros2D::problem()->numTimeLevels()-1).toStdString());

    return timeStep;
}

int PyField::getAdaptivityStep(int adaptivityStep, int timeStep, SolutionMode solutionMode) const
{
    if (adaptivityStep == -1)
        adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(m_fieldInfo, solutionMode, timeStep);
    else if (adaptivityStep < 0 || adaptivityStep > m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() - 1)
        throw out_of_range(QObject::tr("Adaptivity step is out of range. (0 to %1).").arg(m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() - 1).toStdString());

    return adaptivityStep;
}
