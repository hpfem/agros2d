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

#include "scripteditordialog.h"

#include <QCompleter>

#include "scene.h"
#include "scenebasic.h"
#include "sceneview.h"
#include "scenemarkerdialog.h"
#include "scripteditorhighlighter.h"
#include "terminalview.h"
#include "scripteditorcommandpython.h"
#include "gui.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"

static PythonEngine *pythonEngine = NULL;

void createScriptEngine()
{
    logMessage("createScriptEngine()");

    if (pythonEngine == NULL)
    {
        pythonEngine = new PythonEngine();
    }
}

ScriptResult runPythonScript(const QString &script, const QString &fileName)
{
    logMessage("runPythonScript()");

    return pythonEngine->runPythonScript(script, fileName);
}

ExpressionResult runPythonExpression(const QString &expression, bool returnValue)
{
    logMessage("runPythonExpression()");

    return pythonEngine->runPythonExpression(expression, returnValue);
}

bool scriptIsRunning()
{
    logMessage("scriptIsRunning()");

    if (pythonEngine)
        return pythonEngine->isRunning();
    else
        return false;
}

void connectTerminal(Terminal *terminal)
{
    logMessage("connectTerminal()");

    QObject::connect(pythonEngine, SIGNAL(printStdout(QString)), terminal, SLOT(doPrintStdout(QString)));
}

void disconnectTerminal(Terminal *terminal)
{
    logMessage("disconnectTerminal()");

    QObject::disconnect(pythonEngine, SIGNAL(printStdout(QString)), terminal, SLOT(doPrintStdout(QString)));
}

PythonEngine *currentPythonEngine()
{
    logMessage("currentPythonEngine()");

    return pythonEngine;
}

QString createPythonFromModel()
{
    assert(0); //TODO
//    logMessage("createPythonFromModel()");

//    QString str;

//    // model
//    str += "# model\n";
//    str += QString("newdocument(name=\"%1\", type=\"%2\",\n"
//                   "            physicfield=\"%3\", analysistype=\"%4\",\n"
//                   "            numberofrefinements=%5, polynomialorder=%6,\n"
//                   "            nonlineartolerance=%7, nonlinearsteps=%8").
//            arg(Util::scene()->problemInfo()->name).
//            arg(problemTypeToStringKey(Util::scene()->problemInfo()->problemType)).
//            arg(QString::fromStdString(Util::scene()->problemInfo()->module()->id)).
//            arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType)).
//            arg(Util::scene()->problemInfo()->numberOfRefinements).
//            arg(Util::scene()->problemInfo()->polynomialOrder).
//            arg(Util::scene()->problemInfo()->nonlinearTolerance).
//            arg(Util::scene()->problemInfo()->nonlinearSteps);

//    if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
//        str += QString(",\n"
//                       "            adaptivitytype=\"%1\", adaptivitysteps=%2, adaptivitytolerance=%3").
//                arg(adaptivityTypeToStringKey(Util::scene()->problemInfo()->adaptivityType)).
//                arg(Util::scene()->problemInfo()->adaptivitySteps).
//                arg(Util::scene()->problemInfo()->adaptivityTolerance);

//    if (Util::scene()->problemInfo()->frequency > 0.0)
//        str += QString(",\n"
//                       "            frequency=%1").
//                arg(Util::scene()->problemInfo()->frequency);

//    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
//        str += QString(",\n"
//                       "            adaptivitytype=%1, adaptivitysteps=%2, adaptivitytolerance=%3").
//                arg(Util::scene()->problemInfo()->timeStep.text()).
//                arg(Util::scene()->problemInfo()->timeTotal.text()).
//                arg(Util::scene()->problemInfo()->initialCondition.text());

//    str += ")\n\n";

//    // startup script
//    if (!Util::scene()->problemInfo()->scriptStartup.isEmpty())
//    {
//        str += "# startup script\n";
//        str += Util::scene()->problemInfo()->scriptStartup;
//        str += "\n\n";
//    }

//    // boundaries
//    if (Util::scene()->boundaries.count() > 1)
//    {
//        str += "# boundaries\n";
//        for (int i = 1; i<Util::scene()->boundaries.count(); i++)
//        {
//            str += Util::scene()->boundaries[i]->script() + "\n";
//        }
//        str += "\n";
//    }

//    // materials
//    if (Util::scene()->materials.count() > 1)
//    {
//        str += "# materials\n";
//        for (int i = 1; i<Util::scene()->materials.count(); i++)
//        {
//            str += Util::scene()->materials[i]->script() + "\n";
//        }
//        str += "\n";
//    }

//    // edges
//    if (Util::scene()->edges.count() > 0)
//    {
//        str += "# edges\n";
//        for (int i = 0; i<Util::scene()->edges.count(); i++)
//        {
//            str += QString("addedge(%1, %2, %3, %4").
//                    arg(Util::scene()->edges[i]->nodeStart->point.x).
//                    arg(Util::scene()->edges[i]->nodeStart->point.y).
//                    arg(Util::scene()->edges[i]->nodeEnd->point.x).
//                    arg(Util::scene()->edges[i]->nodeEnd->point.y);

//            assert(0);
////             if (Util::scene()->edges[i]->boundary->name != "none")
////                str += QString(", boundary=\"%1\"").
////                        arg(QString::fromStdString(Util::scene()->edges[i]->boundary->name));

//            if (Util::scene()->edges[i]->angle > 0.0)
//                str += ", angle=" + QString::number(Util::scene()->edges[i]->angle);

//            if (Util::scene()->edges[i]->refineTowardsEdge > 0)
//                str += ", refine=" + QString::number(Util::scene()->edges[i]->refineTowardsEdge);

//            str += ")\n";
//        }
//        str += "\n";
//    }

//    // labels
//    if (Util::scene()->labels.count() > 0)
//    {
//        str += "# labels\n";
//        for (int i = 0; i<Util::scene()->labels.count(); i++)
//        {
//            str += QString("addlabel(%1, %2, material=\"%3\"").
//                    arg(Util::scene()->labels[i]->point.x).
//                    arg(Util::scene()->labels[i]->point.y).
//                    arg(QString::fromStdString(Util::scene()->labels[i]->material->name));

//            if (Util::scene()->labels[i]->area > 0.0)
//                str += ", area=" + QString::number(Util::scene()->labels[i]->area);
//            if (Util::scene()->labels[i]->polynomialOrder > 0)
//                str += ", order=" + QString::number(Util::scene()->labels[i]->polynomialOrder);

//            str += ")\n";
//        }

//    }
//    return str;
}

ScriptEngineRemote::ScriptEngineRemote()
{
    logMessage("ScriptEngineRemote::ScriptEngineRemote()");

    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer("agros2d-server");
    if (!m_server->listen("agros2d-server"))
    {
        qWarning() << tr("Error: Unable to start the server (agros2d-server): %1.").arg(m_server->errorString());
        return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(connected()));
}

ScriptEngineRemote::~ScriptEngineRemote()
{
    logMessage("ScriptEngineRemote::~ScriptEngineRemote()");

    delete m_server;
    delete m_client_socket;
}

void ScriptEngineRemote::connected()
{
    logMessage("ScriptEngineRemote::connected()");

    command = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void ScriptEngineRemote::readCommand()
{
    logMessage("ScriptEngineRemote::readCommand()");

    QTextStream in(m_server_socket);
    command = in.readAll();
}

void ScriptEngineRemote::disconnected()
{
    logMessage("ScriptEngineRemote::disconnected()");

    m_server_socket->deleteLater();

    ScriptResult result;
    if (!command.isEmpty())
    {
        result = runPythonScript(command);
    }

    m_client_socket = new QLocalSocket();
    connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

    m_client_socket->connectToServer("agros2d-client");
    if (m_client_socket->waitForConnected(1000))
    {
        QTextStream out(m_client_socket);
        out << result.text;
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
    logMessage("ScriptEngineRemote::displayError()");

    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        qWarning() << tr("Server error: The host was not found.");
        break;
    case QLocalSocket::ConnectionRefusedError:
        qWarning() << tr("Server error: The connection was refused by the peer. Make sure the agros2d-client server is running.");
        break;
    default:
        qWarning() << tr("Server error: The following error occurred: %1.").arg(m_client_socket->errorString());
    }
}

// ***************************************************************************************************************************

ScriptEditorWidget::ScriptEditorWidget(QWidget *parent) : QWidget(parent)
{
    logMessage("ScriptEditorWidget::ScriptEditorWidget()");

    file = "";

    txtEditor = new ScriptEditor(this);

    createControls();
}

ScriptEditorWidget::~ScriptEditorWidget()
{
    logMessage("ScriptEditorWidget::~ScriptEditorWidget()");

    delete txtEditor;
}

void ScriptEditorWidget::createControls()
{
    logMessage("ScriptEditorWidget::createControls()");

    // contents
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(txtEditor);

    setLayout(layout);
}

// ***********************************************************************************************************

ScriptEditorDialog::ScriptEditorDialog(QWidget *parent) : QMainWindow(parent)
{
    logMessage("ScriptEditorDialog::ScriptEditorDialog()");

    setWindowIcon(icon("script"));

    // search dialog
    searchDialog = new SearchDialog(this);

    createStatusBar();
    createActions();
    createViews();
    createControls();

    filBrowser->refresh();

    connect(actRunPython, SIGNAL(triggered()), this, SLOT(doRunPython()));

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());
    recentFiles = settings.value("ScriptEditorDialog/RecentFiles").value<QStringList>();
    restoreState(settings.value("ScriptEditorDialog/State", saveState()).toByteArray());
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    logMessage("ScriptEditorDialog::~ScriptEditorDialog()");

    QSettings settings;
    settings.setValue("ScriptEditorDialog/Geometry", saveGeometry());
    settings.setValue("ScriptEditorDialog/State", saveState());
    settings.setValue("ScriptEditorDialog/RecentFiles", recentFiles);

    delete pythonEngine;
}

void ScriptEditorDialog::showDialog()
{
    logMessage("ScriptEditorDialog::showDialog()");

    show();
    activateWindow();
    raise();
    txtEditor->setFocus();
}

void ScriptEditorDialog::createActions()
{
    logMessage("ScriptEditorDialog::createActions()");

    actFileNew = new QAction(icon("document-new"), tr("&New"), this);
    actFileNew->setShortcuts(QKeySequence::AddTab);
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

    actFileOpenRecentGroup = new QActionGroup(this);
    connect(actFileOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doFileOpenRecent(QAction *)));

    actFileClose = new QAction(icon(""), tr("&Close"), this);
    actFileClose->setShortcuts(QKeySequence::Close);
    connect(actFileClose, SIGNAL(triggered()), this, SLOT(doFileClose()));

    actFilePrint = new QAction(icon(""), tr("&Print"), this);
    actFilePrint->setShortcuts(QKeySequence::Print);
    connect(actFilePrint, SIGNAL(triggered()), this, SLOT(doFilePrint()));

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

    actFindNext = new QAction(icon("edit-find"), tr("Find &next"), this);
    actFindNext->setShortcut(QKeySequence::FindNext);
    connect(actFindNext, SIGNAL(triggered()), this, SLOT(doFindNext()));

    actReplace = new QAction(icon("edit-find-replace"), tr("Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actReplace = new QAction(icon("edit-find-replace"), tr("Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actIndentSelection = new QAction(icon(""), tr("Indent"), this);
    actUnindentSelection = new QAction(icon(""), tr("Unindent"), this);

    actCommentSelection = new QAction(icon(""), tr("Comment"), this);
    // actCommentSelection->setShortcut(tr("Ctrl+"));
    actUncommentSelection = new QAction(icon(""), tr("Uncomment"), this);
    actUncommentSelection->setShortcut(tr("Ctrl+U"));

    actGotoLine = new QAction(icon(""), tr("Goto line"), this);
    actGotoLine->setShortcut(tr("Alt+G"));

    actRunPython = new QAction(icon("run"), tr("&Run Python script"), this);
    actRunPython->setShortcut(QKeySequence(tr("Ctrl+R")));

    actCreateFromModel = new QAction(icon("script-create"), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit script editor"));
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpKeywordList = new QAction(icon("help-contents"), tr("&Keyword List"), this);
    actHelpKeywordList->setShortcut(QKeySequence::HelpContents);
    connect(actHelpKeywordList, SIGNAL(triggered()), this, SLOT(doHelpKeywordList()));
}

void ScriptEditorDialog::createControls()
{
    logMessage("ScriptEditorDialog::createControls()");

    mnuRecentFiles = new QMenu(tr("&Recent files"), this);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actFileNew);
    mnuFile->addAction(actFileOpen);
    mnuFile->addAction(actFileSave);
    mnuFile->addAction(actFileSaveAs);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addAction(actFileClose);
    mnuFile->addSeparator();
    mnuFile->addAction(actFilePrint);
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
    mnuEdit->addSeparator();
    mnuEdit->addAction(actIndentSelection);
    mnuEdit->addAction(actUnindentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCommentSelection);
    mnuEdit->addAction(actUncommentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actGotoLine);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actRunPython);
    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);
    mnuHelp->addAction(actHelpKeywordList);

#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    QToolBar *tlbFile = addToolBar(tr("File"));
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbFile->setObjectName("File");
    tlbFile->addAction(actFileNew);
    tlbFile->addAction(actFileOpen);
    tlbFile->addAction(actFileSave);

    QToolBar *tlbEdit = addToolBar(tr("Edit"));
#ifdef Q_WS_MAC
    tlbEdit->setFixedHeight(iconHeight);
    tlbEdit->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbEdit->setObjectName("Edit");
    tlbEdit->addAction(actUndo);
    tlbEdit->addAction(actRedo);
    tlbEdit->addSeparator();
    tlbEdit->addAction(actCut);
    tlbEdit->addAction(actCopy);
    tlbEdit->addAction(actPaste);

    QToolBar *tlbTools = addToolBar(tr("Tools"));
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->setObjectName("Tools");
    tlbTools->addAction(actRunPython);
    tlbTools->addSeparator();
    tlbTools->addAction(actCreateFromModel);

    // path
    QLineEdit *txtPath = new QLineEdit(this);
    txtPath->setReadOnly(true);

    QPushButton *btnPath = new QPushButton(icon("three-dots"), "");
    btnPath->setMaximumSize(btnPath->sizeHint());

    connect(btnPath, SIGNAL(clicked()), this, SLOT(doPathChangeDir()));
    connect(filBrowser, SIGNAL(directoryChanged(QString)), txtPath, SLOT(setText(QString)));

    QToolBar *tlbPath = addToolBar(tr("Path"));
#ifdef Q_WS_MAC
    tlbPath->setFixedHeight(iconHeight);
    tlbPath->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbPath->setObjectName("Path");
    tlbPath->addWidget(new QLabel(tr("Path: "), this));
    tlbPath->addWidget(txtPath);
    tlbPath->addWidget(btnPath);

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

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    // recent files
    setRecentFiles();

    setCentralWidget(widget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(doDataChanged()));
}

void ScriptEditorDialog::createViews()
{
    logMessage("ScriptEditorDialog::createViews()");

    QSettings settings;

    // file browser
    filBrowser = new FileBrowser(this);
    filBrowser->setNameFilter("*.py");
    filBrowser->setDir(settings.value("ScriptEditorDialog/WorkDir", datadir()).value<QString>());

    connect(filBrowser, SIGNAL(fileItemDoubleClick(QString)), this, SLOT(doFileItemDoubleClick(QString)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(filBrowser);
    layout->setContentsMargins(0, 0, 0, 7);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    fileBrowserView = new QDockWidget(tr("File browser"), this);
    fileBrowserView->setObjectName("ScriptEditorFileBrowserView");
    fileBrowserView->setWidget(widget);
    fileBrowserView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, fileBrowserView);

    terminalView = new TerminalView(this);
    terminalView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, terminalView);
}

void ScriptEditorDialog::createStatusBar()
{
    logMessage("ScriptEditorDialog::createStatusBar()");

    lblCurrentPosition = new QLabel(statusBar());

    statusBar()->showMessage(tr("Ready"));
    statusBar()->addPermanentWidget(lblCurrentPosition);
}

void ScriptEditorDialog::doRunPython()
{
    logMessage("ScriptEditorDialog::doRunPython()");

    if (!scriptEditorWidget()->file.isEmpty())
        filBrowser->setDir(QFileInfo(scriptEditorWidget()->file).absolutePath());

    // disable controls
    terminalView->terminal()->setEnabled(false);
    // actRunPython->setEnabled(false);
    scriptEditorWidget()->setCursor(Qt::BusyCursor);
    QApplication::processEvents();

    // run script
    terminalView->terminal()->doPrintStdout("Run script: " + tabWidget->tabText(tabWidget->currentIndex()).replace("* ", "") + "\n", Qt::gray);
    connect(pythonEngine, SIGNAL(printStdout(QString)), terminalView->terminal(), SLOT(doPrintStdout(QString)));

    // benchmark
    QTime time;
    time.start();

    ScriptResult result;
    if (txtEditor->textCursor().hasSelection())
    {
        result = runPythonScript(txtEditor->textCursor().selectedText().replace(0x2029, "\n"), "");
    }
    else if (scriptEditorWidget()->file.isEmpty())
    {
        result = runPythonScript(txtEditor->toPlainText());
    }
    else
    {
        if (!scriptEditorWidget()->file.isEmpty() &&
                QFile::exists(scriptEditorWidget()->file))
            doFileSave();

        result = runPythonScript(txtEditor->toPlainText(),
                                 QFileInfo(scriptEditorWidget()->file).absoluteFilePath());
    }

    if (result.isError)
    {
        terminalView->terminal()->doPrintStdout(result.text + "\n", Qt::red);
        if (!txtEditor->textCursor().hasSelection() && result.line >= 0)
            txtEditor->gotoLine(result.line, true);
    }

    // disconnect
    disconnect(pythonEngine, SIGNAL(printStdout(QString)), terminalView->terminal(), SLOT(doPrintStdout(QString)));

    // enable controls
    terminalView->terminal()->setEnabled(true);
    scriptEditorWidget()->setCursor(Qt::ArrowCursor);
    // actRunPython->setEnabled(true);

    txtEditor->setFocus();
    activateWindow();
}

void ScriptEditorDialog::doCreatePythonFromModel()
{
    logMessage("ScriptEditorDialog::doCreatePythonFromModel()");

    txtEditor->setPlainText(createPythonFromModel());
}

void ScriptEditorDialog::doFileItemDoubleClick(const QString &path)
{
    logMessage("ScriptEditorDialog::doFileItemDoubleClick()");

    QFileInfo fileInfo(path);

    QSettings settings;
    if (QDir(path).exists())
        settings.setValue("ScriptEditorDialog/WorkDir", path);
    else
    {
        settings.setValue("ScriptEditorDialog/WorkDir", fileInfo.absolutePath());

        if (fileInfo.suffix() == "py")
            doFileOpen(fileInfo.absoluteFilePath());
    }
}

void ScriptEditorDialog::doPathChangeDir()
{
    logMessage("ScriptEditorDialog::doPathChangeDir()");

    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select directory"), filBrowser->basePath(), options);
    if (!directory.isEmpty())
        filBrowser->setDir(directory);
}

void ScriptEditorDialog::doFileNew()
{
    logMessage("ScriptEditorDialog::doFileNew()");

    tabWidget->addTab(new ScriptEditorWidget(this), tr("Untitled"));
    tabWidget->setCurrentIndex(tabWidget->count()-1);
    doCurrentPageChanged(tabWidget->count()-1);
}

void ScriptEditorDialog::doFileOpen(const QString &file)
{
    logMessage("ScriptEditorDialog::doFileOpen()");

    QSettings settings;
    QString dir = settings.value("General/LastScriptDir", "data/scripts").toString();

    // open dialog
    QString fileName = file;
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), dir, tr("Python files (*.py)"));

    // read text
    if (!fileName.isEmpty())
    {
        ScriptEditorWidget *scriptEditor = scriptEditorWidget();

        for (int i = 0; i < tabWidget->count(); i++)
        {
            ScriptEditorWidget *scriptEditorWidgetTmp = dynamic_cast<ScriptEditorWidget *>(tabWidget->widget(i));
            if (scriptEditorWidgetTmp->file == fileName)
            {
                tabWidget->setCurrentIndex(i);
                QMessageBox::information(this, tr("Information"), tr("Script is already opened."));
                return;
            }
        }

        // check empty document
        if (!scriptEditor->txtEditor->toPlainText().isEmpty())
        {
            doFileNew();
            // new widget
            scriptEditor = scriptEditorWidget();
        }

        scriptEditor->file = fileName;
        txtEditor->setPlainText(readFileContent(scriptEditor->file));

        setRecentFiles();

        QFileInfo fileInfo(scriptEditor->file);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());

        doCurrentPageChanged(tabWidget->currentIndex());

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastScriptDir", fileInfo.absolutePath());
    }
}

void ScriptEditorDialog::doFileOpenRecent(QAction *action)
{
    logMessage("ScriptEditorDialog::doFileOpenRecent()");

    QString fileName = action->text();
    if (QFile::exists(fileName))
    {
        doFileOpen(fileName);
        setRecentFiles();
    }
}

void ScriptEditorDialog::doFileSave()
{
    logMessage("ScriptEditorDialog::doFileSave()");

    QSettings settings;
    QString dir = settings.value("General/LastScriptDir", "data/scripts").toString();

    // save dialog
    if (scriptEditorWidget()->file.isEmpty())
        scriptEditorWidget()->file = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python files (*.py)"));

    // write text
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        if (fileInfo.suffix() != "py") scriptEditorWidget()->file += ".py";

        QFile fileName(scriptEditorWidget()->file);
        if (fileName.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&fileName);
            out << txtEditor->toPlainText();
            fileName.close();

            setRecentFiles();

            tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
            txtEditor->document()->setModified(false);
        }
        else
        {
            ErrorResult errorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved.").arg(scriptEditorWidget()->file));
            errorResult.showDialog();
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastScriptDir", fileInfo.absolutePath());
    }
}

void ScriptEditorDialog::doFileSaveAs()
{
    logMessage("ScriptEditorDialog::doFileSaveAs()");

    QSettings settings;
    QString dir = settings.value("General/LastScriptDir", "data/scripts").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python files (*.py)"));
    if (!fileName.isEmpty())
    {
        scriptEditorWidget()->file = fileName;
        doFileSave();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastScriptDir", fileInfo.absolutePath());
    }
}

void ScriptEditorDialog::doFileClose()
{
    logMessage("ScriptEditorDialog::doFileClose()");

    doCloseTab(tabWidget->currentIndex());
}

void ScriptEditorDialog::doFilePrint()
{
    logMessage("ScriptEditorDialog::doFilePrint()");

    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);
    printDialog.addEnabledOption(QAbstractPrintDialog::PrintCollateCopies);
    printDialog.setWindowTitle(tr("Print Document"));
    if (printDialog.exec() == QDialog::Accepted)
    {
        txtEditor->print(&printer);
    }
}

void ScriptEditorDialog::doFind()
{
    logMessage("ScriptEditorDialog::doFind()");

    if (searchDialog->showDialogFind() == QDialog::Accepted)
    {
        doFindNext(true);
    }
    searchDialog->hide();
}

void ScriptEditorDialog::doFindNext(bool fromBegining)
{
    logMessage("ScriptEditorDialog::doFindNext()");

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
    logMessage("ScriptEditorDialog::doReplace()");

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
    logMessage("ScriptEditorDialog::doDataChanged()");

    actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void ScriptEditorDialog::doHelp()
{
    logMessage("ScriptEditorDialog::doHelp()");

    showPage("scripting/commands.html");
}

void ScriptEditorDialog::doHelpKeywordList()
{
    logMessage("ScriptEditorDialog::doHelpKeywordList()");

    showPage("scripting/keyword_list.html");
}

void ScriptEditorDialog::doCloseTab(int index)
{
    logMessage("ScriptEditorDialog::doCloseTab()");

    tabWidget->setCurrentIndex(index);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        fileName = fileInfo.completeBaseName();
    }

    while (txtEditor->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"), tr("File '%1' has been modified.\nDo you want to save your changes?").arg(fileName),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            doFileSave();
        else if (ret == QMessageBox::Discard)
            break;
        else if (ret == QMessageBox::Cancel)
            return;
    }

    if (tabWidget->count() == 1)
    {
        doFileNew();
    }

    tabWidget->removeTab(index);
}

void ScriptEditorDialog::doCurrentPageChanged(int index)
{
    logMessage("ScriptEditorDialog::doCurrentPageChanged()");

    txtEditor = scriptEditorWidget()->txtEditor;

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

    actIndentSelection->disconnect();
    connect(actIndentSelection, SIGNAL(triggered()), txtEditor, SLOT(indentSelection()));
    actUnindentSelection->disconnect();
    connect(actUnindentSelection, SIGNAL(triggered()), txtEditor, SLOT(unindentSelection()));
    actCommentSelection->disconnect();
    connect(actCommentSelection, SIGNAL(triggered()), txtEditor, SLOT(commentSelection()));
    actUncommentSelection->disconnect();
    connect(actUncommentSelection, SIGNAL(triggered()), txtEditor, SLOT(uncommentSelection()));
    actGotoLine->disconnect();
    connect(actGotoLine, SIGNAL(triggered()), txtEditor, SLOT(gotoLine()));

    txtEditor->document()->disconnect(actUndo);
    txtEditor->document()->disconnect(actRedo);
    connect(txtEditor->document(), SIGNAL(undoAvailable(bool)), actUndo, SLOT(setEnabled(bool)));
    connect(txtEditor->document(), SIGNAL(redoAvailable(bool)), actRedo, SLOT(setEnabled(bool)));
    txtEditor->disconnect(actCut);
    txtEditor->disconnect(actCopy);
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCut, SLOT(setEnabled(bool)));
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCopy, SLOT(setEnabled(bool)));

    // modifications
    connect(txtEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(doCurrentDocumentChanged(bool)));

    // line number
    connect(txtEditor, SIGNAL(cursorPositionChanged()), this, SLOT(doCursorPositionChanged()));
    doCursorPositionChanged();

    actUndo->setEnabled(txtEditor->document()->isUndoAvailable());
    actRedo->setEnabled(txtEditor->document()->isRedoAvailable());

    // tabWidget->setTabsClosable(tabWidget->count() > 1);
    tabWidget->setTabsClosable(true);
    tabWidget->cornerWidget(Qt::TopLeftCorner)->setEnabled(true);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        fileName = fileInfo.completeBaseName();
    }
    setWindowTitle(tr("Script editor - %1").arg(fileName));

    txtEditor->setFocus();
}

void ScriptEditorDialog::doCursorPositionChanged()
{
    logMessage("ScriptEditorDialog::doCursorPositionChanged()");

    QTextCursor cur(txtEditor->textCursor());
    lblCurrentPosition->setText(tr("Line: %1, Col: %2").arg(cur.blockNumber()+1)
                                .arg(cur.columnNumber()+1));
}

void ScriptEditorDialog::doCurrentDocumentChanged(bool changed)
{
    logMessage("ScriptEditorDialog::doCurrentDocumentChanged()");

    // modified
    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        fileName = fileInfo.completeBaseName();
    }

    if (changed)
        tabWidget->setTabText(tabWidget->currentIndex(), QString("* %1").arg(fileName));
    else
        tabWidget->setTabText(tabWidget->currentIndex(), fileName);
}

void ScriptEditorDialog::setRecentFiles()
{
    logMessage("ScriptEditorDialog::setRecentFiles()");

    if (!tabWidget) return;

    // recent files
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        if (recentFiles.indexOf(fileInfo.absoluteFilePath()) == -1)
            recentFiles.insert(0, fileInfo.absoluteFilePath());
        else
            recentFiles.move(recentFiles.indexOf(fileInfo.absoluteFilePath()), 0);

        while (recentFiles.count() > 15) recentFiles.removeLast();
    }

    mnuRecentFiles->clear();
    for (int i = 0; i<recentFiles.count(); i++)
    {
        QAction *actMenuRecentItem = new QAction(recentFiles[i], this);
        actFileOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}

void ScriptEditorDialog::closeTabs()
{
    for (int i = tabWidget->count()-1; i >= 0 ; i--)
        doCloseTab(i);
}

bool ScriptEditorDialog::isScriptModified()
{
    return txtEditor->document()->isModified();
}

// ******************************************************************************************************

ScriptEditor::ScriptEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    logMessage("ScriptEditor::ScriptEditor()");

    lineNumberArea = new ScriptEditorLineNumberArea(this);

#ifndef Q_WS_MAC
    setFont(QFont("Monospace", 10));
#endif
    setTabStopWidth(fontMetrics().width("    "));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    // highlighter
    new QScriptSyntaxHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

ScriptEditor::~ScriptEditor()
{
    logMessage("ScriptEditor::~ScriptEditor()");

    delete lineNumberArea;
}

int ScriptEditor::lineNumberAreaWidth()
{
    logMessage("ScriptEditor::lineNumberAreaWidth()");

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
    logMessage("ScriptEditor::resizeEvent()");

    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::keyPressEvent(QKeyEvent *event)
{
    logMessage("ScriptEditor::keyPressEvent()");

    if (event->key() == Qt::Key_Tab)
    {
        if (textCursor().hasSelection())
        {
            indentSelection();
            return;
        }
    }

    if (event->key() == Qt::Key_Backtab)
    {
        if (textCursor().hasSelection())
        {
            unindentSelection();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void ScriptEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    logMessage("ScriptEditor::updateLineNumberAreaWidth()");

    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    logMessage("ScriptEditor::updateLineNumberArea()");

    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void ScriptEditor::indentSelection()
{
    logMessage("ScriptEditor::indentSelection()");

    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.insertText("\t");
            end++;
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::unindentSelection()
{
    logMessage("ScriptEditor::unindentSelection()");

    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == "\t")
            {
                cursor.removeSelectedText();
                end--;
            }
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::commentSelection()
{
    logMessage("ScriptEditor::commentSelection()");

    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.insertText("#");
            end++;
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::uncommentSelection()
{
    logMessage("ScriptEditor::uncommentSelection()");

    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == "#")
            {
                cursor.removeSelectedText();
                end--;
            }
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::gotoLine(int line, bool isError)
{
    logMessage("ScriptEditor::gotoLine()");

    // use dialog when (line == -1)
    if (line == -1)
    {
        bool ok;
        int lineDialog = QInputDialog::getInt(this, tr("Goto line"), tr("Line number:"),
                                              0, 1, document()->blockCount(), 1, &ok);
        if (ok)
            line = lineDialog;
    }

    if (line >= 0 && line <= document()->blockCount())
    {
        int pos = document()->findBlockByNumber(line - 1).position();
        QTextCursor cur = textCursor();
        cur.setPosition(pos, QTextCursor::MoveAnchor);
        setTextCursor(cur);
        ensureCursorVisible();
        highlightCurrentLine(true);
        setFocus();
    }
}

void ScriptEditor::highlightCurrentLine(bool isError)
{
    logMessage("ScriptEditor::highlightCurrentLine()");

    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(180);
        if (isError)
            lineColor = QColor(Qt::red).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);

    matchParentheses('(', ')');
    // matchParentheses('[', ']');
    // matchParentheses('{', '}');
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    logMessage("ScriptEditor::lineNumberAreaPaintEvent()");

    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
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

void ScriptEditor::matchParentheses(char left, char right)
{
    logMessage("ScriptEditor::matchParentheses()");

    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data)
    {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i)
        {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == left)
            {
                if (matchLeftParenthesis(left, right, textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
            else if (info->position == curPos - 1 && info->character == right)
            {
                if (matchRightParenthesis(left, right, textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool ScriptEditor::matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int i, int numLeftParentheses)
{
    logMessage("ScriptEditor::matchLeftParenthesis()");

    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i)
    {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == left)
        {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == right && numLeftParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numLeftParentheses;
        }
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(left, right, currentBlock, 0, numLeftParentheses);

    return false;
}

bool ScriptEditor::matchRightParenthesis(char left, char right, QTextBlock currentBlock, int i, int numRightParentheses)
{
    logMessage("ScriptEditor::matchRightParenthesis()");

    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i)
    {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == right)
        {
            ++numRightParentheses;
            continue;
        }
        if (info->character == left && numRightParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numRightParentheses;
        }
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(left, right, currentBlock, 0, numRightParentheses);

    return false;
}

void ScriptEditor::createParenthesisSelection(int pos)
{
    logMessage("ScriptEditor::createParenthesisSelection()");

    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setForeground(Qt::red);
    // format.setBackground(Qt::lightGray);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}

// ***********************************************************************************************

SearchDialog::SearchDialog(QWidget *parent): QDialog(parent)
{
    logMessage("SearchDialog::SearchDialog()");

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
    logMessage("SearchDialog::~SearchDialog()");

    delete txtFind;
    delete txtReplace;

    delete chkSearchRegExp;
    delete chkCaseSensitive;

    delete btnCancel;
    delete btnConfirm;
}

int SearchDialog::showDialogFind()
{
    logMessage("SearchDialog::showDialogFind()");

    txtFind->setFocus();
    txtReplace->setEnabled(false);
    btnConfirm->setText(tr("Find"));
    chkSearchRegExp->setEnabled(true);

    return exec();
}

int SearchDialog::showDialogReplace()
{
    logMessage("SearchDialog::showDialogReplace()");

    txtFind->setFocus();
    txtReplace->setEnabled(true);
    btnConfirm->setText(tr("Replace"));
    chkSearchRegExp->setEnabled(false);

    return exec();
}
