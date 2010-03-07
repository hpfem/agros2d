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

#include "scene.h"
#include "util.h"
#include "gui.h"
#include "localvalueview.h"

class Chart;

class ChartDialog : public QDialog
{
    Q_OBJECT

signals:
    void setChartLine(const Point &start, const Point &end);

public slots:
    void doPlot();

public:
    ChartDialog(QWidget *parent = 0);
    ~ChartDialog();

    void showDialog();

protected:
    void hideEvent(QHideEvent *event);

private:
    QTabWidget* tabOutput;
    QTabWidget* tabAnalysisType;

    QwtPlotPicker *picker;

    // geometry
    QComboBox *cmbTimeStep;

    QLabel *lblStartX;
    QLabel *lblStartY;
    QLabel *lblEndX;
    QLabel *lblEndY;

    SLineEditDouble *txtStartX;
    SLineEditDouble *txtStartY;
    SLineEditDouble *txtEndX;
    SLineEditDouble *txtEndY;

    QRadioButton *radAxisLength;
    QRadioButton *radAxisX;
    QRadioButton *radAxisY;

    QSpinBox *txtAxisPoints;

    // time
    QLabel *lblPointX;
    QLabel *lblPointY;
    SLineEditDouble *txtPointX;
    SLineEditDouble *txtPointY;

    QComboBox *cmbFieldVariable;
    QComboBox *cmbFieldVariableComp;

    QWidget *widGeometry;
    QWidget *widTime;

    Chart *chart;
    QTableWidget *trvTable;

    void createControls();

    void plotGeometry();
    void plotTime();

private slots:
    void doFieldVariable(int index);
    void doFieldVariableComp(int index);
    void doSaveImage();
    void doExportData();

    void doMoved(const QPoint &);
    void doChartLine();
    void doTimeStepChanged(int index);
};

#endif // CHARTDIALOG_H
