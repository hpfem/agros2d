#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScriptEngine>
#include <QtScriptTools/QScriptEngineDebugger>

#include "util.h"
#include "scene.h"
#include "scenebasic.h"
#include "sceneview.h"
#include "sceneinfoview.h"
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
    void runScript(const QString &fileName);

private slots:
    void doSetWindowTitle(const QString &name) { setWindowTitle("Agros2D - " + name); }

    void doDocumentNew();
    void doDocumentOpen(const QString &fileName = "");
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
    void doScriptEditorRunScript(const QString &fileName = "");
    void doScriptEditorRunCommand(const QString &command = "");
    void doScriptStartup();
    void doOptions();
    void doReport();
    void doCreateVideo();

    void doCut();
    void doCopy();
    void doPaste();

    void doHelp();
    void doHelpShortCut();
    void doAbout();
    void doInvalidated();

    void doTimeStepChanged(int index);

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
    QAction *actDocumentSaveAs;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actDocumentSaveImage;
    QAction *actExit;
    QActionGroup *actDocumentOpenRecentGroup;

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
    QAction *actScriptEditorRunScript;
    QAction *actScriptEditorRunCommand;
    QAction *actScriptStartup;
    QAction *actReport;
    QAction *actCreateVideo;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actAbout;
    QAction *actAboutQt;

    QLabel *lblProblemType;
    QLabel *lblPhysicField;
    QLabel *lblMessage;
    QLabel *lblPosition;
    QLabel *lblTimeStep;

    QComboBox *cmbTimeStep;

    SceneView *sceneView;
    SceneInfoView *sceneInfoView;
    LocalPointValueView *localPointValueView;
    VolumeIntegralValueView *volumeIntegralValueView;
    SurfaceIntegralValueView *surfaceIntegralValueView;

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
