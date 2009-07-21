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

static SceneView *m_sceneView;

class ScriptEditor;

class ScriptEditorDialog : public QDialog
{
    Q_OBJECT
public:
    ScriptEditorDialog(SceneView *sceneView, QWidget *parent = 0);
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

    ScriptEditor *txtEditor;
    QPlainTextEdit *txtOutput;

    QSplitter *splitter;

    void createControls();
    void createEngine();
    void setupEditor();
};

class ScriptStartupDialog : public QDialog
{
    Q_OBJECT
public:
    ScriptStartupDialog(ProblemInfo &problemInfo, QWidget *parent = 0);
    ~ScriptStartupDialog();

    int showDialog();

private slots:
    void doAccept();
    void doReject();

private:
    ProblemInfo *m_problemInfo;

    ScriptEditor *txtEditor;

    void createControls();
};

class ScriptEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    ScriptEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void doUpdateLineNumberAreaWidth(int newBlockCount);
    void doHighlightCurrentLine();
    void doUpdateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

class ScriptEditorLineNumberArea : public QWidget
{
public:
    ScriptEditorLineNumberArea(ScriptEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    ScriptEditor *codeEditor;
};

#endif // SCRIPTEDITORDIALOG_H
