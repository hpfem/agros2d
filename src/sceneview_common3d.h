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
    SceneViewCommon3D(PostHermes *postHermes, QWidget *parent = 0);
    ~SceneViewCommon3D();

    QAction *actSetProjectionXY;
    QAction *actSetProjectionXZ;
    QAction *actSetProjectionYZ;

    void loadProjection3d(bool setScene = false);

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
    void doZoomRegion(const Point &start, const Point &end) {}

    void paintBackground(); // gradient background
    void paintAxes(); // axes

private:
    void createActions();
};

#endif // SCENEVIEWCOMMON3D_H
