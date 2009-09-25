#include "scripteditordialog.h"
#include "scripteditorcommandsecma.cpp"

QScriptEngine *m_engine;

QString runEcma(const QString &script)
{
    if (!m_engine)
        m_engine = scriptEngine();

    if (script.isEmpty())
        return "";

    // scene
    QScriptValue sceneValue = m_engine->newQObject(Util::scene());
    m_engine->globalObject().setProperty("scene", sceneValue);

    // scene view
    QScriptValue sceneViewValue = m_engine->newQObject(m_sceneView);
    m_engine->globalObject().setProperty("sceneView", sceneViewValue);

    // general functions
    // print
    QScriptValue funPrint = m_engine->newFunction(scriptPrint);
    m_engine->globalObject().setProperty("print", funPrint);

    m_engine->globalObject().setProperty("version", m_engine->newFunction(scriptVersion));
    m_engine->globalObject().setProperty("message", m_engine->newFunction(scriptMessage));
    m_engine->globalObject().setProperty("input", m_engine->newFunction(scriptInput));
    m_engine->globalObject().setProperty("quit", m_engine->newFunction(scriptQuit));

    m_engine->globalObject().setProperty("include", m_engine->newFunction(scriptInclude));
    m_engine->globalObject().setProperty("printToFile", m_engine->newFunction(scriptPrintToFile));

    // document
    m_engine->globalObject().setProperty("newDocument", m_engine->newFunction(scriptNewDocument));
    m_engine->globalObject().setProperty("openDocument", m_engine->newFunction(scriptOpenDocument));
    m_engine->globalObject().setProperty("saveDocument", m_engine->newFunction(scriptSaveDocument));

    m_engine->globalObject().setProperty("mode", m_engine->newFunction(scriptMode));

    // geometry
    m_engine->globalObject().setProperty("addNode", m_engine->newFunction(scriptAddNode));
    m_engine->globalObject().setProperty("addEdge", m_engine->newFunction(scriptAddEdge));
    m_engine->globalObject().setProperty("addLabel", m_engine->newFunction(scriptAddLabel));

    m_engine->globalObject().setProperty("selectNone", m_engine->newFunction(scriptSelectNone));
    m_engine->globalObject().setProperty("selectAll", m_engine->newFunction(scriptSelectAll));
    m_engine->globalObject().setProperty("selectNode", m_engine->newFunction(scriptSelectNode));
    m_engine->globalObject().setProperty("selectEdge", m_engine->newFunction(scriptSelectEdge));
    m_engine->globalObject().setProperty("selectLabel", m_engine->newFunction(scriptSelectLabel));
    m_engine->globalObject().setProperty("selectNodePoint", m_engine->newFunction(scriptSelectNodePoint));
    m_engine->globalObject().setProperty("selectEdgePoint", m_engine->newFunction(scriptSelectEdgePoint));
    m_engine->globalObject().setProperty("selectLabelPoint", m_engine->newFunction(scriptSelectLabelPoint));

    m_engine->globalObject().setProperty("moveSelection", m_engine->newFunction(scriptMoveSelection));
    m_engine->globalObject().setProperty("rotateSelection", m_engine->newFunction(scriptRotateSelection));
    m_engine->globalObject().setProperty("scaleSelection", m_engine->newFunction(scriptScaleSelection));
    m_engine->globalObject().setProperty("deleteSelection", m_engine->newFunction(scriptDeleteSelection));

    m_engine->globalObject().setProperty("zoomBestFit", m_engine->newFunction(scriptZoomBestFit));
    m_engine->globalObject().setProperty("zoomIn", m_engine->newFunction(scriptZoomIn));
    m_engine->globalObject().setProperty("zoomOut", m_engine->newFunction(scriptZoomOut));
    m_engine->globalObject().setProperty("zoomRegion", m_engine->newFunction(scriptZoomRegion));

    // materials and boundaries
    m_engine->globalObject().setProperty("addBoundary", m_engine->newFunction(scriptAddBoundary));
    m_engine->globalObject().setProperty("addMaterial", m_engine->newFunction(scriptAddMaterial));

    // solver    
    m_engine->globalObject().setProperty("mesh", m_engine->newFunction(scriptMesh));
    m_engine->globalObject().setProperty("solve", m_engine->newFunction(scriptSolve));

    // postprocessor
    m_engine->globalObject().setProperty("pointResult", m_engine->newFunction(scriptPointResult));
    m_engine->globalObject().setProperty("volumeIntegral", m_engine->newFunction(scriptVolumeIntegral));
    m_engine->globalObject().setProperty("surfaceIntegral", m_engine->newFunction(scriptSurfaceIntegral));
    m_engine->globalObject().setProperty("showGrid", m_engine->newFunction(scriptShowGrid));
    m_engine->globalObject().setProperty("showGeometry", m_engine->newFunction(scriptShowGeometry));
    m_engine->globalObject().setProperty("showInitialMesh", m_engine->newFunction(scriptShowInitialMesh));
    m_engine->globalObject().setProperty("showSolutionMesh", m_engine->newFunction(scriptShowSolutionMesh));
    m_engine->globalObject().setProperty("showContours", m_engine->newFunction(scriptShowContours));
    m_engine->globalObject().setProperty("showVectors", m_engine->newFunction(scriptShowVectors));
    m_engine->globalObject().setProperty("showScalar", m_engine->newFunction(scriptShowScalar));

    // check syntax
    QScriptSyntaxCheckResult syntaxResult = m_engine->checkSyntax(script);

    if (syntaxResult.state() == QScriptSyntaxCheckResult::Valid)
    {
        Util::scene()->undoStack()->setActive(false);
        Util::scene()->blockSignals(true);
        // startup script
        m_engine->evaluate(Util::scene()->problemInfo().scriptStartup);
        // result
        QScriptValue result = m_engine->evaluate(script);
        Util::scene()->blockSignals(false);
        Util::scene()->refresh();
        Util::scene()->undoStack()->setActive(true);

        return funPrint.data().toString().trimmed();
    }
    else
    {
        return QObject::tr("Error: %1 (line %2, column %3)").arg(syntaxResult.errorMessage()).arg(syntaxResult.errorLineNumber()).arg(syntaxResult.errorColumnNumber());
    }

    return "";
}

QString createEcmaFromModel()
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

    return str;
}

ScriptEngineRemote::ScriptEngineRemote()
{
    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer("agros2d-server");
    if (!m_server->listen("agros2d-server"))
    {
        cout << tr("Error: Unable to start the server (agros2d-server): %1.").arg(m_server->errorString()).toStdString() << endl;
        return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(connected()));
}

ScriptEngineRemote::~ScriptEngineRemote()
{
    delete m_server;
    delete m_client_socket;
}

void ScriptEngineRemote::connected()
{
    command = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));   
}

void ScriptEngineRemote::readCommand()
{
    QTextStream in(m_server_socket);
    command = in.readAll();
}

void ScriptEngineRemote::disconnected()
{
    m_server_socket->deleteLater();

    QString result = "";
    if (!command.isEmpty())
    {
        result = runEcma(command);
    }

    m_client_socket = new QLocalSocket();
    connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

    m_client_socket->connectToServer("agros2d-client");
    if (m_client_socket->waitForConnected(1000))
    {
        QTextStream out(m_client_socket);
        out << result;
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
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        cout << tr("Server error: The host was not found.").toStdString() << endl;
        break;
    case QLocalSocket::ConnectionRefusedError:
        cout << tr("Server error: The connection was refused by the peer. Make sure the agros2d-client server is running.").toStdString() << endl;
        break;
    default:
        cout << tr("Server error: The following error occurred: %1.").arg(m_client_socket->errorString()).toStdString() << endl;
    }
}

// ***************************************************************************************************************************

ScriptEditorWidget::ScriptEditorWidget(QWidget *parent) : QWidget(parent)
{
    file = "";

    txtEditor = new ScriptEditor(this);
    txtOutput = new QPlainTextEdit(this);
    splitter = new QSplitter(this);

    createControls();

    QSettings settings;
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

void ScriptEditorWidget::doRunEcma(const QString &script)
{
    QString scriptContent;
    if (script.isEmpty())
        scriptContent = txtEditor->toPlainText();
    else
        scriptContent = script;

    QString output = runEcma(scriptContent);
    txtOutput->setPlainText(output);
}

void ScriptEditorWidget::doCreateEcmaFromModel()
{
    txtEditor->setPlainText(createEcmaFromModel());
}

// ***********************************************************************************************************

ScriptEditorDialog::ScriptEditorDialog(SceneView *sceneView, QWidget *parent) : QMainWindow(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());

    m_sceneView = sceneView;

    setWindowIcon(icon("script"));    

    searchDialog = new SearchDialog(this);

    createActions();
    createControls();

    restoreState(settings.value("ScriptEditorDialog/State", saveState()).toByteArray());

    setMinimumSize(600, 400);
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    QSettings settings;
    settings.setValue("ScriptEditorDialog/Geometry", saveGeometry());   
    settings.setValue("ScriptEditorDialog/State", saveState());
}

void ScriptEditorDialog::showDialog()
{
    show();
    txtEditor->setFocus();
}

void ScriptEditorDialog::runScript(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            // run script
            runEcma(file.readAll());
        }
        file.close();
    }
}

void ScriptEditorDialog::runCommand(const QString &command)
{
    if (!command.isEmpty())
    {
        // run script
        runEcma(command);
    }
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

    actFind = new QAction(icon("edit-find"), tr("&Find"), this);
    actFind->setShortcut(QKeySequence::Find);
    connect(actFind, SIGNAL(triggered()), this, SLOT(doFind()));

    actFindNext = new QAction(icon("edit-find"), tr("&Find next"), this);
    actFindNext->setShortcut(QKeySequence::FindNext);
    connect(actFindNext, SIGNAL(triggered()), this, SLOT(doFindNext()));

    actReplace = new QAction(icon("edit-find-replace"), tr("&Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actRunEcma = new QAction(icon("system-run"), tr("&Run"), this);
    actRunEcma->setShortcut(QKeySequence(tr("Ctrl+R")));
    
    // actRunPython = new QAction(icon("system-run"), tr("&Run Python script"), this);
    // actRunPython->setShortcut(QKeySequence(tr("Ctrl+T")));

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
    mnuEdit->addSeparator();
    mnuEdit->addAction(actFind);
    mnuEdit->addAction(actFindNext);
    mnuEdit->addAction(actReplace);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actRunEcma);
    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);

    tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->addAction(actFileNew);
    tlbFile->addAction(actFileOpen);
    tlbFile->addAction(actFileSave);

    tlbEdit = addToolBar(tr("Edit"));
    tlbEdit->setObjectName("Edit");
    tlbEdit->addAction(actUndo);
    tlbEdit->addAction(actRedo);
    tlbEdit->addSeparator();
    tlbEdit->addAction(actCut);
    tlbEdit->addAction(actCopy);
    tlbEdit->addAction(actPaste);

    tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->addAction(actRunEcma);
    // tlbTools->addAction(actRunPython);
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
    tabWidget->addTab(new ScriptEditorWidget(this), tr("Untitled"));
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
        for (int i = 0; i < tabWidget->count(); i++)
        {
            ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->widget(i));
            if (scriptEditorWidget->file == fileName)
            {
                tabWidget->setCurrentIndex(i);
                QMessageBox::information(this, tr("Information"), tr("Script is already opened."));
                return;
            }
        }

        // check empty document
        if (!scriptEditorWidget->txtEditor->toPlainText().isEmpty())
        {
            doFileNew();
            // new widget
            scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());
        }

        scriptEditorWidget->file = fileName;
        QFile fileName(scriptEditorWidget->file);
        if (fileName.open(QFile::ReadOnly | QFile::Text))
            txtEditor->setPlainText(fileName.readAll());

        QFileInfo fileInfo(scriptEditorWidget->file);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());

        doCurrentPageChanged(tabWidget->currentIndex());
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

void ScriptEditorDialog::doFind()
{
    if (searchDialog->showDialogFind() == QDialog::Accepted)
    {
        doFindNext(true);
    }
    searchDialog->hide();
}

void ScriptEditorDialog::doFindNext(bool fromBegining)
{
    if (!searchDialog->searchString().isEmpty())
    {
        QString search = searchDialog->searchString();

        QTextDocument::FindFlags flags;
        if (searchDialog->caseSensitive())
            flags |= QTextDocument::FindCaseSensitively;

        // Search
        QTextCursor cursor = txtEditor->textCursor();
        if (searchDialog->searchStringIsRegExp())
        {
            QRegExp searchReg(search);
            if (fromBegining)
                cursor = txtEditor->document()->find(searchReg, flags);
            else
                cursor = txtEditor->document()->find(searchReg, cursor, flags);
        }
        else
        {
            if (fromBegining)
                cursor = txtEditor->document()->find(search, flags);
            else
                cursor = txtEditor->document()->find(search, cursor, flags);
        }

        if (cursor.position() >= 0)
            txtEditor->setTextCursor(cursor);
        txtEditor->setFocus();
    }
}

void ScriptEditorDialog::doReplace()
{
    if (searchDialog->showDialogReplace() == QDialog::Accepted)
    {
        if (!searchDialog->searchString().isEmpty())
        {
            QString search = searchDialog->searchString();
            QString replace = searchDialog->replaceString();

            QTextCursor cursor = txtEditor->textCursor();

            QString text = txtEditor->document()->toPlainText();
            text.replace(search, replace, (searchDialog->caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive));
            txtEditor->document()->setPlainText(text);

            txtEditor->setTextCursor(cursor);
        }
    }
    searchDialog->hide();
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

    actRunEcma->disconnect();
    connect(actRunEcma, SIGNAL(triggered()), scriptEditorWidget, SLOT(doRunEcma()));
    actCreateFromModel->disconnect();
    connect(actCreateFromModel, SIGNAL(triggered()), scriptEditorWidget, SLOT(doCreateEcmaFromModel()));

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

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget->file);
        fileName = fileInfo.completeBaseName();
    }
    setWindowTitle(tr("Script editor - %1").arg(fileName));

    txtEditor->setFocus();
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
    if (m_engine->canEvaluate(txtEditor->toPlainText()))
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

// ***********************************************************************************************

SearchDialog::SearchDialog(QWidget *parent): QDialog(parent)
{
    // Title
    setWindowTitle(tr("Search and replace"));
    setWindowIcon(icon("edit-find"));
    setModal(true);

    // Find and replace
    QGroupBox *findReplaceGroup = new QGroupBox(this);
    findReplaceGroup->setTitle(tr("Find and replace"));

    txtFind = new QLineEdit();
    txtReplace = new QLineEdit();

    QGridLayout *findReplaceLayout = new QGridLayout();
    findReplaceGroup->setLayout(findReplaceLayout);
    findReplaceLayout->addWidget(new QLabel(tr("Search for:")), 0, 0);
    findReplaceLayout->addWidget(txtFind, 0, 1);
    findReplaceLayout->addWidget(new QLabel(tr("Replace with:")), 1, 0);
    findReplaceLayout->addWidget(txtReplace, 1, 1);

    // Options
    QGroupBox *optionsGroup = new QGroupBox(this);
    optionsGroup->setTitle(tr("Options"));

    chkSearchRegExp = new QCheckBox();
    chkSearchRegExp->setText(tr("Regular expression"));

    chkCaseSensitive = new QCheckBox(optionsGroup);
    chkCaseSensitive->setText(tr("Case sensitive"));

    QVBoxLayout *optionsLayout = new QVBoxLayout();
    optionsGroup->setLayout(optionsLayout);
    optionsLayout->addWidget(chkSearchRegExp);
    optionsLayout->addWidget(chkCaseSensitive);

    // Buttons
    btnConfirm = new QPushButton(this);
    btnConfirm->setDefault(true);
    connect(btnConfirm, SIGNAL(clicked()), this, SLOT(accept()));

    btnCancel = new QPushButton(this);
    btnCancel->setText(tr("Cancel"));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnConfirm);
    buttonsLayout->addWidget(btnCancel);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    layout->addWidget(findReplaceGroup);
    layout->addWidget(optionsGroup);
    layout->addLayout(buttonsLayout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

SearchDialog::~SearchDialog()
{
    delete txtFind;
    delete txtReplace;

    delete chkSearchRegExp;
    delete chkCaseSensitive;

    delete btnCancel;
    delete btnConfirm;
}

int SearchDialog::showDialogFind()
{
    txtReplace->setEnabled(false);
    btnConfirm->setText(tr("Find"));
    chkSearchRegExp->setEnabled(true);

    return exec();
}

int SearchDialog::showDialogReplace()
{
    txtReplace->setEnabled(true);
    btnConfirm->setText(tr("Replace"));
    chkSearchRegExp->setEnabled(false);

    return exec();
}
