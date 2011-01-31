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

#include "mainwindow.h"

#include "gui.h"

#include "scene.h"
#include "scenebasic.h"
#include "sceneview.h"
#include "sceneinfoview.h"
#include "terminalview.h"
#include "tooltipview.h"
#include "chartdialog.h"
#include "configdialog.h"
#include "scripteditordialog.h"
#include "reportdialog.h"
#include "videodialog.h"
#include "logdialog.h"
#include "helpdialog.h"
#include "problemdialog.h"
#include "datatabledialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    logMessage("MainWindow::MainWindow()");

    createScriptEngine();
    createScene();

    helpDialog = new HelpDialog(this);
    chartDialog = new ChartDialog(this);
    scriptEditorDialog = new ScriptEditorDialog(this);
    reportDialog = new ReportDialog(sceneView, this);
    videoDialog = new VideoDialog(sceneView, this);
    logDialog = new LogDialog(this);

    createActions();
    createViews();
    createMenus();
    createToolBars();
    createStatusBar();

    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(Util::scene(), SIGNAL(fileNameChanged(QString)), this, SLOT(doSetWindowTitle(QString)));

    connect(sceneView, SIGNAL(mousePressed()), localPointValueView, SLOT(doShowPoint()));
    connect(sceneView, SIGNAL(mousePressed(const Point &)), localPointValueView, SLOT(doShowPoint(const Point &)));
    connect(sceneView, SIGNAL(mousePressed()), volumeIntegralValueView, SLOT(doShowVolumeIntegral()));
    connect(sceneView, SIGNAL(mousePressed()), surfaceIntegralValueView, SLOT(doShowSurfaceIntegral()));
    connect(sceneView, SIGNAL(mousePressed()), surfaceIntegralValueView, SLOT(doShowSurfaceIntegral()));
    connect(sceneView, SIGNAL(sceneModeChanged(SceneMode)), tooltipView, SLOT(loadTooltip(SceneMode)));

    sceneView->doDefaultValues();

    connect(chartDialog, SIGNAL(setChartLine(Point,Point)), sceneView, SLOT(doSetChartLine(Point,Point)));

    QSettings settings;
    restoreGeometry(settings.value("MainWindow/Geometry", saveGeometry()).toByteArray());
    recentFiles = settings.value("MainWindow/RecentFiles").value<QStringList>();
    restoreState(settings.value("MainWindow/State", saveState()).toByteArray());

    Util::scene()->clear();

    sceneView->actSceneModeNode->trigger();
    sceneView->doZoomBestFit();

    // set recent files
    setRecentFiles();

    // run server
    new ScriptEngineRemote();

    // accept drops
    setAcceptDrops(true);

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    doInvalidated();

    if (settings.value("General/CheckVersion", true).value<bool>())
        checkForNewVersion(true);

    // parameters
    QStringList args = QCoreApplication::arguments();
    for (int i = 1; i < args.count(); i++)
    {
        if (args[i] == "--verbose" || args[i] == "/verbose")
            continue;

        if (args[i] == "--run" || args[i] == "-r" || args[i] == "/r")
        {
            QString scriptName = args[++i];

            if (QFile::exists(scriptName))
                runPythonScript(readFileContent(scriptName));
            else
                qWarning() << "Script " << scriptName << "not found.";

            continue;
        }

        QString fileName = args[i];
        open(fileName);
    }
}

MainWindow::~MainWindow()
{
    logMessage("MainWindow::~MainWindow()");

    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/RecentFiles", recentFiles);

    // remove temp files
    removeDirectory(tempProblemDir());
}

void MainWindow::open(const QString &fileName)
{
    logMessage("MainWindow::open()");

    doDocumentOpen(fileName);
}

void MainWindow::createActions()
{
    logMessage("MainWindow::createActions()");

    actDocumentNew = new QAction(icon("document-new"), tr("&New..."), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    actDocumentNew->setStatusTip(tr("Create a new file"));
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(doDocumentNew()));

    actDocumentOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    actDocumentOpen->setStatusTip(tr("Open an existing file"));
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(doDocumentOpen()));

    actDocumentSave = new QAction(icon("document-save"), tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    actDocumentSave->setStatusTip(tr("Save the file to disk"));
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));

#ifdef BETA
    actDocumentSaveWithSolution = new QAction(icon(""), tr("Save with solution"), this);
    actDocumentSaveWithSolution->setStatusTip(tr("Save the file to disk with solution"));
    connect(actDocumentSaveWithSolution, SIGNAL(triggered()), this, SLOT(doDocumentSaveWithSolution()));
#else
    QSettings settings;
    settings.setValue("Solver/SaveProblemWithSolution", false);
#endif

    actDocumentSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    actDocumentSaveAs->setStatusTip(tr("Save the file under a new name"));
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));

    actDocumentClose = new QAction(tr("&Close"), this);
    actDocumentClose->setShortcuts(QKeySequence::Close);
    actDocumentClose->setStatusTip(tr("Close the file"));
    connect(actDocumentClose, SIGNAL(triggered()), this, SLOT(doDocumentClose()));

    actDocumentImportDXF = new QAction(tr("Import DXF..."), this);
    actDocumentImportDXF->setStatusTip(tr("Import AutoCAD DXF"));
    connect(actDocumentImportDXF, SIGNAL(triggered()), this, SLOT(doDocumentImportDXF()));

    actDocumentExportDXF = new QAction(tr("Export DXF..."), this);
    actDocumentExportDXF->setStatusTip(tr("Export AutoCAD DXF"));
    connect(actDocumentExportDXF, SIGNAL(triggered()), this, SLOT(doDocumentExportDXF()));

    actDocumentExportMeshFile = new QAction(tr("Export mesh file..."), this);
    actDocumentExportMeshFile->setStatusTip(tr("Export Hermes2D mesh file"));
    connect(actDocumentExportMeshFile, SIGNAL(triggered()), this, SLOT(doDocumentExportMeshFile()));

    actDocumentSaveImage = new QAction(tr("Export image..."), this);
    actDocumentSaveImage->setStatusTip(tr("Export image to file"));
    connect(actDocumentSaveImage, SIGNAL(triggered()), this, SLOT(doDocumentSaveImage()));

    actCreateVideo = new QAction(icon("video"), tr("Create &video..."), this);
    actCreateVideo->setStatusTip(tr("Create video"));
    connect(actCreateVideo, SIGNAL(triggered()), this, SLOT(doCreateVideo()));

    actLoadBackground = new QAction(tr("Load background..."), this);
    actLoadBackground->setStatusTip(tr("Load background image"));
    connect(actLoadBackground, SIGNAL(triggered()), this, SLOT(doLoadBackground()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit the application"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    // undo framework
    actUndo = Util::scene()->undoStack()->createUndoAction(this);
    actUndo->setIcon(icon("edit-undo"));
    actUndo->setIconText(tr("&Undo"));
    actUndo->setShortcuts(QKeySequence::Undo);
    actUndo->setStatusTip(tr("Undo operation"));

    actRedo = Util::scene()->undoStack()->createRedoAction(this);
    actRedo->setIcon(icon("edit-redo"));
    actRedo->setIconText(tr("&Redo"));
    actRedo->setShortcuts(QKeySequence::Redo);
    actRedo->setStatusTip(tr("Redo operation"));

    actCopy = new QAction(icon("edit-copy"), tr("Copy"), this);
    actCopy->setShortcuts(QKeySequence::Copy);
    actCopy->setStatusTip(tr("Copy image to clipboard."));
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopy()));

    actPaste = new QAction(icon("edit-copy"), tr("Copy"), this);
    actPaste->setShortcuts(QKeySequence::Paste);
    connect(actPaste, SIGNAL(triggered()), this, SLOT(doPaste()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setStatusTip(tr("Show help"));
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpShortCut = new QAction(icon(""), tr("&Shortcuts"), this);
    actHelpShortCut->setStatusTip(tr("Shortcuts"));
    connect(actHelpShortCut, SIGNAL(triggered()), this, SLOT(doHelpShortCut()));

    actOnlineHelp = new QAction(icon(""), tr("&Online help"), this);
    actOnlineHelp->setStatusTip(tr("Online help"));
    connect(actOnlineHelp, SIGNAL(triggered()), this, SLOT(doOnlineHelp()));

    actCheckVersion = new QAction(icon(""), tr("Check version"), this);
    actCheckVersion->setStatusTip(tr("Check version"));
    connect(actCheckVersion, SIGNAL(triggered()), this, SLOT(doCheckVersion()));

    actAbout = new QAction(icon("about"), tr("About &Agros2D"), this);
    actAbout->setStatusTip(tr("Show the application's About box"));
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setStatusTip(tr("Show the Qt library's About box"));
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actOptions = new QAction(icon("options"), tr("&Options"), this);
    actOptions->setStatusTip(tr("Options"));
    actOptions->setMenuRole(QAction::PreferencesRole);
    connect(actOptions, SIGNAL(triggered()), this, SLOT(doOptions()));

    actCreateMesh = new QAction(icon("scene-mesh"), tr("&Mesh area"), this);
    actCreateMesh->setStatusTip(tr("Mesh area"));
    connect(actCreateMesh, SIGNAL(triggered()), this, SLOT(doCreateMesh()));

    actSolve = new QAction(icon("run"), tr("&Solve problem"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    actSolve->setStatusTip(tr("Solve problem"));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actChart = new QAction(icon("chart"), tr("&Chart"), this);
    actChart->setStatusTip(tr("Chart"));
    connect(actChart, SIGNAL(triggered()), this, SLOT(doChart()));

    actFullScreen = new QAction(icon("view-fullscreen"), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actScriptEditor = new QAction(icon("script-python"), tr("Script &editor"), this);
    actScriptEditor->setStatusTip(tr("Script editor"));
    actScriptEditor->setShortcut(Qt::Key_F4);
    connect(actScriptEditor, SIGNAL(triggered()), this, SLOT(doScriptEditor()));

    actScriptEditorRunScript = new QAction(icon("script"), tr("Run &script..."), this);
    actScriptEditorRunScript->setStatusTip(tr("Run script..."));
    connect(actScriptEditorRunScript, SIGNAL(triggered()), this, SLOT(doScriptEditorRunScript()));

    actScriptEditorRunCommand = new QAction(icon("run"), tr("Run &command..."), this);
    actScriptEditorRunCommand->setShortcut(QKeySequence(tr("Alt+C")));
    actScriptEditorRunCommand->setStatusTip(tr("Run command..."));
    connect(actScriptEditorRunCommand, SIGNAL(triggered()), this, SLOT(doScriptEditorRunCommand()));

    actReport = new QAction(icon("report"), tr("&Report..."), this);
    actReport->setStatusTip(tr("Problem html report"));
    connect(actReport, SIGNAL(triggered()), this, SLOT(doReport()));

    actProgressLog = new QAction(icon("log"), tr("Progress &log"), this);
    actProgressLog->setStatusTip(tr("Show progress log"));
    connect(actProgressLog, SIGNAL(triggered()), this, SLOT(doProgressLog()));

    actApplicationLog = new QAction(icon("log"), tr("Application &log"), this);
    actApplicationLog->setStatusTip(tr("Show application log"));
    connect(actApplicationLog, SIGNAL(triggered()), this, SLOT(doApplicationLog()));
}

void MainWindow::createMenus()
{
    logMessage("MainWindow::createMenus()");

    mnuRecentFiles = new QMenu(tr("&Recent files"), this);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addAction(actDocumentSave);
#ifdef BETA
    mnuFile->addAction(actDocumentSaveWithSolution);
#endif
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentClose);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentImportDXF);
    mnuFile->addAction(actDocumentExportDXF);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentExportMeshFile);
    mnuFile->addAction(actDocumentSaveImage);
    mnuFile->addSeparator();
    mnuFile->addAction(actLoadBackground);
#ifndef Q_WS_MAC
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);
#endif

    mnuEdit = menuBar()->addMenu(tr("E&dit"));
    mnuEdit->addAction(actUndo);
    mnuEdit->addAction(actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCopy);
    mnuEdit->addAction(actPaste);
    mnuEdit->addSeparator();
    mnuEdit->addAction(Util::scene()->actDeleteSelected);
#ifdef Q_WS_X11
    mnuEdit->addSeparator();
    mnuEdit->addAction(actOptions);
#endif

    mnuView = menuBar()->addMenu(tr("&View"));
    mnuView->addAction(sceneView->actSceneZoomBestFit);
    mnuView->addAction(sceneView->actSceneZoomRegion);
    mnuView->addAction(sceneView->actSceneZoomIn);
    mnuView->addAction(sceneView->actSceneZoomOut);
    QMenu *mnuShow = new QMenu(tr("&Show"), this);
    mnuView->addMenu(mnuShow);
    mnuShow->addAction(sceneView->actSceneShowGrid);
    mnuShow->addAction(sceneView->actSceneSnapToGrid);
    mnuShow->addAction(sceneView->actSceneShowRulers);
    mnuView->addSeparator();
    mnuView->addAction(actFullScreen);
    mnuView->addSeparator();
    mnuView->addAction(sceneView->actSceneViewProperties);
    mnuView->addSeparator();
    mnuView->addAction(actApplicationLog);
    mnuView->addAction(actProgressLog);

    mnuProblem = menuBar()->addMenu(tr("&Problem"));
    mnuProblem->addAction(sceneView->actSceneModeNode);
    mnuProblem->addAction(sceneView->actSceneModeEdge);
    mnuProblem->addAction(sceneView->actSceneModeLabel);
    mnuProblem->addAction(sceneView->actSceneModePostprocessor);
    mnuProblem->addSeparator();
    QMenu *mnuAdd = new QMenu(tr("&Add"), this);
    mnuProblem->addMenu(mnuAdd);
    mnuAdd->addAction(Util::scene()->actNewNode);
    mnuAdd->addAction(Util::scene()->actNewEdge);
    mnuAdd->addAction(Util::scene()->actNewLabel);
    mnuAdd->addSeparator();
    mnuAdd->addAction(Util::scene()->actNewEdgeMarker);
    mnuAdd->addAction(Util::scene()->actNewLabelMarker);
    mnuProblem->addSeparator();
    mnuProblem->addAction(sceneView->actSceneViewSelectRegion);
    mnuProblem->addAction(Util::scene()->actTransform);
    mnuProblem->addSeparator();
    mnuProblem->addAction(sceneView->actPostprocessorModeLocalPointValue);
    mnuProblem->addAction(sceneView->actPostprocessorModeSurfaceIntegral);
    mnuProblem->addAction(sceneView->actPostprocessorModeVolumeIntegral);
    mnuProblem->addAction(sceneView->actSceneViewSelectMarker);
    mnuProblem->addSeparator();
    mnuProblem->addAction(actCreateMesh);
    mnuProblem->addAction(actSolve);
    mnuProblem->addAction(Util::scene()->actClearSolution);
    mnuProblem->addSeparator();
    mnuProblem->addAction(Util::scene()->actProblemProperties);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actChart);
    mnuTools->addSeparator();
    mnuTools->addAction(actScriptEditor);
    mnuTools->addAction(actScriptEditorRunScript);
    mnuTools->addAction(actScriptEditorRunCommand);
    mnuTools->addSeparator();
    mnuTools->addAction(actReport);
    mnuTools->addAction(actCreateVideo);
#ifdef Q_WS_WIN
    mnuTools->addSeparator();
    mnuTools->addAction(actOptions);
#endif

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);
    mnuHelp->addAction(actHelpShortCut);
    mnuHelp->addAction(actOnlineHelp);
    mnuHelp->addAction(actCheckVersion);
#ifndef Q_WS_MAC
    mnuHelp->addSeparator();
#else
    mnuHelp->addAction(actOptions); // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actExit);    // will be added to "Agros2D" MacOSX menu
#endif
    mnuHelp->addAction(actAbout);   // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros2D" MacOSX menu
}

void MainWindow::createToolBars()
{
    logMessage("MainWindow::createToolBars()");

    tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->addAction(actDocumentNew);
    tlbFile->addAction(actDocumentOpen);
    tlbFile->addAction(actDocumentSave);

    tlbEdit = addToolBar(tr("Edit"));
    tlbEdit->setObjectName("Edit");
    tlbEdit->addAction(actUndo);
    tlbEdit->addAction(actRedo);
    tlbEdit->addSeparator();
    // tlbEdit->addAction(actCut);
    // tlbEdit->addAction(actCopy);
    // tlbEdit->addAction(actPaste);
    tlbEdit->addAction(Util::scene()->actDeleteSelected);

    tlbView = addToolBar(tr("View"));
    tlbView->setObjectName("View");
    tlbView->addAction(sceneView->actSceneZoomBestFit);
    tlbView->addAction(sceneView->actSceneZoomRegion);
    tlbView->addAction(sceneView->actSceneZoomIn);
    tlbView->addAction(sceneView->actSceneZoomOut);

    tlbProblem = addToolBar(tr("Problem"));
    tlbProblem->setObjectName("Problem");
    tlbProblem->addAction(sceneView->actSceneModeNode);
    tlbProblem->addAction(sceneView->actSceneModeEdge);
    tlbProblem->addAction(sceneView->actSceneModeLabel);
    tlbProblem->addAction(sceneView->actSceneModePostprocessor);
    tlbProblem->addSeparator();
    tlbProblem->addAction(sceneView->actSceneViewSelectRegion);
    tlbProblem->addAction(Util::scene()->actTransform);
    tlbProblem->addSeparator();
    tlbProblem->addAction(sceneView->actPostprocessorModeLocalPointValue);
    tlbProblem->addAction(sceneView->actPostprocessorModeSurfaceIntegral);
    tlbProblem->addAction(sceneView->actPostprocessorModeVolumeIntegral);
    tlbProblem->addSeparator();
    tlbProblem->addAction(actCreateMesh);
    tlbProblem->addAction(actSolve);

    tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->hide();
    tlbTools->addAction(actChart);
    tlbTools->addAction(actScriptEditor);
    tlbTools->addSeparator();
    tlbTools->addAction(Util::scene()->actProblemProperties);
    tlbTools->addAction(sceneView->actSceneViewProperties);

    tlbTransient = addToolBar(tr("Transient"));
    tlbTransient->setObjectName("Transient");
    tlbTransient->addWidget(new QLabel(tr("Time step:") + " "));
    cmbTimeStep = new QComboBox(this);
    cmbTimeStep->setMinimumWidth(1.7*fontMetrics().width("0.00e+00"));
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStepChanged(int)));
    tlbTransient->addWidget(cmbTimeStep);

    tlbWorkspace = addToolBar(tr("Workspace"));
    tlbWorkspace->setObjectName("Workspace");
    tlbWorkspace->addAction(sceneView->actSceneShowGrid);
    tlbWorkspace->addAction(sceneView->actSceneSnapToGrid);
    tlbWorkspace->addAction(sceneView->actSceneShowRulers);
}

void MainWindow::createStatusBar()
{
    logMessage("MainWindow::createStatusBar()");

    lblMessage = new QLabel(statusBar());
    // lblMessage->setStyleSheet("QLabel {border-left: 1px solid gray;}");

    lblPosition = new QLabel(statusBar());
    lblPosition->setMinimumWidth(170);
    // lblPosition->setStyleSheet("QLabel {border: 1px solid gray;}");

    lblProblemType = new QLabel(statusBar());
    // lblProblemType->setStyleSheet("QLabel {border: 1px solid gray;}");

    lblPhysicField = new QLabel(statusBar());
    // lblPhysicField->setStyleSheet("QLabel {border: 1px solid gray;}");

    lblAnalysisType = new QLabel(statusBar());
    // lblAnalysisType->setStyleSheet("QLabel {border: 1px solid gray;}");

    statusBar()->showMessage(tr("Ready"));
    statusBar()->addPermanentWidget(lblProblemType);
    statusBar()->addPermanentWidget(lblPhysicField);
    statusBar()->addPermanentWidget(lblAnalysisType);
    statusBar()->addPermanentWidget(lblPosition);

    connect(sceneView, SIGNAL(mouseMoved(const QPointF &)), this, SLOT(doSceneMouseMoved(const QPointF &)));
}

void MainWindow::createScene()
{
    logMessage("MainWindow::createScene()");

    QHBoxLayout *layout = new QHBoxLayout;

    sceneView = new SceneView(this);
    layout->addWidget(sceneView);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);
}

void MainWindow::createViews()
{
    logMessage("MainWindow::createViews()");

    sceneInfoView = new SceneInfoView(sceneView, this);
    sceneInfoView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, sceneInfoView);

    localPointValueView = new LocalPointValueView(this);
    localPointValueView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, localPointValueView);

    volumeIntegralValueView = new VolumeIntegralValueView(this);
    volumeIntegralValueView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, volumeIntegralValueView);

    surfaceIntegralValueView = new SurfaceIntegralValueView(this);
    surfaceIntegralValueView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, surfaceIntegralValueView);

    terminalView = new TerminalView(this);
    terminalView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, terminalView);

    tooltipView = new TooltipView(this);
    tooltipView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, tooltipView);

    /*
    modelView = new ModelView(this);
    connect(Util::scene()->sceneSolution(), SIGNAL(meshed()), modelView, SLOT(invalidated()));
    modelView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, modelView);
    */
}

void MainWindow::doSceneMouseMoved(const QPointF &position)
{
    logMessage("MainWindow::doSceneMouseMoved()");

    lblPosition->setText(tr("Position: [%1; %2]").arg(position.x(), 8, 'f', 5).arg(position.y(), 8, 'f', 5));
}

void MainWindow::setRecentFiles()
{
    logMessage("MainWindow::setRecentFiles()");

    // recent files
    if (Util::scene()->problemInfo()->fileName != "")
    {
        QFileInfo fileInfo(Util::scene()->problemInfo()->fileName);
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
        actDocumentOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    logMessage("MainWindow::dragEnterEvent()");

    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    logMessage("MainWindow::dragLeacceEvent()");

    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    logMessage("MainWindow::dropEvent()");

    if (event->mimeData()->hasUrls())
    {
        QString fileName = QUrl(event->mimeData()->urls().at(0)).toLocalFile().trimmed();
        if (QFile::exists(fileName))
        {
            doDocumentOpen(fileName);

            event->acceptProposedAction();
        }
    }
}

void MainWindow::doDocumentNew()
{
    logMessage("MainWindow::doDocumentNew()");

    ProblemInfo *problemInfo = new ProblemInfo();
    ProblemDialog problemDialog(problemInfo, true, this);
    if (problemDialog.showDialog() == QDialog::Accepted)
    {
        Util::scene()->clear();
        Util::scene()->setProblemInfo(problemInfo);
        Util::scene()->refresh();

        sceneView->actSceneModeNode->trigger();
        sceneView->doZoomBestFit();
    }
    else
    {
        delete problemInfo;
    }
}

void MainWindow::doDocumentOpen(const QString &fileName)
{
    logMessage("MainWindow::doDocumentOpen()");

    QSettings settings;
    QString fileNameDocument;

    if (fileName.isEmpty())
    {
        QString dir = settings.value("General/LastProblemDir", "data").toString();

        fileNameDocument = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("Agros2D files (*.a2d *.py);;Agros2D data files (*.a2d);;Python script (*.py)"));
    }
    else
    {
        fileNameDocument = fileName;
    }

    if (fileNameDocument.isEmpty()) return;

    if (QFile::exists(fileNameDocument))
    {
    QFileInfo fileInfo(fileNameDocument);
        if (fileInfo.suffix() == "a2d")
        {
            // a2d data file
            ErrorResult result = Util::scene()->readFromFile(fileNameDocument);
            if (!result.isError())
            {
                setRecentFiles();
                settings.setValue("General/LastProblemDir", fileInfo.absolutePath());

                sceneView->actSceneModeNode->trigger();
                sceneView->doZoomBestFit();
                return;
            }
            else
            {
                result.showDialog();
                return;
            }
        }
        if (fileInfo.suffix() == "py")
        {
            // python script
            scriptEditorDialog->doFileOpen(fileNameDocument);
            scriptEditorDialog->showDialog();
            return;
        }
        QMessageBox::critical(this, tr("File open"), tr("Unknown suffix."));
    }
    else
    {
         QMessageBox::critical(this, tr("File open"), tr("File '%1' is not found.").arg(fileNameDocument));
    }
}

void MainWindow::doDocumentOpenRecent(QAction *action)
{
    logMessage("MainWindow::doDocumentOpenRecent()");

    QString fileName = action->text();
    if (QFile::exists(fileName))
    {
        ErrorResult result = Util::scene()->readFromFile(fileName);
        if (!result.isError())
        {
            setRecentFiles();

            sceneView->doZoomBestFit();
            return;
        }
        else
            result.showDialog();
    }
}

void MainWindow::doDocumentSave()
{
    logMessage("MainWindow::doDocumentSave()");

    if (QFile::exists(Util::scene()->problemInfo()->fileName))
    {
        ErrorResult result = Util::scene()->writeToFile(Util::scene()->problemInfo()->fileName);
        if (result.isError())
            result.showDialog();
    }
    else
        doDocumentSaveAs();
}

void MainWindow::doDocumentSaveWithSolution()
{
    logMessage("MainWindow::doDocumentSaveWithSolution()");

    QSettings settings;

    // save state
    bool state = settings.value("Solver/SaveProblemWithSolution", false).value<bool>();
    settings.setValue("Solver/SaveProblemWithSolution", true);

    doDocumentSave();

    settings.setValue("Solver/SaveProblemWithSolution", state);
}

void MainWindow::doDocumentSaveAs()
{
    logMessage("MainWindow::doDocumentSaveAs()");

    QSettings settings;
    QString dir = settings.value("General/LastProblemDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Agros2D files (*.a2d)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "a2d") fileName += ".a2d";

        ErrorResult result = Util::scene()->writeToFile(fileName);
        if (result.isError())
            result.showDialog();

        setRecentFiles();
        settings.setValue("General/LastProblemDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentClose()
{
    logMessage("MainWindow::doDocumentClose()");

    // WILL BE FIXED
    /*
    while (!Util::scene()->undoStack()->isClean())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"), tr("Problem has been modified.\nDo you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            doDocumentSave();
        else if (ret == QMessageBox::Discard)
            break;
        else if (ret == QMessageBox::Cancel)
            return;
    }
    */

    Util::scene()->clear();
    sceneView->doDefaultValues();
    Util::scene()->refresh();

    sceneView->actSceneModeNode->trigger();
    sceneView->doZoomBestFit();
}

void MainWindow::doDocumentImportDXF()
{
    logMessage("MainWindow::doDocumentImportDXF()");

    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        Util::scene()->readFromDxf(fileName);
        sceneView->doZoomBestFit();

        QFileInfo fileInfo(fileName);
        settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentExportDXF()
{
    logMessage("MainWindow::doDocumentExportDXF()");

    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "dxf") fileName += ".dxf";
        Util::scene()->writeToDxf(fileName);

        settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentSaveImage()
{
    logMessage("MainWindow::doDocumentSaveImage()");

    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export image to file"), dir, tr("PNG files (*.png)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "png") fileName += ".png";

        ErrorResult result = sceneView->saveImageToFile(fileName);
        if (result.isError())
            result.showDialog();

        settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doCreateVideo()
{
    logMessage("MainWindow::doCreateVideo()");

    videoDialog->showDialog();
}

void MainWindow::doCreateMesh()
{
    logMessage("MainWindow::doCreateMesh()");

    // create mesh
    Util::scene()->sceneSolution()->solve(SolverMode_Mesh);
    if (Util::scene()->sceneSolution()->isMeshed())
    {
        sceneView->actSceneModeLabel->trigger();
        sceneView->sceneViewSettings().showInitialMesh = true;
        sceneView->doInvalidated();
    }

    doInvalidated();
}

void MainWindow::doFullScreen()
{
    logMessage("MainWindow::doFullScreen()");

    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::doSolve()
{
    logMessage("MainWindow::doSolve()");

    // solve problem
    Util::scene()->sceneSolution()->solve(SolverMode_MeshAndSolve);
    if (Util::scene()->sceneSolution()->isSolved())
    {
        sceneView->actSceneModePostprocessor->trigger();

        // show local point values
        Point point = Point(0, 0);
        localPointValueView->doShowPoint(point);
    }

    doInvalidated();
    setFocus();
    activateWindow();
}

void MainWindow::doOptions()
{
    logMessage("MainWindow::doOptions()");

    ConfigDialog configDialog(this);
    if (configDialog.exec())
    {
        sceneView->timeStepChanged(true);
        sceneView->doInvalidated();
    }

    activateWindow();
}

void MainWindow::doReport()
{
    logMessage("MainWindow::doReport()");

    reportDialog->showDialog();
}

void MainWindow::doChart()
{
    logMessage("MainWindow::doChart()");

    chartDialog->showDialog();
}

void MainWindow::doScriptEditor()
{
    logMessage("MainWindow::doScriptEditor()");

    scriptEditorDialog->showDialog();
}

void MainWindow::doScriptEditorRunScript(const QString &fileName)
{
    logMessage("MainWindow::doScriptEditorRunScript()");

    QString fileNameScript;
    QSettings settings;

    if (fileName.isEmpty())
    {
        QString dir = settings.value("General/LastScriptDir").toString();

        // open dialog
        fileNameScript = QFileDialog::getOpenFileName(this, tr("Open File"), dir, tr("Python script (*.py)"));
    }
    else
    {
        fileNameScript = fileName;
    }

    if (QFile::exists(fileNameScript))
    {
        terminalView->terminal()->doPrintStdout("Run script: " + QFileInfo(fileNameScript).fileName().left(QFileInfo(fileNameScript).fileName().length() - 3) + "\n", Qt::gray);
        connectTerminal(terminalView->terminal());

        ScriptResult result = runPythonScript(readFileContent(fileNameScript), fileNameScript);
        if (result.isError)
            terminalView->terminal()->doPrintStdout(result.text + "\n", Qt::red);

        disconnectTerminal(terminalView->terminal());

        QFileInfo fileInfo(fileNameScript);
        settings.setValue("General/LastScriptDir", fileInfo.absolutePath());
    }
    else
    {
        if (!fileNameScript.isEmpty())
            QMessageBox::critical(this, tr("File open"), tr("File '%1' doesn't exists.").arg(fileNameScript));
    }
}

void MainWindow::doScriptEditorRunCommand()
{
    logMessage("MainWindow::doScriptEditorRunCommand()");

    terminalView->show();
    terminalView->activateWindow();
}

void MainWindow::doCut()
{
    logMessage("MainWindow::doCut()");
}

void MainWindow::doCopy()
{
    logMessage("MainWindow::doCopy()");

    // copy image to clipboard
    QPixmap pixmap = sceneView->renderScenePixmap();
    QApplication::clipboard()->setImage(pixmap.toImage());
}

void MainWindow::doPaste()
{
    logMessage("MainWindow::doPaste()");

    double x[] = { 0, 100, 200, 300, 400, 500, 600, 2000 };
    double y[] = { 54, 49.4, 46.9, 43.9, 40.2, 37.2, 33.5, 23.0 };

    DataTable table(x, y, 8);

    DataTableDialog dataTableDialog(this);
    dataTableDialog.setTable(&table);
    dataTableDialog.exec();
}

void MainWindow::doTimeStepChanged(int index)
{
    logMessage("MainWindow::doTimeStepChanged()");

    if (cmbTimeStep->currentIndex() != -1)
        Util::scene()->sceneSolution()->setTimeStep(cmbTimeStep->currentIndex(), false);
}

void MainWindow::doInvalidated()
{
    logMessage("MainWindow::doInvalidated()");

#ifdef BETA
    actDocumentSaveWithSolution->setEnabled(Util::scene()->sceneSolution()->isSolved());
#endif
    actChart->setEnabled(Util::scene()->sceneSolution()->isSolved());
    actCreateVideo->setEnabled(Util::scene()->sceneSolution()->isSolved() && (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient));
    tlbTransient->setEnabled(Util::scene()->sceneSolution()->isSolved());
    fillComboBoxTimeStep(cmbTimeStep);

    lblProblemType->setText(tr("Problem Type: %1").arg(problemTypeString(Util::scene()->problemInfo()->problemType)));
    lblPhysicField->setText(tr("Physic Field: %1").arg(physicFieldString(Util::scene()->problemInfo()->physicField())));
    lblAnalysisType->setText(tr("Analysis type: %1").arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)));

    //actProgressLog->setEnabled(Util::config()->enabledProgressLog);
    //actApplicationLog->setEnabled(Util::config()->enabledApplicationLog);
}

void MainWindow::doHelp()
{
    logMessage("MainWindow::doHelp()");

    Util::helpDialog()->showPage("index.html");
    Util::helpDialog()->show();
}

void MainWindow::doHelpShortCut()
{
    logMessage("MainWindow::doHelpShortCut()");

    Util::helpDialog()->showPage("getting_started/basic_control.html#shortcut-keys");
    Util::helpDialog()->show();
}

void MainWindow::doOnlineHelp()
{
    logMessage("MainWindow::doOnlineHelp()");

    QDesktopServices::openUrl(QUrl("http://hpfem.org/agros2d/help"));
}

void MainWindow::doCheckVersion()
{
    logMessage("MainWindow::doCheckVersion()");

    checkForNewVersion();
}

void MainWindow::doAbout()
{
    logMessage("MainWindow::doAbout()");

    QString str(tr("<b>Agros2D %1</b><br/> <i>hp</i>-FEM multiphysics application based on <a href=\"http://hpfem.org/hermes2d/\">Hermes2D</a> library.<br/><br/>"
                   "Web page: <a href=\"http://hpfem.org/agros2d/\">http://hpfem.org/agros2d/</a><br/>"
                   "Issues: <a href=\"http://github.com/hpfem/agros2d/issues\">http://github.com/hpfem/agros2d/issues</a><br/><br/><b>Authors:</b>"
                   "<p><table>"
                   "<tr><td>Agros2D:</td><td>Pavel Karban <a href=\"mailto:pkarban@gmail.com\">pkarban@gmail.com</a> (main developer)</td></tr>"
                   "<tr><td>&nbsp;</td><td>Frantisek Mach <a href=\"mailto:mach.frantisek@gmail.com\">mach.frantisek@gmail.com</a> (developer, documentation)</td></tr>"
                   "<tr><td>&nbsp;</td><td>Bartosz Sawicki <a href=\"mailto:sawickib@iem.pw.edu.pl\">sawickib@iem.pw.edu.pl</a> (developer, Polish translation)</td></tr>"
                   "<tr><td>Hermes 2D:&nbsp;&nbsp;</td><td>Pavel Solin <a href=\"mailto:solin@unr.edu\">solin@unr.edu</a></td></tr>"
                   "<tr><td>&nbsp;</td><td>Jakub Cerveny <a href=\"mailto:jakub.cerveny@gmail.com\">jakub.cerveny@gmail.com</a></td></tr>"
                   "<tr><td>&nbsp;</td><td>Lenka Dubcova <a href=\"mailto:dubcova@gmail.com\">dubcova@gmail.com</a></td></tr>"
                   "<tr><td>&nbsp;</td><td>Ondrej Certik <a href=\"mailto:ondrej@certik.cz\">ondrej@certik.cz</a></td></tr>"
                   "<tr><td>Nokia Qt:</td><td>Nokia Qt (<a href=\"http://qt.nokia.com/\">Qt - A cross-platform framework</a>)</td></tr>"
                   "<tr><td>Qwt:</td><td>Qwt (<a href=\"http://qwt.sourceforge.net/\">Qt Widgets for Technical Applications</a>)</td></tr>"
                   "<tr><td>Python:</td><td>Python Programming Language (<a href=\"http://www.python.org\">Python</a>)</td></tr>"
                   "<tr><td>dxflib:</td><td>Andrew Mustun (<a href=\"http://www.ribbonsoft.com/dxflib.html\">RibbonSoft</a>)</td></tr>"
                   "<tr><td>Triangle:</td><td>Jonathan Richard Shewchuk (<a href=\"http://www.cs.cmu.edu/~quake/triangle.html\">Triangle</a>)</td></tr>"
                   "<tr><td>FFmpeg:</td><td>FFmpeg group (<a href=\"http://ffmpeg.org/\">FFmpeg</a>)</td></tr>"
                   "</table></p>"
                   "<br/><b>License:</b><p>Agros2D is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.</p><p>Agros2D is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p><p>You should have received a copy of the GNU General Public License along with Agros2D. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p>").
                arg(QApplication::applicationVersion()));

    QMessageBox::about(this, tr("About Agros2D"), str);
}

void MainWindow::doDocumentExportMeshFile()
{
    logMessage("MainWindow::doDocumentExportMeshFile()");

    // generate mesh file
    bool commutator = Util::config()->deleteHermes2DMeshFile;
    if (commutator)
        Util::config()->deleteHermes2DMeshFile = !commutator;

    Util::scene()->sceneSolution()->solve(SolverMode_Mesh);
    if (Util::scene()->sceneSolution()->isMeshed())
    {
        sceneView->actSceneModeLabel->trigger();
        sceneView->sceneViewSettings().showInitialMesh = true;
        sceneView->doInvalidated();

        QSettings settings;
        QString dir = settings.value("General/LastMeshDir").toString();

        QString fileName = QFileDialog::getSaveFileName(this, tr("Export mesh file"), dir, tr("Mesh files (*.mesh)"));
        fileName.remove(".mesh");
        QFileInfo fileInfo(fileName);

        // move mesh file
        if (!Util::scene()->problemInfo()->fileName.isEmpty())
        {
            QString sourceFileName = Util::scene()->problemInfo()->fileName;
            sourceFileName.replace("a2d", "mesh");
            if (!fileName.isEmpty())
            {
                // remove existing file
                if (QFile::exists(fileName + ".mesh"))
                    QFile::remove(fileName + ".mesh");

                // copy file
                QFile::copy(sourceFileName, fileName + ".mesh");
                settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
            }

            QFile::remove(sourceFileName);
        }

        if (!fileName.isEmpty())
        {
            QFile::copy(tempProblemFileName() + ".mesh", fileName + ".mesh");
            settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
        }

        QFile::remove(tempProblemFileName() + ".mesh");
    }

    Util::config()->deleteHermes2DMeshFile = commutator;

    doInvalidated();
}

void MainWindow::doProgressLog()
{
    logMessage("MainWindow::doProgressLog()");

    logDialog->loadProgressLog();
}

void MainWindow::doApplicationLog()
{
    logMessage("MainWindow::doApplicationLog()");

    logDialog->loadApplicationLog();
}

void MainWindow::doLoadBackground()
{
    logMessage("MainWindow::doLoadBackground()");

    ImageLoaderDialog imageLoaderDialog;
    if (imageLoaderDialog.exec() == QDialog::Accepted)
    {
        sceneView->loadBackgroundImage(imageLoaderDialog.fileName(),
                                       imageLoaderDialog.position().x(),
                                       imageLoaderDialog.position().y(),
                                       imageLoaderDialog.position().width(),
                                       imageLoaderDialog.position().height());
        sceneView->refresh();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
 {
    // WILL BE FIXED
    /*
    logMessage("MainWindow::closeEvent()");

    if (!Util::scene()->undoStack()->isClean())
        doDocumentClose();

    if (Util::scene()->undoStack()->isClean() && !scriptEditorDialog->isScriptModified())
        event->accept();
    else
    {
        event->ignore();
        if (scriptEditorDialog->isScriptModified()) scriptEditorDialog->show();
    }
    */

    // check script editor
    scriptEditorDialog->closeTabs();

    if (!scriptEditorDialog->isScriptModified())
        event->accept();
    else
    {
        event->ignore();
        // show script editor
        if (scriptEditorDialog->isScriptModified())
            scriptEditorDialog->show();
    }
}
