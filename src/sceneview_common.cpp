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

SceneViewWidget::SceneViewWidget(SceneViewCommon *widget, QWidget *parent) : QWidget(parent)
{
    // label
    sceneViewLabelPixmap = new QLabel();
    sceneViewLabelLeft = new QLabel();
    sceneViewLabelCenter = new QLabel();
    sceneViewLabelRight = new QLabel();

    QHBoxLayout *sceneViewLabelLayout = new QHBoxLayout();
    sceneViewLabelLayout->addWidget(sceneViewLabelPixmap);
    sceneViewLabelLayout->addWidget(sceneViewLabelLeft);
    sceneViewLabelLayout->addStretch(0.5);
    sceneViewLabelLayout->addWidget(sceneViewLabelCenter);
    sceneViewLabelLayout->addStretch(0.5);
    sceneViewLabelLayout->addWidget(sceneViewLabelRight);

    // view
    QVBoxLayout *sceneViewLayout = new QVBoxLayout();
    sceneViewLayout->addLayout(sceneViewLabelLayout);
    sceneViewLayout->addWidget(widget);
    sceneViewLayout->setStretch(1, 1);

    setLayout(sceneViewLayout);

    iconLeft(widget->iconView());
    labelLeft(widget->labelView());

    connect(widget, SIGNAL(labelCenter(QString)), this, SLOT(labelCenter(QString)));
    connect(widget, SIGNAL(labelRight(QString)), this, SLOT(labelRight(QString)));
}

SceneViewWidget::~SceneViewWidget()
{
}

void SceneViewWidget::labelLeft(const QString &left)
{
    sceneViewLabelLeft->setText(left);
}

void SceneViewWidget::labelCenter(const QString &center)
{
    sceneViewLabelCenter->setText(center);
}

void SceneViewWidget::labelRight(const QString &right)
{
    sceneViewLabelRight->setText(right);
}

void SceneViewWidget::iconLeft(const QIcon &left)
{
    QPixmap pixmap = left.pixmap(QSize(16, 16));
    sceneViewLabelPixmap->setPixmap(pixmap);
}


// **********************************************************************************************

SceneViewCommon::SceneViewCommon(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    logMessage("SceneViewCommon::SceneView()");

    m_mainWindow = (QMainWindow *) parent;

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
}

void SceneViewCommon::resizeGL(int w, int h)
{
    setupViewport(w, h);
}

void SceneViewCommon::setupViewport(int w, int h)
{
    glViewport(0, 0, w, h);
}

QPixmap SceneViewCommon::renderScenePixmap(int w, int h, bool useContext)
{
    QPixmap pixmap = renderPixmap(w, h, useContext);

    resizeGL(width(), height());

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

    RectPoint rect = Util::scene()->boundingBox();
    doZoomRegion(rect.start, rect.end);
    doZoomRegion(rect.start, rect.end); //TODO - FIX twice run is needed
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

void SceneViewCommon::clear()
{
    doInvalidated();
    doZoomBestFit();
}

void SceneViewCommon::doInvalidated()
{
    resize(((QWidget *) parent())->size());

    emit mousePressed();

    paintGL();
    updateGL();
}

void SceneViewCommon::refresh()
{
    paintGL();
    updateGL();
}

void SceneViewCommon::doMaterialGroup(QAction *action)
{
    if (SceneMaterial *material = action->data().value<SceneMaterial *>())
        Util::scene()->setMaterial(material);
}

void SceneViewCommon::doBoundaryGroup(QAction *action)
{
    logMessage("SceneViewCommon::doBoundaryGroup()");

    if (SceneBoundary *boundary = action->data().value<SceneBoundary *>())
        Util::scene()->setBoundary(boundary);
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
    //    SceneViewSettings sceneViewSettingsCopy = Util::scene()ViewSettings;
    //    SceneMode sceneModeCopy = Util::scene()Mode;
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

    //    Util::scene()ViewSettings.showGeometry = true;
    //    Util::scene()ViewSettings.showContours = false;
    //    Util::scene()ViewSettings.showVectors = false;
    //    Util::scene()ViewSettings.showInitialMesh = false;
    //    Util::scene()ViewSettings.showSolutionMesh = false;

    //    // geometry
    //    actSceneModeLabel->trigger();
    //    ErrorResult resultGeometry = saveImageToFile(path + "/geometry.png", w, h);
    //    if (resultGeometry.isError())
    //        resultGeometry.showDialog();

    //    // mesh
    //    if (Util::scene()->activeSceneSolution()->isMeshed())
    //    {
    //        // show only initial mesh
    //        actSceneModeLabel->trigger();

    //        Util::scene()ViewSettings.showInitialMesh = true;
    //        Util::scene()ViewSettings.showSolutionMesh = true;
    //        ErrorResult resultMesh1 = saveImageToFile(path + "/mesh.png", w, h);
    //        if (resultMesh1.isError())
    //            resultMesh1.showDialog();
    //        Util::scene()ViewSettings.showInitialMesh = false;
    //        Util::scene()ViewSettings.showSolutionMesh = false;
    //    }

    //    if (Util::scene()->activeSceneSolution()->isSolved())
    //    {
    //        // when solved show both meshes
    //        actSceneModePostprocessor->trigger();

    //        Util::scene()->activeSceneSolution()->processSolutionMesh();
    //        Util::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        updateGL();

    //        Util::scene()ViewSettings.showInitialMesh = true;
    //        Util::scene()ViewSettings.showSolutionMesh = true;
    //        ErrorResult resultMesh2 = saveImageToFile(path + "/mesh.png", w, h);
    //        if (resultMesh2.isError())
    //            resultMesh2.showDialog();
    //        Util::scene()ViewSettings.showInitialMesh = false;
    //        Util::scene()ViewSettings.showSolutionMesh = false;

    //        // contours
    //        Util::scene()->activeSceneSolution()->processRangeContour();
    //        Util::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        Util::scene()ViewSettings.showContours = true;
    //        updateGL();
    //        ErrorResult resultContourView = saveImageToFile(path + "/contourview.png", w, h);
    //        if (resultContourView.isError())
    //            resultContourView.showDialog();
    //        Util::scene()ViewSettings.showContours = false;

    //        // vectors
    //        Util::scene()->activeSceneSolution()->processRangeVector();
    //        Util::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        Util::scene()ViewSettings.showVectors = true;
    //        Util::scene()ViewSettings.vectorPhysicFieldVariable = Util::scene()->problemInfo()->module()->view_default_vector_variable->id;
    //        updateGL();
    //        ErrorResult resultVectorView = saveImageToFile(path + "/vectorview.png", w, h);
    //        if (resultVectorView.isError())
    //            resultVectorView.showDialog();
    //        Util::scene()ViewSettings.showVectors = false;

    //        // order
    //        Util::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_Order;
    //        updateGL();
    //        ErrorResult resultOrder = saveImageToFile(path + "/order.png", w, h);
    //        if (resultOrder.isError())
    //            resultOrder.showDialog();

    //        actSceneModePostprocessor->trigger();

    //        // last step
    //        if (Util::scene()->problemInfo()->module()->transient_solutions)
    //            Util::scene()->activeSceneSolution()->setTimeStep(Util::scene()->activeSceneSolution()->timeStepCount() - 1);

    //        // scalar field
    //        Util::scene()->activeSceneSolution()->processRangeScalar();
    //        Util::scene()ViewSettings.scalarRangeAuto = true;
    //        Util::scene()ViewSettings.scalarPhysicFieldVariable = Util::scene()->problemInfo()->module()->view_default_scalar_variable->id;
    //        Util::scene()ViewSettings.scalarPhysicFieldVariableComp = Util::scene()->problemInfo()->module()->view_default_scalar_variable_comp();
    //        Util::scene()ViewSettings.vectorPhysicFieldVariable = Util::scene()->problemInfo()->module()->view_default_vector_variable->id;

    //        Util::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_ScalarView;
    //        updateGL();
    //        ErrorResult resultScalarView = saveImageToFile(path + "/scalarview.png", w, h);
    //        if (resultScalarView.isError())
    //            resultScalarView.showDialog();
    //    }

    //    // restore sceneview settings
    //    Util::scene()ViewSettings = sceneViewSettingsCopy;
    //    Util::scene()Mode = sceneModeCopy;
    //    m_scale2d = scale2dCopy;
    //    m_offset2d = offset2dCopy;
    //    m_offset3d = offset3dCopy;
    //    m_rotation3d = rotation3dCopy;

    //    Util::config()->showRulers = showRulersCopy;
    //    Util::config()->showGrid = showGridCopy;
    //    Util::config()->showAxes = showAxesCopy;
    //    Util::config()->showLabel = showLabelCopy;

    //    if (Util::scene()Mode == SceneMode_OperateOnNodes) actSceneModeNode->trigger();
    //    if (Util::scene()Mode == SceneMode_OperateOnLabels) actSceneModeEdge->isChecked();
    //    if (Util::scene()Mode == SceneMode_OperateOnLabels) actSceneModeLabel->isChecked();
    //    if (Util::scene()Mode == SceneMode_Postprocessor) actSceneModePostprocessor->isChecked();

    //    refresh();
}

void SceneViewCommon::setSceneFont()
{
    logMessage("SceneViewCommon::setSceneFont()");

    setFont(Util::config()->sceneFont);
}
