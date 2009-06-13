#ifndef SCRIPTEDITORDIALOG_H
#define SCRIPTEDITORDIALOG_H

#include <QScriptEngine>
#include <QtGui/QDialog>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QToolBar>
#include <QtGui/QSplitter>

#include "scene.h"
#include "scenemarker.h"

typedef QScriptValue (*FunctionWithArgSignature)(QScriptContext *, QScriptEngine *, void *);

class Scene;

static Scene *m_scene;

class ScriptEditorDialog : public QDialog
{
    Q_OBJECT
public:
    ScriptEditorDialog(Scene *scene, QWidget *parent = 0);
    ~ScriptEditorDialog();

    void showDialog();

 public slots:
     void doFileNew();
     void doFileOpen(const QString &fileName = QString());
     void doFileSave();
     void doRun();
     void doCreateFromModel();

protected:
    QString m_fileName;
    // Scene *m_scene;
    QScriptEngine *m_engine;

    QAction *actFileNew;
    QAction *actFileOpen;
    QAction *actFileSave;
    QAction *actFileSaveAs;
    QAction *actRun;
    QAction *actCreateFromModel;

    QPlainTextEdit *txtEditor;
    QPlainTextEdit *txtOutput;

    QSplitter *splitter;

    void createControls();
    void createEngine();
    void setupEditor();
};

// *****************************************************************************************************************

QScriptValue scriptPrint(QScriptContext *context, QScriptEngine *engine);


#endif // SCRIPTEDITORDIALOG_H
