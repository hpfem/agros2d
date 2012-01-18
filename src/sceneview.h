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
#include "chartdialog.h"

class Scene;
class SceneView;

class SceneNode;
class SceneEdge;
class SceneLabel;

class LocalPointValue;
class SurfaceIntegralValue;
class VolumeIntegralValue;

// scene view
SceneView *sceneView();

struct SceneViewSettings
{
    bool showGeometry;
    bool showInitialMesh;

    SceneViewPostprocessorShow postprocessorShow;

    bool showContours;
    bool showVectors;
    bool showParticleTracing;
    bool showSolutionMesh;

    // contour
    PhysicFieldVariable contourPhysicFieldVariable;

    // scalar view
    PhysicFieldVariable scalarPhysicFieldVariable;
    PhysicFieldVariableComp scalarPhysicFieldVariableComp;
    bool scalarRangeAuto;
    double scalarRangeMin;
    double scalarRangeMax;

    // vector view
    PhysicFieldVariable vectorPhysicFieldVariable;

    SceneViewSettings();

    void defaultValues();
};

class SceneView : public QGLWidget
{
    Q_OBJECT

public slots:
    void doZoomBestFit();
    void doZoomIn();
    void doZoomOut();
    void doZoomRegion(const Point &start, const Point &end);
    void doShowGrid();
    void doSnapToGrid();
    void doShowRulers();
    void doSceneObjectProperties();
    void doSceneModeSet(QAction *action);
    void doSelectMarker();
    void doSelectBasic();
    void doInvalidated();
    void solved();
    void doDefaultValues();
    void doSetChartLine(const ChartLine &chartLine);
    void doSetProjectionXY();
    void doSetProjectionXZ();
    void doSetProjectionYZ();

    void refresh();
    void timeStepChanged(bool showViewProgress = false);

    void processedSolutionMesh();
    void processedRangeContour();
    void processedRangeScalar();
    void processedRangeVector();

public:
    SceneView(QWidget *parent = 0);
    ~SceneView();

    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;
    QAction *actSceneZoomRegion;

    QAction *actSceneShowGrid;
    QAction *actSceneSnapToGrid;
    QAction *actSceneShowRulers;

    QActionGroup *actSceneModeGroup;
    QAction *actSceneModeNode;
    QAction *actSceneModeEdge;
    QAction *actSceneModeLabel;
    QAction *actSceneModePostprocessor;

    QAction *actSetProjectionXY;
    QAction *actSetProjectionXZ;
    QAction *actSetProjectionYZ;

    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

    QAction *actSceneViewSelectRegion;
    QAction *actSceneViewSelectMarker;
    QAction *actSceneObjectProperties;

    SceneNode *findClosestNode(const Point &point);
    SceneEdge *findClosestEdge(const Point &point);
    SceneLabel *findClosestLabel(const Point &point);

    inline SceneViewSettings &sceneViewSettings() { return m_sceneViewSettings; }
    inline SceneMode sceneMode() const { return m_sceneMode; }

    ErrorResult saveImageToFile(const QString &fileName, int w = 0, int h = 0);
    void saveImagesForReport(const QString &path, bool showRulers, bool showGrid, int w = 0, int h = 0);
    QPixmap renderScenePixmap(int w = 0, int h = 0, bool useContext = false);

    void loadBackgroundImage(const QString &fileName, double x = 0, double y = 0, double w = 1.0, double h = 1.0);

    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();

    void setSceneFont();

    inline bool is3DMode() const
    {
        return ((m_sceneMode == SceneMode_Postprocessor) &&
                (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
                 m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid ||
                 m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Model));
    }

signals:
    void mouseMoved(const QPointF &position);
    void mousePressed();
    void mousePressed(const Point &point);
    void sceneModeChanged(SceneMode sceneMode);
    void postprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor);
    void mouseSceneModeChanged(MouseSceneMode mouseSceneMode);

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

    inline int contextWidth() const { return context()->device()->width(); }
    inline int contextHeight() const { return context()->device()->height(); }
    inline double aspect() const { return (double) contextWidth() / (double) contextHeight(); }

private:
    Scene *m_scene;
    QMainWindow *m_mainWindow;

    QPointF m_lastPos; // last position of cursor
    QPointF m_regionPos;
    // 2d
    double m_scale2d; // scale
    Point m_offset2d; // offset
    // 3d
    double m_scale3d; // scale
    Point m_offset3d; // offset
    Point3 m_rotation3d; // rotation

    ChartLine m_chartLine; // line

    SceneNode *m_nodeLast;

    SceneMode m_sceneMode;
    SceneViewSettings m_sceneViewSettings;

    double m_texScale, m_texShift;

    // gl lists
    int m_listInitialMesh;
    int m_listSolutionMesh;
    int m_listContours;
    int m_listVectors;
    int m_listParticleTracing;
    int m_listScalarField;
    int m_listScalarField3D;
    int m_listScalarField3DSolid;
    int m_listOrder;
    int m_listModel;

    // helper for snap to grid
    bool m_snapToGrid;

    // helper for paint region
    bool m_region;

    // solution is prepared for paint (after solve)
    bool m_isSolutionPrepared;

    // background image
    QImage m_backgroundImage;
    int m_backgroundTexture;
    QRectF m_backgroundPosition;

    QMenu *mnuScene;

    QActionGroup *actMaterialGroup;
    QActionGroup *actBoundaryGroup;
    QActionGroup *actPostprocessorModeGroup;
    QActionGroup *actShowGroup;

    QAction *actShowContours;
    QAction *actShowVectors;
    QAction *actShowParticleTracing;
    QAction *actShowSolutionMesh;

    void createActions();
    void createMenu();

    // palette
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate();
    void paletteFilter();
    void paletteUpdateTexAdjust();

    void initLighting();

    void paintBackgroundPixmap(); // pixmap background
    void paintBackground(); // gradient background
    void paintGrid(); // paint grid
    void paintAxes();  // paint axes
    void paintRulers(); // paint rulers
    void paintGeometry(); // paint nodes, edges and labels
    void paintInitialMesh();

    void paintContours(); // paint scalar field contours
    void paintContoursTri(double3* vert, int3* tri, double step);
    void paintVectors(); // paint vector field vectors
    void paintParticleTracing();
    void paintSolutionMesh();

    void paintScalarField(); // paint scalar field surface
    void paintScalarField3D(); // paint scalar field 3d surface
    void paintScalarField3DSolid(); // paint scalar field 3d solid
    void paintScalarField3DSolid_TODOParticle(); // TODO - temporary
    void paintScalarFieldColorBar(double min, double max);
    void paintOrder();
    void paintOrderColorBar();

    void paintSceneModeLabel();
    void paintZoomRegion();
    void paintSnapToGrid();
    void paintChartLine();
    void paintEdgeLine();

    void paintPostprocessorSelectedVolume(); // paint selected volume for integration
    void paintPostprocessorSelectedSurface(); // paint selected surface for integration

    void drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments) const;
    void drawBlend(Point start, Point end, double red = 1.0, double green = 1.0, double blue = 1.0, double alpha = 0.75) const;

    void setZoom(double power);
    void selectRegion(const Point &start, const Point &end);

    void loadProjection2d(bool setScene = false) const;
    void loadProjection3d(bool setScene = false) const;

    inline Point position(double x, double y) const
    {
        return position(Point(x, y));
    }

    inline Point position(const Point &point) const
    {
        return Point((2.0/contextWidth()*point.x-1)/m_scale2d*aspect()+m_offset2d.x,
                     -(2.0/contextHeight()*point.y-1)/m_scale2d+m_offset2d.y);
    }

private slots:
    void doMaterialGroup(QAction *action);
    void doBoundaryGroup(QAction *action);
    void doShowGroup(QAction *action);
    void doPostprocessorModeGroup(QAction *action);
    void clearGLLists();
};

#endif // SCENEVIEW_H
