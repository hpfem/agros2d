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


#include <Python.h>


#include "optilab.h"
#include "optilab_single.h"
#include "optilab_multi.h"

#include "util/constants.h"
#include "gui/lineeditdouble.h"
#include "gui/common.h"
#include "gui/systemoutput.h"
#include "gui/about.h"

#include <fstream>
#include <string>

#include "pythonlab/pythonengine_agros.h"

#include <QXmlSimpleReader>

OptilabWindow::OptilabWindow(PythonEditorAgrosDialog *scriptEditorDialog) : QMainWindow(),
    m_scriptEditorDialog(scriptEditorDialog), m_problemDir("")
{
    setWindowTitle(tr("Optilab"));

    createActions();
    createToolBars();
    createMenus();
    createMain();

    QSettings settings;
    restoreGeometry(settings.value("OptilabWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("OptilabWindow/State", saveState()).toByteArray());
    recentFiles = settings.value("OptilabWindow/RecentFiles").value<QStringList>();

    // set recent files
    setRecentFiles();

    optilabSingle->welcomeInfo();
}

OptilabWindow::~OptilabWindow()
{
    QSettings settings;
    settings.setValue("OptilabWindow/Geometry", saveGeometry());
    settings.setValue("OptilabWindow/State", saveState());
    settings.setValue("OptilabWindow/RecentFiles", recentFiles);

    removeDirectory(tempProblemDir());
}

void OptilabWindow::processOpenError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D");
}

void OptilabWindow::processOpenFinished(int exitCode)
{
    if (exitCode == 0)
    {
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        qDebug() << "Agros2D";
        qDebug() << errorMessage;
    }
}

void OptilabWindow::variantOpenInExternalAgros2D()
{
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        // TODO: template?
        QString str;
        str += "from variant import model\n";
        str += QString("import sys; sys.path.insert(0, '%1')\n").arg(m_problemDir);
        str += "import problem\n";
        str += "p = problem.Model()\n";
        str += QString("p.load('%1')\n").arg(fileName);
        str += "p.create()\n";

        QString id = QUuid::createUuid().toString().remove("{").remove("}");
        QString tempFN = QString("%1/%2.py").arg(tempProblemDir()).arg(id);

        writeStringContent(tempFN, str);

        // run agros2d
        QProcess *process = new QProcess();
        process->setStandardOutputFile(tempProblemDir() + "/agros2d.out");
        process->setStandardErrorFile(tempProblemDir() + "/agros2d.err");
        connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processOpenError(QProcess::ProcessError)));
        connect(process, SIGNAL(finished(int)), this, SLOT(processOpenFinished(int)));

        QString command = QString("\"%1/agros2d\" -x -s \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(tempFN);

        process->start(command);
    }
}

void OptilabWindow::variantOpenInAgros2D()
{
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        QString str = QString("variant.optilab_interface._open_in_agros2d('%1')").arg(fileName);
        // qDebug() << str;

        currentPythonEngine()->runScript(str);
    }
}

void OptilabWindow::variantSolveInExternalSolver()
{
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        // TODO: template?
        QString str;
        str += "from variant import model\n";
        str += QString("import sys; sys.path.insert(0, '%1')\n").arg(m_problemDir);
        str += "import problem\n";
        str += "p = problem.Model()\n";
        str += QString("p.load('%1')\n").arg(fileName);
        str += "p.create()\n";
        str += "p.solve()\n";
        str += "p.process()\n";
        str += QString("p.save('%1')\n").arg(fileName);

        QString command = QString("\"%1/agros2d_solver\" -l -c \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(str);

        SystemOutputWidget *systemOutput = new SystemOutputWidget(this);
        connect(systemOutput, SIGNAL(finished(int)), this, SLOT(processSolveFinished(int)));
        systemOutput->execute(command);
    }
}

void OptilabWindow::variantSolveInAgros2D()
{
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        QString str = QString("variant.optilab_interface._solve_in_agros2d('%1')").arg(fileName);
        // qDebug() << str;

        currentPythonEngine()->runScript(str);
        refreshVariants();
    }
}

void OptilabWindow::processSolveError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D Solver");
}

void OptilabWindow::processSolveFinished(int exitCode)
{
    if (exitCode == 0)
    {
        QApplication::processEvents();

        /*
        int index = trvVariants->currentItem()->data(0, Qt::UserRole).toInt();
        QDomNode nodeResultOld = docXML.elementsByTagName("results").at(0).childNodes().at(index);

        QString fileName = QString("%1/models/%2").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

        QDomNode nodeResultNew = readVariant(fileName);
        nodeResultOld.parentNode().appendChild(nodeResultNew);
        nodeResultOld.parentNode().replaceChild(nodeResultOld, nodeResultNew);
        */

        refreshVariants();
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        qDebug() << "Agros2D";
        qDebug() << errorMessage;
    }
}

void OptilabWindow::createActions()
{
    actAbout = new QAction(icon("about"), tr("About &Optilab"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actScriptEditor = new QAction(icon("script-python"), tr("PythonLab"), this);
    actScriptEditor->setShortcut(Qt::Key_F9);
    connect(actScriptEditor, SIGNAL(triggered()), this, SLOT(scriptEditor()));

    actDocumentNew = new QAction(icon("document-new"), tr("&New..."), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(documentNew()));

    actDocumentOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(documentOpen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(documentOpenRecent(QAction *)));

    actDocumentClose = new QAction(tr("&Close"), this);
    actDocumentClose->setShortcuts(QKeySequence::Close);
    connect(actDocumentClose, SIGNAL(triggered()), this, SLOT(documentClose()));

    // actOpenAgros2D = new QAction(icon("agros2d"), tr("Open Agros2D"), this);
    // actOpenAgros2D->setEnabled(false);
    // connect(actOpenAgros2D, SIGNAL(triggered()), this, SLOT(openProblemAgros2D()));
}

void OptilabWindow::createMenus()
{
    menuBar()->clear();

    mnuRecentFiles = new QMenu(tr("&Recent files"), this);

    QMenu *mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addAction(actDocumentClose);
#ifndef Q_WS_MAC
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);
#endif

    QMenu *mnuHelp = menuBar()->addMenu(tr("&Help"));
#ifndef Q_WS_MAC
    mnuHelp->addSeparator();
#else
    // mnuHelp->addAction(actOptions); // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actExit);    // will be added to "Agros2D" MacOSX menu
#endif
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros2D" MacOSX menu
}

void OptilabWindow::createToolBars()
{
    // top toolbar
#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    QToolBar *tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->setOrientation(Qt::Horizontal);
    tlbFile->setAllowedAreas(Qt::TopToolBarArea);
    tlbFile->setMovable(false);
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbFile->addAction(actDocumentNew);
    tlbFile->addAction(actDocumentOpen);

    QToolBar *tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->setOrientation(Qt::Horizontal);
    tlbTools->setAllowedAreas(Qt::TopToolBarArea);
    tlbTools->setMovable(false);
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->addSeparator();
    // tlbTools->addAction(actOpenAgros2D);
    tlbTools->addAction(actScriptEditor);
}

void OptilabWindow::createMain()
{
    console = new PythonScriptingConsole(currentPythonEngine(), this);

    optilabSingle = new OptilabSingle(this);
    optilabMulti = new OptilabMulti(this);

    tbxAnalysis = new QTabWidget();
    tbxAnalysis->addTab(optilabSingle, icon(""), tr("Single"));
    tbxAnalysis->addTab(optilabMulti, icon(""), tr("Multi"));

    trvVariants = new QTreeWidget(this);
    trvVariants->setMouseTracking(true);
    trvVariants->setColumnCount(2);
    trvVariants->setIndentation(15);
    trvVariants->setIconSize(QSize(16, 16));
    trvVariants->setHeaderHidden(true);
    trvVariants->setMinimumWidth(320);
    trvVariants->setColumnWidth(0, 150);

    connect(trvVariants, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvVariants, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvVariants, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), optilabSingle, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    btnOpenInExternalAgros2D = new QPushButton(tr("Open in external Agros2D"));
    btnOpenInExternalAgros2D->setToolTip(tr("Open in external Agros2D"));
    btnOpenInExternalAgros2D->setEnabled(false);
    connect(btnOpenInExternalAgros2D, SIGNAL(clicked()), this, SLOT(variantOpenInExternalAgros2D()));

    btnSolveInExternalSolver = new QPushButton(tr("Solve in external solver"));
    btnSolveInExternalSolver->setToolTip(tr("Solve in external solver"));
    btnSolveInExternalSolver->setEnabled(false);
    connect(btnSolveInExternalSolver, SIGNAL(clicked()), this, SLOT(variantSolveInExternalSolver()));

    btnOpenInAgros2D = new QPushButton(tr("Open in Agros2D"));
    btnOpenInAgros2D->setToolTip(tr("Open in Agros2D"));
    btnOpenInAgros2D->setEnabled(false);
    connect(btnOpenInAgros2D, SIGNAL(clicked()), this, SLOT(variantOpenInAgros2D()));

    btnSolveInAgros2D = new QPushButton(tr("Solve in Agros2D"));
    btnSolveInAgros2D->setToolTip(tr("Solve in Agros2D"));
    btnSolveInAgros2D->setEnabled(false);
    connect(btnSolveInAgros2D, SIGNAL(clicked()), this, SLOT(variantSolveInAgros2D()));

    QGridLayout *layoutButtons = new QGridLayout();
    layoutButtons->addWidget(btnOpenInExternalAgros2D, 0, 0);
    layoutButtons->addWidget(btnSolveInExternalSolver, 1, 0);
    layoutButtons->addWidget(btnOpenInAgros2D, 0, 1);
    layoutButtons->addWidget(btnSolveInAgros2D, 1, 1);

    lblProblems = new QLabel("");

    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addWidget(trvVariants);
    layoutLeft->addLayout(layoutButtons);
    layoutLeft->addWidget(lblProblems);

    QHBoxLayout *layoutRight = new QHBoxLayout();
    layoutRight->addWidget(tbxAnalysis);
    layoutRight->addWidget(console);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addLayout(layoutLeft);
    layout->addLayout(layoutRight, 1);

    QWidget *main = new QWidget();
    main->setLayout(layout);

    setCentralWidget(main);
}

void OptilabWindow::openProblemAgros2D()
{
    QString problem = QString("%1/problem.py").arg(m_problemDir);
    if (QFile::exists(problem))
    {
        QProcess process;

        QString command = QString("\"%1/agros2d\" -s \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(problem);

        process.startDetached(command);
    }
}

void OptilabWindow::scriptEditor()
{
    m_scriptEditorDialog->showDialog();
}

void OptilabWindow::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{    
    btnOpenInAgros2D->setEnabled(current);
    btnSolveInAgros2D->setEnabled(current);
    btnOpenInExternalAgros2D->setEnabled(current);
    btnSolveInExternalSolver->setEnabled(current);
}

void OptilabWindow::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item)
        variantOpenInAgros2D();
}

void OptilabWindow::documentNew()
{
    QSettings settings;

    QString dir = settings.value("General/LastProblemDir", "data").toString();
    QString fileNameDocument = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("OptiLab files (problem.py)"));

    if (!fileNameDocument.isEmpty())
    {

    }
}

void OptilabWindow::documentOpen(const QString &fileName)
{
    QSettings settings;
    QString fileNameDocument;

    if (fileName.isEmpty())
    {
        QString dir = settings.value("General/LastProblemDir", "data").toString();
        fileNameDocument = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("OptiLab files (problem.py)"));
    }
    else
    {
        fileNameDocument = fileName;
    }

    if (fileNameDocument.isEmpty() || !QFile::exists(fileNameDocument))
        return;

    QFileInfo fileInfo(fileNameDocument);
    if (fileInfo.suffix() == "py")
    {
        m_problemDir = QFileInfo(fileNameDocument).absolutePath();
        settings.setValue("General/LastProblemDir", m_problemDir);

        optilabSingle->doItemChanged(NULL, NULL);

        // set recent files
        setRecentFiles();

        refreshVariants();
    }
}

void OptilabWindow::documentOpenRecent(QAction *action)
{
    QString fileName = action->text();
    documentOpen(fileName);
}

void OptilabWindow::documentClose()
{
    m_problemDir = "";

    // clear listview
    trvVariants->clear();

    // actOpenAgros2D->setEnabled(false);

    currentPythonEngine()->runExpression("del variant.optilab_interface._md; del agros2d_model");

    optilabSingle->welcomeInfo();

    refreshVariants();
}

void OptilabWindow::doAbout()
{
    AboutDialog about(this);
    about.exec();
}

void OptilabWindow::showDialog()
{
    show();
    activateWindow();
    // txtEditor->setFocus();
}

void OptilabWindow::refreshVariants()
{
    QString str = QString("agros2d_variants = variant.optilab_interface._md_models('%1/models/')").arg(m_problemDir);
    currentPythonEngine()->runExpression(str);

    trvVariants->setUpdatesEnabled(false);

    // save current item
    QString selectedItem;
    if (trvVariants->currentItem())
        selectedItem = trvVariants->currentItem()->data(0, Qt::UserRole).toString();

    QTime time;
    time.start();

    // clear listview
    trvVariants->clear();

    int count = 0;
    int countSolved = 0;

    // extract values
    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_variants");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *d = PyList_GetItem(result, i);
            Py_INCREF(d);

            QString name = QString::fromWCharArray(PyUnicode_AsUnicode(PyDict_GetItemString(d, "key")));
            bool isSolved = PyLong_AsLong(PyDict_GetItemString(d, "solved"));

            QTreeWidgetItem *variantItem = new QTreeWidgetItem(trvVariants->invisibleRootItem());
            if (isSolved)
                variantItem->setIcon(0, icon("browser-other"));
            else
                variantItem->setIcon(0, icon("browser-class"));
            variantItem->setText(0, QFileInfo(name).baseName());
            variantItem->setText(1, QFileInfo(QString("%1/models/%2").
                                              arg(m_problemDir).
                                              arg(name)).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            variantItem->setData(0, Qt::UserRole, name);

            Py_XDECREF(d);

            // increase counter
            count++;
            if (isSolved)
                countSolved++;
        }
        Py_XDECREF(result);
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_variants");

    trvVariants->setUpdatesEnabled(true);

    // select first
    if (selectedItem.isEmpty() && trvVariants->topLevelItemCount() > 0)
        selectedItem = trvVariants->topLevelItem(0)->data(0, Qt::UserRole).toString();

    if (!selectedItem.isEmpty())
    {
        for (int i = 0; i < trvVariants->topLevelItemCount(); i++ )
        {
            QTreeWidgetItem *item = trvVariants->topLevelItem(i);

            if (selectedItem == item->data(0, Qt::UserRole).toString())
            {
                // qDebug() << "selected" << selectedItem;

                item->setSelected(true);
                trvVariants->setCurrentItem(item);
                // ensure visible
                trvVariants->scrollToItem(item);
            }
        }
    }

    lblProblems->setText(tr("Solutions: %1/%2").arg(countSolved).arg(count));

    qDebug() << "refresh" << time.elapsed();
}

void OptilabWindow::setRecentFiles()
{
    // recent files
    if (!m_problemDir.isEmpty())
    {
        if (recentFiles.indexOf(m_problemDir) == -1)
            recentFiles.insert(0, m_problemDir);
        else
            recentFiles.move(recentFiles.indexOf(m_problemDir), 0);

        while (recentFiles.count() > 15) recentFiles.removeLast();
    }

    mnuRecentFiles->clear();
    for (int i = 0; i<recentFiles.count(); i++)
    {
        QAction *actMenuRecentItem = new QAction(recentFiles[i], this);
        actDocumentOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}
