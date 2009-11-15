#include "scripteditorcommandpython.h"

#include <Python.h>
#include "python/agros2d.c"

#include "util.h"
#include "scene.h"
#include "scenemarker.h"
#include "scripteditordialog.h"

SceneView *sceneView = NULL;

// FIX *****************************************************************************************************************************************************************************
// Terible, is it possible to write this code better???
#define python_int_array() \
const int count = 60; \
                  int index[count]; \
                  for (int i = 0; i < count; i++) \
                  index[i] = INT_MIN; \
                             if (PyArg_ParseTuple(args, "i|iiiiiiiiiiiiiiiiiiiiiiiiiiiii", \
                                                  &index[ 0], &index[ 1], &index[ 2], &index[ 3], &index[ 4], &index[ 5], &index[ 6], &index[ 7], &index[ 8], &index[ 9], \
                                                  &index[10], &index[11], &index[12], &index[13], &index[14], &index[15], &index[16], &index[17], &index[18], &index[19], \
                                                  &index[20], &index[21], &index[22], &index[23], &index[24], &index[25], &index[26], &index[27], &index[28], &index[29], \
                                                  &index[30], &index[31], &index[32], &index[33], &index[34], &index[35], &index[36], &index[37], &index[38], &index[39], \
                                                  &index[40], &index[41], &index[42], &index[43], &index[44], &index[45], &index[46], &index[47], &index[48], &index[49], \
                                                  &index[50], &index[51], &index[52], &index[53], &index[54], &index[55], &index[56], &index[57], &index[58], &index[59])) \
                             // FIX *****************************************************************************************************************************************************************************

                             // version()
                             static PyObject *pythonVersion(PyObject *self, PyObject *args)
{    
    return Py_BuildValue("s", QApplication::applicationVersion().toStdString());
}

// message(string)
void pythonMessage(char *str)
{
    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Message"), QString(str));
}

// variable = input(string)
char *pythonInput(char *str)
{
    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString(str));
    return const_cast<char*>(text.toStdString().c_str());
}

// version()
char *pythonVersion()
{
    return const_cast<char*>(QApplication::applicationVersion().toStdString().c_str());
}

// meshfilename()
char *pythonMeshFileName()
{
    if (Util::scene()->sceneSolution()->isMeshed())
    {
        return const_cast<char*>(QString(tempProblemFileName() + ".mesh").toStdString().c_str());
    }
    else
        throw out_of_range(QObject::tr("Problem is not meshed.").toStdString());
}

// solutionfilename()
char *pythonSolutionFileName()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        char *fileName = const_cast<char*>(QString(tempProblemFileName() + ".sln").toStdString().c_str());
        Util::scene()->sceneSolution()->sln()->save(fileName);
        return fileName;
    }
    else
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
}

Solution *pythonSolutionObject()
{
    if (Util::scene()->sceneSolution()->isSolved())
        return Util::scene()->sceneSolution()->sln();
    else
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());
}

// quit()
void pythonQuit()
{
    QApplication::exit(0);
}

// newdocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition)
void pythonNewDocument(char *name, char *type, char *physicfield,
                       int numberofrefinements, int polynomialorder, char *adaptivitytype,
                       double adaptivitysteps, double adaptivitytolerance,
                       double frequency,
                       char *analysistype, double timestep, double totaltime, double initialcondition)
{   
    Util::scene()->clear();

    Util::scene()->problemInfo()->name = QString(name);

    // type
    Util::scene()->problemInfo()->problemType = problemTypeFromStringKey(QString(type));
    if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_UNDEFINED)
        throw out_of_range(QObject::tr("Problem type '%1' is not implemented.").arg(QString(type)).toStdString());

    // physicfield
    PhysicField physicField = physicFieldFromStringKey(QString(physicfield));
    if (physicField != PHYSICFIELD_UNDEFINED)
        Util::scene()->problemInfo()->setHermes(hermesFieldFactory(physicField));
    else
        throw out_of_range(QObject::tr("Physic field '%1' is not implemented.").arg(QString(physicfield)).toStdString());

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
    if (Util::scene()->problemInfo()->adaptivityType == ADAPTIVITYTYPE_UNDEFINED)
        throw out_of_range(QObject::tr("Adaptivity type '%1' is not suported.").arg(QString(adaptivitytype)).toStdString());

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
    if (Util::scene()->problemInfo()->physicField() == PHYSICFIELD_HARMONICMAGNETIC)
    {
        if (frequency >= 0)
            Util::scene()->problemInfo()->frequency = frequency;
        else
            throw out_of_range(QObject::tr("Frequency can be used only for harmonic magnetic problems.").toStdString());
    }

    // analysis type
    Util::scene()->problemInfo()->analysisType = analysisTypeFromStringKey(QString(analysistype));
    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_UNDEFINED)
        throw out_of_range(QObject::tr("Analysis type '%1' is not suported").arg(QString(adaptivitytype)).toStdString());

    // transient timestep
    if (timestep > 0)
        Util::scene()->problemInfo()->timeStep = timestep;
    else if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
        throw out_of_range(QObject::tr("Time step must be positive.").toStdString());

    // transient timetotal
    if (totaltime > 0)
        Util::scene()->problemInfo()->timeTotal = totaltime;
    else if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
        throw out_of_range(QObject::tr("Total time must be positive.").toStdString());

    // transient initial condition
    Util::scene()->problemInfo()->initialCondition = initialcondition;

    // invalidate
    sceneView->doDefaults();
    Util::scene()->refresh();
}

// opendocument(filename)
void pythonOpenDocument(char *str)
{
    Util::scene()->readFromFile(QString(str));
}

// savedocument(filename)
void pythonSaveDocument(char *str)
{
    Util::scene()->writeToFile(QString(str));
}

// addnode(x, y)
void pythonAddNode(double x, double y)
{
    Util::scene()->addNode(new SceneNode(Point(x, y)));
}

// addedge(x1, y1, x2, y2, angle = 0, marker = "none")
void pythonAddEdge(double x1, double y1, double x2, double y2, double angle, char *marker)
{
    SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(QString(marker));
    if (!edgeMarker)
        throw out_of_range(QObject::tr("Marker '%1' is not defined.").arg(marker).toStdString());

    // start node
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
    // end node
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

    Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, edgeMarker, angle));
}

// addlabel(x, y, area = 0, marker = "none")
void pythonAddLabel(double x, double y, double area, char *marker)
{
    SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(QString(marker));
    if (!labelMarker)
        throw out_of_range(QObject::tr("Marker '%1' is not defined.").arg(marker).toStdString());

    Util::scene()->addLabel(new SceneLabel(Point(x, y), labelMarker, area));
}


// addboundary(name, type, value, ...)
static PyObject *pythonAddBoundary(PyObject *self, PyObject *args)
{
    SceneEdgeMarker *marker = Util::scene()->problemInfo()->hermes()->newEdgeMarker(self, args);
    if (marker)
    {
        Util::scene()->addEdgeMarker(marker);
        Py_RETURN_NONE;
    }
    else
    {
        if (!PyErr_Occurred)
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker already exists.").toStdString().c_str());
        return NULL;
    }
}

// modifyBoundary(name, type, value, ...)
static PyObject *pythonModifyBoundary(PyObject *self, PyObject *args)
{
    SceneEdgeMarker *markerNew = Util::scene()->problemInfo()->hermes()->newEdgeMarker(self, args);
    SceneEdgeMarker *markerOld = Util::scene()->getEdgeMarker(markerNew->name);
    if (markerOld)
    {
        Util::scene()->setEdgeMarker(markerNew->name, markerNew);
        Py_RETURN_NONE;
    }
    else
    {
        if (!PyErr_Occurred)
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(markerNew->name).toStdString().c_str());
        delete markerNew;
        return NULL;
    }
}

// addmaterial(name, type, value, ...)
static PyObject *pythonAddMaterial(PyObject *self, PyObject *args)
{
    SceneLabelMarker *marker = Util::scene()->problemInfo()->hermes()->newLabelMarker(self, args);
    if (marker)
    {
        Util::scene()->addLabelMarker(marker);
        Py_RETURN_NONE;
    }
    else
    {
        if (!PyErr_Occurred)
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker already exists.").toStdString().c_str());
        return NULL;
    }
}

// modifymaterial(name, type, value, ...)
static PyObject *pythonModifyMaterial(PyObject *self, PyObject *args)
{
    SceneLabelMarker *markerNew = Util::scene()->problemInfo()->hermes()->newLabelMarker(self, args);
    SceneLabelMarker *markerOld = Util::scene()->getLabelMarker(markerNew->name);
    if (markerOld)
    {
        Util::scene()->setLabelMarker(markerNew->name, markerNew);
        Py_RETURN_NONE;
    }
    else
    {
        if (!PyErr_Occurred)
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker with name '%1' doesn't exists.").arg(markerNew->name).toStdString().c_str());
        delete markerNew;
        return NULL;
    }
}

// selectnone()
void pythonSelectNone()
{
    Util::scene()->selectNone();
}

// selectall()
void pythonSelectAll()
{
    if (sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
    {
        // select volume integral area
        if (sceneView->actPostprocessorModeVolumeIntegral->isChecked())
            Util::scene()->selectAll(SCENEMODE_OPERATE_ON_LABELS);

        // select surface integral area
        if (sceneView->actPostprocessorModeSurfaceIntegral->isChecked())
            Util::scene()->selectAll(SCENEMODE_OPERATE_ON_EDGES);
    }
    else
    {
        Util::scene()->selectAll(sceneView->sceneMode());
    }
    sceneView->doInvalidated();
}

// selectnode(index, ...)
static PyObject *pythonSelectNode(PyObject *self, PyObject *args)
{
    python_int_array()
    {
        sceneView->actSceneModeEdge->trigger();
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
        sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// selectnodepoint(x, y)
void pythonSelectNodePoint(double x, double y)
{
    SceneNode *node = sceneView->findClosestNode(Point(x, y));
    if (node)
    {
        node->isSelected = true;
        sceneView->doInvalidated();
    }
}

// selectedge(index, ...)
static PyObject *pythonSelectEdge(PyObject *self, PyObject *args)
{
    python_int_array()
    {
        sceneView->actSceneModeEdge->trigger();
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
        sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// selectedgepoint(x, y)
void pythonSelectEdgePoint(double x, double y)
{    
    SceneEdge *edge = sceneView->findClosestEdge(Point(x, y));
    if (edge)
    {
        edge->isSelected = true;
        sceneView->doInvalidated();
    }
}

// selectlabel(index, ...)
static PyObject *pythonSelectLabel(PyObject *self, PyObject *args)
{
    python_int_array()
    {
        sceneView->actSceneModeLabel->trigger();
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
        sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// selectlabelpoint(x, y)
void pythonSelectLabelPoint(double x, double y)
{
    SceneLabel *label = sceneView->findClosestLabel(Point(x, y));
    if (label)
    {
        label->isSelected = true;
        sceneView->doInvalidated();
    }
}

// rotateselection(x, y, angle, copy = {true, false})
void pythonRotateSelection(double x, double y, double angle, bool copy)
{
    Util::scene()->transformRotate(Point(x, y), angle, copy);
    sceneView->doInvalidated();
}

// scaleselection(x, y, scale, copy = {true, false})
void pythonScaleSelection(double x, double y, double scale, bool copy)
{
    Util::scene()->transformScale(Point(x, y), scale, copy);
    sceneView->doInvalidated();
}

// moveselection(dx, dy, copy = {true, false})
void pythonMoveSelection(double dx, double dy, bool copy)
{
    Util::scene()->transformTranslate(Point(dx, dy), copy);
    sceneView->doInvalidated();
}

// deleteselection()
void pythonDeleteSelection()
{
    Util::scene()->deleteSelected();
}

// mesh()
void pythonMesh()
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH);
    Util::scene()->refresh();
}

// solve()
void pythonSolve()
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH_AND_SOLVE);
    Util::scene()->refresh();
    sceneView->actSceneModePostprocessor->trigger();
}

// zoombestfit()
void pythonZoomBestFit()
{
    sceneView->doZoomBestFit();
}

// zoomin()
void pythonZoomIn()
{
    sceneView->doZoomIn();
}

// zoomout()
void pythonZoomOut()
{
    sceneView->doZoomOut();
}

// zoomregion(x1, y1, x2, y2)
void pythonZoomRegion(double x1, double y1, double x2, double y2)
{
    sceneView->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

// mode(mode = {"node", "edge", "label", "postprocessor"})
void pythonMode(char *str)
{
    if (QString(str) == "node")
        sceneView->actSceneModeNode->trigger();
    else if (QString(str) == "edge")
        sceneView->actSceneModeEdge->trigger();
    else if (QString(str) == "label")
        sceneView->actSceneModeLabel->trigger();
    else if (QString(str) == "postprocessor")
        if (Util::scene()->sceneSolution()->isSolved())
            sceneView->actSceneModePostprocessor->trigger();
    else
        throw out_of_range(QObject::tr("Mode '%1' is not implemented.").arg(QString(str)).toStdString());

    sceneView->doInvalidated();
}

// postprocessormode(mode = {"point", "surface", "volume"})
void pythonPostprocessorMode(char *str)
{
    if (Util::scene()->sceneSolution()->isSolved())
        sceneView->actSceneModePostprocessor->trigger();
    else
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());

    if (QString(str) == "point")
        sceneView->actPostprocessorModeLocalPointValue->trigger();
    else if (QString(str) == "surface")
        sceneView->actPostprocessorModeSurfaceIntegral->trigger();
    else if (QString(str) == "volume")
        sceneView->actPostprocessorModeVolumeIntegral->trigger();
    else
        throw out_of_range(QObject::tr("Mode '%1' is not implemented.").arg(QString(str)).toStdString());

    sceneView->doInvalidated();
}

// result = pointresult(x, y)
static PyObject *pythonPointResult(PyObject *self, PyObject *args)
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView->actSceneModePostprocessor->trigger();
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
        return NULL;
    }

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

    return NULL;
}

// result = surfaceintegral(index, ...)
static PyObject *pythonSurfaceIntegral(PyObject *self, PyObject *args)
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView->actSceneModePostprocessor->trigger();
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
        return NULL;
    }

    // set mode
    sceneView->actSceneModePostprocessor->trigger();
    sceneView->actPostprocessorModeSurfaceIntegral->trigger();

    python_int_array()
    {
        for (int i = 0; i < count; i++)
        {
            sceneView->actSceneModeEdge->trigger();
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
    return NULL;
}

// result = volumeintegral(index, ...)
static PyObject *pythonVolumeIntegral(PyObject *self, PyObject *args)
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView->actSceneModePostprocessor->trigger();
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
        return NULL;
    }

    // set mode
    sceneView->actSceneModePostprocessor->trigger();
    sceneView->actPostprocessorModeVolumeIntegral->trigger();

    python_int_array()
    {
        sceneView->actSceneModeLabel->trigger();
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
    return NULL;
}

// showscalar(type = { "none", "scalar", "scalar3d", "order" }, variable, component, rangemin, rangemax)
void pythonShowScalar(char *type, char *variable, char *component, int rangemin, int rangemax)
{
    // type
    SceneViewPostprocessorShow postprocessorShow = sceneViewPostprocessorShowFromStringKey(QString(type));
    if (postprocessorShow != SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED)
        sceneView->sceneViewSettings().postprocessorShow = postprocessorShow;
    else
        throw out_of_range(QObject::tr("View type '%1' is not implemented.").arg(QString(type)).toStdString());

    // variable
    sceneView->sceneViewSettings().scalarPhysicFieldVariable = physicFieldVariableFromStringKey(QString(variable));
    if (sceneView->sceneViewSettings().scalarPhysicFieldVariable == PHYSICFIELDVARIABLE_UNDEFINED)
        throw out_of_range(QObject::tr("Physic field variable '%1' is not defined.").arg(QString(variable)).toStdString());
    if (Util::scene()->problemInfo()->hermes()->physicFieldVariableCheck(sceneView->sceneViewSettings().scalarPhysicFieldVariable))
        throw out_of_range(QObject::tr("Physic field variable '%1' cannot be used with this field.").arg(QString(variable)).toStdString());

    // variable component
    sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = physicFieldVariableCompFromStringKey(QString(component));
    if (sceneView->sceneViewSettings().scalarPhysicFieldVariableComp == PHYSICFIELDVARIABLECOMP_UNDEFINED)
        throw out_of_range(QObject::tr("Physic field variable component '%1' is not implemented.").arg(QString(component)).toStdString());
    if ((isPhysicFieldVariableScalar(sceneView->sceneViewSettings().scalarPhysicFieldVariable)) &&
        (sceneView->sceneViewSettings().scalarPhysicFieldVariableComp != PHYSICFIELDVARIABLECOMP_SCALAR))
        throw out_of_range(QObject::tr("Physic field variable is scalar variable.").toStdString());

    // range
    if (rangemin != INT_MIN)
    {
        sceneView->sceneViewSettings().scalarRangeAuto = false;
        sceneView->sceneViewSettings().scalarRangeMin = rangemin;
    }
    else
    {
        sceneView->sceneViewSettings().scalarRangeAuto = true;
    }
    if (rangemax != INT_MIN)
        sceneView->sceneViewSettings().scalarRangeMax = rangemax;

    sceneView->doInvalidated();
}

// showgrid(show = {true, false})
void pythonShowGrid(bool show)
{
    sceneView->sceneViewSettings().showGrid = show;
    sceneView->doInvalidated();
}

// showgeometry(show = {true, false})
void pythonShowGeometry(bool show)
{
    sceneView->sceneViewSettings().showGeometry = show;
    sceneView->doInvalidated();
}

// showinitialmesh(show = {true, false})
void pythonShowInitialMesh(bool show)
{
    sceneView->sceneViewSettings().showInitialMesh = show;
    sceneView->doInvalidated();
}

// showsolutionmesh(show = {true, false})
void pythonShowSolutionMesh(bool show)
{
    sceneView->sceneViewSettings().showSolutionMesh = show;
    sceneView->doInvalidated();
}

// showcontours(show = {true, false})
void pythonShowContours(bool show)
{
    sceneView->sceneViewSettings().showContours = show;
    sceneView->doInvalidated();
}

// showvectors(show = {true, false})
void pythonShowVectors(bool show)
{
    sceneView->sceneViewSettings().showVectors = show;
    sceneView->doInvalidated();
}

// settimestep(level)
void pythonSetTimeStep(int timestep)
{
    if (Util::scene()->sceneSolution()->isSolved())
        sceneView->actSceneModePostprocessor->trigger();
    else
        throw out_of_range(QObject::tr("Problem is not solved.").toStdString());

    if (Util::scene()->problemInfo()->analysisType != ANALYSISTYPE_STEADYSTATE)
        throw out_of_range(QObject::tr("Solved problem is not transient.").toStdString());

    if ((timestep >= 0) && (timestep < Util::scene()->sceneSolution()->timeStepCount()))
        throw out_of_range(QObject::tr("Time step must be between 0 and %1..").arg(Util::scene()->sceneSolution()->timeStepCount()).toStdString());

    Util::scene()->sceneSolution()->setTimeStep(timestep);
}

// timestepcount()
int pythonTimeStepCount()
{
    return Util::scene()->sceneSolution()->timeStepCount();
}

// saveimage(filename)
void pythonSaveImage(char *str)
{
    sceneView->saveImageToFile(QString(str));
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
    {NULL, NULL, 0, NULL}
};

// ******************************************************************************************************************************************************

PythonEngine::PythonEngine()
{
    // init python
    Py_Initialize();

    // read functions
    m_functions = readFileContent(QApplication::applicationDirPath() + "/functions.py");

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
    // finalize and garbage python
    Py_DECREF(m_dict);

    if (Py_IsInitialized())
        Py_Finalize();
}

void PythonEngine::setSceneView(SceneView *sceneView)
{
    m_sceneView = sceneView;
}

ScriptResult PythonEngine::runPython(const QString &script, bool isExpression, const QString &fileName)
{
    sceneView = m_sceneView;
    ScriptResult scriptResult;

    // startup script
    PyRun_String(Util::scene()->problemInfo()->scriptStartup.toStdString().c_str(), Py_file_input, m_dict, m_dict);

    QString exp;
    if (isExpression)
        exp.append("result = " +  script + "\n");
    else
        exp.append(script + "\n");

    PyObject *output = PyRun_String(exp.toStdString().c_str(), Py_file_input, m_dict, m_dict);

    if (output)
    {
        // parse result
        if (isExpression)
        {
            PyObject *result = PyDict_GetItemString(m_dict, "result");
            if (result)
            {
                Py_INCREF(result);
                double value = 0;
                PyArg_Parse(result, "d", &value);
                scriptResult.value = value;
                Py_DECREF(result);
            }
        }

        // read stdout
        scriptResult.text = readFileContent(tempProblemDir() + "/stdout.txt");
        QFile::remove(tempProblemDir() + "/stdout.txt");

        // error
        scriptResult.isError = false;
    }
    else
    {
        scriptResult.value = 0.0;
        scriptResult.isError = true;

        PyObject *type = NULL, *value = NULL, *traceback = NULL, *str = NULL;
        PyErr_Fetch(&type, &value, &traceback);

        scriptResult.text = "";
        if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
        {
            Py_INCREF(type);
            scriptResult.text.append(PyString_AsString(str));
            if (type) Py_DECREF(type);
            if (str) Py_DECREF(str);
        }
        else
        {
            scriptResult.text.append("<unknown exception type> ");
        }

        if (value != NULL && (str = PyObject_Str(value)) != NULL && (PyString_Check(str)))
        {
            Py_INCREF(value);
            scriptResult.text.append(": ");
            scriptResult.text.append(PyString_AsString(value));
            if (value) Py_DECREF(value);
            if (str) Py_DECREF(str);
        }
        else
        {
            scriptResult.text.append("<unknown exception date> ");
        }


    }
    Py_DECREF(Py_None);

    return scriptResult;
}
