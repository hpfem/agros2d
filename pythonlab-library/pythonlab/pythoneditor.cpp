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

#include "pythoneditor.h"
#include "pythonhighlighter.h"
#include "pythonconsole.h"
#include "pythonbrowser.h"
#include "pythonengine.h"
#include "pythoncompleter.h"

// #include "util/constants.h"
#include "gui/filebrowser.h"
#include "gui/about.h"

const QString TABS = "    ";
const int TABS_SIZE = 4;

int firstNonSpace(const QString& text)
{
    int i = 0;
    while (i < text.size())
    {
        if (!text.at(i).isSpace())
            return i;
        ++i;
    }
    return i;
}

int indentedColumn(int column, bool doIndent)
{
    int aligned = (column / TABS_SIZE) * TABS_SIZE;
    if (doIndent)
        return aligned + TABS_SIZE;
    if (aligned < column)
        return aligned;
    return qMax(0, aligned - TABS_SIZE);
}

int columnAt(const QString& text, int position)
{
    int column = 0;
    for (int i = 0; i < position; ++i)
    {
        if (text.at(i) == QLatin1Char('\t'))
            column = column - (column % TABS_SIZE) + TABS_SIZE;
        else
            ++column;
    }
    return column;
}

PythonEditorWidget::PythonEditorWidget(PythonEngine *pythonEngine, QWidget *parent)
    : QWidget(parent), pythonEngine(pythonEngine)
{
    m_fileName = "";

    createControls();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(pyFlakesAnalyse()));
    timer->start(4000);

    txtEditor->setAcceptDrops(false);
}

PythonEditorWidget::~PythonEditorWidget()
{
    QSettings settings;

    settings.setValue("PythonEditorWidget/Geometry", saveGeometry());
    settings.setValue("PythonEditorWidget/SplitterState", splitter->saveState());
    settings.setValue("PythonEditorWidget/SplitterState", splitter->saveState());
    settings.setValue("PythonEditorWidget/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("PythonEditorWidget/EditorHeight", txtEditor->height());
}

void PythonEditorWidget::createControls()
{
    txtEditor = new ScriptEditor(pythonEngine, this);
    searchWidget = new SearchWidget(txtEditor, this);

    QVBoxLayout *layoutEditor = new QVBoxLayout();
    layoutEditor->addWidget(txtEditor);
    layoutEditor->addWidget(searchWidget);

    QWidget *editor = new QWidget();
    editor->setLayout(layoutEditor);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(editor);

    QSettings settings;
    trvPyLint = new QTreeWidget(this);
    trvPyLint->setHeaderHidden(true);
    trvPyLint->setMouseTracking(true);
    trvPyLint->setColumnCount(1);
    trvPyLint->setIndentation(12);
    trvPyLint->setMaximumHeight(150);
    connect(trvPyLint, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doHighlightLine(QTreeWidgetItem *, int)));

    splitter->addWidget(trvPyLint);

    // contents
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(1);
    layout->addWidget(splitter);

    setLayout(layout);

    restoreGeometry(settings.value("PythonEditorWidget/Geometry", saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("PythonEditorWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PythonEditorWidget/SplitterGeometry").toByteArray());
    txtEditor->resize(txtEditor->height(), settings.value("PythonEditorWidget/EditorHeight").toInt());
}

void PythonEditorWidget::pyLintAnalyse()
{
    trvPyLint->clear();

    QProcess processPyLint;
    processPyLint.setStandardOutputFile(tempProblemFileName() + ".pylint.out");
    processPyLint.setStandardErrorFile(tempProblemFileName() + ".pylint.err");
    connect(&processPyLint, SIGNAL(finished(int)), this, SLOT(pyLintAnalyseStopped(int)));

#ifdef Q_WS_X11
    QString pylintBinary = datadir() + "/resources/python/pylint_lab";
#endif
#ifdef Q_WS_WIN
    QString pylintBinary = datadir() + "/resources/python/pylint_lab.bat";
#endif

    QString test = txtEditor->toPlainText();
    writeStringContent(tempProblemFileName() + ".pylint.py", &test);

    QStringList arguments;
    arguments << "-i" << "yes" << tempProblemFileName() + ".pylint.py";

    processPyLint.setWorkingDirectory(datadir() + "/resources/python");
    processPyLint.start(pylintBinary, arguments);

    if (!processPyLint.waitForStarted())
    {
        qDebug() << "Could not start PyLint: " << processPyLint.errorString();

        processPyLint.kill();
        return;
    }

    while (!processPyLint.waitForFinished()) {}
}

void PythonEditorWidget::pyLintAnalyseStopped(int exitCode)
{
    // QString output = readFileContent(tempProblemFileName() + ".pylint.out");
    // qDebug() << output;

    QTreeWidgetItem *itemConvention = new QTreeWidgetItem(trvPyLint);
    itemConvention->setText(0, tr("Convention"));
    itemConvention->setIcon(0, icon("check-convention"));
    QTreeWidgetItem *itemWarning = new QTreeWidgetItem(trvPyLint);
    itemWarning->setText(0, tr("Warning"));
    itemWarning->setIcon(0, icon("check-warning"));
    itemWarning->setExpanded(true);
    QTreeWidgetItem *itemError = new QTreeWidgetItem(trvPyLint);
    itemError->setText(0, tr("Error"));
    itemError->setIcon(0, icon("check-error"));
    itemError->setExpanded(true);

    QFile fileOutput(tempProblemFileName() + ".pylint.out");
    if (!fileOutput.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << tr("Could not read PyLint output.");
        return;
    }
    QTextStream inOutput(&fileOutput);

    QString line_str;
    do
    {
        line_str = inOutput.readLine();

        if (!line_str.isEmpty())
        {
            if (line_str.startsWith("C") || line_str.startsWith("W") || line_str.startsWith("E"))
            {
                QString type;
                QString typeFamily;
                QString line_column;
                int line;
                QString message;

                QStringList list = line_str.split(":");
                if (list.count() == 3)
                {
                    type = list[0];
                    line_column = list[1];
                    line = line_column.split(",").at(0).toInt();
                    message = list[2];

                    QTreeWidgetItem *item;
                    if (type.startsWith("C"))
                    {
                        typeFamily = tr("Convention");
                        item = new QTreeWidgetItem(itemConvention);
                    }
                    else if (type.startsWith("W"))
                    {
                        typeFamily = tr("Warning");
                        item = new QTreeWidgetItem(itemWarning);
                    }
                    else
                    {
                        typeFamily = tr("Error");
                        item = new QTreeWidgetItem(itemError);
                    }

                    item->setText(0, QString("%1: %2").
                                  arg(line_column).
                                  arg(message));

                    item->setData(0, Qt::UserRole, line);
                }
            }
        }
    } while (!line_str.isNull());

    txtEditor->repaint();

    // QString error = readFileContent(tempProblemFileName() + ".pylint.err");
    // qDebug() << error;
}

void PythonEditorWidget::pyFlakesAnalyse()
{
    if (isVisible() && !pythonEngine->isScriptRunning())
    {
        QString fn = tempProblemFileName() + ".pyflakes_str.py";
        QString str = txtEditor->toPlainText();
        writeStringContent(fn, &str);

        QStringList messages = pythonEngine->codePyFlakes(fn);

        txtEditor->errorMessagesPyFlakes.clear();
        foreach (QString line, messages)
        {
            if (!line.isEmpty())
            {
                int number;
                QString message;

                QStringList list = line.split(":");
                if (list.count() == 3)
                {
                    number = list[1].toInt();
                    message = list[2];

                    txtEditor->errorMessagesPyFlakes[number] = message;
                }
            }
        }

        QFile::remove(fn);

        txtEditor->repaint();
    }
}

void PythonEditorWidget::doHighlightLine(QTreeWidgetItem *item, int role)
{
    if (item)
    {
        int line = item->data(0, Qt::UserRole).value<int>();

        txtEditor->gotoLine(line, true);
    }
}

// ***********************************************************************************************************

PythonEditorDialog::PythonEditorDialog(PythonEngine *pythonEngine, QStringList args, QWidget *parent)
    : QMainWindow(parent), pythonEngine(pythonEngine)
{
    setWindowIcon(icon("pythonlab"));

    createStatusBar();
    createActions();
    createViews();
    createControls();

    QSettings settings;
    fileBrowser->setDir(settings.value("PythonEditorDialog/WorkDir", datadir()).value<QString>());
    fileBrowser->refresh();

    connect(actRunPython, SIGNAL(triggered()), this, SLOT(doRunPython()));
    connect(actStopPython, SIGNAL(triggered()), this, SLOT(doStopScript()));
    connect(actReplaceTabsWithSpaces, SIGNAL(triggered()), this, SLOT(doReplaceTabsWithSpaces()));
    connect(actCheckPyLint, SIGNAL(triggered()), this, SLOT(doPyLintPython()));

    connect(pythonEngine, SIGNAL(startedScript()), this, SLOT(doStartedScript()));
    connect(pythonEngine, SIGNAL(executedScript()), this, SLOT(doExecutedScript()));

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    // parameters
    for (int i = 1; i < args.count(); i++)
    {
        QString fileName =
                QFile::exists(args[i]) ? args[i] : QApplication::applicationDirPath() + QDir::separator() + args[i];

        if (QFile::exists(fileName))
        {
            QFileInfo fileInfo(fileName);
            doFileOpen(fileInfo.absoluteFilePath());
        }
    }

    setAcceptDrops(true);

    restoreGeometry(settings.value("PythonEditorDialog/Geometry", saveGeometry()).toByteArray());
    m_recentFiles = settings.value("PythonEditorDialog/RecentFiles").value<QStringList>();
    restoreState(settings.value("PythonEditorDialog/State", saveState()).toByteArray());

    // set recent files
    setRecentFiles();
}

PythonEditorDialog::~PythonEditorDialog()
{
    QSettings settings;
    settings.setValue("PythonEditorDialog/Geometry", saveGeometry());
    settings.setValue("PythonEditorDialog/State", saveState());
    settings.setValue("PythonEditorDialog/RecentFiles", m_recentFiles);
}

void PythonEditorDialog::closeEvent(QCloseEvent *event)
{
    // check script editor
    closeTabs();

    if (!isScriptModified())
        event->accept();
    else
    {
        event->ignore();
        // show script editor
        if (isScriptModified())
            show();
    }
}

void PythonEditorDialog::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void PythonEditorDialog::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void PythonEditorDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QString fileName = QUrl(event->mimeData()->urls().at(0)).toLocalFile().trimmed();
        if (QFile::exists(fileName))
        {
            QFileInfo fileInfo(fileName);
            doFileOpen(fileInfo.absoluteFilePath());

            event->acceptProposedAction();
        }
    }
}

void PythonEditorDialog::showDialog()
{
    show();
    activateWindow();
    raise();
    txtEditor->setFocus();
}

void PythonEditorDialog::createActions()
{
    actFileNew = new QAction(icon("document-new"), tr("&New"), this);
    actFileNew->setShortcuts(QKeySequence::AddTab);
    connect(actFileNew, SIGNAL(triggered()), this, SLOT(doFileNew()));

    actFileOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actFileOpen->setShortcuts(QKeySequence::Open);
    connect(actFileOpen, SIGNAL(triggered()), this, SLOT(doFileOpen()));

    actFileSave = new QAction(icon("document-save"), tr("&Save"), this);
    actFileSave->setShortcuts(QKeySequence::Save);
    connect(actFileSave, SIGNAL(triggered()), this, SLOT(doFileSave()));

    actFileSaveAs = new QAction(icon("document-save-as"), tr("Save &as..."), this);
    actFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actFileSaveAs, SIGNAL(triggered()), this, SLOT(doFileSaveAs()));

    actFileSaveConsoleAs = new QAction(icon(""), tr("Save console output as..."), this);
    connect(actFileSaveConsoleAs, SIGNAL(triggered()), this, SLOT(doFileSaveConsoleAs()));

    actFileOpenRecentGroup = new QActionGroup(this);
    connect(actFileOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doFileOpenRecent(QAction *)));

    actFileClose = new QAction(icon(""), tr("&Close"), this);
    actFileClose->setShortcuts(QKeySequence::Close);
    connect(actFileClose, SIGNAL(triggered()), this, SLOT(doFileClose()));

    actFilePrint = new QAction(icon(""), tr("&Print"), this);
    actFilePrint->setShortcuts(QKeySequence::Print);
    connect(actFilePrint, SIGNAL(triggered()), this, SLOT(doFilePrint()));

    actUndo = new QAction(icon("edit-undo"), tr("&Undo"), this);
    actUndo->setShortcut(QKeySequence::Undo);

    actRedo = new QAction(icon("edit-redo"), tr("&Redo"), this);
    actRedo->setShortcut(QKeySequence::Redo);

    actCut = new QAction(icon("edit-cut"), tr("Cu&t"), this);
    actCut->setShortcut(QKeySequence::Cut);
    actCut->setEnabled(false);

    actCopy = new QAction(icon("edit-copy"), tr("&Copy"), this);
    actCopy->setShortcut(QKeySequence::Copy);
    actCopy->setEnabled(false);

    actPaste = new QAction(icon("edit-paste"), tr("&Paste"), this);
    actPaste->setShortcut(QKeySequence::Paste);

    actFind = new QAction(icon("edit-find"), tr("&Find"), this);
    actFind->setShortcut(QKeySequence::Find);
    connect(actFind, SIGNAL(triggered()), this, SLOT(doFind()));

    actFindNext = new QAction(icon("edit-find"), tr("Find &next"), this);
    actFindNext->setShortcut(QKeySequence::FindNext);
    connect(actFindNext, SIGNAL(triggered()), this, SLOT(doFindNext()));

    actReplace = new QAction(icon("edit-find-replace"), tr("Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actReplace = new QAction(icon("edit-find-replace"), tr("Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actIndentSelection = new QAction(icon(""), tr("Indent"), this);
    actUnindentSelection = new QAction(icon(""), tr("Unindent"), this);

    actCommentAndUncommentSelection = new QAction(icon(""), tr("Toggle comment selection"), this);
    actCommentAndUncommentSelection->setShortcut(tr("Ctrl+/"));

    actGotoLine = new QAction(icon(""), tr("Goto line"), this);
    actGotoLine->setShortcut(tr("Alt+G"));

    actRunPython = new QAction(icon("run"), tr("&Run Python script"), this);
    actRunPython->setShortcut(QKeySequence(tr("Ctrl+R")));

    actStopPython = new QAction(icon("stop"), tr("Stop Python script"), this);
    actStopPython->setEnabled(false);

    actReplaceTabsWithSpaces = new QAction(icon(""), tr("Replace tabs with spaces"), this);

    QSettings settings;
    actCheckPyLint = new QAction(icon("checkbox"), tr("&Check Python script (PyLint)"), this);
    actCheckPyLint->setEnabled(settings.value("PythonEditorWidget/EnablePyLint", true).toBool());
    actCheckPyLint->setShortcut(QKeySequence(tr("Alt+C")));

    actOptionsEnablePyFlakes = new QAction(icon(""), tr("PyFlakes enabled"), this);
    actOptionsEnablePyFlakes->setCheckable(true);
    actOptionsEnablePyFlakes->setChecked(settings.value("PythonEditorWidget/EnablePyFlakes", true).toBool());
    connect(actOptionsEnablePyFlakes, SIGNAL(triggered()), this, SLOT(doOptionsEnablePyFlakes()));

    actOptionsEnablePyLint = new QAction(icon(""), tr("PyLint enabled"), this);
    actOptionsEnablePyLint->setCheckable(true);
    actOptionsEnablePyLint->setChecked(settings.value("PythonEditorWidget/EnablePyLint", true).toBool());
    connect(actOptionsEnablePyLint, SIGNAL(triggered()), this, SLOT(doOptionsEnablePyLint()));

    actOptionsPrintStacktrace = new QAction(icon(""), tr("Print stacktrace"), this);
    actOptionsPrintStacktrace->setCheckable(true);
    actOptionsPrintStacktrace->setChecked(settings.value("PythonEditorWidget/PrintStacktrace", true).toBool());
    connect(actOptionsPrintStacktrace, SIGNAL(triggered()), this, SLOT(doOptionsPrintStacktrace()));

    actOptionsEnableUseProfiler = new QAction(icon(""), tr("Profiler enabled"), this);
    actOptionsEnableUseProfiler->setCheckable(true);
    actOptionsEnableUseProfiler->setChecked(settings.value("PythonEditorWidget/UseProfiler", false).toBool());
    connect(actOptionsEnableUseProfiler, SIGNAL(triggered()), this, SLOT(doOptionsEnableUseProfiler()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    actHelp->setEnabled(false);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpKeywordList = new QAction(icon("help-contents"), tr("&Keyword List"), this);
    actHelpKeywordList->setShortcut(QKeySequence::HelpContents);
    actHelpKeywordList->setEnabled(false);
    connect(actHelpKeywordList, SIGNAL(triggered()), this, SLOT(doHelpKeywordList()));

    actAbout = new QAction(icon("about"), tr("About &PythonLab"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void PythonEditorDialog::createControls()
{
    mnuRecentFiles = new QMenu(tr("&Recent files"), this);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actFileNew);
    mnuFile->addAction(actFileOpen);
    mnuFile->addAction(actFileSave);
    mnuFile->addAction(actFileSaveAs);
    mnuFile->addAction(actFileSaveConsoleAs);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addAction(actFileClose);
    mnuFile->addSeparator();
    mnuFile->addAction(actFilePrint);
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);

    mnuEdit = menuBar()->addMenu(tr("&Edit"));
    mnuEdit->addAction(actUndo);
    mnuEdit->addAction(actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCut);
    mnuEdit->addAction(actCopy);
    mnuEdit->addAction(actPaste);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actFind);
    mnuEdit->addAction(actFindNext);
    mnuEdit->addAction(actReplace);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actIndentSelection);
    mnuEdit->addAction(actUnindentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCommentAndUncommentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actGotoLine);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actRunPython);
    mnuTools->addAction(actStopPython);
    mnuTools->addAction(actCheckPyLint);
    mnuTools->addSeparator();
    mnuTools->addAction(actReplaceTabsWithSpaces);

    mnuOptions = menuBar()->addMenu(tr("&Options"));
    mnuOptions->addAction(actOptionsEnablePyFlakes);
    mnuOptions->addAction(actOptionsEnablePyLint);
    mnuOptions->addSeparator();
    mnuOptions->addAction(actOptionsPrintStacktrace);
    mnuOptions->addSeparator();
    mnuOptions->addAction(actOptionsEnableUseProfiler);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    // mnuHelp->addAction(actHelp);
    // mnuHelp->addAction(actHelpKeywordList);
    mnuHelp->addAction(actAbout);   // will be added to "PythonLab" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "PythonLab" MacOSX menu

#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    tlbFile = addToolBar(tr("File"));
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbFile->setObjectName("File");
    tlbFile->addAction(actFileNew);
    tlbFile->addAction(actFileOpen);
    tlbFile->addAction(actFileSave);

    tlbEdit = addToolBar(tr("Edit"));
#ifdef Q_WS_MAC
    tlbEdit->setFixedHeight(iconHeight);
    tlbEdit->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbEdit->setObjectName("Edit");
    tlbEdit->addAction(actUndo);
    tlbEdit->addAction(actRedo);
    tlbEdit->addSeparator();
    tlbEdit->addAction(actCut);
    tlbEdit->addAction(actCopy);
    tlbEdit->addAction(actPaste);

    tlbRun = addToolBar(tr("Run"));
#ifdef Q_WS_MAC
    tlbRun->setFixedHeight(iconHeight);
    tlbRun->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbRun->setObjectName("Run");
    tlbRun->addAction(actRunPython);
    tlbRun->addAction(actStopPython);

    tlbTools = addToolBar(tr("Tools"));
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->setObjectName("Tools");
    tlbTools->addAction(actCheckPyLint);

    // path
    QLabel *lblPath = new QLabel();
    lblPath->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPushButton *btnPath = new QPushButton(icon("three-dots"), "");
    btnPath->setMaximumSize(btnPath->sizeHint());

    connect(btnPath, SIGNAL(clicked()), this, SLOT(doPathChangeDir()));
    connect(fileBrowser, SIGNAL(directoryChanged(QString)), lblPath, SLOT(setText(QString)));

    QToolBar *tlbPath = addToolBar(tr("Path"));
#ifdef Q_WS_MAC
    tlbPath->setFixedHeight(iconHeight);
    tlbPath->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbPath->setObjectName("Path");
    tlbPath->addWidget(new QLabel(tr("Working directory: ")));
    tlbPath->addWidget(lblPath);
    tlbPath->addWidget(btnPath);

    // contents
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);

    QToolButton *btnNewTab = new QToolButton(this);
    btnNewTab->setAutoRaise(true);
    btnNewTab->setToolTip(tr("Add new document"));
    btnNewTab->setIcon(icon("tabadd"));
    btnNewTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tabWidget->setCornerWidget(btnNewTab, Qt::TopLeftCorner);
    connect(btnNewTab, SIGNAL(clicked()), this, SLOT(doFileNew()));

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(doCloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(doCurrentPageChanged(int)));

    // main widget
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(tabWidget);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    setCentralWidget(widget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(doDataChanged()));

    // new file
    doFileNew();
    // recent files
    setRecentFiles();
}

void PythonEditorDialog::createViews()
{
    // file browser
    fileBrowser = new FileBrowser(this);
    fileBrowser->setNameFilter("*.py");
    connect(fileBrowser, SIGNAL(fileItemDoubleClick(QString)), this, SLOT(doFileItemDoubleClick(QString)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(fileBrowser);
    layout->setContentsMargins(0, 0, 0, 7);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    fileBrowserView = new QDockWidget(tr("File browser"), this);
    fileBrowserView->setObjectName("ScriptEditorFileBrowserView");
    fileBrowserView->setWidget(widget);
    fileBrowserView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, fileBrowserView);

    consoleView = new PythonScriptingConsoleView(pythonEngine, this);
    consoleView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, consoleView);

    consoleHistoryView = new PythonScriptingHistoryView(consoleView->console(), this);
    consoleHistoryView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, consoleHistoryView);

    variablesView = new PythonBrowserView(pythonEngine, consoleView->console(), this);
    variablesView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, variablesView);
}

void PythonEditorDialog::createStatusBar()
{
    lblCurrentPosition = new QLabel(statusBar());

    statusBar()->showMessage(tr("Ready"));
    statusBar()->addPermanentWidget(lblCurrentPosition);
}

void PythonEditorDialog::doRunPython()
{
    if (pythonEngine->isScriptRunning())
        return;

    if (!scriptEditorWidget()->fileName().isEmpty())
        fileBrowser->setDir(QFileInfo(scriptEditorWidget()->fileName()).absolutePath());

    scriptPrepare();

    // connect stdout and set current path
    consoleView->console()->connectStdOut(QFile::exists(scriptEditorWidget()->fileName()) ?
                                              QFileInfo(scriptEditorWidget()->fileName()).absolutePath() : "");

    // run script
    QTime time;
    time.start();
    consoleView->console()->consoleMessage(tr("Run script: %1\n").arg(tabWidget->tabText(tabWidget->currentIndex()).replace("* ", "")), Qt::gray);

    bool successfulRun = false;
    if (txtEditor->textCursor().hasSelection())
    {
        successfulRun = pythonEngine->runScript(txtEditor->textCursor().selectedText().replace(0x2029, "\n"), "");
    }
    else if (scriptEditorWidget()->fileName().isEmpty())
    {
        successfulRun = pythonEngine->runScript(txtEditor->toPlainText());
    }
    else
    {
        if (!scriptEditorWidget()->fileName().isEmpty() &&
                QFile::exists(scriptEditorWidget()->fileName()))
            doFileSave();

        // set profiler
        QSettings settings;
        bool useProfiler = settings.value("PythonEditorWidget/UseProfiler", false).toBool();

        successfulRun = pythonEngine->runScript(txtEditor->toPlainText(),
                                                QFileInfo(scriptEditorWidget()->fileName()).absoluteFilePath(),
                                                useProfiler);

        // set profiled
        txtEditor->setProfiled(useProfiler);

        txtEditor->setProfilerAccumulatedLines(currentPythonEngine()->profilerAccumulatedLines());
        txtEditor->setProfilerAccumulatedTimes(currentPythonEngine()->profilerAccumulatedTimes());

        txtEditor->setProfilerMaxAccumulatedLine(currentPythonEngine()->profilerMaxAccumulatedLine());
        txtEditor->setProfilerMaxAccumulatedTime(currentPythonEngine()->profilerMaxAccumulatedTime());
        txtEditor->setProfilerMaxAccumulatedCallLine(currentPythonEngine()->profilerMaxAccumulatedCallLine());
        txtEditor->setProfilerMaxAccumulatedCall(currentPythonEngine()->profilerMaxAccumulatedCall());

        // refresh
        txtEditor->updateLineNumberAreaWidth(0);
    }

    // run script
    consoleView->console()->consoleMessage(tr("Finish script: %1\n").arg(milisecondsToTime(time.elapsed()).toString("hh:mm:ss.zzz")), Qt::gray);

    // disconnect stdout
    consoleView->console()->disconnectStdOut();

    if (!successfulRun)
    {
        // parse error
        ErrorResult result = pythonEngine->parseError();

        consoleView->console()->stdErr(result.error());

        QSettings settings;
        if (settings.value("PythonEditorWidget/PrintStacktrace", true).toBool())
        {
            consoleView->console()->stdErr("\nStacktrace:\n");
            consoleView->console()->stdErr(result.traceback());
        }

        if (!txtEditor->textCursor().hasSelection() && result.line() >= 0)
            txtEditor->gotoLine(result.line(), true);
    }
    consoleView->console()->appendCommandPrompt();

    scriptFinish();
}

void PythonEditorDialog::doStopScript()
{
    actStopPython->setEnabled(false);

    // run script
    consoleView->console()->consoleMessage(tr("\nScript is being aborted.\n"), Qt::blue);

    currentPythonEngine()->abortScript();
    QApplication::processEvents();
}

void PythonEditorDialog::doStartedScript()
{
    // disable controls
    setEnabledControls(false);
    scriptEditorWidget()->setCursor(Qt::BusyCursor);

    actRunPython->setEnabled(false);
    actStopPython->setEnabled(true);

    // QApplication::processEvents();
}

void PythonEditorDialog::doExecutedScript()
{
    // enable controls
    setEnabledControls(true);
    scriptEditorWidget()->setCursor(Qt::ArrowCursor);

    actRunPython->setEnabled(true);
    actStopPython->setEnabled(false);

    txtEditor->setFocus();
    activateWindow();
}

void PythonEditorDialog::setEnabledControls(bool state)
{
    tlbFile->setEnabled(state);
    tlbEdit->setEnabled(state);
    tlbTools->setEnabled(state);

    txtEditor->setEnabled(state);
    consoleView->setEnabled(state);
    consoleHistoryView->setEnabled(state);
    consoleHistoryView->setEnabled(state);
    variablesView->setEnabled(state);
    fileBrowserView->setEnabled(state);
}

void PythonEditorDialog::doReplaceTabsWithSpaces()
{
    txtEditor->replaceTabsWithSpaces();
}

void PythonEditorDialog::doPyLintPython()
{
    if (!scriptEditorWidget()->fileName().isEmpty())
        fileBrowser->setDir(QFileInfo(scriptEditorWidget()->fileName()).absolutePath());

    // analyse by pylint
    scriptEditorWidget()->pyLintAnalyse();

    txtEditor->setFocus();
    activateWindow();
}

void PythonEditorDialog::doOptionsPrintStacktrace()
{
    QSettings settings;
    settings.setValue("PythonEditorWidget/PrintStacktrace", actOptionsPrintStacktrace->isChecked());
}

void PythonEditorDialog::doOptionsEnablePyFlakes()
{
    QSettings settings;
    settings.setValue("PythonEditorWidget/EnablePyFlakes", actOptionsEnablePyFlakes->isChecked());
}

void PythonEditorDialog::doOptionsEnablePyLint()
{
    actCheckPyLint->setEnabled(actOptionsEnablePyLint->isChecked());

    QSettings settings;
    settings.setValue("PythonEditorWidget/EnablePyLint", actOptionsEnablePyLint->isChecked());
}

void PythonEditorDialog::doOptionsEnableUseProfiler()
{
    QSettings settings;
    settings.setValue("PythonEditorWidget/UseProfiler", actOptionsEnableUseProfiler->isChecked());

    // refresh
    txtEditor->setPlainText(txtEditor->toPlainText());
}

void PythonEditorDialog::doFileItemDoubleClick(const QString &path)
{
    QFileInfo fileInfo(path);

    QSettings settings;
    if (QDir(path).exists())
        settings.setValue("PythonEditorDialog/WorkDir", path);
    else
    {
        settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());

        if (fileInfo.suffix() == "py")
            doFileOpen(fileInfo.absoluteFilePath());
    }
}

void PythonEditorDialog::doPathChangeDir()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select directory"), fileBrowser->basePath(), options);
    if (!directory.isEmpty())
        fileBrowser->setDir(directory);
}

void PythonEditorDialog::doFileNew()
{
    tabWidget->addTab(new PythonEditorWidget(pythonEngine, this), tr("Untitled"));
    tabWidget->setCurrentIndex(tabWidget->count()-1);
    doCurrentPageChanged(tabWidget->count()-1);
}

void PythonEditorDialog::doFileOpen(const QString &file)
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    // open dialog
    QString fileName = file;
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), dir, tr("Python scripts (*.py)"));

    // read text
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "py")
            return;

        PythonEditorWidget *scriptEditor = scriptEditorWidget();

        for (int i = 0; i < tabWidget->count(); i++)
        {
            PythonEditorWidget *scriptEditorWidgetTmp = dynamic_cast<PythonEditorWidget *>(tabWidget->widget(i));
            if (scriptEditorWidgetTmp->fileName() == fileName)
            {
                tabWidget->setCurrentIndex(i);
                QMessageBox::information(this, tr("Information"), tr("Script is already opened."));
                return;
            }
        }

        // check empty document
        if (!scriptEditor->txtEditor->toPlainText().isEmpty())
        {
            doFileNew();
            // new widget
            scriptEditor = scriptEditorWidget();
        }

        scriptEditor->setFileName(fileName);
        txtEditor->setPlainText(readFileContent(scriptEditor->fileName()));

        setRecentFiles();

        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());

        doCurrentPageChanged(tabWidget->currentIndex());

        if (fileInfo.absoluteDir() != tempProblemDir() && !fileName.contains("resources/examples"))
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorDialog::doFileOpenRecent(QAction *action)
{
    QString fileName = action->text();
    if (QFile::exists(fileName))
    {
        doFileOpen(fileName);
        setRecentFiles();
    }
}

void PythonEditorDialog::doFileSave()
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    // save dialog
    if (scriptEditorWidget()->fileName().isEmpty())
        scriptEditorWidget()->setFileName(QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python scripts (*.py)")));

    // write text
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        if (fileInfo.suffix() != "py") scriptEditorWidget()->fileName() += ".py";

        QFile fileName(scriptEditorWidget()->fileName());
        if (fileName.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&fileName);
            out << txtEditor->toPlainText();
            fileName.close();

            setRecentFiles();

            tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
            txtEditor->document()->setModified(false);
        }
        else
        {
            // throw AgrosException(tr("File '%1' cannot be saved.").arg(scriptEditorWidget()->fileName));
            qDebug() << tr("File '%1' cannot be saved.").arg(scriptEditorWidget()->fileName());
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorDialog::doFileSaveAs()
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python scripts (*.py)"));
    if (!fileName.isEmpty())
    {
        if (QFileInfo(fileName).suffix() != "py") fileName += ".py";

        scriptEditorWidget()->setFileName(fileName);
        doFileSave();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorDialog::doFileSaveConsoleAs()
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Html files (*.html)"));
    if (!fileName.isEmpty())
    {
        if (QFileInfo(fileName).suffix() == "html" || QFileInfo(fileName).suffix() == "htm")
            ;
        else
            fileName += ".html";

        QString str = consoleView->console()->document()->toHtml();
        writeStringContent(fileName, &str);

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorDialog::doFileClose()
{
    doCloseTab(tabWidget->currentIndex());
}

void PythonEditorDialog::doFilePrint()
{
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);
    printDialog.addEnabledOption(QAbstractPrintDialog::PrintCollateCopies);
    printDialog.setWindowTitle(tr("Print Document"));
    if (printDialog.exec() == QDialog::Accepted)
    {
        txtEditor->print(&printer);
    }
}

void PythonEditorDialog::doFind()
{
    QTextCursor cursor = txtEditor->textCursor();
    scriptEditorWidget()->searchWidget->showFind(cursor.selectedText());
}

void PythonEditorDialog::doFindNext(bool fromBegining)
{
    scriptEditorWidget()->searchWidget->findNext(false);
}

void PythonEditorDialog::doReplace()
{
    QTextCursor cursor = txtEditor->textCursor();
    scriptEditorWidget()->searchWidget->showReplaceAll(cursor.selectedText());
}

void PythonEditorDialog::doDataChanged()
{
    actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void PythonEditorDialog::doHelp()
{
    showPage("scripting/commands.html");
}

void PythonEditorDialog::doHelpKeywordList()
{
    showPage("scripting/keyword_list.html");
}

void PythonEditorDialog::doAbout()
{
    AboutDialog about(this);
    about.exec();
}

void PythonEditorDialog::onOtherInstanceMessage(const QString &msg)
{
    QStringList args = msg.split("#!#");
    for (int i = 1; i < args.count()-1; i++)
    {
        QString fileName =
                QFile::exists(args[i]) ? args[i] : QApplication::applicationDirPath() + QDir::separator() + args[i];

        if (QFile::exists(fileName))
            doFileOpen(fileName);
    }

    // setWindowState(Qt::WindowMinimized);
    // setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    show();
}

void PythonEditorDialog::doCloseTab(int index)
{
    tabWidget->setCurrentIndex(index);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        fileName = fileInfo.completeBaseName();
    }

    while (txtEditor->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"), tr("File '%1' has been modified.\nDo you want to save your changes?").arg(fileName),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            doFileSave();
        else if (ret == QMessageBox::Discard)
            break;
        else if (ret == QMessageBox::Cancel)
            return;
    }

    if (tabWidget->count() == 1)
    {
        doFileNew();
    }

    tabWidget->removeTab(index);
}

void PythonEditorDialog::doCurrentPageChanged(int index)
{
    txtEditor = scriptEditorWidget()->txtEditor;

    actCut->disconnect();
    connect(actCut, SIGNAL(triggered()), txtEditor, SLOT(cut()));
    actCopy->disconnect();
    connect(actCopy, SIGNAL(triggered()), txtEditor, SLOT(copy()));
    actPaste->disconnect();
    connect(actPaste, SIGNAL(triggered()), txtEditor, SLOT(paste()));
    actUndo->disconnect();
    connect(actUndo, SIGNAL(triggered()), txtEditor, SLOT(undo()));
    actRedo->disconnect();
    connect(actRedo, SIGNAL(triggered()), txtEditor, SLOT(redo()));

    actIndentSelection->disconnect();
    connect(actIndentSelection, SIGNAL(triggered()), txtEditor, SLOT(indentSelection()));
    actUnindentSelection->disconnect();
    connect(actUnindentSelection, SIGNAL(triggered()), txtEditor, SLOT(unindentSelection()));
    actCommentAndUncommentSelection->disconnect();
    connect(actCommentAndUncommentSelection, SIGNAL(triggered()), txtEditor, SLOT(commentAndUncommentSelection()));
    actGotoLine->disconnect();
    connect(actGotoLine, SIGNAL(triggered()), txtEditor, SLOT(gotoLine()));

    txtEditor->document()->disconnect(actUndo);
    txtEditor->document()->disconnect(actRedo);
    connect(txtEditor->document(), SIGNAL(undoAvailable(bool)), actUndo, SLOT(setEnabled(bool)));
    connect(txtEditor->document(), SIGNAL(redoAvailable(bool)), actRedo, SLOT(setEnabled(bool)));
    txtEditor->disconnect(actCut);
    txtEditor->disconnect(actCopy);
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCut, SLOT(setEnabled(bool)));
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCopy, SLOT(setEnabled(bool)));

    // modifications
    connect(txtEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(doCurrentDocumentChanged(bool)));

    // line number
    connect(txtEditor, SIGNAL(cursorPositionChanged()), this, SLOT(doCursorPositionChanged()));
    doCursorPositionChanged();

    actUndo->setEnabled(txtEditor->document()->isUndoAvailable());
    actRedo->setEnabled(txtEditor->document()->isRedoAvailable());

    // tabWidget->setTabsClosable(tabWidget->count() > 1);
    tabWidget->setTabsClosable(true);
    tabWidget->cornerWidget(Qt::TopLeftCorner)->setEnabled(true);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        fileName = fileInfo.completeBaseName();
    }
    setWindowTitle(tr("PythonLab - %1").arg(fileName));

    txtEditor->setFocus();
}

void PythonEditorDialog::doCursorPositionChanged()
{
    QTextCursor cur(txtEditor->textCursor());
    lblCurrentPosition->setText(tr("Line: %1, Col: %2").arg(cur.blockNumber()+1)
                                .arg(cur.columnNumber()+1));
}

void PythonEditorDialog::doCurrentDocumentChanged(bool changed)
{
    // modified
    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        fileName = fileInfo.completeBaseName();
    }

    if (changed)
        tabWidget->setTabText(tabWidget->currentIndex(), QString("* %1").arg(fileName));
    else
        tabWidget->setTabText(tabWidget->currentIndex(), fileName);
}

void PythonEditorDialog::setRecentFiles()
{
    if (!tabWidget) return;

    // recent files
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        if (m_recentFiles.indexOf(fileInfo.absoluteFilePath()) == -1)
            m_recentFiles.insert(0, fileInfo.absoluteFilePath());
        else
            m_recentFiles.move(m_recentFiles.indexOf(fileInfo.absoluteFilePath()), 0);

        while (m_recentFiles.count() > 15) m_recentFiles.removeLast();
    }

    mnuRecentFiles->clear();
    for (int i = 0; i<m_recentFiles.count(); i++)
    {
        QAction *actMenuRecentItem = new QAction(m_recentFiles[i], this);
        actFileOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}

void PythonEditorDialog::closeTabs()
{
    for (int i = tabWidget->count()-1; i >= 0 ; i--)
        doCloseTab(i);
}

bool PythonEditorDialog::isScriptModified()
{
    return txtEditor->document()->isModified();
}

// ********************************************************************************

ScriptEditor::ScriptEditor(PythonEngine *pythonEngine, QWidget *parent)
    : PlainTextEditParenthesis(parent), pythonEngine(pythonEngine), m_isProfiled(false)
{
    lineNumberArea = new ScriptEditorLineNumberArea(this);

    setFont(FONT);
    setTabStopWidth(fontMetrics().width(TABS));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    // highlighter
    new PythonHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    completer = new PythonCompleter();
    completer->setWidget(this);
    connect(completer, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
}

ScriptEditor::~ScriptEditor()
{
    delete completer;
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        lineNumberArea->scroll(0, dy);
    }
    else
    {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void ScriptEditor::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textCursor();
    int oldPos = cursor.position();
    int indent = firstNonSpace(cursor.block().text());

    if (completer && completer->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:

            event->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    if (event->key() == Qt::Key_Tab && !(event->modifiers() & Qt::ShiftModifier))
    {
        if (!textCursor().hasSelection())
        {
            // insert 4 spaces instead of tab
            textCursor().insertText(QString(4, ' '));
        }
        else
        {
            // indent the selection
            indentSelection();
        }
    }
    else if (event->key() == Qt::Key_Backtab && (event->modifiers() & Qt::ShiftModifier))
    {
        if (!textCursor().hasSelection())
        {
            // moves position backward 4 spaces
            QTextCursor cursor = textCursor();
            cursor.setPosition(cursor.position() - 4, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
        }
        else
        {
            // unindent the selection
            unindentSelection();
        }
    }
    else if ((event->key() == Qt::Key_Backspace) && (document()->characterAt(oldPos - 1) == ' ')
             && (document()->characterAt(oldPos - 2) == ' ')
             && (document()->characterAt(oldPos - 3) == ' ')
             && (document()->characterAt(oldPos - 4) == ' '))
    {
        cursor.beginEditBlock();
        // determine selection to delete
        int newPos = oldPos - 4;
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        int startPosOfLine = cursor.position();
        if (newPos < startPosOfLine)
            newPos = startPosOfLine;
        // make selection
        cursor.setPosition(oldPos, QTextCursor::MoveAnchor);
        cursor.setPosition(newPos, QTextCursor::KeepAnchor);
        cursor.deleteChar();
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
    else if ((event->key() == Qt::Key_Return) && (indent))
    {
        cursor.beginEditBlock();

        // add 1 extra indent if current line begins a code block
        bool inCodeBlock = false;
        if (QRegExp("\\:").indexIn(cursor.block().text()) != -1)
        {
            indent += TABS_SIZE;
            inCodeBlock = true;
        }

        cursor.insertBlock();
        QString spaces(indent, true ? QLatin1Char(' ') : QLatin1Char('\t'));
        cursor.insertText(spaces);

        cursor.endEditBlock();
        setTextCursor(cursor);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }

    if ((event->key() == Qt::Key_Space && event->modifiers() & Qt::ControlModifier)
            || completer->popup()->isVisible())
    {
        QTextCursor tc = textCursor();
        // tc.select(QTextCursor::WordUnderCursor);
        // QString textToComplete = tc.selectedText();
        QString textToComplete = textCursor().block().text().trimmed();

        QStringList found = pythonEngine->codeCompletionScript(toPlainText(),
                                                               tc.blockNumber() + 1,
                                                               tc.columnNumber() + 1);

        // experimental - intelligent text completion
        if (false)
        {
            foreach (QString row, toPlainText().split("\n"))
            {
                // remove comment
                if (int index = row.indexOf('#'))
                {
                    if (index > 0)
                    {
                        qDebug() << index;
                        row = row.left(index);
                    }
                    else
                    {
                        continue;
                    }
                }

                QRegExp rx("^[.,]*$");
                QStringList strs = row.split(rx);
                foreach (QString str, strs)
                {
                    if (!str.isEmpty() && !found.contains(str))
                    {
                        qDebug() << str;
                        found.append(str);
                    }
                }

                /*
                QRegExp rx("(^[a-zA-Z0-9_]+$)");
                int pos = 0;

                while ((pos = rx.indexIn(row, pos)) != -1)
                {
                    QString str = rx.cap(1);
                    if (!found.contains(str))
                    {
                        qDebug() << str;
                        found.append(str);
                    }
                    pos += rx.matchedLength();
                }
                */
            }
        }

        foreach (QString str, found)
        {
            // qDebug() << str;
        }

        if (!found.isEmpty())
        {
            // completer->setCompletionPrefix(textToComplete);
            completer->setModel(new QStringListModel(found, completer));
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::StartOfWord);
            QRect cr = cursorRect(c);
            cr.setWidth(completer->popup()->sizeHintForColumn(0)
                        + completer->popup()->verticalScrollBar()->sizeHint().width() + 30);
            cr.translate(lineNumberAreaWidth(), 4);
            completer->complete(cr);
        }
        else
        {
            completer->popup()->hide();
        }
    }
}

void ScriptEditor::indentSelection()
{
    indentAndUnindentSelection(true);
}

void ScriptEditor::unindentSelection()
{
    indentAndUnindentSelection(false);
}

void ScriptEditor::indentAndUnindentSelection(bool doIndent)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // indent or unindent the selected lines
    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    QTextDocument *doc = document();
    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end-1).next();

    for (QTextBlock block = startBlock; block != endBlock; block = block.next())
    {
        QString text = block.text();
        if (doIndent)
        {
            int indentPosition = firstNonSpace(text);
            cursor.setPosition(block.position() + indentPosition);
            cursor.insertText(QString(TABS_SIZE, ' '));
        }
        else
        {
            int indentPosition = firstNonSpace(text);
            int targetColumn = indentedColumn(columnAt(text, indentPosition), false);
            cursor.setPosition(block.position() + indentPosition);
            cursor.setPosition(block.position() + targetColumn, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
    }

    // reselect the selected lines
    cursor.setPosition(startBlock.position());
    cursor.setPosition(endBlock.previous().position(), QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void ScriptEditor::commentAndUncommentSelection()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // previous selection state
    int selStart = cursor.selectionStart();
    int selEnd = cursor.selectionEnd();
    cursor.setPosition(selEnd, QTextCursor::MoveAnchor);
    int blockEnd = cursor.blockNumber();

    // extend selStart to first blocks's start-of-block
    // extend selEnd to last block's end-of-block
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    selStart = cursor.position();
    cursor.setPosition(selEnd, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    selEnd = cursor.position();

    // process first block
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    QRegExp commentPattern("^#");
    if (commentPattern.indexIn(cursor.block().text()) == -1)
    {
        // comment it, if the block does not starts with '#'
        cursor.insertText("#");
        selEnd += 1;
    }
    else
    {
        // else uncomment it
        cursor.setPosition(selStart + commentPattern.matchedLength(), QTextCursor::KeepAnchor);
        cursor.deleteChar();
        selEnd -= 1;
    }

    // loop through all blocks
    while (cursor.blockNumber() < blockEnd)
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        if (commentPattern.indexIn(cursor.block().text()) == -1)
        {
            cursor.insertText("#");
            selEnd += 1;
        }
        else
        {
            cursor.setPosition(cursor.position() + commentPattern.matchedLength(), QTextCursor::KeepAnchor);
            cursor.deleteChar();
            selEnd -= 1;
        }
    }

    // restore selection state
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    cursor.setPosition(selEnd, QTextCursor::KeepAnchor);

    // update
    cursor.endEditBlock();
    setTextCursor(cursor);
}

void ScriptEditor::gotoLine(int line, bool isError)
{
    // use dialog when (line == -1)
    if (line == -1)
    {
        bool ok;
        int lineDialog = QInputDialog::getInt(this, tr("Goto line"), tr("Line number:"),
                                              0, 1, document()->blockCount(), 1, &ok);
        if (ok)
            line = lineDialog;
    }

    if (line >= 0 && line <= document()->blockCount())
    {
        int pos = document()->findBlockByNumber(line - 1).position();
        QTextCursor cur = textCursor();
        cur.setPosition(pos, QTextCursor::MoveAnchor);
        setTextCursor(cur);
        ensureCursorVisible();
        highlightCurrentLine(true);
        setFocus();
    }
}

void ScriptEditor::highlightCurrentLine(bool isError)
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(180);
        if (isError)
            lineColor = QColor(Qt::red).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);

    matchParentheses('(', ')');
    // matchParentheses('[', ']');
    // matchParentheses('{', '}');
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    // line numbers
    const QBrush bookmarkBrushPyFlakes(QColor(Qt::red).lighter());

    int timesWidth = 0;
    int callWidth = 0;
    if (isProfiled())
    {
        timesWidth = fontMetrics().width(QLatin1Char('9')) * QString::number(profilerMaxAccumulatedTime()).length() + 1;
        callWidth = fontMetrics().width(QLatin1Char('9')) * QString::number(profilerMaxAccumulatedCall()).length() + 1;
    }

    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            // line number
            QString lineNumber = QString::number(blockNumber + 1);

            // draw rect
            if (errorMessagesPyFlakes.contains(blockNumber + 1))
                painter.fillRect(0, top, lineNumberArea->width(), fontMetrics().height(),
                                 bookmarkBrushPyFlakes);

            // draw line number
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, lineNumber);

            // draw profiler number
            if (isProfiled())
            {
                if (profilerAccumulatedTimes().value(blockNumber + 1) > 0)
                {
                    QString number = QString::number(profilerAccumulatedTimes().value(blockNumber + 1));
                    painter.setPen(Qt::darkBlue);
                    painter.drawText(0, top, timesWidth,
                                     fontMetrics().height(),
                                     Qt::AlignRight, number);

                    number = QString::number(profilerAccumulatedLines().value(blockNumber + 1));
                    painter.setPen(Qt::darkGreen);
                    painter.drawText(0, top, timesWidth + callWidth + 3, fontMetrics().height(),
                                     Qt::AlignRight, number);
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void ScriptEditor::lineNumberAreaMouseMoveEvent(QMouseEvent *event)
{
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();

    int line = blockNumber + event->pos().y() / (int) blockBoundingRect(block).height() + 1;

    if (line <= document()->blockCount())
    {
        if (errorMessagesPyFlakes.contains(line))
            QToolTip::showText(event->globalPos(), errorMessagesPyFlakes[line]);
    }
}

int ScriptEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    if (isProfiled())
    {
        digits += QString::number(profilerMaxAccumulatedTime()).length() +
                QString::number(profilerMaxAccumulatedCall()).length();
    }

    int space = 15 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void ScriptEditor::insertCompletion(const QString& completion)
{
    QString str = completion.left(completion.indexOf("(") - 1);

    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    // if (tc.selectedText() == ".")
    // {
    //     tc.insertText(QString(".") + str);
    // }
    // else
    {
        tc = textCursor();
        tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        tc.insertText(str);
        setTextCursor(tc);
    }
}

void ScriptEditor::replaceTabsWithSpaces()
{
    QString text = document()->toPlainText();
    text = text.replace("\t", TABS);
    document()->setPlainText(text);
}

// ********************************************************************************************************

SearchWidget::SearchWidget(ScriptEditor *txtEditor, QWidget *parent)
    : QWidget(parent), txtEditor(txtEditor)
{
    lblFind = new QLabel(tr("Search for:"));
    lblReplace = new QLabel(tr("Replace with:"));

    txtFind = new QLineEdit();
    connect(txtFind, SIGNAL(returnPressed()), this, SLOT(find()));
    txtReplace = new QLineEdit();
    connect(txtReplace, SIGNAL(returnPressed()), this, SLOT(replaceAll()));

    btnFind = new QPushButton(tr("Find"), this);
    btnFind->setDefault(true);
    connect(btnFind, SIGNAL(clicked()), this, SLOT(find()));

    btnReplace = new QPushButton(tr("Replace all"), this);
    connect(btnReplace, SIGNAL(clicked()), this, SLOT(replaceAll()));

    btnHide = new QPushButton(tr("Hide"), this);
    connect(btnHide, SIGNAL(clicked()), this, SLOT(hideWidget()));

    QGridLayout *findReplaceLayout = new QGridLayout();
    findReplaceLayout->setMargin(2);

    findReplaceLayout->addWidget(lblFind, 0, 0);
    findReplaceLayout->addWidget(txtFind, 0, 1);
    findReplaceLayout->addWidget(btnFind, 0, 2);
    findReplaceLayout->addWidget(btnHide, 0, 3);
    findReplaceLayout->addWidget(lblReplace, 1, 0);
    findReplaceLayout->addWidget(txtReplace, 1, 1);
    findReplaceLayout->addWidget(btnReplace, 1, 2);

    setLayout(findReplaceLayout);

    lblReplace->setVisible(false);
    txtReplace->setVisible(false);
    btnReplace->setVisible(false);

    setVisible(false);
}

void SearchWidget::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    switch (key)
    {
    case Qt::Key_Escape:
        hideWidget();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void SearchWidget::showFind(const QString &text)
{
    if (!text.isEmpty())
        txtFind->setText(text);

    txtFind->setFocus();
    txtFind->selectAll();
    lblReplace->setVisible(false);
    txtReplace->setVisible(false);
    btnReplace->setVisible(false);

    startFromBeginning = true;

    show();
}

void SearchWidget::showReplaceAll(const QString &text)
{
    if (!text.isEmpty())
        txtFind->setText(text);

    txtFind->setFocus();
    txtFind->selectAll();
    lblReplace->setVisible(true);
    txtReplace->setVisible(true);
    btnReplace->setVisible(true);

    show();
}

void SearchWidget::find()
{
    findNext(startFromBeginning);
    startFromBeginning = false;
}

void SearchWidget::findNext(bool fromBegining)
{
    if (!txtFind->text().isEmpty())
    {
        // Search
        QTextCursor cursor = txtEditor->textCursor();
        if (fromBegining)
            cursor = txtEditor->document()->find(txtFind->text());
        else
            cursor = txtEditor->document()->find(txtFind->text(), cursor);

        if (cursor.position() >= 0)
            txtEditor->setTextCursor(cursor);
        txtEditor->setFocus();

        if (isVisible())
            txtFind->setFocus();
    }
}

void SearchWidget::replaceAll()
{
    if (!txtFind->text().isEmpty())
    {
        QTextCursor cursor = txtEditor->textCursor();

        QString text = txtEditor->document()->toPlainText();
        text.replace(txtFind->text(), txtReplace->text());
        txtEditor->document()->setPlainText(text);

        txtEditor->setTextCursor(cursor);

        hideWidget();
    }
}

void SearchWidget::hideWidget()
{
    hide();
    txtEditor->setFocus();
}
