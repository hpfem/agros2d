#ifndef SCRIPTEDITORDIALOG_H
#define SCRIPTEDITORDIALOG_H

#include <QScriptEngine>
#include <QUrl>
#include <QtGui/QDialog>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QToolBar>
#include <QtGui/QSplitter>
#include <QtGui/QDesktopServices>

#include "scene.h"
#include "scenemarker.h"
#include "scripteditorhighlighter.h"

class Scene;
class SceneView;

static Scene *m_scene;
static SceneView *m_sceneView;

class ScriptEditorDialog : public QDialog
{
    Q_OBJECT
public:
    ScriptEditorDialog(Scene *scene, SceneView *sceneView, QWidget *parent = 0);
    ~ScriptEditorDialog();

    void showDialog();

 public slots:
     void doFileNew();
     void doFileOpen(const QString &fileName = QString());
     void doFileSave();
     void doRun();
     void doCreateFromModel();
     void doHelp();

protected:
    QString m_fileName;
    QScriptEngine *m_engine;

    // ScriptEditorHelpDialog *scriptEditorHelpDialog;

    QAction *actFileNew;
    QAction *actFileOpen;
    QAction *actFileSave;
    QAction *actFileSaveAs;
    QAction *actRun;
    QAction *actCreateFromModel;
    QAction *actHelp;

    QPlainTextEdit *txtEditor;
    QPlainTextEdit *txtOutput;

    QSplitter *splitter;

    void createControls();
    void createEngine();
    void setupEditor();
};

// *****************************************************************************************************************

class ScriptEditorHelpDialog : public QDialog
{
    Q_OBJECT
public:
    ScriptEditorHelpDialog(QWidget *parent = 0);
    ~ScriptEditorHelpDialog();

    void showDialog();

private:
    // QWebView *webHelp;

    void createControls();
};


#endif // SCRIPTEDITORDIALOG_H
