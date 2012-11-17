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

#include "pythonlab_agros.h"

#include <Python.h>
#include "../resources_source/python/agros2d.cpp"

#include "scene.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "datatable.h"

#include "hermes2d/plugin_interface.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"

#include "util/constants.h"

// current python engine agros
PythonEngineAgros *currentPythonEngineAgros()
{
    return static_cast<PythonEngineAgros *>(currentPythonEngine());
}

QString createPythonFromModel()
{
    QString str;

    // description
    /*
    if (!Util::problem()->config()->description().isEmpty())
    {
        str += "# description\n";
        str += QString("# %1").arg(Util::problem()->config()->description());
        str += "\n\n";
    }
    */

    // startup script
    if (!Util::problem()->config()->startupscript().isEmpty())
    {
        str += "# startup script\n\n";
        str += Util::problem()->config()->startupscript();
        str += "\n\n";
    }

    // model
    str += "import agros2d\n\n";
    str += "# problem\n";
    str += QString("problem = agros2d.problem(clear = True)\n");
    if (!Util::problem()->config()->name().isEmpty())
        str += QString("problem.name = \"%1\"\n").arg(Util::problem()->config()->name());
    str += QString("problem.coordinate_type = \"%1\"\n").arg(coordinateTypeToStringKey(Util::problem()->config()->coordinateType()));
    str += QString("problem.mesh_type = \"%1\"\n").arg(meshTypeToStringKey(Util::problem()->config()->meshType()));
    str += QString("problem.matrix_solver = \"%1\"\n").arg(matrixSolverTypeToStringKey(Util::problem()->config()->matrixSolver()));

    if (Util::problem()->isHarmonic())
        str += QString("problem.frequency = %1\n").
                arg(Util::problem()->config()->frequency());

    if (Util::problem()->isTransient())
        str += QString("problem.time_step_method = \"%1\"\n"
                       "problem.time_method_order = %2\n"
                       "problem.time_method_tolerance = %3\n"
                       "problem.time_total = %4\n"
                       "problem.time_steps = %5\n").
                arg(timeStepMethodToStringKey(Util::problem()->config()->timeStepMethod())).
                arg(Util::problem()->config()->timeOrder()).
                arg(Util::problem()->config()->timeMethodTolerance().toString()).
                arg(Util::problem()->config()->timeTotal().toString()).
                arg(Util::problem()->config()->numConstantTimeSteps());

    // fields
    str += "\n# fields\n";
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        str += QString("# %1\n").arg(fieldInfo->fieldId());

        // str += QString("%1 = agros2d.field(field_id = \"%2\")\n").
        str += QString("%1 = agros2d.field(\"%2\")\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->fieldId());
        str += QString("%1.analysis_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(analysisTypeToStringKey(fieldInfo->analysisType()));

        if (Util::problem()->isTransient())
        {
            if (fieldInfo->analysisType() == analysisTypeFromStringKey("transient"))
                str += QString("%1.initial_condition = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->initialCondition().number());
        }
        else
        {
            str += QString("%1.time_skip = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->timeSkip().number());
        }

        if (fieldInfo->numberOfRefinements() > 0)
            str += QString("%1.number_of_refinements = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->numberOfRefinements());

        if (fieldInfo->polynomialOrder() > 0)
            str += QString("%1.polynomial_order = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->polynomialOrder());

        str += QString("%1.linearity_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(linearityTypeToStringKey(fieldInfo->linearityType()));

        if (fieldInfo->linearityType() != LinearityType_Linear)
        {
            str += QString("%1.nonlinear_tolerance = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->nonlinearTolerance());

            str += QString("%1.nonlinear_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->nonlinearSteps());
        }

        str += QString("%1.adaptivity_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(adaptivityTypeToStringKey(fieldInfo->adaptivityType()));

        if (fieldInfo->adaptivityType() != AdaptivityType_None)
        {
            str += QString("%1.adaptivity_steps= %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->adaptivitySteps());

            str += QString("%1.adaptivity_tolerance= %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->adaptivityTolerance());
        }

        str += "\n";

        str += "\n# boundaries\n";
        foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(fieldInfo).items())
        {
            const QHash<QString, Value> values = boundary->values();

            QString variables = "{";

            Module::BoundaryType *boundaryType = fieldInfo->module()->boundaryType(boundary->type());
            foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables())
            {
                Value value = values[variable->id()];

                if (value.hasExpression())
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\" }, ").
                            arg(variable->id()).
                            arg(value.text());
                }
                else
                {
                    variables += QString("\"%1\" : %2, ").
                            arg(variable->id()).
                            arg(value.toString());
                }
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_boundary(\"%2\", \"%3\", %4)\n").
                    arg(fieldInfo->fieldId()).
                    arg(boundary->name()).
                    arg(boundary->type()).
                    arg(variables);
        }

        str += "\n";

        str += "\n# materials\n";
        foreach (SceneMaterial *material, Util::scene()->materials->filter(fieldInfo).items())
        {
            const QHash<QString, Value> values = material->values();

            QString variables = "{";
            foreach (Module::MaterialTypeVariable *variable, material->fieldInfo()->module()->materialTypeVariables())
            {
                Value value = values[variable->id()];

                if (value.hasTable())
                {
                    if (value.hasExpression())
                        variables += QString("\"%1\" : { \"expression\" : \"%2\", \"x\" : [%3], \"y\" : [%4] }, ").
                                arg(variable->id()).
                                arg(value.text()).
                                arg(QString::fromStdString(value.table()->toStringX())).
                                arg(QString::fromStdString(value.table()->toStringY()));
                    else
                        variables += QString("\"%1\" : { \"value\" : %2, \"x\" : [%3], \"y\" : [%4] }, ").
                                arg(variable->id()).
                                arg(value.number()).
                                arg(QString::fromStdString(value.table()->toStringX())).
                                arg(QString::fromStdString(value.table()->toStringY()));
                }
                else if (value.hasExpression())
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\" }, ").
                            arg(variable->id()).
                            arg(value.text());
                }
                else
                {
                    variables += QString("\"%1\" : %2, ").
                            arg(variable->id()).
                            arg(value.toString());
                }
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_material(\"%2\", %3)\n").
                    arg(fieldInfo->fieldId()).
                    arg(material->name()).
                    arg(variables);
        }

        str += "\n";
    }

    // geometry
    str += "# geometry\n";
    str += "geometry = agros2d.geometry\n";

    // edges
    if (Util::scene()->edges->count() > 0)
    {
        //str += "\n# edges\n";
        foreach (SceneEdge *edge, Util::scene()->edges->items())
        {
            str += QString("geometry.add_edge(%1, %2, %3, %4").
                    arg(edge->nodeStart()->point().x).
                    arg(edge->nodeStart()->point().y).
                    arg(edge->nodeEnd()->point().x).
                    arg(edge->nodeEnd()->point().y);

            if (edge->angle() > 0.0)
                str += ", angle = " + QString::number(edge->angle());

            // refinement
            if (Util::problem()->fieldInfos().count() > 0)
            {
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    if (fieldInfo->edgeRefinement(edge) > 0)
                    {
                        refinements += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->edgeRefinement(edge));
                    }
                }
                refinements = (refinements.endsWith(", ") ? refinements.left(refinements.length() - 2) : refinements) + "}";
                str += refinements;
            }

            // boundaries
            if (Util::problem()->fieldInfos().count() > 0)
            {
                QString boundaries = ", boundaries = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    SceneBoundary *marker = edge->marker(fieldInfo);

                    if (marker != Util::scene()->boundaries->getNone(fieldInfo))
                    {
                        boundaries += QString("\"%1\" : \"%2\", ").
                                arg(fieldInfo->fieldId()).
                                arg(marker->name());
                    }
                }
                boundaries = (boundaries.endsWith(", ") ? boundaries.left(boundaries.length() - 2) : boundaries) + "}";
                str += boundaries;
            }

            str += ")\n";
        }
        str += "\n";
    }

    // labels
    if (Util::scene()->labels->count() > 0)
    {
        //str += "# labels\n";
        foreach (SceneLabel *label, Util::scene()->labels->items())
        {
            str += QString("geometry.add_label(%1, %2").
                    arg(label->point().x).
                    arg(label->point().y);

            if (label->area() > 0.0)
                str += ", area = " + QString::number(label->area());

            // refinements
            if (Util::problem()->fieldInfos().count() > 0)
            {
                int refinementsCount = 0;
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    if (fieldInfo->labelRefinement(label) > 0)
                    {
                        refinements += QString("\"%1\" : \"%2\", ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->labelRefinement(label));

                        refinementsCount++;
                    }
                }
                refinements = (refinements.endsWith(", ") ? refinements.left(refinements.length() - 2) : refinements) + "}";
                if (refinementsCount > 0)
                    str += refinements;
            }

            // orders
            if (Util::problem()->fieldInfos().count() > 0)
            {
                int ordersCount = 0;
                QString orders = ", orders = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    if (fieldInfo->labelPolynomialOrder(label) != fieldInfo->polynomialOrder())
                    {
                        orders += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->labelPolynomialOrder(label));

                        ordersCount++;
                    }
                }
                orders = (orders.endsWith(", ") ? orders.left(orders.length() - 2) : orders) + "}";
                if (ordersCount > 0)
                    str += orders;
            }

            // materials
            if (Util::problem()->fieldInfos().count() > 0)
            {
                QString materials = ", materials = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    SceneMaterial *marker = label->marker(fieldInfo);

                    materials += QString("\"%1\" : \"%2\", ").
                            arg(fieldInfo->fieldId()).
                            arg(marker->name());
                }
                materials = (materials.endsWith(", ") ? materials.left(materials.length() - 2) : materials) + "}";
                str += materials;
            }

            str += ")\n";
        }
    }

    return str;
}

// ************************************************************************************

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
    currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

    Util::problem()->solve();
    if (Util::problem()->isSolved())
    {
        // trigger postprocessor
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
    }
}

PyField::PyField(char *fieldId)
{
    QMap<QString, QString> modules = availableModules();

    if (modules.keys().contains(QString(fieldId)))
        if (Util::problem()->hasField(QString(fieldId)))
        {
            m_fieldInfo = Util::problem()->fieldInfo(fieldId);
        }
        else
        {
            m_fieldInfo = new FieldInfo(fieldId);
            Util::problem()->addField(fieldInfo());
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
    if (availableAnalyses(m_fieldInfo->fieldId()).contains(analysisTypeFromStringKey(QString(analysisType))))
    {
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setAnalysisType(analysisTypeFromStringKey(QString(analysisType)));
    }
    else
    {
        QStringList list;
        QList<AnalysisType> analyses = availableAnalyses(m_fieldInfo->fieldId()).keys();
        foreach (AnalysisType analysis, analyses)
            list.append(analysisTypeToStringKey(analysis));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyField::setNumberOfRefinements(const int numberOfRefinements)
{
    if (numberOfRefinements >= 0 && numberOfRefinements <= 5)
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setNumberOfRefinements(numberOfRefinements);
    else
        throw invalid_argument(QObject::tr("Number of refenements is out of range (0 - 5).").toStdString());
}

void PyField::setPolynomialOrder(const int polynomialOrder)
{
    if (polynomialOrder > 0 && polynomialOrder <= 10)
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setPolynomialOrder(polynomialOrder);
    else
        throw invalid_argument(QObject::tr("Polynomial order is out of range (1 - 10).").toStdString());
}

void PyField::setLinearityType(const char *linearityType)
{
    if (linearityTypeStringKeys().contains(QString(linearityType)))
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setLinearityType(linearityTypeFromStringKey(QString(linearityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(linearityTypeStringKeys())).toStdString());
}

void PyField::setNonlinearTolerance(const double nonlinearTolerance)
{
    if (nonlinearTolerance > 0.0)
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setNonlinearTolerance(nonlinearTolerance);
    else
        throw invalid_argument(QObject::tr("Nonlinearity tolerance must be positive.").toStdString());
}

void PyField::setNonlinearSteps(const int nonlinearSteps)
{
    if (nonlinearSteps >= 1)
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setNonlinearSteps(nonlinearSteps);
    else
        throw invalid_argument(QObject::tr("Nonlinearity steps must be higher than 1.").toStdString());
}

void PyField::setDampingCoeff(const double dampingCoeff)
{
    if ((dampingCoeff <= 1) && (dampingCoeff > 0))
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setNewtonDampingCoeff(dampingCoeff);
    else
        throw invalid_argument(QObject::tr("Damping coefficient must be between 0 and 1 .").toStdString());
}

void PyField::setAutomaticDamping(const bool automaticDamping)
{
    Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setNewtonAutomaticDamping(automaticDamping);
}

void PyField::setDampingNumberToIncrease(const int dampingNumberToIncrease)
{
    if ((dampingNumberToIncrease <= 5) && (dampingNumberToIncrease >= 1))
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setNewtonDampingNumberToIncrease(dampingNumberToIncrease);
    else
        throw invalid_argument(QObject::tr("Number of steps needed to increase the damping coefficient must be between 1 and 5 .").toStdString());
}

void PyField::setAdaptivityType(const char *adaptivityType)
{
    if (adaptivityTypeStringKeys().contains(QString(adaptivityType)))
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivityType(adaptivityTypeFromStringKey(QString(adaptivityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityTypeStringKeys())).toStdString());
}

void PyField::setAdaptivityTolerance(const double adaptivityTolerance)
{
    if (adaptivityTolerance > 0.0)
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivityTolerance(adaptivityTolerance);
    else
        throw invalid_argument(QObject::tr("Adaptivity tolerance must be positive.").toStdString());
}

void PyField::setAdaptivitySteps(const int adaptivitySteps)
{
    if (adaptivitySteps >= 1)
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setAdaptivitySteps(adaptivitySteps);
    else
        throw invalid_argument(QObject::tr("Adaptivity steps must be higher than 1.").toStdString());
}

void PyField::setInitialCondition(const double initialCondition)
{
    Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setInitialCondition(Value(QString::number(initialCondition)));
}

void PyField::setTimeSkip(const double timeSkip)
{
    if (timeSkip >= 0 && timeSkip < Util::problem()->config()->timeTotal().number())
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setTimeSkip(Value(QString::number(timeSkip)));
    else
        throw invalid_argument(QObject::tr("Time skip is out of range (0 - %1).").arg(Util::problem()->config()->timeTotal().number()).toStdString());
}

void PyField::addBoundary(char *name, char *type, map<char*, double> parameters, map<char *, char *> expressions)
{
    // check boundaries with same name
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(Util::problem()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (boundary->name() == name)
            throw invalid_argument(QObject::tr("Boundary '%1' already exists.").arg(QString(name)).toStdString());
    }

    Module::BoundaryType *boundaryType = Util::problem()->fieldInfo(m_fieldInfo->fieldId())->module()->boundaryType(QString(type));
    if (!boundaryType)
        throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(type).toStdString());

    // browse boundary parameters
    QHash<QString, Value> values;
    for (map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables())
        {
            if (variable->id() == QString(QString((*i).first)))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    values[variable->id()] = Value(m_fieldInfo,
                                                   (*i).second,
                                                   vector<double>(),
                                                   vector<double>());
                else
                    values[variable->id()] = Value(m_fieldInfo,
                                                   expressions[(*i).first],
                                                   vector<double>(),
                                                   vector<double>());
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString((*i).first)).toStdString());
    }

    Util::scene()->addBoundary(new SceneBoundary(fieldInfo(), name, type, values));
}

void PyField::setBoundary(char *name, char *type, map<char*, double> parameters, map<char *, char *> expressions)
{
    SceneBoundary *sceneBoundary = Util::scene()->getBoundary(fieldInfo(), QString(name));
    if (sceneBoundary == NULL)
        throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(name).toStdString());

    // browse boundary types
    foreach (Module::BoundaryType *boundaryType, sceneBoundary->fieldInfo()->module()->boundaryTypes())
    {
        if (QString(type) == boundaryType->id())
        {
            sceneBoundary->setType(QString(type));
            break;
        }
        else
            throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(type).toStdString());
    }

    // browse boundary parameters
    Module::BoundaryType *boundaryType = Util::problem()->fieldInfo(m_fieldInfo->fieldId())->module()->boundaryType(sceneBoundary->type());
    for (map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables())
        {
            if (variable->id() == QString(QString((*i).first)))
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
    Util::scene()->removeBoundary(Util::scene()->getBoundary(fieldInfo(), QString(name)));
}

void PyField::addMaterial(char *name, map<char*, double> parameters,
                          map<char*, char* > expressions,
                          map<char*, vector<double> > nonlin_x,
                          map<char*, vector<double> > nonlin_y)
{
    // check materials with same name
    foreach (SceneMaterial *material, Util::scene()->materials->filter(Util::problem()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (material->name() == name)
            throw invalid_argument(QObject::tr("Material '%1' already exists.").arg(QString(name)).toStdString());
    }

    // browse material parameters
    QHash<QString, Value> values;
    for (std::map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        QList<Module::MaterialTypeVariable *> materials = Util::problem()->fieldInfo(m_fieldInfo->fieldId())->module()->materialTypeVariables();

        bool assigned = false;
        foreach (Module::MaterialTypeVariable *variable, materials)
        {
            if (variable->id() == QString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x[(*i).first].size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y[(*i).first].size() : 0);
                if (lenx != leny)
                    if (lenx > leny)
                        throw out_of_range(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw out_of_range(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

                assigned = true;
                if (expressions.count((*i).first) == 0)
                    values[variable->id()] = Value(m_fieldInfo,
                                                   (*i).second,
                                                   (lenx > 0) ? nonlin_x[(*i).first] : vector<double>(),
                                                   (leny > 0) ? nonlin_y[(*i).first] : vector<double>());
                else
                    values[variable->id()] = Value(m_fieldInfo,
                                                   expressions[(*i).first],
                                                   (lenx > 0) ? nonlin_x[(*i).first] : vector<double>(),
                                                   (leny > 0) ? nonlin_y[(*i).first] : vector<double>());
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString((*i).first)).toStdString());
    }

    Util::scene()->addMaterial(new SceneMaterial(fieldInfo(), QString(name), values));
}

void PyField::setMaterial(char *name, map<char*, double> parameters,
                          map<char*, char* > expressions,
                          map<char*, vector<double> > nonlin_x,
                          map<char*, vector<double> > nonlin_y)
{
    SceneMaterial *sceneMaterial = Util::scene()->getMaterial(fieldInfo(), QString(name));

    if (sceneMaterial == NULL)
        throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(name).toStdString());

    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
    {
        QList<Module::MaterialTypeVariable *> materialVariables = Util::problem()->fieldInfo(m_fieldInfo->fieldId())->module()->materialTypeVariables();

        bool assigned = false;
        foreach (Module::MaterialTypeVariable *variable, materialVariables)
        {
            if (variable->id() == QString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x[(*i).first].size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y[(*i).first].size() : 0);
                if (lenx != leny)
                    if (lenx > leny)
                        throw out_of_range(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw out_of_range(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

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
    Util::scene()->removeMaterial(Util::scene()->getMaterial(fieldInfo(), QString(name)));
}

void PyField::localValues(double x, double y, map<std::string, double> &results)
{
    map<std::string, double> values;

    if (Util::problem()->isSolved())
    {
        // set mode
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeLocalPointValue->trigger();

        Point point(x, y);

        LocalValue *value = Util::plugins()[fieldInfo()->fieldId()]->localValue(fieldInfo(), point);
        QMapIterator<Module::LocalVariable *, PointValue> it(value->values());
        while (it.hasNext())
        {
            it.next();

            if (it.key()->isScalar())
            {
                values[it.key()->shortname().toStdString()] = it.value().scalar;
            }
            else
            {
                values[it.key()->shortname().toStdString()] = it.value().vector.magnitude();
                values[it.key()->shortname().toStdString() + Util::problem()->config()->labelX().toLower().toStdString()] = it.value().vector.x;
                values[it.key()->shortname().toStdString() + Util::problem()->config()->labelY().toLower().toStdString()] = it.value().vector.y;
            }
        }
        delete value;
    }
    else
    {
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::surfaceIntegrals(vector<int> edges, map<std::string, double> &results)
{
    map<std::string, double> values;

    if (Util::problem()->isSolved())
    {
        // set mode
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeSurfaceIntegral->trigger();
        Util::scene()->selectNone();

        if (!edges.empty())
        {
            for (vector<int>::iterator it = edges.begin(); it != edges.end(); ++it)
            {
                if ((*it >= 0) && (*it < Util::scene()->edges->length()))
                {
                    Util::scene()->edges->at(*it)->setSelected(true);
                }
                else
                {
                    throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges->length()-1).toStdString());
                    results = values;
                    return;
                }
            }

            currentPythonEngineAgros()->sceneViewPost2D()->updateGL();
        }
        else
        {
            Util::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
        }

        IntegralValue *integral = Util::plugins()[fieldInfo()->fieldId()]->surfaceIntegral(fieldInfo());
        QMapIterator<Module::Integral *, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();

            values[it.key()->shortname().toStdString()] = it.value();
        }
        delete integral;
    }
    else
    {
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::volumeIntegrals(vector<int> labels, map<std::string, double> &results)
{
    map<std::string, double> values;

    if (Util::problem()->isSolved())
    {
        // set mode
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeVolumeIntegral->trigger();
        Util::scene()->selectNone();

        if (!labels.empty())
        {
            for (vector<int>::iterator it = labels.begin(); it != labels.end(); ++it)
            {
                if ((*it >= 0) && (*it < Util::scene()->labels->length()))
                {
                    if (Util::scene()->labels->at(*it)->marker(m_fieldInfo) != Util::scene()->materials->getNone(m_fieldInfo))
                    {
                        Util::scene()->labels->at(*it)->setSelected(true);
                    }
                    else
                    {
                        throw out_of_range(QObject::tr("Label with index '%1' is 'none'.").arg(*it).toStdString());
                    }
                }
                else
                {
                    throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->labels->length()-1).toStdString());
                    results = values;
                    return;
                }
            }

            currentPythonEngineAgros()->sceneViewPost2D()->updateGL();
        }
        else
        {
            Util::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
        }

        IntegralValue *integral = Util::plugins()[fieldInfo()->fieldId()]->volumeIntegral(fieldInfo());
        QMapIterator<Module::Integral *, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();

            values[it.key()->shortname().toStdString()] = it.value();
        }
        delete integral;
    }
    else
    {
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyGeometry::activate()
{
    currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();
}

void PyGeometry::addNode(double x, double y)
{
    Util::scene()->addNode(new SceneNode(Point(x, y)));
}

void PyGeometry::addEdge(double x1, double y1, double x2, double y2, double angle, map<char*, int> refinements, map<char*, char*> boundaries)
{
    // nodes
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

    // angle
    if (angle > 90.0 || angle < 0.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());

    SceneEdge *sceneEdge = new SceneEdge(nodeStart, nodeEnd, angle);

    // boundaries
    for (map<char*, char*>::iterator i = boundaries.begin(); i != boundaries.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
        {
            delete sceneEdge;
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());
        }

        bool assigned = false;
        foreach (SceneBoundary *sceneBoundary, Util::scene()->boundaries->filter(Util::problem()->fieldInfo(QString((*i).first))).items())
        {
            if ((sceneBoundary->fieldId() == QString((*i).first)) && (sceneBoundary->name() == QString((*i).second)))
            {
                assigned = true;
                sceneEdge->addMarker(sceneBoundary);
                break;
            }
        }

        if (!assigned)
        {
            delete sceneEdge;
            throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString((*i).second)).toStdString());
        }
    }

    Util::scene()->addEdge(sceneEdge);

    // refinements
    setMeshRefinementOnEdge(sceneEdge, refinements);
}

void PyGeometry::addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, double angle, map<char *, int> refinements, map<char*, char*> boundaries)
{
    // nodes
    if (angle > 90.0 || angle < 0.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());

    if (Util::scene()->nodes->isEmpty())
        throw out_of_range(QObject::tr("Geometry does not contain nodes.").toStdString());

    if (nodeStartIndex > (Util::scene()->nodes->length() - 1) || nodeStartIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeStartIndex).toStdString());
    if (nodeEndIndex > (Util::scene()->nodes->length() - 1) || nodeEndIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeEndIndex).toStdString());

    SceneNode *nodeStart = Util::scene()->nodes->at(nodeStartIndex);
    SceneNode *nodeEnd = Util::scene()->nodes->at(nodeEndIndex);

    SceneEdge *sceneEdge = new SceneEdge(nodeStart, nodeEnd, angle);

    // boundaries
    for (map<char*, char*>::iterator i = boundaries.begin(); i != boundaries.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
        {
            delete sceneEdge;
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());
        }

        bool assigned = false;
        foreach (SceneBoundary *sceneBoundary, Util::scene()->boundaries->filter(Util::problem()->fieldInfo(QString((*i).first))).items())
        {
            if ((sceneBoundary->fieldId() == QString((*i).first)) && (sceneBoundary->name() == QString((*i).second)))
            {
                assigned = true;
                sceneEdge->addMarker(sceneBoundary);
                break;
            }
        }

        if (!assigned)
        {
            delete sceneEdge;
            throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString((*i).second)).toStdString());
        }
    }

    Util::scene()->addEdge(sceneEdge);

    // refinements
    setMeshRefinementOnEdge(sceneEdge, refinements);
}

void PyGeometry::setMeshRefinementOnEdge(SceneEdge *edge, map<char *, int> refinements)
{
    for (map<char*, int>::iterator i = refinements.begin(); i != refinements.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());

        if (((*i).second < 0) || ((*i).second > 10))
            throw out_of_range(QObject::tr("Number of refinements '%1' is out of range (0 - 10).").arg((*i).second).toStdString());

        Util::problem()->fieldInfo(QString((*i).first))->setEdgeRefinement(edge, (*i).second);
    }
}

void PyGeometry::addLabel(double x, double y, double area, map<char *, int> refinements, map<char *, int> orders, map<char *, char *> materials)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    // TODO: (Franta) if (order < 0 || order > 10) throw out_of_range(QObject::tr("Polynomial order is out of range (0 - 10).").toStdString());

    SceneLabel *sceneLabel = new SceneLabel(Point(x, y), area);

    // materials
    for( map<char*, char*>::iterator i = materials.begin(); i != materials.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
        {
            delete sceneLabel;
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());
        }

        if (QString((*i).second) != "none")
        {

            bool assigned = false;
            foreach (SceneMaterial *sceneMaterial, Util::scene()->materials->filter(Util::problem()->fieldInfo(QString((*i).first))).items())
            {
                if ((sceneMaterial->fieldId() == QString((*i).first)) && (sceneMaterial->name() == QString((*i).second)))
                {
                    assigned = true;
                    sceneLabel->addMarker(sceneMaterial);
                    break;
                }
            }

            if (!assigned)
            {
                delete sceneLabel;
                throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(QString((*i).second)).toStdString());
            }
        }
    }

    Util::scene()->addLabel(sceneLabel);

    // refinements
    for (map<char*, int>::iterator i = refinements.begin(); i != refinements.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());

        if (((*i).second < 0) || ((*i).second > 10))
            throw out_of_range(QObject::tr("Number of refinements '%1' is out of range (0 - 10).").arg((*i).second).toStdString());

        Util::problem()->fieldInfo(QString((*i).first))->setLabelRefinement(sceneLabel, (*i).second);
    }

    // orders
    for (map<char*, int>::iterator i = orders.begin(); i != orders.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());

        if (((*i).second < 1) || ((*i).second > 10))
            throw out_of_range(QObject::tr("Polynomial order '%1' is out of range (1 - 10).").arg((*i).second).toStdString());

        Util::problem()->fieldInfo(QString((*i).first))->setLabelPolynomialOrder(sceneLabel, (*i).second);
    }
}

void PyGeometry::removeNode(int index)
{
    if (index < 0 || index >= Util::scene()->nodes->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());

    Util::scene()->removeNode(Util::scene()->nodes->at(index));
}

void PyGeometry::removeEdge(int index)
{
    if (index < 0 || index >= Util::scene()->edges->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());

    Util::scene()->removeEdge(Util::scene()->edges->at(index));
}

void PyGeometry::removeLabel(int index)
{
    if (index < 0 || index >= Util::scene()->labels->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());

    Util::scene()->removeLabel(Util::scene()->labels->at(index));
}

void PyGeometry::removeNodePoint(double x, double y)
{
    Util::scene()->nodes->remove(Util::scene()->getNode(Point(x, y)));
}

void PyGeometry::removeEdgePoint(double x1, double y1, double x2, double y2, double angle)
{
    Util::scene()->edges->remove(Util::scene()->getEdge(Point(x1, y1), Point(x2, y2), angle));
}

void PyGeometry::removeLabelPoint(double x, double y)
{
    Util::scene()->labels->remove(Util::scene()->getLabel(Point(x, y)));
}

void PyGeometry::selectNodes(vector<int> nodes)
{
    Util::scene()->selectNone();

    if (!nodes.empty())
    {
        for (vector<int>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it >= 0) && (*it < Util::scene()->nodes->length()))
                Util::scene()->nodes->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Util::scene()->nodes->length()-1).toStdString());
        }
    }
    else
    {
        Util::scene()->selectAll(SceneGeometryMode_OperateOnNodes);
    }

    Util::scene()->invalidate();
}

void PyGeometry::selectEdges(vector<int> edges)
{
    Util::scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Util::scene()->edges->length()))
                Util::scene()->edges->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges->length()-1).toStdString());
        }
    }
    else
    {
        Util::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
    }

    Util::scene()->invalidate();
}

void PyGeometry::selectLabels(vector<int> labels)
{
    Util::scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Util::scene()->labels->length()))
                Util::scene()->labels->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->labels->length()-1).toStdString());
        }
    }
    else
    {
        Util::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
    }

    Util::scene()->invalidate();
}

void PyGeometry::selectNodePoint(double x, double y)
{
    SceneNode *node = currentPythonEngineAgros()->sceneViewPreprocessor()->findClosestNode(Point(x, y));
    if (node)
    {
        node->setSelected(true);
        // sceneView()->doInvalidated();
    }
}

void PyGeometry::selectEdgePoint(double x, double y)
{
    SceneEdge *edge = currentPythonEngineAgros()->sceneViewPreprocessor()->findClosestEdge(Point(x, y));
    if (edge)
    {
        edge->setSelected(true);
        // sceneView()->doInvalidated();
    }
}

void PyGeometry::selectLabelPoint(double x, double y)
{
    SceneLabel *label = currentPythonEngineAgros()->sceneViewPreprocessor()->findClosestLabel(Point(x, y));
    if (label)
    {
        label->setSelected(true);
        // sceneView()->doInvalidated();
    }
}

void PyGeometry::selectNone()
{
    Util::scene()->selectNone();
    Util::scene()->invalidate();
}

void PyGeometry::moveSelection(double dx, double dy, bool copy)
{
    Util::scene()->transformTranslate(Point(dx, dy), copy);
    currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::rotateSelection(double x, double y, double angle, bool copy)
{
    Util::scene()->transformRotate(Point(x, y), angle, copy);
    currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::scaleSelection(double x, double y, double scale, bool copy)
{
    Util::scene()->transformScale(Point(x, y), scale, copy);
    currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::removeSelection()
{
    Util::scene()->deleteSelected();
    currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::mesh()
{
    // trigger preprocessor
    currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

    Util::problem()->mesh();
    if (Util::problem()->isMeshed())
    {
        // trigger mesh
        currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
    }
}

char *PyGeometry::meshFileName()
{
    if (Util::problem()->isMeshed())
        return const_cast<char*>(QString(tempProblemFileName() + ".mesh").toStdString().c_str());
    else
        throw invalid_argument(QObject::tr("Problem is not meshed.").toStdString());
}

void PyGeometry::zoomBestFit()
{
    currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomBestFit();
}

void PyGeometry::zoomIn()
{
    currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomIn();
}

void PyGeometry::zoomOut()
{
    currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomOut();
}

void PyGeometry::zoomRegion(double x1, double y1, double x2, double y2)
{
    currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

// ****************************************************************************************************

void PyViewConfig::setField(char* fieldid)
{
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        if (fieldInfo->fieldId() == QString(fieldid))
        {
            Util::scene()->setActiveViewField(fieldInfo);
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Util::problem()->fieldInfos().keys())).toStdString());
}

void PyViewConfig::setActiveTimeStep(int timeStep)
{
    if (timeStep < 0 || timeStep >= Util::problem()->numTimeLevels())
        throw invalid_argument(QObject::tr("Time step must be in the range from 0 to %1.").arg(Util::problem()->numTimeLevels()).toStdString());

    Util::scene()->setActiveTimeStep(timeStep);
}

void PyViewConfig::setActiveAdaptivityStep(int adaptivityStep)
{
    if (adaptivityStep < 0 || adaptivityStep > Util::scene()->activeViewField()->adaptivitySteps())
        throw invalid_argument(QObject::tr("Adaptivity step for active field (%1) must be in the range from 0 to %2.").arg(Util::scene()->activeViewField()->fieldId()).arg(Util::scene()->activeViewField()->adaptivitySteps()).toStdString());

    Util::scene()->setActiveAdaptivityStep(adaptivityStep);
}

void PyViewConfig::setActiveSolutionType(char* solutionType)
{
    if (solutionTypeStringKeys().contains(QString(solutionType)))
        Util::scene()->setActiveSolutionType(solutionTypeFromStringKey(QString(solutionType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solutionTypeStringKeys())).toStdString());
}

void PyViewConfig::setGridShow(bool show)
{
    Util::config()->showGrid = show;
}

void PyViewConfig::setGridStep(double step)
{
    Util::config()->gridStep = step;
}

void PyViewConfig::setAxesShow(bool show)
{
    Util::config()->showAxes = show;
}

void PyViewConfig::setRulersShow(bool show)
{
    Util::config()->showRulers = show;
}

// ****************************************************************************************************

void PyViewMesh::activate()
{
    if (Util::problem()->isMeshed())
        currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
}

void PyViewMesh::setInitialMeshViewShow(bool show)
{
    Util::config()->showInitialMeshView = show;
}

void PyViewMesh::setSolutionMeshViewShow(bool show)
{
    Util::config()->showSolutionMeshView = show;
}

void PyViewMesh::setOrderViewShow(bool show)
{
    Util::config()->showOrderView = show;
}

void PyViewMesh::setOrderViewColorBar(bool show)
{
    Util::config()->showOrderColorBar = show;
}

void PyViewMesh::setOrderViewLabel(bool show)
{
    Util::config()->orderLabel = show;
}

void PyViewMesh::setOrderViewPalette(char* palette)
{
    if (paletteOrderTypeStringKeys().contains(QString(palette)))
        Util::config()->orderPaletteOrderType = paletteOrderTypeFromStringKey(QString(palette));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteOrderTypeStringKeys())).toStdString());
}

// ****************************************************************************************************

void PyViewPost2D::activate()
{
    if (Util::problem()->isSolved())
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
}

void PyViewPost2D::setScalarViewShow(bool show)
{
    Util::config()->showScalarView = show;
}

void PyViewPost2D::setScalarViewVariable(char* var)
{
    QStringList list;

    // scalar variables
    foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewScalarVariables())
    {
        list.append(variable->id());
        if (variable->id() == QString(var))
        {
            Util::config()->scalarVariable = QString(var);
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setScalarViewVariableComp(char* component)
{
    if (physicFieldVariableCompTypeStringKeys().contains(QString(component)))
        Util::config()->scalarVariableComp = physicFieldVariableCompFromStringKey(QString(component));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());
}

void PyViewPost2D::setScalarViewPalette(char* palette)
{
    if (paletteTypeStringKeys().contains(QString(palette)))
        Util::config()->paletteType = paletteTypeFromStringKey(QString(palette));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());
}

void PyViewPost2D::setScalarViewPaletteQuality(char* quality)
{
    if (paletteQualityStringKeys().contains(QString(quality)))
        Util::config()->linearizerQuality = paletteQualityFromStringKey(QString(quality));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteQualityStringKeys())).toStdString());
}

void PyViewPost2D::setScalarViewPaletteSteps(int steps)
{
    if (steps >= PALETTESTEPSMIN && steps <= PALETTESTEPSMAX)
        Util::config()->paletteSteps = steps;
    else
        throw invalid_argument(QObject::tr("Palette steps must be in the range from %1 to %2.").arg(PALETTESTEPSMIN).arg(PALETTESTEPSMAX).toStdString());
}

void PyViewPost2D::setScalarViewPaletteFilter(bool filter)
{
    Util::config()->paletteFilter = filter;
}

void PyViewPost2D::setScalarViewRangeLog(bool log)
{
    Util::config()->scalarRangeLog = log;
}

void PyViewPost2D::setScalarViewRangeBase(double base)
{
    Util::config()->scalarRangeBase = base;
}

void PyViewPost2D::setScalarViewColorBar(bool show)
{
    Util::config()->showScalarColorBar = show;
}

void PyViewPost2D::setScalarViewDecimalPlace(int place)
{
    if (place >= SCALARDECIMALPLACEMIN && place <= SCALARDECIMALPLACEMAX)
        Util::config()->scalarDecimalPlace = place;
    else
        throw invalid_argument(QObject::tr("Decimal place must be in the range from %1 to %2.").arg(SCALARDECIMALPLACEMIN).arg(SCALARDECIMALPLACEMAX).toStdString());
}

void PyViewPost2D::setScalarViewRangeAuto(bool autoRange)
{
    Util::config()->scalarRangeAuto = autoRange;
}

void PyViewPost2D::setScalarViewRangeMin(double min)
{
    Util::config()->scalarRangeMin = min;
}

void PyViewPost2D::setScalarViewRangeMax(double max)
{
    Util::config()->scalarRangeMax = max;
}

void PyViewPost2D::setContourShow(bool show)
{
    Util::config()->showContourView = show;
}

void PyViewPost2D::setContourCount(int count)
{
    if (count > CONTOURSCOUNTMIN && count <= CONTOURSCOUNTMAX)
        Util::config()->contoursCount = count;
    else
        throw invalid_argument(QObject::tr("Contour count must be in the range from %1 to %2.").arg(CONTOURSCOUNTMIN).arg(CONTOURSCOUNTMAX).toStdString());
}

void PyViewPost2D::setContourVariable(char* var)
{
    QStringList list;
    foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewScalarVariables())
    {
        if (variable->isScalar())
        {
            list.append(variable->id());

            if (variable->id() == QString(var))
            {
                Util::config()->contourVariable = QString(var);
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorShow(bool show)
{
    Util::config()->showVectorView = show;
}

void PyViewPost2D::setVectorCount(int count)
{
    if (count > VECTORSCOUNTMIN && count <= VECTORSCOUNTMAX)
        Util::config()->vectorCount = count;
    else
        throw invalid_argument(QObject::tr("Vector count must be in the range from %1 to %2.").arg(VECTORSCOUNTMIN).arg(VECTORSCOUNTMAX).toStdString());
}

void PyViewPost2D::setVectorScale(double scale)
{
    if (scale > VECTORSSCALEMIN && scale <= VECTORSSCALEMAX)
        Util::config()->vectorScale = scale;
    else
        throw invalid_argument(QObject::tr("Vector scale must be in the range from %1 to %2.").arg(VECTORSSCALEMIN).arg(VECTORSSCALEMAX).toStdString());
}

void PyViewPost2D::setVectorVariable(char* var)
{
    QStringList list;
    foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewVectorVariables())
    {
        list.append(variable->id());
        if (variable->id() == QString(var))
        {
            Util::config()->vectorVariable = QString(var);
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorProportional(bool show)
{
    Util::config()->vectorProportional = show;
}

void PyViewPost2D::setVectorColor(bool show)
{
    Util::config()->vectorColor = show;
}

// ****************************************************************************************************

void PyViewPost3D::activate()
{
    if (Util::problem()->isSolved())
        currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->trigger();
}

void PyViewPost3D::setPost3DMode(char* mode)
{
    if (sceneViewPost3DModeStringKeys().contains(QString(mode)))
        Util::config()->showPost3D = sceneViewPost3DModeFromStringKey(QString(mode));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(sceneViewPost3DModeStringKeys())).toStdString());
}

// TODO: (Franta) duplicated code
void PyViewPost3D::setScalarViewVariable(char* var)
{
    QStringList list;

    // scalar variables
    foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewScalarVariables())
    {
        list.append(variable->id());
        if (variable->id() == QString(var))
        {
            Util::config()->scalarVariable = QString(var);
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost3D::setScalarViewVariableComp(char* component)
{
    if (physicFieldVariableCompTypeStringKeys().contains(QString(component)))
        Util::config()->scalarVariableComp = physicFieldVariableCompFromStringKey(QString(component));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());
}

void PyViewPost3D::setScalarViewPalette(char* palette)
{
    if (paletteTypeStringKeys().contains(QString(palette)))
        Util::config()->paletteType = paletteTypeFromStringKey(QString(palette));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());
}

void PyViewPost3D::setScalarViewPaletteQuality(char* quality)
{
    if (paletteQualityStringKeys().contains(QString(quality)))
        Util::config()->linearizerQuality = paletteQualityFromStringKey(QString(quality));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteQualityStringKeys())).toStdString());
}

void PyViewPost3D::setScalarViewPaletteSteps(int steps)
{
    if (steps >= PALETTESTEPSMIN && steps <= PALETTESTEPSMAX)
        Util::config()->paletteSteps = steps;
    else
        throw invalid_argument(QObject::tr("Palette steps must be in the range from %1 to %2.").arg(PALETTESTEPSMIN).arg(PALETTESTEPSMAX).toStdString());
}

void PyViewPost3D::setScalarViewPaletteFilter(bool filter)
{
    Util::config()->paletteFilter = filter;
}

void PyViewPost3D::setScalarViewRangeLog(bool log)
{
    Util::config()->scalarRangeLog = log;
}

void PyViewPost3D::setScalarViewRangeBase(double base)
{
    Util::config()->scalarRangeBase = base;
}

void PyViewPost3D::setScalarViewColorBar(bool show)
{
    Util::config()->showScalarColorBar = show;
}

void PyViewPost3D::setScalarViewDecimalPlace(int place)
{
    if (place >= SCALARDECIMALPLACEMIN && place <= SCALARDECIMALPLACEMAX)
        Util::config()->scalarDecimalPlace = place;
    else
        throw invalid_argument(QObject::tr("Decimal place must be in the range from %1 to %2.").arg(SCALARDECIMALPLACEMIN).arg(SCALARDECIMALPLACEMAX).toStdString());
}

void PyViewPost3D::setScalarViewRangeAuto(bool autoRange)
{
    Util::config()->scalarRangeAuto = autoRange;
}

void PyViewPost3D::setScalarViewRangeMin(double min)
{
    Util::config()->scalarRangeMin = min;
}

void PyViewPost3D::setScalarViewRangeMax(double max)
{
    Util::config()->scalarRangeMax = max;
}

// ****************************************************************************************************

// particle tracing
void PyParticleTracing::solve()
{
    if (!Util::problem()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    // store values
    // double particleStartingRadius = Util::config()->particleStartingRadius;
    // int particleNumberOfParticles = Util::config()->particleNumberOfParticles;
    Util::config()->particleStartingRadius = 0.0;
    Util::config()->particleNumberOfParticles = 1;

    // clear
    m_positions.clear();
    m_velocities.clear();

    Util::scene()->computeParticleTracingPath(&m_positions, &m_velocities, false);
    currentPythonEngineAgros()->postHermes()->refresh();

    // restore values
    // Util::config()->particleStartingRadius = particleStartingRadius;
    // Util::config()->particleNumberOfParticles = particleNumberOfParticles;
}

void PyParticleTracing::positions(std::vector<double> &x,
                                  std::vector<double> &y,
                                  std::vector<double> &z)
{
    std::vector<double> outX;
    std::vector<double> outY;
    std::vector<double> outZ;
    for (int i = 0; i < length(); i++)
    {
        outX.push_back(m_positions[i].x);
        outY.push_back(m_positions[i].y);
        outZ.push_back(m_positions[i].z);
    }

    x = outX;
    y = outY;
    z = outZ;
}

void PyParticleTracing::velocities(std::vector<double> &x,
                                   std::vector<double> &y,
                                   std::vector<double> &z)
{
    std::vector<double> outX;
    std::vector<double> outY;
    std::vector<double> outZ;
    for (int i = 0; i < length(); i++)
    {
        outX.push_back(m_velocities[i].x);
        outY.push_back(m_velocities[i].y);
        outZ.push_back(m_velocities[i].z);
    }

    x = outX;
    y = outY;
    z = outZ;
}

void PyParticleTracing::setInitialPosition(double x, double y)
{
    RectPoint rect = Util::scene()->boundingBox();

    if (x < rect.start.x || x > rect.end.x)
        throw out_of_range(QObject::tr("x coordinate is out of range.").toStdString());
    if (y < rect.start.y || y > rect.end.y)
        throw out_of_range(QObject::tr("y coordinate is out of range.").toStdString());

    Util::config()->particleStart = Point(x, y);
    Util::scene()->invalidate();
}

void PyParticleTracing::setInitialVelocity(double x, double y)
{
    Util::config()->particleStartVelocity = Point(x, y);
    Util::scene()->invalidate();
}

void PyParticleTracing::setParticleMass(double mass)
{
    if (mass <= 0.0)
        throw out_of_range(QObject::tr("Mass must be positive.").toStdString());

    Util::config()->particleMass = mass;
    Util::scene()->invalidate();
}

void PyParticleTracing::setParticleCharge(double charge)
{
    Util::config()->particleConstant = charge;
    Util::scene()->invalidate();
}

void PyParticleTracing::setDragForceDensity(double rho)
{
    if (rho < 0.0)
        throw out_of_range(QObject::tr("Density cannot be negative.").toStdString());

    Util::config()->particleDragDensity = rho;
    Util::scene()->invalidate();
}

void PyParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area cannot be negative.").toStdString());

    Util::config()->particleDragReferenceArea = area;
    Util::scene()->invalidate();
}

void PyParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient cannot be negative.").toStdString());

    Util::config()->particleDragCoefficient = coeff;
    Util::scene()->invalidate();
}

void PyParticleTracing::setIncludeGravitation(int include)
{
    Util::config()->particleIncludeGravitation = include;
    Util::scene()->invalidate();
}

void PyParticleTracing::setReflectOnDifferentMaterial(int reflect)
{
    Util::config()->particleReflectOnDifferentMaterial = reflect;
    Util::scene()->invalidate();
}

void PyParticleTracing::setReflectOnBoundary(int reflect)
{
    Util::config()->particleReflectOnBoundary = reflect;
    Util::scene()->invalidate();
}

void PyParticleTracing::setCoefficientOfRestitution(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient of restitution must be between 0 (collide inelastically) and 1 (collide elastically).").toStdString());

    Util::config()->particleCoefficientOfRestitution = coeff;
    Util::scene()->invalidate();
}

void PyParticleTracing::setMaximumTolerance(double tolerance)
{
    if (tolerance < 0.0)
        throw out_of_range(QObject::tr("Tolerance cannot be negative.").toStdString());

    Util::config()->particleMaximumRelativeError = tolerance;
    Util::scene()->invalidate();
}

void PyParticleTracing::setMaximumNumberOfSteps(int steps)
{
    if (steps < 0.0)
        throw out_of_range(QObject::tr("Maximum number of steps cannot be negative.").toStdString());

    Util::config()->particleMaximumNumberOfSteps = steps;
    Util::scene()->invalidate();
}

void PyParticleTracing::setMinimumStep(int step)
{
    if (step < 0.0)
        throw out_of_range(QObject::tr("Minimum step cannot be negative.").toStdString());

    Util::config()->particleMinimumStep = step;
    Util::scene()->invalidate();
}

// **************************************************************************************************

void pyOpenDocument(char *str)
{
    ErrorResult result = Util::scene()->readFromFile(QString(str));
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

void pySaveDocument(char *str)
{
    ErrorResult result = Util::scene()->writeToFile(QString(str));
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

void pyCloseDocument()
{
    Util::scene()->clear();
    // sceneView()->doDefaultValues();
    Util::scene()->invalidate();

    currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

    currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomBestFit();
    currentPythonEngineAgros()->sceneViewMesh()->doZoomBestFit();
    currentPythonEngineAgros()->sceneViewPost2D()->doZoomBestFit();
    currentPythonEngineAgros()->sceneViewPost3D()->doZoomBestFit();
}

void pySaveImage(char *str, int w, int h)
{
    // ErrorResult result = sceneView()->saveImageToFile(QString(str), w, h);
    // if (result.isError())
    //    throw invalid_argument(result.message().toStdString());
}

// ************************************************************************************

// solutionfilename()
char *pythonSolutionFileName()
{
    if (Util::problem()->isSolved())
    {
        char *fileName = const_cast<char*>(QString(tempProblemFileName() + ".sln").toStdString().c_str());
        //Util::scene()->sceneSolution()->sln()->save(fileName);
        return fileName;
    }
    else
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());
}

// *******************************************************************************************

void PythonEngineAgros::addCustomExtensions()
{
    PythonEngine::addCustomExtensions();

    // init agros cython extensions
    initagros2d();
}

void PythonEngineAgros::addCustomFunctions()
{
    addFunctions(readFileContent(datadir() + "/functions_agros2d.py"));
}

void PythonEngineAgros::runPythonHeader()
{
    QString script;

    // global script
    if (!Util::config()->globalScript.isEmpty())
        script += Util::config()->globalScript + "\n";

    // startup script
    if (!Util::problem()->config()->startupscript().isEmpty())
        script += Util::problem()->config()->startupscript() + "\n";

    // run script
    if (!script.isEmpty())
        PyRun_String(script.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

PythonLabAgros::PythonLabAgros(PythonEngine *pythonEngine, QStringList args, QWidget *parent)
    : PythonEditorDialog(pythonEngine, args, parent)
{
    // add create from model
    actCreateFromModel = new QAction(icon("script-create"), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    tlbTools->addSeparator();
    tlbTools->addAction(actCreateFromModel);
}

void PythonLabAgros::doCreatePythonFromModel()
{
    txtEditor->setPlainText(createPythonFromModel());
}
