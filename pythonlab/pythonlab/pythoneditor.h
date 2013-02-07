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

#ifndef SCRIPTEDITORDIALOG_H
#define SCRIPTEDITORDIALOG_H

#include <QtGui>
#include <QtCore>

#include "gui/textedit.h"

class PythonEngine;
class PythonScriptingConsole;
class PythonScriptingConsoleView;
class PythonScriptingHistoryView;
class PythonBrowserView;

class SceneView;
class FileBrowser;

class Scene;
class SceneView;
class ScriptEditor;
class SearchDialog;
class SearchWidget;

struct ScriptResult;
struct ExpressionResult;

#ifdef Q_WS_X11
    const QFont FONT = QFont("Monospace", 9);
#endif
#ifdef Q_WS_WIN
    const QFont FONT = QFont("Courier New", 9);
#endif
#ifdef Q_WS_MAC
    const QFont FONT = QFont("Monaco", 12);
#endif

class AGROS_API PythonEditorWidget : public QWidget
{
    Q_OBJECT
public:
    QString fileName;
    ScriptEditor *txtEditor;
    QTreeWidget *trvPyLint;
    SearchWidget *searchWidget;
    QSplitter *splitter;

    PythonEditorWidget(PythonEngine *pythonEngine, QWidget *parent);
    ~PythonEditorWidget();

public slots:
    void pyLintAnalyse();
    void pyFlakesAnalyse();

private:
    PythonEngine *pythonEngine;

    void createControls();
    void createEngine();

private slots:
    void pyLintAnalyseStopped(int exitCode);    
    void doHighlightLine(QTreeWidgetItem *item, int role);   
};

class AGROS_API PythonEditorDialog : public QMainWindow
{
    Q_OBJECT
public:
    PythonEditorDialog(PythonEngine *pythonEngine, QStringList args, QWidget *parent = 0);
    virtual ~PythonEditorDialog();

    void showDialog();
    void closeTabs();
    bool isScriptModified();

public slots:
    void doFileNew();
    void doFileOpen(const QString &file = QString());
    void doFileSave();
    void doFileSaveAs();
    void doFileSaveConsoleAs();
    void doFileClose();
    void doFileOpenRecent(QAction *action);
    void doFilePrint();

    void doFind();
    void doFindNext(bool fromBegining = false);
    void doReplace();

    void doDataChanged();

    void doHelp();
    void doHelpKeywordList();
    void doAbout();

    void doCloseTab(int index);

    // message from another app
    void onOtherInstanceMessage(const QString &msg);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

protected:
    PythonEngine *pythonEngine;

    QStringList recentFiles;

    // gui
    FileBrowser *filBrowser;

    ScriptEditor *txtEditor;

    PythonScriptingConsoleView *consoleView;
    PythonScriptingHistoryView *consoleHistoryView;
    PythonBrowserView *variablesView;
    QDockWidget *fileBrowserView;

    QLabel *lblCurrentPosition;

    QMenu *mnuFile;
    QMenu *mnuRecentFiles;
    QMenu *mnuEdit;
    QMenu *mnuTools;
    QMenu *mnuOptions;
    QMenu *mnuHelp;

    QToolBar *tlbFile;
    QToolBar *tlbTools;

    QAction *actFileNew;
    QAction *actFileOpen;
    QAction *actFileSave;
    QAction *actFileSaveAs;
    QAction *actFileSaveConsoleAs;
    QAction *actFileClose;
    QAction *actFilePrint;
    QAction *actExit;
    QActionGroup *actFileOpenRecentGroup;

    QAction *actUndo;
    QAction *actRedo;
    QAction *actCut;
    QAction *actCopy;
    QAction *actPaste;

    QAction *actFind;
    QAction *actFindNext;
    QAction *actReplace;

    QAction *actIndentSelection;
    QAction *actUnindentSelection;
    QAction *actCommentSelection;
    QAction *actUncommentSelection;
    QAction *actGotoLine;

    QAction *actRunPython;
    QAction *actReplaceTabsWithSpaces;
    QAction *actCheckPyLint;

    QAction *actOptionsEnablePyLint;
    QAction *actOptionsEnablePyFlakes;
    QAction *actOptionsPrintStacktrace;

    QAction *actHelp;
    QAction *actHelpKeywordList;
    QAction *actAbout;
    QAction *actAboutQt;

    QTabWidget *tabWidget;

    void createActions();
    void createControls();
    void createViews();
    void createStatusBar();

    void setRecentFiles();

    inline PythonEditorWidget *scriptEditorWidget() { return dynamic_cast<PythonEditorWidget *>(tabWidget->currentWidget()); }

private slots:
    void doRunPython();
    void doReplaceTabsWithSpaces();
    void doPyLintPython();
    void doFileItemDoubleClick(const QString &path);
    void doPathChangeDir();
    void doCurrentDocumentChanged(bool changed);
    void doCurrentPageChanged(int index);
    void doCursorPositionChanged();
    void doOptionsEnablePyFlakes();
    void doOptionsEnablePyLint();
    void doOptionsPrintStacktrace();
};

class AGROS_API ScriptEditor : public PlainTextEditParenthesis
{
    Q_OBJECT

public:
    QMap<int, QString> errorMessagesPyFlakes;

    ScriptEditor(PythonEngine *pythonEngine, QWidget *parent = 0);
    ~ScriptEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineNumberAreaMouseMoveEvent(QMouseEvent *event);
    int lineNumberAreaWidth();

    void replaceTabsWithSpaces();

public slots:
    void gotoLine(int line = -1, bool isError = false);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine(bool isError = false);
    void updateLineNumberArea(const QRect &, int);

    void indentSelection();
    void unindentSelection();
    void commentSelection();
    void uncommentSelection();

    void insertCompletion(const QString& completion);

private:
    PythonEngine *pythonEngine;
    QCompleter* completer;

    QWidget *lineNumberArea;
};

class AGROS_API ScriptEditorLineNumberArea : public QWidget
{
public:
    ScriptEditorLineNumberArea(ScriptEditor *editor) : QWidget(editor)
    {
        setMouseTracking(true);
        codeEditor = editor;
    }

    QSize sizeHint() const { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) { codeEditor->lineNumberAreaPaintEvent(event); }

    virtual void mouseMoveEvent(QMouseEvent *event) { codeEditor->lineNumberAreaMouseMoveEvent(event); }

private:
    ScriptEditor *codeEditor;
};

// ************************************************************************************************************

class AGROS_API SearchWidget: public QWidget
{
    Q_OBJECT
public:
    SearchWidget(ScriptEditor *txtEditor, QWidget *parent = 0);

    void showFind(const QString &text = "");
    void showReplaceAll(const QString &text = "");

public slots:
    void find();
    void findNext(bool fromBegining);
    void replaceAll();
    void hideWidget();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    ScriptEditor *txtEditor;

    QLabel *lblFind, *lblReplace;
    QLineEdit *txtFind, *txtReplace;
    QPushButton *btnFind, *btnReplace, *btnHide;

    bool startFromBeginning;
};

#endif // SCRIPTEDITORDIALOG_H
