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

#include "sceneview_geometry_chart.h"

#include "util.h"
#include "util/global.h"
#include "util/loops.h"
#include "logview.h"

#include "scene.h"
#include "hermes2d/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenebasicselectdialog.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

SceneViewPreprocessorChart::SceneViewPreprocessorChart(QWidget *parent)
    : SceneViewCommon2D(NULL, parent)
{
    setMinimumSize(100, 50);
}

SceneViewPreprocessorChart::~SceneViewPreprocessorChart()
{
}

void SceneViewPreprocessorChart::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO)
        return;

    double sceneWidth = end.x - start.x;
    double sceneHeight = end.y - start.y;

    double maxScene = ((width() / height()) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale2d = 1.8/maxScene;

    m_offset2d.x = (start.x + end.x) / 2.0;
    m_offset2d.y = (start.y + end.y) / 2.0;

    setZoom(0);
}

void SceneViewPreprocessorChart::refresh()
{
    if (Agros2D::problem()->isSolved())
        SceneViewCommon::refresh();
}

void SceneViewPreprocessorChart::clear()
{
    if (Agros2D::problem()->isSolved())
        doZoomBestFit();
}

void SceneViewPreprocessorChart::setChartLine(ChartLine chartLine)
{
    m_chartLine = chartLine;

    updateGL();
}

void SceneViewPreprocessorChart::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundRed).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundGreen).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundBlue).toInt() / 255.0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // geometry
    paintGeometry();
    paintChartLine();
}

void SceneViewPreprocessorChart::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesBlue).toInt() / 255.0);
        glLineWidth(1.0);

        if (edge->isStraight())
        {
            glBegin(GL_LINES);
            glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
            glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
            glEnd();
        }
        else
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle());
        }
    }
}

void SceneViewPreprocessorChart::paintChartLine()
{
    loadProjection2d(true);

    RectPoint rect = Agros2D::scene()->boundingBox();
    double dm = qMax(rect.width(), rect.height()) / 25.0;

    glColor3d(1.0, 0.1, 0.1);
    glLineWidth(2.0);

    // line
    if (m_chartLine.start == m_chartLine.end)
    {
        glPointSize(5.0);
        glBegin(GL_POINTS);
        glVertex2d(m_chartLine.start.x, m_chartLine.start.y);
        glEnd();
    }
    else
    {
        glBegin(GL_LINES);
        glVertex2d(m_chartLine.start.x, m_chartLine.start.y);
        glVertex2d(m_chartLine.end.x, m_chartLine.end.y);
        glEnd();

        double angle = atan2(m_chartLine.end.y - m_chartLine.start.y,
                             m_chartLine.end.x - m_chartLine.start.x);

        // shaft for an arrow
        double vs1x = m_chartLine.end.x + dm / 2.5 * cos(angle + M_PI/2.0) - dm * cos(angle);
        double vs1y = m_chartLine.end.y + dm / 2.5 * sin(angle + M_PI/2.0) - dm * sin(angle);
        double vs2x = m_chartLine.end.x + dm / 2.5 * cos(angle - M_PI/2.0) - dm * cos(angle);
        double vs2y = m_chartLine.end.y + dm / 2.5 * sin(angle - M_PI/2.0) - dm * sin(angle);
        double vs3x = m_chartLine.end.x;
        double vs3y = m_chartLine.end.y;

        glLineWidth(1.0);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        glVertex2d(vs1x, vs1y);
        glVertex2d(vs2x, vs2y);
        glVertex2d(vs3x, vs3y);
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}
