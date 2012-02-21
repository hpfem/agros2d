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

#include "sceneview_common.h"
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
#include "hermes2d/problem.h"

// scene view
static SceneViewCommon *m_sceneView = NULL;

SceneViewCommon *sceneView()
{
    return m_sceneView;
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

SceneViewCommon::SceneViewCommon(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
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
    logMessage("SceneViewCommon::SceneView()");

    m_sceneView = this;
    m_mainWindow = (QMainWindow *) parent;
    m_scene = Util::scene();

    connect(Util::problem(), SIGNAL(timeStepChanged(bool)), this, SLOT(timeStepChanged(bool)));
    // connect(Util::problem(), SIGNAL(solved()), this, SLOT(solved()));
    connect(Util::problem(), SIGNAL(meshed()), this, SLOT(clearGLLists()));
    connect(Util::problem(), SIGNAL(processedRangeContour()), this, SLOT(processedRangeContour()));
    connect(Util::problem(), SIGNAL(processedRangeScalar()), this, SLOT(processedRangeScalar()));
    connect(Util::problem(), SIGNAL(processedRangeVector()), this, SLOT(processedRangeVector()));

    createActions();

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setSceneFont();

    setMinimumSize(400, 400);
}

SceneViewCommon::~SceneViewCommon()
{
    logMessage("SceneViewCommon::~SceneView()");

}

void SceneViewCommon::createActions()
{
    logMessage("SceneViewCommon::createActions()");

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

    // material
    actMaterialGroup = new QActionGroup(this);
    connect(actMaterialGroup, SIGNAL(triggered(QAction *)), this, SLOT(doMaterialGroup(QAction *)));

    // boundary
    actBoundaryGroup = new QActionGroup(this);
    connect(actBoundaryGroup, SIGNAL(triggered(QAction *)), this, SLOT(doBoundaryGroup(QAction *)));
}

void SceneViewCommon::initializeGL()
{
    logMessage("SceneViewCommon::initializeGL()");

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    clearGLLists();
}

void SceneViewCommon::resizeGL(int w, int h)
{
    setupViewport(w, h);
}

void SceneViewCommon::setupViewport(int w, int h)
{
    logMessage("SceneViewCommon::setupViewport()");

    glViewport(0, 0, w, h);
}

QPixmap SceneViewCommon::renderScenePixmap(int w, int h, bool useContext)
{
    logMessage("SceneViewCommon::renderScenePixmap()");

    QPixmap pixmap = renderPixmap(w, h, useContext);

    resizeGL(contextWidth(), contextHeight());

    return pixmap;
}

void SceneViewCommon::loadProjectionViewPort()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-0.5, 0.5, -0.5, 0.5, -10.0, -10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// paint *****************************************************************************************************************************

void SceneViewCommon::paintSceneModeLabel(const QString &text)
{
    logMessage("SceneViewCommon::paintSceneModeLabel()");

    loadProjectionViewPort();

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

// events *****************************************************************************************************************************

void SceneViewCommon::closeEvent(QCloseEvent *event)
{
    logMessage("SceneViewCommon::closeEvent()");

    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneViewCommon::doZoomBestFit()
{
    logMessage("SceneViewCommon::doZoomBestFit()");

    RectPoint rect = m_scene->boundingBox();
    doZoomRegion(rect.start, rect.end);
}

void SceneViewCommon::doZoomIn()
{
    logMessage("SceneViewCommon::doZoomIn()");

    setZoom(1.2);
}

void SceneViewCommon::doZoomOut()
{
    logMessage("SceneViewCommon::doZoomOut()");

    setZoom(-1/1.2);
}

void SceneViewCommon::doShowGrid()
{
    logMessage("SceneViewCommon::doShowGrid()");

    Util::config()->showGrid = !Util::config()->showGrid;
    Util::config()->save();
    doInvalidated();
}

void SceneViewCommon::doSnapToGrid()
{
    logMessage("SceneViewCommon::doSnapToGrid()");

    Util::config()->snapToGrid = !Util::config()->snapToGrid;
    Util::config()->save();
}

void SceneViewCommon::doShowRulers()
{
    logMessage("SceneViewCommon::doShowRulers()");

    Util::config()->showRulers = !Util::config()->showRulers;
    Util::config()->save();
    doInvalidated();
}

void SceneViewCommon::doSetChartLine(const ChartLine &chartLine)
{
    logMessage("SceneViewCommon::doSetChartLine()");

    // set line for chart
    m_chartLine = chartLine;

    updateGL();
}

void SceneViewCommon::doDefaultValues()
{
    logMessage("SceneViewCommon::doDefaultValues()");

    m_isSolutionPrepared = false;

    m_sceneViewSettings.defaultValues();

    deleteTexture(m_backgroundTexture);
    m_backgroundTexture = -1;

    doInvalidated();
    doZoomBestFit();
}

void SceneViewCommon::doInvalidated()
{
    logMessage("SceneViewCommon::doInvalidated()");

    resizeGL(width(), height());

    //TODO m_scene->actDeleteSelected->setEnabled(m_sceneMode != SceneMode_Postprocessor);
    //TODO m_scene->actTransform->setEnabled(m_sceneMode != SceneMode_Postprocessor);

    actSceneShowGrid->setChecked(Util::config()->showGrid);
    actSceneSnapToGrid->setChecked(Util::config()->snapToGrid);
    actSceneShowRulers->setChecked(Util::config()->showRulers);

    setSceneFont();

    emit mousePressed();

    paintGL();
    updateGL();
}

void SceneViewCommon::timeStepChanged(bool showViewProgress)
{
    logMessage("SceneViewCommon::timeStepChanged()");

    if (!Util::problem()->isSolving())
    {
        QTime time;
        time.start();

        Util::scene()->activeSceneSolution()->processView(showViewProgress);
    }

    clearGLLists();
    m_isSolutionPrepared = true;

    refresh();
}

void SceneViewCommon::refresh()
{
    logMessage("SceneViewCommon::refresh()");

    paintGL();
    updateGL();
}

void SceneViewCommon::doMaterialGroup(QAction *action)
{
    logMessage("SceneViewCommon::doMaterialGroup()");

    if (SceneMaterial *material = action->data().value<SceneMaterial *>())
        m_scene->setMaterial(material);
}

void SceneViewCommon::doBoundaryGroup(QAction *action)
{
    logMessage("SceneViewCommon::doBoundaryGroup()");

    if (SceneBoundary *boundary = action->data().value<SceneBoundary *>())
        m_scene->setBoundary(boundary);
}

void SceneViewCommon::drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments) const
{
    logMessage("SceneViewCommon::drawArc()");

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

void SceneViewCommon::drawBlend(Point start, Point end, double red, double green, double blue, double alpha) const
{
    logMessage("SceneViewCommon::drawBlend()");

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

void SceneViewCommon::loadBackgroundImage(const QString &fileName, double x, double y, double w, double h)
{
    logMessage("SceneViewCommon::loadBackgroundImage()");

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

ErrorResult SceneViewCommon::saveImageToFile(const QString &fileName, int w, int h)
{
    logMessage("SceneViewCommon::saveImageToFile()");

    QPixmap pixmap = renderScenePixmap(w, h);
    if (pixmap.save(fileName, "PNG"))
        resizeGL(width(), height());
    else
        return ErrorResult(ErrorResultType_Critical, tr("Image cannot be saved to the file '%1'.").arg(fileName));

    return ErrorResult();
}

void SceneViewCommon::saveImagesForReport(const QString &path, bool showGrid, bool showRulers, bool showAxes, bool showLabel, int w, int h)
{
    assert(0); //TODO
    //    logMessage("SceneViewCommon::saveImagesForReport()");

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
    //    if (m_scene->activeSceneSolution()->isMeshed())
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

    //    if (m_scene->activeSceneSolution()->isSolved())
    //    {
    //        // when solved show both meshes
    //        actSceneModePostprocessor->trigger();

    //        m_scene->activeSceneSolution()->processSolutionMesh();
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
    //        m_scene->activeSceneSolution()->processRangeContour();
    //        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        m_sceneViewSettings.showContours = true;
    //        updateGL();
    //        ErrorResult resultContourView = saveImageToFile(path + "/contourview.png", w, h);
    //        if (resultContourView.isError())
    //            resultContourView.showDialog();
    //        m_sceneViewSettings.showContours = false;

    //        // vectors
    //        m_scene->activeSceneSolution()->processRangeVector();
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
    //            m_scene->activeSceneSolution()->setTimeStep(m_scene->activeSceneSolution()->timeStepCount() - 1);

    //        // scalar field
    //        m_scene->activeSceneSolution()->processRangeScalar();
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

void SceneViewCommon::setSceneFont()
{
    logMessage("SceneViewCommon::setSceneFont()");

    setFont(Util::config()->sceneFont);
}

// **************************************************************************************************

void SceneViewCommon::clearGLLists()
{
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

const double* SceneViewCommon::paletteColor(double x) const
{
    logMessage("SceneViewCommon::paletteColor()");

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

const double* SceneViewCommon::paletteColorOrder(int n) const
{
    logMessage("SceneViewCommon::paletteColorOrder()");

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

void SceneViewCommon::paletteCreate()
{
    logMessage("SceneViewCommon::paletteCreate()");

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

void SceneViewCommon::paletteFilter()
{
    logMessage("SceneViewCommon::paletteFilter()");

    int palFilter = Util::config()->paletteFilter ? GL_LINEAR : GL_NEAREST;
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, palFilter);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, palFilter);
    paletteUpdateTexAdjust();
}

void SceneViewCommon::paletteUpdateTexAdjust()
{
    logMessage("SceneViewCommon::paletteUpdateTexAdjust()");

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

void SceneViewCommon::processedSolutionMesh()
{
    logMessage("SceneViewCommon::processedSolutionMesh()");
}

void SceneViewCommon::processedRangeContour()
{
    logMessage("SceneViewCommon::processedRangeContour()");
}

void SceneViewCommon::processedRangeScalar()
{
    logMessage("SceneViewCommon::processedRangeScalar()");

    paletteFilter();
    paletteUpdateTexAdjust();
    paletteCreate();

    if (m_sceneViewSettings.scalarRangeAuto)
    {
        m_sceneViewSettings.scalarRangeMin = m_scene->activeSceneSolution()->linScalarView().get_min_value();
        m_sceneViewSettings.scalarRangeMax = m_scene->activeSceneSolution()->linScalarView().get_max_value();
        cout << "setting limits (" << m_sceneViewSettings.scalarRangeMin << ", " << m_sceneViewSettings.scalarRangeMax << ")" << endl;
    }
}

void SceneViewCommon::processedRangeVector()
{
    logMessage("SceneViewCommon::processedRangeVector()");
}

void SceneViewCommon::paintScalarFieldColorBar(double min, double max)
{
    logMessage("SceneViewCommon::paintScalarFieldColorBar()");

    if (!Util::config()->showScalarScale) return;

    loadProjectionViewPort();

    glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
    glTranslated(-contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    // dimensions
    int textWidth = fontMetrics().width(QString::number(-1.0, '+e', Util::config()->scalarDecimalPlace)) + 3;
    int textHeight = fontMetrics().height();
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // contextHeight() - 20.0
    Point scaleBorder = Point(10.0, 10.0);
    double scaleLeft = (contextWidth() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBegin(GL_QUADS);
    if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) > EPS_ZERO)
        glTexCoord1d(m_texScale + m_texShift);
    else
        glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    glDisable(GL_TEXTURE_1D);

    // ticks
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks+1; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = 0.0;
        if (!Util::config()->scalarRangeLog)
            value = min + (double) (i-1) / (numTicks-1) * (max - min);
        else
            value = min + (double) pow(Util::config()->scalarRangeBase, ((i-1) / (numTicks-1)))/Util::config()->scalarRangeBase * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        renderText(scaleLeft + 33.0 + ((value >= 0.0) ? fontMetrics().width("-") : 0.0),
                   scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                   0.0,
                   QString::number(value, '+e', Util::config()->scalarDecimalPlace));
    }

    // variable
    QString str = QString("%1 (%2)").
            arg(QString::fromStdString(m_sceneViewSettings.scalarPhysicFieldVariable != "" ? Util::scene()->activeViewField()->module()->get_variable(m_sceneViewSettings.scalarPhysicFieldVariable)->shortname : "")).
            arg(QString::fromStdString(m_sceneViewSettings.scalarPhysicFieldVariable != "" ? Util::scene()->activeViewField()->module()->get_variable(m_sceneViewSettings.scalarPhysicFieldVariable)->unit : ""));

    renderText(scaleLeft + scaleSize.x / 2.0 - fontMetrics().width(str) / 2.0,
               scaleBorder.y + scaleSize.y - 20.0,
               0.0,
               str);
    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();
}
