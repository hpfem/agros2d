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

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype/stb_truetype.h"
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

static const int TEXTURE_SIZE = 512;

SceneViewCommon::SceneViewCommon(QWidget *parent)
    : QGLWidget(parent),
      actSceneZoomRegion(NULL),
      m_textureLabelRulers(0),
      m_textureLabelPost(0),
      m_textureLabelRulersName(""),
      m_textureLabelPostName(""),
      m_textureLabelRulersSize(0),
      m_textureLabelPostSize(0)
{
    m_mainWindow = (QMainWindow *) parent;

    createFontTexture();
    createActions();

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setMinimumSize(200, 200);
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
    // TODO: doesn't work on windows
    // if (m_textureLabelRulersName != Agros2D::problem()->configView()->rulersFont || m_textureLabelRulersSize != Agros2D::problem()->configView()->rulersFontSize)
    {
        makeCurrent();
        if (glIsTexture(m_textureLabelRulers))
            glDeleteTextures(1, &m_textureLabelRulers);
        glGenTextures(1, &m_textureLabelRulers);
        m_textureLabelRulersName = Agros2D::problem()->configView()->rulersFont;
        m_textureLabelRulersSize = Agros2D::problem()->configView()->rulersFontSize;
        initFont(m_textureLabelRulers, m_charDataRulers, Agros2D::problem()->configView()->rulersFont, Agros2D::problem()->configView()->rulersFontSize);
    }
    // qDebug() << "textureLabelRulers: " << m_textureLabelRulers;

    // rulers font
    // TODO: doesn't work on windows
    // if (m_textureLabelPostName != Agros2D::problem()->configView()->postFont || m_textureLabelPostSize != Agros2D::problem()->configView()->postFontSize)
    {
        makeCurrent();
        if (glIsTexture(m_textureLabelPost))
            glDeleteTextures(1, &m_textureLabelPost);
        glGenTextures(1, &m_textureLabelPost);
        m_textureLabelPostName = Agros2D::problem()->configView()->rulersFont;
        m_textureLabelPostSize = Agros2D::problem()->configView()->postFontSize;
        initFont(m_textureLabelPost, m_charDataPost, Agros2D::problem()->configView()->postFont, Agros2D::problem()->configView()->postFontSize);
    }
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
    glColor3d(Agros2D::problem()->configView()->colorCross.darker().redF(),
              Agros2D::problem()->configView()->colorCross.darker().greenF(),
              Agros2D::problem()->configView()->colorCross.darker().blueF());

    glBindTexture(GL_TEXTURE_2D, m_textureLabelRulers);
    printAt(penX, penY, text, m_charDataRulers);
}

void SceneViewCommon::printPostAt(int penX, int penY, const QString &text)
{
    glColor3d(0.0, 0.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, m_textureLabelPost);
    printAt(penX, penY, text, m_charDataPost);
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

void getBakedQuad(stbtt_bakedchar *chardata, int pw, int ph, int char_index,
                  float *xpos, float *ypos, stbtt_aligned_quad *q)
{

}

void SceneViewCommon::printAt(int penX, int penY, const QString &text, stbtt_bakedchar *fnt)
{   
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double xpos = 0.0;

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < text.length(); ++i)
    {
        ushort c = text.at(i).unicode();

        if (c >= 32 && c < 128)
        {
            stbtt_aligned_quad q;

            stbtt_bakedchar *b = fnt + c - 32;

            int round_x = std::floor(penX + b->xoff);
            int round_y = std::floor(penY - b->yoff);

            q.x0 = (float) round_x + xpos;
            q.y0 = (float) round_y;
            q.x1 = (float) round_x + b->x1 - b->x0 + xpos;
            q.y1 = (float) round_y - b->y1 + b->y0;

            q.s0 = b->x0 / (float) TEXTURE_SIZE;
            q.t0 = b->y0 / (float) TEXTURE_SIZE;
            q.s1 = b->x1 / (float) TEXTURE_SIZE;
            q.t1 = b->y1 / (float) TEXTURE_SIZE;

            xpos += b->xadvance;

            glTexCoord2f(q.s0, q.t0);
            glVertex2i(q.x0, q.y0);
            glTexCoord2f(q.s1, q.t1);
            glVertex2i(q.x1, q.y1);
            glTexCoord2f(q.s1, q.t0);
            glVertex2i(q.x1, q.y0);

            glTexCoord2f(q.s0, q.t0);
            glVertex2i(q.x0, q.y0);
            glTexCoord2f(q.s0, q.t1);
            glVertex2i(q.x0, q.y1);
            glTexCoord2f(q.s1, q.t1);
            glVertex2i(q.x1, q.y1);
        }
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void SceneViewCommon::initFont(GLuint textureID, stbtt_bakedchar *fnt, const QString fontName, int pointSize)
{
    // load font
    QString fntx = QFileInfo(QString("%1/resources/fonts/%2.ttf").arg(datadir()).arg(fontName)).absoluteFilePath();
    if (!QFile::exists(fntx)) return;

    FILE* fp = fopen(fntx.toStdString().c_str(), "rb");
    if (!fp) return;
    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* ttfBuffer = (unsigned char*)malloc(fsize);
    if (!ttfBuffer)
    {
        fclose(fp);
    }

    fread(ttfBuffer, 1, fsize, fp);
    fclose(fp);
    fp = NULL;

    unsigned char* bmap = (unsigned char*) malloc(TEXTURE_SIZE*TEXTURE_SIZE);
    if (!bmap)
    {
        free(ttfBuffer);
    }

    stbtt_BakeFontBitmap(ttfBuffer, 0, pointSize, bmap, TEXTURE_SIZE, TEXTURE_SIZE, 32, 96, fnt);

    // can free ttf_buffer at this point
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(ttfBuffer);
    free(bmap);
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
        Agros2D::log()->printError(tr("Problem"), tr("Image cannot be saved to the file '%1'.").arg(fileName));
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

