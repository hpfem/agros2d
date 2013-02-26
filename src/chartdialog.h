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

#ifndef CHARTDIALOG_H
#define CHARTDIALOG_H

#include "util.h"
#include "gui/chart.h"
#include "hermes2d/plugin_interface.h"

#include <QSvgWidget>
#include <QSvgRenderer>

class ChartBasic;
class LineEditDouble;
class LineEditDouble;
class LocalValue;
class FieldInfo;
class SceneViewPost2D;
class PhysicalFieldWidget;

namespace Module
{
struct LocalVariable;
}


class SvgWidget : public QSvgWidget
{
public:
    SvgWidget(QWidget *parent=0);

    void load(const QString &fileName);

protected:
    void paintEvent(QPaintEvent* qp);

private:
    QSvgRenderer renderer;
};

// definition of chart line
struct ChartLine
{
    Point start;
    Point end;
    int numberOfPoints;
    bool reverse;

    ChartLine() : start(Point()), end(end), numberOfPoints(0), reverse(false) {}

    ChartLine(Point start, Point end, int numberOfPoints = 200, bool reverse = false)
    {
        this->start = start;
        this->end = end;
        this->numberOfPoints = numberOfPoints;
        this->reverse = reverse;
    }

    QList<Point> getPoints();
};

class ChartView : public QWidget
{
    Q_OBJECT

public:
    ChartView(QWidget *parent = 0);

    QAction *actSceneModeChart;

    inline ChartBasic *chart() { return m_chart; }

public slots:
    void setControls();

private:
    Chart *m_chart;
};

class ChartWidget : public QWidget
{
    Q_OBJECT

public slots:
    void doApply();
    void updateControls();

public:
    ChartWidget(ChartView *chart,
                        QWidget *parent = 0);
    ~ChartWidget();

private:
    // variable widget
    SvgWidget *viewerSVG;

    QTabWidget* tbxAnalysisType;

    // buttons
    QPushButton *btnOK;
    QPushButton *btnSaveImage;
    QPushButton *btnExportData;

    // geometry
    QLabel *lblStartX;
    QLabel *lblStartY;
    QLabel *lblEndX;
    QLabel *lblEndY;

    LineEditDouble *txtStartX;
    LineEditDouble *txtStartY;
    LineEditDouble *txtEndX;
    LineEditDouble *txtEndY;

    QRadioButton *radHorizontalAxisLength;
    QRadioButton *radHorizontalAxisX;
    QRadioButton *radHorizontalAxisY;

    QSpinBox *txtHorizontalAxisPoints;
    QCheckBox *chkHorizontalAxisReverse;

    // time
    QLabel *lblPointX;
    QLabel *lblPointY;
    LineEditDouble *txtTimeX;
    LineEditDouble *txtTimeY;

    PhysicalFieldWidget *fieldWidget;

    QComboBox *cmbFieldVariable;
    QComboBox *cmbFieldVariableComp;

    QWidget *widGeometry;
    QWidget *widTime;

    ChartView *m_chart;

    void createControls();

    QList<double> horizontalAxisValues(ChartLine *chartLine);

    void plotGeometry();
    void plotTime();

    void fillTableRow(LocalValue *localValue, double time, int row);

    QStringList headers();
    void addValue(LocalValue *localPointValue, double time, double *yval, int i, int N,
                  PhysicFieldVariableComp physicFieldVariableComp,
                  Module::LocalVariable *physicFieldVariable);

private slots:
    void doField();
    void doFieldVariable(int index);
    void doExportData();
    QMap<QString, double> getData(Point point, int timeStep, int adaptivityStep, SolutionMode solutionType);

    void createChartLine();
};

#endif // CHARTDIALOG_H
