#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("MainWindow/Geometry", saveGeometry()).toByteArray());
    recentFiles = settings.value("MainWindow/RecentFiles").value<QStringList>();

    m_scene = new Scene();

    createActions();
    createScene();
    createViews();
    createMenus();
    createToolBars();
    createStatusBar();

    connect(m_scene, SIGNAL(invalidated()), sceneView, SLOT(doInvalidated()));
    connect(m_scene, SIGNAL(invalidated()), sceneInfoView, SLOT(doInvalidated()));
    connect(m_scene, SIGNAL(invalidated()), this, SLOT(doInvalidated()));

    connect(m_scene, SIGNAL(solved()), sceneView, SLOT(doSolved()));

    connect(sceneView, SIGNAL(mousePressed(LocalPointValue *)), localPointValueView, SLOT(doShowPoint(LocalPointValue *)));
    connect(sceneView, SIGNAL(mousePressed(VolumeIntegralValue *)), volumeIntegralValueView, SLOT(doShowVolumeIntegral(VolumeIntegralValue *)));
    connect(sceneView, SIGNAL(mousePressed(SurfaceIntegralValue *)), surfaceIntegralValueView, SLOT(doShowSurfaceIntegral(SurfaceIntegralValue *)));

    m_scene->clear();
    sceneView->doDefaults();

    restoreState(settings.value("MainWindow/State", saveState()).toByteArray());

    chartDialog = new ChartDialog(m_scene, this);
    scriptEditorDialog = new ScriptEditorDialog(m_scene, sceneView, this);

    sceneView->actSceneModeNode->trigger();
    sceneView->doZoomBestFit();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/RecentFiles", recentFiles);
}

void MainWindow::createActions()
{
    actDocumentNew = new QAction(icon("document-new"), tr("&New"), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    actDocumentNew->setStatusTip(tr("Create a new file"));
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(doDocumentNew()));
    
    actDocumentOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    actDocumentOpen->setStatusTip(tr("Open an existing file"));
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(doDocumentOpen()));
    
    actDocumentSave = new QAction(icon("document-save"), tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    actDocumentSave->setStatusTip(tr("Save the document to disk"));
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));
    
    actDocumentSaveAs = new QAction(tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    actDocumentSaveAs->setStatusTip(tr("Save the document under a new name"));
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));
    
    actDocumentImportDXF = new QAction(tr("Import DXF..."), this);
    actDocumentImportDXF->setStatusTip(tr("Import DXF"));
    connect(actDocumentImportDXF, SIGNAL(triggered()), this, SLOT(doDocumentImportDXF()));

    actDocumentExportDXF = new QAction(tr("Export DXF..."), this);
    actDocumentExportDXF->setStatusTip(tr("Export DXF"));
    connect(actDocumentExportDXF, SIGNAL(triggered()), this, SLOT(doDocumentExportDXF()));

    actDocumentSaveImage = new QAction(tr("Export image..."), this);
    actDocumentSaveImage->setStatusTip(tr("Export image to file"));
    connect(actDocumentSaveImage, SIGNAL(triggered()), this, SLOT(doDocumentSaveImage()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit the application"));
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));
    
    actCut = new QAction(icon("edit-cut"), tr("Cu&t"), this);
    actCut->setShortcuts(QKeySequence::Cut);
    actCut->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    // connect(actCut, SIGNAL(triggered()), textEdit, SLOT(cut()));
    
    actCopy = new QAction(icon("edit-copy"), tr("&Copy"), this);
    actCopy->setShortcuts(QKeySequence::Copy);
    actCopy->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    // connect(actCopy, SIGNAL(triggered()), textEdit, SLOT(copy()));
    
    actPaste = new QAction(icon("edit-paste"), tr("&Paste"), this);
    actPaste->setShortcuts(QKeySequence::Paste);
    actPaste->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(actPaste, SIGNAL(triggered()), this, SLOT(doPaste()));
    
    actAbout = new QAction(icon("about"), tr("&About"), this);
    actAbout->setStatusTip(tr("Show the application's About box"));
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));
    
    actAboutQt = new QAction(tr("About &Qt"), this);
    actAboutQt->setStatusTip(tr("Show the Qt library's About box"));
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    
    actCut->setEnabled(false);
    actCopy->setEnabled(false);
    // connect(textEdit, SIGNAL(copyAvailable(bool)), actCut, SLOT(setEnabled(bool)));
    // connect(textEdit, SIGNAL(copyAvailable(bool)), actCopy, SLOT(setEnabled(bool)));

    actOptions = new QAction(icon("options"), tr("Options"), this);
    actOptions->setStatusTip(tr("Options"));
    connect(actOptions, SIGNAL(triggered()), this, SLOT(doOptions()));

    actCreateMesh = new QAction(icon("scene-mesh"), tr("Mesh area"), this);
    actCreateMesh->setStatusTip(tr("Mesh area"));
    connect(actCreateMesh, SIGNAL(triggered()), this, SLOT(doCreateMesh()));

    actSolve = new QAction(icon("system-run"), tr("Solve problem"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+s")));
    actSolve->setStatusTip(tr("Solve problem"));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actChart = new QAction(icon("chart"), tr("Plot chart"), this);
    actChart->setStatusTip(tr("Plot chart"));
    connect(actChart, SIGNAL(triggered()), this, SLOT(doChart()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actScriptEditor = new QAction(icon("script"), tr("Script editor"), this);
    actScriptEditor->setStatusTip(tr("Script editor"));
    connect(actScriptEditor, SIGNAL(triggered()), this, SLOT(doScriptEditor()));
}

void MainWindow::createMenus()
{
    mnuRecentFiles = new QMenu(tr("Recent projects"), this);
    setRecentFiles();

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addAction(actDocumentSave);
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentImportDXF);
    mnuFile->addAction(actDocumentExportDXF);
    mnuFile->addAction(actDocumentSaveImage);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);
    
    mnuEdit = menuBar()->addMenu(tr("&Edit"));
    mnuEdit->addAction(actCut);
    mnuEdit->addAction(actCopy);
    mnuEdit->addAction(actPaste);
    
    mnuScene = menuBar()->addMenu(tr("&Project"));
    mnuScene->addAction(sceneView->actSceneModeNode);
    mnuScene->addAction(sceneView->actSceneModeEdge);
    mnuScene->addAction(sceneView->actSceneModeLabel);
    mnuScene->addAction(sceneView->actSceneModePostprocessor);
    mnuScene->addSeparator();
    mnuScene->addAction(sceneView->actSceneZoomBestFit);
    mnuScene->addAction(sceneView->actSceneZoomIn);
    mnuScene->addAction(sceneView->actSceneZoomOut);
    mnuScene->addSeparator();
    mnuScene->addAction(m_scene->actNewNode);
    mnuScene->addAction(m_scene->actNewEdge);
    mnuScene->addAction(m_scene->actNewLabel);
    mnuScene->addSeparator();
    mnuScene->addAction(m_scene->actNewEdgeMarker);
    mnuScene->addAction(m_scene->actNewLabelMarker);
    mnuScene->addSeparator();
    mnuScene->addAction(m_scene->actProjectProperties);

    mnuTools = menuBar()->addMenu(tr("Tools"));
    mnuTools->addAction(actChart);
    mnuTools->addAction(actScriptEditor);
    mnuTools->addSeparator();
    mnuTools->addAction(actOptions);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actAbout);
    mnuHelp->addAction(actAboutQt);
}

void MainWindow::createToolBars()
{
    tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->addAction(actDocumentNew);
    tlbFile->addAction(actDocumentOpen);
    tlbFile->addAction(actDocumentSave);
    
    tlbEdit = addToolBar(tr("Edit"));
    tlbEdit->setObjectName("Edit");
    tlbEdit->addAction(actCut);
    tlbEdit->addAction(actCopy);
    tlbEdit->addAction(actPaste);
    
    tlbScene = addToolBar(tr("Project"));
    tlbScene->setObjectName("Project");
    tlbScene->addAction(sceneView->actSceneModeNode);
    tlbScene->addAction(sceneView->actSceneModeEdge);
    tlbScene->addAction(sceneView->actSceneModeLabel);
    tlbScene->addAction(sceneView->actSceneModePostprocessor);
    tlbScene->addSeparator();
    tlbScene->addAction(sceneView->actSceneViewSelectRegion);
    tlbScene->addAction(m_scene->actTransform);
    tlbScene->addSeparator();
    tlbScene->addAction(actCreateMesh);
    tlbScene->addAction(actSolve);
    tlbScene->addAction(sceneView->actSceneViewProperties);
    tlbScene->addAction(actChart);
    tlbScene->addAction(actScriptEditor);

    tlbZoom = addToolBar(tr("Zoom"));
    tlbZoom->setObjectName("Zoom");
    tlbZoom->addAction(sceneView->actSceneZoomBestFit);
    tlbZoom->addAction(sceneView->actSceneZoomRegion);
    tlbZoom->addAction(sceneView->actSceneZoomIn);
    tlbZoom->addAction(sceneView->actSceneZoomOut);

    tlbPostprocessor = addToolBar("Postprocessor");
    tlbPostprocessor->setObjectName("Postprocessor");
    tlbPostprocessor->addAction(sceneView->actPostprocessorModeLocalPointValue);
    tlbPostprocessor->addAction(sceneView->actPostprocessorModeSurfaceIntegral);
    tlbPostprocessor->addAction(sceneView->actPostprocessorModeVolumeIntegral);
}

void MainWindow::createStatusBar()
{
    lblMessage = new QLabel(statusBar());
    // lblMessage->setStyleSheet("QLabel {border-left: 1px solid gray;}");
    lblPosition = new QLabel(statusBar());
    // lblPosition->setStyleSheet("QLabel {border: 1px solid gray;}");
    lblPosition->setMinimumWidth(170);
    lblProblemType = new QLabel(statusBar());
    // lblProblemType->setStyleSheet("QLabel {border: 1px solid gray;}");
    lblPhysicField = new QLabel(statusBar());
    // lblPhysicField->setStyleSheet("QLabel {border: 1px solid gray;}");

    statusBar()->showMessage(tr("Ready"));
    // statusBar()->addPermanentWidget(lblMessage);
    statusBar()->addPermanentWidget(lblProblemType);
    statusBar()->addPermanentWidget(lblPhysicField);
    statusBar()->addPermanentWidget(lblPosition);

    connect(sceneView, SIGNAL(mouseMoved(const QPointF &)), this, SLOT(doSceneMouseMoved(const QPointF &)));
}

void MainWindow::doSceneMouseMoved(const QPointF &position)
{
    lblPosition->setText(tr("Position: [%1; %2]").arg(position.x(), 8, 'f', 5).arg(position.y(), 8, 'f', 5));
}

void MainWindow::createScene()
{
    QHBoxLayout *layout = new QHBoxLayout;
    
    sceneView = new SceneView(m_scene, this);
    layout->addWidget(sceneView);
    
    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    
    setCentralWidget(widget);
    setWindowTitle(tr("Carbon 2D"));
}

void MainWindow::createViews()
{
    sceneInfoView = new SceneInfoView(sceneView, this);
    sceneInfoView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, sceneInfoView);
    
    localPointValueView = new LocalPointValueView(m_scene, this);
    localPointValueView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, localPointValueView);

    volumeIntegralValueView = new VolumeIntegralValueView(this);
    volumeIntegralValueView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, volumeIntegralValueView);

    surfaceIntegralValueView = new SurfaceIntegralValueView(this);
    surfaceIntegralValueView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, surfaceIntegralValueView);
}

void MainWindow::setRecentFiles()
{
    // recent files
    if (m_scene->projectInfo().fileName != "")
    {
        QFileInfo fileInfo(m_scene->projectInfo().fileName);
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

void MainWindow::doDocumentNew()
{
    ProjectInfo projectInfo;
    ProjectDialog *projectDialog = new ProjectDialog(projectInfo, true, this);
    if (projectDialog->showDialog() == QDialog::Accepted)
    {
        m_scene->clear();
        sceneView->doDefaults();
        m_scene->projectInfo() = projectInfo;
        m_scene->refresh();

        sceneView->actSceneModeNode->trigger();
        sceneView->doZoomBestFit();
    }
    delete projectDialog;
}

void MainWindow::doDocumentOpen()
{
    QSettings settings;
    QString dir = settings.value("LastDataDir", "data").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("Carbon 2D files (*.h2d)"));
    if (!fileName.isEmpty())
    {
        m_scene->readFromFile(fileName);
        setRecentFiles();

        sceneView->doDefaults();
        sceneView->doZoomBestFit();
    }
}

void MainWindow::doDocumentOpenRecent(QAction *action)
{
    QString fileName = action->text();
    if (QFile::exists(fileName))
    {
        m_scene->readFromFile(fileName);
        setRecentFiles();

        sceneView->doDefaults();
        sceneView->doZoomBestFit();
    }
}

void MainWindow::doDocumentSave()
{
    QFile file(m_scene->projectInfo().fileName);
    if (QFile::exists(m_scene->projectInfo().fileName) && file.isWritable())
        m_scene->writeToFile(m_scene->projectInfo().fileName);
    else
        doDocumentSaveAs();
}

void MainWindow::doDocumentSaveAs()
{
    QSettings settings;
    QString dir = settings.value("LastDataDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Carbon 2D files (*.h2d)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "h2d") fileName += ".h2d";
        m_scene->writeToFile(fileName);
        setRecentFiles();
    }
}

void MainWindow::doDocumentImportDXF()
{    
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import file"), "data", "DXF files (*.dxf)");
    if (!fileName.isEmpty())
    {
        m_scene->readFromDxf(fileName);
        sceneView->doZoomBestFit();
    }
}

void MainWindow::doDocumentExportDXF()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export file"), "data", "DXF files (*.dxf)");
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "dxf") fileName += ".dxf";
        m_scene->writeToDxf(fileName);
    }
}

void MainWindow::doDocumentSaveImage()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export image to file"), "data", "PNG files (*.png)");
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "png") fileName += ".png";
        sceneView->saveImageToFile(fileName);
    }
}

void MainWindow::doCreateMesh()
{
    // create mesh
    m_scene->createMeshAndSolve(SOLVER_MESH);

    sceneView->actSceneModeLabel->trigger();
    sceneView->sceneViewSettings().showInitialMesh = true;
    sceneView->doInvalidated();

    doInvalidated();
}


void MainWindow::doSolve()
{
    // solve project
    m_scene->createMeshAndSolve(SOLVER_MESH_AND_SOLVE);
    if (m_scene->sceneSolution()->isSolved())
        sceneView->actSceneModePostprocessor->trigger();

    // show local point values
    Point point = Point(0, 0);
    localPointValueView->doShowPoint(localPointValueFactory(point, m_scene));

    doInvalidated();
}

void MainWindow::doOptions()
{
    OptionsDialog *optionsDialog = new OptionsDialog(this);
    optionsDialog->exec();

    delete optionsDialog;
}

void MainWindow::doChart()
{
    chartDialog->showDialog();
}

void MainWindow::doScriptEditor()
{    
    scriptEditorDialog->showDialog();
}

void MainWindow::doPaste()
{
    m_scene->readFromFile("data/electrostatic_axisymmetric_capacitor.h2d");
    // m_scene->readFromFile("data/electrostatic_axisymmetric_sparkgap.h2d");
    // m_scene->readFromFile("data/heat_transfer_axisymmetric.h2d");
    // m_scene->readFromFile("data/heat_transfer_planar.h2d");
    // m_scene->readFromFile("data/heat_transfer_detail.h2d");
    // m_scene->readFromFile("data/magnetostatic_planar.h2d");
    // m_scene->readFromFile("data/magnetostatic_axisymmetric_actuator.h2d");
    // m_scene->readFromFile("data/magnetostatic_planar_magnet.h2d");
    // m_scene->readFromFile("data/current_feeder.h2d");
    // m_scene->readFromFile("data/elasticity_planar.h2d");

    sceneView->doDefaults();
    doInvalidated();
    sceneView->doZoomBestFit();

    // doSolve();
    // sceneView->doZoomBestFit();
}

void MainWindow::doInvalidated()
{
    actChart->setEnabled(m_scene->sceneSolution()->isSolved());

    lblProblemType->setText(tr("Problem Type: ") + problemTypeString(m_scene->projectInfo().problemType));
    lblPhysicField->setText(tr("Physic Field: ") + physicFieldStringKey(m_scene->projectInfo().physicField));
}

void MainWindow::doAbout()
{
    QString str(tr("<b>Carbon2D %1</b><br/> <i>hp</i>-FEM multiphysics solver based on Hermes2D library.<br/><br/>Web page: <a href=\"http://hpfem.org/\">http://hpfem.math.unr.edu/projects/hermes2d-new/</a><br/><br/><b>Authors:</b><p><table><tr><td>Carbon 2D:</td><td>Pavel Karban <a href=\"mailto:pkarban@gmail.com\">pkarban@gmail.com</a></td></tr><tr><td>Hermes 2D:&nbsp;&nbsp;</td><td>Pavel Solin <a href=\"mailto:solin@unr.edu\">solin@unr.edu</a></td></tr><tr><td>&nbsp;</td><td>Jakub Cerveny <a href=\"mailto:jakub.cerveny@gmail.com\">jakub.cerveny@gmail.com</a></td></tr><tr><td>&nbsp;</td><td>Lenka Dubcova <a href=\"mailto:dubcova@gmail.com\">dubcova@gmail.com</a></td></tr><tr><td>dxflib:</td><td>Andrew Mustun (<a href=\"http://www.ribbonsoft.com/dxflib.html\">RibbonSoft</a>)</td></tr></table></p><br/><b>License:</b><p>Carbon 2D is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.</p><p>Hermes2D is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p><p>You should have received a copy of the GNU General Public License along with Hermes2D. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p>").arg(QApplication::applicationVersion()));

    QMessageBox::about(this, tr("About Carbon 2D"), str);
}
