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

#include "util.h"

class PythonEngine;

class SceneView;
class FileBrowser;
class TerminalView;
class Terminal;

class Scene;
class SceneView;
class ScriptEditor;
class SearchDialog;

struct ScriptResult;
struct ExpressionResult;

void createScriptEngine();
PythonEngine *currentPythonEngine();
void connectTerminal(Terminal *terminal);
void disconnectTerminal(Terminal *terminal);
bool scriptIsRunning();

QString createPythonFromModel();
ScriptResult runPythonScript(const QString &script, const QString &fileName = "");
ExpressionResult runPythonExpression(const QString &expression);
QString fillTimeFunction(const QString &expression, double time_min, double time_max, double N, QList<double> *list);

class ScriptEngineRemote : QObject
{
    Q_OBJECT

public:
    ScriptEngineRemote();
    ~ScriptEngineRemote();

private slots:
    void connected();
    void readCommand();
    void disconnected();

    void displayError(QLocalSocket::LocalSocketError socketError);

private:
    QString command;

    QLocalServer *m_server;
    QLocalSocket *m_server_socket;
    QLocalSocket *m_client_socket;
};

class ScriptEditorWidget : public QWidget
{
    Q_OBJECT
public:
    QString file;
    ScriptEditor *txtEditor;

    ScriptEditorWidget(QWidget *parent);
    ~ScriptEditorWidget();

    void createControls();
    void createEngine();
};

class ScriptEditorDialog : public QMainWindow
{
    Q_OBJECT
public:
    ScriptEditorDialog(QWidget *parent = 0);
    ~ScriptEditorDialog();

    void showDialog();
    void closeTabs();
    bool isScriptModified();

public slots:
    void doFileNew();
    void doFileOpen(const QString &file = QString());
    void doFileSave();
    void doFileSaveAs();
    void doFileClose();
    void doFileOpenRecent(QAction *action);
    void doFilePrint();

    void doFind();
    void doFindNext(bool fromBegining = false);
    void doReplace();

    void doDataChanged();
    void doHelp();

    void doCloseTab(int index);

private:
    QStringList recentFiles;

    FileBrowser *filBrowser;

    ScriptEditor *txtEditor;
    SearchDialog *searchDialog;

    TerminalView *terminalView;
    QDockWidget *fileBrowserView;

    QLabel *lblCurrentPosition;

    QMenu *mnuFile;
    QMenu *mnuRecentFiles;
    QMenu *mnuEdit;
    QMenu *mnuTools;
    QMenu *mnuHelp;

    QAction *actFileNew;
    QAction *actFileOpen;
    QAction *actFileSave;
    QAction *actFileSaveAs;
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
    QAction *actCreateFromModel;

    QAction *actHelp;

    QTabWidget *tabWidget;

    void createActions();
    void createControls();
    void createViews();
    void createStatusBar();

    void setRecentFiles();

    inline ScriptEditorWidget *scriptEditorWidget() { return dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget()); }

private slots:
    void doRunPython();
    void doCreatePythonFromModel();
    void doFileItemDoubleClick(const QString &path);
    void doPathChangeDir();
    void doCurrentDocumentChanged(bool changed);
    void doCurrentPageChanged(int index);
    void doCursorPositionChanged();
};

class ScriptEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    ScriptEditor(QWidget *parent = 0);
    ~ScriptEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

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

    void matchParentheses(char left, char right);

private:
    QWidget *lineNumberArea;

    bool matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(char left, char right, QTextBlock currentBlock, int index, int numLeftParentheses);
    void createParenthesisSelection(int pos);
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

// ************************************************************************************************************

class SearchDialog: public QDialog
{
    Q_OBJECT
public:
    // Constructor
    SearchDialog(QWidget *parent=0);
    ~SearchDialog();

    int showDialogFind();
    int showDialogReplace();

    inline QString searchString()  { return txtFind->text(); }
    inline QString replaceString() { return txtReplace->text(); }

    inline bool searchStringIsRegExp() { return chkSearchRegExp->checkState(); }
    inline bool caseSensitive() { return chkCaseSensitive->checkState(); }

private:
    QLineEdit *txtFind, *txtReplace;
    QCheckBox *chkSearchRegExp, *chkCaseSensitive;
    QPushButton *btnCancel, *btnConfirm;
};

#endif // SCRIPTEDITORDIALOG_H
