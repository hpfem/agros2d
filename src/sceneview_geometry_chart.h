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

#ifndef SCENEVIEWGEOMETRYCHART_H
#define SCENEVIEWGEOMETRYCHART_H

#include "util.h"

#include "sceneview_common2d.h"
#include "chartdialog.h"

class SceneViewPreprocessorChart : public SceneViewCommon2D
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();

public:
    SceneViewPreprocessorChart(QWidget *parent = 0);
    ~SceneViewPreprocessorChart();

    void setChartLine(ChartLine chartLine);

protected:
    void paintGL();
    virtual void doZoomRegion(const Point &start, const Point &end);

    void paintChartLine();
    void paintGeometry(); // paint nodes, edges and labels

private:
    ChartLine m_chartLine;
};

#endif // SCENEVIEWGEOMETRYCHART_H
