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

#ifndef OPTILABMULTI_H
#define OPTILABMULTI_H

#include "../util/util.h"
#include "optilab.h"

class OptilabMulti : public QWidget
{
    Q_OBJECT
public:
    OptilabMulti(OptilabWindow *parent = 0);

public slots:
    void refreshVariables();

private:
    OptilabWindow *optilabMain;

    // chart
    QComboBox *cmbChartLineX;
    QComboBox *cmbChartXYX;
    QComboBox *cmbChartXYY;
    QCustomPlot *chartLine;
    QCustomPlot *chartXY;
    QTabWidget *tbxPlot;

private slots:
    void refreshLineChart();
    void refreshXYChart();
    // void refreshChartWithAxes();
    // void refreshChartControls();

    // void graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event);
};

#endif // OPTILABMULTI_H
