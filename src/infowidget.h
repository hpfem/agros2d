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

#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include "util.h"
#include "sceneview_common.h"

#include <QWebView>

class SceneViewPreprocessor;

class ChartInfoWidget : public QDialog
{
    Q_OBJECT
public:
    ChartInfoWidget(QWidget *parent);

    void createControls();
    void setDataCurve(double *x, double *y, int size, QColor color = Qt::blue);

    void setXLabel(const QString &xlabel);
    void setYLabel(const QString &ylabel);

private:
    Chart *chart;

private slots:
    void saveImage();
};

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    InfoWidget(SceneViewPreprocessor *sceneView, QWidget *parent = 0);
    ~InfoWidget();

public slots:
    void refresh();

private:
    SceneViewPreprocessor *m_sceneViewGeometry;

    QPushButton *btnAdaptiveError;
    QPushButton *btnDOFs;

    QWebView *webView;

    QDialog *createChart(const QString &xlabel, const QString &ylabel,
                         double *x1, double *y1, int size1,
                         double *x2, double *y2, int size2);

private slots:
    void showInfo();

    void doAdaptiveError();
    void doAdaptiveDOFs();
};

#endif // SCENEINFOVIEW_H
