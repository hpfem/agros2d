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
#include "scene.h"
#include "scenebasic.h"
#include "sceneview.h"
#include "sceneinfoview.h"
#include "terminalview.h"
#include "tooltipview.h"
#include "helpdialog.h"
#include "solverdialog.h"
#include "chartdialog.h"
#include "optionsdialog.h"
#include "scripteditordialog.h"
#include "reportdialog.h"
#include "videodialog.h"

class MainWindow : public QMainWindow
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
    void doDocumentOpenRecent(QAction *action);
    void doDocumentSave();
    void doDocumentSaveWithSolution();
    void doDocumentSaveAs();
    void doDocumentClose();
    void doDocumentImportDXF();
    void doDocumentExportDXF();
    void doDocumentSaveImage();

    void doSceneMouseMoved(const QPointF &position);

    void doCreateMesh();
    void doSolve();

    void doChart();
    void doScriptEditor();
    void doScriptEditorRunScript(const QString &fileName = "");
    void doScriptEditorRunCommand();
    void doOptions();
    void doReport();
    void doCreateVideo();
    void doFullScreen();

    void doCut();
    void doCopy();
    void doPaste();

    void doHelp();
    void doHelpShortCut();
    void doOnlineHelp();
    void doCheckVersion();
    void doAbout();
    void doInvalidated();

    void doTimeStepChanged(int index);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QStringList recentFiles;

    QMenu *mnuFile;
    QMenu *mnuRecentFiles;
    QMenu *mnuEdit;
    QMenu *mnuView;
    QMenu *mnuProblem;
    QMenu *mnuAdd;
    QMenu *mnuTools;
    QMenu *mnuHelp;

    QToolBar *tlbFile;
    QToolBar *tlbEdit;
    QToolBar *tlbView;
    QToolBar *tlbProblem;
    QToolBar *tlbTools;
    QToolBar *tlbTransient;

    QAction *actDocumentNew;
    QAction *actDocumentOpen;   
    QAction *actDocumentSave;
    QAction *actDocumentSaveWithSolution;
    QAction *actDocumentSaveAs;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actDocumentSaveImage;
    QAction *actExit;
    QActionGroup *actDocumentOpenRecentGroup;

    QAction *actUndo;
    QAction *actRedo;
    QAction *actCopy;

    QAction *actOptions;
    QAction *actCreateMesh;
    QAction *actSolve;
    QAction *actChart;
    QAction *actFullScreen;

    QAction *actScriptEditor;
    QAction *actScriptEditorRunScript;
    QAction *actScriptEditorRunCommand;
    QAction *actReport;
    QAction *actCreateVideo;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actOnlineHelp;
    QAction *actCheckVersion;
    QAction *actAbout;
    QAction *actAboutQt;

    QLabel *lblProblemType;
    QLabel *lblPhysicField;
    QLabel *lblMessage;
    QLabel *lblPosition;
    QLabel *lblAnalysisType;

    QComboBox *cmbTimeStep;

    SceneView *sceneView;
    SceneInfoView *sceneInfoView;
    LocalPointValueView *localPointValueView;
    VolumeIntegralValueView *volumeIntegralValueView;
    SurfaceIntegralValueView *surfaceIntegralValueView;
    TerminalView *terminalView;
    TooltipView *tooltipView;

    HelpDialog *helpDialog;
    ChartDialog *chartDialog;
    ScriptEditorDialog *scriptEditorDialog;
    ReportDialog *reportDialog;
    VideoDialog *videoDialog;

    void setRecentFiles();

    void createActions();
    void createToolBox();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createScene();
    void createViews();
};

#endif // MAINWINDOW_H
