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
#include "../resources_source/python/agros2d.c"

#include "scene.h"
#include "sceneview.h"
#include "scenesolution.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"

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

// version()
char *pythonVersion()
{
    logMessage("pythonVersion()");

    return const_cast<char*>(QApplication::applicationVersion().toStdString().c_str());
}

// message(string)
void pythonMessage(char *str)
{
    logMessage("pythonMessage()");

    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Script message"), QString(str));
}

// variable = input(string)
char *pythonInput(char *str)
{
    logMessage("pythonInput()");

    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString(str));
    return const_cast<char*>(text.toStdString().c_str());
}

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

// quit()
void pythonQuit()
{
    logMessage("pythonQuit()");

    // doesn't work without main event loop (run from script)
    // QApplication::exit(0);

    exit(0);
}

// newdocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance,
// frequency, analysistype, timestep, totaltime, initialcondition, linearitytype, nonlineartolerance, nonlinearsteps)
void pythonNewDocument(char *name, char *type, char *physicfield,
                       int numberofrefinements, int polynomialorder, char *adaptivitytype,
                       double adaptivitysteps, double adaptivitytolerance,
                       double frequency,
                       char *analysistype, double timestep, double totaltime, double initialcondition,
                       char *linearitytype, double nonlineartolerance, int nonlinearsteps)
{
    assert(0); //TODO
    //    logMessage("pythonNewDocument()");

    //    Util::scene()->clear();
    //    Util::scene()->fieldInfo("TODO")->name = QString(name);

    //    // type
    //    Util::scene()->fieldInfo("TODO")->problemType = problemTypeFromStringKey(QString(type));
    //    if (Util::scene()->fieldInfo("TODO")->problemType == ProblemType_Undefined)
    //        throw invalid_argument(QObject::tr("Problem type '%1' is not implemented.").arg(QString(type)).toStdString());

    //    // analysis type
    //    Util::scene()->fieldInfo("TODO")->analysisType() = analysisTypeFromStringKey(QString(analysistype));
    //    if (Util::scene()->fieldInfo("TODO")->analysisType() == AnalysisType_Undefined)
    //        throw invalid_argument(QObject::tr("Analysis type '%1' is not implemented").arg(QString(adaptivitytype)).toStdString());

    //    // physicfield
    //    QString field = physicfield;
    //    if (field != "")
    //    {
    //        Util::scene()->fieldInfo("TODO")->setModule(moduleFactory(field.toStdString(),
    //                                                              problemTypeFromStringKey(QString(type)),
    //                                                              analysisTypeFromStringKey(QString(analysistype)),
    //                                                              ""));
    //    }
    //    else
    //        throw invalid_argument(QObject::tr("Physic field '%1' is not implemented.").arg(QString(physicfield)).toStdString());

    //    // numberofrefinements
    //    if (numberofrefinements >= 0)
    //        Util::scene()->fieldInfo("TODO")->numberOfRefinements = numberofrefinements;
    //    else
    //        throw out_of_range(QObject::tr("Number of refinements '%1' is out of range.").arg(numberofrefinements).toStdString());

    //    // polynomialorder
    //    if (polynomialorder >= 1 && polynomialorder <= 10)
    //        Util::scene()->fieldInfo("TODO")->polynomialOrder = polynomialorder;
    //    else
    //        throw out_of_range(QObject::tr("Polynomial order '%1' is out of range.").arg(polynomialorder).toStdString());

    //    // adaptivitytype
    //    Util::scene()->fieldInfo("TODO")->adaptivityType = adaptivityTypeFromStringKey(QString(adaptivitytype));
    //    if (Util::scene()->fieldInfo("TODO")->adaptivityType == AdaptivityType_Undefined)
    //        throw invalid_argument(QObject::tr("Adaptivity type '%1' is not implemented.").arg(QString(adaptivitytype)).toStdString());

    //    // adaptivitysteps
    //    if (adaptivitysteps >= 0)
    //        Util::scene()->fieldInfo("TODO")->adaptivitySteps = adaptivitysteps;
    //    else
    //        throw out_of_range(QObject::tr("Adaptivity step '%1' is out of range.").arg(adaptivitysteps).toStdString());

    //    // adaptivitytolerance
    //    if (adaptivitytolerance >= 0)
    //        Util::scene()->fieldInfo("TODO")->adaptivityTolerance = adaptivitytolerance;
    //    else
    //        throw out_of_range(QObject::tr("Adaptivity tolerance '%1' is out of range.").arg(adaptivitytolerance).toStdString());

    //    // frequency
    //    if (Util::scene()->fieldInfo("TODO")->module()->harmonic_solutions)
    //    {
    //        if (frequency >= 0)
    //            Util::scene()->fieldInfo("TODO")->frequency = frequency;
    //        else
    //            throw invalid_argument(QObject::tr("The frequency can not be used for this problem.").toStdString());
    //    }

    //    // transient timestep
    //    if (timestep > 0)
    //        Util::scene()->fieldInfo("TODO")->timeStep = Value(QString::number(timestep));
    //    else if (Util::scene()->fieldInfo("TODO")->analysisType() == AnalysisType_Transient)
    //        throw out_of_range(QObject::tr("Time step must be positive.").toStdString());

    //    // transient timetotal
    //    if (totaltime > 0)
    //        Util::scene()->fieldInfo("TODO")->timeTotal = Value(QString::number(totaltime));
    //    else if (Util::scene()->fieldInfo("TODO")->analysisType() == AnalysisType_Transient)
    //        throw out_of_range(QObject::tr("Total time must be positive.").toStdString());

    //    // nonlineartolerance
    //    if (nonlineartolerance >= 0)
    //        Util::scene()->fieldInfo("TODO")->nonlinearTolerance = nonlineartolerance;
    //    else
    //        throw out_of_range(QObject::tr("Nonlinear tolerance '%1' is out of range.").arg(nonlineartolerance).toStdString());

    //    // nonlinearsteps
    //    if (nonlinearsteps >= 0)
    //        Util::scene()->fieldInfo("TODO")->nonlinearSteps = nonlinearsteps;
    //    else
    //        throw out_of_range(QObject::tr("Number of nonlinear steps '%1' must be positive.").arg(nonlinearsteps).toStdString());

    //    // linearity type
    //    Util::scene()->fieldInfo("TODO")->linearityType = linearityTypeFromStringKey(QString(linearitytype));
    //    if (Util::scene()->fieldInfo("TODO")->linearityType == LinearityType_Undefined)
    //        throw invalid_argument(QObject::tr("Linearity type '%1' is not implemented").arg(QString(linearitytype)).toStdString());

    //    // transient initial condition
    //    Util::scene()->fieldInfo("TODO")->initialCondition = Value(QString::number(initialcondition));

    //    // invalidate
    //    sceneView()->doDefaultValues();
    //    Util::scene()->refresh();
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

// addnode(x, y)
void pythonAddNode(double x, double y)
{
    logMessage("pythonAddNode()");

    Util::scene()->addNode(new SceneNode(Point(x, y)));
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

// addedge(x1, y1, x2, y2, angle = 0, marker = "none")
void pythonAddEdge(double x1, double y1, double x2, double y2, char *boundary, double angle, int refine)
{
    assert(0); //TODO
    //    logMessage("pythonAddEdge()");

    //    if (angle > 180.0 || angle < 0.0)
    //        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());

    //    if (refine < 0)
    //        throw out_of_range(QObject::tr("Number of refinements '%1' is out of range.").arg(angle).toStdString());

    //    SceneBoundary *scene_boundary = Util::scene()->getBoundary(QString(boundary));
    //    if (!scene_boundary)
    //        throw invalid_argument(QObject::tr("Boundary '%1' is not defined.").arg(boundary).toStdString());

    //    // start node
    //    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
    //    // end node
    //    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

    //    Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, scene_boundary, angle, refine));
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

// addlabel(x, y, area = 0, marker = "none", polynomialorder = 0)
void pythonAddLabel(double x, double y, char *material, double area, int order)
{
    assert(0); //TODO
    //    logMessage("pythonAddLabel()");

    //    if (order < 0)
    //        throw out_of_range(QObject::tr("Polynomial order '%1' is out of range.").arg(order).toStdString());

    //    SceneMaterial *scene_material = Util::scene()->getMaterial(QString(material));
    //    if (!scene_material)
    //        throw invalid_argument(QObject::tr("Material '%1' is not defined.").arg(material).toStdString());

    //    Util::scene()->addLabel(new SceneLabel(Point(x, y), scene_material, area, order));
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

// addboundary(name, type, value, ...)
static PyObject *pythonAddBoundary(PyObject *self, PyObject *args)
{
    assert(0); //TODO
    //    logMessage("pythonAddBoundary()");

    //    PyObject *dict;
    //    char *name, *type;
    //    if (PyArg_ParseTuple(args, "ssO", &name, &type, &dict))
    //    {
    //        // check name
    //        if (Util::scene()->getBoundary(name))
    //        {
    //            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary already exists.").toStdString().c_str());
    //            return NULL;
    //        }

    //        PyObject *key, *value;
    //        Py_ssize_t pos = 0;

    //        std::map<std::string, Value> values;
    //        while (PyDict_Next(dict, &pos, &key, &value))
    //        {
    //            double val;
    //            char *str;

    //            // key
    //            PyArg_Parse(key, "s", &str);
    //            PyArg_Parse(value, "d", &val);

    //            Hermes::Module::BoundaryType *boundary_type = Util::scene()->fieldInfo("TODO")->module()->get_boundary_type(type);
    //            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
    //            {
    //                Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
    //                if (variable->shortname == std::string(str))
    //                    values[variable->id] = Value(QString::number(val));
    //            }
    //        }

    //        assert(0); //TODO
    //        //Util::scene()->addBoundary(new SceneBoundary(name, type, values));
    //        Py_RETURN_NONE;
    //    }

    //    return NULL;
}

// modifyBoundary(name, type, value, ...)
static PyObject *pythonModifyBoundary(PyObject *self, PyObject *args)
{
    assert(0); //TODO
    //    logMessage("pythonModifyBoundary()");

    //    PyObject *dict;
    //    char *name, *type;
    //    if (PyArg_ParseTuple(args, "ssO", &name, &type, &dict))
    //    {
    //        if (SceneBoundary *boundary = Util::scene()->getBoundary(name))
    //        {
    //            if (Hermes::Module::BoundaryType *boundary_type = Util::scene()->fieldInfo("TODO")->module()->get_boundary_type(type))
    //            {
    // boundary type

    //                boundary->type = type;

    //                // variables
    //                PyObject *key, *value;
    //                Py_ssize_t pos = 0;

    //                while (PyDict_Next(dict, &pos, &key, &value))
    //                {
    //                    double val;
    //                    char *str;

    //                    // key
    //                    PyArg_Parse(key, "s", &str);
    //                    PyArg_Parse(value, "d", &val);

    //                    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
    //                    {
    //                        Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
    //                        if (variable->shortname == std::string(str))
    //                            boundary->values[variable->id] = Value(QString::number(val));
    //                    }
    //                }

    //                Py_RETURN_NONE;
    //            }
    //            else
    //            {
    //                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
    //                return NULL;
    //            }
    //        }
    //        else
    //        {
    //            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary with name '%1' doesn't exists.").arg(name).toStdString().c_str());
    //            return NULL;
    //        }
    //    }

    //    return NULL;
}

// addmaterial(name, type, value, ...)
static PyObject *pythonAddMaterial(PyObject *self, PyObject *args)
{
    assert(0); //TODO
    //    logMessage("pythonAddMaterial()");

    //    PyObject *dict;
    //    char *name;
    //    if (PyArg_ParseTuple(args, "sO", &name, &dict))
    //    {
    //        // check name
    //        if (Util::scene()->getMaterial(name))
    //        {
    //            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker already exists.").toStdString().c_str());
    //            return NULL;
    //        }

    //        PyObject *key, *value;
    //        Py_ssize_t pos = 0;

    //        std::map<std::string, Value> values;
    //        while (PyDict_Next(dict, &pos, &key, &value))
    //        {
    //            double val;
    //            char *str;

    //            // key
    //            PyArg_Parse(key, "s", &str);
    //            PyArg_Parse(value, "d", &val);

    //            Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->fieldInfo("TODO")->module()->material_type_variables;
    //            for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
    //            {
    //                Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
    //                if (variable->shortname == std::string(str))
    //                    values[variable->id] = Value(QString::number(val));
    //            }
    //        }

    //        Util::scene()->addMaterial(new SceneMaterial(name, values));
    //        Py_RETURN_NONE;
    //    }

    //    return NULL;
}

// modifymaterial(name, type, value, ...)
static PyObject *pythonModifyMaterial(PyObject *self, PyObject *args)
{
    assert(0); //TODO
    //    logMessage("pythonModifyMaterial()");

    //    PyObject *dict;
    //    char *name, *type;
    //    if (PyArg_ParseTuple(args, "sO", &name, &dict))
    //    {
    //        if (SceneMaterial *material = Util::scene()->getMaterial(name))
    //        {
    //            // variables
    //            PyObject *key, *value;
    //            Py_ssize_t pos = 0;

    //            while (PyDict_Next(dict, &pos, &key, &value))
    //            {
    //                double val;
    //                char *str;

    //                // key
    //                PyArg_Parse(key, "s", &str);
    //                PyArg_Parse(value, "d", &val);

    //                Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->fieldInfo("TODO")->module()->material_type_variables;
    //                for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
    //                {
    //                    Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
    //                    if (variable->shortname == std::string(str))
    //                        material->values[variable->id] = Value(QString::number(val));
    //                }
    //            }

    //            Py_RETURN_NONE;
    //        }
    //        else
    //        {
    //            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Material with name '%1' doesn't exists.").arg(name).toStdString().c_str());
    //            return NULL;
    //        }
    //    }

    //    return NULL;
}

// selectnone()
void pythonSelectNone()
{
    logMessage("pythonSelectNone()");

    Util::scene()->selectNone();
}

// selectall()
void pythonSelectAll()
{
    logMessage("pythonSelectAll()");

    if (sceneView()->sceneMode() == SceneMode_Postprocessor)
    {
        // select volume integral area
        if (sceneView()->actPostprocessorModeVolumeIntegral->isChecked())
            Util::scene()->selectAll(SceneMode_OperateOnLabels);

        // select surface integral area
        if (sceneView()->actPostprocessorModeSurfaceIntegral->isChecked())
            Util::scene()->selectAll(SceneMode_OperateOnEdges);
    }
    else
    {
        Util::scene()->selectAll(sceneView()->sceneMode());
    }
    sceneView()->doInvalidated();
}

// selectnode(node)
static PyObject *pythonSelectNode(PyObject *self, PyObject *args)
{
    logMessage("pythonSelectNode()");

    PyObject *list;
    if (PyArg_ParseTuple(args, "O", &list))
    {
        sceneView()->actSceneModeNode->trigger();
        Util::scene()->selectNone();

        Py_ssize_t size = PyList_Size(list);
        for (int i = 0; i < size; i++)
        {
            PyObject *value = PyList_GetItem(list, i);

            int index;
            PyArg_Parse(value, "i", &index);

            if ((index >= 0) && index < Util::scene()->nodes->length())
            {
                Util::scene()->nodes->at(index)->isSelected = true;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Node index must be between 0 and '%1'.").arg(Util::scene()->edges->length()-1).toStdString().c_str());
                return NULL;
            }
        }

        sceneView()->doInvalidated();
        Py_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Parameter is not a list.").toStdString().c_str());
    }
    return NULL;
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

// selectedge(list)
static PyObject *pythonSelectEdge(PyObject *self, PyObject *args)
{
    assert(0); //TODO
    //    logMessage("pythonSelectEdge()");

    //    PyObject *list;
    //    if (PyArg_ParseTuple(args, "O", &list))
    //    {
    //        sceneView()->actSceneModeEdge->trigger();
    //        Util::scene()->selectNone();

    //        Py_ssize_t size = PyList_Size(list);
    //        for (int i = 0; i < size; i++)
    //        {
    //            PyObject *value = PyList_GetItem(list, i);

    //            int index;
    //            PyArg_Parse(value, "i", &index);

    //            if ((index >= 0) && index < Util::scene()->edges.count())
    //            {
    //                Util::scene()->edges[index]->isSelected = true;
    //            }
    //            else
    //            {
    //                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges.count()-1).toStdString().c_str());
    //                return NULL;
    //            }
    //        }

    //        sceneView()->doInvalidated();
    //        Py_RETURN_NONE;
    //    }
    //    else
    //    {
    //        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Parameter is not a list.").toStdString().c_str());
    //    }
    //    return NULL;
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

// selectlabel(list)
static PyObject *pythonSelectLabel(PyObject *self, PyObject *args)
{
    logMessage("pythonSelectLabel()");

    PyObject *list;
    if (PyArg_ParseTuple(args, "O", &list))
    {
        Util::scene()->selectNone();

        Py_ssize_t size = PyList_Size(list);
        for (int i = 0; i < size; i++)
        {
            sceneView()->actSceneModeLabel->trigger();
            PyObject *value = PyList_GetItem(list, i);

            int index;
            PyArg_Parse(value, "i", &index);

            if ((index >= 0) && index < Util::scene()->labels->length())
            {
                Util::scene()->labels->at(index)->isSelected = true;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->edges->length()-1).toStdString().c_str());
                return NULL;
            }
        }

        sceneView()->doInvalidated();
        Py_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Parameter is not a list.").toStdString().c_str());
    }
    return NULL;
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

// rotateselection(x, y, angle, copy = {True, False})
void pythonRotateSelection(double x, double y, double angle, bool copy)
{
    logMessage("pythonRotateSelection()");

    Util::scene()->transformRotate(Point(x, y), angle, copy);
    sceneView()->doInvalidated();
}

// scaleselection(x, y, scale, copy = {True, False})
void pythonScaleSelection(double x, double y, double scale, bool copy)
{
    logMessage("pythonScaleSelection()");

    Util::scene()->transformScale(Point(x, y), scale, copy);
    sceneView()->doInvalidated();
}

// moveselection(dx, dy, copy = {True, False})
void pythonMoveSelection(double dx, double dy, bool copy)
{
    logMessage("pythonMoveSelection()");

    Util::scene()->transformTranslate(Point(dx, dy), copy);
    sceneView()->doInvalidated();
}

// deleteselection()
void pythonDeleteSelection()
{
    logMessage("pythonDeleteSelection()");

    Util::scene()->deleteSelected();
}

// mesh()
void pythonMesh()
{
    logMessage("pythonMesh()");

    Util::scene()->sceneSolution()->solve(SolverMode_Mesh);
    Util::scene()->refresh();
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

// zoombestfit()
void pythonZoomBestFit()
{
    logMessage("pythonZoomBestFit()");

    sceneView()->doZoomBestFit();
}

// zoomin()
void pythonZoomIn()
{
    logMessage("pythonZoomIn()");

    sceneView()->doZoomIn();
}

// zoomout()
void pythonZoomOut()
{
    logMessage("pythonZoomOut()");

    sceneView()->doZoomOut();
}

// zoomregion(x1, y1, x2, y2)
void pythonZoomRegion(double x1, double y1, double x2, double y2)
{
    logMessage("pythonZoomRegion()");

    sceneView()->doZoomRegion(Point(x1, y1), Point(x2, y2));
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
    {"addboundary", pythonAddBoundary, METH_VARARGS, "addboundary(name, type, value, ...)"},
    {"modifyboundary", pythonModifyBoundary, METH_VARARGS, "modifyBoundary(name, type, value, ...)"},
    {"addmaterial", pythonAddMaterial, METH_VARARGS, "addmaterial(name, type, value, ...)"},
    {"modifymaterial", pythonModifyMaterial, METH_VARARGS, "modifymaterial(name, type, value, ...)"},
    {"selectnode", pythonSelectNode, METH_VARARGS, "selectnode(list)"},
    {"selectedge", pythonSelectEdge, METH_VARARGS, "selectedge(list)"},
    {"selectlabel", pythonSelectLabel, METH_VARARGS, "selectlabel(list)"},
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
    Py_InitModule("agros2file", pythonMethodsAgros);
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
