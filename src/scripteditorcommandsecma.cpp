#include <QScriptEngine>

#include "scene.h"
#include "sceneview.h"
#include "scenemarker.h"
#include "scripteditorhighlighter.h"

// print(string)
QScriptValue scriptPrint(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }
    
    // QScriptValue calleeData = context->callee().data();
    // QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(calleeData.toQObject());
    // edit->appendPlainText(result);
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
    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Script message"), context->argument(0).toString());
    return engine->undefinedValue();
}

// variable = input(string)
QScriptValue scriptInput(QScriptContext *context, QScriptEngine *engine)
{
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
    if (QFile::exists(context->argument(0).toString()))
    {
        QFile file(context->argument(0).toString());
        if (!file.open(QIODevice::ReadOnly))
        {
            return QString("Could not open file '%1'.").arg(context->argument(0).toString());
        }
        
        QTextStream inFile(&file);
        engine->currentContext()->setActivationObject(engine->currentContext()->parentContext()->activationObject());
        engine->evaluate(inFile.readAll(), context->argument(0).toString());
        file.close();
        
        return true;
    }
    else
    {
        return QString("File not found '%1'.").arg(context->argument(0).toString());
    }
}

// printToFile(filename, string, mode)
QScriptValue scriptPrintToFile(QScriptContext *context, QScriptEngine *engine)
{
    QFile file(context->argument(0).toString());
    bool ok = true;
    if (context->argument(2).toString() == "append")
        ok = file.open(QIODevice::Append);
    else
        ok = file.open(QIODevice::WriteOnly);
    
    if (!ok)
    {
        return QString("Could not open file '%1'.").arg(context->argument(0).toString());
    }
    
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
    Util::scene()->problemInfo() = problemInfo;
    Util::scene()->problemInfo().name = context->argument(0).toString();
    if (context->argument(1).toString() == problemTypeStringKey(PROBLEMTYPE_PLANAR)) Util::scene()->problemInfo().problemType = PROBLEMTYPE_PLANAR;
    if (context->argument(1).toString() == problemTypeStringKey(PROBLEMTYPE_AXISYMMETRIC)) Util::scene()->problemInfo().problemType = PROBLEMTYPE_AXISYMMETRIC;
    Util::scene()->problemInfo().physicField = physicFieldFromStringKey(context->argument(2).toString());
    Util::scene()->problemInfo().numberOfRefinements = context->argument(3).toNumber();
    Util::scene()->problemInfo().polynomialOrder = context->argument(4).toNumber();
    Util::scene()->problemInfo().frequency = context->argument(5).toNumber();
    if (context->argument(6).toString() == adaptivityTypeStringKey(ADAPTIVITYTYPE_NONE)) Util::scene()->problemInfo().adaptivityType = ADAPTIVITYTYPE_NONE;
    if (context->argument(6).toString() == adaptivityTypeStringKey(ADAPTIVITYTYPE_H)) Util::scene()->problemInfo().adaptivityType = ADAPTIVITYTYPE_H;
    if (context->argument(6).toString() == adaptivityTypeStringKey(ADAPTIVITYTYPE_P)) Util::scene()->problemInfo().adaptivityType = ADAPTIVITYTYPE_P;
    if (context->argument(6).toString() == adaptivityTypeStringKey(ADAPTIVITYTYPE_HP)) Util::scene()->problemInfo().adaptivityType = ADAPTIVITYTYPE_HP;
    Util::scene()->problemInfo().adaptivitySteps = context->argument(7).toNumber();
    Util::scene()->problemInfo().adaptivityTolerance = context->argument(8).toNumber();
    
    m_sceneView->doDefaults();
    Util::scene()->refresh();
    return engine->undefinedValue();
}

// openDocument(filename)
QScriptValue scriptOpenDocument(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->readFromFile(context->argument(0).toString());
    return engine->undefinedValue();
}

// saveDocument(filename)
QScriptValue scriptSaveDocument(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->writeToFile(context->argument(0).toString());
    return engine->undefinedValue();
}

// addNode(x, y)
QScriptValue scriptAddNode(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));
    return engine->undefinedValue();
}

// addLabel(x, y, area, marker)
QScriptValue scriptAddLabel(QScriptContext *context, QScriptEngine *engine)
{
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
        foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
        {
            if (labelMarker->name == context->argument(3).toString())
            {
                marker = labelMarker;
                break;
            }
        }
    }
    else
        marker = Util::scene()->labelMarkers[0];
    
    Util::scene()->addLabel(new SceneLabel(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), marker, area));
    return engine->undefinedValue();
}

// addEdge(x1, y1, x2, y2, angle, marker)
QScriptValue scriptAddEdge(QScriptContext *context, QScriptEngine *engine)
{
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
        foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
        {
            if (edgeMarker->name == context->argument(5).toString())
            {
                marker = edgeMarker;
                break;
            }
        }
    }
    else
        marker = Util::scene()->edgeMarkers[0];
    
    // edge
    Util::scene()->addEdge(new SceneEdge(nodeStart, nodeEnd, marker, angle));
    return engine->undefinedValue();
}

// addBoundary(name, type, value, ...)
QScriptValue scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)
{
    PhysicFieldBC type;
    switch (Util::scene()->problemInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL)) type = PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE)) type = PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE;
        Util::scene()->addEdgeMarker(new SceneEdgeElectrostaticMarker(context->argument(0).toString(),
                                                                      type,
                                                                      Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL)) type = PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT)) type = PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT;
        Util::scene()->addEdgeMarker(new SceneEdgeMagnetostaticMarker(context->argument(0).toString(),
                                                                      type,
                                                                      Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_HARMONIC_MAGNETIC_VECTOR_POTENTIAL)) type = PHYSICFIELDBC_HARMONIC_MAGNETIC_VECTOR_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_HARMONIC_MAGNETIC_SURFACE_CURRENT)) type = PHYSICFIELDBC_HARMONIC_MAGNETIC_SURFACE_CURRENT;
        Util::scene()->addEdgeMarker(new SceneEdgeHarmonicMagneticMarker(context->argument(0).toString(),
                                                                         type,
                                                                         Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_HEAT_TEMPERATURE))
        {
            type = PHYSICFIELDBC_HEAT_TEMPERATURE;
            Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(context->argument(0).toString(),
                                                                 type,
                                                                 Value(context->argument(2).toString())));
        }
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_HEAT_HEAT_FLUX))
        {
            type = PHYSICFIELDBC_HEAT_HEAT_FLUX;
            Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(context->argument(0).toString(), type,
                                                                 Value(context->argument(2).toString()),
                                                                 Value(context->argument(3).toString()),
                                                                 Value(context->argument(4).toString())));
        }
        break;
    case PHYSICFIELD_CURRENT:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_CURRENT_POTENTIAL)) type = PHYSICFIELDBC_CURRENT_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW)) type = PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW;
        Util::scene()->addEdgeMarker(new SceneEdgeCurrentMarker(context->argument(0).toString(),
                                                                type,
                                                                Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_ELASTICITY:
        PhysicFieldBC typeX, typeY;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FREE)) typeX = PHYSICFIELDBC_ELASTICITY_FREE;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FIXED)) typeX = PHYSICFIELDBC_ELASTICITY_FIXED;
        if (context->argument(2).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FREE)) typeY = PHYSICFIELDBC_ELASTICITY_FREE;
        if (context->argument(2).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FIXED)) typeY = PHYSICFIELDBC_ELASTICITY_FIXED;
        Util::scene()->addEdgeMarker(new SceneEdgeElasticityMarker(context->argument(0).toString(), typeX, typeY,
                                                                   context->argument(3).toNumber(),
                                                                   context->argument(4).toNumber()));
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(Util::scene()->problemInfo().physicField).toStdString() + "' is not implemented. scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)" << endl;
        throw;
        break;
    }
    
    return engine->undefinedValue();
}

// addMaterial(name, type, value, ...)
QScriptValue scriptAddMaterial(QScriptContext *context, QScriptEngine *engine)
{
    PhysicFieldBC type;
    switch (Util::scene()->problemInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        Util::scene()->addLabelMarker(new SceneLabelElectrostaticMarker(context->argument(0).toString(),
                                                                        Value(context->argument(1).toString()),
                                                                        Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        Util::scene()->addLabelMarker(new SceneLabelMagnetostaticMarker(context->argument(0).toString(),
                                                                        Value(context->argument(1).toString()),
                                                                        Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        Util::scene()->addLabelMarker(new SceneLabelHarmonicMagneticMarker(context->argument(0).toString(),
                                                                           Value(context->argument(1).toString()),
                                                                           Value(context->argument(2).toString()),
                                                                           Value(context->argument(3).toString()),
                                                                           Value(context->argument(4).toString())));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        Util::scene()->addLabelMarker(new SceneLabelHeatMarker(context->argument(0).toString(),
                                                               Value(context->argument(1).toString()),
                                                               Value(context->argument(2).toString())));
        break;
    case PHYSICFIELD_CURRENT:
        Util::scene()->addLabelMarker(new SceneLabelCurrentMarker(context->argument(0).toString(),
                                                                  Value(context->argument(1).toString())));
        break;
    case PHYSICFIELD_ELASTICITY:
        Util::scene()->addLabelMarker(new SceneLabelElasticityMarker(context->argument(0).toString(),
                                                                     context->argument(2).toNumber(),
                                                                     context->argument(3).toNumber()));
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(Util::scene()->problemInfo().physicField).toStdString() + "' is not implemented. scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)" << endl;
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

// zoomRegion()
QScriptValue scriptZoomRegion(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->doZoomRegion(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), Point(context->argument(2).toNumber(), context->argument(3).toNumber()));

    return engine->undefinedValue();
}

// mode(mode = {"node", "edge", "label", "postprocessor"})
QScriptValue scriptMode(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).toString() == "node")
        m_sceneView->actSceneModeNode->trigger();
    if (context->argument(0).toString() == "edge")
        m_sceneView->actSceneModeEdge->trigger();
    if (context->argument(0).toString() == "label")
        m_sceneView->actSceneModeLabel->trigger();
    if (context->argument(0).toString() == "postprocessor")
        if (Util::scene()->sceneSolution()->isSolved())
            m_sceneView->actSceneModePostprocessor->trigger();
    
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

// selectNode()
QScriptValue scriptSelectNode(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();
    
    m_sceneView->actSceneModeNode->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
        Util::scene()->nodes[context->argument(i).toNumber()]->isSelected = true;
    
    m_sceneView->doInvalidated();
    
    return engine->undefinedValue();
}

// selectNodePoint(x, y)
QScriptValue scriptSelectNodePoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();
    
    SceneNode *node = m_sceneView->findClosestNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber()));
    if (node)
    {
        node->isSelected = true;
        m_sceneView->doInvalidated();
    }
    
    return engine->undefinedValue();
}

// selectEdge()
QScriptValue scriptSelectEdge(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();
    
    m_sceneView->actSceneModeEdge->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
        Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;
    
    m_sceneView->doInvalidated();
    
    return engine->undefinedValue();
}

// selectEdgePoint(x, y)
QScriptValue scriptSelectEdgePoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();
    
    SceneEdge *edge = m_sceneView->findClosestEdge(Point(context->argument(0).toNumber(), context->argument(1).toNumber()));
    if (edge)
    {
        edge->isSelected = true;
        m_sceneView->doInvalidated();
    }
    
    return engine->undefinedValue();
}

// selectLabel()
QScriptValue scriptSelectLabel(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();
    
    m_sceneView->actSceneModeLabel->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
        Util::scene()->labels[context->argument(i).toNumber()]->isSelected = true;
    
    m_sceneView->doInvalidated();
    
    return engine->undefinedValue();
}

// selectLabelPoint(x, y)
QScriptValue scriptSelectLabelPoint(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->selectNone();
    
    SceneLabel *label = m_sceneView->findClosestLabel(Point(context->argument(0).toNumber(), context->argument(1).toNumber()));
    if (label)
    {
        label->isSelected = true;
        m_sceneView->doInvalidated();
    }
}

// moveSelection(dx, dy, copy = false)
QScriptValue scriptMoveSelection(QScriptContext *context, QScriptEngine *engine)
{
    bool copy = (context->argumentCount() == 2) ? false : context->argument(2).toBoolean();
    Util::scene()->transformTranslate(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), copy);
    
    return engine->undefinedValue();
}

// rotateSelection(x, y, angle, copy = false)
QScriptValue scriptRotateSelection(QScriptContext *context, QScriptEngine *engine)
{
    bool copy = (context->argumentCount() == 3) ? false : context->argument(2).toBoolean();
    Util::scene()->transformRotate(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), context->argument(2).toNumber(), copy);
    
    return engine->undefinedValue();
}

// scaleSelection(x, y, scale, copy = false)
QScriptValue scriptScaleSelection(QScriptContext *context, QScriptEngine *engine)
{
    bool copy = (context->argumentCount() == 2) ? false : context->argument(2).toBoolean();
    Util::scene()->transformScale(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), context->argument(2).toNumber(), copy);
    
    return engine->undefinedValue();
}

// deleteSelection(dx, dy, copy = false)
QScriptValue scriptDeleteSelection(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->deleteSelected();
    
    return engine->undefinedValue();
}

// result = pointResult(x, y)
QScriptValue scriptPointResult(QScriptContext *context, QScriptEngine *engine)
{
    Point point(context->argument(0).toNumber(), context->argument(1).toNumber());
    LocalPointValue *localPointValue = localPointValueFactory(point);
    
    QStringList headers = localPointValueHeaderFactory(Util::scene()->problemInfo().physicField);
    QStringList variables = localPointValue->variables();
    
    QScriptValue value = engine->newObject();
    for (int i = 0; i < variables.length(); i++)
        value.setProperty(headers[i], QString(variables[i]).toDouble());
    
    delete localPointValue;
    
    return value;
}

// result = scriptVolumeIntegral(index ...)
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
            foreach (SceneLabel *label, Util::scene()->labels)
                label->isSelected = true;
        else
            for (int i = 0; i<context->argumentCount(); i++)
                Util::scene()->labels[context->argument(i).toNumber()]->isSelected = true;
        
        VolumeIntegralValue *volumeIntegral = volumeIntegralValueFactory();
        
        QStringList headers = volumeIntegralValueHeaderFactory(Util::scene()->problemInfo().physicField);
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

// result = surfaceIntegral(index ...)
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
            for (int i = 0; i<context->argumentCount(); i++)
                Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;
        
        SurfaceIntegralValue *surfaceIntegral = surfaceIntegralValueFactory();
        
        QStringList headers = surfaceIntegralValueHeaderFactory(Util::scene()->problemInfo().physicField);
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

// showGrid(bool)
QScriptValue scriptShowGrid(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->sceneViewSettings().showGrid = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showGeometry(bool)
QScriptValue scriptShowGeometry(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->sceneViewSettings().showGeometry = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showInitialMesh(bool)
QScriptValue scriptShowInitialMesh(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->sceneViewSettings().showInitialMesh = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showSolutionMesh(bool)
QScriptValue scriptShowSolutionMesh(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->sceneViewSettings().showSolutionMesh = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showContours(bool)
QScriptValue scriptShowContours(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->sceneViewSettings().showContours = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showVectors(bool)
QScriptValue scriptShowVectors(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->sceneViewSettings().showVectors = context->argument(0).toBool();

    return engine->undefinedValue();
}

// showScalar(type = { 'none', 'scalar', 'scalar3d', 'order' })
QScriptValue scriptShowScalar(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argument(0).toString() == "none")
        m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_NONE;
    if (context->argument(0).toString() == "scalar")
        m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW;
    if (context->argument(0).toString() == "scalar3d")
        m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D;
    if (context->argument(0).toString() == "order")
        m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_ORDER;

    return engine->undefinedValue();
}
