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
#include "qcustomplot/qcustomplot.h"

#include "pythonengine_optilab.h"

class SystemOutputWidget;

class OutputVariable
{
public:
    OutputVariable()
        : m_number(0.0), m_name(""), m_x(QVector<double>()), m_y(QVector<double>()) {}

    OutputVariable(const QString &valueName, const QString &valueString)
        : m_number(0.0), m_name(valueName), m_x(QVector<double>()), m_y(QVector<double>())
    {
        bool isNum;

        valueString.toDouble(&isNum);
        if (isNum)
        {
            m_number = valueString.toDouble();
        }
        else
        {
            QString valueTMP = valueString;

            QStringList data = valueTMP.replace("[[", "").replace("]]", "").split("], [");
            QStringList dataX = data[0].split(",");
            QStringList dataY = data[1].split(",");

            m_x.clear();
            m_y.clear();
            for (int i = 0; i < dataX.size(); i++)
            {
                m_x.append(dataX[i].toDouble());
                m_y.append(dataY[i].toDouble());
            }
        }
    }

    inline QString name() const { return m_name; }

    inline bool isNumber() const { return m_x.size() == 0; }
    inline double number() const { return m_number; }

    inline QVector<double> x() const { return m_x; }
    inline QVector<double> y() const { return m_y; }
    inline int size() const { assert(m_x.size() == m_y.size()); return m_x.size(); }

private:
    // name
    QString m_name;

    // number
    double m_number;

    // table
    QVector<double> m_x;
    QVector<double> m_y;
};

class OutputVariablesAnalysis
{
public:
    inline void append(const QString &fileName, const QList<OutputVariable> &variables)
    {
        m_variables[fileName] = variables;
    }
    inline int size() const { return m_variables.size(); }
    inline void clear()
    {
        for (int i = 0; i < size(); i++)
            m_variables.values().clear();
        m_variables.clear();
    }
    inline QHash<QString, QList<OutputVariable> > variables() const { return m_variables; }

    QStringList fileNames() { return m_variables.keys(); }

    QVector<double> values(const QString &name) const
    {
        QVector<double> vals;

        for (int i = 0; i < size(); i++)
        {            
            for (int j = 0; j < m_variables.values().at(i).size(); j++)
            {
                const OutputVariable *variable = &m_variables.values().at(i).at(j);

                if (variable->name() == name)
                    vals.append(variable->number());
            }
        }

        return vals;
    }

    QStringList variableNames(bool onlyNumbers = false) const
    {
        QStringList vars;

        // TODO: ensure that all variables have same name
        if (size() > 0)
        {
            for (int j = 0; j < m_variables.values().at(0).size(); j++)
            {
                if (onlyNumbers && !m_variables.values().at(0).at(j).isNumber())
                    continue;

                vars.append(m_variables.values().at(0).at(j).name());
            }
        }

        return vars;
    }

private:
    QHash<QString, QList<OutputVariable> > m_variables;
};

class OptilabWindow : public QMainWindow
{
    Q_OBJECT
public:
    OptilabWindow();
    ~OptilabWindow();

    inline QString selectedFilename() { return m_selectedFilename; }

private slots:
    void openInAgros2D();
    void solveInSolver();

    void doScriptEditor();

    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void pathChangeDir();
    void readVariants();
    void readVariantsThread();

    void linkClicked(const QUrl &url);

    void processOpenError(QProcess::ProcessError error);
    void processOpenFinished(int exitCode);
    void processSolveError(QProcess::ProcessError error);
    void processSolveFinished(int exitCode);

    void setPythonVariables();
    void setChart();

    void graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event);

private:
    QWebView *webView;
    QString m_cascadeStyleSheet;

    QLabel *lblPath;
    QTreeWidget *lstProblems;
    QLabel *lblProblems;
    QString m_selectedFilename;

    QComboBox *cmbX;
    QComboBox *cmbY;

    QAction *actScriptEditor;
    QAction *actExit;
    QAction *actAbout;
    QAction *actAboutQt;
    QAction *actReadVariants;
    QAction *actOpenInAgros2D;
    QAction *actSolverInSolver;

    PythonScriptingConsole *console;
    PythonEditorOptilabDialog *scriptEditorDialog;

    OutputVariablesAnalysis outputVariables;

    QCustomPlot *chart;

    void variantInfo(const QString &fileName);
    bool variantOutputCache(const QString &fileName);

    void createActions();
    void createMenus();
    void createToolBars();
    void createMain();
};

#endif // OPTILABDIALOG_H
