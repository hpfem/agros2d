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
#include "pythonengine_optilab.h"

class SystemOutputWidget;

class OptilabWindow : public QMainWindow
{
    Q_OBJECT
public:
    OptilabWindow();
    ~OptilabWindow();

private slots:
    void openProblemAgros2D();

    // PythonLab
    void scriptEditor();

    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void documentNew();
    void documentOpen(const QString &fileName = "");
    void documentClose();
    void documentOpenRecent(QAction *action);
    void doAbout();

    void refreshVariants();

    void variantOpenInAgros2D();
    void variantSolveInSolver();

    void processOpenError(QProcess::ProcessError error);
    void processOpenFinished(int exitCode);
    void processSolveError(QProcess::ProcessError error);
    void processSolveFinished(int exitCode);

    void linkClicked(const QUrl &url);

    void setPythonVariables();
    void refreshChart();
    void refreshChartWithAxes();
    void refreshChartControls();

    void graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event);

private:
    // problem
    QString m_problemDir;

    QStringList recentFiles;
    QMenu *mnuRecentFiles;
    QActionGroup *actDocumentOpenRecentGroup;

    QWebView *webView;
    QString m_cascadeStyleSheet;

    QTreeWidget *trvVariants;
    QLabel *lblProblems;

    // chart
    QComboBox *cmbX;
    QComboBox *cmbY;
    QCustomPlot *chart;
    QRadioButton *radChartLine;
    QRadioButton *radChartXY;

    QAction *actScriptEditor;
    QAction *actExit;
    QAction *actAbout;
    QAction *actAboutQt;
    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    QAction *actDocumentClose;
    QAction *actOpenAgros2D;

    QPushButton *btnSolveInSolver;
    QPushButton *btnOpenInAgros2D;

    PythonScriptingConsole *console;
    PythonEditorOptilabDialog *scriptEditorDialog;

    OutputVariablesAnalysis outputVariables;

    void variantInfo(const QString &key);
    void welcomeInfo();

    void createActions();
    void createMenus();
    void createToolBars();
    void createMain();

    void setRecentFiles();
};

#endif // OPTILABDIALOG_H
