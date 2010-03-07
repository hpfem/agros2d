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

#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QGLWidget>

#include "util.h"
#include "scene.h"

#include "sceneviewdialog.h"
#include "scenebasic.h"

class Scene;

class SceneNode;
class SceneEdge;
class SceneLabel;

class LocalPointValue;
class SurfaceIntegralValue;
class VolumeIntegralValue;

class SceneViewSettings
{
public:
    bool showGeometry;
    bool showGrid;
    bool showRulers;
    bool showInitialMesh;

    SceneViewPostprocessorShow postprocessorShow;

    bool showContours;
    bool showVectors;
    bool showSolutionMesh;

    // geometry
    double geometryNodeSize;
    double geometryEdgeWidth;
    double geometryLabelSize;

    // grid
    double gridStep;
    bool snapToGrid;

    // contour
    int contoursCount;
    PhysicFieldVariable contourPhysicFieldVariable;

    // scalar view
    PhysicFieldVariable scalarPhysicFieldVariable;
    PhysicFieldVariableComp scalarPhysicFieldVariableComp;
    bool scalarRangeAuto;
    double scalarRangeMin;
    double scalarRangeMax;
    bool scalarRangeLog;
    double scalarRangeBase;

    // vector view
    PhysicFieldVariable vectorPhysicFieldVariable;
    bool vectorRangeAuto;
    double vectorRangeMin;
    double vectorRangeMax;

    // 3d
    bool scalarView3DLighting;

    // palette
    PaletteType paletteType;
    int paletteSteps;
    bool paletteFilter;

    // colors
    QColor colorBackground;
    QColor colorGrid;
    QColor colorCross;
    QColor colorNodes;
    QColor colorEdges;
    QColor colorLabels;
    QColor colorContours;
    QColor colorVectors;
    QColor colorInitialMesh;
    QColor colorSolutionMesh;
    QColor colorHighlighted;
    QColor colorSelected;    

    SceneViewSettings();

    void defaultValues();
    void load();
    void save();
};

class SceneView : public QGLWidget
{
    Q_OBJECT
    
public slots:
    void doZoomBestFit();
    void doZoomIn();
    void doZoomOut();
    void doZoomRegion(const Point &start, const Point &end);
    void doSceneViewProperties();
    void doSceneObjectProperties();
    void doSceneModeSet(QAction *);
    void doSelectMarker();
    void doInvalidated();
    void doSolved();
    void doDefaultValues();
    void doRefresh();
    void doSetChartLine(const Point &start, const Point &end);

public:
    SceneView(QWidget *parent = 0);
    ~SceneView();

    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;
    QAction *actSceneZoomRegion;

    QActionGroup *actSceneModeGroup;
    QAction *actSceneModeNode;
    QAction *actSceneModeEdge;
    QAction *actSceneModeLabel;
    QAction *actSceneModePostprocessor;

    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

    QAction *actSceneViewSelectRegion;
    QAction *actSceneViewSelectMarker;
    QAction *actSceneViewProperties;
    QAction *actSceneObjectProperties;

    SceneNode *findClosestNode(const Point &point);
    SceneEdge *findClosestEdge(const Point &point);
    SceneLabel *findClosestLabel(const Point &point);

    inline SceneViewSettings &sceneViewSettings() { return m_sceneViewSettings; }
    inline SceneMode sceneMode() { return m_sceneMode; }

    ErrorResult saveImageToFile(const QString &fileName, int w = 0, int h = 0);
    void saveImagesForReport(const QString &path, int w = 0, int h = 0);

signals:
    void mouseMoved(const QPointF &position);
    void mousePressed();
    void mousePressed(const Point &point);    
    
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void setupViewport(int w, int h);
    void renderTextPos(double x, double y, double z, const QString &str, bool blend = true);
    
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);

    inline int contextWidth() { return context()->device()->width(); }
    inline int contextHeight() { return context()->device()->height(); }

private:
    Scene *m_scene;
    QMainWindow *m_mainWindow;

    QPointF m_lastPos; // last position of cursor
    QPointF m_regionPos;
    double m_scale; // scale
    double m_aspect;
    Point3 m_offset; // offset
    Point m_rotation; // rotation
    double3* m_normals;

    RectPoint m_chartLine; // line

    SceneNode *m_nodeLast;

    SceneMode m_sceneMode;
    SceneViewSettings m_sceneViewSettings;

    double m_texScale, m_texShift;

    // helper for snap to grid
    bool m_snapToGrid;

    // helper for paint region
    bool m_region;

    // solution is prepared for paint (after solve)
    bool m_isSolutionPrepared;

    QMenu *mnuInfo;

    QActionGroup *actMaterialGroup;
    QActionGroup *actBoundaryGroup;
    QActionGroup *actPostprocessorModeGroup;
    QActionGroup *actShowGroup;

    QAction *actShowContours;
    QAction *actShowVectors;
    QAction *actShowSolutionMesh;

    void createActions();
    void createMenu();

    // palette
    const float *paletteColor(double x);
    void paletteCreate();
    void paletteFilter();
    void paletteUpdateTexAdjust();

    void paintGrid(); // paint grid
    void paintRulers(); // paint rulers
    void paintGeometry(); // paint nodes, edges and labels
    void paintInitialMesh();

    void paintContours(); // paint scalar field contours
    void paintContoursTri(double3* vert, int3* tri, double step);
    void paintVectors(); // paint vector field vectors
    void paintSolutionMesh();

    void paintScalarField(); // paint scalar field surface
    void paintScalarField3D(); // paint scalar field 3d surface
    void paintScalarField3DSolid(); // paint scalar field 3d solid
    void paintOrder();

    void paintColorBar(double min, double max);
    void paintSceneModeLabel();
    void paintZoomRegion();
    void paintSnapToGrid();
    void paintChartLine();

    void paintPostprocessorSelectedVolume(); // paint selected volume for integration
    void paintPostprocessorSelectedSurface(); // paint selected surface for integration

    void drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments);
    void drawBlend(Point start, Point end, double red = 1.0, double green = 1.0, double blue = 1.0, double alpha = 0.75);

    void setZoom(double power);
    void setRangeContour();
    void setRangeScalar();
    void setRangeVector();
    void selectRegion(const Point &start, const Point &end);
    inline Point &position(const Point &point) { Point p((2.0/contextWidth()*point.x-1)/m_scale*m_aspect+m_offset.x, -(2.0/contextHeight()*point.y-1)/m_scale+m_offset.y); return p; }

private slots:
    void doMaterialGroup(QAction *action);
    void doBoundaryGroup(QAction *action);
    void doShowGroup(QAction *action);
    void doPostprocessorModeGroup(QAction *action);
    void doProcessSolution();
};

#endif // SCENEVIEW_H
