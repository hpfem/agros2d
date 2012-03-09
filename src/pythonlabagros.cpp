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
#include "python/agros2d.cpp"

#include "util.h"
#include "scene.h"
#include "sceneview.h"
#include "scenemarker.h"

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
    logMessage("createPythonFromModel()");

    QString str;

    // model
    str += "# model\n";
    str += QString("newdocument(\"%1\", \"%2\", \"%3\", %4, %5, \"%6\", %7, %8, %9, \"%10\", %11, %12, %13)").
            arg(Util::scene()->problemInfo()->name).
            arg(problemTypeToStringKey(Util::scene()->problemInfo()->problemType)).
            arg(physicFieldToStringKey(Util::scene()->problemInfo()->physicField())).
            arg(Util::scene()->problemInfo()->numberOfRefinements).
            arg(Util::scene()->problemInfo()->polynomialOrder).
            arg(adaptivityTypeToStringKey(Util::scene()->problemInfo()->adaptivityType)).
            arg(Util::scene()->problemInfo()->adaptivitySteps).
            arg(Util::scene()->problemInfo()->adaptivityTolerance).
            arg(Util::scene()->problemInfo()->frequency).
            arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType)).
            arg(Util::scene()->problemInfo()->timeStep.text).
            arg(Util::scene()->problemInfo()->timeTotal.text).
            arg(Util::scene()->problemInfo()->initialCondition.text)
            + "\n";
    str += "\n";

    // startup script
    if (!Util::scene()->problemInfo()->scriptStartup.isEmpty())
    {
        str += "# startup script\n";
        str += Util::scene()->problemInfo()->scriptStartup;
        str += "\n\n";
    }

    // boundaries
    if (Util::scene()->boundaries.count() > 1)
    {
        str += "# boundaries\n";
        for (int i = 1; i<Util::scene()->boundaries.count(); i++)
        {
            str += Util::scene()->boundaries[i]->script() + "\n";
        }
        str += "\n";
    }

    // materials
    if (Util::scene()->materials.count() > 1)
    {
        str += "# materials\n";
        for (int i = 1; i<Util::scene()->materials.count(); i++)
        {
            str += Util::scene()->materials[i]->script() + "\n";
        }
        str += "\n";
    }

    // edges
    if (Util::scene()->edges.count() > 0)
    {
        str += "# edges\n";
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            str += QString("addedge(%1, %2, %3, %4, %5, \"%6\")").
                    arg(Util::scene()->edges[i]->nodeStart->point.x).
                    arg(Util::scene()->edges[i]->nodeStart->point.y).
                    arg(Util::scene()->edges[i]->nodeEnd->point.x).
                    arg(Util::scene()->edges[i]->nodeEnd->point.y).
                    arg(Util::scene()->edges[i]->angle).
                    arg(Util::scene()->edges[i]->boundary->name) + "\n";
        }
        str += "\n";
    }

    // labels
    if (Util::scene()->labels.count() > 0)
    {
        str += "# labels\n";
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            str += QString("addlabel(%1, %2, %3, %4, \"%5\")").
                    arg(Util::scene()->labels[i]->point.x).
                    arg(Util::scene()->labels[i]->point.y).
                    arg(Util::scene()->labels[i]->area).
                    arg(Util::scene()->labels[i]->polynomialOrder).
                    arg(Util::scene()->labels[i]->material->name) + "\n";
        }

    }
    return str;
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

// FIX ********************************************************************************************************************************************************************
// Terible, is it possible to write this code better???
#define python_int_array() \
    const int count = 100; \
    int index[count]; \
    for (int i = 0; i < count; i++) \
    index[i] = INT_MIN; \
    if (PyArg_ParseTuple(args, "i|iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", \
    &index[ 0], &index[ 1], &index[ 2], &index[ 3], &index[ 4], &index[ 5], &index[ 6], &index[ 7], &index[ 8], &index[ 9], \
    &index[10], &index[11], &index[12], &index[13], &index[14], &index[15], &index[16], &index[17], &index[18], &index[19], \
    &index[20], &index[21], &index[22], &index[23], &index[24], &index[25], &index[26], &index[27], &index[28], &index[29], \
    &index[30], &index[31], &index[32], &index[33], &index[34], &index[35], &index[36], &index[37], &index[38], &index[39], \
    &index[40], &index[41], &index[42], &index[43], &index[44], &index[45], &index[46], &index[47], &index[48], &index[49], \
    &index[50], &index[51], &index[52], &index[53], &index[54], &index[55], &index[56], &index[57], &index[58], &index[59], \
    &index[60], &index[61], &index[62], &index[63], &index[64], &index[65], &index[66], &index[67], &index[68], &index[69], \
    &index[70], &index[71], &index[72], &index[73], &index[74], &index[75], &index[76], &index[77], &index[78], &index[79], \
    &index[80], &index[81], &index[82], &index[83], &index[84], &index[85], &index[86], &index[87], &index[88], &index[89], \
    &index[90], &index[91], &index[92], &index[93], &index[94], &index[95], &index[96], &index[97], &index[98], &index[99]  \
    )) \
    // FIX ********************************************************************************************************************************************************************

// particle tracing
void PyParticleTracing::solve()
{
    if (!Util::scene()->sceneSolution()->isSolved())
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
    Util::scene()->refresh();
}

void PyParticleTracing::setInitialVelocity(double x, double y)
{
    Util::config()->particleStartVelocity = Point(x, y);
    Util::scene()->refresh();
}

void PyParticleTracing::setParticleMass(double mass)
{
    if (mass <= 0.0)
        throw out_of_range(QObject::tr("Mass must be positive.").toStdString());

    Util::config()->particleMass = mass;
    Util::scene()->refresh();
}

void PyParticleTracing::setParticleCharge(double charge)
{
    Util::config()->particleConstant = charge;
    Util::scene()->refresh();
}

void PyParticleTracing::setDragForceDensity(double rho)
{
    if (rho < 0.0)
        throw out_of_range(QObject::tr("Density cannot be negative.").toStdString());

    Util::config()->particleDragDensity = rho;
    Util::scene()->refresh();
}

void PyParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area cannot be negative.").toStdString());

    Util::config()->particleDragReferenceArea = area;
    Util::scene()->refresh();
}

void PyParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient cannot be negative.").toStdString());

    Util::config()->particleDragCoefficient = coeff;
    Util::scene()->refresh();
}

void PyParticleTracing::setIncludeGravitation(int include)
{
    Util::config()->particleIncludeGravitation = include;
    Util::scene()->refresh();
}

void PyParticleTracing::setTerminateOnDifferentMaterial(int terminate)
{
    Util::config()->particleTerminateOnDifferentMaterial = terminate;
    Util::scene()->refresh();
}

void PyParticleTracing::setMaximumTolerance(double tolerance)
{
    if (tolerance < 0.0)
       throw out_of_range(QObject::tr("Tolerance cannot be negative.").toStdString());

    Util::config()->particleMaximumRelativeError = tolerance;
    Util::scene()->refresh();
}

void PyParticleTracing::setMaximumSteps(int steps)
{
    if (steps < 0.0)
        throw out_of_range(QObject::tr("Maximum steps cannot be negative.").toStdString());

    Util::config()->particleMaximumSteps = steps;
    Util::scene()->refresh();
}


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

Solution *pythonSolutionObject()
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

// newdocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition)
void pythonNewDocument(char *name, char *type, char *physicfield,
                       int numberofrefinements, int polynomialorder, char *adaptivitytype,
                       double adaptivitysteps, double adaptivitytolerance,
                       double frequency,
                       char *analysistype, double timestep, double totaltime, double initialcondition)
{
    logMessage("pythonNewDocument()");

    Util::scene()->clear();
    Util::scene()->problemInfo()->name = QString(name);

    // type
    Util::scene()->problemInfo()->problemType = problemTypeFromStringKey(QString(type));
    if (Util::scene()->problemInfo()->problemType == ProblemType_Undefined)
        throw invalid_argument(QObject::tr("Problem type '%1' is not implemented.").arg(QString(type)).toStdString());

    // physicfield
    PhysicField physicField = physicFieldFromStringKey(QString(physicfield));
    if (physicField != PhysicField_Undefined)
        Util::scene()->problemInfo()->setHermes(hermesFieldFactory(physicField));
    else
        throw invalid_argument(QObject::tr("Physic field '%1' is not implemented.").arg(QString(physicfield)).toStdString());

    // numberofrefinements
    if (numberofrefinements >= 0)
        Util::scene()->problemInfo()->numberOfRefinements = numberofrefinements;
    else
        throw out_of_range(QObject::tr("Number of refinements '%1' is out of range.").arg(numberofrefinements).toStdString());

    // polynomialorder
    if (polynomialorder >= 1 && polynomialorder <= 10)
        Util::scene()->problemInfo()->polynomialOrder = polynomialorder;
    else
        throw out_of_range(QObject::tr("Polynomial order '%1' is out of range.").arg(polynomialorder).toStdString());

    // adaptivitytype
    Util::scene()->problemInfo()->adaptivityType = adaptivityTypeFromStringKey(QString(adaptivitytype));
    if (Util::scene()->problemInfo()->adaptivityType == AdaptivityType_Undefined)
        throw invalid_argument(QObject::tr("Adaptivity type '%1' is not implemented.").arg(QString(adaptivitytype)).toStdString());

    // adaptivitysteps
    if (adaptivitysteps >= 0)
        Util::scene()->problemInfo()->adaptivitySteps = adaptivitysteps;
    else
        throw out_of_range(QObject::tr("Adaptivity step '%1' is out of range.").arg(adaptivitysteps).toStdString());

    // adaptivitytolerance
    if (adaptivitytolerance >= 0)
        Util::scene()->problemInfo()->adaptivityTolerance = adaptivitytolerance;
    else
        throw out_of_range(QObject::tr("Adaptivity tolerance '%1' is out of range.").arg(adaptivitytolerance).toStdString());

    // frequency
    if (Util::scene()->problemInfo()->hermes()->hasHarmonic())
    {
        if (frequency >= 0)
            Util::scene()->problemInfo()->frequency = frequency;
        else
            throw invalid_argument(QObject::tr("The frequency can not be used for this problem.").toStdString());
    }

    // analysis type
    Util::scene()->problemInfo()->analysisType = analysisTypeFromStringKey(QString(analysistype));
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Undefined)
        throw invalid_argument(QObject::tr("Analysis type '%1' is not implemented").arg(QString(adaptivitytype)).toStdString());

    // analysis type
    Util::scene()->problemInfo()->analysisType = analysisTypeFromStringKey(QString(analysistype));
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Undefined)
        throw invalid_argument(QObject::tr("Analysis type '%1' is not implemented").arg(QString(adaptivitytype)).toStdString());

    // transient timestep
    if (timestep > 0)
        Util::scene()->problemInfo()->timeStep = Value(QString::number(timestep));
    else if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        throw out_of_range(QObject::tr("Time step must be positive.").toStdString());

    // transient timetotal
    if (totaltime > 0)
        Util::scene()->problemInfo()->timeTotal = Value(QString::number(totaltime));
    else if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        throw out_of_range(QObject::tr("Total time must be positive.").toStdString());

    // transient initial condition
    Util::scene()->problemInfo()->initialCondition = Value(QString::number(initialcondition));

    // invalidate
    sceneView()->doDefaultValues();
    Util::scene()->refresh();
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

    if (index < 0 || index >= Util::scene()->nodes.count())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());
    Util::scene()->removeNode(Util::scene()->nodes[index]);
}

void pythonDeleteNodePoint(double x, double y)
{
    logMessage("pythonDeleteNodePoint()");

    Util::scene()->removeNode(Util::scene()->getNode(Point(x, y)));
}

// addedge(x1, y1, x2, y2, angle = 0, marker = "none")
void pythonAddEdge(double x1, double y1, double x2, double y2, double angle, char *marker)
{
    logMessage("pythonAddEdge()");

    if (angle > 90.0 || angle < 0.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());

    SceneBoundary *boundary = Util::scene()->getBoundary(QString(marker));
    if (!boundary)
        throw invalid_argument(QObject::tr("Boundary '%1' is not defined.").arg(marker).toStdString());

    // start node
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
    // end node
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

    // FIXME 0 -> variable
    Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, boundary, angle, 0));
}

void pythonDeleteEdge(int index)
{
    logMessage("pythonDeleteEdge()");

    if (index < 0 || index >= Util::scene()->edges.count())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());
    Util::scene()->removeEdge(Util::scene()->edges[index]);
}

void pythonDeleteEdgePoint(double x1, double y1, double x2, double y2, double angle)
{
    logMessage("pythonDeleteEdgePoint()");

    Util::scene()->removeEdge(Util::scene()->getEdge(Point(x1, y1), Point(x2, y2), angle));
}

// addlabel(x, y, area = 0, marker = "none", polynomialorder = 0)
void pythonAddLabel(double x, double y, double area, int polynomialOrder, char *marker)
{
    logMessage("pythonAddLabel()");

    SceneMaterial *material = Util::scene()->getMaterial(QString(marker));
    if (!material)
        throw invalid_argument(QObject::tr("Material '%1' is not defined.").arg(marker).toStdString());

    Util::scene()->addLabel(new SceneLabel(Point(x, y), material, area, polynomialOrder));
}

void pythonDeleteLabel(int index)
{
    logMessage("pythonDeleteLabel()");

    if (index < 0 || index >= Util::scene()->labels.count())
        throw out_of_range(QObject::tr("Index '%1' is out of range.").arg(index).toStdString());
    Util::scene()->removeLabel(Util::scene()->labels[index]);
}

void pythonDeleteLabelPoint(double x, double y)
{
    logMessage("pythonDeleteLabelPoint()");

    Util::scene()->removeLabel(Util::scene()->getLabel(Point(x, y)));
}

// addboundary(name, type, value, ...)
static PyObject *pythonAddBoundary(PyObject *self, PyObject *args)
{
    logMessage("pythonAddBoundary()");

    SceneBoundary *marker = Util::scene()->problemInfo()->hermes()->newBoundary(self, args);
    if (marker)
    {
        Util::scene()->addBoundary(marker);
        Py_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker already exists.").toStdString().c_str());
        return NULL;
    }
}

// modifyBoundary(name, type, value, ...)
static PyObject *pythonModifyBoundary(PyObject *self, PyObject *args)
{
    logMessage("pythonModifyBoundary()");

    if (Util::scene()->problemInfo()->hermes()->modifyBoundary(self,args))
        Py_RETURN_NONE;
    else
        return NULL;
}

// addmaterial(name, type, value, ...)
static PyObject *pythonAddMaterial(PyObject *self, PyObject *args)
{
    logMessage("pythonAddMaterial()");

    SceneMaterial *marker = Util::scene()->problemInfo()->hermes()->newMaterial(self, args);
    if (marker)
    {
        Util::scene()->addMaterial(marker);
        Py_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker already exists.").toStdString().c_str());
        return NULL;
    }
}

// modifymaterial(name, type, value, ...)
static PyObject *pythonModifyMaterial(PyObject *self, PyObject *args)
{
    logMessage("pythonModifyMaterial()");

    if (Util::scene()->problemInfo()->hermes()->modifyMaterial(self, args))
        Py_RETURN_NONE;
    else
        return NULL;
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

// selectnode(index, ...)
static PyObject *pythonSelectNode(PyObject *self, PyObject *args)
{
    logMessage("pythonSelectNode()");

    python_int_array()
    {
        sceneView()->actSceneModeEdge->trigger();
        Util::scene()->selectNone();

        for (int i = 0; i < count; i++)
        {
            if (index[i] == INT_MIN)
                continue;
            if ((index[i] >= 0) && index[i] < Util::scene()->nodes.count())
            {
                Util::scene()->nodes[index[i]]->isSelected = true;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Node index must be between 0 and '%1'.").arg(Util::scene()->nodes.count()-1).toStdString().c_str());
                return NULL;
            }
        }
        sceneView()->doInvalidated();
        Py_RETURN_NONE;
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

// selectedge(index, ...)
static PyObject *pythonSelectEdge(PyObject *self, PyObject *args)
{
    logMessage("pythonSelectEdge()");

    python_int_array()
    {
        sceneView()->actSceneModeEdge->trigger();
        Util::scene()->selectNone();

        for (int i = 0; i < count; i++)
        {
            if (index[i] == INT_MIN)
                continue;
            if ((index[i] >= 0) && index[i] < Util::scene()->edges.count())
            {
                Util::scene()->edges[index[i]]->isSelected = true;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges.count()-1).toStdString().c_str());
                return NULL;
            }
        }
        sceneView()->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
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

// selectlabel(index, ...)
static PyObject *pythonSelectLabel(PyObject *self, PyObject *args)
{
    logMessage("pythonSelectLabel()");

    python_int_array()
    {
        sceneView()->actSceneModeLabel->trigger();
        Util::scene()->selectNone();

        for (int i = 0; i < count; i++)
        {
            if (index[i] == INT_MIN)
                continue;
            if ((index[i] >= 0) && index[i] < Util::scene()->labels.count())
            {
                Util::scene()->labels[index[i]]->isSelected = true;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->labels.count()-1).toStdString().c_str());
                return NULL;
            }
        }
        sceneView()->doInvalidated();
        Py_RETURN_NONE;
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
            Point point(x, y);
            LocalPointValue *localPointValue = Util::scene()->problemInfo()->hermes()->localPointValue(point);

            QStringList headers = Util::scene()->problemInfo()->hermes()->localPointValueHeader();
            QStringList variables = localPointValue->variables();

            PyObject *dict = PyDict_New();
            for (int i = 0; i < variables.length(); i++)
                PyDict_SetItemString(dict, headers[i].toStdString().c_str(), Py_BuildValue("d", QString(variables[i]).toDouble()));

            delete localPointValue;

            return dict;
        }
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
    }
    return NULL;
}

// result = surfaceintegral(index, ...)
static PyObject *pythonSurfaceIntegral(PyObject *self, PyObject *args)
{
    logMessage("pythonSurfaceIntegral()");

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // set mode
        sceneView()->actSceneModePostprocessor->trigger();
        sceneView()->actPostprocessorModeSurfaceIntegral->trigger();
        Util::scene()->selectNone();

        python_int_array()
        {
            for (int i = 0; i < count; i++)
            {
                if (index[i] == INT_MIN)
                    continue;
                if ((index[i] >= 0) && index[i] < Util::scene()->edges.count())
                {
                    Util::scene()->edges[index[i]]->isSelected = true;
                }
                else
                {
                    PyErr_SetString(PyExc_RuntimeError, QObject::tr("Edge index must be between 0 and '%1'.").arg(Util::scene()->edges.count()-1).toStdString().c_str());
                    return NULL;
                }
            }

            SurfaceIntegralValue *surfaceIntegral = Util::scene()->problemInfo()->hermes()->surfaceIntegralValue();

            QStringList headers = Util::scene()->problemInfo()->hermes()->surfaceIntegralValueHeader();
            QStringList variables = surfaceIntegral->variables();

            PyObject *dict = PyDict_New();
            for (int i = 0; i < variables.length(); i++)
                PyDict_SetItemString(dict, headers[i].toStdString().c_str(), Py_BuildValue("d", QString(variables[i]).toDouble()));

            delete surfaceIntegral;

            return dict;
        }
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
    }
    return NULL;
}

// result = volumeintegral(index, ...)
static PyObject *pythonVolumeIntegral(PyObject *self, PyObject *args)
{
    logMessage("pythonVolumeIntegral()");

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // set mode
        sceneView()->actSceneModePostprocessor->trigger();
        sceneView()->actPostprocessorModeVolumeIntegral->trigger();
        Util::scene()->selectNone();

        python_int_array()
        {
            for (int i = 0; i < count; i++)
            {
                if (index[i] == INT_MIN)
                    continue;
                if ((index[i] >= 0) && index[i] < Util::scene()->labels.count())
                {
                    Util::scene()->labels[index[i]]->isSelected = true;
                }
                else
                {
                    PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label index must be between 0 and '%1'.").arg(Util::scene()->labels.count()-1).toStdString().c_str());
                    return NULL;
                }
            }

            VolumeIntegralValue *volumeIntegral = Util::scene()->problemInfo()->hermes()->volumeIntegralValue();

            QStringList headers = Util::scene()->problemInfo()->hermes()->volumeIntegralValueHeader();
            QStringList variables = volumeIntegral->variables();

            PyObject *dict = PyDict_New();
            for (int i = 0; i < variables.length(); i++)
                PyDict_SetItemString(dict, headers[i].toStdString().c_str(), Py_BuildValue("d", QString(variables[i]).toDouble()));

            delete volumeIntegral;

            return dict;
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
    sceneView()->sceneViewSettings().scalarPhysicFieldVariable = physicFieldVariableFromStringKey(QString(variable));
    if (sceneView()->sceneViewSettings().scalarPhysicFieldVariable == PhysicFieldVariable_Undefined)
        throw invalid_argument(QObject::tr("Physic field variable '%1' is not implemented.").arg(QString(variable)).toStdString());
    if (!Util::scene()->problemInfo()->hermes()->physicFieldVariableCheck(sceneView()->sceneViewSettings().scalarPhysicFieldVariable))
        throw invalid_argument(QObject::tr("Physic field variable '%1' cannot be used with this field.").arg(QString(variable)).toStdString());

    // variable component
    sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = physicFieldVariableCompFromStringKey(QString(component));
    if (sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp == PhysicFieldVariableComp_Undefined)
        throw invalid_argument(QObject::tr("Physic field variable component '%1' is not implemented.").arg(QString(component)).toStdString());
    if ((isPhysicFieldVariableScalar(sceneView()->sceneViewSettings().scalarPhysicFieldVariable)) &&
            (sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp != PhysicFieldVariableComp_Scalar))
        throw invalid_argument(QObject::tr("Physic field variable is scalar variable.").toStdString());

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

// showparticletracing(show = {True, False})
void pythonShowParticleTracing(bool show)
{
    logMessage("pythonShowParticleTracing()");

    sceneView()->sceneViewSettings().showParticleTracing = show;
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
    logMessage("pythonSetTimeStep()");

    if (Util::scene()->sceneSolution()->isSolved())
        sceneView()->actSceneModePostprocessor->trigger();
    else
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    if (Util::scene()->problemInfo()->analysisType != AnalysisType_Transient)
        throw invalid_argument(QObject::tr("Solved problem is not transient.").toStdString());

    if ((timestep < 0) || (timestep > Util::scene()->sceneSolution()->timeStepCount()))
        throw out_of_range(QObject::tr("Time step must be between 0 and %1.").arg(Util::scene()->sceneSolution()->timeStepCount()).toStdString());

    Util::scene()->sceneSolution()->setTimeStep(timestep, false);
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
    {"selectnode", pythonSelectNode, METH_VARARGS, "selectnode(index, ...)"},
    {"selectedge", pythonSelectEdge, METH_VARARGS, "selectedge(index, ...)"},
    {"selectlabel", pythonSelectLabel, METH_VARARGS, "selectlabel(index, ...)"},
    {"pointresult", pythonPointResult, METH_VARARGS, "pointresult(x, y)"},
    {"volumeintegral", pythonVolumeIntegral, METH_VARARGS, "volumeintegral(index, ...)"},
    {"surfaceintegral", pythonSurfaceIntegral, METH_VARARGS, "surfaceintegral(index, ...)"},
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
    if (!Util::scene()->problemInfo()->scriptStartup.isEmpty())
        PyRun_String(Util::scene()->problemInfo()->scriptStartup.toStdString().c_str(), Py_file_input, m_dict, m_dict);
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
