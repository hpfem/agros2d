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
#include "sceneview.h"
#include "scenesolution.h"
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

ScriptResult runPythonScript(const QString &script, const QString &fileName)
{
    logMessage("runPythonScript()");

    return currentPythonEngine()->runPythonScript(script, fileName);
}

ExpressionResult runPythonExpression(const QString &expression, bool returnValue)
{
    logMessage("runPythonExpression()");

    return currentPythonEngine()->runPythonExpression(expression, returnValue);
}

bool scriptIsRunning()
{
    logMessage("scriptIsRunning()");

    if (currentPythonEngine())
        return currentPythonEngine()->isRunning();
    else
        return false;
}

QString createPythonFromModel()
{
    assert(0); //TODO
    //    logMessage("createPythonFromModel()");

    //    QString str;

    //    // model
    //    str += "# model\n";
    //    str += QString("newdocument(name=\"%1\", type=\"%2\",\n"
    //                   "            physicfield=\"%3\", analysistype=\"%4\",\n"
    //                   "            numberofrefinements=%5, polynomialorder=%6,\n"
    //                   "            nonlineartolerance=%7, nonlinearsteps=%8").
    //            arg(Util::scene()->problemInfo()->name).
    //            arg(problemTypeToStringKey(Util::scene()->problemInfo()->problemType)).
    //            arg(QString::fromStdString(Util::scene()->problemInfo()->fieldId())).
    //            arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType)).
    //            arg(Util::scene()->problemInfo()->numberOfRefinements).
    //            arg(Util::scene()->problemInfo()->polynomialOrder).
    //            arg(Util::scene()->problemInfo()->nonlinearTolerance).
    //            arg(Util::scene()->problemInfo()->nonlinearSteps);

    //    if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
    //        str += QString(",\n"
    //                       "            adaptivitytype=\"%1\", adaptivitysteps=%2, adaptivitytolerance=%3").
    //                arg(adaptivityTypeToStringKey(Util::scene()->problemInfo()->adaptivityType)).
    //                arg(Util::scene()->problemInfo()->adaptivitySteps).
    //                arg(Util::scene()->problemInfo()->adaptivityTolerance);

    //    if (Util::scene()->problemInfo()->frequency > 0.0)
    //        str += QString(",\n"
    //                       "            frequency=%1").
    //                arg(Util::scene()->problemInfo()->frequency);

    //    if (Util::scene()->problemInfo()->analysisType() == AnalysisType_Transient)
    //        str += QString(",\n"
    //                       "            adaptivitytype=%1, adaptivitysteps=%2, adaptivitytolerance=%3").
    //                arg(Util::scene()->problemInfo()->timeStep.text()).
    //                arg(Util::scene()->problemInfo()->timeTotal.text()).
    //                arg(Util::scene()->problemInfo()->initialCondition.text());

    //    str += ")\n\n";

    //    // startup script
    //    if (!Util::scene()->problemInfo()->scriptStartup.isEmpty())
    //    {
    //        str += "# startup script\n";
    //        str += Util::scene()->problemInfo()->scriptStartup;
    //        str += "\n\n";
    //    }

    //    // boundaries
    //    if (Util::scene()->boundaries.count() > 1)
    //    {
    //        str += "# boundaries\n";
    //        for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    //        {
    //            str += Util::scene()->boundaries[i]->script() + "\n";
    //        }
    //        str += "\n";
    //    }

    //    // materials
    //    if (Util::scene()->materials.count() > 1)
    //    {
    //        str += "# materials\n";
    //        for (int i = 1; i<Util::scene()->materials.count(); i++)
    //        {
    //            str += Util::scene()->materials[i]->script() + "\n";
    //        }
    //        str += "\n";
    //    }

    //    // edges
    //    if (Util::scene()->edges.count() > 0)
    //    {
    //        str += "# edges\n";
    //        for (int i = 0; i<Util::scene()->edges.count(); i++)
    //        {
    //            str += QString("addedge(%1, %2, %3, %4").
    //                    arg(Util::scene()->edges[i]->nodeStart->point.x).
    //                    arg(Util::scene()->edges[i]->nodeStart->point.y).
    //                    arg(Util::scene()->edges[i]->nodeEnd->point.x).
    //                    arg(Util::scene()->edges[i]->nodeEnd->point.y);

    //            assert(0);
    ////             if (Util::scene()->edges[i]->boundary->name != "none")
    ////                str += QString(", boundary=\"%1\"").
    ////                        arg(QString::fromStdString(Util::scene()->edges[i]->boundary->name));

    //            if (Util::scene()->edges[i]->angle > 0.0)
    //                str += ", angle=" + QString::number(Util::scene()->edges[i]->angle);

    //            if (Util::scene()->edges[i]->refineTowardsEdge > 0)
    //                str += ", refine=" + QString::number(Util::scene()->edges[i]->refineTowardsEdge);

    //            str += ")\n";
    //        }
    //        str += "\n";
    //    }

    //    // labels
    //    if (Util::scene()->labels.count() > 0)
    //    {
    //        str += "# labels\n";
    //        for (int i = 0; i<Util::scene()->labels.count(); i++)
    //        {
    //            str += QString("addlabel(%1, %2, material=\"%3\"").
    //                    arg(Util::scene()->labels[i]->point.x).
    //                    arg(Util::scene()->labels[i]->point.y).
    //                    arg(QString::fromStdString(Util::scene()->labels[i]->material->name));

    //            if (Util::scene()->labels[i]->area > 0.0)
    //                str += ", area=" + QString::number(Util::scene()->labels[i]->area);
    //            if (Util::scene()->labels[i]->polynomialOrder > 0)
    //                str += ", order=" + QString::number(Util::scene()->labels[i]->polynomialOrder);

    //            str += ")\n";
    //        }

    //    }
    //    return str;
}

ScriptEngineRemote::ScriptEngineRemote()
{
    logMessage("ScriptEngineRemote::ScriptEngineRemote()");

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
    logMessage("ScriptEngineRemote::~ScriptEngineRemote()");

    delete m_server;
    delete m_client_socket;
}

void ScriptEngineRemote::connected()
{
    logMessage("ScriptEngineRemote::connected()");

    command = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void ScriptEngineRemote::readCommand()
{
    logMessage("ScriptEngineRemote::readCommand()");

    QTextStream in(m_server_socket);
    command = in.readAll();
}

void ScriptEngineRemote::disconnected()
{
    logMessage("ScriptEngineRemote::disconnected()");

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
    logMessage("ScriptEngineRemote::displayError()");

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

PyProblem::PyProblem(char *coordinateType, char *name, char *meshType, char *matrixSolver, double frequency, double timeStep, double timeTotal)
{
    logMessage("PyProblem::PyProblem()");

    Util::scene()->clear();

    Util::scene()->problemInfo()->name = QString(name);
    Util::scene()->problemInfo()->coordinateType = coordinateTypeFromStringKey(QString(coordinateType));
    Util::scene()->problemInfo()->meshType = meshTypeFromStringKey(QString(meshType));
    Util::scene()->problemInfo()->matrixSolver = matrixSolverTypeFromStringKey(QString(matrixSolver));

    if (frequency >= 0.0)
        Util::scene()->problemInfo()->frequency = frequency;
    else
        throw invalid_argument(QObject::tr("The frequency must be positive.").toStdString());

    // time parameters
    if (timeStep >= 0.0)
        Util::scene()->problemInfo()->timeStep = Value(QString::number(timeStep));
    else
        throw invalid_argument(QObject::tr("The time step must be positive.").toStdString());

    if (timeTotal >= 0.0)
        Util::scene()->problemInfo()->timeTotal = Value(QString::number(timeTotal));
    else
        throw invalid_argument(QObject::tr("The total time must be positive.").toStdString());
}

PyField::PyField(char *fieldId, char *analysisType, int numberOfRefinements, int polynomialOrder, char *linearityType, double nonlinearTolerance, int nonlinearSteps,
                 char *adaptivityType, double adaptivityTolerance, int adaptivitySteps, double initialCondition, char *weakForms)
{
    logMessage("PyField::PyField()");

    m_fieldInfo = new FieldInfo(Util::scene()->problemInfo(), fieldId);

    m_fieldInfo->setAnalysisType(analysisTypeFromStringKey(QString(analysisType)));

    if (numberOfRefinements >= 0 && numberOfRefinements <= 5)
        m_fieldInfo->numberOfRefinements = numberOfRefinements;
    else
        throw invalid_argument(QObject::tr("Number of refenements is out of range (0 - 5).").toStdString());

    if (polynomialOrder >= 1 && polynomialOrder <= 10)
        m_fieldInfo->polynomialOrder = polynomialOrder;
    else
        throw invalid_argument(QObject::tr("Polynomial order is out of range (1 - 10).").toStdString());

    // nonlinearity
    m_fieldInfo->linearityType = linearityTypeFromStringKey(QString(linearityType));

    if (nonlinearTolerance > 0.0)
        m_fieldInfo->nonlinearTolerance = nonlinearTolerance;
    else
        throw invalid_argument(QObject::tr("Nonlinearity tolerance must be positive.").toStdString());

    if (nonlinearSteps >= 1)
        m_fieldInfo->nonlinearSteps = nonlinearSteps;
    else
        throw invalid_argument(QObject::tr("Nonlinearity steps must be higher than 1.").toStdString());

    // adaptivity
    m_fieldInfo->adaptivityType = adaptivityTypeFromStringKey(QString(adaptivityType));

    if (adaptivityTolerance > 0.0)
        m_fieldInfo->adaptivityTolerance = adaptivityTolerance;
    else
        throw invalid_argument(QObject::tr("Adaptivity tolerance must be positive.").toStdString());

    if (adaptivitySteps >= 1)
        m_fieldInfo->adaptivitySteps = adaptivitySteps;
    else
        throw invalid_argument(QObject::tr("Adaptivity steps must be higher than 1.").toStdString());

    m_fieldInfo->initialCondition = Value(QString::number(initialCondition));
    m_fieldInfo->weakFormsType = weakFormsTypeFromStringKey(QString(weakForms));

    Util::scene()->addField(fieldInfo());
}

FieldInfo *PyField::fieldInfo()
{
    return m_fieldInfo;
}

void PyField::addBoundary(char *name, char *type, map<char*, double> parameters)
{
    logMessage("PyField::addBoundary()");

    // check boundaries with same name
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(Util::scene()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (boundary->getName() == name)
            throw invalid_argument(QObject::tr("Boundary '%1' already exists.").arg(QString(name)).toStdString());
    }

    Hermes::Module::BoundaryType *boundaryType = Util::scene()->fieldInfo(m_fieldInfo->fieldId())->module()->get_boundary_type(std::string(type));

    // browse boundary parameters
    std::map<std::string, Value> values;
    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
    {
        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundaryType->variables.begin(); it < boundaryType->variables.end(); ++it)
        {
            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            if (variable->id == std::string((*i).first))
                values[variable->id] = Value(QString::number((*i).second));
        }
    }

    Util::scene()->addBoundary(new SceneBoundary(fieldInfo(), std::string(name), std::string(type), values));
}

void PyField::setBoundary(char *name, char *type, map<char*, double> parameters)
{
    logMessage("PyField::setBoundary()");

    SceneBoundary *sceneBoundary = Util::scene()->getBoundary(QString(name));
    if (std::string(type) != "")
        sceneBoundary->setType(std::string(type));

    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
        sceneBoundary->setValue(std::string((*i).first), Value(QString::number((*i).second)));
}

void PyField::removeBoundary(char *name)
{
    logMessage("PyField::removeBoundary()");

    Util::scene()->removeBoundary(Util::scene()->getBoundary(QString(name)));
}

void PyField::addMaterial(char *name, map<char*, double> parameters)
{
    logMessage("PyField::addMaterial()");

    // check materials with same name
    foreach (SceneMaterial *material, Util::scene()->materials->filter(Util::scene()->fieldInfo(QString(fieldInfo()->fieldId()))).items())
    {
        if (material->getName() == name)
            throw invalid_argument(QObject::tr("Material '%1' already exists.").arg(QString(name)).toStdString());
    }

    // browse material parameters
    std::map<std::string, Value> values;
    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
    {
        Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->fieldInfo(m_fieldInfo->fieldId())->module()->material_type_variables;

        for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
        {
            Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
            if (variable->id == std::string((*i).first))
                values[variable->id] = Value(QString::number((*i).second));
        }
    }

    Util::scene()->addMaterial(new SceneMaterial(fieldInfo(), std::string(name), values));
}

void PyField::setMaterial(char *name, map<char*, double> parameters)
{
    logMessage("PyField::setMaterial()");

    SceneMaterial *sceneMaterial = Util::scene()->getMaterial(QString(name));

    for( map<char*, double>::iterator i=parameters.begin(); i!=parameters.end(); ++i)
        sceneMaterial->setValue(std::string((*i).first), Value(QString::number((*i).second)));
}

void PyField::removeMaterial(char *name)
{
    logMessage("PyField::removeMaterial()");

    Util::scene()->removeMaterial(Util::scene()->getMaterial(QString(name)));
}

void PyGeometry::addNode(double x, double y)
{
    logMessage("PyGeometry::addNode()");

    Util::scene()->addNode(new SceneNode(Point(x, y)));
}

void PyGeometry::addEdge(double x1, double y1, double x2, double y2, double angle, int refinement, map<char*, char*> boundaries)
{
    logMessage("PyGeometry::addEdge()");

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
    for( map<char*, char*>::iterator i=boundaries.begin(); i!=boundaries.end(); ++i)
    {
        //qDebug() << "boundary" << (*i).first << ": " << (*i).second;

        foreach (SceneBoundary *sceneBoundary, Util::scene()->boundaries->filter(Util::scene()->fieldInfo(QString((*i).first))).items())
        {
            if ((sceneBoundary->fieldId() == QString((*i).first)) && (sceneBoundary->getName() == std::string((*i).second)))
                sceneEdge->addMarker(sceneBoundary);
        }
    }

    Util::scene()->addEdge(sceneEdge);
}

void PyGeometry::addLabel(double x, double y, double area, int order, map<char*, char*> materials)
{
    logMessage("PyGeometry::addLabel()");

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    if (order <= 0 || order >= 10)
        throw out_of_range(QObject::tr("Polynomial order is out of range (1 - 10).").toStdString());

    SceneLabel *sceneLabel = new SceneLabel(Point(x, y), area, order);

    // materials
    for( map<char*, char*>::iterator i=materials.begin(); i!=materials.end(); ++i)
    {
        //qDebug() << "material" << (*i).first << ": " << (*i).second;

        foreach (SceneMaterial *sceneMaterial, Util::scene()->materials->filter(Util::scene()->fieldInfo(QString((*i).first))).items())
        {
            if ((sceneMaterial->fieldId() == QString((*i).first)) && (sceneMaterial->getName() == std::string((*i).second)))
            {
                sceneLabel->addMarker(sceneMaterial);
            }
        }
    }

    Util::scene()->addLabel(sceneLabel);
}

void PyGeometry::removeNode(int index)
{
    logMessage("PyGeometry::removeNode()");

    if (index < 0 || index >= Util::scene()->nodes->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());

    Util::scene()->removeNode(Util::scene()->nodes->at(index));
}

void PyGeometry::removeEdge(int index)
{
    logMessage("PyGeometry::removeEdge()");

    if (index < 0 || index >= Util::scene()->edges->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());

    Util::scene()->removeEdge(Util::scene()->edges->at(index));
}

void PyGeometry::removeLabel(int index)
{
    logMessage("PyGeometry::removeLabel()");

    if (index < 0 || index >= Util::scene()->labels->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());

    Util::scene()->removeLabel(Util::scene()->labels->at(index));
}

void PyGeometry::selectNodes(vector<int> nodes)
{
    logMessage("PyGeometry::selectNode()");

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
        Util::scene()->selectAll(SceneMode_OperateOnNodes);
    }

    Util::scene()->refresh();
}

void PyGeometry::selectEdges(vector<int> edges)
{
    logMessage("PyGeometry::selectEdge()");

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
        Util::scene()->selectAll(SceneMode_OperateOnEdges);
    }

    Util::scene()->refresh();
}

void PyGeometry::selectLabels(vector<int> labels)
{
    logMessage("PyGeometry::selectLabel()");

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
        Util::scene()->selectAll(SceneMode_OperateOnLabels);
    }

    Util::scene()->refresh();
}

void PyGeometry::selectNone()
{
    logMessage("PyGeometry::selectNone()");

    Util::scene()->selectNone();
    Util::scene()->refresh();
}

void PyGeometry::moveSelection(double dx, double dy, bool copy)
{
    logMessage("PyGeometry::moveSelection()");

    Util::scene()->transformTranslate(Point(dx, dy), copy);
    sceneView()->doInvalidated();
}

void PyGeometry::rotateSelection(double x, double y, double angle, bool copy)
{
    logMessage("PyGeometry::rotateSelection()");

    Util::scene()->transformRotate(Point(x, y), angle, copy);
    sceneView()->doInvalidated();
}

void PyGeometry::scaleSelection(double x, double y, double scale, bool copy)
{
    logMessage("PyGeometry::scaleSelection()");

    Util::scene()->transformScale(Point(x, y), scale, copy);
    sceneView()->doInvalidated();
}

void PyGeometry::removeSelection()
{
    logMessage("PyGeometry::deleteSelection()");

    Util::scene()->deleteSelected();
}

void PyGeometry::mesh()
{
    logMessage("PyGeometry::mesh()");

    Util::scene()->sceneSolution()->solve(SolverMode_Mesh);
    // FIXME - scene refresh
}

void PyGeometry::zoomBestFit()
{
    logMessage("PyGeometry::zoomBestFit()");

    sceneView()->doZoomBestFit();
}

void PyGeometry::zoomIn()
{
    logMessage("PyGeometry::zoomIn()");

    sceneView()->doZoomIn();
}

void PyGeometry::zoomOut()
{
    logMessage("PyGeometry::zoomOut()");

    sceneView()->doZoomOut();
}

void PyGeometry::zoomRegion(double x1, double y1, double x2, double y2)
{
    logMessage("PyGeometry::zoomRegion()");

    sceneView()->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

char *pyVersion()
{
    logMessage("pyVersion()");

    return const_cast<char*>(QApplication::applicationVersion().toStdString().c_str());
}

char *pyInput(char *str)
{
    logMessage("pyInput()");

    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString(str));
    return const_cast<char*>(text.toStdString().c_str());
}

void pyMessage(char *str)
{
    logMessage("pyMessage()");

    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Script message"), QString(str));
}

void pyQuit()
{
    logMessage("pyQuit()");

    // doesn't work without main event loop (run from script)
    // QApplication::exit(0);

    exit(0);
}

// ************************************************************************************

// meshfilename()
char *pythonMeshFileName()
{
    logMessage("pythonMeshFileName()");

    if (Util::scene()->sceneSolution()->isMeshed())
        return const_cast<char*>(QString(tempProblemFileName() + ".mesh").toStdString().c_str());
    else
        throw invalid_argument(QObject::tr("Problem is not meshed.").toStdString());
}

template <typename Scalar>
Hermes::Hermes2D::Solution<Scalar> *pythonSolutionObject()
{
    logMessage("pythonSolutionObject()");

    if (Util::scene()->sceneSolution()->isSolved())
        return Util::scene()->sceneSolution()->sln();
    else
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());
}

// solutionfilename()
char *pythonSolutionFileName()
{
    logMessage("pythonSolutionFileName()");

    if (Util::scene()->sceneSolution()->isSolved())
    {
        char *fileName = const_cast<char*>(QString(tempProblemFileName() + ".sln").toStdString().c_str());
        //Util::scene()->sceneSolution()->sln()->save(fileName);
        return fileName;
    }
    else
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());
}

// opendocument(filename)
void pythonOpenDocument(char *str)
{
    logMessage("pythonOpenDocument()");

    ErrorResult result = Util::scene()->readFromFile(QString(str));
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

// savedocument(filename)
void pythonSaveDocument(char *str)
{
    logMessage("pythonSaveDocument()");

    ErrorResult result = Util::scene()->writeToFile(QString(str));
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

// closedocument(filename)
void pythonCloseDocument()
{
    logMessage("pythonCloseDocument()");

    Util::scene()->clear();
    sceneView()->doDefaultValues();
    Util::scene()->refresh();

    sceneView()->actSceneModeNode->trigger();
    sceneView()->doZoomBestFit();
}

void pythonDeleteNode(int index)
{
    logMessage("pythonDeleteNode()");

    if (index < 0 || index >= Util::scene()->nodes->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());
    Util::scene()->nodes->remove(Util::scene()->nodes->at(index));
}

void pythonDeleteNodePoint(double x, double y)
{
    logMessage("pythonDeleteNodePoint()");

    Util::scene()->nodes->remove(Util::scene()->getNode(Point(x, y)));
}

void pythonDeleteEdge(int index)
{
    logMessage("pythonDeleteEdge()");

    if (index < 0 || index >= Util::scene()->edges->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());
    Util::scene()->edges->remove(Util::scene()->edges->at(index));
}

void pythonDeleteEdgePoint(double x1, double y1, double x2, double y2, double angle)
{
    logMessage("pythonDeleteEdgePoint()");

    Util::scene()->edges->remove(Util::scene()->getEdge(Point(x1, y1), Point(x2, y2), angle));
}

void pythonDeleteLabel(int index)
{
    logMessage("pythonDeleteLabel()");

    if (index < 0 || index >= Util::scene()->labels->length())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());
    Util::scene()->labels->remove(Util::scene()->labels->at(index));
}

void pythonDeleteLabelPoint(double x, double y)
{
    logMessage("pythonDeleteLabelPoint()");

    Util::scene()->labels->remove(Util::scene()->getLabel(Point(x, y)));
}

// selectnodepoint(x, y)
void pythonSelectNodePoint(double x, double y)
{
    logMessage("pythonSelectNodePoint()");

    SceneNode *node = sceneView()->findClosestNode(Point(x, y));
    if (node)
    {
        node->isSelected = true;
        sceneView()->doInvalidated();
    }
}

// selectedgepoint(x, y)
void pythonSelectEdgePoint(double x, double y)
{
    logMessage("pythonSelectEdgePoint()");

    SceneEdge *edge = sceneView()->findClosestEdge(Point(x, y));
    if (edge)
    {
        edge->isSelected = true;
        sceneView()->doInvalidated();
    }
}

// selectlabelpoint(x, y)
void pythonSelectLabelPoint(double x, double y)
{
    logMessage("pythonSelectLabelPoint()");

    SceneLabel *label = sceneView()->findClosestLabel(Point(x, y));
    if (label)
    {
        label->isSelected = true;
        sceneView()->doInvalidated();
    }
}

// solve()
void pythonSolve()
{
    logMessage("pythonSolve()");

    Util::scene()->sceneSolution()->solve(SolverMode_MeshAndSolve);
    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView()->actSceneModePostprocessor->trigger();
        Util::scene()->refresh();
    }
}

// solveAdaptiveStep()
void pythonSolveAdaptiveStep()
{
    logMessage("pythonSolveAdaptiveStep()");

    // store adaptivity steps
    int adaptivitySteps = Util::scene()->fieldInfo("TODO")->adaptivitySteps;
    Util::scene()->fieldInfo("TODO")->adaptivitySteps = 1;

    // solve
    if (Util::scene()->sceneSolution()->isSolved())
        Util::scene()->sceneSolution()->solve(SolverMode_SolveAdaptiveStep);
    else
        Util::scene()->sceneSolution()->solve(SolverMode_MeshAndSolve);

    // refresh
    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView()->actSceneModePostprocessor->trigger();
        Util::scene()->refresh();
    }

    // restore adaptivity steps
    Util::scene()->fieldInfo("TODO")->adaptivitySteps = adaptivitySteps;
}

// mode(mode = {"node", "edge", "label", "postprocessor"})
void pythonMode(char *str)
{
    logMessage("pythonMode()");

    if (QString(str) == "node")
        sceneView()->actSceneModeNode->trigger();
    else if (QString(str) == "edge")
        sceneView()->actSceneModeEdge->trigger();
    else if (QString(str) == "label")
        sceneView()->actSceneModeLabel->trigger();
    else if (QString(str) == "postprocessor")
        if (Util::scene()->sceneSolution()->isSolved())
            sceneView()->actSceneModePostprocessor->trigger();
        else
            throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());
    else
        throw invalid_argument(QObject::tr("Mode '%1' is not implemented.").arg(QString(str)).toStdString());

    sceneView()->doInvalidated();
}

// postprocessormode(mode = {"point", "surface", "volume"})
void pythonPostprocessorMode(char *str)
{
    logMessage("pythonPostprocessorMode()");

    if (Util::scene()->sceneSolution()->isSolved())
        sceneView()->actSceneModePostprocessor->trigger();
    else
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    if (QString(str) == "point")
        sceneView()->actPostprocessorModeLocalPointValue->trigger();
    else if (QString(str) == "surface")
        sceneView()->actPostprocessorModeSurfaceIntegral->trigger();
    else if (QString(str) == "volume")
        sceneView()->actPostprocessorModeVolumeIntegral->trigger();
    else
        throw invalid_argument(QObject::tr("Postprocessor mode '%1' is not implemented.").arg(QString(str)).toStdString());

    sceneView()->doInvalidated();
}

// result = pointresult(x, y)
static PyObject *pythonPointResult(PyObject *self, PyObject *args)
{
    logMessage("pythonPointResult()");

    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView()->actSceneModePostprocessor->trigger();

        double x, y;
        if (PyArg_ParseTuple(args, "dd", &x, &y))
        {
            PyObject *dict = PyDict_New();

            // coordinates
            PyDict_SetItemString(dict,
                                 Util::scene()->problemInfo()->labelX().toLower().toStdString().c_str(),
                                 Py_BuildValue("d", x));
            PyDict_SetItemString(dict,
                                 Util::scene()->problemInfo()->labelY().toLower().toStdString().c_str(),
                                 Py_BuildValue("d", y));

            Point point(x, y);

            // local point variables
            foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
            {
                LocalPointValue value(fieldInfo, point);
                for (std::map<Hermes::Module::LocalVariable *, PointValue>::iterator it = value.values.begin(); it != value.values.end(); ++it)
                {
                    if (it->first->is_scalar)
                    {
                        // scalar
                        PyDict_SetItemString(dict,
                                             QString::fromStdString(it->first->shortname).toStdString().c_str(),
                                             Py_BuildValue("d", it->second.scalar));
                    }
                    else
                    {
                        // magnitude
                        PyDict_SetItemString(dict,
                                             QString::fromStdString(it->first->shortname).toStdString().c_str(),
                                             Py_BuildValue("d", it->second.vector.magnitude()));

                        // x
                        PyDict_SetItemString(dict,
                                             (QString::fromStdString(it->first->shortname) + Util::scene()->problemInfo()->labelX().toLower()).toStdString().c_str(),
                                             Py_BuildValue("d", it->second.vector.x));

                        // y
                        PyDict_SetItemString(dict,
                                             (QString::fromStdString(it->first->shortname) + Util::scene()->problemInfo()->labelY().toLower()).toStdString().c_str(),
                                             Py_BuildValue("d", it->second.vector.y));
                    }
                }
            }

            return dict;
        }
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
    }
    return NULL;
}

// result = surfaceintegral(list)
static PyObject *pythonSurfaceIntegral(PyObject *self, PyObject *args)
{
    logMessage("pythonSurfaceIntegral()");

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // set mode
        sceneView()->actSceneModePostprocessor->trigger();
        sceneView()->actPostprocessorModeSurfaceIntegral->trigger();
        Util::scene()->selectNone();

        PyObject *list;
        if (PyArg_ParseTuple(args, "O", &list))
        {
            Py_ssize_t size = PyList_Size(list);
            for (int i = 0; i < size; i++)
            {
                PyObject *value = PyList_GetItem(list, i);

                int index;
                PyArg_Parse(value, "i", &index);

                if ((index >= 0) && index < Util::scene()->edges->length())
                {
                    Util::scene()->edges->at(index)->isSelected = true;
                }
                else
                {
                    PyErr_SetString(PyExc_RuntimeError, QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges->length()-1).toStdString().c_str());
                    return NULL;
                }
            }

            PyObject *dict = PyDict_New();

            foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
            {
                SurfaceIntegralValue surfaceIntegral(fieldInfo);
                for (std::map<Hermes::Module::Integral *, double>::iterator it = surfaceIntegral.values.begin(); it != surfaceIntegral.values.end(); ++it)
                {
                    PyDict_SetItemString(dict,
                                         QString::fromStdString(it->first->shortname).toStdString().c_str(),
                                         Py_BuildValue("d", it->second));
                }
            }

            return dict;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Parameter is not a list.").toStdString().c_str());
        }
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
    }
    return NULL;
}

// result = volumeintegral(list)
static PyObject *pythonVolumeIntegral(PyObject *self, PyObject *args)
{
    logMessage("pythonVolumeIntegral()");

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // set mode
        sceneView()->actSceneModePostprocessor->trigger();
        sceneView()->actPostprocessorModeVolumeIntegral->trigger();
        Util::scene()->selectNone();

        PyObject *list;
        if (PyArg_ParseTuple(args, "O", &list))
        {
            Py_ssize_t size = PyList_Size(list);
            for (int i = 0; i < size; i++)
            {
                PyObject *value = PyList_GetItem(list, i);

                int index;
                PyArg_Parse(value, "i", &index);

                if ((index >= 0) && index < Util::scene()->labels->length())
                {
                    Util::scene()->labels->at(index)->isSelected = true;
                }
                else
                {
                    PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->labels->length()-1).toStdString().c_str());
                    return NULL;
                }
            }

            PyObject *dict = PyDict_New();

            foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
            {
                VolumeIntegralValue volumeIntegral(fieldInfo);
                for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegral.values.begin(); it != volumeIntegral.values.end(); ++it)
                {
                    PyDict_SetItemString(dict,
                                         QString::fromStdString(it->first->shortname).toStdString().c_str(),
                                         Py_BuildValue("d", it->second));
                }
            }

            return dict;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Parameter is not a list.").toStdString().c_str());
        }
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
    }
    return NULL;
}

// showscalar(type = { "none", "scalar", "scalar3d", "order" }, variable, component, rangemin, rangemax)
void pythonShowScalar(char *type, char *variable, char *component, double rangemin, double rangemax)
{
    logMessage("pythonShowScalar()");

    // type
    SceneViewPostprocessorShow postprocessorShow = sceneViewPostprocessorShowFromStringKey(QString(type));
    if (postprocessorShow != SceneViewPostprocessorShow_Undefined)
        sceneView()->sceneViewSettings().postprocessorShow = postprocessorShow;
    else
        throw invalid_argument(QObject::tr("View type '%1' is not implemented.").arg(QString(type)).toStdString());

    // variable
    if (QString(variable) == "default")
    {
        sceneView()->sceneViewSettings().scalarPhysicFieldVariable = Util::scene()->fieldInfo("TODO")->module()->view_default_scalar_variable->id;
    }
    else
    {
        bool ok = false;
        for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = Util::scene()->fieldInfo("TODO")->module()->variables.begin();
             it < Util::scene()->fieldInfo("TODO")->module()->variables.end(); ++it )
        {
            Hermes::Module::LocalVariable *var = ((Hermes::Module::LocalVariable *) *it);
            if (QString::fromStdString(var->id) == QString(variable))
            {
                sceneView()->sceneViewSettings().scalarPhysicFieldVariable = QString(variable).toStdString();
                ok = true;

                // variable component
                if (QString(component) == "default")
                {
                    sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = Util::scene()->fieldInfo("TODO")->module()->view_default_scalar_variable_comp();
                }
                else
                {
                    PhysicFieldVariableComp comp = physicFieldVariableCompFromStringKey(QString(component));
                    if (comp == PhysicFieldVariableComp_Undefined)
                        throw invalid_argument(QObject::tr("Physic field variable component '%1' is not implemented.").arg(QString(component)).toStdString());
                    if (!var->is_scalar && comp == PhysicFieldVariableComp_Scalar)
                        throw invalid_argument(QObject::tr("Physic field variable is scalar variable.").toStdString());

                    sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = comp;
                }
            }
        }

        if (!ok)
            throw invalid_argument(QObject::tr("Physic field variable '%1' is not implemented.").arg(QString(variable)).toStdString());
    }

    // range
    if (rangemin != -123456)
    {
        sceneView()->sceneViewSettings().scalarRangeAuto = false;
        sceneView()->sceneViewSettings().scalarRangeMin = rangemin;
    }
    else
    {
        sceneView()->sceneViewSettings().scalarRangeAuto = true;
    }
    if (rangemax != -123456)
        sceneView()->sceneViewSettings().scalarRangeMax = rangemax;

    // sceneView()->doInvalidated();
    Util::scene()->sceneSolution()->setTimeStep(Util::scene()->sceneSolution()->timeStep(), false);
}

// showgrid(show = {True, False})
void pythonShowGrid(bool show)
{
    logMessage("pythonShowGrid()");

    Util::config()->showGrid = show;
    sceneView()->doInvalidated();
}

// showgeometry(show = {True, False})
void pythonShowGeometry(bool show)
{
    logMessage("pythonShowGeometry()");

    sceneView()->sceneViewSettings().showGeometry = show;
    sceneView()->doInvalidated();
}

// showinitialmesh(show = {True, False})
void pythonShowInitialMesh(bool show)
{
    logMessage("pythonShowInitialMesh()");

    sceneView()->sceneViewSettings().showInitialMesh = show;
    sceneView()->doInvalidated();
}

// showsolutionmesh(show = {True, False})
void pythonShowSolutionMesh(bool show)
{
    logMessage("pythonShowSolutionMesh()");

    sceneView()->sceneViewSettings().showSolutionMesh = show;
    sceneView()->doInvalidated();
}

// showcontours(show = {True, False})
void pythonShowContours(bool show)
{
    logMessage("pythonShowContours()");

    sceneView()->sceneViewSettings().showContours = show;

    // sceneView()->doInvalidated();
    Util::scene()->sceneSolution()->setTimeStep(Util::scene()->sceneSolution()->timeStep(), false);
}

// showvectors(show = {True, False})
void pythonShowVectors(bool show)
{
    logMessage("pythonShowVectors()");

    sceneView()->sceneViewSettings().showVectors = show;

    // sceneView()->doInvalidated();
    Util::scene()->sceneSolution()->setTimeStep(Util::scene()->sceneSolution()->timeStep(), false);
}

// settimestep(level)
void pythonSetTimeStep(int timestep)
{
    assert(0); //TODO
    //    logMessage("pythonSetTimeStep()");

    //    if (Util::scene()->sceneSolution()->isSolved())
    //        sceneView()->actSceneModePostprocessor->trigger();
    //    else
    //        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    //    if (Util::scene()->fieldInfo("TODO")->analysisType() != AnalysisType_Transient)
    //        throw invalid_argument(QObject::tr("Solved problem is not transient.").toStdString());

    //    if ((timestep < 0) || (timestep > Util::scene()->sceneSolution()->timeStepCount()))
    //        throw out_of_range(QObject::tr("Time step must be between 0 and %1.").arg(Util::scene()->sceneSolution()->timeStepCount()).toStdString());

    //    Util::scene()->sceneSolution()->setTimeStep(timestep, false);
}

// timestepcount()
int pythonTimeStepCount()
{
    logMessage("pythonTimeStepCount()");

    return Util::scene()->sceneSolution()->timeStepCount();
}

// saveimage(filename)
void pythonSaveImage(char *str, int w, int h)
{
    logMessage("pythonSaveImage()");

    ErrorResult result = sceneView()->saveImageToFile(QString(str), w, h);
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

static PyMethodDef pythonMethodsAgros[] =
{
    {"pointresult", pythonPointResult, METH_VARARGS, "pointresult(x, y)"},
    {"volumeintegral", pythonVolumeIntegral, METH_VARARGS, "volumeintegral(list)"},
    {"surfaceintegral", pythonSurfaceIntegral, METH_VARARGS, "surfaceintegral(list)"},
    {NULL, NULL, 0, NULL}
};

// *******************************************************************************************

void PythonEngineAgros::addCustomExtensions()
{
    // init agros cython extensions
    initagros2d();
    // agros2d file
    Py_InitModule("agros2d", pythonMethodsAgros);

    connect(this, SIGNAL(executedScript()), this, SLOT(doExecutedScript()));
}

void PythonEngineAgros::runPythonHeader()
{
    // global script
    if (!Util::config()->globalScript.isEmpty())
        PyRun_String(Util::config()->globalScript.toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // startup script
    if (!Util::scene()->problemInfo()->startupscript.isEmpty())
        PyRun_String(Util::scene()->problemInfo()->startupscript.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

void PythonEngineAgros::doExecutedScript()
{
    Util::scene()->refresh();
    sceneView()->doInvalidated();
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
    logMessage("ScriptEditorDialog::doCreatePythonFromModel()");

    txtEditor->setPlainText(createPythonFromModel());
}
