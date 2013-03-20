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

#ifndef GUI_CHART_H
#define GUI_CHART_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>

#include "util.h"

class QwtPlotGrid;

class ChartBasic : public QwtPlot
{
    Q_OBJECT
public:
    ChartBasic(QWidget *parent = 0);
    ~ChartBasic();

    inline QwtPlotCurve *curve() { return m_curve; }

public slots:
   void setData(double *xval, double *yval, int count);
   void setData(QList<double> xval, QList<double> yval);
   void saveImage(const QString &fileName = "");

protected:
    QwtPlotCurve *m_curve;
    QwtPlotGrid *m_grid;

private slots:
    void pickerValueMoved(const QPoint &pos);
};

class Chart : public ChartBasic
{
    Q_OBJECT
public:
    Chart(QWidget *parent = 0);
    ~Chart();

public slots:

protected:
   virtual void contextMenuEvent(QContextMenuEvent *event);

private:
    QMenu *mnuChart;

    QAction *showGrid;

private slots:
    void showGridChanged();
};

#endif // GUI_CHART_H
