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

#include "gui/about.h"
#include "gui/imageloader.h"

#include "util/global.h"
#include "util/checkversion.h"

#include "scene.h"
#include "scenebasic.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "sceneview_particle.h"
#include "logview.h"
#include "infowidget.h"
#include "settings.h"
#include "preprocessorview.h"
#include "postprocessorview.h"
#include "chartdialog.h"
#include "confdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "reportdialog.h"
#include "videodialog.h"
#include "problemdialog.h"
#include "collaboration.h"
#include "resultsview.h"
#include "materialbrowserdialog.h"
#include "hermes2d/module.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "scenetransformdialog.h"
#include "chartdialog.h"
#include "examplesdialog.h"
#include "hermes2d/solver.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowIcon(icon("agros2d"));

    Agros2D::createSingleton();

    // log stdout
    logStdOut = NULL;
    if (Agros2D::configComputer()->showLogStdOut)
        logStdOut = new LogStdOut();

    // FIXME: curve elements from script doesn't work
    // Module::readMeshDirtyFix();

    createPythonEngine(new PythonEngineAgros());

    // scene
    postHermes = new PostHermes();

    sceneInfoWidget = new InfoWidget(sceneViewPreprocessor, this);
    sceneViewPreprocessor = new SceneViewPreprocessor(this);
    sceneViewMesh = new SceneViewMesh(postHermes, this);
    sceneViewPost2D = new SceneViewPost2D(postHermes, this);
    sceneViewPost3D = new SceneViewPost3D(postHermes, this);
    sceneViewChart = new ChartView(this);
    sceneViewParticleTracing = new SceneViewParticleTracing(postHermes, this);
    sceneViewBlank = new QLabel("", this);

    // preprocessor
    preprocessorWidget = new PreprocessorWidget(sceneViewPreprocessor, this);
    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), preprocessorWidget, SLOT(refresh()));
    // postprocessor
    postprocessorWidget = new PostprocessorWidget(postHermes,
                                                  sceneViewPreprocessor,
                                                  sceneViewMesh,
                                                  sceneViewPost2D,
                                                  sceneViewPost3D,
                                                  this);
    // particle tracing
    particleTracingWidget = new ParticleTracingWidget(sceneViewParticleTracing, this);

    // chart
    chartWidget = new ChartWidget(sceneViewChart, this);

    // settings
    settingsWidget = new SettingsWidget(this);
    // problem
    problemWidget = new ProblemWidget(this);

    scriptEditorDialog = new PythonLabAgros(currentPythonEngine(), QApplication::arguments(), this);
    // collaborationDownloadDialog = new ServerDownloadDialog(this);
    sceneTransformDialog = new SceneTransformDialog(sceneViewPreprocessor, this);

    createActions();
    createViews();
    createMenus();
    createToolBars();
    createMain();

    // post hermes
    connect(problemWidget, SIGNAL(changed()), postHermes, SLOT(refresh()));
    connect(settingsWidget, SIGNAL(apply()), postHermes, SLOT(refresh()));
    connect(postprocessorWidget, SIGNAL(apply()), postHermes, SLOT(refresh()));
    currentPythonEngineAgros()->setPostHermes(postHermes);

    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(doSolveFinished()));
    //connect(Agros2D::problem(), SIGNAL(calculationStoped()), this, SLOT(doSolveFinished()));

    connect(tabViewLayout, SIGNAL(currentChanged(int)), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(fileNameChanged(QString)), this, SLOT(doSetWindowTitle(QString)));
    connect(Agros2D::scene()->actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));
    connect(postprocessorWidget, SIGNAL(apply()), this, SLOT(setControls()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(setControls()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), sceneViewPreprocessor, SLOT(refresh()));

    // preprocessor
    connect(problemWidget, SIGNAL(changed()), sceneViewPreprocessor, SLOT(refresh()));
    connect(settingsWidget, SIGNAL(apply()), sceneViewPreprocessor, SLOT(refresh()));
    connect(sceneViewPreprocessor, SIGNAL(sceneGeometryModeChanged(SceneGeometryMode)), preprocessorWidget, SLOT(loadTooltip(SceneGeometryMode)));
    connect(currentPythonEngine(), SIGNAL(executedScript()), Agros2D::scene(), SLOT(doInvalidated()));
    connect(currentPythonEngine(), SIGNAL(executedScript()), Agros2D::scene()->loopsInfo(), SLOT(processPolygonTriangles()));
    currentPythonEngineAgros()->setSceneViewGeometry(sceneViewPreprocessor);

    // particle tracing
    currentPythonEngineAgros()->setSceneViewParticleTracing(sceneViewParticleTracing);

    // mesh
    connect(Agros2D::scene(), SIGNAL(cleared()), sceneViewMesh, SLOT(clear()));
    currentPythonEngineAgros()->setSceneViewMesh(sceneViewMesh);

    // postprocessor 2d
    connect(sceneViewPost2D, SIGNAL(mousePressed()), resultsView, SLOT(doShowResults()));
    connect(sceneViewPost2D, SIGNAL(mousePressed(const Point &)), resultsView, SLOT(showPoint(const Point &)));
    connect(sceneViewPost2D, SIGNAL(postprocessorModeGroupChanged(SceneModePostprocessor)), resultsView, SLOT(doPostprocessorModeGroupChanged(SceneModePostprocessor)));
    // connect(sceneViewPost2D, SIGNAL(postprocessorModeGroupChanged(SceneModePostprocessor)), this, SLOT(doPostprocessorModeGroupChanged(SceneModePostprocessor)));
    currentPythonEngineAgros()->setSceneViewPost2D(sceneViewPost2D);

    // postprocessor 3d
    currentPythonEngineAgros()->setSceneViewPost3D(sceneViewPost3D);

    // info
    connect(problemWidget, SIGNAL(changed()), sceneInfoWidget, SLOT(refresh()));
    connect(postprocessorWidget, SIGNAL(apply()), sceneInfoWidget, SLOT(refresh()));

    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(doFieldsChanged()));

    sceneViewPreprocessor->clear();
    sceneViewMesh->clear();
    sceneViewPost2D->clear();
    sceneViewPost3D->clear();
    sceneViewParticleTracing->clear();

    QSettings settings;
    recentFiles = settings.value("MainWindow/RecentFiles").value<QStringList>();

    Agros2D::scene()->clear();

    problemWidget->actProperties->trigger();
    sceneViewPreprocessor->doZoomBestFit();

    // set recent files
    setRecentFiles();

    // accept drops
    setAcceptDrops(true);

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    if (settings.value("General/CheckVersion", true).value<bool>())
        checkForNewVersion(true);

    restoreGeometry(settings.value("MainWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("MainWindow/State", saveState()).toByteArray());
    splitter->restoreState(settings.value("MainWindow/SplitterState").toByteArray());
    // show/hide control panel
    actHideControlPanel->setChecked(settings.value("MainWindow/ControlPanel", true).toBool());
    doHideControlPanel();

    setControls();

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
                currentPythonEngineAgros()->runScript(readFileContent(scriptName));
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
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/RecentFiles", recentFiles);
    settings.setValue("MainWindow/SplitterState", splitter->saveState());
    settings.setValue("MainWindow/ControlPanel", actHideControlPanel->isChecked());

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());

    delete logStdOut;
}

void MainWindow::open(const QString &fileName)
{
    doDocumentOpen(fileName);
}

void MainWindow::createActions()
{
    actDocumentNew = new QAction(icon("document-new"), tr("&New..."), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(doDocumentNew()));

    actDocumentOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(doDocumentOpen()));

    // actDocumentDownloadFromServer = new QAction(icon(""), tr("&Download from server..."), this);
    // actDocumentDownloadFromServer->setShortcut(tr("Ctrl+Shift+O"));
    // connect(actDocumentDownloadFromServer, SIGNAL(triggered()), this, SLOT(doDocumentDownloadFromServer()));

    actDocumentSave = new QAction(icon("document-save"), tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));

    actDocumentSaveSolution = new QAction(icon(""), tr("Save solution"), this);
    connect(actDocumentSaveSolution, SIGNAL(triggered()), this, SLOT(doDocumentSaveSolution()));

    actDocumentSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));

    // actDocumentUploadToServer = new QAction(icon(""), tr("Upload to server..."), this);
    // connect(actDocumentUploadToServer, SIGNAL(triggered()), this, SLOT(doDocumentUploadToServer()));

    actDocumentClose = new QAction(tr("&Close"), this);
    actDocumentClose->setShortcuts(QKeySequence::Close);
    connect(actDocumentClose, SIGNAL(triggered()), this, SLOT(doDocumentClose()));

    actDocumentImportDXF = new QAction(tr("Import DXF..."), this);
    connect(actDocumentImportDXF, SIGNAL(triggered()), this, SLOT(doDocumentImportDXF()));

    actDocumentExportDXF = new QAction(tr("Export DXF..."), this);
    connect(actDocumentExportDXF, SIGNAL(triggered()), this, SLOT(doDocumentExportDXF()));

    actDocumentExportMeshFile = new QAction(tr("Export mesh file..."), this);
    connect(actDocumentExportMeshFile, SIGNAL(triggered()), this, SLOT(doDocumentExportMeshFile()));

    actDocumentSaveImage = new QAction(tr("Export image..."), this);
    connect(actDocumentSaveImage, SIGNAL(triggered()), this, SLOT(doDocumentSaveImage()));

    actDocumentSaveGeometry = new QAction(tr("Export geometry..."), this);
    connect(actDocumentSaveGeometry, SIGNAL(triggered()), this, SLOT(doDocumentSaveGeometry()));

    actExamples = new QAction(tr("Open example..."), this);
    connect(actExamples, SIGNAL(triggered()), this, SLOT(doExamples()));

    actCreateVideo = new QAction(icon("video"), tr("Create &video..."), this);
    connect(actCreateVideo, SIGNAL(triggered()), this, SLOT(doCreateVideo()));

    actLoadBackground = new QAction(tr("Load background..."), this);
    connect(actLoadBackground, SIGNAL(triggered()), this, SLOT(doLoadBackground()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    // undo framework
    actUndo = Agros2D::scene()->undoStack()->createUndoAction(this);
    actUndo->setIcon(icon("edit-undo"));
    actUndo->setIconText(tr("&Undo"));
    actUndo->setShortcuts(QKeySequence::Undo);

    actRedo = Agros2D::scene()->undoStack()->createRedoAction(this);
    actRedo->setIcon(icon("edit-redo"));
    actRedo->setIconText(tr("&Redo"));
    actRedo->setShortcuts(QKeySequence::Redo);

    actCopy = new QAction(icon("edit-copy"), tr("Copy image to clipboard"), this);
    // actCopy->setShortcuts(QKeySequence::Copy);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopy()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    // actHelp->setEnabled(false);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpShortCut = new QAction(icon(""), tr("&Shortcuts"), this);
    actHelpShortCut->setEnabled(false);
    connect(actHelpShortCut, SIGNAL(triggered()), this, SLOT(doHelpShortCut()));

    // actCollaborationServer = new QAction(icon("collaboration"), tr("Collaboration server"), this);
    // connect(actCollaborationServer, SIGNAL(triggered()), this, SLOT(doCollaborationServer()));

    actOnlineHelp = new QAction(icon(""), tr("&Online help"), this);
    actOnlineHelp->setEnabled(false);
    connect(actOnlineHelp, SIGNAL(triggered()), this, SLOT(doOnlineHelp()));

    actCheckVersion = new QAction(icon(""), tr("Check version"), this);
    connect(actCheckVersion, SIGNAL(triggered()), this, SLOT(doCheckVersion()));

    actAbout = new QAction(icon("about"), tr("About &Agros2D"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actOptions = new QAction(icon("options"), tr("&Options"), this);
    actOptions->setMenuRole(QAction::PreferencesRole);
    connect(actOptions, SIGNAL(triggered()), this, SLOT(doOptions()));

    actCreateMesh = new QAction(icon("scene-meshgen"), tr("&Mesh area"), this);
    actCreateMesh->setShortcut(QKeySequence(tr("Alt+W")));
    connect(actCreateMesh, SIGNAL(triggered()), this, SLOT(doCreateMesh()));

    actSolve = new QAction(icon("run"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actSolveAdaptiveStep = new QAction(icon("run-step"), tr("Adaptive\nstep"), this);
    connect(actSolveAdaptiveStep, SIGNAL(triggered()), this, SLOT(doSolveAdaptiveStep()));

    actFullScreen = new QAction(icon("view-fullscreen"), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actScriptEditor = new QAction(icon("script-python"), tr("PythonLab"), this);
    actScriptEditor->setShortcut(Qt::Key_F9);
    connect(actScriptEditor, SIGNAL(triggered()), this, SLOT(doScriptEditor()));

    actScriptEditorRunScript = new QAction(icon("script"), tr("Run &script..."), this);
    connect(actScriptEditorRunScript, SIGNAL(triggered()), this, SLOT(doScriptEditorRunScript()));

    // actReport = new QAction(icon("report"), tr("&Report..."), this);
    // connect(actReport, SIGNAL(triggered()), this, SLOT(doReport()));

    actMaterialBrowser = new QAction(icon(""), tr("Material browser..."), this);
    actMaterialBrowser->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actMaterialBrowser, SIGNAL(triggered()), this, SLOT(doMaterialBrowser()));

    // zoom actions (geometry, post2d and post3d)
    // scene - zoom
    actSceneZoomIn = new QAction(icon("zoom-in"), tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);

    actSceneZoomOut = new QAction(icon("zoom-out"), tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);

    actSceneZoomBestFit = new QAction(icon("zoom-original"), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));

    actSceneZoomRegion = new QAction(icon("zoom-fit-best"), tr("Zoom region"), this);
    actSceneZoomRegion->setCheckable(true);

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(problemWidget->actProperties);
    actSceneModeGroup->addAction(sceneViewPreprocessor->actSceneModePreprocessor);
    actSceneModeGroup->addAction(sceneViewMesh->actSceneModeMesh);
    actSceneModeGroup->addAction(sceneViewPost2D->actSceneModePost2D);
    actSceneModeGroup->addAction(sceneViewPost3D->actSceneModePost3D);
    actSceneModeGroup->addAction(sceneViewChart->actSceneModeChart);
    actSceneModeGroup->addAction(sceneViewParticleTracing->actSceneModeParticleTracing);
    actSceneModeGroup->addAction(settingsWidget->actSettings);

    actHideControlPanel = new QAction(icon("showhide"), tr("Show/hide control panel"), this);
    actHideControlPanel->setShortcut(tr("Alt+0"));
    actHideControlPanel->setCheckable(true);
    connect(actHideControlPanel, SIGNAL(triggered()), this, SLOT(doHideControlPanel()));
}


void MainWindow::doFieldsChanged()
{
    // TODO: it is not necessary to create whole menu, it only needs to be adjusted
    // TODO: what hapens to old menu object? Memory leaks???
    createMenus();
}

void MainWindow::createMenus()
{
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
    mnuFileImportExport->addAction(sceneViewMesh->actExportVTKMesh);
    mnuFileImportExport->addAction(sceneViewMesh->actExportVTKOrder);
    mnuFileImportExport->addAction(sceneViewPost2D->actExportVTKScalar);

    // QMenu *mnuServer = new QMenu(tr("Colaboration"), this);
    // mnuServer->addAction(actDocumentDownloadFromServer);
    // mnuServer->addAction(actDocumentUploadToServer);
    // mnuServer->addAction(actCollaborationServer);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actExamples);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentSave);
    mnuFile->addAction(actDocumentSaveSolution);
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuFileImportExport);
    // mnuFile->addMenu(mnuServer);
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
    mnuEdit->addAction(actCopy);
    mnuEdit->addSeparator();
    mnuEdit->addAction(Agros2D::scene()->actDeleteSelected);
    mnuEdit->addSeparator();
    mnuEdit->addAction(sceneViewPreprocessor->actSceneViewSelectRegion);
    mnuEdit->addAction(Agros2D::scene()->actTransform);
#ifdef Q_WS_X11
    mnuEdit->addSeparator();
    mnuEdit->addAction(actOptions);
#endif

    QMenu *mnuShowPanels = new QMenu(tr("Panels"), this);
    mnuShowPanels->addAction(resultsView->toggleViewAction());
    mnuShowPanels->addAction(consoleView->toggleViewAction());
    mnuShowPanels->addAction(logView->toggleViewAction());

    mnuView = menuBar()->addMenu(tr("&View"));
    mnuView->addAction(problemWidget->actProperties);
    mnuView->addAction(sceneViewPreprocessor->actSceneModePreprocessor);
    mnuView->addAction(sceneViewMesh->actSceneModeMesh);
    mnuView->addAction(sceneViewPost2D->actSceneModePost2D);
    mnuView->addAction(sceneViewPost3D->actSceneModePost3D);
    mnuView->addAction(sceneViewChart->actSceneModeChart);
    mnuView->addAction(sceneViewParticleTracing->actSceneModeParticleTracing);
    mnuView->addAction(settingsWidget->actSettings);
    mnuView->addSeparator();
    mnuView->addAction(actHideControlPanel);
    mnuView->addSeparator();
    mnuView->addAction(actSceneZoomBestFit);
    mnuView->addAction(actSceneZoomIn);
    mnuView->addAction(actSceneZoomOut);
    mnuView->addAction(actSceneZoomRegion);
    mnuView->addSeparator();
    mnuView->addAction(actLoadBackground);
    mnuView->addSeparator();
    mnuView->addMenu(mnuShowPanels);
    mnuView->addSeparator();
    mnuView->addAction(actFullScreen);

    mnuProblem = menuBar()->addMenu(tr("&Problem"));
    QMenu *mnuAdd = new QMenu(tr("&Add"), this);
    mnuProblem->addMenu(mnuAdd);
    mnuAdd->addAction(Agros2D::scene()->actNewNode);
    mnuAdd->addAction(Agros2D::scene()->actNewEdge);
    mnuAdd->addAction(Agros2D::scene()->actNewLabel);
    mnuAdd->addSeparator();
    Agros2D::scene()->addBoundaryAndMaterialMenuItems(mnuAdd, this);
    mnuProblem->addSeparator();
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeNothing);
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeLocalPointValue);
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeSurfaceIntegral);
    mnuProblem->addAction(sceneViewPost2D->actPostprocessorModeVolumeIntegral);
    mnuProblem->addAction(sceneViewPost2D->actSelectPoint);
    mnuProblem->addAction(sceneViewPost2D->actSelectByMarker);
    mnuProblem->addSeparator();
    mnuProblem->addAction(actCreateMesh);
    mnuProblem->addAction(actSolve);
    mnuProblem->addAction(actSolveAdaptiveStep);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actScriptEditor);
    mnuTools->addAction(actScriptEditorRunScript);
    mnuTools->addSeparator();
    mnuTools->addAction(actMaterialBrowser);
    mnuTools->addSeparator();
    // mnuTools->addAction(actReport);
    mnuTools->addAction(actCreateVideo);
#ifdef Q_WS_WIN
    mnuTools->addSeparator();
    mnuTools->addAction(actOptions);
#endif

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);
    // mnuHelp->addAction(actOnlineHelp);
    // mnuHelp->addAction(actHelpShortCut);
    // mnuHelp->addAction(actCollaborationServer);
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
    // top toolbar
#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->setOrientation(Qt::Horizontal);
    tlbFile->setAllowedAreas(Qt::TopToolBarArea);
    tlbFile->setMovable(false);
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbFile->addAction(actDocumentNew);
    tlbFile->addAction(actDocumentOpen);
    tlbFile->addAction(actDocumentSave);
    tlbFile->addSeparator();
    tlbFile->addAction(actHideControlPanel);

    tlbView = addToolBar(tr("View"));
    tlbView->setObjectName("View");
    tlbView->setOrientation(Qt::Horizontal);
    tlbView->setAllowedAreas(Qt::TopToolBarArea);
    tlbView->setMovable(false);
#ifdef Q_WS_MAC
    tlbView->setFixedHeight(iconHeight);
    tlbView->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif

    tlbZoom = addToolBar(tr("Zoom"));
    tlbZoom->setObjectName("Zoom");
    tlbZoom->addAction(actSceneZoomBestFit);
    tlbZoom->addAction(actSceneZoomRegion);
    tlbZoom->addAction(actSceneZoomIn);
    tlbZoom->addAction(actSceneZoomOut);
    tlbZoom->setMovable(false);

    tlbGeometry = addToolBar(tr("Geometry"));
    tlbGeometry->setObjectName("Geometry");
    tlbGeometry->setOrientation(Qt::Horizontal);
    tlbGeometry->setAllowedAreas(Qt::TopToolBarArea);
    // tlbGeometry->setMovable(false);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(actUndo);
    tlbGeometry->addAction(actRedo);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(sceneViewPreprocessor->actOperateOnNodes);
    tlbGeometry->addAction(sceneViewPreprocessor->actOperateOnEdges);
    tlbGeometry->addAction(sceneViewPreprocessor->actOperateOnLabels);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(sceneViewPreprocessor->actSceneViewSelectRegion);
    tlbGeometry->addAction(Agros2D::scene()->actTransform);
    tlbGeometry->addSeparator();
    tlbGeometry->addAction(Agros2D::scene()->actDeleteSelected);
    tlbGeometry->setMovable(false);

    tlbPost2D = addToolBar(tr("Postprocessor 2D"));
    tlbPost2D->setObjectName("Postprocessor 2D");
    tlbPost2D->setOrientation(Qt::Horizontal);
    tlbPost2D->setAllowedAreas(Qt::TopToolBarArea);
    // tlbPost2D->setMovable(false);
#ifdef Q_WS_MAC
    tlbPost2D->setFixedHeight(iconHeight);
    tlbPost2D->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbPost2D->addSeparator();
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeNothing);
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeLocalPointValue);
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeSurfaceIntegral);
    tlbPost2D->addAction(sceneViewPost2D->actPostprocessorModeVolumeIntegral);
    tlbPost2D->addSeparator();
    tlbPost2D->addAction(sceneViewPost2D->actSelectPoint);
    tlbPost2D->addAction(sceneViewPost2D->actSelectByMarker);
    tlbPost2D->setMovable(false);
}

void MainWindow::createMain()
{
    sceneViewInfoWidget = new SceneViewWidget(sceneInfoWidget, this);
    sceneViewBlankWidget = new SceneViewWidget(sceneViewBlank, this);
    sceneViewPreprocessorWidget = new SceneViewWidget(sceneViewPreprocessor, this);
    sceneViewMeshWidget = new SceneViewWidget(sceneViewMesh, this);
    sceneViewPost2DWidget = new SceneViewWidget(sceneViewPost2D, this);
    sceneViewPost3DWidget = new SceneViewWidget(sceneViewPost3D, this);
    sceneViewPostParticleTracingWidget = new SceneViewWidget(sceneViewParticleTracing, this);
    sceneViewChartWidget = new SceneViewWidget(sceneViewChart, this);

    tabViewLayout = new QStackedLayout();
    tabViewLayout->setContentsMargins(0, 0, 0, 0);
    tabViewLayout->addWidget(sceneViewInfoWidget);
    tabViewLayout->addWidget(sceneViewBlankWidget);
    tabViewLayout->addWidget(sceneViewPreprocessorWidget);
    tabViewLayout->addWidget(sceneViewMeshWidget);
    tabViewLayout->addWidget(sceneViewPost2DWidget);
    tabViewLayout->addWidget(sceneViewPost3DWidget);
    tabViewLayout->addWidget(sceneViewPostParticleTracingWidget);
    tabViewLayout->addWidget(sceneViewChartWidget);

    QWidget *viewWidget = new QWidget();
    viewWidget->setLayout(tabViewLayout);

    tabControlsLayout = new QStackedLayout();
    tabControlsLayout->setContentsMargins(0, 0, 0, 0);
    tabControlsLayout->addWidget(problemWidget);
    tabControlsLayout->addWidget(preprocessorWidget);
    tabControlsLayout->addWidget(postprocessorWidget);
    tabControlsLayout->addWidget(particleTracingWidget);
    tabControlsLayout->addWidget(chartWidget);
    tabControlsLayout->addWidget(settingsWidget);

    viewControls = new QWidget();
    viewControls->setLayout(tabControlsLayout);

    // spacing
    QLabel *spacing = new QLabel;
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // left toolbar
    QToolBar *tlbLeftBar = new QToolBar();
    tlbLeftBar->setOrientation(Qt::Vertical);
    // fancy layout
#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif
    tlbLeftBar->setStyleSheet(QString("QToolBar { border: 1px solid rgba(200, 200, 200, 255); }"
                                      "QToolBar { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:1 rgba(120, 120, 120, 255)); }"
                                      "QToolButton { border: 0px; color: rgba(230, 230, 230, 255); font: bold; font-size: %1pt; width: 65px; }"
                                      "QToolButton:hover { border: 0px; background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:0.5 rgba(160, 160, 160, 255), stop:1 rgba(150, 150, 150, 255)); }"
                                      "QToolButton:checked:hover, QToolButton:checked { border: 0px; color: rgba(30, 30, 30, 255); background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(160, 160, 160, 255), stop:0.5 rgba(220, 220, 220, 255), stop:1 rgba(160, 160, 160, 255)); }").arg(fontSize));
    // system layout
    // leftToolBar->setStyleSheet("QToolButton { font: bold; font-size: 8pt; width: 65px; }");

    tlbLeftBar->setIconSize(QSize(32, 32));
    tlbLeftBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    tlbLeftBar->addAction(problemWidget->actProperties);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(sceneViewPreprocessor->actSceneModePreprocessor);
    tlbLeftBar->addAction(sceneViewMesh->actSceneModeMesh);
    tlbLeftBar->addAction(sceneViewPost2D->actSceneModePost2D);
    tlbLeftBar->addAction(sceneViewPost3D->actSceneModePost3D);
    tlbLeftBar->addAction(sceneViewParticleTracing->actSceneModeParticleTracing);
    tlbLeftBar->addAction(sceneViewChart->actSceneModeChart);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(settingsWidget->actSettings);
    tlbLeftBar->addWidget(spacing);
    tlbLeftBar->addAction(actCreateMesh);
    tlbLeftBar->addAction(actSolve);
    tlbLeftBar->addAction(actSolveAdaptiveStep);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(actScriptEditor);

    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(viewControls);
    splitter->addWidget(viewWidget);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    QList<int> sizes;
    sizes << 230 << 0;
    splitter->setSizes(sizes);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(tlbLeftBar);
    layoutMain->addWidget(splitter);

    QWidget *main = new QWidget();
    main->setLayout(layoutMain);

    setCentralWidget(main);
}

void MainWindow::createViews()
{
    consoleView = new PythonScriptingConsoleView(currentPythonEngine(), this);
    consoleView->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    consoleView->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, consoleView);

    logView = new LogView(this);
    logView->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, logView);

    resultsView = new ResultsView(postHermes, this);
    resultsView->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, resultsView);

    // tabify dock together
    tabifyDockWidget(resultsView, consoleView);
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
    // recent files
    if (!Agros2D::problem()->config()->fileName().isEmpty())
    {
        QFileInfo fileInfo(Agros2D::problem()->config()->fileName());
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
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
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
    FieldSelectDialog dialog(QList<QString>(), this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        Agros2D::scene()->clear();
        Agros2D::problem()->clearFieldsAndConfig();

        // add field
        try
        {
            FieldInfo *fieldInfo = new FieldInfo(dialog.selectedFieldId());

            Agros2D::problem()->addField(fieldInfo);

            problemWidget->actProperties->trigger();
            sceneViewPreprocessor->doZoomBestFit();
            sceneViewMesh->doZoomBestFit();
            sceneViewPost2D->doZoomBestFit();
            sceneViewPost3D->doZoomBestFit();
            sceneViewParticleTracing->doZoomBestFit();
            settingsWidget->updateControls();
        }
        catch (AgrosPluginException& e)
        {
            Agros2D::scene()->clear();

            Agros2D::log()->printError(tr("Problem"), e.toString());
        }
    }
}

void MainWindow::doDocumentOpen(const QString &fileName)
{
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
            try
            {
                // load problem
                Agros2D::scene()->readFromFile(fileNameDocument);
                setRecentFiles();

                // load solution
                Agros2D::scene()->readSolutionFromFile(fileNameDocument);

                problemWidget->actProperties->trigger();
                sceneViewPreprocessor->doZoomBestFit();
                sceneViewMesh->doZoomBestFit();
                sceneViewPost2D->doZoomBestFit();
                sceneViewPost3D->doZoomBestFit();
                sceneViewParticleTracing->doZoomBestFit();

                return;
            }
            catch (AgrosException &e)
            {
                Agros2D::scene()->clear();

                Agros2D::log()->printError(tr("Problem"), e.toString());
                return;
            }
            catch(AgrosModuleException& e)
            {
                Agros2D::scene()->clear();

                Agros2D::log()->printError(tr("Problem"), e.toString());
                return;
            }
            catch(AgrosPluginException& e)
            {
                Agros2D::scene()->clear();

                Agros2D::log()->printError(tr("Problem"), e.toString());
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

        Agros2D::log()->printError(tr("Problem"), tr("Unknown suffix."));
    }
    else
    {
        Agros2D::log()->printError(tr("Problem"), tr("File '%1' is not found.").arg(fileNameDocument));
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
    QString fileName = action->text();
    doDocumentOpen(fileName);
}

void MainWindow::doDocumentSave()
{
    if (QFile::exists(Agros2D::problem()->config()->fileName()))
    {
        try
        {
            Agros2D::scene()->writeToFile(Agros2D::problem()->config()->fileName(), true);
        }
        catch (AgrosException &e)
        {
            Agros2D::log()->printError(tr("Problem"), e.toString());
        }
    }
    else
    {
        doDocumentSaveAs();
    }
}

void MainWindow::doDocumentSaveSolution()
{
    if (QFile::exists(Agros2D::problem()->config()->fileName()))
    {
        Agros2D::scene()->writeSolutionToFile(Agros2D::problem()->config()->fileName());
    }
}

void MainWindow::doDocumentSaveAs()
{
    QSettings settings;
    QString dir = settings.value("General/LastProblemDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Agros2D files (*.a2d)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "a2d") fileName += ".a2d";

        try
        {
            Agros2D::scene()->writeToFile(fileName, true);
            setRecentFiles();
        }
        catch (AgrosException &e)
        {
            Agros2D::log()->printError(tr("Problem"), e.toString());
        }
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
    // WILL BE FIXED
    /*
    while (!Agros2D::scene()->undoStack()->isClean())
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

    Agros2D::scene()->clear();
    Agros2D::problem()->clearFieldsAndConfig();

    problemWidget->actProperties->trigger();
    sceneViewPreprocessor->actOperateOnNodes->trigger();
    sceneViewPreprocessor->doZoomBestFit();
    sceneViewMesh->doZoomBestFit();
    sceneViewPost2D->doZoomBestFit();
    sceneViewPost3D->doZoomBestFit();
    sceneViewParticleTracing->doZoomBestFit();
}

void MainWindow::doDocumentImportDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        Agros2D::scene()->readFromDxf(fileName);
        sceneViewPreprocessor->doZoomBestFit();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentExportDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "dxf") fileName += ".dxf";
        Agros2D::scene()->writeToDxf(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentSaveImage()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export image to file"), dir, tr("PNG files (*.png)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "png") fileName += ".png";

        if (sceneViewPreprocessor->actSceneModePreprocessor->isChecked())
            sceneViewPreprocessor->saveImageToFile(fileName);
        else if (sceneViewMesh->actSceneModeMesh->isChecked())
            sceneViewMesh->saveImageToFile(fileName);
        else if (sceneViewPost2D->actSceneModePost2D->isChecked())
            sceneViewPost2D->saveImageToFile(fileName);
        else if (sceneViewPost3D->actSceneModePost3D->isChecked())
            sceneViewPost3D->saveImageToFile(fileName);
        else if (sceneViewParticleTracing->actSceneModeParticleTracing->isChecked())
            sceneViewParticleTracing->saveImageToFile(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentSaveGeometry()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();


    QString selected;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export geometry to file"), dir,
                                                    tr("SVG files (*.svg)"),
                                                    &selected);

    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (selected == "SVG files (*.svg)")
        {
            if (fileInfo.suffix().toLower() != "svg") fileName += ".svg";
        }

        sceneViewPreprocessor->saveGeometryToSvg(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doExamples()
{
    ExamplesDialog *examples = new ExamplesDialog(this);
    if (examples->showDialog() == QDialog::Accepted)
    {
        if (QFile::exists(examples->selectedFilename()))
        {
            doDocumentOpen(examples->selectedFilename());
        }
    }

    delete examples;
}

void MainWindow::doCreateVideo()
{
    VideoDialog *videoDialog = NULL;
    if (sceneViewMesh->actSceneModeMesh->isChecked())
        videoDialog = new VideoDialog(sceneViewMesh, postHermes, this);
    else if (sceneViewPost2D->actSceneModePost2D->isChecked())
        videoDialog = new VideoDialog(sceneViewPost2D, postHermes, this);
    else if (sceneViewPost3D->actSceneModePost3D->isChecked())
        videoDialog = new VideoDialog(sceneViewPost3D, postHermes, this);

    if (videoDialog)
    {
        videoDialog->showDialog();
        delete videoDialog;
    }
}

void MainWindow::doCreateMesh()
{
    LogDialog *logDialog = new LogDialog(this, tr("Mesh"));
    logDialog->show();

    // create mesh
    Agros2D::problem()->mesh();
    if (Agros2D::problem()->isMeshed())
    {
        // raise mesh viewer
        sceneViewMesh->actSceneModeMesh->trigger();

        // successful run
        logDialog->close();
    }
}

void MainWindow::doSolve()
{
    LogDialog *logDialog = new LogDialog(this, tr("Solver"));
    logDialog->show();

    // solve problem
    Agros2D::problem()->solve();
}

void MainWindow::doSolveAdaptiveStep()
{
    LogDialog *logDialog = new LogDialog(this, tr("Adaptive step"));
    logDialog->show();

    // solve problem
    Agros2D::problem()->solveAdaptiveStep();
}

void MainWindow::doSolveFinished()
{
    if (Agros2D::problem()->isSolved())
    {
        sceneViewPost2D->actSceneModePost2D->trigger();

        // show local point values
        resultsView->showEmpty();

        // raise postprocessor
        postprocessorWidget->raise();
    }

    if (!currentPythonEngineAgros()->isRunning())
    {
        setFocus();
        activateWindow();
    }
}

void MainWindow::doFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::doOptions()
{
    ConfigComputerDialog configDialog(this);
    if (configDialog.exec())
    {
        // postHermes->refresh();
        // setControls();
    }

    activateWindow();
}

void MainWindow::doReport()
{
    ReportDialog reportDialog(sceneViewPost2D, this);
    reportDialog.showDialog();
}

void MainWindow::doTransform()
{
    sceneTransformDialog->showDialog();
}

void MainWindow::doMaterialBrowser()
{
    MaterialBrowserDialog materialBrowserDialog(this);
    materialBrowserDialog.showDialog(false);
}

void MainWindow::doScriptEditor()
{
    scriptEditorDialog->showDialog();
}

void MainWindow::doScriptEditorRunScript(const QString &fileName)
{
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
        consoleView->console()->consoleMessage(tr("Run script: %1\n").
                                               arg(QFileInfo(fileNameScript).fileName().left(QFileInfo(fileNameScript).fileName().length() - 3)),
                                               Qt::gray);

        consoleView->console()->connectStdOut();
        ScriptResult result = currentPythonEngineAgros()->runScript(readFileContent(fileNameScript), fileNameScript);
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

void MainWindow::doCut()
{

}

void MainWindow::doCopy()
{
    // copy image to clipboard
    QPixmap pixmap;
    if (sceneViewPreprocessor->actSceneModePreprocessor->isChecked())
        pixmap = sceneViewPreprocessor->renderScenePixmap();
    else if (sceneViewMesh->actSceneModeMesh->isChecked())
        pixmap = sceneViewMesh->renderScenePixmap();
    else if (sceneViewPost2D->actSceneModePost2D->isChecked())
        pixmap = sceneViewPost2D->renderScenePixmap();
    else if (sceneViewPost3D->actSceneModePost3D->isChecked())
        pixmap = sceneViewPost3D->renderScenePixmap();
    else if (sceneViewParticleTracing->actSceneModeParticleTracing->isChecked())
        pixmap = sceneViewParticleTracing->renderScenePixmap();

    QApplication::clipboard()->setImage(pixmap.toImage());
}

void MainWindow::doPaste()
{

}

void MainWindow::clear()
{
    sceneViewPreprocessor->actSceneModePreprocessor->trigger();

    setControls();
}

void MainWindow::setControls()
{
    setUpdatesEnabled(false);

    actDocumentSaveSolution->setEnabled(Agros2D::problem()->isSolved());

    // set controls
    Agros2D::scene()->actTransform->setEnabled(false);

    sceneViewPreprocessor->actSceneZoomRegion = NULL;
    sceneViewMesh->actSceneZoomRegion = NULL;
    sceneViewPost2D->actSceneZoomRegion = NULL;
    sceneViewPost3D->actSceneZoomRegion = NULL;
    sceneViewParticleTracing->actSceneZoomRegion = NULL;

    tlbGeometry->setVisible(sceneViewPreprocessor->actSceneModePreprocessor->isChecked());
    tlbPost2D->setVisible(sceneViewPost2D->actSceneModePost2D->isChecked());
    bool showZoom = sceneViewPreprocessor->actSceneModePreprocessor->isChecked() ||
            sceneViewMesh->actSceneModeMesh->isChecked() ||
            sceneViewPost2D->actSceneModePost2D->isChecked() ||
            sceneViewPost3D->actSceneModePost3D->isChecked() ||
            sceneViewParticleTracing->actSceneModeParticleTracing->isChecked();

    tlbZoom->setVisible(showZoom);
    actSceneZoomIn->setVisible(showZoom);
    actSceneZoomOut->setVisible(showZoom);
    actSceneZoomBestFit->setVisible(showZoom);
    actSceneZoomRegion->setVisible(showZoom);
    actSceneZoomRegion->setEnabled(sceneViewPreprocessor->actSceneModePreprocessor->isChecked() ||
                                   sceneViewMesh->actSceneModeMesh->isChecked() ||
                                   sceneViewPost2D->actSceneModePost2D->isChecked());

    // disconnect signals
    actSceneZoomIn->disconnect();
    actSceneZoomOut->disconnect();
    actSceneZoomBestFit->disconnect();

    if (problemWidget->actProperties->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewInfoWidget);
        tabControlsLayout->setCurrentWidget(problemWidget);
    }
    if (sceneViewPreprocessor->actSceneModePreprocessor->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewPreprocessorWidget);
        tabControlsLayout->setCurrentWidget(preprocessorWidget);

        Agros2D::scene()->actTransform->setEnabled(true);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewPreprocessor, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewPreprocessor, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewPreprocessor, SLOT(doZoomBestFit()));
        sceneViewPreprocessor->actSceneZoomRegion = actSceneZoomRegion;
    }
    if (sceneViewMesh->actSceneModeMesh->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewMeshWidget);
        tabControlsLayout->setCurrentWidget(postprocessorWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewMesh, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewMesh, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewMesh, SLOT(doZoomBestFit()));
        sceneViewMesh->actSceneZoomRegion = actSceneZoomRegion;
    }
    if (sceneViewPost2D->actSceneModePost2D->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewPost2DWidget);
        tabControlsLayout->setCurrentWidget(postprocessorWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewPost2D, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewPost2D, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewPost2D, SLOT(doZoomBestFit()));
        sceneViewPost2D->actSceneZoomRegion = actSceneZoomRegion;

        // hide transform dialog
        sceneTransformDialog->hide();
    }
    if (sceneViewPost3D->actSceneModePost3D->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewPost3DWidget);
        tabControlsLayout->setCurrentWidget(postprocessorWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewPost3D, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewPost3D, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewPost3D, SLOT(doZoomBestFit()));

        // hide transform dialog
        sceneTransformDialog->hide();
    }
    if (sceneViewParticleTracing->actSceneModeParticleTracing->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewPostParticleTracingWidget);
        tabControlsLayout->setCurrentWidget(particleTracingWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewParticleTracing, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewParticleTracing, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewParticleTracing, SLOT(doZoomBestFit()));

        // hide transform dialog
        sceneTransformDialog->hide();
    }
    if (sceneViewChart->actSceneModeChart->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewChartWidget);
        tabControlsLayout->setCurrentWidget(chartWidget);
    }

    if (settingsWidget->actSettings->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewBlankWidget);
        tabControlsLayout->setCurrentWidget(settingsWidget);
    }

    actDocumentExportMeshFile->setEnabled(Agros2D::problem()->isMeshed());

    // actSolve->setEnabled(Agros2D::problem()->fieldInfos().count() > 0);
//    actSolveAdaptiveStep->setEnabled(false);
//    actSolveAdaptiveStep->setVisible(false);
    actSolveAdaptiveStep->setEnabled(Agros2D::problem()->fieldInfos().count() > 0 && Agros2D::problem()->fieldInfos().count() <= 1 && (!Agros2D::problem()->isTransient()) );

    setUpdatesEnabled(true);

    QTimer::singleShot(0, postprocessorWidget, SLOT(updateControls()));

    // set current timestep
    //    cmbTimeStep->setCurrentIndex(Agros2D::problem()->timeStep());

    //actProgressLog->setEnabled(Agros2D::problem()->configView()->enabledProgressLog);
    //actApplicationLog->setEnabled(Agros2D::problem()->configView()->enabledApplicationLog);
}

void MainWindow::doHelp()
{
    showPage("index.html");
}

void MainWindow::doHelpShortCut()
{
    showPage("getting_started/shortcut_keys.html");
}

void MainWindow::doCollaborationServer()
{
    QDesktopServices::openUrl(QUrl(Agros2D::configComputer()->collaborationServerURL + "problems.php"));
}

void MainWindow::doOnlineHelp()
{
    QDesktopServices::openUrl(QUrl("http://hpfem.org/agros2d/help"));
}

void MainWindow::doCheckVersion()
{
    checkForNewVersion();
}

void MainWindow::doAbout()
{
    AboutDialog about(this);
    about.exec();
}

void MainWindow::doHideControlPanel()
{
    viewControls->setVisible(actHideControlPanel->isChecked());
}

void MainWindow::doDocumentExportMeshFile()
{
    if (Agros2D::problem()->isMeshed())
    {
        QSettings settings;
        QString dir = settings.value("General/LastMeshDir").toString();

        QString fileName = QFileDialog::getSaveFileName(this, tr("Export mesh file"), dir, tr("Mesh files (*.msh)"));
        QFileInfo fileInfo(fileName);

        if (!fileName.isEmpty())
        {
            if (fileInfo.suffix() != "msh") fileName += ".msh";

            // remove existing file
            if (QFile::exists(fileName + ".msh"))
                QFile::remove(fileName + ".msh");

            // copy file
            QFile::copy(cacheProblemDir() + "/initial.msh", fileName);
            if (fileInfo.absoluteDir() != cacheProblemDir())
                settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
        }
    }
    else
    {
        Agros2D::log()->printMessage(tr("Problem"), tr("The problem is not meshed"));
    }
}

void MainWindow::doLoadBackground()
{
    ImageLoaderDialog imageLoaderDialog;
    if (imageLoaderDialog.exec() == QDialog::Accepted)
    {
        sceneViewPreprocessor->loadBackgroundImage(imageLoaderDialog.fileName(),
                                                   imageLoaderDialog.position().x(),
                                                   imageLoaderDialog.position().y(),
                                                   imageLoaderDialog.position().width(),
                                                   imageLoaderDialog.position().height());
        sceneViewPreprocessor->refresh();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // WILL BE FIXED
    /*
    if (!Agros2D::scene()->undoStack()->isClean())
        doDocumentClose();

    if (Agros2D::scene()->undoStack()->isClean() && !scriptEditorDialog->isScriptModified())
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
