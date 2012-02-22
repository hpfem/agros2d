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

#include "sceneview_post2d.h"
#include "util.h"
#include "scene.h"
#include "scenesolution.h"
#include "hermes2d/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

SceneViewPost2D::SceneViewPost2D(QWidget *parent)
    : SceneViewCommon2D(parent)
{
    createActionsPost2D();

    connect(m_scene, SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(m_scene, SIGNAL(defaultValues()), this, SLOT(doDefaultValues()));
}

SceneViewPost2D::~SceneViewPost2D()
{
}

void SceneViewPost2D::createActionsPost2D()
{
    // scene mode
    actSceneModePost2D = new QAction(icon("scene-post2d"), tr("Postprocessor 2D"), this);
    actSceneModePost2D->setShortcut(Qt::Key_F7);
    actSceneModePost2D->setStatusTip(tr("Postprocessor 2D"));
    actSceneModePost2D->setCheckable(true);

    actSceneViewSelectByMarker = new QAction(icon(""), tr("Select by marker"), this);
    actSceneViewSelectByMarker->setStatusTip(tr("Select by marker"));
    connect(actSceneViewSelectByMarker, SIGNAL(triggered()), this, SLOT(doSelectMarker()));

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
}

void SceneViewPost2D::keyPressEvent(QKeyEvent *event)
{
    SceneViewCommon2D::keyPressEvent(event);

    switch (event->key())
    {
    case Qt::Key_A:
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {

            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                m_scene->selectAll(SceneGeometryMode_OperateOnLabels);
                emit mousePressed();
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                m_scene->selectAll(SceneGeometryMode_OperateOnEdges);
                emit mousePressed();
            }


            refresh();
        }
    }
        break;
    default:
        QGLWidget::keyPressEvent(event);
    }
}

void SceneViewPost2D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon2D::mousePressEvent(event);

    m_lastPos = event->pos();
    Point p = position(Point(event->pos().x(), event->pos().y()));

    // local point value
    if (actPostprocessorModeLocalPointValue->isChecked())
        emit mousePressed(p);
    // select volume integral area
    if (actPostprocessorModeVolumeIntegral->isChecked())
    {
        int index = m_scene->activeSceneSolution()->findElementInMesh(m_scene->activeSceneSolution()->meshInitial(), p);
        if (index != -1)
        {
            //  find label marker
            int labelIndex = atoi(Util::problem()->meshInitial()->get_element_markers_conversion().get_user_marker(m_scene->activeSceneSolution()->meshInitial()->get_element_fast(index)->marker).marker.c_str());

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

void SceneViewPost2D::paintGL()
{
    logMessage("SceneViewCommon::paintGL()");

    glClearColor(Util::config()->colorBackground.redF(),
                 Util::config()->colorBackground.greenF(),
                 Util::config()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    if (Util::config()->showGrid) paintGrid();

    // view
    if (Util::problem()->isSolved())
    {
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView) paintScalarField();
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Order) paintOrder();

        if (m_sceneViewSettings.showContours) paintContours();
        if (m_sceneViewSettings.showVectors) paintVectors();
        if (m_sceneViewSettings.showSolutionMesh) paintSolutionMesh();
    }

    // initial mesh
    if (Util::problem()->isMeshed())
    {
        if (m_sceneViewSettings.showInitialMesh) paintInitialMesh();
    }

    // geometry
    if (m_sceneViewSettings.showGeometry) paintGeometry();

    if (Util::problem()->isSolved())
    {
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();

        // bars
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView) paintScalarFieldColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Order) paintOrderColorBar();
    }

    // rulers
    if (Util::config()->showRulers)
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Util::config()->showAxes) paintAxes();

    paintZoomRegion();
    paintChartLine();

    if (Util::config()->showLabel)
    {
        switch (m_sceneViewSettings.postprocessorShow)
        {
        case SceneViewPostprocessorShow_ScalarView:
        {
            if (Util::problem()->isSolved())
            {
                QString text = QString::fromStdString(m_sceneViewSettings.scalarPhysicFieldVariable != "" ? Util::scene()->activeViewField()->module()->get_variable(m_sceneViewSettings.scalarPhysicFieldVariable)->name : "");
                if (m_sceneViewSettings.scalarPhysicFieldVariableComp != PhysicFieldVariableComp_Scalar)
                    text += " - " + physicFieldVariableCompString(m_sceneViewSettings.scalarPhysicFieldVariableComp);
                paintSceneModeLabel(text);
            }
        }
            break;
        default:
            paintSceneModeLabel(tr("Postprocessor 2D"));
        }
    }
}

void SceneViewPost2D::resizeGL(int w, int h)
{
    SceneViewCommon::resizeGL(w, h);

    if (Util::problem()->isSolved())
    {
        paletteFilter();
        paletteUpdateTexAdjust();
        paletteCreate();
    }
}

void SceneViewPost2D::paintGeometry()
{
    logMessage("SceneViewCommon::paintGeometry()");

    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, m_scene->edges->items())
    {
        glColor3d(Util::config()->colorEdges.redF(),
                  Util::config()->colorEdges.greenF(),
                  Util::config()->colorEdges.blueF());
        glLineWidth(Util::config()->edgeWidth);

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

        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintChartLine()
{
    logMessage("SceneViewCommon::paintChartLine()");

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

void SceneViewPost2D::paintSolutionMesh()
{
    logMessage("SceneViewCommon::paintSolutionMesh()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    Util::scene()->activeSceneSolution()->linSolutionMeshView().lock_data();

    double3* linVert = Util::scene()->activeSceneSolution()->linSolutionMeshView().get_vertices();
    int3* linEdges = Util::scene()->activeSceneSolution()->linSolutionMeshView().get_edges();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(Util::config()->colorSolutionMesh.redF(),
              Util::config()->colorSolutionMesh.greenF(),
              Util::config()->colorSolutionMesh.blueF());
    glLineWidth(1.3);

    // triangles
    // qDebug() << Util::problem()->linSolutionMeshView().get_num_edges();
    glBegin(GL_LINES);
    for (int i = 0; i < Util::scene()->activeSceneSolution()->linSolutionMeshView().get_num_edges(); i++)
    {
        glVertex2d(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]);
        glVertex2d(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]);
    }
    glEnd();

    Util::scene()->activeSceneSolution()->linSolutionMeshView().unlock_data();

}

void SceneViewPost2D::paintOrder()
{
    logMessage("SceneViewCommon::paintOrder()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listOrder == -1)
    {
        m_listOrder = glGenLists(1);
        glNewList(m_listOrder, GL_COMPILE);

        // order scalar view
        m_scene->activeSceneSolution()->ordView().lock_data();

        double3* vert = m_scene->activeSceneSolution()->ordView().get_vertices();
        int3* tris = m_scene->activeSceneSolution()->ordView().get_triangles();

        // draw mesh
        int min = 11;
        int max = 1;
        for (int i = 0; i < m_scene->activeSceneSolution()->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->activeSceneSolution()->ordView().get_num_triangles(); i++)
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

        m_scene->activeSceneSolution()->ordView().lock_data();

        double3* vert = m_scene->activeSceneSolution()->ordView().get_vertices();
        int* lvert;
        char** ltext;
        double2* lbox;
        int nl = m_scene->activeSceneSolution()->ordView().get_labels(lvert, ltext, lbox);

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

        m_scene->activeSceneSolution()->ordView().unlock_data();
    }
}

void SceneViewPost2D::paintOrderColorBar()
{
    logMessage("SceneViewCommon::paintOrderColorBar()");

    if (!m_isSolutionPrepared || !Util::config()->showOrderScale) return;

    // order scalar view
    m_scene->activeSceneSolution()->ordView().lock_data();

    double3* vert = m_scene->activeSceneSolution()->ordView().get_vertices();
    int3* tris = m_scene->activeSceneSolution()->ordView().get_triangles();

    int min = 11;
    int max = 1;
    for (int i = 0; i < m_scene->activeSceneSolution()->ordView().get_num_triangles(); i++)
    {
        if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
        if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
    }

    m_scene->activeSceneSolution()->ordView().unlock_data();

    // order color map
    loadProjectionViewPort();

    glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
    glTranslated(- contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    // dimensions
    int textWidth = fontMetrics().width("00");
    int textHeight = fontMetrics().height();
    Point scaleSize = Point(20 + 3 * textWidth, (20 + max * (2 * textHeight) - textHeight / 2.0 + 2));
    Point scaleBorder = Point(10.0, (Util::config()->showRulers) ? - (m_rulersAreaWidth.y/4.0)*m_scale2d*contextHeight() + 20.0 : 10.0);
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

void SceneViewPost2D::paintScalarField()
{
    logMessage("SceneViewCommon::paintScalarField()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listScalarField == -1)
    {
        // qDebug() << "SceneViewCommon::paintScalarField(), min = " << m_sceneViewSettings.scalarRangeMin << ", max = " << m_sceneViewSettings.scalarRangeMax;

        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        // range
        double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
        // special case: constant solution
        if (fabs(m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin) < EPS_ZERO)
            irange = 1.0;

        m_scene->activeSceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->activeSceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->activeSceneSolution()->linScalarView().get_triangles();
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
        for (int i = 0; i < m_scene->activeSceneSolution()->linScalarView().get_num_triangles(); i++)
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

        m_scene->activeSceneSolution()->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField);
    }
    else
    {
        glCallList(m_listScalarField);
    }
}

void SceneViewPost2D::paintContours()
{
    logMessage("SceneViewCommon::paintContours()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listContours == -1)
    {
        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        m_scene->activeSceneSolution()->linContourView().lock_data();

        double3* tvert = m_scene->activeSceneSolution()->linContourView().get_vertices();
        int3* tris = m_scene->activeSceneSolution()->linContourView().get_triangles();

        // transform variable
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        double3* vert = new double3[m_scene->activeSceneSolution()->linContourView().get_num_vertices()];
        for (int i = 0; i < m_scene->activeSceneSolution()->linContourView().get_num_vertices(); i++)
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
        for (int i = 0; i < m_scene->activeSceneSolution()->linContourView().get_num_triangles(); i++)
        {
            if (finite(vert[tris[i][0]][2]) && finite(vert[tris[i][1]][2]) && finite(vert[tris[i][2]][2]))
            {
                paintContoursTri(vert, &tris[i], step);
            }
        }
        glEnd();

        delete vert;

        m_scene->activeSceneSolution()->linContourView().unlock_data();

        glEndList();

        glCallList(m_listContours);
    }
    else
    {
        glCallList(m_listContours);
    }
}

void SceneViewPost2D::paintContoursTri(double3* vert, int3* tri, double step)
{
    logMessage("SceneViewCommon::paintContoursTri()");

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

void SceneViewPost2D::paintVectors()
{
    logMessage("SceneViewCommon::paintVectors()");

    if (!m_isSolutionPrepared) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        double vectorRangeMin = m_scene->activeSceneSolution()->vecVectorView().get_min_value();
        double vectorRangeMax = m_scene->activeSceneSolution()->vecVectorView().get_max_value();

        //Add 20% margin to the range
        double vectorRange = vectorRangeMax - vectorRangeMin;
        vectorRangeMin = vectorRangeMin - 0.2*vectorRange;
        vectorRangeMax = vectorRangeMax + 0.2*vectorRange;

        // qDebug() << "SceneViewCommon::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (vectorRangeMax - vectorRangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        RectPoint rect = m_scene->boundingBox();
        double gs = (rect.width() + rect.height()) / Util::config()->vectorCount;

        // paint
        m_scene->activeSceneSolution()->vecVectorView().lock_data();

        double4* vecVert = m_scene->activeSceneSolution()->vecVectorView().get_vertices();
        int3* vecTris = m_scene->activeSceneSolution()->vecVectorView().get_triangles();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->activeSceneSolution()->vecVectorView().get_num_triangles(); i++)
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

        m_scene->activeSceneSolution()->vecVectorView().unlock_data();

        glEndList();

        glCallList(m_listVectors);
    }
    else
    {
        glCallList(m_listVectors);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedVolume()
{
    logMessage("SceneViewCommon::paintPostprocessorSelectedVolume()");

    if (!Util::problem()->isMeshed()) return;

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
    for (int i = 0; i < m_scene->activeSceneSolution()->meshInitial()->get_num_active_elements(); i++)
    {
        Hermes::Hermes2D::Element *element = m_scene->activeSceneSolution()->meshInitial()->get_element(i);
        if (m_scene->labels->at(atoi(Util::problem()->meshInitial()->get_element_markers_conversion().get_user_marker(element->marker).marker.c_str()))->isSelected)
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
    logMessage("SceneViewCommon::paintPostprocessorSelectedVolume()");

    if (!m_scene->activeSceneSolution()->isMeshed()) return;

    m_scene->activeSceneSolution()->linInitialMeshView().lock_data();

    double3* linVert = m_scene->activeSceneSolution()->linInitialMeshView().get_vertices();
    int3* linTris = m_scene->activeSceneSolution()->linInitialMeshView().get_triangles();

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
    for (int i = 0; i < Util::problem()->linSolutionMeshView().get_num_triangles(); i++)
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

    Util::problem()->linSolutionMeshView().unlock_data();
    */
}

void SceneViewPost2D::paintPostprocessorSelectedSurface()
{
    logMessage("SceneViewCommon::paintPostprocessorSelectedSurface()");

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

void SceneViewPost2D::doDefaultValues()
{
    actPostprocessorModeLocalPointValue->trigger();

    SceneViewCommon2D::doDefaultValues();
}

void SceneViewPost2D::doSelectMarker()
{
    logMessage("SceneViewCommon::doSelectMarker()");

    SceneMarkerSelectDialog sceneMarkerSelectDialog(this, QApplication::activeWindow());
    sceneMarkerSelectDialog.exec();
}

void SceneViewPost2D::doPostprocessorModeGroup(QAction *action)
{
    logMessage("SceneViewCommon::doPostprocessorModeGroup()");

    if (actPostprocessorModeLocalPointValue->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_LocalValue);
    if (actPostprocessorModeSurfaceIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_SurfaceIntegral);
    if (actPostprocessorModeVolumeIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_VolumeIntegral);

    m_scene->selectNone();
    updateGL();
}


void SceneViewPost2D::doShowGroup(QAction *action)
{
    logMessage("SceneViewCommon::doShowGroup()");

    m_sceneViewSettings.showContours = actShowContours->isChecked();
    m_sceneViewSettings.showVectors = actShowVectors->isChecked();
    m_sceneViewSettings.showSolutionMesh = actShowSolutionMesh->isChecked();

    doInvalidated();
}
