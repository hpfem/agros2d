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

#ifndef SCENEVIEWCOMMON2D_H
#define SCENEVIEWCOMMON2D_H

#include <QGLWidget>

#include "sceneview_post.h"

class SceneViewCommon2D : public SceneViewPostInterface
{
    Q_OBJECT

public slots:
    virtual void clear();

public:
    SceneViewCommon2D(QWidget *parent = 0);
    ~SceneViewCommon2D();

    SceneNode *findClosestNode(const Point &point);
    SceneEdge *findClosestEdge(const Point &point);
    SceneLabel *findClosestLabel(const Point &point);

    void doZoomRegion(const Point &start, const Point &end);

protected:
    double m_scale2d; // scale
    Point m_offset2d; // offset

    void loadProjection2d(bool setScene = false);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    void renderTextPos(double x, double y,
                       const QString &str, bool blend = true, QFont fnt = QFont(),
                       bool horizontal = true);

    void setZoom(double power);

    virtual void paintGL() = 0;

    void paintZoomRegion();
    void paintChartLine();

    void paintBackgroundPixmap(); // pixmap background
    void paintGrid(); // paint grid
    void paintAxes();  // paint axes
    void paintRulers(); // paint rulers
    void paintRulersHints();

    inline Point position(double x, double y) const { return position(Point(x, y)); }
    Point position(const Point &point) const;

    // rulers
    QFont fontLabel();
    Point rulersAreaWidth();
    double rulersNumbersWidth();

private slots:
    void updatePosition(const Point &point);
};

#endif // SCENEVIEWCOMMON2D_H
