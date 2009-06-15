#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QGLWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QWheelEvent>
#include <QTimer>
#include <QPainter>
#include <QAction>
#include <QActionGroup>
#include <QMenu>

#include <math.h>
#include <iostream>

#include "util.h"

#include "sceneviewdialog.h"
#include "scenebasic.h"
#include "localvalueview.h"
#include "volumeintegralview.h"
#include "surfaceintegralview.h"
#include "scene.h"

#include "sceneview_data.h"

class SceneNode;
class SceneEdge;
class SceneLabel;

class LocalPointValue;
class VolumeIntegralValue;
class SurfaceIntegralValue;

class SceneView : public QGLWidget
{
    Q_OBJECT
    
public slots:
    void doZoomBestFit();
    void doZoomIn();
    void doZoomOut();
    void doZoomRegion(const Point &start, const Point &end);
    void doSceneViewProperties();
    void doSceneModeSet(QAction *);
    void doInvalidated();
    void doSolved();
    void doDefaults();
    void doRefresh();

public:
    SceneView(Scene *scene, QWidget *parent = 0);
    ~SceneView();

    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;
    QAction *actSceneZoomRegion;

    QAction *actSceneShowMesh;
    QAction *actSceneShowContours;
    QAction *actSceneShowScalarField;
    QAction *actSceneShowVectors;

    QActionGroup *actSceneModeGroup;
    QAction *actSceneModeNode;
    QAction *actSceneModeEdge;
    QAction *actSceneModeLabel;
    QAction *actSceneModePostprocessor;

    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

    QAction *actSceneViewSelectRegion;
    QAction *actSceneViewProperties;

    inline Scene *scene() { return m_scene; }
    inline SceneViewSettings &sceneViewSettings() { return m_sceneViewSettings; }
    inline SceneMode sceneMode() { return m_sceneMode; }

    void saveImageToFile(const QString &fileName);

signals:
    void mouseMoved(const QPointF &position);
    void mousePressed(LocalPointValue *localPointValue);
    void mousePressed(VolumeIntegralValue *volumeIntegralValue);
    void mousePressed(SurfaceIntegralValue *surfaceIntegralValue);
    
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void setupViewport(int width, int height);
    
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QPointF m_lastPos; // last position of cursor
    QPointF m_regionPos;
    double m_scale; // scale
    double m_drawScaleX;
    double m_drawScaleY;
    Point m_offset; // offset

    SceneNode *m_nodeLast;

    Scene *m_scene;
    SceneMode m_sceneMode;
    SceneViewSettings m_sceneViewSettings;

    double m_texScale, m_texShift;

    QMenu *mnuInfo;
    QMenu *mnuMarkerGroup;
    QMenu *mnuShowGroup;

    QActionGroup *actMaterialGroup;
    QActionGroup *actBoundaryGroup;
    QActionGroup *actPostprocessorModeGroup;
    QActionGroup *actShowGroup;

    QAction *actShowSolutionMesh;
    QAction *actShowContours;
    QAction *actShowScalarField;
    QAction *actShowVectors;

    void createActions();
    void createMenu();

    SceneNode *findClosestNode(const Point &point);
    SceneEdge *findClosestEdge(const Point &point);
    SceneLabel *findClosestLabel(const Point &point);

    // palette
    const float* paletteColor(double x);
    void paletteCreate();
    void paletteFilter();
    void paletteUpdateTexAdjust();

    void paintGrid(); // paint grid
    void paintGeometry(); // paint nodes, edges and labels
    void paintScalarField(); // paint scalar field surface
    void paintContours(); // paint scalar field contours
    void paintContoursTri(double3* vert, int3* tri, double step);
    void paintVectors(); // paint vector field vectors
    void paintInitialMesh();
    void paintSolutionMesh();
    void paintOrder();
    void paintColorBar(double min, double max);
    void paintSceneModeLabel();

    void paintPostprocessorSelectedVolume(); // paint selected volume for integration
    void paintPostprocessorSelectedSurface(); // paint selected surface for integration

    void drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments);
    void setZoom(double power);
    void setRangeContour();
    void setRangeScalar();
    void setRangeVector();
    void selectRegion(const Point &start, const Point &end);
    inline Point &position(const Point &point) { Point p((2.0/width()*point.x-1)/m_scale*m_drawScaleY+m_offset.x, -(2.0/height()*point.y-1)/m_scale*m_drawScaleX+m_offset.y); return p; }  // cursor position

private slots:
    void doMaterialGroup(QAction *action);
    void doBoundaryGroup(QAction *action);
    void doShowGroup(QAction *action);
    void doPostprocessorModeGroup(QAction *action);
};

#endif // SCENEVIEW_H
