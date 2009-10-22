#include <QScriptEngine>

#include "util.h"
#include "scene.h"
#include "sceneview.h"
#include "scenemarker.h"
#include "scripteditorhighlighter.h"

// print(string)
QScriptValue scriptPrint(QScriptContext *context, QScriptEngine *engine)
{
    QString result;

    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

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
        return context->throwError(QObject::tr("Few parameters."));

    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Message"), context->argument(0).toString());

    return engine->undefinedValue();
}

// variable = input(string)
QScriptValue scriptInput(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

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
        return context->throwError(QObject::tr("Few parameters."));

    if (QFile::exists(context->argument(0).toString()))
    {
        QFile file(context->argument(0).toString());
        if (!file.open(QIODevice::ReadOnly))
            return context->throwError(QObject::tr("Could not open file '%1'.").arg(context->argument(0).toString()));

        QTextStream inFile(&file);
        engine->currentContext()->setActivationObject(engine->currentContext()->parentContext()->activationObject());
        engine->evaluate(inFile.readAll(), context->argument(0).toString());
        file.close();

        return true;
    }
    else
    {
        return context->throwError(QObject::tr("File '%1' not found.").arg(context->argument(0).toString()));
    }
}

// printToFile(filename, string, mode = {"append"})
QScriptValue scriptPrintToFile(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
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

// newDocument(name, type, physicfield, numberofrefinements, polynomialorder, frequency, adaptivitytype, adaptivitysteps, adaptivitytolerance)
QScriptValue scriptNewDocument(QScriptContext *context, QScriptEngine *engine)
{
    ProblemInfo problemInfo;
    Util::scene()->clear();

    for (int i = 0; i <= 8; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }
    for (int i = 3; i <= 8; i++) {
        if (!context->argument(i).isNumber())
            return context->throwError(QObject::tr("Parameter %1 must be number. Now is used '%2' as this parametr.").arg(i+1).arg(context->argument(i).toString()));
        if (i == 5) i = 6;
    }

    // name
    if (context->argument(0).toString() != "")
        problemInfo.name = context->argument(0).toString();
    else
        problemInfo.name = QObject::tr("unnamed");

    // type
    problemInfo.problemType = problemTypeFromStringKey(context->argument(1).toString());
    if (problemInfo.problemType == PROBLEMTYPE_UNDEFINED)
        return context->throwError(QObject::tr("Problem type '%1' is not implemented.").arg(context->argument(1).toString()));

    // physicfield
    PhysicField physicField = physicFieldFromStringKey(context->argument(2).toString());
    if (physicField != PHYSICFIELD_UNDEFINED)
        problemInfo.hermes = hermesFieldFactory(physicField);
    else
        return context->throwError(QObject::tr("Physic field '%1' is not implemented.").arg(context->argument(2).toString()));

    // numberofrefinements
    if (context->argument(3).toNumber() >= 0)
        problemInfo.numberOfRefinements = context->argument(3).toNumber();
    else
        return context->throwError(QObject::tr("Number of refinements '%1' is out of range.").arg(context->argument(3).toString()));

    // polynomialorder
    if (context->argument(4).toNumber() >= 1 && context->argument(4).toNumber() <= 10)
        problemInfo.polynomialOrder = context->argument(4).toNumber();
    else
        return context->throwError(QObject::tr("Polynomial order '%1' is out of range.").arg(context->argument(4).toString()));

    // frequency
    if (context->argument(5).toNumber() > 0 && problemInfo.physicField() != PHYSICFIELD_HARMONICMAGNETIC)
        return context->throwError(QObject::tr("Frequency can be used only for harmonic magnetic problems."));
    else
        problemInfo.frequency = context->argument(5).toNumber();

    // adaptivitytype, adaptivitysteps, adaptivitytolerance
    problemInfo.adaptivityType = adaptivityTypeFromStringKey(context->argument(6).toString());
    if (problemInfo.adaptivityType == ADAPTIVITYTYPE_UNDEFINED)
        return context->throwError(QObject::tr("Adaptivity type '%1' is not suported.").arg(context->argument(6).toString()));

    // adaptivitysteps
    if (context->argument(7).isNumber() && context->argument(7).toNumber() >= 0)
        problemInfo.adaptivitySteps = context->argument(7).toNumber();
    else
        return context->throwError(QObject::tr("Adaptivity step '%1' is out of range.").arg(context->argument(7).toString()));

    // adaptivitytolerance
    if (context->argument(8).isNumber() && context->argument(8).toNumber() >= 0)
        problemInfo.adaptivityTolerance = context->argument(8).toNumber();
    else
        return context->throwError(QObject::tr("Adaptivity tolerance '%1' is out of range.").arg(context->argument(8).toString()));

    Util::scene()->problemInfo() = problemInfo;
    m_sceneView->doDefaults();
    Util::scene()->refresh();

    return engine->undefinedValue();
}

// openDocument(filename)
QScriptValue scriptOpenDocument(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    Util::scene()->readFromFile(context->argument(0).toString());

    return engine->undefinedValue();
}

// saveDocument(filename)
QScriptValue scriptSaveDocument(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    Util::scene()->writeToFile(context->argument(0).toString());

    return engine->undefinedValue();
}

// addNode(x, y)
QScriptValue scriptAddNode(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    Util::scene()->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));

    return engine->undefinedValue();
}

// addLabel(x, y, area, marker)
QScriptValue scriptAddLabel(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    // area
    double area;
    if (context->argumentCount() > 2)
        area = context->argument(2).toNumber();
    else
        area = 0.0;

    // marker
    SceneLabelMarker *marker;
    if (context->argumentCount() == 4)
    {
        // find marker by name
        bool ok = false;
        foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
        {
            if (context->argument(3).toString() == labelMarker->name)
            {
                marker = labelMarker;
                ok = true;
                break;
            }
        }
        if (ok == false)
            return context->throwError(QObject::tr("Marker '%1' is not defined.").arg(context->argument(3).toString()));
    }
    else
    {
        marker = Util::scene()->labelMarkers[0];
    }

    Util::scene()->addLabel(new SceneLabel(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), marker, area));

    return engine->undefinedValue();
}

// addEdge(x1, y1, x2, y2, angle, marker)
QScriptValue scriptAddEdge(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 4; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    // start node
    SceneNode *nodeStart = Util::scene()->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));
    // end node
    SceneNode *nodeEnd = Util::scene()->addNode(new SceneNode(Point(context->argument(2).toNumber(), context->argument(3).toNumber())));

    // angle
    double angle;
    if (context->argumentCount() > 4)
        angle = context->argument(4).toNumber();
    else
        angle = 0.0;

    // marker
    SceneEdgeMarker *marker;
    if (context->argumentCount() == 6)
    {
        // find marker by name
        bool ok = false;
        foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
        {
            if (edgeMarker->name == context->argument(5).toString())
            {
                marker = edgeMarker;
                ok = true;
                break;
            }
        }
        if (ok == false)
            return context->throwError(QObject::tr("Marker '%1' is not defined.").arg(context->argument(3).toString()));
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
    for (int i = 0; i <= 2; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    // test for names
    foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
    {
        if (edgeMarker->name == context->argument(0).toString())
            return context->throwError(QObject::tr("Boundary marker with name '%1' already exists.").arg(context->argument(0).toString()));
    }

    PhysicFieldBC type;
    switch (Util::scene()->problemInfo().physicField())
    {
    case PHYSICFIELD_ELECTROSTATIC:
        if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL))
            type = PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE))
            type = PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE;
        else
            return context->throwError(QObject::tr("Boundary type '%1' is not defined.").arg(context->argument(1).toString()));
        Util::scene()->addEdgeMarker(new SceneEdgeElectrostaticMarker(context->argument(0).toString(),
                                                                      type,
                                                                      Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL))
            type = PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT))
            type = PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT;
        else
            return context->throwError(QObject::tr("Boundary type '%1' is not defined.").arg(context->argument(1).toString()));
        Util::scene()->addEdgeMarker(new SceneEdgeMagnetostaticMarker(context->argument(0).toString(),
                                                                      type,
                                                                      Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_HARMONICMAGNETIC:
        if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL))
            type = PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT))
            type = PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT;
        else
            return context->throwError(QObject::tr("Boundary type '%1' is not defined.").arg(context->argument(1).toString()));
        Util::scene()->addEdgeMarker(new SceneEdgeHarmonicMagneticMarker(context->argument(0).toString(),
                                                                         type,
                                                                         Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_HEAT:
        if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_HEAT_TEMPERATURE))
        {
            type = PHYSICFIELDBC_HEAT_TEMPERATURE;
            Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(context->argument(0).toString(),
                                                                 type,
                                                                 Value(context->argument(2).toString())));
        }
        else if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_HEAT_HEAT_FLUX))
        {
            type = PHYSICFIELDBC_HEAT_HEAT_FLUX;
            Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(context->argument(0).toString(), type,
                                                                 Value(context->argument(2).toString()),
                                                                 Value(context->argument(3).toString()),
                                                                 Value(context->argument(4).toString())));
        }
        else
            return context->throwError(QObject::tr("Boundary type '%1' is not defined.").arg(context->argument(1).toString()));
        break;
    case PHYSICFIELD_CURRENT:
        if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_CURRENT_POTENTIAL))
            type = PHYSICFIELDBC_CURRENT_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW))
            type = PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW;
        else
            return context->throwError(QObject::tr("Boundary type '%1' is not defined.").arg(context->argument(1).toString()));
        Util::scene()->addEdgeMarker(new SceneEdgeCurrentMarker(context->argument(0).toString(),
                                                                type,
                                                                Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_ELASTICITY:
        PhysicFieldBC typeX, typeY;
        if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_ELASTICITY_FREE))
            typeX = PHYSICFIELDBC_ELASTICITY_FREE;
        else if (context->argument(1).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_ELASTICITY_FIXED))
            typeX = PHYSICFIELDBC_ELASTICITY_FIXED;
        else
            return context->throwError(QObject::tr("Boundary type '%1' is not defined.").arg(context->argument(1).toString()));

        if (context->argument(2).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_ELASTICITY_FREE))
            typeY = PHYSICFIELDBC_ELASTICITY_FREE;
        else if (context->argument(2).toString() == physicFieldBCToStringKey(PHYSICFIELDBC_ELASTICITY_FIXED))
            typeY = PHYSICFIELDBC_ELASTICITY_FIXED;
        else
            Util::scene()->addEdgeMarker(new SceneEdgeElasticityMarker(context->argument(0).toString(), typeX, typeY,
                                                                       Value(context->argument(3).toString()),
                                                                       Value(context->argument(4).toString())));
        break;
    default:
        std::cerr << "Physical field '" + QString::number(Util::scene()->problemInfo().physicField()).toStdString() + "' is not implemented. scriptAddBoundary()" << endl;
        throw;
        break;
    }

    return engine->undefinedValue();
}

// addMaterial(name, type, value, ...)
QScriptValue scriptAddMaterial(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    // test for names
    foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
    {
        if (labelMarker->name == context->argument(0).toString())
            return context->throwError(QObject::tr("Label marker with name '%1' already exists.").arg(context->argument(0).toString()));
    }

    PhysicFieldBC type;
    switch (Util::scene()->problemInfo().physicField())
    {
    case PHYSICFIELD_ELECTROSTATIC:
        if (context->argument(2).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
        Util::scene()->addLabelMarker(new SceneLabelElectrostaticMarker(context->argument(0).toString(),
                                                                        Value(context->argument(1).toString()),
                                                                        Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        if (context->argument(4).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
        Util::scene()->addLabelMarker(new SceneLabelMagnetostaticMarker(context->argument(0).toString(),
                                                                        Value(context->argument(1).toString()),
                                                                        Value(context->argument(2).toString()),
                                                                        Value(context->argument(3).toString()),
                                                                        Value(context->argument(4).toString())));
        break;
    case PHYSICFIELD_HARMONICMAGNETIC:
        if (context->argument(4).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
        Util::scene()->addLabelMarker(new SceneLabelHarmonicMagneticMarker(context->argument(0).toString(),
                                                                           Value(context->argument(1).toString()),
                                                                           Value(context->argument(2).toString()),
                                                                           Value(context->argument(3).toString()),
                                                                           Value(context->argument(4).toString())));
        break;
    case PHYSICFIELD_HEAT:
        if (context->argument(4).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
        Util::scene()->addLabelMarker(new SceneLabelHeatMarker(context->argument(0).toString(),
                                                               Value(context->argument(1).toString()),
                                                               Value(context->argument(2).toString()),
                                                               Value(context->argument(3).toString()),
                                                               Value(context->argument(4).toString())));
        break;
    case PHYSICFIELD_CURRENT:
        if (context->argument(1).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
        Util::scene()->addLabelMarker(new SceneLabelCurrentMarker(context->argument(0).toString(),
                                                                  Value(context->argument(1).toString())));
        break;
    case PHYSICFIELD_ELASTICITY:
        if (context->argument(4).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
        Util::scene()->addLabelMarker(new SceneLabelElasticityMarker(context->argument(0).toString(),
                                                                     Value(context->argument(2).toString()),
                                                                     Value(context->argument(3).toString())));
        break;
    default:
        std::cerr << "Physical field '" + QString::number(Util::scene()->problemInfo().physicField()).toStdString() + "' is not implemented. scriptAddMaterial()" << endl;
        throw;
        break;
    }

    return engine->undefinedValue();
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
    for (int i = 0; i <= 3; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    m_sceneView->doZoomRegion(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), Point(context->argument(2).toNumber(), context->argument(3).toNumber()));

    return engine->undefinedValue();
}

// mode(mode = {"node", "edge", "label", "postprocessor"})
QScriptValue scriptMode(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

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
    Util::scene()->selectAll(m_sceneView->sceneMode());
    /*
    if (m_sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
    {
        if (m_sceneView->actPostprocessorModeVolumeIntegral->isChecked())
            Util::scene()->selectAll(SCENEMODE_OPERATE_ON_LABELS);
        if (m_sceneView->actPostprocessorModeSurfaceIntegral->isChecked())
            Util::scene()->selectAll(SCENEMODE_OPERATE_ON_EDGES);
    }
    */

    return engine->undefinedValue();
}

// selectNode(index, ...)
QScriptValue scriptSelectNode(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->actSceneModeNode->trigger();
    for (int i = 0; i<context->argumentCount(); i++) {
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

    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
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
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->actSceneModeEdge->trigger();
    for (int i = 0; i<context->argumentCount(); i++) {
        if (Util::scene()->edges.count() < context->argument(i).toNumber())
            return context->throwError(QObject::tr("Edge with index '%1' does not exists.").arg(context->argument(0).toString()));

        Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;
    }
    m_sceneView->doInvalidated();

    return engine->undefinedValue();
}

// selectEdgePoint(x, y)
QScriptValue scriptSelectEdgePoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();

    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
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
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->actSceneModeLabel->trigger();
    for (int i = 0; i<context->argumentCount(); i++) {
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

    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
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
    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    bool copy = true;
    if (context->argument(2).isUndefined()) copy = false;
    Util::scene()->transformTranslate(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), copy);

    return engine->undefinedValue();
}

// rotateSelection(x, y, angle, copy = {true, false})
QScriptValue scriptRotateSelection(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    bool copy = true;
    if (context->argument(2).isUndefined()) copy = false;
    Util::scene()->transformRotate(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), context->argument(2).toNumber(), copy);

    return engine->undefinedValue();
}

// scaleSelection(x, y, scale, copy = {true, false})
QScriptValue scriptScaleSelection(QScriptContext *context, QScriptEngine *engine)
{
    for (int i = 0; i <= 2; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
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
    for (int i = 0; i <= 1; i++) {
        if (context->argument(i).isUndefined())
            return context->throwError(QObject::tr("Few parameters."));
    }

    Point point(context->argument(0).toNumber(), context->argument(1).toNumber());
    LocalPointValue *localPointValue = Util::scene()->problemInfo().hermes->localPointValue(point);

    QStringList headers = Util::scene()->problemInfo().hermes->localPointValueHeader();
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
            for (int i = 0; i<context->argumentCount(); i++) {
                if (Util::scene()->labels.count() < context->argument(i).toNumber())
                    return context->throwError(QObject::tr("Label with index '%1' does not exists.").arg(context->argument(0).toString()));

                Util::scene()->labels[context->argument(i).toNumber()]->isSelected = true;
            }
        }

        VolumeIntegralValue *volumeIntegral = Util::scene()->problemInfo().hermes->volumeIntegralValue();

        QStringList headers = Util::scene()->problemInfo().hermes->volumeIntegralValueHeader();
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

    if (m_sceneView->sceneMode() == SCENEMODE_POSTPROCESSOR)
    {
        m_sceneView->actPostprocessorModeSurfaceIntegral->trigger();
        Util::scene()->selectNone();

        // select all or indices
        if (context->argumentCount() == 0)
            foreach (SceneEdge *edge, Util::scene()->edges)
                edge->isSelected = true;
        else
            for (int i = 0; i<context->argumentCount(); i++) {
            if (Util::scene()->edges.count() < context->argument(i).toNumber())
                return context->throwError(QObject::tr("Edge with index '%1' does not exists.").arg(context->argument(0).toString()));

            Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;
        }

        SurfaceIntegralValue *surfaceIntegral = Util::scene()->problemInfo().hermes->surfaceIntegralValue();

        QStringList headers = Util::scene()->problemInfo().hermes->surfaceIntegralValueHeader();
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
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->sceneViewSettings().showGrid = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showGeometry(show = {true, false})
QScriptValue scriptShowGeometry(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->sceneViewSettings().showGeometry = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showInitialMesh(show = {true, false})
QScriptValue scriptShowInitialMesh(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->sceneViewSettings().showInitialMesh = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showSolutionMesh(show = {true, false})
QScriptValue scriptShowSolutionMesh(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->sceneViewSettings().showSolutionMesh = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showContours(show = {true, false})
QScriptValue scriptShowContours(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->sceneViewSettings().showContours = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showVectors(show = {true, false})
QScriptValue scriptShowVectors(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->sceneViewSettings().showVectors = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showScalar(type = { "none", "scalar", "scalar3d", "order" }, variable, component, rangemin, rangemax)
QScriptValue scriptShowScalar(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    SceneViewPostprocessorShow postprocessorShow = sceneViewPostprocessorShowFromStringKey(context->argument(0).toString());
    if (postprocessorShow != SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED)
        m_sceneView->sceneViewSettings().postprocessorShow = postprocessorShow;
    else
        return context->throwError(QObject::tr("View type '%1' is not implemented.").arg(context->argument(0).toString()));

    m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = physicFieldVariableFromStringKey(context->argument(1).toString());
    /*
    switch (Util::scene()->problemInfo().physicField())
    {
    case PHYSICFIELD_ELECTROSTATIC:
        if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY;
        else
            return context->throwError(QObject::tr("Physic field variable '%1' is not implemented.").arg(context->argument(1).toString()));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY;
        else
            return context->throwError(QObject::tr("Physic field variable '%1' is not implemented.").arg(context->argument(1).toString()));
        break;
    case PHYSICFIELD_HARMONICMAGNETIC:
        if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_REAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_REAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_IMAG))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_IMAG;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_REAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_REAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_IMAG))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_IMAG;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_REAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_REAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_IMAG))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_IMAG;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_REAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_REAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_IMAG))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_IMAG;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_REAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_REAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_IMAG))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_IMAG;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_POWER_LOSSES))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_POWER_LOSSES;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_ENERGY_DENSITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_ENERGY_DENSITY;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HARMONICMAGNETIC_PERMEABILITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONICMAGNETIC_PERMEABILITY;
        else
            return context->throwError(QObject::tr("Physic field variable '%1' is not implemented.").arg(context->argument(1).toString()));
        break;
    case PHYSICFIELD_CURRENT:
        if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_CURRENT_POTENTIAL))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_POTENTIAL;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_CURRENT_LOSSES))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_LOSSES;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY;
        else
            return context->throwError(QObject::tr("Physic field variable '%1' is not implemented.").arg(context->argument(1).toString()));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_TEMPERATURE;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HEAT_FLUX))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_FLUX;
        else if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY;
        else
            return context->throwError(QObject::tr("Physic field variable '%1' is not implemented.").arg(context->argument(1).toString()));
        break;
    case PHYSICFIELD_ELASTICITY:
        if (context->argument(1).toString() == physicFieldVariableStringKey(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS))
            m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS;
        else
            return context->throwError(QObject::tr("Physic field variable '%1' is not implemented.").arg(context->argument(1).toString()));
        break;
    default:
        std::cerr << "Physical field '" + QString::number(Util::scene()->problemInfo().physicField()).toStdString() + "' is not implemented. scriptShowScalar()" << endl;
        throw;
        break;
    }
    */

    m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = physicFieldVariableCompFromStringKey(context->argument(2).toString());
    if (m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp == PHYSICFIELDVARIABLECOMP_UNDEFINED)
        return context->throwError(QObject::tr("Physic field variable component '%1' is not implemented.").arg(context->argument(2).toString()));
    /*
    if ( == physicFieldVariableCompStringKey(PHYSICFIELDVARIABLECOMP_SCALAR))
        m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_SCALAR;
    else if (context->argument(2).toString() == physicFieldVariableCompStringKey(PHYSICFIELDVARIABLECOMP_MAGNITUDE))
        m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_MAGNITUDE;
    else if (context->argument(2).toString() == Util::scene()->problemInfo().labelX())
        m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_X;
    else if (context->argument(2).toString() == Util::scene()->problemInfo().labelY())
        m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_Y;
    else
    */

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

// saveImage(filename)
QScriptValue scriptSaveImage(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).isUndefined())
        return context->throwError(QObject::tr("Few parameters."));

    m_sceneView->saveImageToFile(context->argument(0).toString());

    return engine->undefinedValue();
}
