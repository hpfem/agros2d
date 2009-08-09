#include "scripteditordialog.h"

// print(text)
QScriptValue scriptPrint(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(calleeData.toQObject());
    edit->appendPlainText(result);

    return engine->undefinedValue();
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
        // find marker by name
        foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
            if (labelMarker->name == context->argument(3).toString())
            {
        marker = labelMarker;
        break;
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
        // find marker by name
        foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
            if (edgeMarker->name == context->argument(5).toString())
            {
        marker = edgeMarker;
        break;
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

// solve()
QScriptValue scriptSolve(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->createMeshAndSolve(SOLVER_MESH_AND_SOLVE);
    Util::scene()->refresh();

    return engine->undefinedValue();
}

// zoomBestFit()
QScriptValue scriptZoomBestFit(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->doZoomBestFit();

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

// ***********************************************************************************************************

ScriptEditorWidget::ScriptEditorWidget(QWidget *parent) : QWidget(parent)
{
    file = "";

    txtEditor = new ScriptEditor(this);
    txtOutput = new QPlainTextEdit(this);
    splitter = new QSplitter(this);

    createControls();
    createEngine();

    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("ScriptEditorDialog/SplitterState", splitter->saveState()).toByteArray());
}

ScriptEditorWidget::~ScriptEditorWidget()
{
    QSettings settings;
    settings.setValue("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("ScriptEditorDialog/SplitterState", splitter->saveState());

    delete txtEditor;
    delete txtOutput;
    delete splitter;
}

void ScriptEditorWidget::createControls()
{
    txtOutput->setFont(QFont("Monospaced", 10));
    txtOutput->setReadOnly(true);

    // contents
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(txtEditor);
    splitter->addWidget(txtOutput);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(splitter);

    setLayout(layout);

    QSettings settings;
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/Splitter", splitter->saveGeometry()).toByteArray());
}

void ScriptEditorWidget::createEngine()
{

}

void ScriptEditorWidget::doRun()
{
    QScriptEngine engine;

    // scene
    QScriptValue sceneValue = engine.newQObject(Util::scene());
    engine.globalObject().setProperty("scene", sceneValue);

    // scene view
    QScriptValue sceneViewValue = engine.newQObject(m_sceneView);
    engine.globalObject().setProperty("sceneView", sceneViewValue);

    // print
    QScriptValue funPrint = engine.newFunction(scriptPrint);
    funPrint.setData(engine.newQObject(txtOutput));
    engine.globalObject().setProperty("print", funPrint);

    engine.globalObject().setProperty("include", engine.newFunction(scriptInclude));
    engine.globalObject().setProperty("printToFile", engine.newFunction(scriptPrintToFile));

    engine.globalObject().setProperty("newDocument", engine.newFunction(scriptNewDocument));
    engine.globalObject().setProperty("openDocument", engine.newFunction(scriptOpenDocument));
    engine.globalObject().setProperty("saveDocument", engine.newFunction(scriptSaveDocument));

    engine.globalObject().setProperty("addNode", engine.newFunction(scriptAddNode));
    engine.globalObject().setProperty("addEdge", engine.newFunction(scriptAddEdge));
    engine.globalObject().setProperty("addLabel", engine.newFunction(scriptAddLabel));

    engine.globalObject().setProperty("addBoundary", engine.newFunction(scriptAddBoundary));
    engine.globalObject().setProperty("addMaterial", engine.newFunction(scriptAddMaterial));

    engine.globalObject().setProperty("solve", engine.newFunction(scriptSolve));
    engine.globalObject().setProperty("zoomBestFit", engine.newFunction(scriptZoomBestFit));

    engine.globalObject().setProperty("mode", engine.newFunction(scriptMode));

    engine.globalObject().setProperty("selectNone", engine.newFunction(scriptSelectNone));
    engine.globalObject().setProperty("selectAll", engine.newFunction(scriptSelectAll));
    engine.globalObject().setProperty("selectNode", engine.newFunction(scriptSelectNode));
    engine.globalObject().setProperty("selectEdge", engine.newFunction(scriptSelectEdge));
    engine.globalObject().setProperty("selectLabel", engine.newFunction(scriptSelectLabel));

    engine.globalObject().setProperty("moveSelection", engine.newFunction(scriptMoveSelection));
    engine.globalObject().setProperty("rotateSelection", engine.newFunction(scriptRotateSelection));
    engine.globalObject().setProperty("scaleSelection", engine.newFunction(scriptScaleSelection));

    engine.globalObject().setProperty("pointResult", engine.newFunction(scriptPointResult));
    engine.globalObject().setProperty("volumeIntegral", engine.newFunction(scriptVolumeIntegral));
    engine.globalObject().setProperty("surfaceIntegral", engine.newFunction(scriptSurfaceIntegral));

    // run
    txtOutput->clear();

    // check syntax
    QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(txtEditor->toPlainText());

    if (syntaxResult.state() == QScriptSyntaxCheckResult::Valid)
    {
        Util::scene()->blockSignals(true);
        // startup script
        engine.evaluate(Util::scene()->problemInfo().scriptStartup);
        // result
        QScriptValue result = engine.evaluate(txtEditor->toPlainText(), file);
        Util::scene()->blockSignals(false);
        Util::scene()->refresh();
    }
    else
    {
        txtOutput->appendPlainText(tr("Error: %1 (line %2, column %3)").arg(syntaxResult.errorMessage()).arg(syntaxResult.errorLineNumber()).arg(syntaxResult.errorColumnNumber()));
    }
}

void ScriptEditorWidget::doCreateFromModel()
{
    QString str;

    // model
    str += "// model\n";
    str += QString("newDocument(\"%1\", \"%2\", \"%3\", %4, %5, %6, \"%7\", %8, %9);").
           arg(Util::scene()->problemInfo().name).
           arg(problemTypeStringKey(Util::scene()->problemInfo().problemType)).
           arg(physicFieldStringKey(Util::scene()->problemInfo().physicField)).
           arg(Util::scene()->problemInfo().numberOfRefinements).
           arg(Util::scene()->problemInfo().polynomialOrder).
           arg(Util::scene()->problemInfo().frequency).
           arg(adaptivityTypeStringKey(Util::scene()->problemInfo().adaptivityType)).
           arg(Util::scene()->problemInfo().adaptivitySteps).
           arg(Util::scene()->problemInfo().adaptivityTolerance) + "\n";
    str += "\n";

    // boundaries
    str += "// boundaries\n";
    for (int i = 1; i<Util::scene()->edgeMarkers.count(); i++)
    {
        str += Util::scene()->edgeMarkers[i]->script() + "\n";
    }
    str += "\n";

    // materials
    str += "// materials\n";
    for (int i = 1; i<Util::scene()->labelMarkers.count(); i++)
    {
        str += Util::scene()->labelMarkers[i]->script() + "\n";
    }
    str += "\n";

    // edges
    str += "// edges\n";
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        str += QString("addEdge(%1, %2, %3, %4, %5, \"%6\");").
               arg(Util::scene()->edges[i]->nodeStart->point.x).
               arg(Util::scene()->edges[i]->nodeStart->point.y).
               arg(Util::scene()->edges[i]->nodeEnd->point.x).
               arg(Util::scene()->edges[i]->nodeEnd->point.y).
               arg(Util::scene()->edges[i]->angle).
               arg(Util::scene()->edges[i]->marker->name) + "\n";
    }
    str += "\n";

    // labels
    str += "// labels\n";
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        str += QString("addLabel(%1, %2, %3, \"%4\");").
               arg(Util::scene()->labels[i]->point.x).
               arg(Util::scene()->labels[i]->point.y).
               arg(Util::scene()->labels[i]->area).
               arg(Util::scene()->labels[i]->marker->name) + "\n";
    }

    txtEditor->setPlainText(str);
}

// ***********************************************************************************************************


ScriptEditorDialog::ScriptEditorDialog(SceneView *sceneView, QWidget *parent) : QMainWindow(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());

    m_sceneView = sceneView;

    setWindowIcon(icon("script"));
    setWindowTitle(tr("Script editor"));

    createActions();
    createControls();

    setMinimumSize(600, 400);
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    QSettings settings;
    settings.setValue("ScriptEditorDialog/Geometry", saveGeometry());   
}

void ScriptEditorDialog::showDialog()
{
    show();
}

void ScriptEditorDialog::createActions()
{
    actFileNew = new QAction(icon("document-new"), tr("&New"), this);
    actFileNew->setShortcuts(QKeySequence::New);
    connect(actFileNew, SIGNAL(triggered()), this, SLOT(doFileNew()));

    actFileOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actFileOpen->setShortcuts(QKeySequence::Open);
    connect(actFileOpen, SIGNAL(triggered()), this, SLOT(doFileOpen()));

    actFileSave = new QAction(icon("document-save"), tr("&Save"), this);
    actFileSave->setShortcuts(QKeySequence::Save);
    connect(actFileSave, SIGNAL(triggered()), this, SLOT(doFileSave()));

    actFileSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actFileSaveAs, SIGNAL(triggered()), this, SLOT(doFileSaveAs()));

    actFileClose = new QAction(icon(""), tr("Close"), this);
    actFileClose->setShortcuts(QKeySequence::Close);
    connect(actFileClose, SIGNAL(triggered()), this, SLOT(doFileClose()));

    actUndo = new QAction(icon("edit-undo"), tr("&Undo"), this);
    actUndo->setShortcut(QKeySequence::Undo);

    actRedo = new QAction(icon("edit-redo"), tr("&Redo"), this);
    actRedo->setShortcut(QKeySequence::Redo);

    actCut = new QAction(icon("edit-cut"), tr("Cu&t"), this);
    actCut->setShortcut(QKeySequence::Cut);
    actCut->setEnabled(false);

    actCopy = new QAction(icon("edit-copy"), tr("&Copy"), this);
    actCopy->setShortcut(QKeySequence::Copy);
    actCopy->setEnabled(false);

    actPaste = new QAction(icon("edit-paste"), tr("&Paste"), this);
    actPaste->setShortcut(QKeySequence::Paste);

    actRun = new QAction(icon("system-run"), tr("&Run"), this);
    actRun->setShortcut(QKeySequence(tr("Ctrl+R")));
    
    actCreateFromModel = new QAction(icon("script-create"), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit script editor"));
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actHelp = new QAction(icon("help-browser"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));
}

void ScriptEditorDialog::createControls()
{
    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actFileNew);
    mnuFile->addAction(actFileOpen);
    mnuFile->addAction(actFileSave);
    mnuFile->addAction(actFileSaveAs);
    mnuFile->addSeparator();
    mnuFile->addAction(actFileClose);
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);

    mnuEdit = menuBar()->addMenu(tr("&Edit"));
    mnuEdit->addAction(actUndo);
    mnuEdit->addAction(actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCut);
    mnuEdit->addAction(actCopy);
    mnuEdit->addAction(actPaste);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actRun);
    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);

    tlbFile = addToolBar(tr("File"));
    tlbFile->addAction(actFileNew);
    tlbFile->addAction(actFileOpen);
    tlbFile->addAction(actFileSave);
    tlbEdit = addToolBar(tr("Edit"));
    tlbEdit->addAction(actCut);
    tlbEdit->addAction(actCopy);
    tlbEdit->addAction(actPaste);
    tlbEdit->addSeparator();
    tlbEdit->addAction(actUndo);
    tlbEdit->addAction(actRedo);
    tlbTools = addToolBar(tr("Tools"));
    tlbTools->addAction(actRun);
    tlbTools->addSeparator();
    tlbTools->addAction(actCreateFromModel);

    // contents
    tabWidget = new QTabWidget;
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);

    QToolButton *btnNewTab = new QToolButton(this);
    btnNewTab->setAutoRaise(true);
    btnNewTab->setToolTip(tr("Add new page"));
    btnNewTab->setIcon(icon("tabadd"));
    tabWidget->setCornerWidget(btnNewTab, Qt::TopLeftCorner);
    connect(btnNewTab, SIGNAL(clicked()), this, SLOT(doFileNew()));

    doFileNew();

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(doCloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(doCurrentPageChanged(int)));

    // main widget
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(tabWidget);
    layout->setMargin(6);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    setCentralWidget(widget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(doDataChanged()));
}

void ScriptEditorDialog::doFileNew()
{
    tabWidget->addTab(new ScriptEditorWidget(this), tr("unnamed"));
    tabWidget->setCurrentIndex(tabWidget->count()-1);
    doCurrentPageChanged(tabWidget->count()-1);
}

void ScriptEditorDialog::doFileOpen(const QString &file)
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    // open dialog
    QString fileName = file;
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "data", tr("Agros2D script files (*.qs)"));

    // read text
    if (!fileName.isEmpty()) {
        // check empty document
        if (!scriptEditorWidget->txtEditor->toPlainText().isEmpty())
            doFileNew();

        scriptEditorWidget->file = fileName;
        QFile fileName(scriptEditorWidget->file);
        if (fileName.open(QFile::ReadOnly | QFile::Text))
            txtEditor->setPlainText(fileName.readAll());

        QFileInfo fileInfo(scriptEditorWidget->file);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
    }    
}

void ScriptEditorDialog::doFileSave()
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    // open dialog
    if (scriptEditorWidget->file.isEmpty())
        scriptEditorWidget->file = QFileDialog::getSaveFileName(this, tr("Save file"), "data", tr("Agros2D script files (*.qs)"));

    // write text
    if (!scriptEditorWidget->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget->file);
        if (fileInfo.suffix() != "qs") scriptEditorWidget->file += ".qs";

        QFile fileName(dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget())->file);
        if (fileName.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&fileName);
            out << txtEditor->toPlainText();
            fileName.close();
        }

        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
    }
}

void ScriptEditorDialog::doFileSaveAs()
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    scriptEditorWidget->file = QFileDialog::getSaveFileName(this, tr("Save file"), "data", tr("Agros2D script files (*.qs)"));
    doFileSave();
}

void ScriptEditorDialog::doFileClose()
{
    if (tabWidget->count() > 1)
        doCloseTab(tabWidget->currentIndex());
    else
        hide();
}

void ScriptEditorDialog::doDataChanged()
{
    actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void ScriptEditorDialog::doHelp()
{
    Util::helpDialog()->showPage("scripting/scripting.html");
    Util::helpDialog()->show();
}

void ScriptEditorDialog::doCloseTab(int index)
{
    tabWidget->removeTab(index);
}

void ScriptEditorDialog::doCurrentPageChanged(int index)
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    txtEditor = scriptEditorWidget->txtEditor;
    txtOutput = scriptEditorWidget->txtOutput;

    actRun->disconnect();
    connect(actRun, SIGNAL(triggered()), scriptEditorWidget, SLOT(doRun()));
    actCreateFromModel->disconnect();
    connect(actCreateFromModel, SIGNAL(triggered()), scriptEditorWidget, SLOT(doCreateFromModel()));

    actCut->disconnect();
    connect(actCut, SIGNAL(triggered()), txtEditor, SLOT(cut()));
    actCopy->disconnect();
    connect(actCopy, SIGNAL(triggered()), txtEditor, SLOT(copy()));
    actPaste->disconnect();
    connect(actPaste, SIGNAL(triggered()), txtEditor, SLOT(paste()));
    actUndo->disconnect();
    connect(actUndo, SIGNAL(triggered()), txtEditor, SLOT(undo()));
    actRedo->disconnect();
    connect(actRedo, SIGNAL(triggered()), txtEditor, SLOT(redo()));

    txtEditor->document()->disconnect(actUndo);
    txtEditor->document()->disconnect(actRedo);
    connect(txtEditor->document(), SIGNAL(undoAvailable(bool)), actUndo, SLOT(setEnabled(bool)));
    connect(txtEditor->document(), SIGNAL(redoAvailable(bool)), actRedo, SLOT(setEnabled(bool)));
    txtEditor->disconnect(actCut);
    txtEditor->disconnect(actCopy);
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCut, SLOT(setEnabled(bool)));
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCopy, SLOT(setEnabled(bool)));

    actUndo->setEnabled(txtEditor->document()->isUndoAvailable());
    actRedo->setEnabled(txtEditor->document()->isRedoAvailable());

    tabWidget->setTabsClosable(tabWidget->count() > 1);
    tabWidget->cornerWidget(Qt::TopLeftCorner)->setEnabled(true);
}

// ******************************************************************************************************

ScriptStartupDialog::ScriptStartupDialog(ProblemInfo &problemInfo, QWidget *parent) : QDialog(parent)
{
    m_problemInfo = &problemInfo;

    setWindowIcon(icon("script-startup"));
    setWindowFlags(Qt::Window);
    setWindowTitle(tr("Startup script"));
    setMinimumSize(400, 300);

    createControls();
}

ScriptStartupDialog::~ScriptStartupDialog()
{
    delete txtEditor;
}

int ScriptStartupDialog::showDialog()
{
    return exec();
}

void ScriptStartupDialog::createControls()
{
    txtEditor = new ScriptEditor(this);
    txtEditor->setPlainText(Util::scene()->problemInfo().scriptStartup);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtEditor);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void ScriptStartupDialog::doAccept()
{
    QScriptEngine engine;

    if (engine.canEvaluate(txtEditor->toPlainText()))
    {
        Util::scene()->problemInfo().scriptStartup = txtEditor->toPlainText();
        accept();
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("Script cannot be evaluated."));
    }
}

void ScriptStartupDialog::doReject()
{
    reject();
}

// ******************************************************************************************************

ScriptEditor::ScriptEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new ScriptEditorLineNumberArea(this);

    setFont(QFont("Monospace", 10));
    setTabStopWidth(40);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    // highlighter
    new QScriptSyntaxHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(doUpdateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(doUpdateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(doHighlightCurrentLine()));

    doUpdateLineNumberAreaWidth(0);
    doHighlightCurrentLine();
}

ScriptEditor::~ScriptEditor()
{
    delete lineNumberArea;
}

int ScriptEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::doUpdateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::doUpdateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        doUpdateLineNumberAreaWidth(0);
}

void ScriptEditor::doHighlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
