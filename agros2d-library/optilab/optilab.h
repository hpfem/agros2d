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

#ifndef OPTILABDIALOG_H
#define OPTILABDIALOG_H

#include "util.h"
#include "optilab_data.h"

#include "qcustomplot/qcustomplot.h"

class SystemOutputWidget;
class PythonScriptingConsole;
class PythonEditorAgrosDialog;
class OptilabSingle;
class OptilabMulti;

class OptilabWindow : public QMainWindow
{
    Q_OBJECT
public:
    OptilabWindow(PythonEditorAgrosDialog *scriptEditorDialog);
    ~OptilabWindow();

    void showDialog();

    QStringList *recentFiles() { return &m_recentFiles; }

public slots:
    void documentNew();
    void documentOpen(const QString &fileName = "");
    void documentClose();

private slots:
    void openProblemAgros2D();
    void scriptEditor();

    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void documentOpenRecent(QAction *action);
    void doAbout();

    void refreshVariants();

    void variantOpenInExternalAgros2D();
    void variantSolveInExternalSolver();

    void variantOpenInAgros2D();
    void variantSolveInAgros2D();

    void processOpenError(QProcess::ProcessError error);
    void processOpenFinished(int exitCode);
    void processSolveError(QProcess::ProcessError error);
    void processSolveFinished(int exitCode);

private:
    // script editor
    PythonEditorAgrosDialog *m_scriptEditorDialog;

    QString m_problem;

    QStringList m_recentFiles;
    QMenu *mnuRecentFiles;
    QActionGroup *actDocumentOpenRecentGroup;

    QTreeWidget *trvVariants;
    QLabel *lblProblems;

    QTabWidget *tbxAnalysis;
    OptilabSingle *optilabSingle;
    OptilabMulti *optilabMulti;

    QAction *actScriptEditor;
    QAction *actExit;
    QAction *actAbout;
    QAction *actAboutQt;
    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    QAction *actDocumentClose;

    QPushButton *btnOpenInAgros2D;
    QPushButton *btnSolveInAgros2D;
    QPushButton *btnSolveInExternalSolver;
    QPushButton *btnOpenInExternalAgros2D;

    PythonScriptingConsole *console;

    void createActions();
    void createMenus();
    void createToolBars();
    void createMain();

    void setRecentFiles();

    friend class OptilabSingle;
    friend class OptilabMulti;
};

#endif // OPTILABDIALOG_H
