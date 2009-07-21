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

// newDocument(name, type, physicfield, numberofrefinements, polynomialorder, adaptivitysteps, adaptivitytolerance)
QScriptValue scriptNewDocument(QScriptContext *context, QScriptEngine *engine)
{
    ProblemInfo problemInfo;

    Util::scene()->clear();
    Util::scene()->problemInfo() = problemInfo;
    Util::scene()->problemInfo().name = context->argument(0).toString();
    if (context->argument(1).toString() == "planar")
        Util::scene()->problemInfo().problemType = PROBLEMTYPE_PLANAR;
    else
        Util::scene()->problemInfo().problemType = PROBLEMTYPE_AXISYMMETRIC;
    Util::scene()->problemInfo().physicField = physicFieldFromStringKey(context->argument(2).toString());
    Util::scene()->problemInfo().numberOfRefinements = context->argument(3).toNumber();
    Util::scene()->problemInfo().polynomialOrder = context->argument(4).toNumber();
    Util::scene()->problemInfo().adaptivitySteps = context->argument(5).toNumber();
    Util::scene()->problemInfo().adaptivityTolerance = context->argument(6).toNumber();

    Util::scene()->refresh();
    return engine->undefinedValue();
}

// openDocument(filename)
QScriptValue scriptOpenDocument(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->readFromFile(context->argument(0).toString());
}

// saveDocument(filename)
QScriptValue scriptSaveDocument(QScriptContext *context, QScriptEngine *engine)
{
    Util::scene()->writeToFile(context->argument(0).toString());
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
    m_sceneView->actSceneModeNode->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
        Util::scene()->nodes[context->argument(i).toNumber()]->isSelected = true;

    m_sceneView->doInvalidated();

    return engine->undefinedValue();
}

// selectEdge()
QScriptValue scriptSelectEdge(QScriptContext *context, QScriptEngine *engine)
{
    m_sceneView->actSceneModeEdge->trigger();
    for (int i = 0; i<context->argumentCount(); i++)
        Util::scene()->edges[context->argument(i).toNumber()]->isSelected = true;

    m_sceneView->doInvalidated();

    return engine->undefinedValue();
}

// selectLabel()
QScriptValue scriptSelectLabel(QScriptContext *context, QScriptEngine *engine)
{
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

// result = volumeIntegral(index ...)
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

// ***********************************************************************************************************

ScriptEditorDialog::ScriptEditorDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setWindowIcon(icon("script"));

    createControls();
    createEngine();

    resize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("ScriptEditorDialog/SplitterState", splitter->saveState()).toByteArray());
    // doFileOpen("data/script/capacitor.qs");
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    QSettings settings;
    settings.setValue("ScriptEditorDialog/Geometry", saveGeometry());
    settings.setValue("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("ScriptEditorDialog/SplitterState", splitter->saveState());

    delete txtEditor;
    delete txtOutput;
    delete m_engine;
    delete splitter;
}

void ScriptEditorDialog::showDialog()
{
    show();
}

void ScriptEditorDialog::createControls()
{
    QToolBar *tlbBar = new QToolBar("Script editor", this);

    actFileNew = new QAction(icon("document-new"), tr("&New"), this);
    actFileNew->setShortcuts(QKeySequence::New);
    connect(actFileNew, SIGNAL(triggered()), this, SLOT(doFileNew()));

    actFileOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actFileOpen->setShortcuts(QKeySequence::Open);
    connect(actFileOpen, SIGNAL(triggered()), this, SLOT(doFileOpen()));

    actFileSave = new QAction(icon("document-save"), tr("&Save"), this);
    actFileSave->setShortcuts(QKeySequence::Save);
    connect(actFileSave, SIGNAL(triggered()), this, SLOT(doFileSave()));

    // actFileSaveAs = new QAction(tr("Save &As..."), this);
    // actFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    // connect(actFileSaveAs, SIGNAL(triggered()), this, SLOT(doFileSaveAs()));

    actRun = new QAction(icon("system-run"), tr("Run"), this);
    actRun->setShortcut(QKeySequence(tr("Ctrl+R")));
    connect(actRun, SIGNAL(triggered()), this, SLOT(doRun()));

    actCreateFromModel = new QAction(icon("script-create"), tr("Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreateFromModel()));

    actHelp = new QAction(icon("help-browser"), tr("Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    tlbBar->addAction(actFileNew);
    tlbBar->addAction(actFileOpen);
    tlbBar->addAction(actFileSave);
    tlbBar->addSeparator();
    tlbBar->addAction(actRun);
    tlbBar->addAction(actCreateFromModel);
    tlbBar->addSeparator();
    tlbBar->addAction(actHelp);

    splitter = new QSplitter(this);

    txtEditor = new ScriptEditor(this);

    txtOutput = new QPlainTextEdit(this);
    txtOutput->setFont(QFont("Monospaced", 10));
    txtOutput->setReadOnly(true);

    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(txtEditor);
    splitter->addWidget(txtOutput);
    QSettings settings;
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/Splitter", splitter->saveGeometry()).toByteArray());

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tlbBar);
    layout->addWidget(splitter);

    setLayout(layout);

    doFileOpen("data/script/capacitor.qs");
}

void ScriptEditorDialog::createEngine()
{
    m_engine = new QScriptEngine();

    // print
    QScriptValue funPrint = m_engine->newFunction(scriptPrint);
    funPrint.setData(m_engine->newQObject(txtOutput));
    m_engine->globalObject().setProperty("print", funPrint);

    m_engine->globalObject().setProperty("newDocument", m_engine->newFunction(scriptNewDocument));
    m_engine->globalObject().setProperty("openDocument", m_engine->newFunction(scriptOpenDocument));
    m_engine->globalObject().setProperty("saveDocument", m_engine->newFunction(scriptSaveDocument));

    m_engine->globalObject().setProperty("addNode", m_engine->newFunction(scriptAddNode));
    m_engine->globalObject().setProperty("addEdge", m_engine->newFunction(scriptAddEdge));
    m_engine->globalObject().setProperty("addLabel", m_engine->newFunction(scriptAddLabel));

    m_engine->globalObject().setProperty("addBoundary", m_engine->newFunction(scriptAddBoundary));
    m_engine->globalObject().setProperty("addMaterial", m_engine->newFunction(scriptAddMaterial));

    m_engine->globalObject().setProperty("solve", m_engine->newFunction(scriptSolve));
    m_engine->globalObject().setProperty("zoomBestFit", m_engine->newFunction(scriptZoomBestFit));

    m_engine->globalObject().setProperty("mode", m_engine->newFunction(scriptMode));

    m_engine->globalObject().setProperty("selectNone", m_engine->newFunction(scriptSelectNone));
    m_engine->globalObject().setProperty("selectAll", m_engine->newFunction(scriptSelectAll));
    m_engine->globalObject().setProperty("selectNode", m_engine->newFunction(scriptSelectNode));
    m_engine->globalObject().setProperty("selectEdge", m_engine->newFunction(scriptSelectEdge));
    m_engine->globalObject().setProperty("selectLabel", m_engine->newFunction(scriptSelectLabel));

    m_engine->globalObject().setProperty("moveSelection", m_engine->newFunction(scriptMoveSelection));
    m_engine->globalObject().setProperty("rotateSelection", m_engine->newFunction(scriptRotateSelection));
    m_engine->globalObject().setProperty("scaleSelection", m_engine->newFunction(scriptScaleSelection));

    m_engine->globalObject().setProperty("pointResult", m_engine->newFunction(scriptPointResult));
    m_engine->globalObject().setProperty("volumeIntegral", m_engine->newFunction(scriptVolumeIntegral));
}

void ScriptEditorDialog::doFileNew()
{
    m_fileName = "";
    txtEditor->clear();

    setWindowTitle(tr("Script editor") + " - " + tr("untitled"));
}

void ScriptEditorDialog::doFileOpen(const QString &fileName)
{
    m_fileName = fileName;

    if (m_fileName.isEmpty())
        m_fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Script files (*.qs)"));

    if (!m_fileName.isEmpty()) {
        QFile file(m_fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            txtEditor->setPlainText(file.readAll());
    }

    setWindowTitle(tr("Script editor") + " - " + m_fileName);
}

void ScriptEditorDialog::doFileSave()
{
    if (m_fileName.isEmpty())
        m_fileName = QFileDialog::getSaveFileName(this, tr("Save file"), "data", tr("Script files (*.qs)"));

    if (!m_fileName.isEmpty())
    {
        QFile file(m_fileName);
        if (file.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&file);
            out << txtEditor->toPlainText();
            file.close();
        }
    }
}

void ScriptEditorDialog::doCreateFromModel()
{
    QString str;

    // model
    str += "// model\n";
    str += QString("newDocument(\"%1\", \"%2\", \"%3\", %4, %5, %6, %7);").
           arg(Util::scene()->problemInfo().name).
           arg(problemTypeStringKey(Util::scene()->problemInfo().problemType)).
           arg(physicFieldStringKey(Util::scene()->problemInfo().physicField)).
           arg(Util::scene()->problemInfo().numberOfRefinements).
           arg(Util::scene()->problemInfo().polynomialOrder).
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

void ScriptEditorDialog::doHelp()
{
    Util::helpDialog()->showPage("script/script.html");
    Util::helpDialog()->show();
}

void eval(const QString &text)
{
}

void ScriptEditorDialog::doRun()
{
    txtOutput->clear();

    // check syntax
    QScriptSyntaxCheckResult syntaxResult = m_engine->checkSyntax(txtEditor->toPlainText());

    if (syntaxResult.state() == QScriptSyntaxCheckResult::Valid)
    {
        Util::scene()->blockSignals(true);
        // startup script
        m_engine->evaluate(Util::scene()->problemInfo().scriptStartup);
        // result
        QScriptValue result = m_engine->evaluate(txtEditor->toPlainText(), m_fileName);
        Util::scene()->blockSignals(false);
        Util::scene()->refresh();
    }
    else
    {
        txtOutput->appendPlainText(tr("Error: %1 (line %2, column %3)").arg(syntaxResult.errorMessage()).arg(syntaxResult.errorLineNumber()).arg(syntaxResult.errorColumnNumber()));
        // txtOutput->setExtraSelections(
    }
}

// ******************************************************************************************************

ScriptStartupDialog::ScriptStartupDialog(ProblemInfo &problemInfo, QWidget *parent) : QDialog(parent)
{
    m_problemInfo = &problemInfo;

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
