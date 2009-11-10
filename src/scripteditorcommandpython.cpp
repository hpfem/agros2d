#include <Python.h>

#include "util.h"
#include "scene.h"
#include "sceneview.h"
#include "scenemarker.h"

#include "scripteditordialog.h"

static SceneView *m_sceneView;

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
    return Py_BuildValue("s", QApplication::applicationVersion().toStdString().c_str());
}

// message(string)
static PyObject *pythonMessage(PyObject *self, PyObject *args)
{
    char *str;
    if (PyArg_ParseTuple(args, "s", &str))
    {
        QMessageBox::information(QApplication::activeWindow(), QObject::tr("Message"), QString(str));
    }

    Py_RETURN_NONE;
}

// variable = input(string)
static PyObject *pythonInput(PyObject *self, PyObject *args)
{
    char *str;
    if (PyArg_ParseTuple(args, "s", &str))
    {
        QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString(str));
        return Py_BuildValue("s", text.toStdString().c_str());
    }

    Py_RETURN_NONE;
}

// quit()
static PyObject *pythonQuit(PyObject *self, PyObject *args)
{
    QApplication::exit(0);

    Py_RETURN_NONE;
}

// newdocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition)
static PyObject *pythonNewDocument(PyObject *self, PyObject *args)
{
    int numberofrefinements = 0, polynomialorder = 1, adaptivitysteps = 0;
    double adaptivitytolerance = 0.0, frequency = 0.0, timestep = 0.0, totaltime = 0.0, initialcondition = 0.0;
    char *name, *type, *physicfield;
    char *adaptivitytype = "disabled"; // const_cast<char*>(adaptivityTypeToStringKey(ADAPTIVITYTYPE_NONE).toStdString().c_str());
    char *analysistype = "steadystate"; // const_cast<char*>(analysisTypeToStringKey(ANALYSISTYPE_STEADYSTATE).toStdString().c_str());
    if (PyArg_ParseTuple(args, "sss|iisiddsddd", &name, &type, &physicfield,
                         &numberofrefinements, &polynomialorder,
                         &adaptivitytype, &adaptivitysteps, &adaptivitytolerance,
                         &frequency, &analysistype, &timestep, &totaltime, &initialcondition))
    {
        Util::scene()->clear();

        Util::scene()->problemInfo()->name = QString(name);

        // type
        Util::scene()->problemInfo()->problemType = problemTypeFromStringKey(QString(type));
        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_UNDEFINED)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem type '%1' is not implemented.").arg(QString(type)).toStdString().c_str());
            return NULL;
        }

        // physicfield
        PhysicField physicField = physicFieldFromStringKey(QString(physicfield));
        if (physicField != PHYSICFIELD_UNDEFINED)
        {
            Util::scene()->problemInfo()->setHermes(hermesFieldFactory(physicField));
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Physic field '%1' is not implemented.").arg(QString(physicfield)).toStdString().c_str());
            return NULL;
        }

        // numberofrefinements
        if (numberofrefinements >= 0)
        {
            Util::scene()->problemInfo()->numberOfRefinements = numberofrefinements;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Number of refinements '%1' is out of range.").arg(numberofrefinements).toStdString().c_str());
            return NULL;
        }

        // polynomialorder
        if (polynomialorder >= 1 && polynomialorder <= 10)
        {
            Util::scene()->problemInfo()->polynomialOrder = polynomialorder;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Polynomial order '%1' is out of range.").arg(polynomialorder).toStdString().c_str());
            return NULL;
        }

        // adaptivitytype
        Util::scene()->problemInfo()->adaptivityType = adaptivityTypeFromStringKey(QString(adaptivitytype));
        if (Util::scene()->problemInfo()->adaptivityType == ADAPTIVITYTYPE_UNDEFINED)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Adaptivity type '%1' is not suported.").arg(QString(adaptivitytype)).toStdString().c_str());
            return NULL;
        }

        // adaptivitysteps
        if (adaptivitysteps >= 0)
        {
            Util::scene()->problemInfo()->adaptivitySteps = adaptivitysteps;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Adaptivity step '%1' is out of range.").arg(adaptivitysteps).toStdString().c_str());
            return NULL;
        }

        // adaptivitytolerance
        if (adaptivitytolerance >= 0)
        {
            Util::scene()->problemInfo()->adaptivityTolerance = adaptivitytolerance;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Adaptivity tolerance '%1' is out of range.").arg(adaptivitytolerance).toStdString().c_str());
            return NULL;
        }

        // frequency
        if (Util::scene()->problemInfo()->physicField() == PHYSICFIELD_HARMONICMAGNETIC)
        {
            if (frequency >= 0)
            {
                Util::scene()->problemInfo()->frequency = frequency;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Frequency can be used only for harmonic magnetic problems.").toStdString().c_str());
                return NULL;
            }
        }

        // analysis type
        Util::scene()->problemInfo()->analysisType = analysisTypeFromStringKey(QString(analysistype));
        if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_UNDEFINED)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Analysis type '%1' is not suported").arg(QString(adaptivitytype)).toStdString().c_str());
            return NULL;
        }

        // transient timestep
        if (timestep > 0)
        {
            Util::scene()->problemInfo()->timeStep = timestep;
        }
        else if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Time step must be positive.").toStdString().c_str());
            return NULL;
        }

        // transient timetotal
        if (totaltime > 0)
        {
            Util::scene()->problemInfo()->timeTotal = totaltime;
        }
        else if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Total time must be positive.").toStdString().c_str());
            return NULL;
        }

        // transient initial condition
        Util::scene()->problemInfo()->initialCondition = initialcondition;

        // invalidate
        m_sceneView->doDefaults();
        Util::scene()->refresh();

        Py_RETURN_NONE;
    }

    return NULL;
}

// opendocument(filename)
static PyObject *pythonOpenDocument(PyObject *self, PyObject *args)
{
    char *str;
    if (PyArg_ParseTuple(args, "s", &str))
    {
        Util::scene()->readFromFile(QString(str));
        Py_RETURN_NONE;
    }
    return NULL;
}

// savedocument(filename)
static PyObject *pythonSaveDocument(PyObject *self, PyObject *args)
{
    char *str;
    if (PyArg_ParseTuple(args, "s", &str))
    {
        Util::scene()->writeToFile(QString(str));
        Py_RETURN_NONE;
    }
    return NULL;
}

// addnode(x, y)
static PyObject *pythonAddNode(PyObject *self, PyObject *args)
{
    double x, y;
    if (PyArg_ParseTuple(args, "dd", &x, &y))
    {
        Util::scene()->addNode(new SceneNode(Point(x, y)));
        Py_RETURN_NONE;
    }
    return NULL;
}

// addedge(x1, y1, x2, y2, angle = 0, marker = "none")
static PyObject *pythonAddEdge(PyObject *self, PyObject *args)
{
    double x1, y1, x2, y2, angle = 0.0;
    char *marker = "none";
    if (PyArg_ParseTuple(args, "dddd|ds", &x1, &y1, &x2, &y2, &angle, &marker))
    {
        SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(QString(marker));
        if (!edgeMarker)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Marker '%1' is not defined.").arg(marker).toStdString().c_str());
            return NULL;
        }

        // start node
        SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(x1, y1)));
        // end node
        SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(x2, y2)));

        Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, edgeMarker, angle));
        Py_RETURN_NONE;
    }
    return NULL;
}
// addlabel(x, y, area = 0, marker = "none")
static PyObject *pythonAddLabel(PyObject *self, PyObject *args)
{
    double x, y, area = 0.0;
    char *marker = "none";
    if (PyArg_ParseTuple(args, "dd|ds", &x, &y, &area, &marker))
    {
        SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(QString(marker));
        if (!labelMarker)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Marker '%1' is not defined.").arg(marker).toStdString().c_str());
            return NULL;
        }

        Util::scene()->addLabel(new SceneLabel(Point(x, y), labelMarker, area));
        Py_RETURN_NONE;
    }
    return NULL;
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
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker with name '%1' doesn't exists.").arg(markerNew->name).toStdString().c_str());
        delete markerNew;
        return NULL;
    }
}

// selectnone()
static PyObject *pythonSelectNone(PyObject *self, PyObject *args)
{
    Util::scene()->selectNone();
    Py_RETURN_NONE;
}

// selectall()
static PyObject *pythonSelectAll(PyObject *self, PyObject *args)
{
    if (m_sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
    {
        // select volume integral area
        if (m_sceneView->actPostprocessorModeVolumeIntegral->isChecked())
            Util::scene()->selectAll(SCENEMODE_OPERATE_ON_LABELS);

        // select surface integral area
        if (m_sceneView->actPostprocessorModeSurfaceIntegral->isChecked())
            Util::scene()->selectAll(SCENEMODE_OPERATE_ON_EDGES);
    }
    else
    {
        Util::scene()->selectAll(m_sceneView->sceneMode());
    }
    m_sceneView->doInvalidated();
    Py_RETURN_NONE;
}

// selectnode(index, ...)
static PyObject *pythonSelectNode(PyObject *self, PyObject *args)
{
    python_int_array()
    {
        m_sceneView->actSceneModeEdge->trigger();
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
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// selectnodepoint(x, y)
static PyObject *pythonSelectNodePoint(PyObject *self, PyObject *args)
{
    double x, y;
    if (PyArg_ParseTuple(args, "dd", &x, &y))
    {
        SceneNode *node = m_sceneView->findClosestNode(Point(x, y));
        if (node)
        {
            node->isSelected = true;
            m_sceneView->doInvalidated();
            Py_RETURN_NONE;
        }
    }
    return NULL;
}

// selectedge(index, ...)
static PyObject *pythonSelectEdge(PyObject *self, PyObject *args)
{
    python_int_array()
    {
        m_sceneView->actSceneModeEdge->trigger();
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
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// selectedgepoint(x, y)
static PyObject *pythonSelectEdgePoint(PyObject *self, PyObject *args)
{
    double x, y;
    if (PyArg_ParseTuple(args, "dd", &x, &y))
    {
        SceneEdge *edge = m_sceneView->findClosestEdge(Point(x, y));
        if (edge)
        {
            edge->isSelected = true;
            m_sceneView->doInvalidated();
            Py_RETURN_NONE;
        }

    }
    return NULL;
}

// selectlabel(index, ...)
static PyObject *pythonSelectLabel(PyObject *self, PyObject *args)
{
    python_int_array()
    {
        m_sceneView->actSceneModeLabel->trigger();
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
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// selectlabelpoint(x, y)
static PyObject *pythonSelectLabelPoint(PyObject *self, PyObject *args)
{
    double x, y;
    if (PyArg_ParseTuple(args, "dd", &x, &y))
    {
        SceneLabel *label = m_sceneView->findClosestLabel(Point(x, y));
        if (label)
        {
            label->isSelected = true;
            m_sceneView->doInvalidated();
            Py_RETURN_NONE;
        }
    }
    return NULL;
}

// rotateselection(x, y, angle, copy = {true, false})
static PyObject *pythonRotateSelection(PyObject *self, PyObject *args)
{
    double x, y, angle;
    bool copy = false;
    if (PyArg_ParseTuple(args, "ddd|b", &x, &y, &angle, &copy))
    {
        Util::scene()->transformRotate(Point(x, y), angle, copy);
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// scaleselection(x, y, scale, copy = {true, false})
static PyObject *pythonScaleSelection(PyObject *self, PyObject *args)
{
    double x, y, scale;
    bool copy = false;
    if (PyArg_ParseTuple(args, "ddd|b", &x, &y, &scale, &copy))
    {
        Util::scene()->transformScale(Point(x, y), scale, copy);
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// moveselection(dx, dy, copy = {true, false})
static PyObject *pythonMoveSelection(PyObject *self, PyObject *args)
{
    double dx, dy;
    bool copy = false;
    if (PyArg_ParseTuple(args, "dd|b", &dx, &dy, &copy))
    {
        Util::scene()->transformTranslate(Point(dx, dy), copy);
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// deleteselection()
static PyObject *pythonDeleteSelection(PyObject *self, PyObject *args)
{
    Util::scene()->deleteSelected();
    Py_RETURN_NONE;
}

// mesh()
static PyObject *pythonMesh(PyObject *self, PyObject *args)
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH);
    Util::scene()->refresh();
    Py_RETURN_NONE;
}

// solve()
static PyObject *pythonSolve(PyObject *self, PyObject *args)
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH_AND_SOLVE);
    Util::scene()->refresh();
    m_sceneView->actSceneModePostprocessor->trigger();
    Py_RETURN_NONE;
}

// zoombestfit()
static PyObject *pythonZoomBestFit(PyObject *self, PyObject *args)
{
    m_sceneView->doZoomBestFit();
    Py_RETURN_NONE;
}

// zoomin()
static PyObject *pythonZoomIn(PyObject *self, PyObject *args)
{
    m_sceneView->doZoomIn();
    Py_RETURN_NONE;
}

// zoomout()
static PyObject *pythonZoomOut(PyObject *self, PyObject *args)
{
    m_sceneView->doZoomOut();
    Py_RETURN_NONE;
}

// zoomregion(x1, y1, x2, y2)
static PyObject *pythonZoomRegion(PyObject *self, PyObject *args)
{
    double x1, y1, x2, y2;
    if (PyArg_ParseTuple(args, "dddd", &x1, &y1, &x2, &y2))
    {
        m_sceneView->doZoomRegion(Point(x1, y1), Point(x2, y2));
        Py_RETURN_NONE;
    }

    return NULL;
}

// mode(mode = {"node", "edge", "label", "postprocessor"})
static PyObject *pythonMode(PyObject *self, PyObject *args)
{
    char *str;
    if (PyArg_ParseTuple(args, "s", &str))
    {
        if (QString(str) == "node")
            m_sceneView->actSceneModeNode->trigger();
        else if (QString(str) == "edge")
            m_sceneView->actSceneModeEdge->trigger();
        else if (QString(str) == "label")
            m_sceneView->actSceneModeLabel->trigger();
        else if (QString(str) == "postprocessor")
            if (Util::scene()->sceneSolution()->isSolved())
                m_sceneView->actSceneModePostprocessor->trigger();
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Mode '%1' is not implemented.").arg(QString(str)).toStdString().c_str());
            return NULL;
        }

        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// postprocessormode(mode = {"point", "surface", "volume"})
static PyObject *pythonPostprocessorMode(PyObject *self, PyObject *args)
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        m_sceneView->actSceneModePostprocessor->trigger();
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
        return NULL;
    }

    char *str;
    if (PyArg_ParseTuple(args, "s", &str))
    {
        if (QString(str) == "point")
            m_sceneView->actPostprocessorModeLocalPointValue->trigger();
        else if (QString(str) == "surface")
            m_sceneView->actPostprocessorModeSurfaceIntegral->trigger();
        else if (QString(str) == "volume")
            m_sceneView->actPostprocessorModeVolumeIntegral->trigger();
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Mode '%1' is not implemented.").arg(QString(str)).toStdString().c_str());
            return NULL;
        }
        m_sceneView->doInvalidated();
        Py_RETURN_NONE;
    }
    return NULL;
}

// result = pointresult(x, y)
static PyObject *pythonPointResult(PyObject *self, PyObject *args)
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        m_sceneView->actSceneModePostprocessor->trigger();
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
        m_sceneView->actSceneModePostprocessor->trigger();
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
        return NULL;
    }

    // set mode
    m_sceneView->actSceneModePostprocessor->trigger();
    m_sceneView->actPostprocessorModeSurfaceIntegral->trigger();

    python_int_array()
    {
        for (int i = 0; i < count; i++)
        {
            m_sceneView->actSceneModeEdge->trigger();
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
        m_sceneView->actSceneModePostprocessor->trigger();
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
        return NULL;
    }

    // set mode
    m_sceneView->actSceneModePostprocessor->trigger();
    m_sceneView->actPostprocessorModeVolumeIntegral->trigger();

    python_int_array()
    {
        m_sceneView->actSceneModeLabel->trigger();
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

// showgrid(show = {true, false})
static PyObject *pythonShowGrid(PyObject *self, PyObject *args)
{
    bool show;
    if (PyArg_ParseTuple(args, "b", &show))
    {
        m_sceneView->sceneViewSettings().showGrid = show;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// showgeometry(show = {true, false})
static PyObject *pythonShowGeometry(PyObject *self, PyObject *args)
{
    bool show;
    if (PyArg_ParseTuple(args, "b", &show))
    {
        m_sceneView->sceneViewSettings().showGeometry = show;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// showinitialmesh(show = {true, false})
static PyObject *pythonShowInitialMesh(PyObject *self, PyObject *args)
{
    bool show;
    if (PyArg_ParseTuple(args, "b", &show))
    {
        m_sceneView->sceneViewSettings().showInitialMesh = show;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// showsolutionmesh(show = {true, false})
static PyObject *pythonShowSolutionMesh(PyObject *self, PyObject *args)
{
    bool show;
    if (PyArg_ParseTuple(args, "b", &show))
    {
        m_sceneView->sceneViewSettings().showSolutionMesh = show;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// showcontours(show = {true, false})
static PyObject *pythonShowContours(PyObject *self, PyObject *args)
{
    bool show;
    if (PyArg_ParseTuple(args, "b", &show))
    {
        m_sceneView->sceneViewSettings().showContours = show;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// showvectors(show = {true, false})
static PyObject *pythonShowVectors(PyObject *self, PyObject *args)
{
    bool show;
    if (PyArg_ParseTuple(args, "b", &show))
    {
        m_sceneView->sceneViewSettings().showVectors = show;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// showscalar(type = { "none", "scalar", "scalar3d", "order" }, variable, component, rangemin, rangemax)
static PyObject *pythonShowScalar(PyObject *self, PyObject *args)
{
    char *type, *variable, *component;
    int rangemin = INT_MIN, rangemax = INT_MIN;
    if (PyArg_ParseTuple(args, "sss|dd", &type, &variable, &component, &rangemin, &rangemax))
    {
        // type
        SceneViewPostprocessorShow postprocessorShow = sceneViewPostprocessorShowFromStringKey(QString(type));
        if (postprocessorShow != SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED)
        {
            m_sceneView->sceneViewSettings().postprocessorShow = postprocessorShow;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("View type '%1' is not implemented.").arg(QString(type)).toStdString().c_str());
            return NULL;
        }

        // variable
        m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = physicFieldVariableFromStringKey(QString(variable));
        if (m_sceneView->sceneViewSettings().scalarPhysicFieldVariable == PHYSICFIELDVARIABLE_UNDEFINED)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Physic field variable '%1' is not defined.").arg(QString(variable)).toStdString().c_str());
            return NULL;
        }
        if (Util::scene()->problemInfo()->hermes()->physicFieldVariableCheck(m_sceneView->sceneViewSettings().scalarPhysicFieldVariable))
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Physic field variable '%1' cannot be used with this field.").arg(QString(variable)).toStdString().c_str());
            return NULL;
        }

        // variable component
        m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = physicFieldVariableCompFromStringKey(QString(component));
        if (m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp == PHYSICFIELDVARIABLECOMP_UNDEFINED)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Physic field variable component '%1' is not implemented.").arg(QString(component)).toStdString().c_str());
            return NULL;
        }
        if ((isPhysicFieldVariableScalar(m_sceneView->sceneViewSettings().scalarPhysicFieldVariable)) &&
            (m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp != PHYSICFIELDVARIABLECOMP_SCALAR))
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Physic field variable is scalar variable.").toStdString().c_str());
            return NULL;
        }

        // range
        if (rangemin != INT_MIN)
        {
            m_sceneView->sceneViewSettings().scalarRangeAuto = false;
            m_sceneView->sceneViewSettings().scalarRangeMin = rangemin;
        }
        else
        {
            m_sceneView->sceneViewSettings().scalarRangeAuto = true;
        }
        if (rangemax != INT_MIN)
            m_sceneView->sceneViewSettings().scalarRangeMax = rangemax;
    }
    m_sceneView->doInvalidated();

    Py_RETURN_NONE;
}

// settimestep(level)
static PyObject *pythonSetTimeStep(PyObject *self, PyObject *args)
{
    int timestep;
    if (PyArg_ParseTuple(args, "i", &timestep))
    {
        if (Util::scene()->sceneSolution()->isSolved())
        {
            m_sceneView->actSceneModePostprocessor->trigger();
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Problem is not solved.").toStdString().c_str());
            return NULL;
        }

        if (Util::scene()->problemInfo()->analysisType != ANALYSISTYPE_STEADYSTATE)
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Solved problem is not transient.").toStdString().c_str());
            return NULL;
        }

        if ((timestep >= 0) && (timestep < Util::scene()->sceneSolution()->timeStepCount()))
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Time step must be between 0 and %1..").arg(Util::scene()->sceneSolution()->timeStepCount()).toStdString().c_str());
            return NULL;
        }

        Util::scene()->sceneSolution()->setTimeStep(timestep);
    }

    Py_RETURN_NONE;
}

// timestepcount()
static PyObject *pythonTimeStepCount(PyObject *self, PyObject *args)
{
    return Py_BuildValue("i", Util::scene()->sceneSolution()->timeStepCount());
}

// saveimage(filename)
static PyObject *pythonSaveImage(PyObject *self, PyObject *args)
{
    char *filename;
    if (PyArg_ParseTuple(args, "s", &filename))
    {
        m_sceneView->saveImageToFile(QString(filename));

        Py_RETURN_NONE;
    }

    return NULL;
}


// ******************************************************************************************************************************************************

static PyMethodDef pythonMethods[] =
{
    {"version", pythonVersion, METH_VARARGS, "version()"},
    {"message", pythonMessage, METH_VARARGS, "message(str)"},
    {"input", pythonInput, METH_VARARGS, "out = input(str)"},
    {"quit", pythonQuit, METH_VARARGS, "quit()"},
    {"newdocument", pythonNewDocument, METH_VARARGS, "newdocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition)"},
    {"opendocument", pythonOpenDocument, METH_VARARGS, "opendocument(filename)"},
    {"savedocument", pythonSaveDocument, METH_VARARGS, "savedocument(filename)"},
    {"addnode", pythonAddNode, METH_VARARGS, "addnode(x, y)"},
    {"addedge", pythonAddEdge, METH_VARARGS, "addedge(x1, y1, x2, y2, angle = 0, marker = \"none\")"},
    {"addlabel", pythonAddLabel, METH_VARARGS, "addlabel(x, y, area = 0, marker = \"none\")"},
    {"addboundary", pythonAddBoundary, METH_VARARGS, "addboundary(name, type, value, ...)"},
    {"modifyboundary", pythonModifyBoundary, METH_VARARGS, "modifyBoundary(name, type, value, ...)"},
    {"addmaterial", pythonAddMaterial, METH_VARARGS, "addmaterial(name, type, value, ...)"},    
    {"modifymaterial", pythonModifyMaterial, METH_VARARGS, "modifymaterial(name, type, value, ...)"},
    {"selectnone", pythonSelectNone, METH_VARARGS, "selectnone()"},
    {"selectall", pythonSelectAll, METH_VARARGS, "selectall()"},
    {"selectnode", pythonSelectNode, METH_VARARGS, "selectnode(index, ...)"},
    {"selectnodepoint", pythonSelectNodePoint, METH_VARARGS, "selectnodepoint(x, y)"},
    {"selectedge", pythonSelectEdge, METH_VARARGS, "selectedge(index, ...)"},
    {"selectedgepoint", pythonSelectEdgePoint, METH_VARARGS, "selectedgepoint(x, y)"},
    {"selectlabel", pythonSelectLabel, METH_VARARGS, "selectlabel(index, ...)"},
    {"selectlabelpoint", pythonSelectLabelPoint, METH_VARARGS, "selectlabelpoint(x, y)"},
    {"moveselection", pythonMoveSelection, METH_VARARGS, "moveselection(dx, dy, copy = {true, false})"},
    {"rotateselection", pythonRotateSelection, METH_VARARGS, "rotateselection(x, y, angle, copy = {true, false})"},
    {"scaleselection", pythonScaleSelection, METH_VARARGS, "scaleselection(x, y, scale, copy = {true, false})"},
    {"deleteselection", pythonDeleteSelection, METH_VARARGS, "deleteselection()"},
    {"mesh", pythonMesh, METH_VARARGS, "mesh()"},
    {"solve", pythonSolve, METH_VARARGS, "solve()"},
    {"zoombestfit", pythonZoomBestFit, METH_VARARGS, "zoombestfit()"},
    {"zoomin", pythonZoomOut, METH_VARARGS, "zoomin()"},
    {"zoomout", pythonZoomIn, METH_VARARGS, "zoomout()"},
    {"zoomregion", pythonZoomRegion, METH_VARARGS, "zoomregion(x1, y1, x2, y2)"},
    {"pointresult", pythonPointResult, METH_VARARGS, "pointresult(x, y)"},
    {"volumeintegral", pythonVolumeIntegral, METH_VARARGS, "volumeintegral(index, ...)"},
    {"surfaceintegral", pythonSurfaceIntegral, METH_VARARGS, "surfaceintegral(index, ...)"},
    {"mode", pythonMode, METH_VARARGS, "mode(mode = {\"node\", \"edge\", \"label\", \"postprocessor\"})"},
    {"postprocessormode", pythonPostprocessorMode, METH_VARARGS, "postprocessormode(mode = {\"point\", \"surface\", \"volume\"})"},
    {"showgrid", pythonShowGrid, METH_VARARGS, "showgrid(show = {true, false})"},
    {"showgeometry", pythonShowGeometry, METH_VARARGS, "showgeometry(show = {true, false})"},
    {"showinitialmesh", pythonShowInitialMesh, METH_VARARGS, "showinitialmesh(show = {true, false})"},
    {"showsolutionmesh", pythonShowSolutionMesh, METH_VARARGS, "showsolutionmesh(show = {true, false})"},
    {"showcontours", pythonShowContours, METH_VARARGS, "showcontours(show = {true, false})"},
    {"showvectors", pythonShowVectors, METH_VARARGS, "showvectors(show = {true, false})"},
    {"showscalar", pythonShowScalar, METH_VARARGS, "showscalar(type = { \"none\", \"scalar\", \"scalar3d\", \"order\" }, variable, component, rangemin, rangemax)"},
    {"settimestep", pythonSetTimeStep, METH_VARARGS, "settimestep(level)"},
    {"timestepcount", pythonTimeStepCount, METH_VARARGS, "timestepcount()"},
    {"saveimage", pythonSaveImage, METH_VARARGS, "saveimage(filename)"},
    {NULL, NULL, 0, NULL}
};

class PythonEngine
{
public:
    PythonEngine()
    {
        // init python
        Py_Initialize();

        // read functions
        m_functions = readFileContent(QApplication::applicationDirPath() + "/functions.py");

        m_dict = PyDict_New();
        PyDict_SetItemString(m_dict, "__builtins__", PyEval_GetBuiltins());

        // agros module
        Py_InitModule("agros2d", pythonMethods);

        // stdout
        PyRun_String(QString("agrosstdout = \"" + tempProblemDir() + "/stdout.txt" + "\"").toStdString().c_str(), Py_file_input, m_dict, m_dict);

        // functions.py
        PyRun_String(m_functions.toStdString().c_str(), Py_file_input, m_dict, m_dict);
    }

    ~PythonEngine()
    {
        // finalize and garbage python
        Py_DECREF(m_dict);

        if (Py_IsInitialized())
            Py_Finalize();
    }

    ScriptResult runPython(const QString &script, bool isExpression = false, const QString &fileName = "")
    {
        ScriptResult scriptResult;

        // startup script
        PyRun_String(Util::scene()->problemInfo()->scriptStartup.toStdString().c_str(), Py_file_input, m_dict, m_dict);

        // add path
        // if (!fileName.isEmpty())
        //    exp.append("sys.path.append(\"" + QFileInfo(fileName).absolutePath() + "\")");

        QString exp;
        if (isExpression)
            exp.append("result = " +  script + "\n");
        else
            exp.append(script + "\n");

        // remove path
        // if (!fileName.isEmpty())
        //    exp.append("sys.path.remove(\"" + QFileInfo(fileName).absolutePath() + "\")");

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

            scriptResult.text = "Python error: ";
            if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
            {
                Py_INCREF(type);
                scriptResult.text.append(PyString_AsString(str));
                if (type) Py_DECREF(type);
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
            }
            else
            {
                scriptResult.text.append("<unknown exception date> ");
            }

            if (str) Py_DECREF(str);
        }
        Py_DECREF(Py_None);

        return scriptResult;
    }

private:
    PyObject *m_dict;
    QString m_functions;
};

static PythonEngine *pythonEngine;

