// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SCENEVIEWCOMMON3D_H
#define SCENEVIEWCOMMON3D_H

#include <QGLWidget>

#include "sceneview_post.h"

class SceneViewCommon3D : public SceneViewPostInterface
{
    Q_OBJECT

public slots:
    virtual void clear();

    void doSetProjectionXY();
    void doSetProjectionXZ();
    void doSetProjectionYZ();

public:
    SceneViewCommon3D(PostDeal *postDeal, QWidget *parent = 0);
    ~SceneViewCommon3D();

    QAction *actSetProjectionXY;
    QAction *actSetProjectionXZ;
    QAction *actSetProjectionYZ;

    void loadProjection3d(bool setScene = false, bool plane = true);

protected:
    double m_scale3d; // scale
    Point m_offset3d; // offset
    Point3 m_rotation3d; // rotation

    QMenu *mnuView3D;

    void createMenu();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    void setZoom(double power);
    void initLighting();

    virtual void paintGL() = 0;
    void doZoomRegion(const Point &start, const Point &end);

    void paintBackground(); // gradient background
    void paintAxes(); // axes

private:
    void createActions();
};

#endif // SCENEVIEWCOMMON3D_H
