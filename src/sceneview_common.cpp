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

#include "util/global.h"
#include "logview.h"

#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "hermes2d/module.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

SceneViewCommon::SceneViewCommon(QWidget *parent)
    : QGLWidget(parent),
      actSceneZoomRegion(NULL),
      m_fontPost(NULL),
      m_fontRulers(NULL),
      m_textureLabelRulers(-1),
      m_textureLabelPost(-1)
{
    m_mainWindow = (QMainWindow *) parent;

    createFontTexture();
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
    glDisable(GL_MULTISAMPLE);
    glEnable(GL_NORMALIZE);

    createFontTexture();
}

void SceneViewCommon::createFontTexture()
{
    // rulers font
    if (glIsTexture(m_textureLabelRulers))
        glDeleteTextures(1, &m_textureLabelRulers);
    m_fontRulers = textureFontFromStringKey(Agros2D::problem()->configView()->rulersFont);
    glGenTextures(1, &m_textureLabelRulers);
    initFont(m_textureLabelRulers, m_fontRulers);
    // qDebug() << "textureLabelRulers: " << m_textureLabelRulers;

    // rulers font
    if (glIsTexture(m_textureLabelPost))
        glDeleteTextures(1, &m_textureLabelPost);
    m_fontPost = textureFontFromStringKey(Agros2D::problem()->configView()->postFont);
    glGenTextures(1, &m_textureLabelPost);
    initFont(m_textureLabelPost, m_fontPost);
    // qDebug() << "textureLabelPost: " << m_textureLabelPost;
}

void SceneViewCommon::resizeGL(int w, int h)
{
    setupViewport(w, h);
}

void SceneViewCommon::setupViewport(int w, int h)
{
    glViewport(0, 0, w, h);
}

void SceneViewCommon::printRulersAt(int penX, int penY, const QString &text)
{
    glBindTexture(GL_TEXTURE_2D, m_textureLabelRulers);
    printAt(penX, penY, text, m_fontRulers);
}

void SceneViewCommon::printPostAt(int penX, int penY, const QString &text)
{    
    glBindTexture(GL_TEXTURE_2D, m_textureLabelPost);
    printAt(penX, penY, text, m_fontPost);
}

QPixmap SceneViewCommon::renderScenePixmap(int w, int h, bool useContext)
{
    /*
    QPixmap pixmap = renderPixmap((w == 0) ? width() : w,
                                  (h == 0) ? height() : h,
                                  useContext);
    resizeGL(width(), height());

    return pixmap;
    */

    return QPixmap::fromImage(grabFrameBuffer(false));
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
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

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

void SceneViewCommon::initFont(GLuint textureID, const TextureFont *fnt)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, fnt->tex_width, fnt->tex_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, fnt->tex_data);
}

// events *****************************************************************************************************************************

void SceneViewCommon::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneViewCommon::doZoomBestFit()
{
    RectPoint rect = Agros2D::scene()->boundingBox();

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

void SceneViewCommon::clear()
{
    doZoomBestFit();
}

void SceneViewCommon::refresh()
{
    createFontTexture();

    paintGL();
    updateGL();
}

void SceneViewCommon::doMaterialGroup(QAction *action)
{
    if (SceneMaterial *material = action->data().value<SceneMaterial *>())
        Agros2D::scene()->setMaterial(material);
}

void SceneViewCommon::doBoundaryGroup(QAction *action)
{
    if (SceneBoundary *boundary = action->data().value<SceneBoundary *>())
        Agros2D::scene()->setBoundary(boundary);
}

void SceneViewCommon::drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments) const
{
    if (segments == -1)
    {
        if (arcAngle < 10)
            segments = 10;
        else
            segments = arcAngle / 3;
    }

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

void SceneViewCommon::saveImageToFile(const QString &fileName, int w, int h)
{
    QPixmap pixmap = renderScenePixmap(w, h);
    if (!pixmap.save(fileName, "PNG"))
        Agros2D::log()->printError(tr("SceneView"), tr("Image cannot be saved to the file '%1'.").arg(fileName));
}

void SceneViewCommon::saveImagesForReport(const QString &path, bool showGrid, bool showRulers, bool showAxes, bool showLabel, int w, int h)
{
    assert(0); //TODO
    //    // store sceneview settings
    //    SceneViewSettings sceneViewSettingsCopy = Agros2D::scene()ViewSettings;
    //    SceneMode sceneModeCopy = Agros2D::scene()Mode;
    //    double scale2dCopy = m_scale2d;
    //    Point offset2dCopy = m_offset2d;
    //    Point offset3dCopy = m_offset3d;
    //    Point3 rotation3dCopy = m_rotation3d;

    //    bool showRulersCopy = Agros2D::problem()->configView()->showRulers;
    //    bool showGridCopy = Agros2D::problem()->configView()->showGrid;
    //    bool showAxesCopy = Agros2D::problem()->configView()->showAxes;
    //    bool showLabelCopy = Agros2D::problem()->configView()->showLabel;

    //    Agros2D::problem()->configView()->showRulers = showRulers;
    //    Agros2D::problem()->configView()->showGrid = showGrid;
    //    Agros2D::problem()->configView()->showAxes = showAxes;
    //    Agros2D::problem()->configView()->showLabel = showLabel;

    //    // remove old files
    //    QFile::remove(path + "/geometry.png");
    //    QFile::remove(path + "/mesh.png");
    //    QFile::remove(path + "/order.png");
    //    QFile::remove(path + "/scalarview.png");

    //    doZoomBestFit();

    //    Agros2D::scene()ViewSettings.showGeometry = true;
    //    Agros2D::scene()ViewSettings.showContours = false;
    //    Agros2D::scene()ViewSettings.showVectors = false;
    //    Agros2D::scene()ViewSettings.showInitialMesh = false;
    //    Agros2D::scene()ViewSettings.showSolutionMesh = false;

    //    // geometry
    //    actSceneModeLabel->trigger();
    //    ErrorResult resultGeometry = saveImageToFile(path + "/geometry.png", w, h);
    //    if (resultGeometry.isError())
    //        resultGeometry.showDialog();

    //    // mesh
    //    if (Agros2D::scene()->activeSceneSolution()->isMeshed())
    //    {
    //        // show only initial mesh
    //        actSceneModeLabel->trigger();

    //        Agros2D::scene()ViewSettings.showInitialMesh = true;
    //        Agros2D::scene()ViewSettings.showSolutionMesh = true;
    //        ErrorResult resultMesh1 = saveImageToFile(path + "/mesh.png", w, h);
    //        if (resultMesh1.isError())
    //            resultMesh1.showDialog();
    //        Agros2D::scene()ViewSettings.showInitialMesh = false;
    //        Agros2D::scene()ViewSettings.showSolutionMesh = false;
    //    }

    //    if (Agros2D::scene()->activeSceneSolution()->isSolved())
    //    {
    //        // when solved show both meshes
    //        actSceneModePostprocessor->trigger();

    //        Agros2D::scene()->activeSceneSolution()->processSolutionMesh();
    //        Agros2D::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        updateGL();

    //        Agros2D::scene()ViewSettings.showInitialMesh = true;
    //        Agros2D::scene()ViewSettings.showSolutionMesh = true;
    //        ErrorResult resultMesh2 = saveImageToFile(path + "/mesh.png", w, h);
    //        if (resultMesh2.isError())
    //            resultMesh2.showDialog();
    //        Agros2D::scene()ViewSettings.showInitialMesh = false;
    //        Agros2D::scene()ViewSettings.showSolutionMesh = false;

    //        // contours
    //        Agros2D::scene()->activeSceneSolution()->processRangeContour();
    //        Agros2D::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        Agros2D::scene()ViewSettings.showContours = true;
    //        updateGL();
    //        ErrorResult resultContourView = saveImageToFile(path + "/contourview.png", w, h);
    //        if (resultContourView.isError())
    //            resultContourView.showDialog();
    //        Agros2D::scene()ViewSettings.showContours = false;

    //        // vectors
    //        Agros2D::scene()->activeSceneSolution()->processRangeVector();
    //        Agros2D::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
    //        Agros2D::scene()ViewSettings.showVectors = true;
    //        Agros2D::scene()ViewSettings.vectorPhysicFieldVariable = Agros2D::problem()->config()->module()->view_default_vector_variable->id;
    //        updateGL();
    //        ErrorResult resultVectorView = saveImageToFile(path + "/vectorview.png", w, h);
    //        if (resultVectorView.isError())
    //            resultVectorView.showDialog();
    //        Agros2D::scene()ViewSettings.showVectors = false;

    //        // order
    //        Agros2D::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_Order;
    //        updateGL();
    //        ErrorResult resultOrder = saveImageToFile(path + "/order.png", w, h);
    //        if (resultOrder.isError())
    //            resultOrder.showDialog();

    //        actSceneModePostprocessor->trigger();

    //        // last step
    //        if (Agros2D::problem()->config()->module()->transient_solutions)
    //            Agros2D::scene()->activeSceneSolution()->setTimeStep(Agros2D::scene()->activeSceneSolution()->timeStepCount() - 1);

    //        // scalar field
    //        Agros2D::scene()->activeSceneSolution()->processRangeScalar();
    //        Agros2D::scene()ViewSettings.scalarRangeAuto = true;
    //        Agros2D::scene()ViewSettings.scalarPhysicFieldVariable = Agros2D::problem()->config()->module()->view_default_scalar_variable->id;
    //        Agros2D::scene()ViewSettings.scalarPhysicFieldVariableComp = Agros2D::problem()->config()->module()->view_default_scalar_variable_comp();
    //        Agros2D::scene()ViewSettings.vectorPhysicFieldVariable = Agros2D::problem()->config()->module()->view_default_vector_variable->id;

    //        Agros2D::scene()ViewSettings.postprocessorShow = SceneViewPostprocessorShow_ScalarView;
    //        updateGL();
    //        ErrorResult resultScalarView = saveImageToFile(path + "/scalarview.png", w, h);
    //        if (resultScalarView.isError())
    //            resultScalarView.showDialog();
    //    }

    //    // restore sceneview settings
    //    Agros2D::scene()ViewSettings = sceneViewSettingsCopy;
    //    Agros2D::scene()Mode = sceneModeCopy;
    //    m_scale2d = scale2dCopy;
    //    m_offset2d = offset2dCopy;
    //    m_offset3d = offset3dCopy;
    //    m_rotation3d = rotation3dCopy;

    //    Agros2D::problem()->configView()->showRulers = showRulersCopy;
    //    Agros2D::problem()->configView()->showGrid = showGridCopy;
    //    Agros2D::problem()->configView()->showAxes = showAxesCopy;
    //    Agros2D::problem()->configView()->showLabel = showLabelCopy;

    //    if (Agros2D::scene()Mode == SceneMode_OperateOnNodes) actSceneModeNode->trigger();
    //    if (Agros2D::scene()Mode == SceneMode_OperateOnLabels) actSceneModeEdge->isChecked();
    //    if (Agros2D::scene()Mode == SceneMode_OperateOnLabels) actSceneModeLabel->isChecked();
    //    if (Agros2D::scene()Mode == SceneMode_Postprocessor) actSceneModePostprocessor->isChecked();

    //    refresh();
}

