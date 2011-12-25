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

#include "util.h"
#include "gui.h"

PythonEditorWidget::PythonEditorWidget(PythonEngine *pythonEngine, QWidget *parent)
    : QWidget(parent), pythonEngine(pythonEngine)
{
    file = "";

    createControls();

    QSettings settings;
    if (settings.value("PythonEditorWidget/EnablePyFlakes", true).toBool())
    {
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(pyFlakesAnalyse()));
        timer->start(4000);
    }

    txtEditor->setAcceptDrops(false);
}

PythonEditorWidget::~PythonEditorWidget()
{
    QSettings settings;
    settings.setValue("PythonEditorWidget/SplitterState", splitter->saveState());
    settings.setValue("PythonEditorWidget/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("PythonEditorWidget/EditorHeight", txtEditor->height());
}

void PythonEditorWidget::createControls()
{
    txtEditor = new ScriptEditor(this);
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
    if (settings.value("PythonEditorWidget/EnablePyLint", true).toBool())
    {
        trvPyLint = new QTreeWidget(this);
        trvPyLint->setHeaderHidden(true);
        trvPyLint->setMouseTracking(true);
        trvPyLint->setColumnCount(1);
        trvPyLint->setIndentation(12);
        connect(trvPyLint, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doHighlightLine(QTreeWidgetItem *, int)));

        splitter->addWidget(trvPyLint);
    }

    QSizePolicy policy = splitter->sizePolicy();
    policy.setHorizontalStretch(0.2);
    policy.setVerticalStretch(1.0);
    splitter->setSizePolicy(policy);

    splitter->restoreState(settings.value("PythonEditorWidget/SplitterState", splitter->saveState()).toByteArray());
    splitter->restoreGeometry(settings.value("PythonEditorWidget/SplitterGeometry", splitter->saveGeometry()).toByteArray());
    txtEditor->resize(txtEditor->width(), settings.value("PythonEditorWidget/EditorHeight").toInt());

    // contents
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(1);
    layout->addWidget(splitter);

    setLayout(layout);
}

void PythonEditorWidget::pyLintAnalyse()
{
    trvPyLint->clear();

    QProcess processPyLint;
    processPyLint.setStandardOutputFile(tempProblemFileName() + ".pylint.out");
    processPyLint.setStandardErrorFile(tempProblemFileName() + ".pylint.err");
    connect(&processPyLint, SIGNAL(finished(int)), this, SLOT(pyLintAnalyseStopped(int)));

    QString pylintBinary = "pylint";
    if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "pylint"))
        pylintBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "pylint\"";
    if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "pylint"))
        pylintBinary = QApplication::applicationDirPath() + QDir::separator() + "pylint";

    QString test = txtEditor->toPlainText();
    writeStringContent(tempProblemFileName() + ".pylint.py", &test);

    QStringList arguments;
    arguments << "-i" << "yes" << tempProblemFileName() + ".pylint.py";

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

    QString line;
    do
    {
        line = inOutput.readLine();

        if (!line.isEmpty())
        {
            if (line.startsWith("C") || line.startsWith("W") || line.startsWith("E"))
            {
                QString type;
                QString typeFamily;
                int number;
                QString message;

                QStringList list = line.split(":");
                if (list.count() == 3)
                {
                    type = list[0];
                    number = list[1].toInt();
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
                                  arg(number).
                                  arg(message));

                    item->setData(0, Qt::UserRole, number);
                }
            }
        }
    } while (!line.isNull());

    txtEditor->repaint();

    // QString error = readFileContent(tempProblemFileName() + ".pylint.err");
    // qDebug() << error;
}

void PythonEditorWidget::pyFlakesAnalyse()
{
    if (txtEditor->isVisible() && txtEditor->hasFocus())
    {
        QProcess processPyFlakes;
        processPyFlakes.setStandardOutputFile(tempProblemFileName() + ".pyflakes.out");
        processPyFlakes.setStandardErrorFile(tempProblemFileName() + ".pyflakes.err");
        connect(&processPyFlakes, SIGNAL(finished(int)), this, SLOT(pyFlakesAnalyseStopped(int)));

        QString pyflakesBinary = "pyflakes";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "pyflakes"))
            pyflakesBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "pyflakes";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "pyflakes"))
            pyflakesBinary = QApplication::applicationDirPath() + QDir::separator() + "pyflakes";

        QString test = txtEditor->toPlainText();
        writeStringContent(tempProblemFileName() + ".pyflakes.py", &test);

        QStringList arguments;
        arguments << "-i" << "yes" << tempProblemFileName() + ".pyflakes.py";

        processPyFlakes.start(pyflakesBinary, arguments);

        if (!processPyFlakes.waitForStarted())
        {
            qDebug() << "Could not start PyFlakes: " << processPyFlakes.errorString();

            processPyFlakes.kill();
            return;
        }

        while (!processPyFlakes.waitForFinished()) {}
    }
}

void PythonEditorWidget::pyFlakesAnalyseStopped(int exitCode)
{
    txtEditor->errorMessagesPyFlakes.clear();

    QStringList fileNames;
    fileNames << tempProblemFileName() + ".pyflakes.out"
              << tempProblemFileName() + ".pyflakes.err";

    // read from stdout and stderr
    foreach (QString fileName, fileNames) {
        QFile fileOutput(fileName);
        if (!fileOutput.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << tr("Could not read PyFlakes output.");
            return;
        }
        QTextStream inOutput(&fileOutput);

        QString line;
        do
        {
            line = inOutput.readLine();

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
        } while (!line.isNull());
    }

    txtEditor->repaint();
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

    filBrowser->refresh();

    QSettings settings;

    connect(actRunPython, SIGNAL(triggered()), this, SLOT(doRunPython()));
    if (settings.value("PythonEditorWidget/EnablePyLint", true).toBool())
        connect(actCheckPython, SIGNAL(triggered()), this, SLOT(doPyLintPython()));

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    restoreGeometry(settings.value("PythonEditorDialog/Geometry", saveGeometry()).toByteArray());
    recentFiles = settings.value("PythonEditorDialog/RecentFiles").value<QStringList>();
    restoreState(settings.value("PythonEditorDialog/State", saveState()).toByteArray());

    // parameters
    for (int i = 1; i < args.count(); i++)
    {
        QString fileName =
                QFile::exists(args[i]) ? args[i] : QApplication::applicationDirPath() + QDir::separator() + args[i];

        if (QFile::exists(fileName))
            doFileOpen(fileName);
    }

    setAcceptDrops(true);
}

PythonEditorDialog::~PythonEditorDialog()
{
    QSettings settings;
    settings.setValue("PythonEditorDialog/Geometry", saveGeometry());
    settings.setValue("PythonEditorDialog/State", saveState());
    settings.setValue("PythonEditorDialog/RecentFiles", recentFiles);
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
            doFileOpen(fileName);

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

    actFileSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actFileSaveAs, SIGNAL(triggered()), this, SLOT(doFileSaveAs()));

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

    actCommentSelection = new QAction(icon(""), tr("Comment"), this);
    // actCommentSelection->setShortcut(tr("Ctrl+"));
    actUncommentSelection = new QAction(icon(""), tr("Uncomment"), this);
    actUncommentSelection->setShortcut(tr("Ctrl+U"));

    actGotoLine = new QAction(icon(""), tr("Goto line"), this);
    actGotoLine->setShortcut(tr("Alt+G"));

    actRunPython = new QAction(icon("run"), tr("&Run Python script"), this);
    actRunPython->setShortcut(QKeySequence(tr("Ctrl+R")));

    QSettings settings;
    if (settings.value("PythonEditorWidget/EnablePyLint", true).toBool())
    {
        actCheckPython = new QAction(icon("checkbox"), tr("&Check Python script (PyLint)"), this);
        actCheckPython->setShortcut(QKeySequence(tr("Alt+C")));
    }

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit editor"));
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpKeywordList = new QAction(icon("help-contents"), tr("&Keyword List"), this);
    actHelpKeywordList->setShortcut(QKeySequence::HelpContents);
    connect(actHelpKeywordList, SIGNAL(triggered()), this, SLOT(doHelpKeywordList()));

    actAbout = new QAction(icon("about"), tr("About &PythonLab"), this);
    actAbout->setStatusTip(tr("Show the application's About box"));
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setStatusTip(tr("Show the Qt library's About box"));
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
    mnuEdit->addAction(actCommentSelection);
    mnuEdit->addAction(actUncommentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actGotoLine);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actRunPython);
    QSettings settings;
    if (settings.value("PythonEditorWidget/EnablePyLint", true).toBool())
        mnuTools->addAction(actCheckPython);

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

    QToolBar *tlbEdit = addToolBar(tr("Edit"));
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

    tlbTools = addToolBar(tr("Tools"));
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->setObjectName("Tools");
    tlbTools->addAction(actRunPython);
    if (settings.value("PythonEditorWidget/EnablePyLint", true).toBool())
        tlbTools->addAction(actCheckPython);

    // path
    QLineEdit *txtPath = new QLineEdit(this);
    txtPath->setReadOnly(true);

    QPushButton *btnPath = new QPushButton(icon("three-dots"), "");
    btnPath->setMaximumSize(btnPath->sizeHint());

    connect(btnPath, SIGNAL(clicked()), this, SLOT(doPathChangeDir()));
    connect(filBrowser, SIGNAL(directoryChanged(QString)), txtPath, SLOT(setText(QString)));

    QToolBar *tlbPath = addToolBar(tr("Path"));
#ifdef Q_WS_MAC
    tlbPath->setFixedHeight(iconHeight);
    tlbPath->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbPath->setObjectName("Path");
    tlbPath->addWidget(new QLabel(tr("Working directory: "), this));
    tlbPath->addWidget(txtPath);
    tlbPath->addWidget(btnPath);

    // contents
    tabWidget = new QTabWidget;
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);

    QToolButton *btnNewTab = new QToolButton(this);
    btnNewTab->setAutoRaise(true);
    btnNewTab->setToolTip(tr("Add new document"));
    btnNewTab->setIcon(icon("tabadd"));
    btnNewTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tabWidget->setCornerWidget(btnNewTab, Qt::TopLeftCorner);
    connect(btnNewTab, SIGNAL(clicked()), this, SLOT(doFileNew()));

    doFileNew();

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(doCloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(doCurrentPageChanged(int)));

    // main widget
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(tabWidget);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    // recent files
    setRecentFiles();

    setCentralWidget(widget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(doDataChanged()));
}

void PythonEditorDialog::createViews()
{
    QSettings settings;

    // file browser
    filBrowser = new FileBrowser(this);
    filBrowser->setNameFilter("*.py");
    filBrowser->setDir(settings.value("PythonEditorDialog/WorkDir", datadir()).value<QString>());

    connect(filBrowser, SIGNAL(fileItemDoubleClick(QString)), this, SLOT(doFileItemDoubleClick(QString)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(filBrowser);
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
    addDockWidget(Qt::BottomDockWidgetArea, consoleView);

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
    if (!scriptEditorWidget()->file.isEmpty())
        filBrowser->setDir(QFileInfo(scriptEditorWidget()->file).absolutePath());

    // disable controls
    consoleView->setEnabled(false);
    // actRunPython->setEnabled(false);
    scriptEditorWidget()->setCursor(Qt::BusyCursor);
    QApplication::processEvents();

    // run script
    consoleView->console()->consoleMessage("Run script: " + tabWidget->tabText(tabWidget->currentIndex()).replace("* ", "") + "\n",
                                           Qt::gray);

    // benchmark
    QTime time;
    time.start();

    ScriptResult result;
    if (txtEditor->textCursor().hasSelection())
    {
        result = pythonEngine->runPythonScript(txtEditor->textCursor().selectedText().replace(0x2029, "\n"), "");
    }
    else if (scriptEditorWidget()->file.isEmpty())
    {
        result = pythonEngine->runPythonScript(txtEditor->toPlainText());
    }
    else
    {
        if (!scriptEditorWidget()->file.isEmpty() &&
                QFile::exists(scriptEditorWidget()->file))
            doFileSave();

        result = pythonEngine->runPythonScript(txtEditor->toPlainText(),
                                               QFileInfo(scriptEditorWidget()->file).absoluteFilePath());
    }

    if (result.isError)
    {
        consoleView->console()->stdErr(result.text);

        if (!txtEditor->textCursor().hasSelection() && result.line >= 0)
            txtEditor->gotoLine(result.line, true);
    }
    consoleView->console()->appendCommandPrompt();

    // enable controls
    consoleView->setEnabled(true);
    scriptEditorWidget()->setCursor(Qt::ArrowCursor);
    // actRunPython->setEnabled(true);

    txtEditor->setFocus();
    activateWindow();
}

void PythonEditorDialog::doPyLintPython()
{
    if (!scriptEditorWidget()->file.isEmpty())
        filBrowser->setDir(QFileInfo(scriptEditorWidget()->file).absolutePath());

    // analyse by pylint
    scriptEditorWidget()->pyLintAnalyse();

    txtEditor->setFocus();
    activateWindow();
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
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select directory"), filBrowser->basePath(), options);
    if (!directory.isEmpty())
        filBrowser->setDir(directory);
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
    QString dir = settings.value("General/LastDir", "data").toString();

    // open dialog
    QString fileName = file;
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), dir, tr("Python files (*.py)"));

    // read text
    if (!fileName.isEmpty())
    {
        PythonEditorWidget *scriptEditor = scriptEditorWidget();

        for (int i = 0; i < tabWidget->count(); i++)
        {
            PythonEditorWidget *scriptEditorWidgetTmp = dynamic_cast<PythonEditorWidget *>(tabWidget->widget(i));
            if (scriptEditorWidgetTmp->file == fileName)
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

        scriptEditor->file = fileName;
        txtEditor->setPlainText(readFileContent(scriptEditor->file));

        setRecentFiles();

        QFileInfo fileInfo(scriptEditor->file);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());

        doCurrentPageChanged(tabWidget->currentIndex());

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDir", fileInfo.absolutePath());
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
    QString dir = settings.value("General/LastDir", "data").toString();

    // save dialog
    if (scriptEditorWidget()->file.isEmpty())
        scriptEditorWidget()->file = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python files (*.py)"));

    // write text
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        if (fileInfo.suffix() != "py") scriptEditorWidget()->file += ".py";

        QFile fileName(scriptEditorWidget()->file);
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
            ErrorResult errorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved.").arg(scriptEditorWidget()->file));
            errorResult.showDialog();
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDir", fileInfo.absolutePath());
    }
}

void PythonEditorDialog::doFileSaveAs()
{
    QSettings settings;
    QString dir = settings.value("General/LastDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python files (*.py)"));
    if (!fileName.isEmpty())
    {
        scriptEditorWidget()->file = fileName;
        doFileSave();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDir", fileInfo.absolutePath());
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
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
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
    actCommentSelection->disconnect();
    connect(actCommentSelection, SIGNAL(triggered()), txtEditor, SLOT(commentSelection()));
    actUncommentSelection->disconnect();
    connect(actUncommentSelection, SIGNAL(triggered()), txtEditor, SLOT(uncommentSelection()));
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
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        fileName = fileInfo.completeBaseName();
    }
    setWindowTitle(tr("Python Lab - %1").arg(fileName));

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
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
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
    if (!scriptEditorWidget()->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->file);
        if (recentFiles.indexOf(fileInfo.absoluteFilePath()) == -1)
            recentFiles.insert(0, fileInfo.absoluteFilePath());
        else
            recentFiles.move(recentFiles.indexOf(fileInfo.absoluteFilePath()), 0);

        while (recentFiles.count() > 15) recentFiles.removeLast();
    }

    mnuRecentFiles->clear();
    for (int i = 0; i<recentFiles.count(); i++)
    {
        QAction *actMenuRecentItem = new QAction(recentFiles[i], this);
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

// ******************************************************************************************************

ScriptEditor::ScriptEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new ScriptEditorLineNumberArea(this);

#ifndef Q_WS_MAC
    setFont(QFont("Monospace", 10));
#endif
    setTabStopWidth(fontMetrics().width("    "));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    // highlighter
    new QPythonHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

ScriptEditor::~ScriptEditor()
{
    delete lineNumberArea;
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab)
    {
        if (textCursor().hasSelection())
        {
            indentSelection();
            return;
        }
    }

    if (event->key() == Qt::Key_Backtab)
    {
        if (textCursor().hasSelection())
        {
            unindentSelection();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void ScriptEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void ScriptEditor::indentSelection()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.insertText("\t");
            end++;
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::unindentSelection()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == "\t")
            {
                cursor.removeSelectedText();
                end--;
            }
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::commentSelection()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.insertText("#");
            end++;
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
}

void ScriptEditor::uncommentSelection()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        bool go = true;
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        while (cursor.position() < end && go)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == "#")
            {
                cursor.removeSelectedText();
                end--;
            }
            go = cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
    }
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
    const QBrush bookmarkBrushPyFlakes(Qt::red);

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
            // draw number
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);

            // draw rect
            if (errorMessagesPyFlakes.contains(blockNumber + 1))
                painter.fillRect(2, top+2, 8, fontMetrics().height()-4,
                                 bookmarkBrushPyFlakes);
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

    int space = 15 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void ScriptEditor::matchParentheses(char left, char right)
{
    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data)
    {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i)
        {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == left)
            {
                if (matchLeftParenthesis(left, right, textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
            else if (info->position == curPos - 1 && info->character == right)
            {
                if (matchRightParenthesis(left, right, textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool ScriptEditor::matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i)
    {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == left)
        {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == right && numLeftParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numLeftParentheses;
        }
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(left, right, currentBlock, 0, numLeftParentheses);

    return false;
}

bool ScriptEditor::matchRightParenthesis(char left, char right, QTextBlock currentBlock, int i, int numRightParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i)
    {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == right)
        {
            ++numRightParentheses;
            continue;
        }
        if (info->character == left && numRightParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numRightParentheses;
        }
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(left, right, currentBlock, 0, numRightParentheses);

    return false;
}

void ScriptEditor::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setForeground(Qt::red);
    // format.setBackground(Qt::lightGray);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
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

int SearchWidget::showFind(const QString &text)
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

int SearchWidget::showReplaceAll(const QString &text)
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
