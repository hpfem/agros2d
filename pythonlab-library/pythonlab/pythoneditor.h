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

#include "util.h"
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

class ErrorResult;

#ifdef Q_WS_X11
    const QFont FONT = QFont("Monospace", 9);
#endif
#ifdef Q_WS_WIN
    const QFont FONT = QFont("Courier New", 9);
#endif
#ifdef Q_WS_MAC
    const QFont FONT = QFont("Monaco", 12);
#endif

class AGROS_PYTHONLAB_API PythonEditorWidget : public QWidget
{
    Q_OBJECT
public:    
    ScriptEditor *txtEditor;
    QTreeWidget *trvPyLint;
    SearchWidget *searchWidget;
    QSplitter *splitter;

    PythonEditorWidget(PythonEngine *pythonEngine, QWidget *parent);
    ~PythonEditorWidget();

    inline QString fileName() { return m_fileName; }
    inline void setFileName(const QString &fileName) { m_fileName = QFileInfo(fileName).absoluteFilePath(); }

public slots:
    void pyLintAnalyse();
    void pyFlakesAnalyse();

private:
    QString m_fileName;
    PythonEngine *pythonEngine;

    void createControls();
    void createEngine();

private slots:
    void pyLintAnalyseStopped(int exitCode);
    void doHighlightLine(QTreeWidgetItem *item, int role);
};

class AGROS_PYTHONLAB_API PythonEditorDialog : public QMainWindow
{
    Q_OBJECT
public:
    PythonEditorDialog(PythonEngine *pythonEngine, QStringList args, QWidget *parent = 0);
    ~PythonEditorDialog();

    void showDialog();
    void closeTabs();
    bool isScriptModified();

    QStringList *recentFiles() { return &m_recentFiles; }

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
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    virtual void scriptPrepare() {}
    virtual void scriptFinish() {}

protected:
    PythonEngine *pythonEngine;

    QStringList m_recentFiles;

    // gui
    FileBrowser *fileBrowser;
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
    QToolBar *tlbEdit;
    QToolBar *tlbTools;
    QToolBar *tlbRun;

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
    QAction *actCommentAndUncommentSelection;
    QAction *actGotoLine;

    QAction *actRunPython;
    QAction *actStopPython;
    QAction *actReplaceTabsWithSpaces;
    QAction *actCheckPyLint;

    QAction *actOptionsEnablePyLint;
    QAction *actOptionsEnablePyFlakes;
    QAction *actOptionsPrintStacktrace;
    QAction *actOptionsEnableUseProfiler;

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
    void setEnabledControls(bool state);

    inline PythonEditorWidget *scriptEditorWidget() { return dynamic_cast<PythonEditorWidget *>(tabWidget->currentWidget()); }

private slots:
    void doRunPython();
    void doStopScript();
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
    void doOptionsEnableUseProfiler();

    void doStartedScript();
    void doExecutedScript();
};

class AGROS_PYTHONLAB_API ScriptEditor : public PlainTextEditParenthesis
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

    inline bool isProfiled() const { return m_isProfiled; }
    inline void setProfiled(bool prof) { m_isProfiled = prof; }

    inline QMap<int, int> profilerAccumulatedLines() const { return m_profilerAccumulatedLines; }
    inline void setProfilerAccumulatedLines(QMap<int, int> lines) { m_profilerAccumulatedLines = lines; }
    inline QMap<int, int> profilerAccumulatedTimes() const { return m_profilerAccumulatedTimes; }
    inline void setProfilerAccumulatedTimes(QMap<int, int> times) { m_profilerAccumulatedTimes = times; }

    inline int profilerMaxAccumulatedLine() const { return m_profilerMaxAccumulatedLine; }
    inline void setProfilerMaxAccumulatedLine(int val) { m_profilerMaxAccumulatedLine = val; }
    inline int profilerMaxAccumulatedTime() const { return m_profilerMaxAccumulatedTime; }
    inline void setProfilerMaxAccumulatedTime(int val) { m_profilerMaxAccumulatedTime = val; }
    inline int profilerMaxAccumulatedCallLine() const { return m_profilerMaxAccumulatedCallLine; }
    inline void setProfilerMaxAccumulatedCallLine(int val) { m_profilerMaxAccumulatedCallLine = val; }
    inline int profilerMaxAccumulatedCall() const { return m_profilerMaxAccumulatedCall; }
    inline void setProfilerMaxAccumulatedCall(int val) { m_profilerMaxAccumulatedCall = val; }

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
    void indentAndUnindentSelection(bool doIndent);
    void commentAndUncommentSelection();

    void insertCompletion(const QString& completion);

private:
    PythonEngine *pythonEngine;
    QCompleter* completer;

    QWidget *lineNumberArea;

    // profiler
    bool m_isProfiled;

    QMap<int, int> m_profilerAccumulatedLines;
    QMap<int, int> m_profilerAccumulatedTimes;

    int m_profilerMaxAccumulatedLine;
    int m_profilerMaxAccumulatedTime;
    int m_profilerMaxAccumulatedCallLine;
    int m_profilerMaxAccumulatedCall;

    friend class PythonEditorDialog;
};

class AGROS_PYTHONLAB_API ScriptEditorLineNumberArea : public QWidget
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

class AGROS_PYTHONLAB_API SearchWidget: public QWidget
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
