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

#ifndef SCENEVIEWCOMMON_H
#define SCENEVIEWCOMMON_H

#include <QGLWidget>

#include "util.h"
#include "chartdialog.h"

#include "gui/scenewidget.h"
#include "util/glfont.h"

class Scene;
class SceneViewCommon;

class SceneNode;
class SceneEdge;
class SceneLabel;

class SceneViewInterface;

namespace Module
{
    struct LocalVariable;
}

class SceneViewCommon : public QGLWidget
{
    Q_OBJECT

public slots:
    void doZoomBestFit();
    void doZoomIn();
    void doZoomOut();
    virtual void doZoomRegion(const Point &start, const Point &end) = 0;

    virtual void refresh();
    virtual void clear();

public:
    SceneViewCommon(QWidget *parent = 0);
    ~SceneViewCommon();

    QAction *actSceneZoomRegion;

    ErrorResult saveImageToFile(const QString &fileName, int w = 0, int h = 0);
    void saveImagesForReport(const QString &path, bool showGrid, bool showRulers, bool showAxes, bool showLabel, int w = 0, int h = 0);
    QPixmap renderScenePixmap(int w = 0, int h = 0, bool useContext = false);

    virtual QIcon iconView() { return QIcon(); }
    virtual QString labelView() { return ""; }

signals:
    void mouseMoved(const Point &position);
    void mousePressed();
    void mousePressed(const Point &point);
    void postprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor);
    void mouseSceneModeChanged(MouseSceneMode mouseSceneMode);

    void labelCenter(const QString &center);
    void labelRight(const QString &right);

protected:
    QMainWindow *m_mainWindow;

    QPoint m_lastPos; // last position of cursor
    SceneNode *m_nodeLast;

    TextureFont *m_fontRulers;
    TextureFont *m_fontPost;

    // helper for zoom region
    bool m_zoomRegion;
    QPointF m_zoomRegionPos;

    QActionGroup *actMaterialGroup;
    QActionGroup *actBoundaryGroup;

    void createActions();

    void drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments = -1) const;
    void drawBlend(Point start, Point end, double red = 1.0, double green = 1.0, double blue = 1.0, double alpha = 0.75) const;

    void printAt(int penX, int penY, const QString &text, const TextureFont *fnt);
    void initFont(int textureID, const TextureFont *fnt);

    GLuint m_textureLabelRulers;
    GLuint m_textureLabelPost;

    void printRulersAt(int penX, int penY, const QString &text);
    void printPostAt(int penX, int penY, const QString &text);

    virtual void setZoom(double power) = 0;

    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL() = 0;
    void setupViewport(int w, int h);
    void loadProjectionViewPort();

    void closeEvent(QCloseEvent *event);

    inline double aspect() const { return (double) width() / (double) height(); }

private slots:
    void doMaterialGroup(QAction *action);
    void doBoundaryGroup(QAction *action);    
};

#endif // SCENEVIEWCOMMON_H
