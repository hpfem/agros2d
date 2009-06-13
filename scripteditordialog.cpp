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
    ProjectInfo projectInfo;

    m_scene->clear();
    m_scene->projectInfo() = projectInfo;
    m_scene->projectInfo().name = context->argument(0).toString();
    if (context->argument(1).toString() == "planar")
        m_scene->projectInfo().problemType = PROBLEMTYPE_PLANAR;
    else
        m_scene->projectInfo().problemType = PROBLEMTYPE_AXISYMMETRIC;
    m_scene->projectInfo().physicField = physicFieldFromStringKey(context->argument(2).toString());
    m_scene->projectInfo().numberOfRefinements = context->argument(3).toNumber();
    m_scene->projectInfo().polynomialOrder = context->argument(4).toNumber();
    m_scene->projectInfo().adaptivitySteps = context->argument(5).toNumber();
    m_scene->projectInfo().adaptivityTolerance = context->argument(6).toNumber();

    m_scene->refresh();
    return engine->undefinedValue();
}

// openDocument(filename)
QScriptValue scriptOpenDocument(QScriptContext *context, QScriptEngine *engine)
{
    m_scene->readFromFile(context->argument(0).toString());
}

// saveDocument(filename)
QScriptValue scriptSaveDocument(QScriptContext *context, QScriptEngine *engine)
{
    m_scene->writeToFile(context->argument(0).toString());
}

// addNode(x, y)
QScriptValue scriptAddNode(QScriptContext *context, QScriptEngine *engine)
{
    m_scene->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));
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
        foreach (SceneLabelMarker *labelMarker, m_scene->labelMarkers)
            if (labelMarker->name == context->argument(3).toString())
            {
        marker = labelMarker;
        break;
    }
    else
        marker = m_scene->labelMarkers[0];

    m_scene->addLabel(new SceneLabel(Point(context->argument(0).toNumber(), context->argument(1).toNumber()), marker, area));
    return engine->undefinedValue();
}

// addEdge(x1, y1, x2, y2, angle, marker)
QScriptValue scriptAddEdge(QScriptContext *context, QScriptEngine *engine)
{
    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(context->argument(0).toNumber(), context->argument(1).toNumber())));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(context->argument(2).toNumber(), context->argument(3).toNumber())));

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
        foreach (SceneEdgeMarker *edgeMarker, m_scene->edgeMarkers)
            if (edgeMarker->name == context->argument(5).toString())
            {             
        marker = edgeMarker;
        break;
    }
    else
        marker = m_scene->edgeMarkers[0];

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, marker, angle));
    return engine->undefinedValue();
}

// addBoundary(name, type, value, ...)
QScriptValue scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)
{ 
    PhysicFieldBC type;
    switch (m_scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL)) type = PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE)) type = PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE;
        m_scene->addEdgeMarker(new SceneEdgeElectrostaticMarker(context->argument(0).toString(), type, context->argument(2).toNumber()));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL)) type = PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT)) type = PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT;
        m_scene->addEdgeMarker(new SceneEdgeMagnetostaticMarker(context->argument(0).toString(), type, context->argument(2).toNumber()));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_HEAT_TEMPERATURE))
        {
            type = PHYSICFIELDBC_HEAT_TEMPERATURE;
            m_scene->addEdgeMarker(new SceneEdgeHeatMarker(context->argument(0).toString(), type, context->argument(2).toNumber()));
        }
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_HEAT_HEAT_FLUX))
        {
            type = PHYSICFIELDBC_HEAT_HEAT_FLUX;
            m_scene->addEdgeMarker(new SceneEdgeHeatMarker(context->argument(0).toString(), type,
                                                           context->argument(2).toNumber(),
                                                           context->argument(3).toNumber(),
                                                           context->argument(4).toNumber()));
        }
        break;
    case PHYSICFIELD_CURRENT:
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_CURRENT_POTENTIAL)) type = PHYSICFIELDBC_CURRENT_POTENTIAL;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW)) type = PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW;
        m_scene->addEdgeMarker(new SceneEdgeCurrentMarker(context->argument(0).toString(), type, context->argument(2).toNumber()));
        break;
    case PHYSICFIELD_ELASTICITY:
        PhysicFieldBC typeX, typeY;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FREE)) typeX = PHYSICFIELDBC_ELASTICITY_FREE;
        if (context->argument(1).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FIXED)) typeX = PHYSICFIELDBC_ELASTICITY_FIXED;
        if (context->argument(2).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FREE)) typeY = PHYSICFIELDBC_ELASTICITY_FREE;
        if (context->argument(2).toString() == physicFieldBCStringKey(PHYSICFIELDBC_ELASTICITY_FIXED)) typeY = PHYSICFIELDBC_ELASTICITY_FIXED;
                    m_scene->addEdgeMarker(new SceneEdgeElasticityMarker(context->argument(0).toString(), typeX, typeY,
                                                           context->argument(3).toNumber(),
                                                           context->argument(4).toNumber()));
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(m_scene->projectInfo().physicField).toStdString() + "' is not implemented. scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)" << endl;
        throw;
        break;
    }

    return engine->undefinedValue();
}

// addMaterial(name, type, value, ...)
QScriptValue scriptAddMaterial(QScriptContext *context, QScriptEngine *engine)
{
    PhysicFieldBC type;
    switch (m_scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        m_scene->addLabelMarker(new SceneLabelElectrostaticMarker(context->argument(0).toString(),
                                                                  context->argument(1).toNumber(),
                                                                  context->argument(2).toNumber()));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        m_scene->addLabelMarker(new SceneLabelMagnetostaticMarker(context->argument(0).toString(),
                                                                  context->argument(1).toNumber(),
                                                                  context->argument(2).toNumber()));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        m_scene->addLabelMarker(new SceneLabelHeatMarker(context->argument(0).toString(),
                                                         context->argument(1).toNumber(),
                                                         context->argument(2).toNumber()));
        break;
    case PHYSICFIELD_CURRENT:
        m_scene->addLabelMarker(new SceneLabelCurrentMarker(context->argument(0).toString(),
                                                            context->argument(1).toNumber()));
        break;
    case PHYSICFIELD_ELASTICITY:
        m_scene->addLabelMarker(new SceneLabelElasticityMarker(context->argument(0).toString(),
                                                               context->argument(2).toNumber(),
                                                               context->argument(3).toNumber()));
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(m_scene->projectInfo().physicField).toStdString() + "' is not implemented. scriptAddBoundary(QScriptContext *context, QScriptEngine *engine)" << endl;
        throw;
        break;
    }

    return engine->undefinedValue();
}

// solve()
QScriptValue scriptSolve(QScriptContext *context, QScriptEngine *engine)
{
    m_scene->createMeshAndSolve(SOLVER_MESH_AND_SOLVE);
    m_scene->refresh();
    return engine->undefinedValue();
}

// ***********************************************************************************************************

ScriptEditorDialog::ScriptEditorDialog(Scene *scene, QWidget *parent) : QDialog(parent)
{
    m_scene = scene;

    setWindowIcon(icon("script"));

    createControls();
    createEngine();

    resize(600, 400);
    // resize(sizeHint());
    // setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());

    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("ScriptEditorDialog/SplitterState", splitter->saveState()).toByteArray());

    doFileOpen("data/script/capacitor.js");
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

    tlbBar->addAction(actFileNew);
    tlbBar->addAction(actFileOpen);
    tlbBar->addAction(actFileSave);
    tlbBar->addSeparator();
    tlbBar->addAction(actRun);
    tlbBar->addAction(actCreateFromModel);

    splitter = new QSplitter(this);

    txtEditor = new QPlainTextEdit(this);
    txtEditor->setFont(QFont("Monospace", 10));
    txtEditor->setTabStopWidth(40);

    txtOutput = new QPlainTextEdit(this);
    txtOutput->setFont(QFont("Monospaced", 10));
    txtOutput->setReadOnly(true);

    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(txtEditor);
    splitter->addWidget(txtOutput);
    QSettings settings;
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/Splitter", splitter->saveGeometry()).toByteArray());

    // highlighter = new Highlighter(editor->document());

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tlbBar);
    layout->addWidget(splitter);

    setLayout(layout);
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
        m_fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Script files (*.js)"));

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
        m_fileName = QFileDialog::getSaveFileName(this, tr("Save file"), "data", tr("Script files (*.js)"));
    
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

void ScriptEditorDialog::doRun()
{
    txtOutput->clear();

    QScriptValue result = m_engine->evaluate(txtEditor->toPlainText(), m_fileName);
}

void ScriptEditorDialog::doCreateFromModel()
{
    QString str;

    // model
    str += "// model\n";
    str += QString("newDocument(\"%1\", \"%2\", \"%3\", %4, %5, %6, %7)").
           arg(m_scene->projectInfo().name).
           arg(problemTypeString(m_scene->projectInfo().problemType)).
           arg(physicFieldStringKey(m_scene->projectInfo().physicField)).
           arg(m_scene->projectInfo().numberOfRefinements).
           arg(m_scene->projectInfo().polynomialOrder).
           arg(m_scene->projectInfo().adaptivitySteps).
           arg(m_scene->projectInfo().adaptivityTolerance) + "\n";
    str += "\n";

    // boundaries
    str += "// boundaries\n";
    for (int i = 1; i<m_scene->edgeMarkers.count(); i++)
    {
        str += m_scene->edgeMarkers[i]->script() + "\n";
    }
    str += "\n";

    // materials
    str += "// materials\n";
    for (int i = 1; i<m_scene->labelMarkers.count(); i++)
    {
        str += m_scene->labelMarkers[i]->script() + "\n";
    }
    str += "\n";

    // edges
    str += "// edges\n";
    for (int i = 0; i<m_scene->edges.count(); i++)
    {
        str += QString("addEdge(%1, %2, %3, %4, %5, \"%6\")").
               arg(m_scene->edges[i]->nodeStart->point.x).
               arg(m_scene->edges[i]->nodeStart->point.y).
               arg(m_scene->edges[i]->nodeEnd->point.x).
               arg(m_scene->edges[i]->nodeEnd->point.y).
               arg(m_scene->edges[i]->angle).
               arg(m_scene->edges[i]->marker->name) + "\n";
    }
    str += "\n";

    // labels
    str += "// labels\n";
    for (int i = 0; i<m_scene->labels.count(); i++)
    {
        str += QString("addLabel(%1, %2, %3, \"%4\")").
               arg(m_scene->labels[i]->point.x).
               arg(m_scene->labels[i]->point.y).
               arg(m_scene->labels[i]->area).
               arg(m_scene->labels[i]->marker->name) + "\n";
    }

    txtEditor->setPlainText(str);
}
