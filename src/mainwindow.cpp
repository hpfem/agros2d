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
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "scenesolution.h"
#include "sceneinfoview.h"
#include "tooltipview.h"
#include "logview.h"
#include "postprocessorview.h"
#include "chartdialog.h"
#include "confdialog.h"
#include "pythonlabagros.h"
#include "reportdialog.h"
#include "videodialog.h"
#include "logdialog.h"
#include "problemdialog.h"
#include "progressdialog.h"
#include "collaboration.h"
#include "resultsview.h"
#include "materialbrowserdialog.h"
#include "datatabledialog.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "scenetransformdialog.h"

#include "../lib/gl2ps/gl2ps.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    logMessage("MainWindow::MainWindow()");

    Util::createSingleton();

    // FIXME: curve elements from script doesn't work
    readMeshDirtyFix();

    createScene();
    createPythonEngine(new PythonEngineAgros());

    chartDialog = new ChartDialog(this);
    scriptEditorDialog = new PythonLabAgros(currentPythonEngine(), QApplication::arguments(), this);
    reportDialog = new ReportDialog(sceneViewPost2D, this);
    videoDialog = new VideoDialog(sceneViewPost2D, this);
    logDialog = new LogDialog(this);
    collaborationDownloadDialog = new ServerDownloadDialog(this);
    sceneTransformDialog = new SceneTransformDialog(this);

    createActions();
    createViews();
    createMenus();
    createToolBars();

    connect(tabLayout, SIGNAL(currentChanged(int)), this, SLOT(doInvalidated()));
    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(Util::scene(), SIGNAL(fileNameChanged(QString)), this, SLOT(doSetWindowTitle(QString)));
    connect(Util::scene()->actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    connect(postprocessorView, SIGNAL(apply()), this, SLOT(doInvalidated()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doInvalidated()));

    // geometry
    connect(sceneViewGeometry, SIGNAL(sceneGeometryModeChanged(SceneGeometryMode)), tooltipView, SLOT(loadTooltip(SceneGeometryMode)));
    connect(sceneViewGeometry, SIGNAL(sceneGeometryModeChanged(SceneGeometryMode)), tooltipView, SLOT(loadTooltipPost2D()));    
    connect(Util::scene(), SIGNAL(cleared()), sceneViewGeometry, SLOT(clear()));
    currentPythonEngineAgros()->setSceneViewGeometry(sceneViewGeometry);

    // mesh
    connect(Util::scene(), SIGNAL(cleared()), sceneViewMesh, SLOT(clear()));
    connect(postprocessorView, SIGNAL(apply()), sceneViewMesh, SLOT(clear()));
    currentPythonEngineAgros()->setSceneViewMesh(sceneViewMesh);

    // postprocessor 2d
    connect(sceneViewPost2D, SIGNAL(mousePressed()), resultsView, SLOT(doShowResults()));
    connect(sceneViewPost2D, SIGNAL(mousePressed(const Point &)), resultsView, SLOT(doShowPoint(const Point &)));
    connect(sceneViewPost2D, SIGNAL(postprocessorModeGroupChanged(SceneModePostprocessor)), resultsView, SLOT(doPostprocessorModeGroupChanged(SceneModePostprocessor)));
    connect(sceneViewPost2D, SIGNAL(postprocessorModeGroupChanged(SceneModePostprocessor)), this, SLOT(doPostprocessorModeGroupChanged(SceneModePostprocessor)));
    connect(Util::scene(), SIGNAL(cleared()), sceneViewPost2D, SLOT(clear()));
    connect(postprocessorView, SIGNAL(apply()), sceneViewPost2D, SLOT(clear()));
    currentPythonEngineAgros()->setSceneViewPost2D(sceneViewPost2D);

    // postprocessor 3d
    currentPythonEngineAgros()->setSceneViewPost3D(sceneViewPost3D);

    connect(Util::scene(), SIGNAL(fieldsChanged()), this, SLOT(doFieldsChanged()));

    sceneViewGeometry->clear();
    sceneViewMesh->clear();
    sceneViewPost2D->clear();
    sceneViewPost3D->clear();

    connect(chartDialog, SIGNAL(setChartLine(ChartLine)), sceneViewPost2D, SLOT(doSetChartLine(ChartLine)));

    QSettings settings;
    restoreGeometry(settings.value("MainWindow/Geometry", saveGeometry()).toByteArray());
    recentFiles = settings.value("MainWindow/RecentFiles").value<QStringList>();
    restoreState(settings.value("MainWindow/State", saveState()).toByteArray());

    Util::scene()->clear();

    sceneViewGeometry->actSceneModeGeometry->trigger();
    sceneViewGeometry->doZoomBestFit();

    // set recent files
    setRecentFiles();

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
            {
                consoleView->console()->connectStdOut();
                runPythonScript(readFileContent(scriptName));
                consoleView->console()->disconnectStdOut();
            }
            else
            {
                qWarning() << "Script " << scriptName << "not found.";
            }

            continue;
        }

        QString fileName = args[i];
        open(fileName);
    }

    // DataTableDialog *dataTableDialog = new DataTableDialog(this);
    // dataTableDialog->show();
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

    actDocumentDownloadFromServer = new QAction(icon(""), tr("&Download from server..."), this);
    actDocumentDownloadFromServer->setShortcut(tr("Ctrl+Shift+O"));
    actDocumentDownloadFromServer->setStatusTip(tr("Download from server..."));
    connect(actDocumentDownloadFromServer, SIGNAL(triggered()), this, SLOT(doDocumentDownloadFromServer()));

    actDocumentSave = new QAction(icon("document-save"), tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    actDocumentSave->setStatusTip(tr("Save the file to disk"));
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));

    actDocumentSaveWithSolution = new QAction(icon(""), tr("Save with solution"), this);
    actDocumentSaveWithSolution->setStatusTip(tr("Save the file to disk with solution"));
    connect(actDocumentSaveWithSolution, SIGNAL(triggered()), this, SLOT(doDocumentSaveWithSolution()));

    actDocumentSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    actDocumentSaveAs->setStatusTip(tr("Save the file under a new name"));
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));

    actDocumentUploadToServer = new QAction(icon(""), tr("Upload to server..."), this);
    actDocumentUploadToServer->setStatusTip(tr("Upload to server..."));
    connect(actDocumentUploadToServer, SIGNAL(triggered()), this, SLOT(doDocumentUploadToServer()));

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

    actExportVTKScalar = new QAction(tr("Export VTK scalar..."), this);
    actExportVTKScalar->setStatusTip(tr("Export scalar view as VTK file"));
    connect(actExportVTKScalar, SIGNAL(triggered()), this, SLOT(doExportVTKScalar()));

    actExportVTKOrder = new QAction(tr("Export VTK order..."), this);
    actExportVTKOrder->setStatusTip(tr("Export order view as VTK file"));
    connect(actExportVTKOrder, SIGNAL(triggered()), this, SLOT(doExportVTKOrder()));

    actDocumentSaveImage = new QAction(tr("Export image..."), this);
    actDocumentSaveImage->setStatusTip(tr("Export image to file"));
    connect(actDocumentSaveImage, SIGNAL(triggered()), this, SLOT(doDocumentSaveImage()));

    actDocumentSaveGeometry = new QAction(tr("Export geometry..."), this);
    actDocumentSaveGeometry->setStatusTip(tr("Export geometry to file"));
    connect(actDocumentSaveGeometry, SIGNAL(triggered()), this, SLOT(doDocumentSaveGeometry()));

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

    actCopy = new QAction(icon("edit-copy"), tr("Copy image to clipboard"), this);
    actCopy->setShortcuts(QKeySequence::Copy);
    actCopy->setStatusTip(tr("Copy image from workspace to clipboard."));
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopy()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setStatusTip(tr("Show help"));
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpShortCut = new QAction(icon(""), tr("&Shortcuts"), this);
    actHelpShortCut->setStatusTip(tr("Shortcuts"));
    connect(actHelpShortCut, SIGNAL(triggered()), this, SLOT(doHelpShortCut()));

    actCollaborationServer = new QAction(icon("collaboration"), tr("Collaboration server"), this);
    actCollaborationServer->setStatusTip(tr("Collaboration server..."));
    connect(actCollaborationServer, SIGNAL(triggered()), this, SLOT(doCollaborationServer()));

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

    actCreateMesh = new QAction(icon("scene-mesh"), tr("&Mesh"), this);
    actCreateMesh->setShortcut(QKeySequence(tr("Alt+W")));
    actCreateMesh->setStatusTip(tr("Mesh area"));
    connect(actCreateMesh, SIGNAL(triggered()), this, SLOT(doCreateMesh()));

    actSolve = new QAction(icon("run"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    actSolve->setStatusTip(tr("Solve problem"));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actSolveAdaptiveStep = new QAction(icon("run-step"), tr("Adapt. step"), this);
    actSolveAdaptiveStep->setStatusTip(tr("Adaptivity step"));
    connect(actSolveAdaptiveStep, SIGNAL(triggered()), this, SLOT(doSolveAdaptiveStep()));

    actChart = new QAction(icon("chart"), tr("&Chart"), this);
    actChart->setStatusTip(tr("Chart"));
    connect(actChart, SIGNAL(triggered()), this, SLOT(doChart()));

    actFullScreen = new QAction(icon("view-fullscreen"), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actScriptEditor = new QAction(icon("script-python"), tr("PythonLab"), this);
    actScriptEditor->setStatusTip(tr("Script editor"));
    actScriptEditor->setShortcut(Qt::Key_F9);
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

    actMaterialBrowser = new QAction(icon(""), tr("Material browser..."), this);
    actMaterialBrowser->setStatusTip(tr("Material browser"));
    connect(actMaterialBrowser, SIGNAL(triggered()), this, SLOT(doMaterialBrowser()));

    actProgressLog = new QAction(icon("log"), tr("Progress &log"), this);
    actProgressLog->setStatusTip(tr("Show progress log"));
    connect(actProgressLog, SIGNAL(triggered()), this, SLOT(doProgressLog()));

    actApplicationLog = new QAction(icon("log"), tr("Application &log"), this);
    actApplicationLog->setStatusTip(tr("Show application log"));
    connect(actApplicationLog, SIGNAL(triggered()), this, SLOT(doApplicationLog()));

    // zoom actions (geometry, post2d and post3d)
    // scene - zoom
    actSceneZoomIn = new QAction(icon("zoom-in"), tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);
    actSceneZoomIn->setStatusTip(tr("Zoom in"));

    actSceneZoomOut = new QAction(icon("zoom-out"), tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);
    actSceneZoomOut->setStatusTip(tr("Zoom out"));

    actSceneZoomBestFit = new QAction(icon("zoom-original"), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));
    actSceneZoomBestFit->setStatusTip(tr("Best fit"));

    actSceneZoomRegion = new QAction(icon("zoom-fit-best"), tr("Zoom region"), this);
    actSceneZoomRegion->setStatusTip(tr("Zoom region"));
    actSceneZoomRegion->setCheckable(true);

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(sceneViewGeometry->actSceneModeGeometry);
    actSceneModeGroup->addAction(sceneViewMesh->actSceneModeMesh);
    actSceneModeGroup->addAction(sceneViewPost2D->actSceneModePost2D);
    actSceneModeGroup->addAction(sceneViewPost3D->actSceneModePost3D);
}


void MainWindow::doFieldsChanged()
{
    //TODO it is not necessary to create whole menu, it only needs to be adjusted
    //TODO what hapens to old menu object? Memory leaks???

    createMenus();
}

void MainWindow::createMenus()
{
    logMessage("MainWindow::createMenus()");

    menuBar()->clear();

    mnuRecentFiles = new QMenu(tr("&Recent files"), this);
    mnuFileImportExport = new QMenu(tr("Import/Export"), this);
    mnuFileImportExport->addAction(actDocumentImportDXF);
    mnuFileImportExport->addAction(actDocumentExportDXF);
    mnuFileImportExport->addSeparator();
    mnuFileImportExport->addAction(actDocumentExportMeshFile);
    mnuFileImportExport->addAction(actDocumentSaveImage);
    mnuFileImportExport->addAction(actDocumentSaveGeometry);
    mnuFileImportExport->addSeparator();
    mnuFileImportExport->addAction(actExportVTKScalar);
    mnuFileImportExport->addAction(actExportVTKOrder);

    QMenu *mnuServer = new QMenu(tr("Colaboration"), this);
    mnuServer->addAction(actDocumentDownloadFromServer);
    mnuServer->addAction(actDocumentUploadToServer);
    mnuServer->addAction(actCollaborationServer);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentSave);
    if (Util::config()->showExperimentalFeatures)
        mnuFile->addAction(actDocumentSaveWithSolution);
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuFileImportExport);
    mnuFile->addMenu(mnuServer);
    mnuFile->addSeparator();
#ifndef Q_WS_MAC
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentClose);
    mnuFile->addAction(actExit);
#endif

    mnuEdit = menuBar()->addMenu(tr("E&dit"));
    mnuEdit->addAction(actUndo);
    mnuEdit->addAction(actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(Util::scene()->actDeleteSelected);
    mnuEdit->addSeparator();
    mnuEdit->addAction(sceneViewGeometry->actSceneViewSelectRegion);
    mnuEdit->addAction(Util::scene()->actTransform);
#ifdef Q_WS_X11
    mnuEdit->addSeparator();
    mnuEdit->addAction(actOptions);
#endif

    QMenu *mnuProjection = new QMenu(tr("Projection"), this);
    mnuProjection->addAction(sceneViewPost3D->actSetProjectionXY);
    mnuProjection->addAction(sceneViewPost3D->actSetProjectionXZ);
    mnuProjection->addAction(sceneViewPost3D->actSetProjectionYZ);

    QMenu *mnuShowPanels = new QMenu(tr("Panels"), this);
    mnuShowPanels->addAction(sceneInfoView->toggleViewAction());
    mnuShowPanels->addAction(resultsView->toggleViewAction());
    mnuShowPanels->addAction(postprocessorView->toggleViewAction());
    mnuShowPanels->addAction(consoleView->toggleViewAction());
    mnuShowPanels->addAction(tooltipView->toggleViewAction());

    mnuView = menuBar()->addMenu(tr("&View"));
    mnuView->addAction(sceneViewGeometry->actSceneModeGeometry);
    mnuView->addAction(sceneViewMesh->actSceneModeMesh);
    mnuView->addAction(sceneViewPost2D->actSceneModePost2D);
    mnuView->addAction(sceneViewPost3D->actSceneModePost3D);
    mnuView->addSeparator();
    mnuView->addAction(actSceneZoomBestFit);
    mnuView->addAction(actSceneZoomIn);
    mnuView->addAction(actSceneZoomOut);
    mnuView->addAction(actSceneZoomRegion);
    mnuView->addMenu(mnuProjection);
    mnuView->addSeparator();
    mnuView->addAction(actCopy);
    mnuView->addAction(actLoadBackground);
    mnuView->addSeparator();
    mnuView->addMenu(mnuShowPanels);
    mnuView->addSeparator();
    mnuView->addAction(actFullScreen);

    mnuProblem = menuBar()->addMenu(tr("&Problem"));
    QMenu *mnuAdd = new QMenu(tr("&Add"), this);
    mnuProblem->addMenu(mnuAdd);
    mnuAdd->addAction(Util::scene()->actNewNode);
    mnuAdd->addAction(Util::scene()->actNewEdge);
    mnuAdd->addAction(Util::scene()->actNewLabel);
    mnuAdd->addSeparator();
    Util::scene()->addBoundaryAndMaterialMenuItems(mnuAdd, this);
    mnuProblem->addSeparator();
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeLocalPointValue);
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeSurfaceIntegral);
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeVolumeIntegral);
    mnuProblem->addAction(sceneViewPost2D->actSceneViewSelectByMarker);
    mnuProblem->addSeparator();
    mnuProblem->addAction(actCreateMesh);
    mnuProblem->addAction(actSolve);
    mnuProblem->addAction(actSolveAdaptiveStep);
    mnuProblem->addAction(Util::scene()->actClearSolutions);
    mnuProblem->addSeparator();
    mnuProblem->addAction(Util::scene()->actProblemProperties);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actChart);
    mnuTools->addSeparator();
    mnuTools->addAction(actScriptEditor);
    mnuTools->addAction(actScriptEditorRunScript);
    mnuTools->addAction(actScriptEditorRunCommand);
    mnuTools->addSeparator();
    mnuTools->addAction(actMaterialBrowser);
    mnuTools->addSeparator();
    mnuTools->addAction(actReport);
    mnuTools->addAction(actCreateVideo);
    mnuTools->addSeparator();
    mnuTools->addAction(actApplicationLog);
    mnuTools->addAction(actProgressLog);
#ifdef Q_WS_WIN
    mnuTools->addSeparator();
    mnuTools->addAction(actOptions);
#endif

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);
    mnuHelp->addAction(actOnlineHelp);
    mnuHelp->addAction(actHelpShortCut);
    mnuHelp->addAction(actCollaborationServer);
#ifndef Q_WS_MAC
    mnuHelp->addSeparator();
#else
    mnuHelp->addAction(actOptions); // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actExit);    // will be added to "Agros2D" MacOSX menu
#endif
    mnuHelp->addSeparator();
    mnuHelp->addAction(actCheckVersion);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros2D" MacOSX menu
}

void MainWindow::createToolBars()
{
    // spacing
    QLabel *spacing = new QLabel;
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // left toolbar
    QToolBar *leftToolBar = new QToolBar();
    leftToolBar->setObjectName("Problem");
    leftToolBar->setMovable(false);
    leftToolBar->setStyleSheet("QToolBar { border: 1px solid rgba(200, 200, 200, 255); }"
                               "QToolBar:left, QToolBar:right { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:1 rgba(120, 120, 120, 255)); }"
                               "QToolButton { border: 0px; color: rgba(230, 230, 230, 255); font: bold; font-size: 8pt; width: 65px; }"
                               "QToolButton:hover { border: 0px; background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:0.5 rgba(160, 160, 160, 255), stop:1 rgba(150, 150, 150, 255)); }"
                               "QToolButton:checked:hover, QToolButton:checked { border: 0px; color: rgba(30, 30, 30, 255); background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(160, 160, 160, 255), stop:0.5 rgba(220, 220, 220, 255), stop:1 rgba(160, 160, 160, 255)); }");

    leftToolBar->setIconSize(QSize(32, 32));
    leftToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    leftToolBar->addAction(sceneViewGeometry->actSceneModeGeometry);
    leftToolBar->addAction(sceneViewMesh->actSceneModeMesh);
    leftToolBar->addAction(sceneViewPost2D->actSceneModePost2D);
    leftToolBar->addAction(sceneViewPost3D->actSceneModePost3D);
    leftToolBar->addWidget(spacing);
    leftToolBar->addAction(actCreateMesh);
    leftToolBar->addAction(actSolve);
    leftToolBar->addAction(actSolveAdaptiveStep);
    leftToolBar->addSeparator();
    leftToolBar->addAction(actScriptEditor);
    leftToolBar->addAction(Util::scene()->actProblemProperties);

    addToolBar(Qt::LeftToolBarArea, leftToolBar);

    // top toolbar
#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->setMovable(false);
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbFile->addAction(actDocumentNew);
    tlbFile->addAction(actDocumentOpen);
    tlbFile->addAction(actDocumentSave);

    tlbView = addToolBar(tr("View"));
    tlbView->setObjectName("View");
    tlbView->setMovable(false);
#ifdef Q_WS_MAC
    tlbView->setFixedHeight(iconHeight);
    tlbView->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbView->addAction(actSceneZoomBestFit);
    tlbView->addAction(actSceneZoomRegion);
    tlbView->addAction(actSceneZoomIn);
    tlbView->addAction(actSceneZoomOut);

    tlbGeometry = addToolBar(tr("Geometry"));
    tlbGeometry->setObjectName("Geometry");
    tlbGeometry->setMovable(false);
#ifdef Q_WS_MAC
    tlbProblem->setFixedHeight(iconHeight);
    tlbProblem->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(actUndo);
    tlbGeometry->addAction(actRedo);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(sceneViewGeometry->actOperateOnNodes);
    tlbGeometry->addAction(sceneViewGeometry->actOperateOnEdges);
    tlbGeometry->addAction(sceneViewGeometry->actOperateOnLabels);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(sceneViewGeometry->actSceneViewSelectRegion);
    tlbGeometry->addAction(Util::scene()->actTransform);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(Util::scene()->actDeleteSelected);

    tlbPost2D = addToolBar(tr("Postprocessor 2D"));
    tlbPost2D->setObjectName("Postprocessor 2D");
    tlbPost2D->setMovable(false);
#ifdef Q_WS_MAC
    tlbPost2D->setFixedHeight(iconHeight);
    tlbPost2D->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbPost2D->addSeparator();
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeLocalPointValue);
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeSurfaceIntegral);
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeVolumeIntegral);
    tlbPost2D->addSeparator();
    tlbPost2D->addAction(sceneViewPost2D->actSceneViewSelectByMarker);
    tlbPost2D->addAction(actChart);
}

void MainWindow::createScene()
{
    logMessage("MainWindow::createScene()");

    sceneViewGeometry = new SceneViewGeometry(this);
    sceneViewMesh = new SceneViewMesh(this);
    sceneViewPost2D = new SceneViewPost2D(this);
    sceneViewPost3D = new SceneViewPost3D(this);

    sceneViewGeometryWidget = new SceneViewWidget(sceneViewGeometry, this);
    sceneViewMeshWidget = new SceneViewWidget(sceneViewMesh, this);
    sceneViewPost2DWidget = new SceneViewWidget(sceneViewPost2D, this);
    sceneViewPost3DWidget = new SceneViewWidget(sceneViewPost3D, this);

    tabLayout = new QStackedLayout();
    tabLayout->addWidget(sceneViewGeometryWidget);
    tabLayout->addWidget(sceneViewMeshWidget);
    tabLayout->addWidget(sceneViewPost2DWidget);
    tabLayout->addWidget(sceneViewPost3DWidget);

    QWidget *main = new QWidget();
    main->setLayout(tabLayout);

    setCentralWidget(main);
}

void MainWindow::createViews()
{
    logMessage("MainWindow::createViews()");

    sceneInfoView = new SceneInfoView(sceneViewGeometry, this);
    sceneInfoView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, sceneInfoView);

    resultsView = new ResultsView(this);
    resultsView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, resultsView);

    postprocessorView = new PostprocessorView(sceneViewGeometry, sceneViewMesh, sceneViewPost2D, sceneViewPost3D, this);
    postprocessorView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, postprocessorView);

    consoleView = new PythonScriptingConsoleView(currentPythonEngine(), this);
    consoleView->setAllowedAreas(Qt::AllDockWidgetAreas);
    consoleView->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, consoleView);

    tooltipView = new TooltipView(this);
    tooltipView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, tooltipView);

    logView = new LogView(this);
    logView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, logView);

    // tabify dock together
    tabifyDockWidget(sceneInfoView, postprocessorView);

    // raise scene info view
    sceneInfoView->raise();
}

void MainWindow::doMouseSceneModeChanged(MouseSceneMode mouseSceneMode)
{
    /*
    lblMouseMode->setText("Mode: -");

    switch (mouseSceneMode)
    {
    case MouseSceneMode_Add:
    {
        switch (sceneViewGeometry->sceneMode())
        {
        case SceneGeometryMode_OperateOnNodes:
            lblMouseMode->setText(tr("Mode: Add node"));
            break;
        case SceneGeometryMode_OperateOnEdges:
            lblMouseMode->setText(tr("Mode: Add edge"));
            break;
        case SceneGeometryMode_OperateOnLabels:
            lblMouseMode->setText(tr("Mode: Add label"));
            break;
        default:
            break;
        }
    }
        break;
    case MouseSceneMode_Pan:
        lblMouseMode->setText(tr("Mode: Pan"));
        break;
    case MouseSceneMode_Rotate:
        lblMouseMode->setText(tr("Mode: Rotate"));
        break;
    case MouseSceneMode_Move:
    {
        switch (sceneViewGeometry->sceneMode())
        {
        case SceneGeometryMode_OperateOnNodes:
            lblMouseMode->setText(tr("Mode: Move node"));
            break;
        case SceneGeometryMode_OperateOnEdges:
            lblMouseMode->setText(tr("Mode: Move edge"));
            break;
        case SceneGeometryMode_OperateOnLabels:
            lblMouseMode->setText(tr("Mode: Move label"));
            break;
        default:
            break;
        }
    }
        break;
    default:
        break;
    }
    */
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
    ProblemDialog problemDialog(problemInfo, QMap<QString, FieldInfo *>(), QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >(),
                                true, this);
    if (problemDialog.showDialog() == QDialog::Accepted)
    {
        Util::scene()->clear();
        Util::scene()->setProblemInfo(problemInfo);
        Util::scene()->refresh();

        sceneViewGeometry->actOperateOnNodes->trigger();
        sceneViewGeometry->doZoomBestFit();
        sceneViewMesh->doZoomBestFit();
        sceneViewPost2D->doZoomBestFit();
        sceneViewPost3D->doZoomBestFit();
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

                sceneViewGeometry->actOperateOnNodes->trigger();
                sceneViewGeometry->doZoomBestFit();
                sceneViewMesh->doZoomBestFit();
                sceneViewPost2D->doZoomBestFit();
                sceneViewPost3D->doZoomBestFit();

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

void MainWindow::doDocumentDownloadFromServer()
{
    if (collaborationDownloadDialog->showDialog() == QDialog::Accepted)
    {
        if (QFile::exists(collaborationDownloadDialog->fileName()))
            doDocumentOpen(collaborationDownloadDialog->fileName());
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

            sceneViewGeometry->actOperateOnNodes->trigger();
            sceneViewGeometry->doZoomBestFit();
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
    }
}

void MainWindow::doDocumentUploadToServer()
{
    // save
    doDocumentSave();

    ServerUploadDialog *cloud = new ServerUploadDialog(this);
    cloud->showDialog();
    delete cloud;
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
    sceneViewGeometry->clear();
    sceneViewMesh->clear();
    sceneViewPost2D->clear();
    sceneViewPost3D->clear();
    Util::scene()->refresh();

    sceneViewGeometry->actOperateOnNodes->trigger();
    sceneViewGeometry->doZoomBestFit();
    sceneViewMesh->doZoomBestFit();
    sceneViewPost2D->doZoomBestFit();
    sceneViewPost3D->doZoomBestFit();
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
        sceneViewGeometry->doZoomBestFit();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
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

        if (fileInfo.absoluteDir() != tempProblemDir())
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

        ErrorResult result = sceneViewPost2D->saveImageToFile(fileName);
        if (result.isError())
            result.showDialog();

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentSaveGeometry()
{
    logMessage("MainWindow::doDocumentSaveGeometry()");

    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();


    QString selected;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export geometry to file"), dir,
                                                    tr("PDF files (*.pdf);;EPS files (*.eps);;SVG files (*.svg)"),
                                                    &selected);

    if (!fileName.isEmpty())
    {
        int format = GL2PS_PDF;
        QFileInfo fileInfo(fileName);

        if (selected == "PDF files (*.pdf)")
        {
            if (fileInfo.suffix().toLower() != "pdf") fileName += ".pdf";
            format = GL2PS_PDF;
        }
        if (selected == "EPS files (*.eps)")
        {
            if (fileInfo.suffix().toLower() != "eps") fileName += ".eps";
            format = GL2PS_EPS;
        }
        if (selected == "SVG files (*.svg)")
        {
            if (fileInfo.suffix().toLower() != "svg") fileName += ".svg";
            format = GL2PS_SVG;
        }

        ErrorResult result = sceneViewGeometry->saveGeometryToFile(fileName, format);
        if (result.isError())
            result.showDialog();

        if (fileInfo.absoluteDir() != tempProblemDir())
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
    LogDialog2 *logDialog = new LogDialog2(this, tr("Mesh"));
    logDialog->show();

    // create mesh
    Util::problem()->solve(SolverMode_Mesh);
    if (Util::problem()->isMeshed())
    {
        // raise mesh viewer
        sceneViewMesh->actSceneModeMesh->trigger();

        // successful run
        logDialog->close();
    }

    doInvalidated();
}

void MainWindow::doSolve()
{
    LogDialog2 *logDialog = new LogDialog2(this, tr("Solver"));
    logDialog->show();

    // solve problem
    Util::problem()->solve(SolverMode_MeshAndSolve);
    if (Util::problem()->isSolved())
    {
        sceneViewPost2D->actSceneModePost2D->trigger();

        // show local point values
        Point point = Point(0, 0);
        resultsView->doShowPoint(point);

        // raise postprocessor
        postprocessorView->raise();

        // successful run
        logDialog->close();
    }

    doInvalidated();
    setFocus();
    activateWindow();
}

void MainWindow::doSolveAdaptiveStep()
{
    LogDialog2 *logDialog = new LogDialog2(this, tr("Adaptive step"));
    logDialog->show();

    // solve problem
    Util::problem()->solve(SolverMode_SolveAdaptiveStep);
    if (Util::problem()->isSolved())
    {
        sceneViewPost2D->actSceneModePost2D->trigger();

        // show local point values
        Point point = Point(0, 0);
        resultsView->doShowPoint(point);

        // raise postprocessor
        postprocessorView->raise();

        // successful run
        logDialog->close();
    }

    doInvalidated();
    setFocus();
    activateWindow();
}

void MainWindow::doFullScreen()
{
    logMessage("MainWindow::doFullScreen()");

    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::doOptions()
{
    logMessage("MainWindow::doOptions()");

    ConfigDialog configDialog(this);
    if (configDialog.exec())
    {
        sceneViewPost2D->timeStepChanged(false);
        sceneViewPost2D->doInvalidated();
    }

    activateWindow();
}

void MainWindow::doReport()
{
    reportDialog->showDialog();
}

void MainWindow::doTransform()
{
    sceneTransformDialog->showDialog();
}

void MainWindow::doMaterialBrowser()
{
    logMessage("MainWindow::doMaterialBrowser()");

    MaterialBrowserDialog materialBrowserDialog(this);
    materialBrowserDialog.showDialog(false);
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
        consoleView->console()->consoleMessage("Run script: " + QFileInfo(fileNameScript).fileName().left(QFileInfo(fileNameScript).fileName().length() - 3) + "\n",
                                               Qt::gray);

        consoleView->console()->connectStdOut();
        ScriptResult result = runPythonScript(readFileContent(fileNameScript), fileNameScript);
        consoleView->console()->disconnectStdOut();

        if (result.isError)
            consoleView->console()->stdErr(result.text);

        consoleView->console()->appendCommandPrompt();

        QFileInfo fileInfo(fileNameScript);
        if (fileInfo.absoluteDir() != tempProblemDir())
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

    consoleView->show();
    consoleView->activateWindow();
}

void MainWindow::doCut()
{
    logMessage("MainWindow::doCut()");
}

void MainWindow::doCopy()
{
    logMessage("MainWindow::doCopy()");

    // copy image to clipboard
    QPixmap pixmap = sceneViewPost2D->renderScenePixmap();
    QApplication::clipboard()->setImage(pixmap.toImage());
}

void MainWindow::doPaste()
{
    logMessage("MainWindow::doPaste()");
}

void MainWindow::doTimeStepChanged(int index)
{
    logMessage("MainWindow::doTimeStepChanged()");

    if (cmbTimeStep->currentIndex() != -1)
    {
        Util::scene()->setActiveTimeStep(cmbTimeStep->currentIndex());
        postprocessorView->updateControls();
    }
}

void MainWindow::doInvalidated()
{
    logMessage("MainWindow::doInvalidated()");

    if (Util::config()->showExperimentalFeatures)
        actDocumentSaveWithSolution->setEnabled(Util::problem()->isSolved());

    // set controls
    Util::scene()->actTransform->setEnabled(false);

    actSceneZoomRegion->setChecked(false);
    sceneViewGeometry->actSceneZoomRegion = NULL;
    sceneViewMesh->actSceneZoomRegion = NULL;
    sceneViewPost2D->actSceneZoomRegion = NULL;
    sceneViewPost3D->actSceneZoomRegion = NULL;

    tlbGeometry->setVisible(false);
    tlbPost2D->setVisible(false);

    if (sceneViewGeometry->actSceneModeGeometry->isChecked())
    {
        tabLayout->setCurrentWidget(sceneViewGeometryWidget);
        Util::scene()->actTransform->setEnabled(true);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewGeometry, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewGeometry, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewGeometry, SLOT(doZoomBestFit()));
        sceneViewGeometry->actSceneZoomRegion = actSceneZoomRegion;

        sceneInfoView->raise();
        tlbGeometry->setVisible(true);
    }
    if (sceneViewMesh->actSceneModeMesh->isChecked())
    {
        tabLayout->setCurrentWidget(sceneViewMeshWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewMesh, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewMesh, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewMesh, SLOT(doZoomBestFit()));
        sceneViewPost2D->actSceneZoomRegion = actSceneZoomRegion;

        postprocessorView->raise();
    }
    if (sceneViewPost2D->actSceneModePost2D->isChecked())
    {
        tabLayout->setCurrentWidget(sceneViewPost2DWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewPost2D, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewPost2D, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewPost2D, SLOT(doZoomBestFit()));
        sceneViewPost2D->actSceneZoomRegion = actSceneZoomRegion;

        // hide transform dialog
        sceneTransformDialog->hide();

        postprocessorView->raise();
        tlbPost2D->setVisible(true);
    }
    if (sceneViewPost3D->actSceneModePost3D->isChecked())
    {
        tabLayout->setCurrentWidget(sceneViewPost3DWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewPost3D, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewPost3D, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewPost3D, SLOT(doZoomBestFit()));
        actSceneZoomRegion->setEnabled(false);

        // hide transform dialog
        sceneTransformDialog->hide();

        postprocessorView->raise();
    }

    //    actSolveAdaptiveStep->setEnabled(Util::problem()->isSolved() && Util::scene()->fieldInfo("TODO")->analysisType() != AnalysisType_Transient); // FIXME: timedep
    actChart->setEnabled(Util::problem()->isSolved());

    actExportVTKScalar->setEnabled(Util::problem()->isSolved());
    actExportVTKOrder->setEnabled(Util::problem()->isSolved());

    QTimer::singleShot(0, postprocessorView, SLOT(updateControls()));

    // set current timestep
    //    cmbTimeStep->setCurrentIndex(Util::problem()->timeStep());

    //actProgressLog->setEnabled(Util::config()->enabledProgressLog);
    //actApplicationLog->setEnabled(Util::config()->enabledApplicationLog);
}

void MainWindow::doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor)
{
    //resultsView->raise();
    if (sceneModePostprocessor == SceneModePostprocessor_LocalValue)
        resultsView->doShowPoint();
    else if (sceneModePostprocessor == SceneModePostprocessor_SurfaceIntegral)
        resultsView->doShowSurfaceIntegral();
    else if (sceneModePostprocessor == SceneModePostprocessor_VolumeIntegral)
        resultsView->doShowVolumeIntegral();
}

void MainWindow::doHelp()
{
    logMessage("MainWindow::doHelp()");

    showPage("index.html");
}

void MainWindow::doHelpShortCut()
{
    logMessage("MainWindow::doHelpShortCut()");

    showPage("getting_started/shortcut_keys.html");
}

void MainWindow::doCollaborationServer()
{
    logMessage("MainWindow::doCollaborationServer()");

    QDesktopServices::openUrl(QUrl(Util::config()->collaborationServerURL + "problems.php"));
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
    AboutDialog about(this);
    about.exec();
}

void MainWindow::doDocumentExportMeshFile()
{
    logMessage("MainWindow::doDocumentExportMeshFile()");

    // generate mesh file
    bool commutator = Util::config()->deleteHermes2DMeshFile;
    if (commutator)
        Util::config()->deleteHermes2DMeshFile = !commutator;

    Util::problem()->solve(SolverMode_Mesh);
    if (Util::problem()->isMeshed())
    {
        tabLayout->setCurrentWidget(sceneViewGeometry);
        Util::config()->showInitialMeshView = true;
        sceneViewGeometry->doInvalidated();

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
                if (fileInfo.absoluteDir() != tempProblemDir())
                    settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
            }

            QFile::remove(sourceFileName);
        }

        if (!fileName.isEmpty())
        {
            QFile::copy(tempProblemFileName() + ".mesh", fileName + ".mesh");
            if (fileInfo.absoluteDir() != tempProblemDir())
                settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
        }

        QFile::remove(tempProblemFileName() + ".mesh");
    }

    Util::config()->deleteHermes2DMeshFile = commutator;

    doInvalidated();
}

void MainWindow::doExportVTKScalar()
{
    assert(0); //TODO
    //    logMessage("MainWindow::doDocumentExportVTKScalar()");
    //    if (Util::problem()->isSolved())
    //    {
    //        QSettings settings;
    //        QString dir = settings.value("General/LastVTKDir").toString();

    //        QString fileName = QFileDialog::getSaveFileName(this, tr("Export vtk file"), dir, tr("VTK files (*.vtk)"));
    //        if (fileName.isEmpty())
    //            return;

    //        if (!fileName.endsWith(".vtk"))
    //            fileName.append(".vtk");

    //        // remove existing file
    //        if (QFile::exists(fileName))
    //            QFile::remove(fileName);

    //        Util::problem()->linScalarView().save_solution_vtk(Util::problem()->sln(Util::scene()->problemInfo()->timeStep.number() * Util::scene()->problemInfo()->module()->number_of_solution()),
    //                                                                          fileName.toStdString().c_str(),
    //                                                                          sceneView->sceneViewSettings().scalarPhysicFieldVariable.c_str(),
    //                                                                          true);

    //        if (!fileName.isEmpty())
    //        {
    //            QFileInfo fileInfo(fileName);
    //            if (fileInfo.absoluteDir() != tempProblemDir())
    //                settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
    //        }
    //    }
}

void MainWindow::doExportVTKOrder()
{
    assert(0);
    //    logMessage("MainWindow::doDocumentExportVTKOrder()");
    //    if (Util::problem()->isSolved())
    //    {
    //        QSettings settings;
    //        QString dir = settings.value("General/LastVTKDir").toString();

    //        QString fileName = QFileDialog::getSaveFileName(this, tr("Export vtk file"), dir, tr("VTK files (*.vtk)"));
    //        if (fileName.isEmpty())
    //            return;

    //        if (!fileName.endsWith(".vtk"))
    //            fileName.append(".vtk");

    //        // remove existing file
    //        if (QFile::exists(fileName))
    //            QFile::remove(fileName);

    //        Util::problem()->ordView().save_orders_vtk(Util::problem()->space(Util::scene()->problemInfo()->timeStep.number() * Util::scene()->problemInfo()->module()->number_of_solution()),
    //                                                                  fileName.toStdString().c_str());

    //        if (!fileName.isEmpty())
    //        {
    //            QFileInfo fileInfo(fileName);
    //            if (fileInfo.absoluteDir() != tempProblemDir())
    //                settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
    //        }
    //    }
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
        sceneViewPost2D->loadBackgroundImage(imageLoaderDialog.fileName(),
                                             imageLoaderDialog.position().x(),
                                             imageLoaderDialog.position().y(),
                                             imageLoaderDialog.position().width(),
                                             imageLoaderDialog.position().height());
        sceneViewPost2D->refresh();
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
