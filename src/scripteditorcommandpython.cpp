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

#include "scripteditorcommandpython.h"

#include <Python.h>
#include "python/agros2d.c"

#include "util.h"
#include "scene.h"
#include "sceneview.h"
#include "scenemarker.h"
#include "scripteditordialog.h"

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

    QApplication::exit(0);
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

    if (angle > 180.0 || angle < 0.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());

    SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(QString(marker));
    if (!edgeMarker)
        throw invalid_argument(QObject::tr("Marker '%1' is not defined.").arg(marker).toStdString());

    // start node
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
    // end node
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

    // FIXME 0 -> variable
    Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, edgeMarker, angle, 0));
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

    SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(QString(marker));
    if (!labelMarker)
        throw invalid_argument(QObject::tr("Marker '%1' is not defined.").arg(marker).toStdString());

    Util::scene()->addLabel(new SceneLabel(Point(x, y), labelMarker, area, polynomialOrder));
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

    SceneEdgeMarker *marker = Util::scene()->problemInfo()->hermes()->newEdgeMarker(self, args);
    if (marker)
    {
        Util::scene()->addEdgeMarker(marker);
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

    if (Util::scene()->problemInfo()->hermes()->modifyEdgeMarker(self,args))
        Py_RETURN_NONE;
    else
        return NULL;
}

// addmaterial(name, type, value, ...)
static PyObject *pythonAddMaterial(PyObject *self, PyObject *args)
{
    logMessage("pythonAddMaterial()");

    SceneLabelMarker *marker = Util::scene()->problemInfo()->hermes()->newLabelMarker(self, args);
    if (marker)
    {
        Util::scene()->addLabelMarker(marker);
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

    if (Util::scene()->problemInfo()->hermes()->modifyLabelMarker(self, args))
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
void pythonShowScalar(char *type, char *variable, char *component, int rangemin, int rangemax)
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
    if (Util::scene()->problemInfo()->hermes()->physicFieldVariableCheck(sceneView()->sceneViewSettings().scalarPhysicFieldVariable))
        throw invalid_argument(QObject::tr("Physic field variable '%1' cannot be used with this field.").arg(QString(variable)).toStdString());

    // variable component
    sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = physicFieldVariableCompFromStringKey(QString(component));
    if (sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp == PhysicFieldVariableComp_Undefined)
        throw invalid_argument(QObject::tr("Physic field variable component '%1' is not implemented.").arg(QString(component)).toStdString());
    if ((isPhysicFieldVariableScalar(sceneView()->sceneViewSettings().scalarPhysicFieldVariable)) &&
        (sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp != PhysicFieldVariableComp_Scalar))
        throw invalid_argument(QObject::tr("Physic field variable is scalar variable.").toStdString());

    // range
    if (rangemin != INT_MIN)
    {
        sceneView()->sceneViewSettings().scalarRangeAuto = false;
        sceneView()->sceneViewSettings().scalarRangeMin = rangemin;
    }
    else
    {
        sceneView()->sceneViewSettings().scalarRangeAuto = true;
    }
    if (rangemax != INT_MIN)
        sceneView()->sceneViewSettings().scalarRangeMax = rangemax;

    sceneView()->doInvalidated();
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
    sceneView()->doInvalidated();
}

// showvectors(show = {True, False})
void pythonShowVectors(bool show)
{
    logMessage("pythonShowVectors()");

    sceneView()->sceneViewSettings().showVectors = show;
    sceneView()->doInvalidated();
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

    Util::scene()->sceneSolution()->setTimeStep(timestep);
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

// print stdout
PyObject* pythonCaptureStdout(PyObject* self, PyObject* pArgs)
{
    logMessage("pythonCaptureStdout()");

    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->showMessage(QString(str) + "\n");
        Py_RETURN_NONE;
    }
    return NULL;
}

static PyMethodDef pythonMethods[] =
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
    {"capturestdout", pythonCaptureStdout, METH_VARARGS, "stdout"},
    {NULL, NULL, 0, NULL}
};

// ******************************************************************************************************************************************************

PythonEngine::PythonEngine()
{
    logMessage("PythonEngine::PythonEngine()");

    m_isRunning = false;
    m_stdOut = "";

    // connect stdout
    connect(this, SIGNAL(printStdout(QString)), this, SLOT(doPrintStdout(QString)));

    // init python
    Py_Initialize();

    // read functions
    m_functions = readFileContent(datadir() + "/functions.py");

    m_dict = PyDict_New();
    PyDict_SetItemString(m_dict, "__builtins__", PyEval_GetBuiltins());

    // init agros cython extensions
    initagros2d();
    // agros2d file
    Py_InitModule("agros2file", pythonMethods);

    // stdout
    PyRun_String(QString("agrosstdout = \"" + tempProblemDir() + "/stdout.txt" + "\"").toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // functions.py
    PyRun_String(m_functions.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

PythonEngine::~PythonEngine()
{
    logMessage("PythonEngine::~PythonEngine()");

    // finalize and garbage python
    Py_DECREF(m_dict);

    if (Py_IsInitialized())
        Py_Finalize();
}

void PythonEngine::showMessage(const QString &message)
{
    logMessage("PythonEngine::showMessage()");

    emit printStdout(message);
}

void PythonEngine::doPrintStdout(const QString &message)
{
    logMessage("PythonEngine::doPrintStdout()");

    m_stdOut.append(message);
    QApplication::processEvents();
}

void PythonEngine::runPythonHeader()
{
    logMessage("PythonEngine::runPythonHeader()");

    // global script
    if (!Util::config()->globalScript.isEmpty())
        PyRun_String(Util::config()->globalScript.toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // startup script
    if (!Util::scene()->problemInfo()->scriptStartup.isEmpty())
        PyRun_String(Util::scene()->problemInfo()->scriptStartup.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

ScriptResult PythonEngine::runPythonScript(const QString &script, const QString &fileName)
{
    logMessage("PythonEngine::runPythonScript()");

    m_isRunning = true;
    m_stdOut = "";

    runPythonHeader();

    PyObject *output = NULL;
    if (QFile::exists(fileName))
    {
        // compile
        PyObject *code = Py_CompileString(QString("from os import chdir \nchdir(u'" + QFileInfo(fileName).absolutePath() + "')").toStdString().c_str(), "", Py_file_input);
        // run
        if (code) output = PyEval_EvalCode((PyCodeObject *) code, m_dict, m_dict);
    }
    // compile
    PyObject *code = Py_CompileString(script.toStdString().c_str(), fileName.toStdString().c_str(), Py_file_input);
    // run
    if (code) output = PyEval_EvalCode((PyCodeObject *) code, m_dict, m_dict);

    ScriptResult scriptResult;
    if (output)
    {
        /*
        if (output == Py_None)
        {
            cout << "none" << endl;
        }
        if (PyInt_Check(output))
        {
            cout << "int" << endl;
        }
        if (PyFloat_Check(output))
        {
            cout << "float" << endl;
        }
        if (PyString_Check(output))
        {
            cout << "string" << endl;
        }
        if (PyLong_Check(output))
        {
            cout << "long" << endl;
        }
        if (PyNumber_Check(output))
        {
            cout << "number" << endl;
        }
        if (PyBool_Check(output))
        {
            cout << "bool" << endl;
        }
        */
        scriptResult.isError = false;
        scriptResult.text = m_stdOut;
    }
    else
    {
        scriptResult = parseError();
    }
    Py_DECREF(Py_None);

    m_isRunning = false;
    Util::scene()->refresh();
    sceneView()->doInvalidated();

    return scriptResult;
}

ExpressionResult PythonEngine::runPythonExpression(const QString &expression)
{
    logMessage("PythonEngine::runPythonExpression()");

    runPythonHeader();

    QString exp = "result = " + expression;

    PyObject *output = PyRun_String(exp.toStdString().c_str(), Py_file_input, m_dict, m_dict);

    ExpressionResult expressionResult;
    if (output)
    {
        PyObject *type = NULL, *value = NULL, *traceback = NULL, *str = NULL;
        PyErr_Fetch(&type, &value, &traceback);

        if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
        {
            Py_INCREF(type);

            expressionResult.error = PyString_AsString(str);
            if (type) Py_DECREF(type);
            if (str) Py_DECREF(str);
        }
        else
        {
            // parse result
            PyObject *result = PyDict_GetItemString(m_dict, "result");
            if (result)
            {
                Py_INCREF(result);
                PyArg_Parse(result, "d", &expressionResult.value);
                if (fabs(expressionResult.value) < EPS_ZERO)
                    expressionResult.value = 0.0;
                Py_DECREF(result);
            }
        }
    }
    else
    {
        expressionResult.error = parseError().text;
    }
    Py_DECREF(Py_None);

    return expressionResult;
}

ScriptResult PythonEngine::parseError()
{
    logMessage("PythonEngine::parseError()");

    // error
    ScriptResult error;
    error.isError = true;

    PyObject *type = NULL, *value = NULL, *traceback = NULL, *str = NULL;
    PyErr_Fetch(&type, &value, &traceback);

    if (traceback)
    {
        PyTracebackObject *object = (PyTracebackObject *) traceback;
        error.text.append(QString("Line %1: ").arg(object->tb_lineno));
        error.line = object->tb_lineno;
        Py_DECREF(traceback);
    }

    if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
    {
        Py_INCREF(type);
        error.text.append("\n");
        error.text.append(PyString_AsString(str));
        if (type) Py_DECREF(type);
        if (str) Py_DECREF(str);
    }
    else
    {
        error.text.append("\n");
        error.text.append("<unknown exception type> ");
    }

    if (value != NULL && (str = PyObject_Str(value)) != NULL && (PyString_Check(str)))
    {
        Py_INCREF(value);
        error.text.append("\n");
        error.text.append(PyString_AsString(value));
        if (value) Py_DECREF(value);
        if (str) Py_DECREF(str);
    }
    else
    {
        error.text.append("\n");
        error.text.append("<unknown exception date> ");
    }

    PyErr_Clear();

    return error;
}
