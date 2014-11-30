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

#ifndef SCENEVIEWGEOMETRY_H
#define SCENEVIEWGEOMETRY_H

#include "util.h"
#include "util/loops.h"

#include "sceneview_common2d.h"

class SceneViewPreprocessor : public SceneViewCommon2D
{
    Q_OBJECT
signals:
    void sceneGeometryModeChanged(SceneGeometryMode sceneMode);

public slots:
    virtual void clear();
    virtual void refresh();
    void doSceneGeometryModeSet(QAction *action);
    void doSelectBasic();
    void doSceneObjectProperties();
    void doSceneEdgeSwapDirection();

public:
    SceneViewPreprocessor(QWidget *parent = 0);
    ~SceneViewPreprocessor();

    QAction *actSceneViewSelectRegion;

    QAction *actSceneModePreprocessor;

    QActionGroup *actOperateGroup;
    QAction *actOperateOnNodes;
    QAction *actOperateOnEdges;
    QAction *actOperateOnLabels;

    QAction *actSceneObjectProperties;
    QAction *actSceneEdgeSwapDirection;

    // background image
    QImage m_backgroundImage;
    int m_backgroundTexture;
    QRectF m_backgroundPosition;

    inline SceneGeometryMode sceneMode() const { return m_sceneMode; }
    void saveGeometryToSvg(const QString &fileName);

    void loadBackgroundImage(const QString &fileName, double x, double y, double w, double h);

    virtual QIcon iconView() { return icon("scene-geometry"); }
    virtual QString labelView() { return tr("Geometry editor"); }

protected:
    SceneGeometryMode m_sceneMode;

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual void contextMenuEvent(QContextMenuEvent *event);

    void selectRegion(const Point &start, const Point &end);

    void paintGL();

    void paintEdgeLine();
    void paintSnapToGrid();

    void paintGeometry(); // paint nodes, edges and labels
    void paintRulersHintsEdges();

    void paintSelectRegion();

    void paintBackgroundPixmap();

private:
    QMenu *mnuScene;

    // helper for snap to grid
    bool m_snapToGrid;

    // helper for zoom region
    bool m_selectRegion;
    QPointF m_selectRegionPos;

    void createActionsGeometry();
    void createMenuGeometry();
};

#endif // SCENEVIEWGEOMETRY_H
