#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScriptEngine>
#include <QtScriptTools/QScriptEngineDebugger>

#include "util.h"
#include "scene.h"
#include "scenebasic.h"
#include "sceneview.h"
#include "sceneinfo.h"
#include "helpdialog.h"
#include "solverdialog.h"
#include "localvalueview.h"
#include "volumeintegralview.h"
#include "surfaceintegralview.h"
#include "chartdialog.h"
#include "optionsdialog.h"
#include "scripteditordialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void doDocumentNew();
    void doDocumentOpen();
    void doDocumentOpenRecent(QAction *action);
    void doDocumentSave();
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
    void doScriptEditorRun();
    void doScriptStartup();
    void doOptions();

    void doUndo();
    void doRedo();
    void doCut();
    void doCopy();
    void doPaste();

    void doHelp();
    void doHelpShortCut();
    void doAbout();
    void doInvalidated();

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

    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    QActionGroup *actDocumentOpenRecentGroup;
    QAction *actDocumentSave;
    QAction *actDocumentSaveAs;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actDocumentSaveImage;
    QAction *actExit;

    QAction *actUndo;
    QAction *actRedo;
    QAction *actCut;
    QAction *actCopy;
    QAction *actPaste;

    QAction *actOptions;
    QAction *actCreateMesh;
    QAction *actSolve;
    QAction *actChart; 

    QAction *actScriptEditor;
    QAction *actScriptEditorRun;
    QAction *actScriptStartup;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actAbout;
    QAction *actAboutQt;

    QLabel *lblProblemType;
    QLabel *lblPhysicField;
    QLabel *lblMessage;
    QLabel *lblPosition;

    SceneView *sceneView;
    SceneInfoView *sceneInfoView;
    LocalPointValueView *localPointValueView;
    VolumeIntegralValueView *volumeIntegralValueView;
    SurfaceIntegralValueView *surfaceIntegralValueView;

    HelpDialog *helpDialog;
    ChartDialog *chartDialog;
    ScriptEditorDialog *scriptEditorDialog;

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
