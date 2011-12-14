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

#include "sceneview.h"
#include "sceneview_data.h"
#include "scenesolution.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"

#include "../lib/gl2ps/gl2ps.h"

// scene view
static SceneView *m_sceneView = NULL;

SceneView *sceneView()
{
    return m_sceneView;
}

static void computeNormal(double p0x, double p0y, double p0z,
                          double p1x, double p1y, double p1z,
                          double p2x, double p2y, double p2z,
                          double* normal)
{
    double ax = (p1x - p0x);
    double ay = (p1y - p0y);
    double az = (p1z - p0z);

    double bx = (p2x - p0x);
    double by = (p2y - p0y);
    double bz = (p2z - p0z);

    normal[0] = ay * bz - az * by;
    normal[1] = az * bx - ax * bz;
    normal[2] = ax * by - ay * bx;

    // normalize
    // double l = 1.0 / sqrt(sqr(nx) + sqr(ny) + sqr(nz));
    // double p[3] = { nx*l, ny*l, nz*l };
}

// *******************************************************************************************************

SceneViewSettings::SceneViewSettings()
{
    logMessage("SceneViewSettings::SceneViewSettings()");

    defaultValues();
}

void SceneViewSettings::defaultValues()
{
    logMessage("SceneViewSettings::defaultValues()");

    scalarRangeMin =  numeric_limits<double>::max();
    scalarRangeMax = -numeric_limits<double>::max();

    // visible objects
    showGeometry = true;
    showInitialMesh = false;

    postprocessorShow = SceneViewPostprocessorShow_ScalarView;

    showContours = false;
    showVectors = false;
    showSolutionMesh = false;

    //TODO - allow "no field"
    QString firstField = Util::scene()->fieldInfos().keys().at(0);

    contourPhysicFieldVariable = Util::scene()->fieldInfo(firstField)->module()->view_default_scalar_variable->id;

    scalarPhysicFieldVariable = Util::scene()->fieldInfo(firstField)->module()->view_default_scalar_variable->id;
    scalarPhysicFieldVariableComp = Util::scene()->fieldInfo(firstField)->module()->view_default_scalar_variable_comp();
    scalarRangeAuto = true;

    vectorPhysicFieldVariable = Util::scene()->fieldInfo(firstField)->module()->view_default_vector_variable->id;
}

// *******************************************************************************************************

SceneView::SceneView(QWidget *parent): QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_listInitialMesh(-1),
    m_listSolutionMesh(-1),
    m_listContours(-1),
    m_listVectors(-1),
    m_listScalarField(-1),
    m_listScalarField3D(-1),
    m_listScalarField3DSolid(-1),
    m_listOrder(-1),
    m_listModel(-1)
{
    logMessage("SceneView::SceneView()");

    m_sceneView = this;
    m_mainWindow = (QMainWindow *) parent;
    m_scene = Util::scene();

    connect(m_scene->sceneSolution(), SIGNAL(timeStepChanged(bool)), this, SLOT(timeStepChanged(bool)));
    connect(m_scene->sceneSolution(), SIGNAL(solved()), this, SLOT(solved()));
    connect(m_scene->sceneSolution(), SIGNAL(processedRangeContour()), this, SLOT(processedRangeContour()));
    connect(m_scene->sceneSolution(), SIGNAL(processedRangeScalar()), this, SLOT(processedRangeScalar()));
    connect(m_scene->sceneSolution(), SIGNAL(processedRangeVector()), this, SLOT(processedRangeVector()));
    connect(m_scene->sceneSolution(), SIGNAL(meshed()), this, SLOT(clearGLLists()));

    connect(m_scene, SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(m_scene, SIGNAL(defaultValues()), this, SLOT(doDefaultValues()));

    createActions();
    createMenu();

    doDefaultValues();

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setSceneFont();

    setMinimumSize(400, 400);
}

SceneView::~SceneView()
{
    logMessage("SceneView::~SceneView()");

}

void SceneView::createActions()
{
    logMessage("SceneView::createActions()");

    // scene - zoom
    actSceneZoomIn = new QAction(icon("zoom-in"), tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);
    actSceneZoomIn->setStatusTip(tr("Zoom in"));
    connect(actSceneZoomIn, SIGNAL(triggered()), this, SLOT(doZoomIn()));

    actSceneZoomOut = new QAction(icon("zoom-out"), tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);
    actSceneZoomOut->setStatusTip(tr("Zoom out"));
    connect(actSceneZoomOut, SIGNAL(triggered()), this, SLOT(doZoomOut()));

    actSceneZoomBestFit = new QAction(icon("zoom-original"), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));
    actSceneZoomBestFit->setStatusTip(tr("Best fit"));
    connect(actSceneZoomBestFit, SIGNAL(triggered()), this, SLOT(doZoomBestFit()));

    actSceneZoomRegion = new QAction(icon("zoom-fit-best"), tr("Zoom region"), this);
    actSceneZoomRegion->setStatusTip(tr("Zoom region"));
    actSceneZoomRegion->setCheckable(true);

    // scene - grid
    actSceneShowGrid = new QAction(icon("grid"), tr("Show grid"), this);
    actSceneShowGrid->setStatusTip(tr("Show grid"));
    actSceneShowGrid->setCheckable(true);
    actSceneShowGrid->setChecked(Util::config()->showGrid);
    connect(actSceneShowGrid, SIGNAL(triggered()), this, SLOT(doShowGrid()));

    actSceneSnapToGrid = new QAction(icon("snap-to-grid"), tr("Snap to grid"), this);
    actSceneSnapToGrid->setStatusTip(tr("Snap to grid"));
    actSceneSnapToGrid->setCheckable(true);
    actSceneSnapToGrid->setChecked(Util::config()->snapToGrid);
    connect(actSceneSnapToGrid, SIGNAL(triggered()), this, SLOT(doSnapToGrid()));

    actSceneShowRulers = new QAction(icon("rulers"), tr("Show rulers"), this);
    actSceneShowRulers->setStatusTip(tr("Show rulers"));
    actSceneShowRulers->setCheckable(true);
    actSceneShowRulers->setChecked(Util::config()->showRulers);
    connect(actSceneShowRulers, SIGNAL(triggered()), this, SLOT(doShowRulers()));

    // scene - operate on items
    actSceneModeNode = new QAction(icon("scene-node"), tr("Operate on &nodes"), this);
    actSceneModeNode->setShortcut(Qt::Key_F5);
    actSceneModeNode->setStatusTip(tr("Operate on nodes"));
    actSceneModeNode->setCheckable(true);

    actSceneModeEdge = new QAction(icon("scene-edge"), tr("Operate on &edges"), this);
    actSceneModeEdge->setShortcut(Qt::Key_F6);
    actSceneModeEdge->setStatusTip(tr("Operate on edges"));
    actSceneModeEdge->setCheckable(true);

    actSceneModeLabel = new QAction(icon("scene-label"), tr("Operate on &labels"), this);
    actSceneModeLabel->setShortcut(Qt::Key_F7);
    actSceneModeLabel->setStatusTip(tr("Operate on labels"));
    actSceneModeLabel->setCheckable(true);

    actSceneModePostprocessor = new QAction(icon("scene-postprocessor"), tr("&Postprocessor"), this);
    actSceneModePostprocessor->setShortcut(Qt::Key_F8);
    actSceneModePostprocessor->setStatusTip(tr("Postprocessor"));
    actSceneModePostprocessor->setCheckable(true);

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(actSceneModeNode);
    actSceneModeGroup->addAction(actSceneModeEdge);
    actSceneModeGroup->addAction(actSceneModeLabel);
    actSceneModeGroup->addAction(actSceneModePostprocessor);
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doSceneModeSet(QAction *)));

    // projection
    actSetProjectionXY = new QAction(tr("Projection to %1%2").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelY()), this);
    actSetProjectionXY->setStatusTip(tr("Projection to %1%2 plane.").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelY()));
    connect(actSetProjectionXY, SIGNAL(triggered()), this, SLOT(doSetProjectionXY()));

    actSetProjectionXZ = new QAction(tr("Projection to %1%2").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelZ()), this);
    actSetProjectionXZ->setStatusTip(tr("Projection to %1%2 plane.").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelZ()));
    connect(actSetProjectionXZ, SIGNAL(triggered()), this, SLOT(doSetProjectionXZ()));

    actSetProjectionYZ = new QAction(tr("Projection to %1%2").arg(Util::scene()->problemInfo()->labelY()).arg(Util::scene()->problemInfo()->labelZ()), this);
    actSetProjectionYZ->setStatusTip(tr("Projection to %1%2 plane.").arg(Util::scene()->problemInfo()->labelY()).arg(Util::scene()->problemInfo()->labelZ()));
    connect(actSetProjectionYZ, SIGNAL(triggered()), this, SLOT(doSetProjectionYZ()));

    // material
    actMaterialGroup = new QActionGroup(this);
    connect(actMaterialGroup, SIGNAL(triggered(QAction *)), this, SLOT(doMaterialGroup(QAction *)));

    // boundary
    actBoundaryGroup = new QActionGroup(this);
    connect(actBoundaryGroup, SIGNAL(triggered(QAction *)), this, SLOT(doBoundaryGroup(QAction *)));

    // show
    actShowSolutionMesh = new QAction(tr("Solution mesh"), this);
    actShowSolutionMesh->setCheckable(true);

    actShowContours = new QAction(tr("Contours"), this);
    actShowContours->setCheckable(true);

    actShowVectors = new QAction(tr("Vectors"), this);
    actShowVectors->setCheckable(true);

    actShowGroup = new QActionGroup(this);
    actShowGroup->setExclusive(false);
    connect(actShowGroup, SIGNAL(triggered(QAction *)), this, SLOT(doShowGroup(QAction *)));
    actShowGroup->addAction(actShowSolutionMesh);
    actShowGroup->addAction(actShowContours);
    actShowGroup->addAction(actShowVectors);

    // postprocessor group
    actPostprocessorModeLocalPointValue = new QAction(icon("mode-localpointvalue"), tr("Local Values"), this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("mode-surfaceintegral"), tr("Surface Integrals"), this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("mode-volumeintegral"), tr("Volume Integrals"), this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));

    // object properties
    actSceneObjectProperties = new QAction(icon("scene-properties"), tr("Object properties"), this);
    actSceneObjectProperties->setShortcut(Qt::Key_Space);
    connect(actSceneObjectProperties, SIGNAL(triggered()), this, SLOT(doSceneObjectProperties()));

    // select region
    actSceneViewSelectRegion = new QAction(icon("scene-select-region"), tr("&Select region"), this);
    actSceneViewSelectRegion->setStatusTip(tr("Select region"));
    actSceneViewSelectRegion->setCheckable(true);

    actSceneViewSelectMarker = new QAction(icon(""), tr("Select by marker"), this);
    actSceneViewSelectMarker->setStatusTip(tr("Select by marker"));
    connect(actSceneViewSelectMarker, SIGNAL(triggered()), this, SLOT(doSelectMarker()));
}

void SceneView::createMenu()
{
    logMessage("SceneView::createMenu()");

    mnuScene = new QMenu(this);

    /*
    QMenu *mnuModeGroup = new QMenu(tr("Mode"), this);
    mnuModeGroup->addAction(actSceneModeNode);
    mnuModeGroup->addAction(actSceneModeEdge);
    mnuModeGroup->addAction(actSceneModeLabel);
    mnuModeGroup->addAction(actSceneModePostprocessor);
    */

    //mnuScene->addAction() (m_scene->actNewNode);
    mnuScene->addAction(m_scene->actNewEdge);
    mnuScene->addAction(m_scene->actNewLabel);
    mnuScene->addSeparator();
    Util::scene()->addBdrAndMatMenuItems(mnuScene, this);
    mnuScene->addSeparator();
    mnuScene->addAction(actSceneViewSelectRegion);
    mnuScene->addAction(m_scene->actTransform);
    //mnuScene->addSeparator();
    //mnuScene->addMenu(mnuModeGroup);
    mnuScene->addSeparator();
    mnuScene->addAction(actSceneObjectProperties);
    //mnuScene->addAction(m_scene->actProblemProperties);
}

void SceneView::initializeGL()
{
    logMessage("SceneView::initializeGL()");

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    clearGLLists();
}

void SceneView::resizeGL(int w, int h)
{
    logMessage("SceneView::resizeGL()");

    setupViewport(w, h);

    if (Util::scene()->sceneSolution()->isSolved() && m_sceneMode == SceneMode_Postprocessor)
    {
        paletteFilter();
        paletteUpdateTexAdjust();
        paletteCreate();
    }
}

void SceneView::loadProjection2d(bool setScene) const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-0.5, 0.5, -0.5, 0.5, -10.0, -10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (setScene)
    {
        glScaled(m_scale2d/aspect(), m_scale2d, m_scale2d);

        glTranslated(-m_offset2d.x, -m_offset2d.y, 0.0);
    }
}

void SceneView::loadProjection3d(bool setScene) const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 0.5, -0.5, 0.5, 4.0, 15.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (setScene)
    {
        glScaled(1.0/aspect(), 1.0, 0.1);

        // move to origin
        glTranslated(-m_offset3d.x, -m_offset3d.y, 1.0);

        glRotated(m_rotation3d.x, 1.0, 0.0, 0.0);
        glRotated(m_rotation3d.z, 0.0, 1.0, 0.0);
        glRotated(m_rotation3d.y, 0.0, 0.0, 1.0);

        RectPoint rect = Util::scene()->boundingBox();
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D)
        {
            glTranslated(- m_scale3d * (rect.start.x + rect.end.x) / 2.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
        }
        else
        {
            if (Util::scene()->problemInfo()->coordinateType == CoordinateType_Planar)
            {
                glTranslated(- m_scale3d * (rect.start.x + rect.end.x) / 2.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
            }
            else
            {
                glTranslated(0.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
            }
        }

        glScaled(m_scale3d, m_scale3d, m_scale3d);
    }
}

void SceneView::setupViewport(int w, int h)
{
    logMessage("SceneView::setupViewport()");

    glViewport(0, 0, w, h);
}

QPixmap SceneView::renderScenePixmap(int w, int h, bool useContext)
{
    logMessage("SceneView::renderScenePixmap()");

    QPixmap pixmap = renderPixmap(w, h, useContext);

    resizeGL(contextWidth(), contextHeight());

    return pixmap;
}

void SceneView::paintGL()
{
    logMessage("SceneView::paintGL()");

    glClearColor(Util::config()->colorBackground.redF(),
                 Util::config()->colorBackground.greenF(),
                 Util::config()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // if (Util::scene()->sceneSolution()->isSolving())
    //    return;

    if (is3DMode())
    {
        if (m_scene->sceneSolution()->isMeshed() && (m_sceneMode == SceneMode_Postprocessor))
        {
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Model) paintScalarField3DSolid();
        }

        if (m_scene->sceneSolution()->isSolved() && (m_sceneMode == SceneMode_Postprocessor))
        {
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D) paintScalarField3D();
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid) paintScalarField3DSolid();

            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
                    m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
                paintScalarFieldColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
        }
    }
    else
    {
        glDisable(GL_DEPTH_TEST);

        // background
        // if (Util::config()->showGrid)
        paintBackgroundPixmap();

        // grid
        if (Util::config()->showGrid) paintGrid();

        // view
        if (m_scene->sceneSolution()->isSolved() && (m_sceneMode == SceneMode_Postprocessor))
        {
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView) paintScalarField();
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Order) paintOrder();

            if (m_sceneViewSettings.showContours) paintContours();
            if (m_sceneViewSettings.showVectors) paintVectors();
            if (m_sceneViewSettings.showSolutionMesh) paintSolutionMesh();
        }

        // initial mesh
        if (m_sceneViewSettings.showInitialMesh) paintInitialMesh();

        // geometry
        if (m_sceneViewSettings.showGeometry) paintGeometry();

        if (m_scene->sceneSolution()->isSolved() && (m_sceneMode == SceneMode_Postprocessor))
        {
            if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
            if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();

            // bars
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView) paintScalarFieldColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Order) paintOrderColorBar();
        }

        // rulers
        if (Util::config()->showRulers) paintRulers();

        // axes
        if (Util::config()->showAxes) paintAxes();

        paintZoomRegion();
        paintSnapToGrid();
        paintEdgeLine();
        paintChartLine();
    }

    if (Util::config()->showLabel) paintSceneModeLabel();
}

void SceneView::clearGLLists()
{
    logMessage("SceneView::clearGLLists()");

    if (m_listInitialMesh != -1) glDeleteLists(m_listInitialMesh, 1);
    if (m_listSolutionMesh != -1) glDeleteLists(m_listSolutionMesh, 1);
    if (m_listContours != -1) glDeleteLists(m_listContours, 1);
    if (m_listVectors != -1) glDeleteLists(m_listVectors, 1);
    if (m_listScalarField != -1) glDeleteLists(m_listScalarField, 1);
    if (m_listScalarField3D != -1) glDeleteLists(m_listScalarField3D, 1);
    if (m_listScalarField3DSolid != -1) glDeleteLists(m_listScalarField3DSolid, 1);
    if (m_listOrder != -1) glDeleteLists(m_listOrder, 1);
    if (m_listModel != -1) glDeleteLists(m_listModel, 1);

    m_listInitialMesh = -1;
    m_listSolutionMesh = -1;
    m_listContours = -1;
    m_listVectors = -1;
    m_listScalarField = -1;
    m_listScalarField3D = -1;
    m_listScalarField3DSolid = -1;
    m_listOrder = -1;
    m_listModel = -1;
}

// paint *****************************************************************************************************************************

void SceneView::paintBackground()
{
    logMessage("SceneView::paintBackground()");

    // background
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, -10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBegin(GL_QUADS);
    if (Util::config()->scalarView3DBackground)
        glColor3d(0.99, 0.99, 0.99);
    else
        glColor3d(Util::config()->colorBackground.redF(),
                  Util::config()->colorBackground.greenF(),
                  Util::config()->colorBackground.blueF());
    glVertex3d(-1.0, -1.0, 0.0);
    glVertex3d(1.0, -1.0, 0.0);
    if (Util::config()->scalarView3DBackground)
        glColor3d(0.44, 0.56, 0.89);
    glVertex3d(1.0, 1.0, 0.0);
    glVertex3d(-1.0, 1.0, 0.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    glPopMatrix();
}

void SceneView::paintBackgroundPixmap()
{
    logMessage("SceneView::paintBackgroundPixmap()");

    if (m_backgroundTexture != -1)
    {
        loadProjection2d(true);

        glEnable(GL_TEXTURE_2D);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);

        glColor3d(1.0, 1.0, 1.0);

        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0); glVertex2d(m_backgroundPosition.x(), m_backgroundPosition.y());
        glTexCoord2d(1.0, 0.0); glVertex2d(m_backgroundPosition.x() + m_backgroundPosition.width(), m_backgroundPosition.y());
        glTexCoord2d(1.0, 1.0); glVertex2d(m_backgroundPosition.x() + m_backgroundPosition.width(), m_backgroundPosition.y() + m_backgroundPosition.height());
        glTexCoord2d(0.0, 1.0); glVertex2d(m_backgroundPosition.x(), m_backgroundPosition.y() + m_backgroundPosition.height());
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
}

void SceneView::paintGrid()
{
    logMessage("SceneView::paintGrid()");

    loadProjection2d(true);

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(contextWidth(), contextHeight()));

    glDisable(GL_DEPTH_TEST);

    // heavy line
    int heavyLine = 5;

    glLineWidth(1.0);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x1C47);
    glBegin(GL_LINES);

    if ((((cornerMax.x-cornerMin.x)/Util::config()->gridStep + (cornerMin.y-cornerMax.y)/Util::config()->gridStep) < 200) &&
            ((cornerMax.x-cornerMin.x)/Util::config()->gridStep > 0) && ((cornerMin.y-cornerMax.y)/Util::config()->gridStep > 0))
    {
        // vertical lines
        for (int i = 0; i<cornerMax.x/Util::config()->gridStep; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(Util::config()->colorCross.redF(),
                          Util::config()->colorCross.greenF(),
                          Util::config()->colorCross.blueF());
            else
                glColor3d(Util::config()->colorGrid.redF(),
                          Util::config()->colorGrid.greenF(),
                          Util::config()->colorGrid.blueF());
            glVertex2d(i*Util::config()->gridStep, cornerMin.y);
            glVertex2d(i*Util::config()->gridStep, cornerMax.y);
        }
        for (int i = 0; i>cornerMin.x/Util::config()->gridStep; i--)
        {
            if (i % heavyLine == 0)
                glColor3d(Util::config()->colorCross.redF(),
                          Util::config()->colorCross.greenF(),
                          Util::config()->colorCross.blueF());
            else
                glColor3d(Util::config()->colorGrid.redF(),
                          Util::config()->colorGrid.greenF(),
                          Util::config()->colorGrid.blueF());
            glVertex2d(i*Util::config()->gridStep, cornerMin.y);
            glVertex2d(i*Util::config()->gridStep, cornerMax.y);
        }

        // horizontal lines
        for (int i = 0; i<cornerMin.y/Util::config()->gridStep; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(Util::config()->colorCross.redF(),
                          Util::config()->colorCross.greenF(),
                          Util::config()->colorCross.blueF());
            else
                glColor3d(Util::config()->colorGrid.redF(),
                          Util::config()->colorGrid.greenF(),
                          Util::config()->colorGrid.blueF());
            glVertex2d(cornerMin.x, i*Util::config()->gridStep);
            glVertex2d(cornerMax.x, i*Util::config()->gridStep);
        }
        for (int i = 0; i>cornerMax.y/Util::config()->gridStep; i--)
        {
            if (i % heavyLine == 0)
                glColor3d(Util::config()->colorCross.redF(),
                          Util::config()->colorCross.greenF(),
                          Util::config()->colorCross.blueF());
            else
                glColor3d(Util::config()->colorGrid.redF(),
                          Util::config()->colorGrid.greenF(),
                          Util::config()->colorGrid.blueF());
            glVertex2d(cornerMin.x, i*Util::config()->gridStep);
            glVertex2d(cornerMax.x, i*Util::config()->gridStep);
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    if (m_scene->problemInfo()->coordinateType == CoordinateType_Axisymmetric)
    {
        drawBlend(cornerMin,
                  Point(0, cornerMax.y),
                  Util::config()->colorGrid.redF(),
                  Util::config()->colorGrid.greenF(),
                  Util::config()->colorGrid.blueF(), 0.25);
    }

    // axes
    glColor3d(Util::config()->colorCross.redF(),
              Util::config()->colorCross.greenF(),
              Util::config()->colorCross.blueF());
    glLineWidth(1.0);
    glBegin(GL_LINES);
    // y axis
    glVertex2d(0, cornerMin.y);
    glVertex2d(0, cornerMax.y);
    // x axis
    glVertex2d(((m_scene->problemInfo()->coordinateType == CoordinateType_Axisymmetric) ? 0 : cornerMin.x), 0);
    glVertex2d(cornerMax.x, 0);
    glEnd();
}

void SceneView::paintAxes()
{
    logMessage("SceneView::paintGrid()");

    loadProjection2d();

    glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
    glTranslated(-contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glColor3d(Util::config()->colorCross.redF(),
              Util::config()->colorCross.greenF(),
              Util::config()->colorCross.blueF());

    // x-axis
    glBegin(GL_QUADS);
    glVertex2d(10, 10);
    glVertex2d(26, 10);
    glVertex2d(26, 12);
    glVertex2d(10, 12);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(26, 6);
    glVertex2d(26, 16);
    glVertex2d(45, 11);
    glEnd();

    renderText(48, height() - 12 + fontMetrics().height() / 3, Util::scene()->problemInfo()->labelX());

    // y-axis
    glBegin(GL_QUADS);
    glVertex2d(10, 10);
    glVertex2d(10, 26);
    glVertex2d(12, 26);
    glVertex2d(12, 10);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(6, 26);
    glVertex2d(16, 26);
    glVertex2d(11, 45);
    glEnd();

    renderText(12 - fontMetrics().width(Util::scene()->problemInfo()->labelY()) / 2, height() - 48, Util::scene()->problemInfo()->labelY());

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneView::paintRulers()
{
    logMessage("SceneView::paintRulers()");

    loadProjection2d(true);

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(contextWidth(), contextHeight()));

    // rulers
    double step = (((int) ((cornerMax - cornerMin).x / Util::config()->gridStep) + 1) / 5) * Util::config()->gridStep;

    Point size((2.0/contextWidth()*fontMetrics().width(" "))/m_scale2d*aspect(),
               (2.0/contextHeight()*fontMetrics().height())/m_scale2d);

    if (step > 0.0)
    {
        QString text;

        if (((cornerMax.x-cornerMin.x)/step > 0) && ((cornerMin.y-cornerMax.y)/step > 0))
        {
            glColor3d(0.3, 0.2, 0.0);

            // horizontal ticks
            for (int i = 0; i<cornerMax.x/step; i++)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(i*step - size.x*text.size() / 2.0, cornerMax.y + size.x / 4.0, 0.0, text);
            }
            for (int i = 0; i>cornerMin.x/step; i--)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(i*step - size.x*text.size() / 2.0, cornerMax.y + size.x / 4.0, 0.0, text);
            }

            // vertical ticks
            for (int i = 0; i<cornerMin.y/step; i++)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(cornerMin.x + size.x/4.0, i*step - size.y / 4.0, 0.0, text);
            }
            for (int i = 0; i>cornerMax.y/step; i--)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(cornerMin.x + size.x/4.0, i*step - size.y / 4.0, 0.0, text);
            }
        }
    }
}

void SceneView::paintGeometry()
{
    logMessage("SceneView::paintGeometry()");

    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, m_scene->edges->items())
    {
        if (m_sceneMode == SceneMode_OperateOnEdges)
        {
            // edge without marker
            if (edge->markersCount() == 0)
            {
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0x8FFF);
            }
        }

        glColor3d(Util::config()->colorEdges.redF(),
                  Util::config()->colorEdges.greenF(),
                  Util::config()->colorEdges.blueF());
        glLineWidth(Util::config()->edgeWidth);
        if (edge->isHighlighted)
        {
            glColor3d(Util::config()->colorHighlighted.redF(),
                      Util::config()->colorHighlighted.greenF(),
                      Util::config()->colorHighlighted.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }
        if (edge->isSelected)
        {
            glColor3d(Util::config()->colorSelected.redF(),
                      Util::config()->colorSelected.greenF(),
                      Util::config()->colorSelected.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }

        if (fabs(edge->angle) < EPS_ZERO)
        {
            glBegin(GL_LINES);
            glVertex2d(edge->nodeStart->point.x, edge->nodeStart->point.y);
            glVertex2d(edge->nodeEnd->point.x, edge->nodeEnd->point.y);
            glEnd();
        }
        else
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle, edge->angle/2.0);
        }

        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0);
    }

    // nodes
    if (!(m_sceneMode == SceneMode_Postprocessor))
    {
        foreach (SceneNode *node, m_scene->nodes->items())
        {
            glColor3d(Util::config()->colorNodes.redF(),
                      Util::config()->colorNodes.greenF(),
                      Util::config()->colorNodes.blueF());
            glPointSize(Util::config()->nodeSize);

            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();

            glColor3d(Util::config()->colorBackground.redF(),
                      Util::config()->colorBackground.greenF(),
                      Util::config()->colorBackground.blueF());
            glPointSize(Util::config()->nodeSize - 2.0);

            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();

            if ((node->isSelected) || (node->isHighlighted))
            {
                if (node->isHighlighted)
                    glColor3d(Util::config()->colorHighlighted.redF(),
                              Util::config()->colorHighlighted.greenF(),
                              Util::config()->colorHighlighted.blueF());
                if (node->isSelected)
                    glColor3d(Util::config()->colorSelected.redF(),
                              Util::config()->colorSelected.greenF(),
                              Util::config()->colorSelected.blueF());

                glPointSize(Util::config()->nodeSize - 2.0);
                glBegin(GL_POINTS);
                glVertex2d(node->point.x, node->point.y);
                glEnd();
            }
        }

        glLineWidth(1.0);
    }
    // labels
    if (!(m_sceneMode == SceneMode_Postprocessor))
    {
        foreach (SceneLabel *label, m_scene->labels->items())
        {
            glColor3d(Util::config()->colorLabels.redF(),
                      Util::config()->colorLabels.greenF(),
                      Util::config()->colorLabels.blueF());
            glPointSize(Util::config()->labelSize);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();

            glColor3d(Util::config()->colorBackground.redF(),
                      Util::config()->colorBackground.greenF(),
                      Util::config()->colorBackground.blueF());
            glPointSize(Util::config()->labelSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();

            if ((label->isSelected) || (label->isHighlighted))
            {
                if (label->isHighlighted)
                    glColor3d(Util::config()->colorHighlighted.redF(),
                              Util::config()->colorHighlighted.greenF(),
                              Util::config()->colorHighlighted.blueF());
                if (label->isSelected)
                    glColor3d(Util::config()->colorSelected.redF(),
                              Util::config()->colorSelected.greenF(),
                              Util::config()->colorSelected.blueF());

                glPointSize(Util::config()->labelSize - 2.0);
                glBegin(GL_POINTS);
                glVertex2d(label->point.x, label->point.y);
                glEnd();
            }
            glLineWidth(1.0);

            if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                glColor3d(0.1, 0.1, 0.1);

                // assigned materials
                QString str;
                foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                    str = str + QString("%1, ").
                            arg(QString::fromStdString(label->getMarker(fieldInfo)->getName()));
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                Point point;
                point.x = 2.0/contextWidth()*aspect()*fontMetrics().width(str)/m_scale2d/2.0;
                point.y = 2.0/contextHeight()*fontMetrics().height()/m_scale2d;

                renderTextPos(label->point.x-point.x, label->point.y-point.y, 0.0, str, false);
            }

            // area size
            if ((m_sceneMode == SceneMode_OperateOnLabels) || (m_sceneViewSettings.showInitialMesh))
            {
                double radius = sqrt(label->area/M_PI);
                glColor3d(0, 0.95, 0.9);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i<360; i = i + 10)
                {
                    glVertex2d(label->point.x + radius*cos(i/180.0*M_PI), label->point.y + radius*sin(i/180.0*M_PI));
                }
                glEnd();
            }
        }
    }
}

void SceneView::paintInitialMesh()
{
    logMessage("SceneView::paintInitialMesh()");

    if (!m_scene->sceneSolution()->isMeshed()) return;

    loadProjection2d(true);

    m_scene->sceneSolution()->linInitialMeshView().lock_data();

    double3* linVert = m_scene->sceneSolution()->linInitialMeshView().get_vertices();
    int3* linEdges = m_scene->sceneSolution()->linInitialMeshView().get_edges();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(Util::config()->colorInitialMesh.redF(),
              Util::config()->colorInitialMesh.greenF(),
              Util::config()->colorInitialMesh.blueF());
    glLineWidth(1.3);

    // triangles
    glBegin(GL_LINES);
    for (int i = 0; i < m_scene->sceneSolution()->linInitialMeshView().get_num_edges(); i++)
    {
        glVertex2d(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]);
        glVertex2d(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]);
    }
    glEnd();

    m_scene->sceneSolution()->linInitialMeshView().unlock_data();
}

void SceneView::paintSolutionMesh()
{
    logMessage("SceneView::paintSolutionMesh()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    m_scene->sceneSolution()->linSolutionMeshView().lock_data();

    double3* linVert = m_scene->sceneSolution()->linSolutionMeshView().get_vertices();
    int3* linEdges = m_scene->sceneSolution()->linSolutionMeshView().get_edges();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(Util::config()->colorSolutionMesh.redF(),
              Util::config()->colorSolutionMesh.greenF(),
              Util::config()->colorSolutionMesh.blueF());
    glLineWidth(1.3);

    // triangles
    // qDebug() << m_scene->sceneSolution()->linSolutionMeshView().get_num_edges();
    glBegin(GL_LINES);
    for (int i = 0; i < m_scene->sceneSolution()->linSolutionMeshView().get_num_edges(); i++)
    {
        glVertex2d(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]);
        glVertex2d(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]);
    }
    glEnd();

    m_scene->sceneSolution()->linSolutionMeshView().unlock_data();

}

void SceneView::paintOrder()
{
    logMessage("SceneView::paintOrder()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listOrder == -1)
    {
        m_listOrder = glGenLists(1);
        glNewList(m_listOrder, GL_COMPILE);

        // order scalar view
        m_scene->sceneSolution()->ordView().lock_data();

        double3* vert = m_scene->sceneSolution()->ordView().get_vertices();
        int3* tris = m_scene->sceneSolution()->ordView().get_triangles();

        // draw mesh
        int min = 11;
        int max = 1;
        for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            int color = vert[tris[i][0]][2];
            glColor3d(paletteColorOrder(color)[0], paletteColorOrder(color)[1], paletteColorOrder(color)[2]);

            glVertex2d(vert[tris[i][0]][0], vert[tris[i][0]][1]);
            glVertex2d(vert[tris[i][1]][0], vert[tris[i][1]][1]);
            glVertex2d(vert[tris[i][2]][0], vert[tris[i][2]][1]);
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        glEndList();

        glCallList(m_listOrder);
    }
    else
    {
        glCallList(m_listOrder);
    }

    // paint labels
    if (Util::config()->orderLabel)
    {
        QFont fontLabel = font();
        fontLabel.setPointSize(fontLabel.pointSize() - 3);

        m_scene->sceneSolution()->ordView().lock_data();

        double3* vert = m_scene->sceneSolution()->ordView().get_vertices();
        int* lvert;
        char** ltext;
        double2* lbox;
        int nl = m_scene->sceneSolution()->ordView().get_labels(lvert, ltext, lbox);

        Point size((2.0/contextWidth()*fontMetrics().width(" "))/m_scale2d*aspect(),
                   (2.0/contextHeight()*fontMetrics().height())/m_scale2d);

        for (int i = 0; i < nl; i++)
        {
            glColor3d(1, 1, 1);
            // if (lbox[i][0]/m_scale*aspect() > size.x && lbox[i][1]/m_scale > size.y)
            {
                renderText(vert[lvert[i]][0] - size.x / 2.0,
                           vert[lvert[i]][1] - size.y / 2.0,
                           0.0,
                           ltext[i],
                           fontLabel);
            }
        }

        m_scene->sceneSolution()->ordView().unlock_data();
    }
}

void SceneView::paintOrderColorBar()
{
    logMessage("SceneView::paintOrderColorBar()");

    if (!m_isSolutionPrepared || !Util::config()->showOrderScale) return;

    // order scalar view
    m_scene->sceneSolution()->ordView().lock_data();

    double3* vert = m_scene->sceneSolution()->ordView().get_vertices();
    int3* tris = m_scene->sceneSolution()->ordView().get_triangles();

    int min = 11;
    int max = 1;
    for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
    {
        if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
        if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
    }

    m_scene->sceneSolution()->ordView().unlock_data();

    // order color map
    loadProjection2d();

    glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
    glTranslated(- contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    // dimensions
    int textWidth = fontMetrics().width("00");
    int textHeight = fontMetrics().height();
    Point scaleSize = Point(20 + 3 * textWidth, (20 + max * (2 * textHeight) - textHeight / 2.0 + 2));
    Point scaleBorder = Point(10.0, 10.0);
    double scaleLeft = (contextWidth() - (20 + 3 * textWidth));

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // bars
    glBegin(GL_QUADS);
    for (int i = 1; i < max+1; i++)
    {
        glColor3d(0.0, 0.0, 0.0);
        glVertex2d(scaleLeft + 10,                                 scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - scaleBorder.x, scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - scaleBorder.x, scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 10,                                 scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);

        glColor3d(paletteColorOrder(i)[0], paletteColorOrder(i)[1], paletteColorOrder(i)[2]);
        glVertex2d(scaleLeft + 12,                                     scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - 2 - scaleBorder.x, scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - 2 - scaleBorder.x, scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 12,                                     scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
    }
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // labels
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 1; i < max + 1; i++)
    {
        int sizeNumber = fontMetrics().width(QString::number(i));
        renderText(scaleLeft + 10 + 1.5 * textWidth - sizeNumber,
                   scaleBorder.y + 10.0 + (i-1)*(2.0 * textHeight) + textHeight / 2.0,
                   0.0,
                   QString::number(i));
    }
}

void SceneView::paintScalarFieldColorBar(double min, double max)
{
    assert(0); //TODO
    //    logMessage("SceneView::paintScalarFieldColorBar()");

    //    if (!Util::config()->showScalarScale) return;

    //    loadProjection2d();

    //    glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
    //    glTranslated(-contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    //    // dimensions
    //    int textWidth = fontMetrics().width(QString::number(-1.0, '+e', Util::config()->scalarDecimalPlace)) + 3;
    //    int textHeight = fontMetrics().height();
    //    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // contextHeight() - 20.0
    //    Point scaleBorder = Point(10.0, 10.0);
    //    double scaleLeft = (contextWidth() - (45.0 + textWidth));
    //    int numTicks = 11;

    //    // blended rectangle
    //    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
    //              0.91, 0.91, 0.91);

    //    glDisable(GL_DEPTH_TEST);
    //    glEnable(GL_POLYGON_OFFSET_FILL);
    //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //    // palette border
    //    glColor3d(0.0, 0.0, 0.0);
    //    glBegin(GL_QUADS);
    //    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    //    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    //    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    //    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    //    glEnd();

    //    glDisable(GL_POLYGON_OFFSET_FILL);

    //    // palette
    //    glEnable(GL_TEXTURE_1D);
    //    glBindTexture(GL_TEXTURE_1D, 1);
    //    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    //    glBegin(GL_QUADS);
    //    if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) > EPS_ZERO)
    //        glTexCoord1d(m_texScale + m_texShift);
    //    else
    //        glTexCoord1d(m_texShift);
    //    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    //    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    //    glTexCoord1d(m_texShift);
    //    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    //    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    //    glEnd();

    //    glDisable(GL_TEXTURE_1D);

    //    // ticks
    //    glLineWidth(1.0);
    //    glBegin(GL_LINES);
    //    for (int i = 1; i < numTicks+1; i++)
    //    {
    //        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

    //        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    //        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    //        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    //        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    //    }
    //    glEnd();

    //    // labels
    //    for (int i = 1; i < numTicks+1; i++)
    //    {
    //        double value = 0.0;
    //        if (!Util::config()->scalarRangeLog)
    //            value = min + (double) (i-1) / (numTicks-1) * (max - min);
    //        else
    //            value = min + (double) pow(Util::config()->scalarRangeBase, ((i-1) / (numTicks-1)))/Util::config()->scalarRangeBase * (max - min);

    //        if (fabs(value) < EPS_ZERO) value = 0.0;
    //        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

    //        renderText(scaleLeft + 33.0 + ((value >= 0.0) ? fontMetrics().width("-") : 0.0),
    //                   scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
    //                   0.0,
    //                   QString::number(value, '+e', Util::config()->scalarDecimalPlace));
    //    }

    //    // variable
    //    QString str = QString("%1 (%2)").
    //            arg(QString::fromStdString(m_sceneViewSettings.scalarPhysicFieldVariable != "" ? Util::scene()->problemInfo()->module()->get_variable(m_sceneViewSettings.scalarPhysicFieldVariable)->shortname : "")).
    //            arg(QString::fromStdString(m_sceneViewSettings.scalarPhysicFieldVariable != "" ? Util::scene()->problemInfo()->module()->get_variable(m_sceneViewSettings.scalarPhysicFieldVariable)->unit : ""));

    //    renderText(scaleLeft + scaleSize.x / 2.0 - fontMetrics().width(str) / 2.0,
    //               scaleBorder.y + scaleSize.y - 20.0,
    //               0.0,
    //               str);
    //    // line
    //    glLineWidth(1.0);
    //    glBegin(GL_LINES);
    //    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    //    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    //    glEnd();
}

void SceneView::paintScalarField()
{
    logMessage("SceneView::paintScalarField()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listScalarField == -1)
    {
        // qDebug() << "SceneView::paintScalarField(), min = " << m_sceneViewSettings.scalarRangeMin << ", max = " << m_sceneViewSettings.scalarRangeMax;

        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        // range
        double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
        // special case: constant solution
        if (fabs(m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin) < EPS_ZERO)
            irange = 1.0;

        m_scene->sceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->sceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->sceneSolution()->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        // set texture for coloring
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // set texture transformation matrix
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(m_texShift, 0.0, 0.0);
        glScaled(m_texScale, 0.0, 0.0);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_triangles(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = linVert[linTris[i][j]][0];
                point[j].y = linVert[linTris[i][j]][1];
                value[j]   = linVert[linTris[i][j]][2];
            }

            if (!m_sceneViewSettings.scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                    continue;
            }

            for (int j = 0; j < 3; j++)
            {
                if (Util::config()->scalarRangeLog)
                    glTexCoord1d(log10(1.0 + (Util::config()->scalarRangeBase-1.0)*(value[j] - m_sceneViewSettings.scalarRangeMin) * irange)/log10(Util::config()->scalarRangeBase));
                else
                    glTexCoord1d((value[j] - m_sceneViewSettings.scalarRangeMin) * irange);
                glVertex2d(point[j].x, point[j].y);
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_TEXTURE_1D);

        // switch-off texture transform
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        m_scene->sceneSolution()->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField);
    }
    else
    {
        glCallList(m_listScalarField);
    }
}

void SceneView::paintScalarField3D()
{
    logMessage("SceneView::paintScalarField3D()");

    if (!m_isSolutionPrepared) return;

    loadProjection3d(true);

    if (m_listScalarField3D == -1)
    {
        m_listScalarField3D = glGenLists(1);
        glNewList(m_listScalarField3D, GL_COMPILE);

        // gradient background
        paintBackground();
        glEnable(GL_DEPTH_TEST);

        // range
        double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
        // special case: constant solution
        if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < EPS_ZERO)
        {
            irange = 1.0;
        }

        m_scene->sceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->sceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->sceneSolution()->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        double max = qMax(m_scene->boundingBox().width(), m_scene->boundingBox().height());

        if (Util::config()->scalarView3DLighting)
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        else
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        glPushMatrix();
        glScaled(1.0, 1.0, max / Util::config()->scalarView3DHeight * 1.0/(fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax)));

        initLighting();
        // init normal
        double *normal = new double[3];

        // set texture for coloring
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, 1);

        // set texture transformation matrix
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(m_texShift, 0.0, 0.0);
        glScaled(m_texScale, 0.0, 0.0);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_triangles(); i++)
        {
            point[0].x = linVert[linTris[i][0]][0];
            point[0].y = linVert[linTris[i][0]][1];
            value[0]   = linVert[linTris[i][0]][2];
            point[1].x = linVert[linTris[i][1]][0];
            point[1].y = linVert[linTris[i][1]][1];
            value[1]   = linVert[linTris[i][1]][2];
            point[2].x = linVert[linTris[i][2]][0];
            point[2].y = linVert[linTris[i][2]][1];
            value[2]   = linVert[linTris[i][2]][2];

            if (!m_sceneViewSettings.scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                    continue;
            }

            double delta = 0.0;

            if (Util::config()->scalarView3DLighting)
            {
                computeNormal(point[0].x, point[0].y, - delta - (value[0] - m_sceneViewSettings.scalarRangeMin),
                              point[1].x, point[1].y, - delta - (value[1] - m_sceneViewSettings.scalarRangeMin),
                              point[2].x, point[2].y, - delta - (value[2] - m_sceneViewSettings.scalarRangeMin),
                              normal);

                glNormal3d(normal[0], normal[1], normal[2]);
            }
            for (int j = 0; j < 3; j++)
            {
                glTexCoord1d((value[j] - m_sceneViewSettings.scalarRangeMin) * irange);
                glVertex3d(point[j].x, point[j].y, - delta - (value[j] - m_sceneViewSettings.scalarRangeMin));
            }
        }
        glEnd();

        // remove normal
        delete [] normal;

        glDisable(GL_TEXTURE_1D);
        glDisable(GL_LIGHTING);

        // draw blended mesh
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4d(0.5, 0.5, 0.5, 0.3);

        m_scene->sceneSolution()->linInitialMeshView().lock_data();

        double3* linVertMesh = m_scene->sceneSolution()->linInitialMeshView().get_vertices();
        int3* linTrisMesh = m_scene->sceneSolution()->linInitialMeshView().get_triangles();

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->linInitialMeshView().get_num_triangles(); i++)
        {
            glVertex2d(linVertMesh[linTrisMesh[i][0]][0], linVertMesh[linTrisMesh[i][0]][1]);
            glVertex2d(linVertMesh[linTrisMesh[i][1]][0], linVertMesh[linTrisMesh[i][1]][1]);
            glVertex2d(linVertMesh[linTrisMesh[i][2]][0], linVertMesh[linTrisMesh[i][2]][1]);
        }
        glEnd();

        m_scene->sceneSolution()->linInitialMeshView().unlock_data();

        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_OFFSET_FILL);

        // geometry - edges
        foreach (SceneEdge *edge, m_scene->edges->items())
        {

            glColor3d(Util::config()->colorEdges.redF(),
                      Util::config()->colorEdges.greenF(),
                      Util::config()->colorEdges.blueF());
            glLineWidth(Util::config()->edgeWidth);

            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex3d(edge->nodeStart->point.x, edge->nodeStart->point.y, 0.0);
                glVertex3d(edge->nodeEnd->point.x, edge->nodeEnd->point.y, 0.0);
                glEnd();
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180.0 - 180.0;

                drawArc(center, radius, startAngle, edge->angle, edge->angle/2);
            }

            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
        }

        glDisable(GL_DEPTH_TEST);

        // switch-off texture transform
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glPopMatrix();

        m_scene->sceneSolution()->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField3D);
    }
    else
    {
        glCallList(m_listScalarField3D);
    }
}

void SceneView::paintScalarField3DSolid()
{
    logMessage("SceneView::paintScalarField3DSolid()");

    if (!m_isSolutionPrepared) return;

    loadProjection3d(true);

    if (m_listScalarField3DSolid == -1)
    {
        m_listScalarField3DSolid = glGenLists(1);
        glNewList(m_listScalarField3DSolid, GL_COMPILE);

        bool isModel = (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Model);

        // gradient background
        paintBackground();
        glEnable(GL_DEPTH_TEST);

        RectPoint rect = m_scene->boundingBox();
        double max = qMax(rect.width(), rect.height());
        double depth = max / Util::config()->scalarView3DHeight;

        // range
        double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
        // special case: constant solution
        if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < EPS_ZERO)
        {
            irange = 1.0;
        }

        double phi = Util::config()->scalarView3DAngle;

        m_scene->sceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->sceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->sceneSolution()->linScalarView().get_triangles();
        int3* linEdges = m_scene->sceneSolution()->linScalarView().get_edges();
        Point point[3];
        double value[3];

        glPushMatrix();

        // set texture for coloring
        if (!isModel)
        {
            glEnable(GL_TEXTURE_1D);
            glBindTexture(GL_TEXTURE_1D, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

            // set texture transformation matrix
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glTranslated(m_texShift, 0.0, 0.0);
            glScaled(m_texScale, 0.0, 0.0);
        }
        else
        {
            glColor3d(0.7, 0.7, 0.7);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        }

        initLighting();
        // init normals
        double* normal = new double[3];

        if (m_scene->problemInfo()->coordinateType == CoordinateType_Planar)
        {
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_triangles(); i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    point[j].x = linVert[linTris[i][j]][0];
                    point[j].y = linVert[linTris[i][j]][1];
                    value[j]   = linVert[linTris[i][j]][2];
                }

                if (!m_sceneViewSettings.scalarRangeAuto)
                {
                    double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                    if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                        continue;
                }

                // z = - depth / 2.0
                if (Util::config()->scalarView3DLighting || isModel)
                {
                    computeNormal(point[0].x, point[0].y, -depth/2.0,
                                  point[1].x, point[1].y, -depth/2.0,
                                  point[2].x, point[2].y, -depth/2.0,
                                  normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                for (int j = 0; j < 3; j++)
                {
                    if (!isModel) glTexCoord1d((value[j] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[j].x, point[j].y, -depth/2.0);
                }

                // z = + depth / 2.0
                if (Util::config()->scalarView3DLighting || isModel)
                {
                    computeNormal(point[0].x, point[0].y, depth/2.0,
                                  point[1].x, point[1].y, depth/2.0,
                                  point[2].x, point[2].y, depth/2.0,
                                  normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                for (int j = 0; j < 3; j++)
                {
                    if (!isModel) glTexCoord1d((value[j] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[j].x, point[j].y, depth/2.0);
                }
            }
            glEnd();

            // length
            glBegin(GL_QUADS);
            for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_edges(); i++)
            {
                // draw only boundary edges
                if (!linEdges[i][2]) continue;

                for (int j = 0; j < 2; j++)
                {
                    point[j].x = linVert[linEdges[i][j]][0];
                    point[j].y = linVert[linEdges[i][j]][1];
                    value[j]   = linVert[linEdges[i][j]][2];
                }

                if (!m_sceneViewSettings.scalarRangeAuto)
                {
                    double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                    if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                        continue;
                }

                if (Util::config()->scalarView3DLighting || isModel)
                {
                    computeNormal(point[0].x, point[0].y, -depth/2.0,
                                  point[1].x, point[1].y, -depth/2.0,
                                  point[1].x, point[1].y,  depth/2.0,
                                  normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                if (!isModel) glTexCoord1d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange);
                glVertex3d(point[0].x, point[0].y, -depth/2.0);
                if (!isModel) glTexCoord1d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange);
                glVertex3d(point[1].x, point[1].y, -depth/2.0);
                if (!isModel) glTexCoord1d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange);
                glVertex3d(point[1].x, point[1].y, depth/2.0);
                if (!isModel) glTexCoord1d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange);
                glVertex3d(point[0].x, point[0].y, depth/2.0);
            }
            glEnd();
        }
        else
        {
            // side
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_triangles(); i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    point[j].x = linVert[linTris[i][j]][0];
                    point[j].y = linVert[linTris[i][j]][1];
                    value[j]   = linVert[linTris[i][j]][2];
                }

                if (!m_sceneViewSettings.scalarRangeAuto)
                {
                    double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                    if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                        continue;
                }

                for (int j = 0; j < 2; j++)
                {
                    if (Util::config()->scalarView3DLighting || isModel)
                    {
                        computeNormal(point[0].x * cos(j*phi/180.0*M_PI), point[0].y, point[0].x * sin(j*phi/180.0*M_PI),
                                      point[1].x * cos(j*phi/180.0*M_PI), point[1].y, point[1].x * sin(j*phi/180.0*M_PI),
                                      point[2].x * cos(j*phi/180.0*M_PI), point[2].y, point[2].x * sin(j*phi/180.0*M_PI),
                                      normal);
                        glNormal3d(normal[0], normal[1], normal[2]);
                    }

                    glTexCoord1d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[0].x * cos(j*phi/180.0*M_PI), point[0].y, point[0].x * sin(j*phi/180.0*M_PI));
                    glTexCoord1d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[1].x * cos(j*phi/180.0*M_PI), point[1].y, point[1].x * sin(j*phi/180.0*M_PI));
                    glTexCoord1d((value[2] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[2].x * cos(j*phi/180.0*M_PI), point[2].y, point[2].x * sin(j*phi/180.0*M_PI));
                }
            }
            glEnd();

            // symmetry
            glBegin(GL_QUADS);
            for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_edges(); i++)
            {
                // draw only boundary edges
                if (!linEdges[i][2]) continue;

                for (int j = 0; j < 2; j++)
                {
                    point[j].x = linVert[linEdges[i][j]][0];
                    point[j].y = linVert[linEdges[i][j]][1];
                    value[j]   = linVert[linEdges[i][j]][2];
                }

                if (!m_sceneViewSettings.scalarRangeAuto)
                {
                    double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                    if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                        continue;
                }

                int count = 30;
                double step = phi/count;
                for (int j = 0; j < count; j++)
                {
                    if (Util::config()->scalarView3DLighting || isModel)
                    {

                        computeNormal(point[0].x * cos((j+0)*step/180.0*M_PI), point[0].y, point[0].x * sin((j+0)*step/180.0*M_PI),
                                      point[1].x * cos((j+0)*step/180.0*M_PI), point[1].y, point[1].x * sin((j+0)*step/180.0*M_PI),
                                      point[1].x * cos((j+1)*step/180.0*M_PI), point[1].y, point[1].x * sin((j+1)*step/180.0*M_PI),
                                      normal);
                        glNormal3d(normal[0], normal[1], normal[2]);
                    }

                    if (!isModel) glTexCoord1d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[0].x * cos((j+0)*step/180.0*M_PI), point[0].y, point[0].x * sin((j+0)*step/180.0*M_PI));
                    if (!isModel) glTexCoord1d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[1].x * cos((j+0)*step/180.0*M_PI), point[1].y, point[1].x * sin((j+0)*step/180.0*M_PI));
                    if (!isModel) glTexCoord1d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[1].x * cos((j+1)*step/180.0*M_PI), point[1].y, point[1].x * sin((j+1)*step/180.0*M_PI));
                    if (!isModel) glTexCoord1d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange);
                    glVertex3d(point[0].x * cos((j+1)*step/180.0*M_PI), point[0].y, point[0].x * sin((j+1)*step/180.0*M_PI));
                }
            }
            glEnd();
        }

        // remove normals
        delete [] normal;

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_LIGHTING);

        if (!isModel)
        {
            glDisable(GL_TEXTURE_1D);

            // switch-off texture transform
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
        }

        // geometry
        if (m_scene->problemInfo()->coordinateType == CoordinateType_Planar)
        {
            glColor3d(Util::config()->colorEdges.redF(),
                      Util::config()->colorEdges.greenF(),
                      Util::config()->colorEdges.blueF());
            glLineWidth(Util::config()->edgeWidth);

            // top and bottom
            foreach (SceneEdge *edge, m_scene->edges->items())
            {
                for (int j = 0; j < 2; j++)
                {
                    if (edge->isStraight())
                    {
                        glBegin(GL_LINES);
                        glVertex3d(edge->nodeStart->point.x, edge->nodeStart->point.y, - depth/2.0 + j*depth);
                        glVertex3d(edge->nodeEnd->point.x, edge->nodeEnd->point.y, - depth/2.0 + j*depth);
                        glEnd();
                    }
                    else
                    {
                        Point center = edge->center();
                        double radius = edge->radius();
                        double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180.0 - 180.0;

                        double theta = edge->angle / double(edge->angle/2 - 1);

                        glBegin(GL_LINE_STRIP);
                        for (int i = 0; i < edge->angle/2; i++)
                        {
                            double arc = (startAngle + i*theta)/180.0*M_PI;

                            double x = radius * cos(arc);
                            double y = radius * sin(arc);

                            glVertex3d(center.x + x, center.y + y, - depth/2.0 + j*depth);
                        }
                        glEnd();
                    }
                }
            }

            // side
            glBegin(GL_LINES);
            foreach (SceneNode *node, m_scene->nodes->items())
            {
                glVertex3d(node->point.x, node->point.y,  depth/2.0);
                glVertex3d(node->point.x, node->point.y, -depth/2.0);
            }
            glEnd();

            glLineWidth(1.0);
        }
        else
        {
            // geometry
            glColor3d(Util::config()->colorEdges.redF(),
                      Util::config()->colorEdges.greenF(),
                      Util::config()->colorEdges.blueF());
            glLineWidth(Util::config()->edgeWidth);

            // top
            foreach (SceneEdge *edge, m_scene->edges->items())
            {
                for (int j = 0; j < 2; j++)
                {
                    if (edge->isStraight())
                    {
                        glBegin(GL_LINES);
                        glVertex3d(edge->nodeStart->point.x * cos(j*phi/180.0*M_PI), edge->nodeStart->point.y, edge->nodeStart->point.x * sin(j*phi/180.0*M_PI));
                        glVertex3d(edge->nodeEnd->point.x * cos(j*phi/180.0*M_PI), edge->nodeEnd->point.y, edge->nodeEnd->point.x * sin(j*phi/180.0*M_PI));
                        glEnd();
                    }
                    else
                    {
                        Point center = edge->center();
                        double radius = edge->radius();
                        double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180.0 - 180.0;

                        double theta = edge->angle / double(edge->angle/2 - 1);

                        glBegin(GL_LINE_STRIP);
                        for (int i = 0; i < edge->angle/2; i++)
                        {
                            double arc = (startAngle + i*theta)/180.0*M_PI;

                            double x = radius * cos(arc);
                            double y = radius * sin(arc);

                            glVertex3d((center.x + x) * cos(j*phi/180.0*M_PI), center.y + y, (center.x + x) * sin(j*phi/180.0*M_PI));
                        }
                        glEnd();
                    }
                }
            }

            // side
            foreach (SceneNode *node, m_scene->nodes->items())
            {
                int count = 30;
                double step = phi/count;

                glBegin(GL_LINE_STRIP);
                for (int j = 0; j < count; j++)
                {
                    glVertex3d(node->point.x * cos((j+0)*step/180.0*M_PI), node->point.y, node->point.x * sin((j+0)*step/180.0*M_PI));
                    glVertex3d(node->point.x * cos((j+1)*step/180.0*M_PI), node->point.y, node->point.x * sin((j+1)*step/180.0*M_PI));
                }
                glEnd();
            }

            glLineWidth(1.0);
        }

        glDisable(GL_DEPTH_TEST);

        glPopMatrix();

        m_scene->sceneSolution()->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField3DSolid);
    }
    else
    {
        glCallList(m_listScalarField3DSolid);
    }
}

void SceneView::paintContours()
{
    logMessage("SceneView::paintContours()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listContours == -1)
    {
        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        m_scene->sceneSolution()->linContourView().lock_data();

        double3* tvert = m_scene->sceneSolution()->linContourView().get_vertices();
        int3* tris = m_scene->sceneSolution()->linContourView().get_triangles();

        // transform variable
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        double3* vert = new double3[m_scene->sceneSolution()->linContourView().get_num_vertices()];
        for (int i = 0; i < m_scene->sceneSolution()->linContourView().get_num_vertices(); i++)
        {
            vert[i][0] = tvert[i][0];
            vert[i][1] = tvert[i][1];
            vert[i][2] = tvert[i][2];

            if (vert[i][2] > rangeMax) rangeMax = tvert[i][2];
            if (vert[i][2] < rangeMin) rangeMin = tvert[i][2];
        }

        // value range
        double step = (rangeMax-rangeMin)/Util::config()->contoursCount;

        // draw contours
        glLineWidth(1.0);
        glColor3d(Util::config()->colorContours.redF(),
                  Util::config()->colorContours.greenF(),
                  Util::config()->colorContours.blueF());

        glBegin(GL_LINES);
        for (int i = 0; i < m_scene->sceneSolution()->linContourView().get_num_triangles(); i++)
        {
            if (finite(vert[tris[i][0]][2]) && finite(vert[tris[i][1]][2]) && finite(vert[tris[i][2]][2]))
            {
                paintContoursTri(vert, &tris[i], step);
            }
        }
        glEnd();

        delete vert;

        m_scene->sceneSolution()->linContourView().unlock_data();

        glEndList();

        glCallList(m_listContours);
    }
    else
    {
        glCallList(m_listContours);
    }
}

void SceneView::paintContoursTri(double3* vert, int3* tri, double step)
{
    logMessage("SceneView::paintContoursTri()");

    // sort the vertices by their value, keep track of the permutation sign
    int i, idx[3], perm = 0;
    memcpy(idx, tri, sizeof(idx));
    for (i = 0; i < 2; i++)
    {
        if (vert[idx[0]][2] > vert[idx[1]][2]) { std::swap(idx[0], idx[1]); perm++; }
        if (vert[idx[1]][2] > vert[idx[2]][2]) { std::swap(idx[1], idx[2]); perm++; }
    }
    if (fabs(vert[idx[0]][2] - vert[idx[2]][2]) < 1e-3 * fabs(step)) return;

    // get the first (lowest) contour value
    double val = vert[idx[0]][2];

    double y = ceil(val / step);
    if (y < val / step) y + 1.0;
    val = y * step;

    int l1 = 0, l2 = 1;
    int r1 = 0, r2 = 2;
    while (val < vert[idx[r2]][2])
    {
        double ld = vert[idx[l2]][2] - vert[idx[l1]][2];
        double rd = vert[idx[r2]][2] - vert[idx[r1]][2];

        // draw a slice of the triangle
        while (val < vert[idx[l2]][2])
        {
            double lt = (val - vert[idx[l1]][2]) / ld;
            double rt = (val - vert[idx[r1]][2]) / rd;

            double x1 = (1.0 - lt) * vert[idx[l1]][0] + lt * vert[idx[l2]][0];
            double y1 = (1.0 - lt) * vert[idx[l1]][1] + lt * vert[idx[l2]][1];
            double x2 = (1.0 - rt) * vert[idx[r1]][0] + rt * vert[idx[r2]][0];
            double y2 = (1.0 - rt) * vert[idx[r1]][1] + rt * vert[idx[r2]][1];

            if (perm & 1) { glVertex2d(x1, y1); glVertex2d(x2, y2); }
            else { glVertex2d(x2, y2); glVertex2d(x1, y1); }

            val += step;
        }
        l1 = 1;
        l2 = 2;
    }
}

void SceneView::paintVectors()
{
    logMessage("SceneView::paintVectors()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        double vectorRangeMin = m_scene->sceneSolution()->vecVectorView().get_min_value();
        double vectorRangeMax = m_scene->sceneSolution()->vecVectorView().get_max_value();

        //Add 20% margin to the range
        double vectorRange = vectorRangeMax - vectorRangeMin;
        vectorRangeMin = vectorRangeMin - 0.2*vectorRange;
        vectorRangeMax = vectorRangeMax + 0.2*vectorRange;

        // qDebug() << "SceneView::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (vectorRangeMax - vectorRangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        RectPoint rect = m_scene->boundingBox();
        double gs = (rect.width() + rect.height()) / Util::config()->vectorCount;

        // paint
        m_scene->sceneSolution()->vecVectorView().lock_data();

        double4* vecVert = m_scene->sceneSolution()->vecVectorView().get_vertices();
        int3* vecTris = m_scene->sceneSolution()->vecVectorView().get_triangles();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->vecVectorView().get_num_triangles(); i++)
        {
            Point a(vecVert[vecTris[i][0]][0], vecVert[vecTris[i][0]][1]);
            Point b(vecVert[vecTris[i][1]][0], vecVert[vecTris[i][1]][1]);
            Point c(vecVert[vecTris[i][2]][0], vecVert[vecTris[i][2]][1]);

            RectPoint r;
            r.start = Point(qMin(qMin(a.x, b.x), c.x), qMin(qMin(a.y, b.y), c.y));
            r.end = Point(qMax(qMax(a.x, b.x), c.x), qMax(qMax(a.y, b.y), c.y));

            // double area
            double area2 = a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);

            // plane equation
            double aa = b.x*c.y - c.x*b.y;
            double ab = c.x*a.y - a.x*c.y;
            double ac = a.x*b.y - b.x*a.y;
            double ba = b.y - c.y;
            double bb = c.y - a.y;
            double bc = a.y - b.y;
            double ca = c.x - b.x;
            double cb = a.x - c.x;
            double cc = b.x - a.x;

            double ax = (aa * vecVert[vecTris[i][0]][2] + ab * vecVert[vecTris[i][1]][2] + ac * vecVert[vecTris[i][2]][2]) / area2;
            double bx = (ba * vecVert[vecTris[i][0]][2] + bb * vecVert[vecTris[i][1]][2] + bc * vecVert[vecTris[i][2]][2]) / area2;
            double cx = (ca * vecVert[vecTris[i][0]][2] + cb * vecVert[vecTris[i][1]][2] + cc * vecVert[vecTris[i][2]][2]) / area2;

            double ay = (aa * vecVert[vecTris[i][0]][3] + ab * vecVert[vecTris[i][1]][3] + ac * vecVert[vecTris[i][2]][3]) / area2;
            double by = (ba * vecVert[vecTris[i][0]][3] + bb * vecVert[vecTris[i][1]][3] + bc * vecVert[vecTris[i][2]][3]) / area2;
            double cy = (ca * vecVert[vecTris[i][0]][3] + cb * vecVert[vecTris[i][1]][3] + cc * vecVert[vecTris[i][2]][3]) / area2;

            for (int j = floor(r.start.x / gs); j < ceil(r.end.x / gs); j++)
            {
                for (int k = floor(r.start.y / gs); k < ceil(r.end.y / gs); k++)
                {
                    Point point(j*gs, k*gs);
                    if (k % 2 == 0) point.x += gs/2.0;

                    // find in triangle
                    bool inTriangle = true;

                    for (int l = 0; l < 3; l++)
                    {
                        int p = l + 1;
                        if (p == 3)
                            p = 0;

                        double z = (vecVert[vecTris[i][p]][0] - vecVert[vecTris[i][l]][0]) * (point.y - vecVert[vecTris[i][l]][1]) -
                                (vecVert[vecTris[i][p]][1] - vecVert[vecTris[i][l]][1]) * (point.x - vecVert[vecTris[i][l]][0]);

                        if (z < 0)
                        {
                            inTriangle = false;
                            break;
                        }
                    }

                    if (inTriangle)
                    {
                        // view
                        double dx = ax + bx * point.x + cx * point.y;
                        double dy = ay + by * point.x + cy * point.y;

                        double value = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));
                        double angle = atan2(dy, dx);

                        if ((Util::config()->vectorProportional) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            if ((value / vectorRangeMax) < 1e-6)
                            {
                                dx = 0.0;
                                dy = 0.0;
                            }
                            else
                            {
                                dx = ((value - vectorRangeMin) * irange) * Util::config()->vectorScale * gs * cos(angle);
                                dy = ((value - vectorRangeMin) * irange) * Util::config()->vectorScale * gs * sin(angle);
                            }
                        }
                        else
                        {
                            dx = Util::config()->vectorScale * gs * cos(angle);
                            dy = Util::config()->vectorScale * gs * sin(angle);
                        }

                        double dm = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));

                        // color
                        if ((Util::config()->vectorColor) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            double color = 0.7 - 0.7 * (value - vectorRangeMin) * irange;
                            glColor3d(color, color, color);
                        }
                        else
                        {
                            glColor3d(Util::config()->colorVectors.redF(),
                                      Util::config()->colorVectors.greenF(),
                                      Util::config()->colorVectors.blueF());
                        }

                        // Head for an arrow
                        double vh1x = point.x + dm/5.0 * cos(angle - M_PI/2.0) + dm * cos(angle);
                        double vh1y = point.y + dm/5.0 * sin(angle - M_PI/2.0) + dm * sin(angle);
                        double vh2x = point.x + dm/5.0 * cos(angle + M_PI/2.0) + dm * cos(angle);
                        double vh2y = point.y + dm/5.0 * sin(angle + M_PI/2.0) + dm * sin(angle);
                        double vh3x = point.x + dm * cos(angle) + dm * cos(angle);
                        double vh3y = point.y + dm * sin(angle) + dm * sin(angle);
                        glVertex2d(vh1x,vh1y); glVertex2d(vh2x,vh2y); glVertex2d(vh3x,vh3y);

                        // Shaft for an arrow
                        double vs1x = point.x + dm/15.0 * cos(angle + M_PI/2.0) + dm * cos(angle);
                        double vs1y = point.y + dm/15.0 * sin(angle + M_PI/2.0) + dm * sin(angle);
                        double vs2x = point.x + dm/15.0 * cos(angle - M_PI/2.0) + dm * cos(angle);
                        double vs2y = point.y + dm/15.0 * sin(angle - M_PI/2.0) + dm * sin(angle);
                        double vs3x = vs1x - dm * cos(angle);
                        double vs3y = vs1y - dm * sin(angle);
                        double vs4x = vs2x - dm * cos(angle);
                        double vs4y = vs2y - dm * sin(angle);
                        glVertex2d(vs1x,vs1y); glVertex2d(vs2x,vs2y); glVertex2d(vs3x,vs3y);
                        glVertex2d(vs4x,vs4y); glVertex2d(vs3x,vs3y); glVertex2d(vs2x,vs2y);

                    }
                }
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_scene->sceneSolution()->vecVectorView().unlock_data();

        glEndList();

        glCallList(m_listVectors);
    }
    else
    {
        glCallList(m_listVectors);
    }
}

void SceneView::paintSceneModeLabel()
{
    logMessage("SceneView::paintSceneModeLabel()");

    QString text = "";

    switch (m_sceneMode)
    {
    case SceneMode_OperateOnNodes:
        text = tr("Operate on nodes");
        break;
    case SceneMode_OperateOnEdges:
        text = tr("Operate on edges");
        break;
    case SceneMode_OperateOnLabels:
        text = tr("Operate on labels");
        break;
    case SceneMode_Postprocessor:
        switch (m_sceneViewSettings.postprocessorShow)
        {
        case SceneViewPostprocessorShow_ScalarView:
        case SceneViewPostprocessorShow_ScalarView3D:
        case SceneViewPostprocessorShow_ScalarView3DSolid:
            text = QString::fromStdString(m_sceneViewSettings.scalarPhysicFieldVariable != "" ? Util::scene()->fieldInfo("TODO")->module()->get_variable(m_sceneViewSettings.scalarPhysicFieldVariable)->name : "");
            if (m_sceneViewSettings.scalarPhysicFieldVariableComp != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString(m_sceneViewSettings.scalarPhysicFieldVariableComp);
            break;
        case SceneViewPostprocessorShow_Model:
            text = tr("Model");
            break;
        case SceneViewPostprocessorShow_Order:
            text = tr("Polynomial order");
            break;
        default:
            text = tr("Postprocessor");
        }
        break;
    }

    loadProjection2d();

    glLoadIdentity();

    glScaled(2.0/contextWidth(), 2.0/contextHeight(), 1.0);
    glTranslated(-contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    glDisable(GL_DEPTH_TEST);

    // render viewport label
    QFont fontLabel = font();
    fontLabel.setPointSize(fontLabel.pointSize() + 1);

    Point posText = Point((contextWidth()-QFontMetrics(fontLabel).width(text)) / 2.0,
                          (contextHeight() - QFontMetrics(fontLabel).height() / 1.3));

    // blended rectangle
    double xs = posText.x - QFontMetrics(fontLabel).width(" ");
    double ys = posText.y - QFontMetrics(fontLabel).height() / 3.0;
    double xe = xs + QFontMetrics(fontLabel).width(text + "  ");
    double ye = contextHeight();

    drawBlend(Point(xs, ys), Point(xe, ye), 0.8, 0.8, 0.8, 0.93);

    // text
    glColor3d(0.0, 0.0, 0.0);
    renderText(posText.x, posText.y, 0.0, text, fontLabel);
}

void SceneView::paintZoomRegion()
{
    logMessage("SceneView::paintZoomRegion()");

    loadProjection2d(true);

    // zoom or select region
    if (m_region)
    {
        Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        drawBlend(posStart, posEnd,
                  Util::config()->colorHighlighted.redF(),
                  Util::config()->colorHighlighted.greenF(),
                  Util::config()->colorHighlighted.blueF());
    }
}

void SceneView::paintSnapToGrid()
{
    logMessage("SceneView::paintSnapToGrid()");

    if (m_snapToGrid)
    {
        loadProjection2d(true);

        Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

        Point snapPoint;
        snapPoint.x = floor(p.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
        snapPoint.y = floor(p.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;

        glColor3d(Util::config()->colorHighlighted.redF(),
                  Util::config()->colorHighlighted.greenF(),
                  Util::config()->colorHighlighted.blueF());
        glPointSize(Util::config()->nodeSize - 1.0);
        glBegin(GL_POINTS);
        glVertex2d(snapPoint.x, snapPoint.y);
        glEnd();
    }
}

void SceneView::paintChartLine()
{
    logMessage("SceneView::paintChartLine()");

    loadProjection2d(true);

    glColor3d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF());
    glLineWidth(3.0);

    QList<Point> points = m_chartLine.getPoints();

    glBegin(GL_LINES);
    for (int i = 0; i < points.length() - 1; i++)
    {
        glVertex2d(points.at(i).x, points.at(i).y);
        glVertex2d(points.at(i+1).x, points.at(i+1).y);
    }
    glEnd();

    // angle
    glBegin(GL_TRIANGLES);
    if (points.length() > 1)
    {
        double angle = atan2(points.at(points.length() - 1).y - points.at(points.length() - 2).y,
                             points.at(points.length() - 1).x - points.at(points.length() - 2).x);

        RectPoint rect = m_scene->boundingBox();
        double dm = (rect.width() + rect.height()) / 40.0;

        // head of an arrow
        double vh1x = points.at(points.length() - 1).x - dm/5.0 * cos(angle - M_PI/2.0) - dm * cos(angle);
        double vh1y = points.at(points.length() - 1).y - dm/5.0 * sin(angle - M_PI/2.0) - dm * sin(angle);
        double vh2x = points.at(points.length() - 1).x - dm/5.0 * cos(angle + M_PI/2.0) - dm * cos(angle);
        double vh2y = points.at(points.length() - 1).y - dm/5.0 * sin(angle + M_PI/2.0) - dm * sin(angle);
        double vh3x = points.at(points.length() - 1).x;
        double vh3y = points.at(points.length() - 1).y;
        glVertex2d(vh1x, vh1y);
        glVertex2d(vh2x, vh2y);
        glVertex2d(vh3x, vh3y);
    }
    glEnd();
}

void SceneView::paintEdgeLine()
{
    logMessage("SceneView::paintEdgeLine()");

    if (m_nodeLast)
    {
        if (m_nodeLast->isSelected)
        {
            loadProjection2d(true);

            Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x8FFF);

            glColor3d(Util::config()->colorEdges.redF(),
                      Util::config()->colorEdges.greenF(),
                      Util::config()->colorEdges.blueF());
            glLineWidth(Util::config()->edgeWidth);

            glBegin(GL_LINES);
            glVertex2d(m_nodeLast->point.x, m_nodeLast->point.y);
            glVertex2d(p.x, p.y);
            glEnd();

            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
        }
    }
    else
    {
        return;
    }
}

const double* SceneView::paletteColor(double x) const
{
    logMessage("SceneView::paletteColor()");

    switch (Util::config()->paletteType)
    {
    case Palette_Jet:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataJet[n];
    }
        break;
    case Palette_Copper:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataCopper[n];
    }
        break;
    case Palette_Hot:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataHot[n];
    }
        break;
    case Palette_Cool:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataCool[n];
    }
        break;
    case Palette_Bone:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataBone[n];
    }
        break;
    case Palette_Pink:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataPink[n];
    }
        break;
    case Palette_Spring:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataSpring[n];
    }
        break;
    case Palette_Summer:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataSummer[n];
    }
        break;
    case Palette_Autumn:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataAutumn[n];
    }
        break;
    case Palette_Winter:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataWinter[n];
    }
        break;
    case Palette_HSV:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= numPalEntries;
        int n = (int) x;
        return paletteDataHSV[n];
    }
        break;
    case Palette_BWAsc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = x;
        return color;
    }
        break;
    case Palette_BWDesc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = 1.0 - x;
        return color;
    }
        break;
    default:
        qWarning() << tr("Undefined: %1.").arg(Util::config()->paletteType);
        return NULL;
    }
}

const double* SceneView::paletteColorOrder(int n) const
{
    logMessage("SceneView::paletteColorOrder()");

    switch (Util::config()->orderPaletteOrderType)
    {
    case PaletteOrder_Hermes:
        return paletteOrderHermes[n];
    case PaletteOrder_Jet:
        return paletteOrderJet[n];
    case PaletteOrder_Copper:
        return paletteOrderCopper[n];
    case PaletteOrder_Hot:
        return paletteOrderHot[n];
    case PaletteOrder_Cool:
        return paletteOrderCool[n];
    case PaletteOrder_Bone:
        return paletteOrderBone[n];
    case PaletteOrder_Pink:
        return paletteOrderPink[n];
    case PaletteOrder_Spring:
        return paletteOrderSpring[n];
    case PaletteOrder_Summer:
        return paletteOrderSummer[n];
    case PaletteOrder_Autumn:
        return paletteOrderAutumn[n];
    case PaletteOrder_Winter:
        return paletteOrderWinter[n];
    case PaletteOrder_HSV:
        return paletteOrderHSV[n];
    case PaletteOrder_BWAsc:
        return paletteOrderBWAsc[n];
    case PaletteOrder_BWDesc:
        return paletteOrderBWDesc[n];
    default:
        qWarning() << tr("Undefined: %1.").arg(Util::config()->orderPaletteOrderType);
        return NULL;
    }
}

void SceneView::paletteCreate()
{
    logMessage("SceneView::paletteCreate()");

    int i;
    unsigned char palette[256][3];
    for (i = 0; i < Util::config()->paletteSteps; i++)
    {
        const double* color = paletteColor((double) i / Util::config()->paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (i = Util::config()->paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[Util::config()->paletteSteps-1], 3);

    glBindTexture(GL_TEXTURE_1D, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void SceneView::paletteFilter()
{
    logMessage("SceneView::paletteFilter()");

    int palFilter = Util::config()->paletteFilter ? GL_LINEAR : GL_NEAREST;
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, palFilter);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, palFilter);
    paletteUpdateTexAdjust();
}

void SceneView::paletteUpdateTexAdjust()
{
    logMessage("SceneView::paletteUpdateTexAdjust()");

    if (Util::config()->paletteFilter)
    {
        m_texScale = (double) (Util::config()->paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) Util::config()->paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

void SceneView::initLighting()
{
    logMessage("SceneView::initLighting()");

    if (Util::config()->scalarView3DLighting || m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Model)
    {
        // environment
        float light_specular[] = {  1.0f, 1.0f, 1.0f, 1.0f };
        float light_ambient[]  = {  0.7f, 0.7f, 0.7f, 1.0f };
        float light_diffuse[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
        float light_position[] = {  1.0f, 0.0f, 1.0f, 0.0f };

        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        // material
        float material_ambient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
        float material_diffuse[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
        float material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

        // glEnable(GL_COLOR_MATERIAL);
#if defined(GL_LIGHT_MODEL_COLOR_CONTROL) && defined(GL_SEPARATE_SPECULAR_COLOR)
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
}

// events *****************************************************************************************************************************

void SceneView::keyPressEvent(QKeyEvent *event)
{
    logMessage("SceneView::keyPressEvent()");

    if (is3DMode())
    {

    }
    else
    {
        if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
            emit mouseSceneModeChanged(MouseSceneMode_Add);
        if (!(event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
            emit mouseSceneModeChanged(MouseSceneMode_Pan);
        if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
            emit mouseSceneModeChanged(MouseSceneMode_Move);

        Point stepTemp = position(Point(contextWidth(), contextHeight()));
        stepTemp.x = stepTemp.x - m_offset2d.x;
        stepTemp.y = stepTemp.y - m_offset2d.y;
        double step = qMin(stepTemp.x, stepTemp.y) / 10.0;

        switch (event->key())
        {
        case Qt::Key_Up:
        {
            m_offset2d.y += step;
            refresh();
        }
            break;
        case Qt::Key_Down:
        {
            m_offset2d.y -= step;
            refresh();
        }
            break;
        case Qt::Key_Left:
        {
            m_offset2d.x -= step;
            refresh();
        }
            break;
        case Qt::Key_Right:
        {
            m_offset2d.x += step;
            refresh();
        }
            break;
        case Qt::Key_Plus:
        {
            doZoomIn();
        }
            break;
        case Qt::Key_Minus:
        {
            doZoomOut();
        }
            break;
        case Qt::Key_Delete:
        {
            m_scene->deleteSelected();
        }
            break;
        case Qt::Key_Space:
        {
            doSceneObjectProperties();
        }
            break;
        case Qt::Key_Escape:
        {
            m_nodeLast = NULL;
            m_scene->selectNone();
            emit mousePressed();
            refresh();
        }
            break;
        case Qt::Key_N:
        {
            // add node with coordinates under mouse pointer
            if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
            {
                Point p = position(Point(m_lastPos.x(), m_lastPos.y()));
                m_scene->doNewNode(p);
            }
        }
            break;
        case Qt::Key_L:
        {
            // add label with coordinates under mouse pointer
            if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
            {
                Point p = position(Point(m_lastPos.x(), m_lastPos.y()));
                m_scene->doNewLabel(p);
            }
        }
            break;
        case Qt::Key_A:
        {
            // select all
            if (event->modifiers() & Qt::ControlModifier)
            {
                if (m_sceneMode == SceneMode_Postprocessor)
                {
                    // select volume integral area
                    if (actPostprocessorModeVolumeIntegral->isChecked())
                    {
                        m_scene->selectAll(SceneMode_OperateOnLabels);
                        emit mousePressed();
                    }

                    // select surface integral area
                    if (actPostprocessorModeSurfaceIntegral->isChecked())
                    {
                        m_scene->selectAll(SceneMode_OperateOnEdges);
                        emit mousePressed();
                    }
                }
                else
                {
                    m_scene->selectAll(m_sceneMode);
                }

                refresh();
            }
        }
            break;
        default:
            QGLWidget::keyPressEvent(event);
        }

        // snap to grid
        m_snapToGrid = ((Util::config()->snapToGrid)
                        && (event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier)
                        && (m_sceneMode == SceneMode_OperateOnNodes));
    }
}

void SceneView::keyReleaseEvent(QKeyEvent *event)
{
    logMessage("SceneView::keyReleaseEvent()");

    if (is3DMode())
    {

    }
    else
    {
        // if (m_snapToGrid)
        {
            m_snapToGrid = false;
            updateGL();
        }
    }

    emit mouseSceneModeChanged(MouseSceneMode_Nothing);
}

void SceneView::mousePressEvent(QMouseEvent *event)
{
    logMessage("SceneView::mousePressEvent()");

    m_lastPos = event->pos();

    if (is3DMode())
    {

    }
    else
    {
        Point p = position(Point(event->pos().x(), event->pos().y()));

        // zoom or select region or select local value, volume and surface integral
        // without modificators + left mouse
        if ((event->button() & Qt::LeftButton)
                && !(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier))
        {
            // zoom region
            if (actSceneZoomRegion->isChecked())
            {
                m_regionPos = m_lastPos;
                actSceneZoomRegion->setChecked(false);
                actSceneZoomRegion->setData(true);
                m_region = true;

                return;
            }

            // select region
            if (actSceneViewSelectRegion->isChecked())
            {
                m_regionPos = m_lastPos;
                actSceneViewSelectRegion->setChecked(false);
                actSceneViewSelectRegion->setData(true);
                m_region = true;

                return;
            }

            if ((m_sceneMode == SceneMode_Postprocessor) &&
                    !(m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
                      m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
            {
                // local point value
                if (actPostprocessorModeLocalPointValue->isChecked())
                    emit mousePressed(p);
                // select volume integral area
                if (actPostprocessorModeVolumeIntegral->isChecked())
                {
                    int index = m_scene->sceneSolution()->findElementInMesh(m_scene->sceneSolution()->meshInitial(), p);
                    if (index != -1)
                    {
                        //  find label marker
                        int labelIndex = atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(m_scene->sceneSolution()->meshInitial()->get_element_fast(index)->marker).marker.c_str());

                        m_scene->labels->at(labelIndex)->isSelected = !m_scene->labels->at(labelIndex)->isSelected;
                        updateGL();
                    }
                    emit mousePressed();
                }
                // select surface integral area
                if (actPostprocessorModeSurfaceIntegral->isChecked())
                {
                    //  find edge marker
                    SceneEdge *edge = findClosestEdge(p);

                    edge->isSelected = !edge->isSelected;
                    updateGL();

                    emit mousePressed();
                }
            }
        }

        // add node, edge or label by mouse click
        // control + left mouse
        if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
        {
            // add node directly by mouse click
            if (m_sceneMode == SceneMode_OperateOnNodes)
            {
                Point pointNode;

                // snap to grid
                if (m_snapToGrid)
                {
                    Point snapPoint = position(Point(m_lastPos.x(), m_lastPos.y()));

                    pointNode.x = floor(snapPoint.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
                    pointNode.y = floor(snapPoint.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
                }
                else
                {
                    pointNode = p;
                }

                // coordinates must be greater then or equal to 0 (axisymmetric case)
                if ((Util::scene()->problemInfo()->coordinateType == CoordinateType_Axisymmetric) &&
                        (pointNode.x < 0))
                {
                    QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));

                }
                else
                {
                    SceneNode *node = new SceneNode(pointNode);
                    SceneNode *nodeAdded = m_scene->addNode(node);
                    if (nodeAdded == node) m_scene->undoStack()->push(new SceneNodeCommandAdd(node->point));
                    updateGL();
                }
            }
            if (m_sceneMode == SceneMode_OperateOnEdges)
            {
                // add edge directly by mouse click
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    if (m_nodeLast == NULL)
                    {
                        m_nodeLast = node;
                        m_nodeLast->isSelected = true;
                    }
                    else
                    {
                        if (node != m_nodeLast)
                        {
                            SceneEdge *edge = new SceneEdge(m_nodeLast, node, 0, 0); //TODO - do it better
                            SceneEdge *edgeAdded = m_scene->addEdge(edge);
                            if (edgeAdded == edge) m_scene->undoStack()->push(new SceneEdgeCommandAdd(edge->nodeStart->point,
                                                                                                      edge->nodeEnd->point,
                                                                                                      "TODO",
                                                                                                      edge->angle,
                                                                                                      edge->refineTowardsEdge));
                        }

                        m_nodeLast->isSelected = false;
                        m_nodeLast = NULL;
                    }

                    updateGL();
                }
            }
            // add label directly by mouse click
            if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                // coordinates must be greater then or equal to 0 (axisymmetric case)
                if ((Util::scene()->problemInfo()->coordinateType == CoordinateType_Axisymmetric) &&
                        (p.x < 0))
                {
                    QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));

                }
                else
                {
                    SceneLabel *label = new SceneLabel(p, 0, 0); //TODO - do it better
                    SceneLabel *labelAdded = m_scene->addLabel(label);
                    if (labelAdded == label) m_scene->undoStack()->push(new SceneLabelCommandAdd(label->point,
                                                                                                 "TODO",
                                                                                                 label->area,
                                                                                                 label->polynomialOrder));
                    updateGL();
                }
            }
        }

        // multiple select or just one node or label due to movement
        // nothing or (shift + control) + left mouse
        if ((event->button() & Qt::LeftButton) && ((event->modifiers() == 0)
                                                   || ((event->modifiers() & Qt::ControlModifier)
                                                       && (event->modifiers() & Qt::ShiftModifier)
                                                       && (m_scene->selectedCount() == 0))))
        {
            // select scene objects
            if (m_sceneMode == SceneMode_OperateOnNodes)
            {
                // select the closest node
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    node->isSelected = !node->isSelected;
                    updateGL();
                }
            }

            if (m_sceneMode == SceneMode_OperateOnEdges)
            {
                // select the closest label
                SceneEdge *edge = findClosestEdge(p);
                if (edge)
                {
                    edge->isSelected = !edge->isSelected;
                    updateGL();
                }
            }

            if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                // select the closest label
                SceneLabel *label = findClosestLabel(p);
                if (label)
                {
                    label->isSelected = !label->isSelected;
                    updateGL();
                }
            }
        }
    }
}

void SceneView::mouseDoubleClickEvent(QMouseEvent * event)
{
    logMessage("SceneView::mouseDoubleClickEvent()");

    if (is3DMode())
    {

    }
    else
    {
        Point p = position(Point(event->pos().x(), event->pos().y()));

        // zoom best fit
        if (!(event->modifiers() & Qt::ControlModifier))
        {
            if ((event->buttons() & Qt::MidButton)
                    || ((event->buttons() & Qt::LeftButton)
                        && ((!(event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)))))
            {
                doZoomBestFit();
                return;
            }

            if (event->button() & Qt::LeftButton)
            {
                // select scene objects
                m_scene->selectNone();
                if (m_sceneMode == SceneMode_OperateOnNodes)
                {
                    // select the closest node
                    SceneNode *node = findClosestNode(p);
                    if (node)
                    {
                        node->isSelected = true;
                        updateGL();
                        if (node->showDialog(this) == QDialog::Accepted)
                        {
                            updateGL();
                        }
                    }
                }
                if (m_sceneMode == SceneMode_OperateOnEdges)
                {
                    // select the closest label
                    SceneEdge *edge = findClosestEdge(p);
                    if (edge)
                    {
                        edge->isSelected = true;
                        updateGL();
                        if (edge->showDialog(this) == QDialog::Accepted)
                        {
                            updateGL();
                        }
                    }
                }
                if (m_sceneMode == SceneMode_OperateOnLabels)
                {
                    // select the closest label
                    SceneLabel *label = findClosestLabel(p);
                    if (label)
                    {
                        label->isSelected = true;
                        updateGL();
                        if (label->showDialog(this) == QDialog::Accepted)
                        {
                            updateGL();
                        }
                    }
                }
                m_scene->selectNone();
                updateGL();
            }
        }
    }
}

void SceneView::mouseReleaseEvent(QMouseEvent *event)
{
    logMessage("SceneView::mouseReleaseEvent()");

    setCursor(Qt::ArrowCursor);

    if (is3DMode())
    {

    }
    else
    {
        // zoom or select region
        actSceneZoomRegion->setChecked(false);
        actSceneViewSelectRegion->setChecked(false);

        if (m_region)
        {
            Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
            Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

            if (actSceneZoomRegion->data().value<bool>())
                doZoomRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));
            if (actSceneViewSelectRegion->data().value<bool>())
                selectRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));

            actSceneZoomRegion->setData(false);
            actSceneViewSelectRegion->setData(false);
        }

        m_region = false;

        // move by mouse - select none
        if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
        {
            m_scene->selectNone();
        }
    }

    emit mouseSceneModeChanged(MouseSceneMode_Nothing);

    updateGL();
}

void SceneView::mouseMoveEvent(QMouseEvent *event)
{
    logMessage("SceneView::mouseMoveEvent()");

    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    setToolTip("");

    if (is3DMode())
    {
        // pan
        if ((event->buttons() & Qt::MidButton)
                || ((event->buttons() & Qt::LeftButton)
                    && (((event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))))

        {
            setCursor(Qt::PointingHandCursor);

            m_offset3d.x -= 2.0/contextWidth() * dx*aspect();
            m_offset3d.y += 2.0/contextHeight() * dy;

            emit mouseSceneModeChanged(MouseSceneMode_Pan);

            updateGL();
        }

        // rotate
        if ((event->buttons() & Qt::LeftButton)
                && (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))
        {
            setCursor(Qt::PointingHandCursor);

            m_rotation3d.x -= dy;
            m_rotation3d.y += dx;

            emit mouseSceneModeChanged(MouseSceneMode_Rotate);

            updateGL();
        }
        if ((event->buttons() & Qt::LeftButton)
                && (!(event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier)))
        {
            setCursor(Qt::PointingHandCursor);

            m_rotation3d.z -= dy;

            emit mouseSceneModeChanged(MouseSceneMode_Rotate);

            updateGL();
        }
    }
    else
    {
        Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

        // highlight scene objects + hints
        if ((event->modifiers() == 0)
                || ((event->modifiers() & Qt::ControlModifier)
                    && (event->modifiers() & Qt::ShiftModifier)
                    && (m_scene->selectedCount() == 0)))
        {
            // highlight scene objects
            if (m_sceneMode == SceneMode_OperateOnNodes)
            {
                // highlight the closest node
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    m_scene->highlightNone();
                    node->isHighlighted = true;
                    setToolTip(tr("<h3>Node</h3>Point: [%1; %2]<br/>Index: %3").
                               arg(node->point.x, 0, 'g', 3).
                               arg(node->point.y, 0, 'g', 3).
                               arg(m_scene->nodes->items().indexOf(node)));
                    updateGL();
                }
            }
            if (m_sceneMode == SceneMode_OperateOnEdges)
            {
                // highlight the closest edge
                SceneEdge *edge = findClosestEdge(p);
                if (edge)
                {
                    // assigned boundary conditions
                    QString str;
                    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                        str = str + QString("%1 (%2), ").
                                arg(QString::fromStdString(edge->getMarker(fieldInfo)->getName())).
                                arg(QString::fromStdString(fieldInfo->module()->name));
                    if (str.length() > 0)
                        str = str.left(str.length() - 2);

                    m_scene->highlightNone();
                    edge->isHighlighted = true;
                    setToolTip(tr("<h3>Edge</h3><b>Point:</b> [%1; %2] - [%3; %4]<br/><b>Boundary conditions:</b> %5<br/><b>Angle:</b> %6 deg.<br/><b>Refine towards edge:</b> %7<br/><b>Index:</b> %8").
                               arg(edge->nodeStart->point.x, 0, 'g', 3).
                               arg(edge->nodeStart->point.y, 0, 'g', 3).
                               arg(edge->nodeEnd->point.x, 0, 'g', 3).
                               arg(edge->nodeEnd->point.y, 0, 'g', 3).
                               arg(str).
                               arg(edge->angle, 0, 'f', 0).
                               arg(edge->refineTowardsEdge, 0, 'g', 3).
                               arg(m_scene->edges->items().indexOf(edge)));
                    updateGL();
                }
            }
            if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                // highlight the closest label
                SceneLabel *label = findClosestLabel(p);
                if (label)
                {
                    // assigned materials
                    QString str;
                    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                        str = str + QString("%1 (%2), ").
                                arg(QString::fromStdString(label->getMarker(fieldInfo)->getName())).
                                arg(QString::fromStdString(fieldInfo->module()->name));
                    if (str.length() > 0)
                        str = str.left(str.length() - 2);

                    m_scene->highlightNone();
                    label->isHighlighted = true;
                    setToolTip(tr("<h3>Label</h3><b>Point:</b> [%1; %2]<br/><b>Materials:</b> %3<br/><b>Triangle area:</b> %4 m<sup>2</sup><br/><b>Polynomial order:</b> %5<br/><b>Index:</b> %6").
                               arg(label->point.x, 0, 'g', 3).
                               arg(label->point.y, 0, 'g', 3).
                               arg(str).
                               arg(label->area, 0, 'g', 3).
                               arg(label->polynomialOrder).
                               arg(m_scene->labels->items().indexOf(label)));
                    updateGL();
                }
            }
        }

        // add edge by mouse - draw line
        if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
        {
            // add edge directly by mouse click - highlight
            if (m_sceneMode == SceneMode_OperateOnEdges)
            {
                // add edge directly by mouse click
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    m_scene->highlightNone();
                    node->isHighlighted = true;
                    updateGL();
                }
            }
        }

        // zoom or select region
        if (m_region)
            updateGL();

        // snap to grid - nodes
        m_snapToGrid = ((Util::config()->snapToGrid)
                        && ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
                        && (m_sceneMode == SceneMode_OperateOnNodes));
        if (m_snapToGrid && !(event->modifiers() & Qt::ControlModifier))
        {
            m_snapToGrid = false;
            updateGL();
        }
        if (m_snapToGrid && (event->modifiers() & Qt::ControlModifier))
            updateGL();

        // pan - middle button or shift + left mouse
        if ((event->buttons() & Qt::MidButton)
                || ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))
        {
            setCursor(Qt::PointingHandCursor);

            m_offset2d.x -= 2.0/contextWidth() * dx/m_scale2d*aspect();
            m_offset2d.y += 2.0/contextHeight() * dy/m_scale2d;

            emit mouseSceneModeChanged(MouseSceneMode_Pan);

            updateGL();
        }

        // move nodes and labels directly by mouse - left mouse + control + shift
        if ((event->buttons() & Qt::LeftButton)
                && ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)))
        {
            Point dp = Point(2.0/contextWidth() * dx/m_scale2d*aspect(), -2.0/contextHeight() * dy/m_scale2d);

            if (m_sceneMode == SceneMode_OperateOnNodes)
            {
                // mouse move length memory
                static Point len;
                len = len + dp;

                if (Util::config()->snapToGrid)
                {
                    if (fabs(len.x) > Util::config()->gridStep)
                    {
                        foreach (SceneNode *node, m_scene->nodes->items())
                            if (node->isSelected)
                                node->point.x += (len.x > 0) ? Util::config()->gridStep : -Util::config()->gridStep;
                        len.x = 0;
                        updateGL();
                    }

                    if (fabs(len.y) > Util::config()->gridStep)
                    {
                        foreach (SceneNode *node, m_scene->nodes->items())
                            if (node->isSelected)
                                node->point.y += (len.y > 0) ? Util::config()->gridStep : -Util::config()->gridStep;
                        len.y = 0;
                        updateGL();
                    }
                }
                else
                {
                    m_scene->transformTranslate(dp, false);
                    updateGL();
                }
            }
            else if (m_sceneMode == SceneMode_OperateOnEdges)
            {
                // mouse move length memory
                static Point len;
                len = len + dp;

                if (Util::config()->snapToGrid)
                {
                    if (fabs(len.x) > Util::config()->gridStep)
                    {
                        foreach (SceneEdge *edge, m_scene->edges->items())
                            if (edge->isSelected)
                            {
                                edge->nodeStart->isSelected = true;
                                edge->nodeEnd->isSelected = true;
                            }
                        foreach (SceneNode *node, m_scene->nodes->items())
                            if (node->isSelected)
                                node->point.x += (len.x > 0) ? Util::config()->gridStep : -Util::config()->gridStep;

                        len.x = 0;
                        updateGL();
                    }

                    if (fabs(len.y) > Util::config()->gridStep)
                    {
                        foreach (SceneEdge *edge, m_scene->edges->items())
                            foreach (SceneEdge *edge, m_scene->edges->items())
                                if (edge->isSelected)
                                {
                                    edge->nodeStart->isSelected = true;
                                    edge->nodeEnd->isSelected = true;
                                }
                        foreach (SceneNode *node, m_scene->nodes->items())
                            if (node->isSelected)
                                node->point.y += (len.y > 0) ? Util::config()->gridStep : -Util::config()->gridStep;

                        len.y = 0;
                        updateGL();
                    }
                }
                else
                {
                    m_scene->transformTranslate(dp, false);
                    updateGL();
                }
            }
            else if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                m_scene->transformTranslate(dp, false);
                updateGL();
            }
        }

        if (m_snapToGrid)
        {
            Point snapPoint;
            snapPoint.x = floor(p.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
            snapPoint.y = floor(p.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;

            emit mouseMoved(QPointF(snapPoint.x, snapPoint.y));
        }
        else
        {
            emit mouseMoved(QPointF(p.x, p.y));
        }
    }
}

void SceneView::wheelEvent(QWheelEvent *event)
{
    logMessage("SceneView::wheelEvent()");

    if (is3DMode())
    {
        setZoom(event->delta()/150.0);
    }
    else
    {
        if (Util::config()->zoomToMouse)
        {
            Point posMouse;
            posMouse = Point((2.0/contextWidth()*(event->pos().x() - contextWidth()/2.0))/m_scale2d*aspect(),
                             -(2.0/contextHeight()*(event->pos().y() - contextHeight()/2.0))/m_scale2d);

            m_offset2d.x += posMouse.x;
            m_offset2d.y += posMouse.y;

            m_scale2d = m_scale2d * pow(1.2, event->delta()/150.0);

            posMouse = Point((2.0/contextWidth()*(event->pos().x() - contextWidth()/2.0))/m_scale2d*aspect(),
                             -(2.0/contextHeight()*(event->pos().y() - contextHeight()/2.0))/m_scale2d);

            m_offset2d.x -= posMouse.x;
            m_offset2d.y -= posMouse.y;

            updateGL();
        }
        else
        {
            setZoom(event->delta()/150.0);
        }
    }
}

void SceneView::contextMenuEvent(QContextMenuEvent *event)
{
    logMessage("SceneView::contextMenuEvent()");

    actSceneObjectProperties->setEnabled(false);

    // set boundary context menu
    if (m_sceneMode == SceneMode_OperateOnEdges)
        actSceneObjectProperties->setEnabled(m_scene->selectedCount() > 0);

    // set material context menu
    if (m_sceneMode == SceneMode_OperateOnLabels)
        actSceneObjectProperties->setEnabled(m_scene->selectedCount() > 0);


    if(mnuScene)
        delete mnuScene;
    createMenu();

    mnuScene->exec(event->globalPos());
}

void SceneView::closeEvent(QCloseEvent *event)
{
    logMessage("SceneView::closeEvent()");

    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneView::doZoomBestFit()
{
    logMessage("SceneView::doZoomBestFit()");

    RectPoint rect = m_scene->boundingBox();
    doZoomRegion(rect.start, rect.end);
}

void SceneView::doZoomIn()
{
    logMessage("SceneView::doZoomIn()");

    setZoom(1.2);
}

void SceneView::doZoomOut()
{
    logMessage("SceneView::doZoomOut()");

    setZoom(-1/1.2);
}

void SceneView::doZoomRegion(const Point &start, const Point &end)
{
    logMessage("SceneView::doZoomRegion()");

    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO) return;

    m_offset2d.x = (start.x+end.x)/2.0;
    m_offset2d.y = (start.y+end.y)/2.0;

    double sceneWidth = end.x-start.x;
    double sceneHeight = end.y-start.y;

    double maxScene = (((double) contextWidth() / (double) contextHeight()) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
    {
        m_scale2d = 1.95/maxScene;
        m_scale3d = 0.6 * m_scale2d;
    }

    setZoom(0);
}

void SceneView::doShowGrid()
{
    logMessage("SceneView::doShowGrid()");

    Util::config()->showGrid = !Util::config()->showGrid;
    Util::config()->save();
    doInvalidated();
}

void SceneView::doSnapToGrid()
{
    logMessage("SceneView::doSnapToGrid()");

    Util::config()->snapToGrid = !Util::config()->snapToGrid;
    Util::config()->save();
}

void SceneView::doShowRulers()
{
    logMessage("SceneView::doShowRulers()");

    Util::config()->showRulers = !Util::config()->showRulers;
    Util::config()->save();
    doInvalidated();
}

void SceneView::doSetChartLine(const ChartLine &chartLine)
{
    logMessage("SceneView::doSetChartLine()");

    // set line for chart
    m_chartLine = chartLine;

    updateGL();
}

void SceneView::doDefaultValues()
{
    logMessage("SceneView::doDefaultValues()");

    m_sceneMode = SceneMode_OperateOnNodes;

    m_snapToGrid = false;
    m_region = false;
    m_isSolutionPrepared = false;

    // 2d
    m_scale2d = 1.0;
    m_offset2d = Point();

    // 3d
    m_scale3d = 0.6;
    m_offset3d = Point();
    m_rotation3d = Point3();
    m_rotation3d.x = 66.0;
    m_rotation3d.y = -35.0;
    m_rotation3d.z = 0.0;

    m_chartLine = ChartLine();

    m_sceneViewSettings.defaultValues();

    m_nodeLast = NULL;

    deleteTexture(m_backgroundTexture);
    m_backgroundTexture = -1;

    doInvalidated();
    doZoomBestFit();

    actPostprocessorModeLocalPointValue->trigger();
}

void SceneView::solved()
{
    logMessage("SceneView::solved()");

    actSceneModePostprocessor->trigger();
    m_sceneViewSettings.showInitialMesh = false;
}

void SceneView::doInvalidated()
{
    logMessage("SceneView::doInvalidated()");

    resizeGL(width(), height());

    if (!m_scene->sceneSolution()->isSolved())
    {
        if (m_sceneMode == SceneMode_Postprocessor)
            actSceneModeNode->trigger();
    }

    actSceneModePostprocessor->setEnabled(m_scene->sceneSolution()->isSolved());
    actSceneViewSelectMarker->setEnabled(m_scene->sceneSolution()->isSolved());
    actSceneZoomRegion->setEnabled(!is3DMode());

    m_scene->actDeleteSelected->setEnabled(m_sceneMode != SceneMode_Postprocessor);
    m_scene->actTransform->setEnabled(m_sceneMode != SceneMode_Postprocessor);
    actSceneViewSelectRegion->setEnabled(m_sceneMode != SceneMode_Postprocessor);

    actPostprocessorModeLocalPointValue->setEnabled(m_sceneMode == SceneMode_Postprocessor && !is3DMode());
    actPostprocessorModeSurfaceIntegral->setEnabled(m_sceneMode == SceneMode_Postprocessor && !is3DMode());
    actPostprocessorModeVolumeIntegral->setEnabled(m_sceneMode == SceneMode_Postprocessor && !is3DMode());

    actSceneShowGrid->setChecked(Util::config()->showGrid);
    actSceneSnapToGrid->setChecked(Util::config()->snapToGrid);
    actSceneSnapToGrid->setEnabled(m_sceneMode != SceneMode_Postprocessor);
    actSceneShowRulers->setChecked(Util::config()->showRulers);

    actSetProjectionXY->setEnabled(is3DMode());
    actSetProjectionXZ->setEnabled(is3DMode());
    actSetProjectionYZ->setEnabled(is3DMode());

    setSceneFont();

    emit mousePressed();

    paintGL();
    updateGL();
}

void SceneView::timeStepChanged(bool showViewProgress)
{
    logMessage("SceneView::timeStepChanged()");

    if (!Util::scene()->sceneSolution()->isSolving())
    {
        QTime time;
        time.start();

        Util::scene()->sceneSolution()->processView(showViewProgress);
    }

    clearGLLists();
    m_isSolutionPrepared = true;

    refresh();
}

void SceneView::refresh()
{
    logMessage("SceneView::refresh()");

    paintGL();
    updateGL();
}

void SceneView::doMaterialGroup(QAction *action)
{
    logMessage("SceneView::doMaterialGroup()");

    if (SceneMaterial *material = action->data().value<SceneMaterial *>())
        m_scene->setMaterial(material);
}

void SceneView::doBoundaryGroup(QAction *action)
{
    logMessage("SceneView::doBoundaryGroup()");

    if (SceneBoundary *boundary = action->data().value<SceneBoundary *>())
        m_scene->setBoundary(boundary);
}

void SceneView::doShowGroup(QAction *action)
{
    logMessage("SceneView::doShowGroup()");

    m_sceneViewSettings.showContours = actShowContours->isChecked();
    m_sceneViewSettings.showVectors = actShowVectors->isChecked();
    m_sceneViewSettings.showSolutionMesh = actShowSolutionMesh->isChecked();

    doInvalidated();
}

void SceneView::doPostprocessorModeGroup(QAction *action)
{
    logMessage("SceneView::doPostprocessorModeGroup()");

    if (actPostprocessorModeLocalPointValue->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_LocalValue);
    if (actPostprocessorModeSurfaceIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_SurfaceIntegral);
    if (actPostprocessorModeVolumeIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_VolumeIntegral);

    m_scene->selectNone();
    updateGL();
}

void SceneView::doSceneObjectProperties()
{
    logMessage("SceneView::doSceneObjectProperties()");

    if (m_sceneMode == SceneMode_OperateOnEdges)
    {
        if (m_scene->selectedCount() > 1)
        {
            SceneBoundarySelectDialog boundaryDialog(this);
            boundaryDialog.exec();
        }
        if (m_scene->selectedCount() == 1)
        {
            for (int i = 0; i < m_scene->edges->length(); i++)
            {
                if (m_scene->edges->at(i)->isSelected)
                    m_scene->edges->at(i)->showDialog(this);
            }
        }
    }
    if (m_sceneMode == SceneMode_OperateOnLabels)
    {
        if (m_scene->selectedCount() > 1)
        {
            SceneMaterialSelectDialog materialDialog(this);
            materialDialog.exec();
        }
        if (m_scene->selectedCount() == 1)
        {
            for (int i = 0; i < m_scene->labels->length(); i++)
            {
                if (m_scene->labels->at(i)->isSelected)
                    m_scene->labels->at(i)->showDialog(this);
            }
        }
    }

    m_scene->selectNone();
}

void SceneView::doSceneModeSet(QAction *action)
{
    logMessage("SceneView::doSceneModeSet()");

    actSceneModePostprocessor->setEnabled(m_scene->sceneSolution()->isSolved());

    if (actSceneModeNode->isChecked()) m_sceneMode = SceneMode_OperateOnNodes;
    if (actSceneModeEdge->isChecked()) m_sceneMode = SceneMode_OperateOnEdges;
    if (actSceneModeLabel->isChecked()) m_sceneMode = SceneMode_OperateOnLabels;
    if (actSceneModePostprocessor->isChecked()) m_sceneMode = SceneMode_Postprocessor;

    m_scene->highlightNone();
    m_scene->selectNone();
    m_nodeLast = NULL;

    switch (m_sceneMode)
    {
    case SceneMode_OperateOnNodes:
        break;
    case SceneMode_OperateOnEdges:
        break;
    case SceneMode_OperateOnLabels:
        break;
    case SceneMode_Postprocessor:
        break;
    }

    doInvalidated();

    emit sceneModeChanged(m_sceneMode);
}

void SceneView::doSelectMarker()
{
    logMessage("SceneView::doSelectMarker()");

    SceneMarkerSelectDialog sceneMarkerSelectDialog(this, QApplication::activeWindow());
    sceneMarkerSelectDialog.exec();
}

void SceneView::doSelectBasic()
{
    logMessage("SceneView::doSelectBasic()");

    SceneBasicSelectDialog sceneBasicSelectDialog(this, QApplication::activeWindow());
    sceneBasicSelectDialog.exec();
}

void SceneView::processedSolutionMesh()
{
    logMessage("SceneView::processedSolutionMesh()");
}

void SceneView::processedRangeContour()
{
    logMessage("SceneView::processedRangeContour()");
}

void SceneView::processedRangeScalar()
{
    logMessage("SceneView::processedRangeScalar()");

    paletteFilter();
    paletteUpdateTexAdjust();
    paletteCreate();

    if (m_sceneViewSettings.scalarRangeAuto)
    {
        m_sceneViewSettings.scalarRangeMin = m_scene->sceneSolution()->linScalarView().get_min_value();
        m_sceneViewSettings.scalarRangeMax = m_scene->sceneSolution()->linScalarView().get_max_value();
    }
}

void SceneView::processedRangeVector()
{
    logMessage("SceneView::processedRangeVector()");
}

void SceneView::setZoom(double power)
{
    logMessage("SceneView::setZoom()");

    if (is3DMode())
    {
        m_scale3d = m_scale3d * pow(1.2, power);
    }
    else
    {
        m_scale2d = m_scale2d * pow(1.2, power);
    }

    updateGL();

    Point p(pos().x(), pos().y());
    emit mouseMoved(QPointF(position(p).x, position(p).y));
}

void SceneView::selectRegion(const Point &start, const Point &end)
{
    logMessage("SceneView::selectRegion()");

    m_scene->selectNone();

    switch (m_sceneMode)
    {
    case SceneMode_OperateOnNodes:
        foreach (SceneNode *node, m_scene->nodes->items())
            if (node->point.x >= start.x && node->point.x <= end.x && node->point.y >= start.y && node->point.y <= end.y)
                node->isSelected = true;
        break;
    case SceneMode_OperateOnEdges:
        foreach (SceneEdge *edge, m_scene->edges->items())
            if (edge->nodeStart->point.x >= start.x && edge->nodeStart->point.x <= end.x && edge->nodeStart->point.y >= start.y && edge->nodeStart->point.y <= end.y &&
                    edge->nodeEnd->point.x >= start.x && edge->nodeEnd->point.x <= end.x && edge->nodeEnd->point.y >= start.y && edge->nodeEnd->point.y <= end.y)
                edge->isSelected = true;
        break;
    case SceneMode_OperateOnLabels:
        foreach (SceneLabel *label, m_scene->labels->items())
            if (label->point.x >= start.x && label->point.x <= end.x && label->point.y >= start.y && label->point.y <= end.y)
                label->isSelected = true;
        break;
    }
}

SceneNode *SceneView::findClosestNode(const Point &point)
{
    logMessage("SceneView::findClosestNode()");

    SceneNode *nodeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneNode *node, m_scene->nodes->items())
    {
        double nodeDistance = node->distance(point);
        if (node->distance(point) < distance)
        {
            distance = nodeDistance;
            nodeClosest = node;
        }
    }

    return nodeClosest;
}

SceneEdge *SceneView::findClosestEdge(const Point &point)
{
    logMessage("SceneView::findClosestEdge()");

    SceneEdge *edgeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneEdge *edge, m_scene->edges->items())
    {
        double edgeDistance = edge->distance(point);
        if (edge->distance(point) < distance)
        {
            distance = edgeDistance;
            edgeClosest = edge;
        }
    }

    return edgeClosest;
}

SceneLabel *SceneView::findClosestLabel(const Point &point)
{
    logMessage("SceneView::findClosestLabel()");

    SceneLabel *labelClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneLabel *label, m_scene->labels->items())
    {
        double labelDistance = label->distance(point);
        if (label->distance(point) < distance)
        {
            distance = labelDistance;
            labelClosest = label;
        }
    }

    return labelClosest;
}

void SceneView::drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments) const
{
    logMessage("SceneView::drawArc()");

    double theta = arcAngle / double(segments - 1);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < segments; i++)
    {
        double arc = (startAngle + i*theta)/180.0*M_PI;

        double x = r * cos(arc);
        double y = r * sin(arc);

        glVertex3d(point.x + x, point.y + y, 0.0);
    }
    glEnd();
}

void SceneView::drawBlend(Point start, Point end, double red, double green, double blue, double alpha) const
{
    logMessage("SceneView::drawBlend()");

    // store color
    double color[4];
    glGetDoublev(GL_CURRENT_COLOR, color);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // blended rectangle
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_QUADS);
    glVertex2d(start.x, start.y);
    glVertex2d(end.x, start.y);
    glVertex2d(end.x, end.y);
    glVertex2d(start.x, end.y);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // retrieve color
    glColor4d(color[0], color[1], color[2], color[3]);
}

void SceneView::renderTextPos(double x, double y, double z, const QString &str, bool blend)
{
    logMessage("SceneView::renderTextPos()");

    if (blend)
    {
        Point size((2.0/contextWidth()*fontMetrics().width(" "))/m_scale2d*aspect(),
                   (2.0/contextHeight()*fontMetrics().height())/m_scale2d);

        double xs = x - size.x / 2.0;
        double ys = y - size.y * 1.15 / 3.2;
        double xe = xs + size.x * (str.size() + 1);
        double ye = ys + size.y * 1.15;

        drawBlend(Point(xs, ys), Point(xe, ye));
    }

    renderText(x, y, z, str);
}

void SceneView::paintPostprocessorSelectedVolume()
{
    logMessage("SceneView::paintPostprocessorSelectedVolume()");

    if (!m_scene->sceneSolution()->isMeshed()) return;

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF(),
              0.5);

    // triangles
    for (int i = 0; i < m_scene->sceneSolution()->meshInitial()->get_num_active_elements(); i++)
    {
        Hermes::Hermes2D::Element *element = m_scene->sceneSolution()->meshInitial()->get_element(i);
        if (m_scene->labels->at(atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(element->marker).marker.c_str()))->isSelected)
        {
            if (element->is_triangle())
            {
                glBegin(GL_TRIANGLES);
                glVertex2d(element->vn[0]->x, element->vn[0]->y);
                glVertex2d(element->vn[1]->x, element->vn[1]->y);
                glVertex2d(element->vn[2]->x, element->vn[2]->y);
                glEnd();
            }
            else
            {
                glBegin(GL_QUADS);
                glVertex2d(element->vn[0]->x, element->vn[0]->y);
                glVertex2d(element->vn[1]->x, element->vn[1]->y);
                glVertex2d(element->vn[2]->x, element->vn[2]->y);
                glVertex2d(element->vn[3]->x, element->vn[3]->y);
                glEnd();
            }
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // how to get marker from linearizer?
    /*
    logMessage("SceneView::paintPostprocessorSelectedVolume()");

    if (!m_scene->sceneSolution()->isMeshed()) return;

    m_scene->sceneSolution()->linInitialMeshView().lock_data();

    double3* linVert = m_scene->sceneSolution()->linInitialMeshView().get_vertices();
    int3* linTris = m_scene->sceneSolution()->linInitialMeshView().get_triangles();

    // draw initial mesh
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF(),
              0.5);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_scene->sceneSolution()->linSolutionMeshView().get_num_triangles(); i++)
    {
        if (m_scene->labels[element->marker - 1]->isSelected)
        {
            glVertex2d(linVert[linTris[i][0]][0], linVert[linTris[i][0]][1]);
            glVertex2d(linVert[linTris[i][1]][0], linVert[linTris[i][1]][1]);
            glVertex2d(linVert[linTris[i][2]][0], linVert[linTris[i][2]][1]);
        }
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    m_scene->sceneSolution()->linSolutionMeshView().unlock_data();
    */
}

void SceneView::paintPostprocessorSelectedSurface()
{
    logMessage("SceneView::paintPostprocessorSelectedSurface()");

    // edges
    foreach (SceneEdge *edge, m_scene->edges->items()) {
        glColor3d(Util::config()->colorSelected.redF(), Util::config()->colorSelected.greenF(), Util::config()->colorSelected.blueF());
        glLineWidth(3.0);

        if (edge->isSelected)
        {
            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex2d(edge->nodeStart->point.x, edge->nodeStart->point.y);
                glVertex2d(edge->nodeEnd->point.x, edge->nodeEnd->point.y);
                glEnd();
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180 - 180;

                drawArc(center, radius, startAngle, edge->angle, edge->angle/5);
            }
        }
        glLineWidth(1.0);
    }
}

void SceneView::loadBackgroundImage(const QString &fileName, double x, double y, double w, double h)
{
    logMessage("SceneView::loadBackgroundImage()");

    // delete texture
    if (m_backgroundTexture != -1)
    {
        deleteTexture(m_backgroundTexture);
        m_backgroundTexture = -1;
    }

    if (QFile::exists(fileName))
    {
        m_backgroundImage.load(fileName);
        m_backgroundTexture = bindTexture(m_backgroundImage, GL_TEXTURE_2D, GL_RGBA);
        m_backgroundPosition = QRectF(x, y, w, h);
    }
}

ErrorResult SceneView::saveGeometryToFile(const QString &fileName, int format)
{
    logMessage("SceneView::saveImageToFile()");

    // store old value
    SceneMode sceneMode = m_sceneMode;
    m_sceneMode == SceneMode_OperateOnNodes;
    actSceneModeNode->trigger();

    makeCurrent();
    int state = GL2PS_OVERFLOW;
    int buffsize = 0;
    GLint options = GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT;

    FILE *fp = fopen(fileName.toStdString().c_str(), "wb");
    while (state == GL2PS_OVERFLOW)
    {
        buffsize += 1024*1024;
        gl2psBeginPage("Agros2D", "Agros2D - export", NULL, format,
                       GL2PS_BSP_SORT, options,
                       GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, "xxx.pdf");

        glClearColor(Util::config()->colorBackground.redF(),
                     Util::config()->colorBackground.greenF(),
                     Util::config()->colorBackground.blueF(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // paintSolutionMesh();
        // paintInitialMesh();

        paintGeometry();

        state = gl2psEndPage();
    }
    fclose(fp);

    // restore viewport
    m_sceneMode = sceneMode;
    if (m_sceneMode == SceneMode_OperateOnNodes) actSceneModeNode->trigger();
    if (m_sceneMode == SceneMode_OperateOnLabels) actSceneModeEdge->trigger();
    if (m_sceneMode == SceneMode_OperateOnLabels) actSceneModeLabel->trigger();
    if (m_sceneMode == SceneMode_Postprocessor) actSceneModePostprocessor->trigger();

    return ErrorResult();
}

ErrorResult SceneView::saveImageToFile(const QString &fileName, int w, int h)
{
    logMessage("SceneView::saveImageToFile()");

    QPixmap pixmap = renderScenePixmap(w, h);
    if (pixmap.save(fileName, "PNG"))
        resizeGL(width(), height());
    else
        return ErrorResult(ErrorResultType_Critical, tr("Image cannot be saved to the file '%1'.").arg(fileName));

    return ErrorResult();
}

void SceneView::saveImagesForReport(const QString &path, bool showGrid, bool showRulers, bool showAxes, bool showLabel, int w, int h)
{
    assert(0); //TODO
    //    logMessage("SceneView::saveImagesForReport()");

    //    // store sceneview settings
    //    SceneViewSettings sceneViewSettingsCopy = m_sceneViewSettings;
    //    SceneMode sceneModeCopy = m_sceneMode;
    //    double scale2dCopy = m_scale2d;
    //    Point offset2dCopy = m_offset2d;
    //    Point offset3dCopy = m_offset3d;
    //    Point3 rotation3dCopy = m_rotation3d;

    //    bool showRulersCopy = Util::config()->showRulers;
    //    bool showGridCopy = Util::config()->showGrid;
    //    bool showAxesCopy = Util::config()->showAxes;
    //    bool showLabelCopy = Util::config()->showLabel;

    //    Util::config()->showRulers = showRulers;
    //    Util::config()->showGrid = showGrid;
    //    Util::config()->showAxes = showAxes;
    //    Util::config()->showLabel = showLabel;

    //    // remove old files
    //    QFile::remove(path + "/geometry.png");
    //    QFile::remove(path + "/mesh.png");
    //    QFile::remove(path + "/order.png");
    //    QFile::remove(path + "/scalarview.png");

    //    doZoomBestFit();

    //    m_sceneViewSettings.showGeometry = true;
    //    m_sceneViewSettings.showContours = false;
    //    m_sceneViewSettings.showVectors = false;
    //    m_sceneViewSettings.showInitialMesh = false;
    //    m_sceneViewSettings.showSolutionMesh = false;

    //    // geometry
    //    actSceneModeLabel->trigger();
    //    ErrorResult resultGeometry = saveImageToFile(path + "/geometry.png", w, h);
    //    if (resultGeometry.isError())
    //        resultGeometry.showDialog();

    //    // mesh
    //    if (m_scene->sceneSolution()->isMeshed())
    //    {
    //        // show only initial mesh
    //        actSceneModeLabel->trigger();

    //        m_sceneViewSettings.showInitialMesh = true;
    //        m_sceneViewSettings.showSolutionMesh = true;
    //        ErrorResult resultMesh1 = saveImageToFile(path + "/mesh.png", w, h);
    //        if (resultMesh1.isError())
    //            resultMesh1.showDialog();
    //        m_sceneViewSettings.showInitialMesh = false;
    //        m_sceneViewSettings.showSolutionMesh = false;
    //    }

    //    if (m_scene->sceneSolution()->isSolved())
    //    {
    //        // when solved show both meshes
    //        actSceneModePostprocessor->trigger();

    //        m_scene->sceneSolution()->processSolutionMesh();
    //        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        updateGL();

    //        m_sceneViewSettings.showInitialMesh = true;
    //        m_sceneViewSettings.showSolutionMesh = true;
    //        ErrorResult resultMesh2 = saveImageToFile(path + "/mesh.png", w, h);
    //        if (resultMesh2.isError())
    //            resultMesh2.showDialog();
    //        m_sceneViewSettings.showInitialMesh = false;
    //        m_sceneViewSettings.showSolutionMesh = false;

    //        // contours
    //        m_scene->sceneSolution()->processRangeContour();
    //        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        m_sceneViewSettings.showContours = true;
    //        updateGL();
    //        ErrorResult resultContourView = saveImageToFile(path + "/contourview.png", w, h);
    //        if (resultContourView.isError())
    //            resultContourView.showDialog();
    //        m_sceneViewSettings.showContours = false;

    //        // vectors
    //        m_scene->sceneSolution()->processRangeVector();
    //        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        m_sceneViewSettings.showVectors = true;
    //        m_sceneViewSettings.vectorPhysicFieldVariable = m_scene->problemInfo()->module()->view_default_vector_variable->id;
    //        updateGL();
    //        ErrorResult resultVectorView = saveImageToFile(path + "/vectorview.png", w, h);
    //        if (resultVectorView.isError())
    //            resultVectorView.showDialog();
    //        m_sceneViewSettings.showVectors = false;

    //        // order
    //        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_Order;
    //        updateGL();
    //        ErrorResult resultOrder = saveImageToFile(path + "/order.png", w, h);
    //        if (resultOrder.isError())
    //            resultOrder.showDialog();

    //        actSceneModePostprocessor->trigger();

    //        // last step
    //        if (m_scene->problemInfo()->module()->transient_solutions)
    //            m_scene->sceneSolution()->setTimeStep(m_scene->sceneSolution()->timeStepCount() - 1);

    //        // scalar field
    //        m_scene->sceneSolution()->processRangeScalar();
    //        m_sceneViewSettings.scalarRangeAuto = true;
    //        m_sceneViewSettings.scalarPhysicFieldVariable = m_scene->problemInfo()->module()->view_default_scalar_variable->id;
    //        m_sceneViewSettings.scalarPhysicFieldVariableComp = m_scene->problemInfo()->module()->view_default_scalar_variable_comp();
    //        m_sceneViewSettings.vectorPhysicFieldVariable = m_scene->problemInfo()->module()->view_default_vector_variable->id;

    //        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_ScalarView;
    //        updateGL();
    //        ErrorResult resultScalarView = saveImageToFile(path + "/scalarview.png", w, h);
    //        if (resultScalarView.isError())
    //            resultScalarView.showDialog();
    //    }

    //    // restore sceneview settings
    //    m_sceneViewSettings = sceneViewSettingsCopy;
    //    m_sceneMode = sceneModeCopy;
    //    m_scale2d = scale2dCopy;
    //    m_offset2d = offset2dCopy;
    //    m_offset3d = offset3dCopy;
    //    m_rotation3d = rotation3dCopy;

    //    Util::config()->showRulers = showRulersCopy;
    //    Util::config()->showGrid = showGridCopy;
    //    Util::config()->showAxes = showAxesCopy;
    //    Util::config()->showLabel = showLabelCopy;

    //    if (m_sceneMode == SceneMode_OperateOnNodes) actSceneModeNode->trigger();
    //    if (m_sceneMode == SceneMode_OperateOnLabels) actSceneModeEdge->isChecked();
    //    if (m_sceneMode == SceneMode_OperateOnLabels) actSceneModeLabel->isChecked();
    //    if (m_sceneMode == SceneMode_Postprocessor) actSceneModePostprocessor->isChecked();

    //    refresh();
}

void SceneView::setSceneFont()
{
    logMessage("SceneView::setSceneFont()");

    setFont(Util::config()->sceneFont);
}

void SceneView::doSetProjectionXY()
{
    m_rotation3d.x = m_rotation3d.y = m_rotation3d.z = 0.0;
    updateGL();
}

void SceneView::doSetProjectionXZ()
{
    m_rotation3d.y = m_rotation3d.z = 0.0;
    m_rotation3d.x = 90.0;
    updateGL();
}

void SceneView::doSetProjectionYZ()
{
    m_rotation3d.x = m_rotation3d.y = 90.0;
    m_rotation3d.z = 0.0;
    updateGL();
}
