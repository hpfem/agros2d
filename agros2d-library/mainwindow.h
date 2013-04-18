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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "util.h"
#include "util/enums.h"

class PostHermes;
class SceneViewPost2D;
class SceneViewPost3D;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewParticleTracing;
class InfoWidget;
class SettingsWidget;
class ProblemWidget;
class ParticleTracingWidget;
class ResultsView;
class VolumeIntegralValueView;
class SurfaceIntegralValueView;
class PreprocessorWidget;
class PostprocessorWidget;
class PythonScriptingConsoleView;
class TooltipView;
class LogView;
class LogDialog;

class ChartWidget;
class PythonLabAgros;
class ReportDialog;
class ServerDownloadDialog;
class SceneTransformDialog;
class SceneViewWidget;
class LogStdOut;
class ChartView;

class AGROS_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void open(const QString &fileName);

private slots:
    inline void doSetWindowTitle(const QString &name) { setWindowTitle("Agros2D - " + name); }

    void doDocumentNew();
    void doDocumentOpen(const QString &fileName = "");
    void doDocumentDownloadFromServer();
    void doDocumentOpenRecent(QAction *action);
    void doDocumentSave();
    void doDocumentSaveAs();
    void doDocumentSaveSolution();
    void doDocumentUploadToServer();
    void doDocumentClose();
    void doDocumentImportDXF();
    void doDocumentExportDXF();
    void doDocumentExportMeshFile();
    void doDocumentSaveImage();
    void doDocumentSaveGeometry();
    void doLoadBackground();
    void doExamples();

    void doMouseSceneModeChanged(MouseSceneMode mouseSceneMode);

    void doCreateMesh();
    void doSolve();
    void doSolveAdaptiveStep();
    void doSolveFinished();

    void doScriptEditor();
    void doScriptEditorRunScript(const QString &fileName = "");
    void doOptions();
    void doTransform();
    void doReport();
    void doMaterialBrowser();
    void doCreateVideo();

    void doHideControlPanel();
    void doFullScreen();

    void doCut();
    void doCopy();
    void doPaste();

    void doHelp();
    void doHelpShortCut();
    void doCollaborationServer();
    void doOnlineHelp();
    void doCheckVersion();
    void doAbout();
    void setControls();
    void clear();

    /// fields adeed or removed, menus need to be modified
    void doFieldsChanged();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QStringList recentFiles;

    QMenu *mnuFile;
    QMenu *mnuFileImportExport;
    QMenu *mnuRecentFiles;
    QMenu *mnuEdit;
    QMenu *mnuView;
    QMenu *mnuProblem;
    QMenu *mnuTools;
    QMenu *mnuHelp;

    QToolBar *tlbFile;
    QToolBar *tlbEdit;
    QToolBar *tlbView;
    QToolBar *tlbZoom;
    QToolBar *tlbGeometry;
    QToolBar *tlbPost2D;

    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    // QAction *actDocumentDownloadFromServer;
    QAction *actDocumentSave;
    QAction *actDocumentSaveSolution;
    QAction *actDocumentSaveAs;
    // QAction *actDocumentUploadToServer;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actDocumentExportMeshFile;
    QAction *actDocumentSaveImage;
    QAction *actDocumentSaveGeometry;
    QAction *actExamples;
    QAction *actExit;
    QAction *actLoadBackground;
    QActionGroup *actDocumentOpenRecentGroup;

    QAction *actHideControlPanel;

    QAction *actUndo;
    QAction *actRedo;
    QAction *actCopy;

    QAction *actOptions;
    QAction *actCreateMesh;
    QAction *actSolve;
    QAction *actSolveAdaptiveStep;
    QAction *actFullScreen;
    QAction *actPostprocessorView;

    QAction *actScriptEditor;
    QAction *actScriptEditorRunScript;
    // QAction *actReport;
    QAction *actMaterialBrowser;
    QAction *actCreateVideo;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actOnlineHelp;
    // QAction *actCollaborationServer;
    QAction *actCheckVersion;
    QAction *actAbout;
    QAction *actAboutQt;

    QComboBox *cmbTimeStep;

    // pointers to actions (geometry, post2d and post3d)
    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;
    QAction *actSceneZoomRegion;

    // scene mode
    QActionGroup *actSceneModeGroup;

    SceneViewWidget *sceneViewInfoWidget;
    SceneViewWidget *sceneViewPreprocessorWidget;
    SceneViewWidget *sceneViewMeshWidget;
    SceneViewWidget *sceneViewPost2DWidget;
    SceneViewWidget *sceneViewPost3DWidget;
    SceneViewWidget *sceneViewPostParticleTracingWidget;
    SceneViewWidget *sceneViewBlankWidget;
    SceneViewWidget *sceneViewChartWidget;

    PostHermes *postHermes;

    QStackedLayout *tabViewLayout;
    InfoWidget *sceneInfoWidget;
    SceneViewPreprocessor *sceneViewPreprocessor;
    SceneViewMesh *sceneViewMesh;
    SceneViewPost2D *sceneViewPost2D;
    SceneViewPost3D *sceneViewPost3D;
    SceneViewParticleTracing *sceneViewParticleTracing;
    ChartView *sceneViewChart;
    QWidget *sceneViewBlank;

    QWidget *viewControls;
    QStackedLayout *tabControlsLayout;
    ProblemWidget *problemWidget;
    PreprocessorWidget *preprocessorWidget;
    PostprocessorWidget *postprocessorWidget;
    ChartWidget *chartWidget;
    ParticleTracingWidget *particleTracingWidget;
    SettingsWidget *settingsWidget;

    ResultsView *resultsView;
    PythonScriptingConsoleView *consoleView;
    LogView *logView;

    PythonLabAgros *scriptEditorDialog;
    ServerDownloadDialog *collaborationDownloadDialog;
    SceneTransformDialog *sceneTransformDialog;

    QSplitter *splitter;

    LogStdOut *logStdOut;

    void setRecentFiles();

    void createActions();
    void createToolBox();
    void createMenus();
    void createToolBars();
    void createMain();
    void createViews();   
};

#endif // MAINWINDOW_H
