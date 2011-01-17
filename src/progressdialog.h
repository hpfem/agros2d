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

#ifndef SCENEHERMES_H
#define SCENEHERMES_H

#include <QDomDocument>

#include "util.h"
#include "gui.h"

class Solution;
class Linearizer;
class Vectorizer;
class Orderizer;
class Mesh;

struct SolutionArray
{
    double time;
    double adaptiveError;
    int adaptiveSteps;

    Solution *sln;
    Orderizer *order;

    SolutionArray();
    ~SolutionArray();

    void load(QDomElement *element);
    void save(QDomDocument *doc, QDomElement *element);
};

class ProgressItem : public QObject
{
    Q_OBJECT

protected:
    QString m_name;
    int m_steps;
    bool m_isError;
    bool m_isCanceled;

public:
    ProgressItem();

    inline QString name() { return m_name; }
    inline int steps() { return m_steps; }
    inline bool isCanceled() { return m_isCanceled; }
    inline void emitMessage(const QString &msg, bool isError, int position = 0) { emit message(msg, isError, position); }
    virtual bool run() = 0;

signals:
    void message(const QString &message, bool isError, int position);
    void changed();

protected slots:
    void showMessage(const QString &msg, bool isError, int position);
    virtual void cancelProgressItem() { m_isCanceled = true; }
};

class ProgressItemMesh : public ProgressItem
{
    Q_OBJECT

private slots:
    void meshTriangleCreated(int exitCode);
    bool writeToTriangle();
    bool triangleToHermes2D();

public:
    ProgressItemMesh();

    bool run();
};

class ProgressItemSolve : public ProgressItem
{
    Q_OBJECT

public:
    ProgressItemSolve();

    bool run();
    inline void addAdaptivityError(double error, int dof) { m_adaptivityError.append(error); m_adaptivityDOF.append(dof); emit changed(); }
    inline QList<double> adaptivityError() { return m_adaptivityError; }
    inline QList<int> adaptivityDOF() { return m_adaptivityDOF; }


private slots:
    void solve();

private:
    QList<double> m_adaptivityError;
    QList<int> m_adaptivityDOF;
};

class ProgressItemProcessView : public ProgressItem
{
    Q_OBJECT
private:
    // SceneView *m_sceneView;

private slots:
    void process();

public:
    ProgressItemProcessView();

    bool run();
};

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

    void appendProgressItem(ProgressItem *progressItem);
    bool run(bool showViewProgress = true);

signals:
    void cancelProgressItem();

public slots:
    void showMessage(const QString &msg, bool isError, int position);

private:
    bool m_showViewProgress;
    QTimer *m_refreshTimer;
    QList<ProgressItem *> m_progressItem;
    ProgressItem *m_currentProgressItem;

    QTabWidget *tabType;
    QWidget *controlsProgress;
    QWidget *controlsConvergenceErrorChart;
    QWidget *controlsConvergenceDOFChart;
    QWidget *controlsConvergenceErrorDOFChart;

    QLabel *lblMessage;
    QProgressBar *progressBar;
    QTextEdit *lstMessage;
    QPushButton *btnCancel;
    QPushButton *btnClose;
    QPushButton *btnSaveImage;
    QPushButton *btnSaveData;

    Chart *chartError;
    Chart *chartDOF;
    Chart *chartErrorDOF;
    QwtPlotCurve *curveError;
    QwtPlotCurve *curveErrorMax;
    QwtPlotCurve *curveDOF;
    QwtPlotCurve *curveErrorDOF;
    QwtPlotCurve *curveErrorDOFMax;

    QWidget *createControlsProgress();
    QWidget *createControlsConvergenceErrorChart();
    QWidget *createControlsConvergenceDOFChart();
    QWidget *createControlsConvergenceErrorDOFChart();

    void createControls();
    void clear();
    int progressSteps();
    int currentProgressStep();
    void saveProgressLog();

private slots:
    void finished();
    void start();
    void cancel();
    void close();
    void resetControls(int currentTab);
    void saveImage();
    void saveData();

    void itemChanged();
};

#endif //SCENEHERMES_H
