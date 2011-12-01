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

#include "util.h"

//class Solution;
class Linearizer;
class Vectorizer;

namespace Hermes{
namespace Hermes2D{
namespace Views{
class Orderizer;
}
}
}
class Mesh;

class Chart;
class QwtPlotCurve;


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

    void init();
    virtual void setSteps() = 0;

    inline QString name() { return m_name; }
    inline int steps() { return m_steps; }
    inline bool isCanceled() { return m_isCanceled; }
    inline void emitMessage(const QString &msg, bool isError, int position = 0) { emit message(msg, isError, position); }
    virtual bool run(bool quiet = false) = 0;

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

    void setSteps();

    bool run(bool quiet = false);

    void mesh();

private:
    struct MeshEdge
    {
        MeshEdge()
        {
            this->node[0] = -1;
            this->node[1] = -1;
            this->marker = -1;

            this->isActive = true;
            this->isUsed = true;
        }

        MeshEdge(int node_1, int node_2, int marker)
        {
            this->node[0] = node_1;
            this->node[1] = node_2;
            this->marker = marker;

            this->isActive = true;
            this->isUsed = true;
        }

        int node[2], marker;
        bool isActive, isUsed;
    };

    struct MeshElement
    {
        MeshElement()
        {
            this->node[0] = -1;
            this->node[1] = -1;
            this->node[2] = -1;
            this->node[3] = -1;
            this->marker = -1;

            this->isActive = true;
            this->isUsed = true;
        }

        MeshElement(int node_1, int node_2, int node_3, int marker)
        {
            this->node[0] = node_1;
            this->node[1] = node_2;
            this->node[2] = node_3;
            this->node[3] = -1;
            this->marker = marker;

            this->isActive = true;
            this->isUsed = true;
        }

        MeshElement(int node_1, int node_2, int node_3, int node_4, int marker)
        {
            this->node[0] = node_1;
            this->node[1] = node_2;
            this->node[2] = node_3;
            this->node[3] = node_4;
            this->marker = marker;

            this->isActive = true;
            this->isUsed = true;
        }

        inline bool isTriangle() const { return (node[3] == -1); }

        int node[4], marker;
        bool isActive, isUsed;

        int neigh[3];
    };

    /*
    struct MeshNode
    {
        MeshNode(int n, double x, double y, int marker)
        {
            this->n = n;
            this.x = x;
            this.y = n;
            this->marker = marker;
        }

        int n;
        double x, y;
        int marker;
    };
    */
};

class ProgressItemSolve : public ProgressItem
{
    Q_OBJECT

public:
    ProgressItemSolve();

    void setSteps();

    bool run(bool quiet = false);

    inline void addAdaptivityError(double error, int dof) { m_adaptivityError.append(error); m_adaptivityDOF.append(dof); emit changed(); }
    inline QList<double> adaptivityError() { return m_adaptivityError; }
    inline QList<int> adaptivityDOF() { return m_adaptivityDOF; }

    inline void addNonlinearError(double error) { m_nonlinearError.append(error); emit changed(); }
    inline QList<double> nonlinearError() { return m_nonlinearError; }

protected slots:
    virtual void solve();

protected:
    QList<double> m_adaptivityError;
    QList<int> m_adaptivityDOF;
    QList<double> m_nonlinearError;
};

class ProgressItemSolveAdaptiveStep : public ProgressItemSolve
{
    Q_OBJECT

public:
    ProgressItemSolveAdaptiveStep();

protected slots:
    void solve();
};

class ProgressItemProcessView : public ProgressItem
{
    Q_OBJECT
private:

private slots:
    void process();

public:
    ProgressItemProcessView();

    void setSteps();

    bool run(bool quiet = false);
};

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

    void appendProgressItem(ProgressItem *progressItem);
    bool run(bool showViewProgress = true);
    void clear();

    inline ProgressItem *currentProgressItem() { return m_currentProgressItem; }

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
    QWidget *controlsNonlinear;

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
    Chart *chartNonlinear;
    QwtPlotCurve *curveError;
    QwtPlotCurve *curveErrorMax;
    QwtPlotCurve *curveDOF;
    QwtPlotCurve *curveErrorDOF;
    QwtPlotCurve *curveErrorDOFMax;
    QwtPlotCurve *curveNonlinear;
    QwtPlotCurve *curveNonlinearMax;

    QWidget *createControlsProgress();
    QWidget *createControlsConvergenceErrorChart();
    QWidget *createControlsConvergenceDOFChart();
    QWidget *createControlsConvergenceErrorDOFChart();
    QWidget *createControlsNonlinear();

    void createControls();
    int progressSteps();
    int currentProgressStep();
    void saveProgressLog();

public slots:
    void close();

private slots:
    void finished();
    void start();
    void cancel();
    void resetControls(int currentTab);
    void saveImage();
    void saveData();

    void itemChanged();
};

#endif //SCENEHERMES_H
