#include <QScriptEngine>

#include "util.h"
#include "scene.h"
#include "sceneview.h"
#include "scenemarker.h"
#include "scripteditorhighlighter.h"

static QString m_actualScriptFileName;

void setActualScriptFileName(const QString &fileName)
{
    m_actualScriptFileName = fileName;
}

// print(string)
QScriptValue scriptPrint(QScriptContext *context, QScriptEngine *engine)
{
    QString result;

    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: print(string);"));

    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }
    context->callee().setData(context->callee().data().toString() + result + "\n");

    return engine->undefinedValue();
}

// version()
QScriptValue scriptVersion(QScriptContext *context, QScriptEngine *engine)
{
    return QApplication::applicationVersion();
}

// message(string)
QScriptValue scriptMessage(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: message(string);"));

    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Message"), context->argument(0).toString());

    return engine->undefinedValue();
}

// variable = input(string)
QScriptValue scriptInput(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: variable = input(string);"));

    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), context->argument(0).toString());

    return text;
}

// quit()
QScriptValue scriptQuit(QScriptContext *context, QScriptEngine *engine)
{
    QApplication::exit(0);

    return engine->undefinedValue();
}

// include(filename)
QScriptValue scriptInclude(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: include(filename);"));

    QString fileName;

    // directory of the actual script
    if (!m_actualScriptFileName.isEmpty() &&
        QFile::exists(QFileInfo(m_actualScriptFileName).absolutePath() + '/' + context->argument(0).toString()))
        fileName = QFileInfo(m_actualScriptFileName).absolutePath() + '/' + context->argument(0).toString();

    // actual directory of the problem
    if (fileName.isEmpty() &&
        !Util::scene()->problemInfo()->fileName.isEmpty() &&
        QFile::exists(QFileInfo(Util::scene()->problemInfo()->fileName).absolutePath() + '/' + context->argument(0).toString()))
        fileName = QFileInfo(Util::scene()->problemInfo()->fileName).absolutePath() + '/' + context->argument(0).toString();

    if (fileName.isEmpty())
    {
        return context->throwError(QObject::tr("File '%1' not found.").arg(context->argument(0).toString()));
    }
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
            return context->throwError(QObject::tr("Could not open file '%1'.").arg(fileName));

        QTextStream inFile(&file);
        engine->currentContext()->setActivationObject(engine->currentContext()->parentContext()->activationObject());
        engine->evaluate(inFile.readAll(), fileName);
        file.close();

        return true;
    }
}

// printToFile(filename, string, mode = {"append"})
QScriptValue scriptPrintToFile(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: printToFile(filename, string, mode = {\"append\"});"));
    }

    QFile file(context->argument(0).toString());
    bool ok = true;
    if (context->argument(2).toString() == "append")
        ok = file.open(QIODevice::Append);
    else
        ok = file.open(QIODevice::WriteOnly);

    if (!ok)
        return context->throwError(QObject::tr("Could not open file '%1'.").arg(context->argument(0).toString()));

    QTextStream outFile(&file);
    outFile << context->argument(1).toString() << endl;
    file.close();

    return true;
}

// newDocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition)
QScriptValue scriptNewDocument(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 4; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: newDocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition);"));
    }

    Util::scene()->clear();

    // name
    if (context->argument(0).toString() != "")
        Util::scene()->problemInfo()->name = context->argument(0).toString();
    else
        Util::scene()->problemInfo()->name = QObject::tr("unnamed");

    // type
    Util::scene()->problemInfo()->problemType = problemTypeFromStringKey(context->argument(1).toString());
    if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_UNDEFINED)
        return context->throwError(QObject::tr("Problem type '%1' is not implemented.").arg(context->argument(1).toString()));

    // physicfield
    PhysicField physicField = physicFieldFromStringKey(context->argument(2).toString());
    if (physicField != PHYSICFIELD_UNDEFINED)
        Util::scene()->problemInfo()->setHermes(hermesFieldFactory(physicField));
    else
        return context->throwError(QObject::tr("Physic field '%1' is not implemented.").arg(context->argument(2).toString()));

    // numberofrefinements
    if (context->argument(3).toNumber() >= 0)
        Util::scene()->problemInfo()->numberOfRefinements = context->argument(3).toNumber();
    else
        return context->throwError(QObject::tr("Number of refinements '%1' is out of range.").arg(context->argument(3).toString()));

    // polynomialorder
    if (context->argument(4).toNumber() >= 1 && context->argument(4).toNumber() <= 10)
        Util::scene()->problemInfo()->polynomialOrder = context->argument(4).toNumber();
    else
        return context->throwError(QObject::tr("Polynomial order '%1' is out of range.").arg(context->argument(4).toString()));

    // adaptivitytype, adaptivitysteps, adaptivitytolerance
    if (context->argument(5).isUndefined())
    {
        Util::scene()->problemInfo()->adaptivityType == ADAPTIVITYTYPE_NONE;
    }
    else
    {
        Util::scene()->problemInfo()->adaptivityType = adaptivityTypeFromStringKey(context->argument(5).toString());
        if (Util::scene()->problemInfo()->adaptivityType == ADAPTIVITYTYPE_UNDEFINED)
            return context->throwError(QObject::tr("Adaptivity type '%1' is not suported.").arg(context->argument(5).toString()));
    }

    // adaptivitysteps
    if (context->argument(6).isUndefined())
    {
        Util::scene()->problemInfo()->adaptivitySteps == 5;
    }
    else
    {
        if (context->argument(6).isNumber() && context->argument(6).toNumber() >= 0)
            Util::scene()->problemInfo()->adaptivitySteps = context->argument(6).toNumber();
        else
            return context->throwError(QObject::tr("Adaptivity step '%1' is out of range.").arg(context->argument(6).toString()));
    }

    // adaptivitytolerance
    if (context->argument(7).isUndefined())
    {
        Util::scene()->problemInfo()->adaptivityTolerance == 0.0;
    }
    else
    {
        if (context->argument(7).isNumber() && context->argument(7).toNumber() >= 0)
            Util::scene()->problemInfo()->adaptivityTolerance = context->argument(7).toNumber();
        else
            return context->throwError(QObject::tr("Adaptivity tolerance '%1' is out of range.").arg(context->argument(8).toString()));
    }

    // frequency
    if (context->argument(8).isUndefined())
    {
        Util::scene()->problemInfo()->frequency == 0.0;
    }
    else
    {
        if (context->argument(8).toNumber() > 0 && Util::scene()->problemInfo()->physicField() != PHYSICFIELD_HARMONICMAGNETIC)
            return context->throwError(QObject::tr("Frequency can be used only for harmonic magnetic problems."));
        else
            Util::scene()->problemInfo()->frequency = context->argument(8).toNumber();
    }

    // analysis type
    if (context->argument(9).isUndefined())
    {
        Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_STEADYSTATE;
    }
    else
    {
        Util::scene()->problemInfo()->analysisType = analysisTypeFromStringKey(context->argument(9).toString());
        if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_UNDEFINED)
            return context->throwError(QObject::tr("Analysis type '%1' is not suported.").arg(context->argument(9).toString()));
    }

    // transient timestep
    if (context->argument(10).isUndefined())
    {
        Util::scene()->problemInfo()->timeStep == 1.0;
    }
    else
    {
        if (context->argument(10).isNumber() && context->argument(10).toNumber() > 0)
            Util::scene()->problemInfo()->timeStep = context->argument(10).toNumber();
        else
            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
                return context->throwError(QObject::tr("Time step must be positive."));
    }

    // transient timetotal
    if (context->argument(11).isUndefined())
    {
        Util::scene()->problemInfo()->timeTotal == 1.0;
    }
    else
    {
        if (context->argument(11).isNumber() && context->argument(11).toNumber() > 0)
            Util::scene()->problemInfo()->timeTotal = context->argument(11).toNumber();
        else
            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
                return context->throwError(QObject::tr("Total time must be positive."));
    }

    // transient initial condition
    if (context->argument(12).isUndefined())
    {
        Util::scene()->problemInfo()->initialCondition == 0.0;
    }
    else
    {
        if (context->argument(12).isNumber() && context->argument(12).toNumber() > 0)
            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
                Util::scene()->problemInfo()->initialCondition = context->argument(12).toNumber();
    }

    m_sceneView->doDefaults();
    Util::scene()->refresh();

    return engine->undefinedValue();
}

// openDocument(filename)
QScriptValue scriptOpenDocument(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: openDocument(filename);"));

    Util::scene()->readFromFile(context->argument(0).toString());

    return engine->undefinedValue();
}

// saveDocument(filename)
QScriptValue scriptSaveDocument(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: saveDocument(filename);"));

    Util::scene()->writeToFile(context->argument(0).toString());

    return engine->undefinedValue();
}

// addNode(x, y)
QScriptValue scriptAddNode(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: addNode(x, y);"));
    }

    Util::scene()->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));

    return engine->undefinedValue();
}

// addLabel(x, y, area = 0, marker = "none")
QScriptValue scriptAddLabel(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: addLabel(x, y, area = 0, marker = \"none\");"));
    }

    // area
    double area;
    if (context->argumentCount() > 2)
    {
        if (context->argument(2).isNumber() && context->argument(2).toNumber() >= 0.0)
            area = context->argument(2).toNumber();
        else
            return context->throwError(QObject::tr("Area must be number greater or equal zero."));
    }
    else
    {
        area = 0.0;
    }

    // marker
    SceneLabelMarker *marker;
    if (context->argumentCount() == 4)
    {
        // find marker by name
         marker = Util::scene()->getLabelMarker(context->argument(3).toString());
         if (!marker)
            return context->throwError(QObject::tr("Marker '%1' is not defined.").arg(context->argument(3).toString()));
    }
    else
    {
        marker = Util::scene()->labelMarkers[0];
    }

    Util::scene()->addLabel(new SceneLabel(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), marker, area));

    return engine->undefinedValue();
}

// addEdge(x1, y1, x2, y2, angle = 0, marker = "none")
QScriptValue scriptAddEdge(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 4; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: addEdge(x1, y1, x2, y2, angle = 0, marker = \"none\");"));
    }

    // start node
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));
    // end node
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(context->argument(2).toNumber(), context->argument(3).toNumber())));

    // angle
    double angle;
    if (context->argumentCount() > 4)
    {
        if (context->argument(4).isNumber() && context->argument(4).toNumber() >= 0.0 && context->argument(4).toNumber() <= 180)
            angle = context->argument(4).toNumber();
        else
            return context->throwError(QObject::tr("Angle must be between zero and 180 degs."));
    }
    else
    {
        angle = 0.0;
    }

    // marker
    SceneEdgeMarker *marker;
    if (context->argumentCount() == 6)
    {
        // find marker by name
        marker = Util::scene()->getEdgeMarker(context->argument(5).toString());
        if (!marker)
            return context->throwError(QObject::tr("Marker '%1' is not defined.").arg(context->argument(5).toString()));
    }
    else
    {
        marker = Util::scene()->edgeMarkers[0];
    }

    Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, marker, angle));

    return engine->undefinedValue();
}

// addBoundary(name, type, value, ...)
QScriptValue scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: addBoundary(name, type, value, ...);"));
    }

    // test for names
    foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
    {
        if (edgeMarker->name == context->argument(0).toString())
            return context->throwError(QObject::tr("Boundary marker with name '%1' already exists.").arg(context->argument(0).toString()));
    }

    Util::scene()->addEdgeMarker(Util::scene()->problemInfo()->hermes()->newEdgeMarker(context->argument(0).toString(), context));
    // TODO - physicFieldBCCheck(PhysicFieldBC physicFieldBC)

    return engine->undefinedValue();
}

// modifyBoundary(name, type, value, ...)
QScriptValue scriptModifyBoundary(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: modifyBoundary(name, type, value, ...);"));

    SceneEdgeMarker *marker = Util::scene()->getEdgeMarker(context->argument(0).toString());
    if (marker)
    {
        Util::scene()->setEdgeMarker(context->argument(0).toString(),
                                     Util::scene()->problemInfo()->hermes()->newEdgeMarker(context->argument(0).toString(), context));

        return engine->undefinedValue();
    }
    else
        return context->throwError(QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(context->argument(0).toString()));
}

// addMaterial(name, type, value, ...)
QScriptValue scriptAddMaterial(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: addMaterial(name, type, value, ...);"));

    // test for names
    foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
    {
        if (labelMarker->name == context->argument(0).toString())
            return context->throwError(QObject::tr("Label marker with name '%1' already exists.").arg(context->argument(0).toString()));
    }

    Util::scene()->addLabelMarker(Util::scene()->problemInfo()->hermes()->newLabelMarker(context->argument(0).toString(), context));

    return engine->undefinedValue();
}

// modifyMaterial(name, type, value, ...)
QScriptValue scriptModifyMaterial(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: modifyMaterial(name, type, value, ...);"));

    SceneLabelMarker *marker = Util::scene()->getLabelMarker(context->argument(0).toString());
    if (marker)
    {
        Util::scene()->setLabelMarker(context->argument(0).toString(),
                                     Util::scene()->problemInfo()->hermes()->newLabelMarker(context->argument(0).toString(), context));

        return engine->undefinedValue();
    }
    else
        return context->throwError(QObject::tr("Label marker with name '%1' doesn't exists.").arg(context->argument(0).toString()));
}


// mesh()
QScriptValue scriptMesh(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH);
    Util::scene()->refresh();

    return engine->undefinedValue();
}

// solve()
QScriptValue scriptSolve(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH_AND_SOLVE);
    Util::scene()->refresh();
    m_sceneView->actSceneModePostprocessor->trigger();

    return engine->undefinedValue();
}

// zoomBestFit()
QScriptValue scriptZoomBestFit(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->doZoomBestFit();

    return engine->undefinedValue();
}

// zoomIn()
QScriptValue scriptZoomIn(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->doZoomIn();

    return engine->undefinedValue();
}

// zoomOut()
QScriptValue scriptZoomOut(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->doZoomOut();

    return engine->undefinedValue();
}

// zoomRegion(x1, y1, x2, y2)
QScriptValue scriptZoomRegion(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 3; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: zoomRegion(x1, y1, x2, y2);"));
    }

    m_sceneView->doZoomRegion(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), Point(context->argument(2).toNumber(), context->argument(3).toNumber()));

    return engine->undefinedValue();
}

// mode(mode = {"node", "edge", "label", "postprocessor"})
QScriptValue scriptMode(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: mode(mode = {\"node\", \"edge\", \"label\", \"postprocessor\"});"));

    if (context->argument(0).toString() == "node")
        m_sceneView->actSceneModeNode->trigger();
    else if (context->argument(0).toString() == "edge")
        m_sceneView->actSceneModeEdge->trigger();
    else if (context->argument(0).toString() == "label")
        m_sceneView->actSceneModeLabel->trigger();
    else if (context->argument(0).toString() == "postprocessor")
        if (Util::scene()->sceneSolution()->isSolved())
            m_sceneView->actSceneModePostprocessor->trigger();
    else
        return context->throwError(QObject::tr("Mode '%1' is not implemented.").arg(context->argument(0).toString()));

    return engine->undefinedValue();
}

// selectNone()
QScriptValue scriptSelectNone(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    return engine->undefinedValue();
}

// selectAll()
QScriptValue scriptSelectAll(QScriptContext *context, QScriptEngine *engine)
{
    if (m_sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
        return context->throwError(QObject::tr("Select node, edge or label mode."));
    else
        Util::scene()->selectAll(m_sceneView->sceneMode());

    return engine->undefinedValue();
}

// selectNode(index, ...)
QScriptValue scriptSelectNode(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: selectNode(index, ...);"));

    m_sceneView->actSceneModeNode->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
    {
        if (Util::scene()->nodes.count() < context->argument(i).toNumber())
            return context->throwError(QObject::tr("Node with index '%1' does not exists.").arg(context->argument(0).toString()));

        Util::scene()->nodes[context->argument(i).toNumber()]->isSelected = true;
    }
    m_sceneView->doInvalidated();

    return engine->undefinedValue();
}

// selectNodePoint(x, y)
QScriptValue scriptSelectNodePoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: selectNodePoint(x, y);"));
    }

    SceneNode *node = m_sceneView->findClosestNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber()));
    if (node)
    {
        node->isSelected = true;
        m_sceneView->doInvalidated();
    }

    return engine->undefinedValue();
}

// selectEdge(index, ...)
QScriptValue scriptSelectEdge(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: selectEdge(index, ...);"));

    m_sceneView->actSceneModeEdge->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
    {
        if (Util::scene()->edges.count() < context->argument(i).toNumber())
            return context->throwError(QObject::tr("Edge with index '%1' does not exists.").arg(context->argument(i).toString()));

        Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;
    }
    m_sceneView->doInvalidated();

    return engine->undefinedValue();
}

// selectEdgePoint(x, y)
QScriptValue scriptSelectEdgePoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: selectEdgePoint(x, y);"));
    }

    SceneEdge *edge = m_sceneView->findClosestEdge(Point(context->argument(0).toNumber(), context->argument(1).toNumber()));
    if (edge)
    {
        edge->isSelected = true;
        m_sceneView->doInvalidated();
    }

    return engine->undefinedValue();
}

// selectLabel(index, ...)
QScriptValue scriptSelectLabel(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: selectLabel(index, ...);"));

    m_sceneView->actSceneModeLabel->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
    {
        if (Util::scene()->labels.count() < context->argument(i).toNumber())
            return context->throwError(QObject::tr("Label with index '%1' does not exists.").arg(context->argument(0).toString()));

        Util::scene()->labels[context->argument(i).toNumber()]->isSelected = true;
    }
    m_sceneView->doInvalidated();

    return engine->undefinedValue();
}

// selectLabelPoint(x, y)
QScriptValue scriptSelectLabelPoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: selectLabelPoint(x, y);"));
    }

    SceneLabel *label = m_sceneView->findClosestLabel(Point(context->argument(0).toNumber(), context->argument(1).toNumber()));
    if (label)
    {
        label->isSelected = true;
        m_sceneView->doInvalidated();
    }
}

// moveSelection(dx, dy, copy = {true, false})
QScriptValue scriptMoveSelection(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: moveSelection(dx, dy, copy = {true, false});"));
    }

    bool copy = true;
    if (context->argument(2).isUndefined()) copy = false;
    Util::scene()->transformTranslate(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), copy);

    return engine->undefinedValue();
}

// rotateSelection(x, y, angle, copy = {true, false})
QScriptValue scriptRotateSelection(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: rotateSelection(x, y, angle, copy = {true, false});"));
    }

    bool copy = true;
    if (context->argument(2).isUndefined()) copy = false;
    Util::scene()->transformRotate(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), context->argument(2).toNumber(), copy);

    return engine->undefinedValue();
}

// scaleSelection(x, y, scale, copy = {true, false})
QScriptValue scriptScaleSelection(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: scaleSelection(x, y, scale, copy = {true, false});"));
    }

    bool copy = true;
    if (context->argument(2).isUndefined()) copy = false;
    Util::scene()->transformScale(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), context->argument(2).toNumber(), copy);

    return engine->undefinedValue();
}

// deleteSelection()
QScriptValue scriptDeleteSelection(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->deleteSelected();

    return engine->undefinedValue();
}

// result = pointResult(x, y)
QScriptValue scriptPointResult(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 1; i++)
    {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters. Command syntax: result = pointResult(x, y);"));
    }

    if (Util::scene()->sceneSolution()->isSolved())
        m_sceneView->actSceneModePostprocessor->trigger();
    else
        return context->throwError(QObject::tr("Problem is not solved."));

    Point point(context->argument(0).toNumber(), context->argument(1).toNumber());
    LocalPointValue *localPointValue = Util::scene()->problemInfo()->hermes()->localPointValue(point);

    QStringList headers = Util::scene()->problemInfo()->hermes()->localPointValueHeader();
    QStringList variables = localPointValue->variables();

    QScriptValue value = engine->newObject();
    for (int i = 0; i < variables.length(); i++)
        value.setProperty(headers[i], QString(variables[i]).toDouble());

    delete localPointValue;

    return value;
}

// result = scriptVolumeIntegral(index, ...)
QScriptValue scriptVolumeIntegral(QScriptContext *context, QScriptEngine *engine)
{
    if (Util::scene()->sceneSolution()->isSolved())
        m_sceneView->actSceneModePostprocessor->trigger();
    else
        return context->throwError(QObject::tr("Problem is not solved."));

    if (m_sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
    {
        m_sceneView->actPostprocessorModeVolumeIntegral->trigger();
        Util::scene()->selectNone();

        // select all or indices
        if (context->argumentCount() == 0)
        {
            foreach (SceneLabel *label, Util::scene()->labels)
                label->isSelected = true;
        }
        else
        {
            for (int i = 0; i<context->argumentCount(); i++)
            {
                if (Util::scene()->labels.count() < context->argument(i).toNumber())
                    return context->throwError(QObject::tr("Label with index '%1' does not exists.").arg(context->argument(0).toString()));

                Util::scene()->labels[context->argument(i).toNumber()]->isSelected = true;
            }
        }

        VolumeIntegralValue *volumeIntegral = Util::scene()->problemInfo()->hermes()->volumeIntegralValue();

        QStringList headers = Util::scene()->problemInfo()->hermes()->volumeIntegralValueHeader();
        QStringList variables = volumeIntegral->variables();

        QScriptValue value = engine->newObject();
        for (int i = 0; i < variables.length(); i++)
            value.setProperty(headers[i], QString(variables[i]).toDouble());

        delete volumeIntegral;

        return value;
    }
    else
    {
        return engine->undefinedValue();
    }
}

// result = surfaceIntegral(index, ...)
QScriptValue scriptSurfaceIntegral(QScriptContext *context, QScriptEngine *engine)
{
    if (Util::scene()->sceneSolution()->isSolved())
        m_sceneView->actSceneModePostprocessor->trigger();
    else
        return context->throwError(QObject::tr("Problem is not solved."));

    if (m_sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
    {
        m_sceneView->actPostprocessorModeSurfaceIntegral->trigger();
        Util::scene()->selectNone();

        // select all or indices
        if (context->argumentCount() == 0)
            foreach (SceneEdge *edge, Util::scene()->edges)
                edge->isSelected = true;
        else
            for (int i = 0; i<context->argumentCount(); i++)
            {
            if (Util::scene()->edges.count() < context->argument(i).toNumber())
                return context->throwError(QObject::tr("Edge with index '%1' does not exists.").arg(context->argument(0).toString()));

            Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;
        }

        SurfaceIntegralValue *surfaceIntegral = Util::scene()->problemInfo()->hermes()->surfaceIntegralValue();

        QStringList headers = Util::scene()->problemInfo()->hermes()->surfaceIntegralValueHeader();
        QStringList variables = surfaceIntegral->variables();

        QScriptValue value = engine->newObject();
        for (int i = 0; i < variables.length(); i++)
            value.setProperty(headers[i], QString(variables[i]).toDouble());

        delete surfaceIntegral;

        return value;
    }
    else
    {
        return engine->undefinedValue();
    }
}

// showGrid(show = {true, false})
QScriptValue scriptShowGrid(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showGrid(show = {true, false});"));

    m_sceneView->sceneViewSettings().showGrid = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showGeometry(show = {true, false})
QScriptValue scriptShowGeometry(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showGeometry(show = {true, false});"));

    m_sceneView->sceneViewSettings().showGeometry = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showInitialMesh(show = {true, false})
QScriptValue scriptShowInitialMesh(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showInitialMesh(show = {true, false});"));

    m_sceneView->sceneViewSettings().showInitialMesh = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showSolutionMesh(show = {true, false})
QScriptValue scriptShowSolutionMesh(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showSolutionMesh(show = {true, false});"));

    m_sceneView->sceneViewSettings().showSolutionMesh = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showContours(show = {true, false})
QScriptValue scriptShowContours(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showContours(show = {true, false});"));

    m_sceneView->sceneViewSettings().showContours = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showVectors(show = {true, false})
QScriptValue scriptShowVectors(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showVectors(show = {true, false});"));

    m_sceneView->sceneViewSettings().showVectors = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showScalar(type = { "none", "scalar", "scalar3d", "order" }, variable, component, rangemin, rangemax)
QScriptValue scriptShowScalar(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: showScalar(type = { \"none\", \"scalar\", \"scalar3d\", \"order\" }, variable, component, rangemin, rangemax);"));

    SceneViewPostprocessorShow postprocessorShow = sceneViewPostprocessorShowFromStringKey(context->argument(0).toString());
    if (postprocessorShow != SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED)
        m_sceneView->sceneViewSettings().postprocessorShow = postprocessorShow;
    else
        return context->throwError(QObject::tr("View type '%1' is not implemented.").arg(context->argument(0).toString()));

    m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = physicFieldVariableFromStringKey(context->argument(1).toString());
    if (m_sceneView->sceneViewSettings().scalarPhysicFieldVariable == PHYSICFIELDVARIABLE_UNDEFINED)
        return context->throwError(QObject::tr("Physic field variable '%1' is not defined.").arg(context->argument(1).toString()));
    if (Util::scene()->problemInfo()->hermes()->physicFieldVariableCheck(m_sceneView->sceneViewSettings().scalarPhysicFieldVariable))
        return context->throwError(QObject::tr("Physic field variable '%1' cannot be used with this field.").arg(context->argument(1).toString()));

    m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = physicFieldVariableCompFromStringKey(context->argument(2).toString());
    if (m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp == PHYSICFIELDVARIABLECOMP_UNDEFINED)
        return context->throwError(QObject::tr("Physic field variable component '%1' is not implemented.").arg(context->argument(2).toString()));

    if (context->argument(3).isNumber())
    {
        m_sceneView->sceneViewSettings().scalarRangeAuto = false;
        m_sceneView->sceneViewSettings().scalarRangeMin = context->argument(3).toNumber();
    }
    else
    {
        m_sceneView->sceneViewSettings().scalarRangeAuto = true;
    }
    if (context->argument(4).isNumber())
        m_sceneView->sceneViewSettings().scalarRangeMax = context->argument(4).toNumber();

    return engine->undefinedValue();
}

// setTimeLevel(level)
QScriptValue scriptSetTimeStep(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: setTimeLevel(level);"));

    if (Util::scene()->sceneSolution()->isSolved())
        m_sceneView->actSceneModePostprocessor->trigger();
    else
        return context->throwError(QObject::tr("Problem is not solved."));

    if (Util::scene()->problemInfo()->analysisType != ANALYSISTYPE_STEADYSTATE)
        return context->throwError(QObject::tr("Solved problem is not transient."));

    if (context->argument(0).isNumber() &&
        (context->argument(0).toInteger() >= 0) &&
        (context->argument(0).toInteger() < Util::scene()->sceneSolution()->timeStepCount()))
        return context->throwError(QObject::tr("Time step must be between 0 and %1.").arg(Util::scene()->sceneSolution()->timeStepCount()));

    Util::scene()->sceneSolution()->setTimeStep(context->argument(0).toInteger());
}

// saveImage(filename)
QScriptValue scriptSaveImage(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters. Command syntax: saveImage(filename);"));

    m_sceneView->saveImageToFile(context->argument(0).toString());

    return engine->undefinedValue();
}
