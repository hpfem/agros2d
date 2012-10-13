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
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

SceneViewCommon::SceneViewCommon(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
      m_textureLabelRulers(-1),
      m_textureLabelPost(-1)
{
    m_mainWindow = (QMainWindow *) parent;

    createActions();

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setMinimumSize(400, 400);
}

SceneViewCommon::~SceneViewCommon()
{
}

void SceneViewCommon::createActions()
{
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
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

#ifdef Q_WS_X11
    glDisable(GL_MULTISAMPLE);
#endif
#ifdef Q_WS_WIN
#endif
#ifdef Q_WS_MAC
    glDisable(GL_MULTISAMPLE);
#endif
}

void SceneViewCommon::resizeGL(int w, int h)
{
    if (m_textureLabelRulers == -1)
        glDeleteTextures(1, &m_textureLabelRulers);
    glGenTextures(1, &m_textureLabelRulers);
    initFont(m_textureLabelRulers, textureFontFromStringKey(Util::config()->rulersFont));

    if (m_textureLabelPost == -1)
        glDeleteTextures(1, &m_textureLabelPost);
    glGenTextures(1, &m_textureLabelPost);
    initFont(m_textureLabelPost, textureFontFromStringKey(Util::config()->postFont));

    setupViewport(w, h);
}

void SceneViewCommon::setupViewport(int w, int h)
{
    glViewport(0, 0, w, h);
}

void SceneViewCommon::printRulersAt(int penX, int penY, const QString &text)
{
    // rulers font
    const TextureFont *fnt = textureFontFromStringKey(Util::config()->rulersFont);

    glBindTexture(GL_TEXTURE_2D, m_textureLabelRulers);
    printAt(penX, penY, text, fnt);
}

void SceneViewCommon::printPostAt(int penX, int penY, const QString &text)
{
    // post font
    const TextureFont *fnt = textureFontFromStringKey(Util::config()->postFont);

    glBindTexture(GL_TEXTURE_2D, m_textureLabelPost);
    printAt(penX, penY, text, fnt);
}

QPixmap SceneViewCommon::renderScenePixmap(int w, int h, bool useContext)
{
    QPixmap pixmap = renderPixmap((w == 0) ? width() : w,
                                  (h == 0) ? height() : h,
                                  useContext);
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

void SceneViewCommon::printAt(int penX, int penY, const QString &text, const TextureFont *fnt)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < text.length(); ++i)
    {
        const TextureGlyph *glyph = NULL;
        for (int j = 0; j < fnt->glyphs_count; ++j)
        {
            if (fnt->glyphs[j].charcode == text.at(i) )
            {
                glyph = &fnt->glyphs[j];
                break;
            }
        }
        if (!glyph)
        {
            continue;
        }

        int x = penX + glyph->offset_x;
        int y = penY + glyph->offset_y;
        int w  = glyph->width;
        int h  = glyph->height;

        glTexCoord2f(glyph->s0, glyph->t0); glVertex2i(x,   y );
        glTexCoord2f(glyph->s0, glyph->t1); glVertex2i(x,   y-h);
        glTexCoord2f(glyph->s1, glyph->t1); glVertex2i(x+w, y-h);
        glTexCoord2f(glyph->s0, glyph->t0); glVertex2i(x,   y  );
        glTexCoord2f(glyph->s1, glyph->t1); glVertex2i(x+w, y-h);
        glTexCoord2f(glyph->s1, glyph->t0); glVertex2i(x+w, y  );

        penX += glyph->advance_x;
        penY += glyph->advance_y;

    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void SceneViewCommon::initFont(int textureID, const TextureFont *fnt)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, fnt->tex_width, fnt->tex_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fnt->tex_data);
    // glBindTexture(GL_TEXTURE_2D, texid);
}

// events *****************************************************************************************************************************

void SceneViewCommon::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneViewCommon::doZoomBestFit()
{
    RectPoint rect = Util::scene()->boundingBox();

    doZoomRegion(rect.start, rect.end);
}

void SceneViewCommon::doZoomIn()
{
    setZoom(1.2);
}

void SceneViewCommon::doZoomOut()
{
    setZoom(-1/1.2);
}

void SceneViewCommon::doShowGrid()
{
    Util::config()->showGrid = !Util::config()->showGrid;
    Util::config()->save();
    refresh();
}

void SceneViewCommon::doSnapToGrid()
{
    Util::config()->snapToGrid = !Util::config()->snapToGrid;
    Util::config()->save();
}

void SceneViewCommon::doShowRulers()
{
    Util::config()->showRulers = !Util::config()->showRulers;
    Util::config()->save();
    refresh();
}

void SceneViewCommon::clear()
{
    refresh();
    doZoomBestFit();
}

void SceneViewCommon::refresh()
{
    emit mousePressed();

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
    if (SceneBoundary *boundary = action->data().value<SceneBoundary *>())
        Util::scene()->setBoundary(boundary);
}

void SceneViewCommon::drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments) const
{
    if (segments < 2) segments = 2;
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
    QPixmap pixmap = renderScenePixmap(w, h);
    if (!pixmap.save(fileName, "PNG"))
        return ErrorResult(ErrorResultType_Critical, tr("Image cannot be saved to the file '%1'.").arg(fileName));

    return ErrorResult();
}

void SceneViewCommon::saveImagesForReport(const QString &path, bool showGrid, bool showRulers, bool showAxes, bool showLabel, int w, int h)
{
    assert(0); //TODO
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
    //        Util::scene()ViewSettings.vectorPhysicFieldVariable = Util::problem()->config()->module()->view_default_vector_variable->id;
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
    //        if (Util::problem()->config()->module()->transient_solutions)
    //            Util::scene()->activeSceneSolution()->setTimeStep(Util::scene()->activeSceneSolution()->timeStepCount() - 1);

    //        // scalar field
    //        Util::scene()->activeSceneSolution()->processRangeScalar();
    //        Util::scene()ViewSettings.scalarRangeAuto = true;
    //        Util::scene()ViewSettings.scalarPhysicFieldVariable = Util::problem()->config()->module()->view_default_scalar_variable->id;
    //        Util::scene()ViewSettings.scalarPhysicFieldVariableComp = Util::problem()->config()->module()->view_default_scalar_variable_comp();
    //        Util::scene()ViewSettings.vectorPhysicFieldVariable = Util::problem()->config()->module()->view_default_vector_variable->id;

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

