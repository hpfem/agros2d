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

#include "pythonlabagros.h"

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

#include "hermes2d/surfaceintegral.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/localpoint.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

// current python engine agros
PythonEngineAgros *currentPythonEngineAgros()
{
    return static_cast<PythonEngineAgros *>(currentPythonEngine());
}

ScriptResult runPythonScript(const QString &script, const QString &fileName)
{
    return currentPythonEngine()->runPythonScript(script, fileName);
}

ExpressionResult runPythonExpression(const QString &expression, bool returnValue)
{
    return currentPythonEngine()->runPythonExpression(expression, returnValue);
}

bool scriptIsRunning()
{
    if (currentPythonEngine())
        return currentPythonEngine()->isRunning();
    else
        return false;
}

QString createPythonFromModel()
{
    QString str;

    // startup script
    if (!Util::problem()->config()->startupscript().isEmpty())
    {
        str += "# startup script\n";
        str += Util::problem()->config()->startupscript();
        str += "\n\n";
    }

    // model
    str += "import agros2d\n\n";
    str += "# model\n";
    str += QString("problem = agros2d.problem(clear = True)\n");
    str += QString("problem.coordinate_type = \"%1\"\n").arg(coordinateTypeToStringKey(Util::problem()->config()->coordinateType()));
    str += QString("problem.name = \"%1\"\n").arg(Util::problem()->config()->name());
    str += QString("problem.mesh_type = \"%1\"\n").arg(meshTypeToStringKey(Util::problem()->config()->meshType()));
    str += QString("problem.matrix_solver = \"%1\"\n").arg(matrixSolverTypeToStringKey(Util::problem()->config()->matrixSolver()));

    if (Util::problem()->config()->frequency() > 0.0)
        str += QString("problem.frequency = %1\n").
                arg(Util::problem()->config()->frequency());

    if (Util::problem()->config()->timeTotal().number() > 0 && Util::problem()->config()->timeStep().number() > 0)
        str += QString("problem.time_step = %1\n"
                       "problem.time_total = %2\n").
                arg(Util::problem()->config()->timeStep().text()).
                arg(Util::problem()->config()->timeTotal().text());

    /*
    if (Util::problem()->config()->adaptivityType != AdaptivityType_None)
        str += QString(",\n"
                       "            adaptivitytype=\"%1\", adaptivitysteps=%2, adaptivitytolerance=%3").
                arg(adaptivityTypeToStringKey(Util::problem()->config()->adaptivityType)).
                arg(Util::problem()->config()->adaptivitySteps).
                arg(Util::problem()->config()->adaptivityTolerance);

    */

    // fields
    str += "\n# fields\n";
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        // str += QString("%1 = agros2d.field(field_id = \"%2\")\n").
        str += QString("%1 = agros2d.field(\"%2\")\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->fieldId());
        str += QString("%1.analysis_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(analysisTypeToStringKey(fieldInfo->analysisType()));

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

        str += "\n";
        foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(fieldInfo).items())
        {
            QString variables = "{";
            const std::map<std::string, Value> values = boundary->getValues();
            for (std::map<std::string, Value>::const_iterator it = values.begin(); it != values.end(); ++it)
            {
                variables += QString("\"%1\" : %2, ").
                        arg(QString::fromStdString(it->first)).
                        arg(it->second.toString());
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_boundary(\"%2\", \"%3\", %4)\n").
                    arg(fieldInfo->fieldId()).
                    arg(QString::fromStdString(boundary->getName())).
                    arg(QString::fromStdString(boundary->getType())).
                    arg(variables);
        }

        str += "\n";
        foreach (SceneMaterial *material, Util::scene()->materials->filter(fieldInfo).items())
        {
            QString variables = "{";
            const std::map<std::string, Value> values = material->getValues();
            for (std::map<std::string, Value>::const_iterator it = values.begin(); it != values.end(); ++it)
            {
                variables += QString("\"%1\" : %2, ").
                        arg(QString::fromStdString(it->first)).
                        arg(it->second.toString());
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_material(\"%2\", %3)\n").
                    arg(fieldInfo->fieldId()).
                    arg(QString::fromStdString(material->getName())).
                    arg(variables);
        }

        str += "\n";
    }

    // geometry
    str += "\n# geometry\n";
    str += "geometry = agros2d.geometry()\n\n";

    // edges
    if (Util::scene()->edges->count() > 0)
    {
        str += "# edges\n";
        foreach (SceneEdge *edge, Util::scene()->edges->items())
        {
            str += QString("geometry.add_edge(%1, %2, %3, %4").
                    arg(edge->nodeStart->point.x).
                    arg(edge->nodeStart->point.y).
                    arg(edge->nodeEnd->point.x).
                    arg(edge->nodeEnd->point.y);

            if (Util::problem()->fieldInfos().count() > 0)
            {
                QString boundaries = ", boundaries = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    SceneBoundary *marker = edge->getMarker(fieldInfo);

                    if (marker != Util::scene()->boundaries->getNone(fieldInfo))
                    {
                        boundaries += QString("\"%1\" : \"%2\", ").
                                arg(fieldInfo->fieldId()).
                                arg(QString::fromStdString(marker->getName()));
                    }
                }
                boundaries = (boundaries.endsWith(", ") ? boundaries.left(boundaries.length() - 2) : boundaries) + "}";
                str += boundaries;
            }

            if (edge->angle > 0.0)
                str += ", angle = " + QString::number(edge->angle);

            if (edge->refineTowardsEdge > 0)
                str += ", refinement = " + QString::number(edge->refineTowardsEdge);

            str += ")\n";
        }
        str += "\n";
    }

    // labels
    if (Util::scene()->labels->count() > 0)
    {
        str += "# labels\n";
        foreach (SceneLabel *label, Util::scene()->labels->items())
        {
            str += QString("geometry.add_label(%1, %2").
                    arg(label->point.x).
                    arg(label->point.y);

            if (Util::problem()->fieldInfos().count() > 0)
            {
                QString materials = ", materials = {";
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    SceneMaterial *marker = label->getMarker(fieldInfo);

                    materials += QString("\"%1\" : \"%2\", ").
                            arg(fieldInfo->fieldId()).
                            arg(QString::fromStdString(marker->getName()));
                }
                materials = (materials.endsWith(", ") ? materials.left(materials.length() - 2) : materials) + "}";
                str += materials;
            }

            if (label->area > 0.0)
                str += ", area = " + QString::number(label->area);

            if (label->polynomialOrder > 0)
                str += ", order = " + QString::number(label->polynomialOrder);

            str += ")\n";
        }
        str += "\n";
    }

    return str;
}

ScriptEngineRemote::ScriptEngineRemote()
{
    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer("agros2d-server");
    if (!m_server->listen("agros2d-server"))
    {
        qWarning() << tr("Error: Unable to start the server (agros2d-server): %1.").arg(m_server->errorString());
        return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(connected()));
}

ScriptEngineRemote::~ScriptEngineRemote()
{
    delete m_server;
    delete m_client_socket;
}

void ScriptEngineRemote::connected()
{
    command = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void ScriptEngineRemote::readCommand()
{
    QTextStream in(m_server_socket);
    command = in.readAll();
}

void ScriptEngineRemote::disconnected()
{
    m_server_socket->deleteLater();

    ScriptResult result;
    if (!command.isEmpty())
    {
        result = runPythonScript(command);
    }

    m_client_socket = new QLocalSocket();
    connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

    m_client_socket->connectToServer("agros2d-client");
    if (m_client_socket->waitForConnected(1000))
    {
        QTextStream out(m_client_socket);
        out << result.text;
        out.flush();
        m_client_socket->waitForBytesWritten();
    }
    else
    {
        displayError(QLocalSocket::ConnectionRefusedError);
    }

    delete m_client_socket;
}

void ScriptEngineRemote::displayError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        qWarning() << tr("Server error: The host was not found.");
        break;
    case QLocalSocket::ConnectionRefusedError:
        qWarning() << tr("Server error: The connection was refused by the peer. Make sure the agros2d-client server is running.");
        break;
    default:
        qWarning() << tr("Server error: The following error occurred: %1.").arg(m_client_socket->errorString());
    }
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

void PyProblem::setTimeStep(const double timeStep)
{
    if (timeStep >= 0.0)
        Util::problem()->config()->setTimeStep(Value(QString::number(timeStep)));
    else
        throw invalid_argument(QObject::tr("The time step must be positive.").toStdString());
}

void PyProblem::setTimeTotal(const double timeTotal)
{
    if (timeTotal >= 0.0)
        Util::problem()->config()->setTimeTotal(Value(QString::number(timeTotal)));
    else
        throw invalid_argument(QObject::tr("The total time must be positive.").toStdString());
}

void PyProblem::clear()
{
    Util::problem()->clearFieldsAndConfig();
    Util::scene()->clear();
}

void PyProblem::solve()
{
    Util::scene()->refresh();

    Util::problem()->solve();
    if (Util::problem()->isSolved())
    {
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        // Util::scene()->refresh();
    }
}

PyField::PyField(char *fieldId)
{
    QStringList mods;
    std::map<std::string, std::string> modules = availableModules();
    for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
        mods.append(QString::fromStdString(it->first));

    if (mods.contains(QString(fieldId)))
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
        throw invalid_argument(QObject::tr("Invalid field id. Valid keys: %1").arg(stringListToString(mods)).toStdString());
}

FieldInfo *PyField::fieldInfo()
{
    return m_fieldInfo;
}

void PyField::setAnalysisType(const char *analysisType)
{
    QStringList ans;
    std::map<std::string, std::string> analyses = availableAnalyses(m_fieldInfo->fieldId().toStdString());
    for (std::map<std::string, std::string>::iterator it = analyses.begin(); it != analyses.end(); ++it)
        ans.append(QString::fromStdString(it->first));

    if (ans.contains(QString(analysisType)))
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setAnalysisType(analysisTypeFromStringKey(QString(analysisType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(ans)).toStdString());
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
    // TODO: check
    Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setInitialCondition(Value(QString::number(initialCondition)));
}

void PyField::setWeakForms(const char *weakForms)
{
    if (weakFormsTypeStringKeys().contains(QString(weakForms)))
        Util::problem()->fieldInfo(m_fieldInfo->fieldId())->setWeakFormsType(weakFormsTypeFromStringKey(QString(weakForms)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(weakFormsTypeStringKeys())).toStdString());
}

void PyField::addBoundary(char *name, char *type, map<char*, double> parameters)
{
    // check boundaries with same name
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(Util::problem()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (boundary->getName() == name)
            throw invalid_argument(QObject::tr("Boundary '%1' already exists.").arg(QString(name)).toStdString());
    }

    Hermes::Module::BoundaryType *boundaryType = Util::problem()->fieldInfo(m_fieldInfo->fieldId())->module()->get_boundary_type(std::string(type));
    if (!boundaryType)
        throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(QString::fromStdString(type)).toStdString());

    // browse boundary parameters
    std::map<std::string, Value> values;
    for( map<char*, double>::iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundaryType->variables.begin(); it < boundaryType->variables.end(); ++it)
        {
            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            if (variable->id == std::string((*i).first))
            {
                assigned = true;
                values[variable->id] = Value(QString::number((*i).second));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString(std::string((*i).first))).toStdString());
    }

    Util::scene()->addBoundary(new SceneBoundary(fieldInfo(), std::string(name), std::string(type), values));
}

void PyField::setBoundary(char *name, char *type, map<char*, double> parameters)
{
    SceneBoundary *sceneBoundary = Util::scene()->getBoundary(QString(name));
    if (sceneBoundary == NULL)
        throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    // todo: (Franta) check with defined types
    if (std::string(type) != "")
        sceneBoundary->setType(std::string(type));

    // todo: (Franta) check with defined parameters
    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
        sceneBoundary->setValue(std::string((*i).first), Value(QString::number((*i).second)));
}

void PyField::removeBoundary(char *name)
{
    Util::scene()->removeBoundary(Util::scene()->getBoundary(QString(name)));
}

void PyField::addMaterial(char *name, map<char*, double> parameters)
{
    // check materials with same name
    foreach (SceneMaterial *material, Util::scene()->materials->filter(Util::problem()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (material->getName() == name)
            throw invalid_argument(QObject::tr("Material '%1' already exists.").arg(QString(name)).toStdString());
    }

    // browse material parameters
    std::map<std::string, Value> values;
    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
    {
        Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::problem()->fieldInfo(m_fieldInfo->fieldId())->module()->material_type_variables;

        bool assigned = false;
        for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
        {
            Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
            if (variable->id == std::string((*i).first))
            {
                assigned = true;
                values[variable->id] = Value(QString::number((*i).second));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString(std::string((*i).first))).toStdString());
    }

    Util::scene()->addMaterial(new SceneMaterial(fieldInfo(), std::string(name), values));
}

void PyField::setMaterial(char *name, map<char*, double> parameters)
{
    SceneMaterial *sceneMaterial = Util::scene()->getMaterial(QString(name));

    if (sceneMaterial == NULL)
        throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    // todo: (Franta) check with defined parameters
    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
        sceneMaterial->setValue(std::string((*i).first), Value(QString::number((*i).second)));
}

void PyField::removeMaterial(char *name)
{
    Util::scene()->removeMaterial(Util::scene()->getMaterial(QString(name)));
}

void PyField::localValues(double x, double y, map<string, double> &results)
{
    map<string, double> values;

    if (Util::problem()->isSolved())
    {
        // set mode
        currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        currentPythonEngineAgros()->sceneViewPost2D()->actPostprocessorModeLocalPointValue->trigger();

        Point point(x, y);

        LocalPointValue value(fieldInfo(), point);
        for (std::map<Hermes::Module::LocalVariable *, PointValue>::iterator it = value.values.begin(); it != value.values.end(); ++it)
        {
            if (it->first->is_scalar)
            {
                values[it->first->shortname] = it->second.scalar;
            }
            else
            {
                values[it->first->shortname] = it->second.vector.magnitude();
                values[it->first->shortname + Util::problem()->config()->labelX().toLower().toStdString()] = it->second.vector.x;
                values[it->first->shortname + Util::problem()->config()->labelY().toLower().toStdString()] = it->second.vector.y;
            }
        }
    }
    else
    {
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::surfaceIntegrals(vector<int> edges, map<string, double> &results)
{
    map<string, double> values;

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
                    Util::scene()->edges->at(*it)->isSelected = true;
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

        SurfaceIntegralValue surfaceIntegral(fieldInfo());
        for (std::map<Hermes::Module::Integral *, double>::iterator it = surfaceIntegral.values.begin(); it != surfaceIntegral.values.end(); ++it)
        {
            values[it->first->shortname] = it->second;
        }
    }
    else
    {
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyField::volumeIntegrals(vector<int> labels, map<string, double> &results)
{
    map<string, double> values;

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
                    qDebug() << QString::number(*it) << QString::fromStdString(Util::scene()->labels->at(*it)->getMarker(m_fieldInfo)->getName());

                    if (Util::scene()->labels->at(*it)->getMarker(m_fieldInfo) != Util::scene()->materials->getNone(m_fieldInfo))
                    {
                        Util::scene()->labels->at(*it)->isSelected = true;
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

        VolumeIntegralValue volumeIntegral(fieldInfo());
        for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegral.values.begin(); it != volumeIntegral.values.end(); ++it)
        {
            values[it->first->shortname] = it->second;
        }
    }
    else
    {
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PyGeometry::addNode(double x, double y)
{
    Util::scene()->addNode(new SceneNode(Point(x, y)));
}

void PyGeometry::addEdge(double x1, double y1, double x2, double y2, double angle, int refinement, map<char*, char*> boundaries)
{
    // nodes
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

    // angle
    if (angle > 180.0 || angle < 0.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());

    // refinement
    if (refinement < 0)
        throw out_of_range(QObject::tr("Number of refinements '%1' is out of range.").arg(angle).toStdString());

    SceneEdge *sceneEdge = new SceneEdge(nodeStart, nodeEnd, angle, refinement);

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
            if ((sceneBoundary->fieldId() == QString((*i).first)) && (sceneBoundary->getName() == std::string((*i).second)))
            {
                assigned = true;
                sceneEdge->addMarker(sceneBoundary);
                break;
            }
        }

        if (!assigned)
        {
            delete sceneEdge;
            throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString::fromStdString(std::string((*i).second))).toStdString());
        }
    }

    Util::scene()->addEdge(sceneEdge);
}

void PyGeometry::addLabel(double x, double y, double area, int order, map<char*, char*> materials)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    if (order < 0 || order > 10)
        throw out_of_range(QObject::tr("Polynomial order is out of range (0 - 10).").toStdString());

    SceneLabel *sceneLabel = new SceneLabel(Point(x, y), area, order);

    // materials
    for( map<char*, char*>::iterator i = materials.begin(); i != materials.end(); ++i)
    {
        if (!Util::problem()->hasField(QString((*i).first)))
        {
            delete sceneLabel;
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(QString((*i).first)).toStdString());
        }

        if (std::string((*i).second) != "none")
        {

            bool assigned = false;
            foreach (SceneMaterial *sceneMaterial, Util::scene()->materials->filter(Util::problem()->fieldInfo(QString((*i).first))).items())
            {
                if ((sceneMaterial->fieldId() == QString((*i).first)) && (sceneMaterial->getName() == std::string((*i).second)))
                {
                    assigned = true;
                    sceneLabel->addMarker(sceneMaterial);
                    break;
                }
            }

            if (!assigned)
            {
                delete sceneLabel;
                throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(QString::fromStdString(std::string((*i).second))).toStdString());
            }
        }
    }

    Util::scene()->addLabel(sceneLabel);
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
                Util::scene()->nodes->at(*it)->isSelected = true;
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Util::scene()->nodes->length()-1).toStdString());
        }
    }
    else
    {
        Util::scene()->selectAll(SceneGeometryMode_OperateOnNodes);
    }

    Util::scene()->refresh();
}

void PyGeometry::selectEdges(vector<int> edges)
{
    Util::scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Util::scene()->edges->length()))
                Util::scene()->edges->at(*it)->isSelected = true;
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges->length()-1).toStdString());
        }
    }
    else
    {
        Util::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
    }

    Util::scene()->refresh();
}

void PyGeometry::selectLabels(vector<int> labels)
{
    Util::scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Util::scene()->labels->length()))
                Util::scene()->labels->at(*it)->isSelected = true;
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->labels->length()-1).toStdString());
        }
    }
    else
    {
        Util::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
    }

    Util::scene()->refresh();
}

void PyGeometry::selectNodePoint(double x, double y)
{
    SceneNode *node = currentPythonEngineAgros()->sceneViewGeometry()->findClosestNode(Point(x, y));
    if (node)
    {
        node->isSelected = true;
        // sceneView()->doInvalidated();
    }
}

void PyGeometry::selectEdgePoint(double x, double y)
{
    SceneEdge *edge = currentPythonEngineAgros()->sceneViewGeometry()->findClosestEdge(Point(x, y));
    if (edge)
    {
        edge->isSelected = true;
        // sceneView()->doInvalidated();
    }
}

void PyGeometry::selectLabelPoint(double x, double y)
{
    SceneLabel *label = currentPythonEngineAgros()->sceneViewGeometry()->findClosestLabel(Point(x, y));
    if (label)
    {
        label->isSelected = true;
        // sceneView()->doInvalidated();
    }
}

void PyGeometry::selectNone()
{
    Util::scene()->selectNone();
    Util::scene()->refresh();
}

void PyGeometry::moveSelection(double dx, double dy, bool copy)
{
    Util::scene()->transformTranslate(Point(dx, dy), copy);
    // sceneView()->doInvalidated();
}

void PyGeometry::rotateSelection(double x, double y, double angle, bool copy)
{
    Util::scene()->transformRotate(Point(x, y), angle, copy);
    // sceneView()->doInvalidated();
}

void PyGeometry::scaleSelection(double x, double y, double scale, bool copy)
{
    Util::scene()->transformScale(Point(x, y), scale, copy);
    // sceneView()->doInvalidated();
}

void PyGeometry::removeSelection()
{
    Util::scene()->deleteSelected();
}

void PyGeometry::mesh()
{
    Util::problem()->mesh();
    if (Util::problem()->isMeshed())
    {
        currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
        Util::scene()->refresh();
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
    currentPythonEngineAgros()->sceneViewGeometry()->doZoomBestFit();
}

void PyGeometry::zoomIn()
{
    currentPythonEngineAgros()->sceneViewGeometry()->doZoomIn();
}

void PyGeometry::zoomOut()
{
    currentPythonEngineAgros()->sceneViewGeometry()->doZoomOut();
}

void PyGeometry::zoomRegion(double x1, double y1, double x2, double y2)
{
    currentPythonEngineAgros()->sceneViewGeometry()->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

// ****************************************************************************************************

void PyViewConfig::setField(char* variable)
{
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        if (fieldInfo->fieldId() == QString(variable))
        {
            Util::scene()->setActiveViewField(fieldInfo);
            return;
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Util::problem()->fieldInfos().keys())).toStdString());
}

void PyViewConfig::refreshMesh()
{
    currentPythonEngineAgros()->sceneViewMesh()->refresh();
}

void PyViewConfig::refreshPost2D()
{
    currentPythonEngineAgros()->sceneViewPost2D()->refresh();
}

void PyViewConfig::refreshPost3D()
{
    currentPythonEngineAgros()->sceneViewPost3D()->refresh();
}

// ****************************************************************************************************

void PyViewContour::setShow(int show)
{
    Util::config()->showContourView = show;
}

void PyViewContour::setCount(int count)
{
    if (count > 0)
        Util::config()->contoursCount = count;
    else
        throw invalid_argument(QObject::tr("Contour count must be positive.").toStdString());
}

void PyViewContour::setVariable(char* variable)
{
    QStringList list;
    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = Util::scene()->activeViewField()->module()->view_scalar_variables.begin();
         it < Util::scene()->activeViewField()->module()->view_scalar_variables.end(); ++it )
    {
        Hermes::Module::LocalVariable *var = ((Hermes::Module::LocalVariable *) *it);
        if (var->is_scalar)
        {
            list.append(QString::fromStdString(var->id));

            if (var->id == std::string(variable))
            {
                Util::config()->contourVariable = QString(variable);
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

// ****************************************************************************************************

char *pyVersion()
{
    return const_cast<char*>(QApplication::applicationVersion().toStdString().c_str());
}

void pyQuit()
{
    // doesn't work without main event loop (run from script)
    // QApplication::exit(0);

    exit(0);
}

char *pyInput(char *str)
{
    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString(str));
    return const_cast<char*>(text.toStdString().c_str());
}

void pyMessage(char *str)
{
    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Script message"), QString(str));
}

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
    Util::scene()->refresh();

    currentPythonEngineAgros()->sceneViewGeometry()->actSceneModePreprocessor->trigger();

    currentPythonEngineAgros()->sceneViewGeometry()->doZoomBestFit();
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
    // init agros cython extensions
    initagros2d();

    connect(this, SIGNAL(executedScript()), this, SLOT(doExecutedScript()));
}

void PythonEngineAgros::runPythonHeader()
{
    // global script
    if (!Util::config()->globalScript.isEmpty())
        PyRun_String(Util::config()->globalScript.toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // startup script
    if (!Util::problem()->config()->startupscript().isEmpty())
        PyRun_String(Util::problem()->config()->startupscript().toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

void PythonEngineAgros::doExecutedScript()
{
    Util::scene()->refresh();

    if (m_sceneViewGeometry) m_sceneViewGeometry->updateGL();
    if (Util::problem()->isSolved())
    {
        if (m_sceneViewMesh) m_sceneViewMesh->updateGL();
        if (m_sceneViewPost2D) m_sceneViewPost2D->updateGL();
        if (m_sceneViewPost3D) m_sceneViewPost3D->updateGL();
    }
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
