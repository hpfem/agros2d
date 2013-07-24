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
#include "util/global.h"
#include "util/loops.h"

#include "scene.h"
#include "hermes2d/problem.h"
#include "logview.h"

#include "util/constants.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"
#include "resultsview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/plugin_interface.h"

#include "pythonlab/pythonengine_agros.h"

SceneViewPost2D::SceneViewPost2D(PostHermes *postHermes, QWidget *parent)
    : SceneViewCommon2D(postHermes, parent),
      m_listContours(-1),
      m_listVectors(-1),
      m_listScalarField(-1),
      m_selectedPoint(Point())
{
    createActionsPost2D();

    connect(this, SIGNAL(mousePressed(Point)), this, SLOT(selectedPoint(Point)));

    connect(Agros2D::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postHermes, SIGNAL(processed()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));
}

SceneViewPost2D::~SceneViewPost2D()
{
}

void SceneViewPost2D::createActionsPost2D()
{
    // scene mode
    actSceneModePost2D = new QAction(iconView(), tr("Post 2D"), this);
    actSceneModePost2D->setShortcut(tr("Ctrl+4"));
    actSceneModePost2D->setCheckable(true);

    // point
    actSelectPoint = new QAction(icon("select-by-point"), tr("Local point value"), this);
    connect(actSelectPoint, SIGNAL(triggered()), this, SLOT(selectPoint()));

    // marker
    actSelectByMarker = new QAction(icon("select-by-marker"), tr("Select by marker"), this);
    connect(actSelectByMarker, SIGNAL(triggered()), this, SLOT(selectByMarker()));

    // postprocessor group
    actPostprocessorModeNothing = new QAction(icon("mode-nothing"), tr("Nothing"), this);
    actPostprocessorModeNothing->setCheckable(true);

    actPostprocessorModeLocalPointValue = new QAction(icon("mode-localpointvalue"), tr("Local Values"), this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("mode-surfaceintegral"), tr("Surface Integrals"), this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("mode-volumeintegral"), tr("Volume Integrals"), this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    actPostprocessorModeGroup->addAction(actPostprocessorModeNothing);
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));

    actExportVTKScalar = new QAction(tr("Export VTK scalar..."), this);
    connect(actExportVTKScalar, SIGNAL(triggered()), this, SLOT(exportVTKScalarView()));
}

void SceneViewPost2D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A)
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
                emit mousePressed();
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
                emit mousePressed();
            }

            updateGL();
        }
    }

    SceneViewCommon2D::keyPressEvent(event);
}

void SceneViewPost2D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon2D::mousePressEvent(event);

    m_lastPos = event->pos();
    Point p = transform(Point(event->pos().x(), event->pos().y()));

    if (event->buttons() & Qt::LeftButton)
    {
        // local point value
        if (actPostprocessorModeLocalPointValue->isChecked())
        {
            m_selectedPoint = p;
            emit mousePressed(p);

            updateGL();
        }

        // select volume integral area
        if (actPostprocessorModeVolumeIntegral->isChecked())
        {
            Hermes::Hermes2D::Element *e = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(false, postHermes()->activeViewField()->initialMesh(), p.x, p.y);
            if (e)
            {
                SceneLabel *label = Agros2D::scene()->labels->at(atoi(postHermes()->activeViewField()->initialMesh()->get_element_markers_conversion().
                                                                      get_user_marker(e->marker).marker.c_str()));

                label->setSelected(!label->isSelected());
                updateGL();
            }
            emit mousePressed();
        }

        // select surface integral area
        if (actPostprocessorModeSurfaceIntegral->isChecked())
        {
            //  find edge marker
            SceneEdge *edge = SceneEdge::findClosestEdge(p);

            edge->setSelected(!edge->isSelected());
            updateGL();

            emit mousePressed();
        }
    }
}

void SceneViewPost2D::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundRed).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundGreen).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundBlue).toInt() / 255.0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowGrid).toBool()) paintGrid();

    // view
    if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
    {
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool()) paintScalarField();
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool()) paintContours();
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool()) paintVectors();
    }

    // geometry
    paintGeometry();

    if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
    {
        if (actPostprocessorModeLocalPointValue->isChecked()) paintPostprocessorSelectedPoint();
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();

        // bars
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool()
                && Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool())
            paintScalarFieldColorBar(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble(),
                                     Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble());
    }

    // rulers
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool()) paintAxes();

    paintZoomRegion();

    if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
    {
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool())
        {
            Module::LocalVariable localVariable = postHermes()->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString());
            QString text = Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString() != "" ? localVariable.name() : "";
            if ((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt() != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());

            emit labelCenter(text);
        }
        else
        {
            emit labelCenter(tr("Postprocessor 2D"));
        }
    }
}

void SceneViewPost2D::resizeGL(int w, int h)
{
    if (Agros2D::problem()->isSolved())
    {
        paletteCreate();
    }

    SceneViewCommon::resizeGL(w, h);
}

void SceneViewPost2D::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesBlue).toInt() / 255.0);
        glLineWidth(Agros2D::problem()->setting()->value(ProblemSetting::View_EdgeWidth).toInt());

        if (fabs(edge->angle()) < EPS_ZERO)
        {
            glBegin(GL_LINES);
            glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
            glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
            glEnd();
        }
        else
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle());
        }

        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintScalarField()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->scalarIsPrepared()) return;

    loadProjection2d(true);

    if (m_listScalarField == -1)
    {
        paletteCreate();

        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        // range
        double irange = 1.0 / (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble());
        // special case: constant solution
        if (fabs(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) < EPS_ZERO)
            irange = 1.0;

        m_postHermes->linScalarView().lock_data();

        double3* linVert = m_postHermes->linScalarView().get_vertices();
        int3* linTris = m_postHermes->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        // set texture for coloring
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, m_textureScalar);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        // set texture transformation matrix
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(m_texShift, 0.0, 0.0);
        glScaled(m_texScale, 0.0, 0.0);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = linVert[linTris[i][j]][0];
                point[j].y = linVert[linTris[i][j]][1];
                value[j]   = linVert[linTris[i][j]][2];
            }

            if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() || avgValue > Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble())
                    continue;
            }

            for (int j = 0; j < 3; j++)
            {
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool())
                    glTexCoord1d(log10((double) (1 + (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt() - 1))
                                       * (value[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange) / log10((double) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt()));
                else
                    glTexCoord1d((value[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
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

        m_postHermes->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField);

        /*
        // range
        double irange = PALETTEENTRIES / (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin);
        // special case: constant solution
        if (fabs(Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) < EPS_ZERO)
            irange = PALETTEENTRIES / 2;

        m_postHermes->linScalarView().lock_data();

        double3* linVert = m_postHermes->linScalarView().get_vertices();
        int3* linTris = m_postHermes->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = linVert[linTris[i][j]][0];
                point[j].y = linVert[linTris[i][j]][1];
                value[j]   = linVert[linTris[i][j]][2];
            }

            if (!Agros2D::problem()->configView()->scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < Agros2D::problem()->configView()->scalarRangeMin || avgValue > Agros2D::problem()->configView()->scalarRangeMax)
                    continue;
            }

            for (int j = 0; j < 3; j++)
            {
                m_arrayScalarField.push_back(QVector2D(point[j].x, point[j].y));

                const double* color = paletteColor2((value[j] - Agros2D::problem()->configView()->scalarRangeMin) * irange);

                m_arrayScalarFieldColors.push_back(QVector3D(color[0], color[1], color[2]));
                // qDebug() << ((value[j] - Agros2D::problem()->configView()->scalarRangeMin) * irange) << m_arrayScalarFieldColors.at(m_arrayScalarFieldColors.count() - 1);

            }
        }

        m_postHermes->linScalarView().unlock_data();
        */
    }
    else
    {
        glCallList(m_listScalarField);

        /*
        double3* linVert = m_postHermes->linScalarView().get_vertices();
        int3* linTris = m_postHermes->linScalarView().get_triangles();

        Point point[3];
        double value[3];

        glLineWidth(1.0);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = linVert[linTris[i][j]][0];
                point[j].y = linVert[linTris[i][j]][1];
                value[j]   = linVert[linTris[i][j]][2];

                glVertex2d(point[j].x, point[j].y);
            }
        }
        glEnd();
        */
        /*
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glColorPointer(3, GL_FLOAT, 0, m_arrayScalarFieldColors.constData());
        glVertexPointer(2, GL_FLOAT, 0, m_arrayScalarField.constData());

        glDrawArrays(GL_TRIANGLES, 0, m_arrayScalarField.size());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        */
    }
}

void SceneViewPost2D::paintContours()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->contourIsPrepared()) return;

    loadProjection2d(true);

    if (m_listContours == -1)
    {
        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        m_postHermes->linContourView().lock_data();

        double3* vecVert = m_postHermes->linContourView().get_vertices();
        int3* tris = m_postHermes->linContourView().get_contour_triangles();

        // transform variable
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        double3* vert = new double3[m_postHermes->linContourView().get_num_vertices()];
        for (int i = 0; i < m_postHermes->linContourView().get_num_vertices(); i++)
        {
            vert[i][0] = vecVert[i][0];
            vert[i][1] = vecVert[i][1];
            vert[i][2] = vecVert[i][2];

            if (vert[i][2] > rangeMax) rangeMax = vecVert[i][2];
            if (vert[i][2] < rangeMin) rangeMin = vecVert[i][2];
        }

        // draw contours
        if ((rangeMax-rangeMin) > EPS_ZERO)
        {
            // value range
            double step = (rangeMax-rangeMin) / Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursCount).toInt();

            glLineWidth(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursWidth).toInt());
            glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursRed).toInt() / 255.0,
                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursGreen).toInt() / 255.0,
                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursBlue).toInt() / 255.0);

            glBegin(GL_LINES);
            for (int i = 0; i < m_postHermes->linContourView().get_num_contour_triangles(); i++)
            {
                if (finite(vert[tris[i][0]][2]) && finite(vert[tris[i][1]][2]) && finite(vert[tris[i][2]][2]))
                {
                    paintContoursTri(vert, &tris[i], step);
                }
            }
            glEnd();
        }

        delete vert;

        m_postHermes->linContourView().unlock_data();

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

            if (perm & 1)
            {
                glVertex2d(x1, y1);
                glVertex2d(x2, y2);
            }
            else
            {
                glVertex2d(x2, y2);
                glVertex2d(x1, y1);
            }

            val += step;
        }
        l1 = 1;
        l2 = 2;
    }
}

/*
static int n_vert(int i) { return (i + 1) % 3; }
static int p_vert(int i) { return (i + 2) % 3; }

void SceneViewPost2D::paintVectors()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->vectorIsPrepared()) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        double vectorRangeMin = m_postHermes->vecVectorView().get_min_value();
        double vectorRangeMax = m_postHermes->vecVectorView().get_max_value();

        // add 20 % margin to the range
        double vectorRange = vectorRangeMax - vectorRangeMin;
        vectorRangeMin = vectorRangeMin - 0.2*vectorRange;
        vectorRangeMax = vectorRangeMax + 0.2*vectorRange;

        // qDebug() << "SceneViewCommon::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (vectorRangeMax - vectorRangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        RectPoint rect = Agros2D::scene()->boundingBox();
        double gs = (rect.width() + rect.height()) / Agros2D::problem()->configView()->vectorCount;

        // paint
        m_postHermes->vecVectorView().lock_data();

        double4* vecVert = m_postHermes->vecVectorView().get_vertices();
        int3* vecTris = m_postHermes->vecVectorView().get_triangles();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        double gx = 0.0;
        double gy = 0.0;

        bool hexa = false;
        double gt = gs * sqrt(3.0)/2.0;

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postHermes->vecVectorView().get_num_triangles(); i++)
        {
            double miny = 1e100;
            int idx = -1;

            Point v1(vecVert[vecTris[i][0]][0], vecVert[vecTris[i][0]][1]);
            Point v2(vecVert[vecTris[i][1]][0], vecVert[vecTris[i][1]][1]);
            Point v3(vecVert[vecTris[i][2]][0], vecVert[vecTris[i][2]][1]);

            // double wh = output_height + gt, ww = output_width + gs;
            // if ((vecVert[vecTris[i][0]][0] < -gs) && (vecVert[vecTris[i][1]][0] < -gs) && (vecVert[vecTris[i][2]][0] < -gs)) continue;
            // if ((vecVert[vecTris[i][0]][0] >  ww) && (vecVert[vecTris[i][1]][0] >  ww) && (vecVert[vecTris[i][2]][0] >  ww)) continue;
            // if ((vecVert[vecTris[i][0]][1] < -gt) && (vecVert[vecTris[i][1]][1] < -gt) && (vecVert[vecTris[i][2]][1] < -gt)) continue;
            // if ((vecVert[vecTris[i][0]][1] >  wh) && (vecVert[vecTris[i][1]][1] >  wh) && (vecVert[vecTris[i][2]][1] >  wh)) continue;


            // find vertex with min y-coordinate
            for (int k = 0; k < 3; k++)
                if (vecVert[vecTris[i][k]][1] < miny)
                    miny = vecVert[vecTris[i][idx = k]][1];

            int l1 = vecTris[i][idx];
            int r1 = vecTris[i][idx];
            int l2 = vecTris[i][n_vert(idx)];
            int r2 = vecTris[i][p_vert(idx)];

            // plane of x and y values on triangle

            // double a[2], b[2], c[2], d[2];
            // for (int n = 0; n < 2; n++)
            {
//                 a[n] = (vecVert[l1][1] - vecVert[l2][1])*(vecVert[r1][2 + n] - vecVert[r2][2 + n]) - (vecVert[l1][2 + n] - vecVert[l2][2 + n])*(vecVert[r1][1] - vecVert[r2][1]);
//                 b[n] = (vecVert[l1][2 + n] - vecVert[l2][2 + n])*(vecVert[r1][0] - vecVert[r2][0]) - (vecVert[l1][0] - vecVert[l2][0])*(vecVert[r1][2 + n] - vecVert[r2][2 + n]);
//                 c[n] = (vecVert[l1][0] - vecVert[l2][0])*(vecVert[r1][1] - vecVert[r2][1]) - (vecVert[l1][1] - vecVert[l2][1])*(vecVert[r1][0] - vecVert[r2][0]);
//                 d[n] = -a[n] * vecVert[l1][0] - b[n] * vecVert[l1][1] - c[n] * vecVert[l1][2 + n];
//                 a[n] /= c[n]; b[n] /= c[n]; d[n] /= c[n];
            }

            int s = (int) ceil((vecVert[l1][1] - gy)/gt);  // first step
            double lry = gy + s*gt;
            bool shift = hexa && (s & 1);

            // if there are two points with min y-coordinate, switch to the next segment
            if ((vecVert[l1][1] == vecVert[l2][1]) || (vecVert[r1][1] == vecVert[r2][1]))
            {
                if (vecVert[l1][1] == vecVert[l2][1])
                {
                    l1 = l2;
                    l2 = r2;
                }
                else if (vecVert[r1][1] == vecVert[r2][1])
                {
                    r1 = r2;
                    r2 = l2;
                }
            }

            // slope of the left and right segment
            double ml = (vecVert[l1][0] - vecVert[l2][0])/(vecVert[l1][1] - vecVert[l2][1]);
            double mr = (vecVert[r1][0] - vecVert[r2][0])/(vecVert[r1][1] - vecVert[r2][1]);

            // x-coordinates of the endpoints of the first line
            double lx = vecVert[l1][0] + ml * (lry - (vecVert[l1][1]));
            double rx = vecVert[r1][0] + mr * (lry - (vecVert[r1][1]));

            if (lry < -gt)
            {
                int j = (int) floor(-lry/gt);
                lry += gt * j;
                lx += j * ml * gt;
                rx += j * mr * gt;
            }

            // while we are in triangle
            while (((lry < vecVert[l2][1]) || (lry < vecVert[r2][1]))) // && (lry < wh))
            {
                // while we are in the segment
                while (((lry <= vecVert[l2][1]) && (lry <= vecVert[r2][1]))) // && (lry < wh))
                {
                    double gz = gx;
                    if (shift) gz -= 0.5*gs;
                    s = (int) ceil((lx - gz)/gs);
                    double x = gz + s*gs;
                    if (hexa) shift = !shift;

                    if (x < -gs)
                    {
                        int j = (int) floor(-x/gs);
                        x += gs * j;
                    }

                    // go along the line
                    while ((x >= rx)) // && (x < ww))
                    {
                        // plot the arrow
                        // xval = -a[0]*x - b[0]*lry - d[0];
                        // yval = -a[1]*x - b[1]*lry - d[1];
                        // xval = -1.0*x - 1.0*lry - 0;
                        // yval = -1.0*x - 1.0*lry - 0;
                        // plot_arrow(x, lry, xval, yval, max, min, gs);

                        // qDebug() << x << lry;
                        glBegin(GL_POINTS);
                        glVertex2d(x, lry);
                        glEnd();

                        // color
                        if ((Agros2D::problem()->configView()->vectorColor) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            double color = 0.7 - 0.7 * (value - vectorRangeMin) * irange;
                            glColor3d(color, color, color);
                        }
                        else
                        {
                            glColor3d(Agros2D::problem()->configView()->colorVectors.redF(),
                                      Agros2D::problem()->configView()->colorVectors.greenF(),
                                      Agros2D::problem()->configView()->colorVectors.blueF());
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
                        glVertex2d(vs1x, vs1y); glVertex2d(vs2x, vs2y); glVertex2d(vs3x, vs3y);
                        glVertex2d(vs4x, vs4y); glVertex2d(vs3x, vs3y); glVertex2d(vs2x, vs2y);

                        x -= gs;
                    }

                    // move to the next line
                    lx += ml*gt;
                    rx += mr*gt;
                    lry += gt;
                }
                // change segment
                if (lry >= vecVert[l2][1])
                {
                    l1 = l2;
                    l2 = r2;

                    ml = (vecVert[l1][0] - vecVert[l2][0])/(vecVert[l1][1] - vecVert[l2][1]);
                    lx = vecVert[l1][0] + ml * (lry - (vecVert[l1][1]));
                }
                else
                {
                    r1 = r2;
                    r2 = l2;

                    mr = (vecVert[r1][0] - vecVert[r2][0])/(vecVert[r1][1] - vecVert[r2][1]);
                    rx = vecVert[r1][0] + mr * (lry - (vecVert[r1][1]));
                }
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_postHermes->vecVectorView().unlock_data();

        glEndList();

        glCallList(m_listVectors);
    }
    else
    {
        glCallList(m_listVectors);
    }
}
*/

void SceneViewPost2D::paintVectors()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->vectorIsPrepared()) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        double vectorRangeMin = m_postHermes->vecVectorView().get_min_value();
        double vectorRangeMax = m_postHermes->vecVectorView().get_max_value();

        //Add 20% margin to the range
        double vectorRange = vectorRangeMax - vectorRangeMin;
        vectorRangeMin = vectorRangeMin - 0.2*vectorRange;
        vectorRangeMax = vectorRangeMax + 0.2*vectorRange;

        // qDebug() << "SceneViewCommon::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (vectorRangeMax - vectorRangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        RectPoint rect = Agros2D::scene()->boundingBox();
        double gs = (rect.width() + rect.height()) / Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCount).toInt();

        // paint
        m_postHermes->vecVectorView().lock_data();

        double4* vecVert = m_postHermes->vecVectorView().get_vertices();
        int3* vecTris = m_postHermes->vecVectorView().get_triangles();

        /*
        Point point[3];
        double value[3];

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postHermes->vecVectorView().get_num_triangles(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = vecVert[vecTris[i][j]][0];
                point[j].y = vecVert[vecTris[i][j]][1];
                value[j]   = vecVert[vecTris[i][j]][2];

                glVertex2d(point[j].x, point[j].y);
            }
        }
        glEnd();
        */

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postHermes->vecVectorView().get_num_triangles(); i++)
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

                        if ((Agros2D::problem()->setting()->value(ProblemSetting::View_VectorProportional).toBool()) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            if ((value / vectorRangeMax) < 1e-6)
                            {
                                dx = 0.0;
                                dy = 0.0;
                            }
                            else
                            {
                                dx = ((value - vectorRangeMin) * irange) * Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * cos(angle);
                                dy = ((value - vectorRangeMin) * irange) * Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * sin(angle);
                            }
                        }
                        else
                        {
                            dx = Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * cos(angle);
                            dy = Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * sin(angle);
                        }

                        double dm = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));

                        // color
                        if ((Agros2D::problem()->setting()->value(ProblemSetting::View_VectorColor).toBool())
                                && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            double color = 0.7 - 0.7 * (value - vectorRangeMin) * irange;
                            glColor3d(color, color, color);
                        }
                        else
                        {
                            glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorVectorsRed).toInt() / 255.0,
                                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorVectorsGreen).toInt() / 255.0,
                                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorVectorsBlue).toInt() / 255.0);
                        }

                        // tail
                        Point shiftCenter(0.0, 0.0);
                        if ((VectorCenter) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCenter).toInt() == VectorCenter_Head)
                            shiftCenter = Point(- 2.0*dm * cos(angle), - 2.0*dm * sin(angle)); // head
                        if ((VectorCenter) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCenter).toInt() == VectorCenter_Center)
                            shiftCenter = Point(- dm * cos(angle), - dm * sin(angle)); // center

                        if ((VectorType) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorType).toInt() == VectorType_Arrow)
                        {
                            // arrow and shaft
                            // head for an arrow
                            double vh1x = point.x + dm/5.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vh1y = point.y + dm/5.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vh2x = point.x + dm/5.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vh2y = point.y + dm/5.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                            double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;

                            glVertex2d(vh1x, vh1y);
                            glVertex2d(vh2x, vh2y);
                            glVertex2d(vh3x, vh3y);

                            // shaft for an arrow
                            double vs1x = point.x + dm/15.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vs1y = point.y + dm/15.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vs2x = point.x + dm/15.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vs2y = point.y + dm/15.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vs3x = vs1x - dm * cos(angle);
                            double vs3y = vs1y - dm * sin(angle);
                            double vs4x = vs2x - dm * cos(angle);
                            double vs4y = vs2y - dm * sin(angle);

                            glVertex2d(vs1x, vs1y);
                            glVertex2d(vs2x, vs2y);
                            glVertex2d(vs3x, vs3y);
                            glVertex2d(vs4x, vs4y);
                            glVertex2d(vs3x, vs3y);
                            glVertex2d(vs2x, vs2y);
                        }
                        else if ((VectorType) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorType).toInt() == VectorType_Cone)
                        {
                            // cone
                            double vh1x = point.x + dm/3.5 * cos(angle - M_PI/2.0) + shiftCenter.x;
                            double vh1y = point.y + dm/3.5 * sin(angle - M_PI/2.0) + shiftCenter.y;
                            double vh2x = point.x + dm/3.5 * cos(angle + M_PI/2.0) + shiftCenter.x;
                            double vh2y = point.y + dm/3.5 * sin(angle + M_PI/2.0) + shiftCenter.y;
                            double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                            double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;

                            glVertex2d(vh1x, vh1y);
                            glVertex2d(vh2x, vh2y);
                            glVertex2d(vh3x, vh3y);
                        }
                    }
                }
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_postHermes->vecVectorView().unlock_data();

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
    if (!Agros2D::problem()->isSolved()) return;

    if (Agros2D::scene()->crossings().isEmpty())
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // blended rectangle
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedBlue).toInt() / 255.0,
                  0.5);

        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(Agros2D::scene()->loopsInfo()->polygonTriangles());
        glBegin(GL_TRIANGLES);
        while (i.hasNext())
        {
            i.next();

            if (i.key()->isSelected())
            {
                foreach (LoopsInfo::Triangle triangle, i.value())
                {
                    glVertex2d(triangle.a.x, triangle.a.y);
                    glVertex2d(triangle.b.x, triangle.b.y);
                    glVertex2d(triangle.c.x, triangle.c.y);
                }
            }
        }
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedSurface()
{
    if (!Agros2D::problem()->isSolved()) return;

    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items()) {
        glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedBlue).toInt() / 255.0);
        glLineWidth(3.0);

        if (edge->isSelected())
        {
            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
                glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
                glEnd();
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180 - 180;

                drawArc(center, radius, startAngle, edge->angle());
            }
        }
        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedPoint()
{
    if (!Agros2D::problem()->isSolved()) return;

    glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedRed).toInt() / 255.0,
              Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedGreen).toInt() / 255.0,
              Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedBlue).toInt() / 255.0);
    glPointSize(8.0);

    glBegin(GL_POINTS);
    glVertex2d(m_selectedPoint.x, m_selectedPoint.y);
    glEnd();
}

void SceneViewPost2D::clearGLLists()
{
    if (m_listContours != -1) glDeleteLists(m_listContours, 1);
    if (m_listVectors != -1) glDeleteLists(m_listVectors, 1);
    if (m_listScalarField != -1) glDeleteLists(m_listScalarField, 1);

    m_listContours = -1;
    m_listVectors = -1;
    m_listScalarField = -1;
}

void SceneViewPost2D::refresh()
{
    clearGLLists();

    setControls();

    if (Agros2D::problem()->isSolved())
        SceneViewCommon2D::refresh();
}

void SceneViewPost2D::setControls()
{
    // actions
    actSceneModePost2D->setEnabled(Agros2D::problem()->isSolved());
    actPostprocessorModeGroup->setEnabled(Agros2D::problem()->isSolved());
    actSelectByMarker->setEnabled(Agros2D::problem()->isSolved() && (actPostprocessorModeSurfaceIntegral->isChecked() || actPostprocessorModeVolumeIntegral->isChecked()));
    actSelectPoint->setEnabled(Agros2D::problem()->isSolved() && actPostprocessorModeLocalPointValue->isChecked());
    actExportVTKScalar->setEnabled(Agros2D::problem()->isSolved());
}

void SceneViewPost2D::clear()
{
    actPostprocessorModeNothing->trigger();

    setControls();

    SceneViewCommon2D::clear();
    if (Agros2D::problem()->isSolved())
        SceneViewCommon::refresh();
}

void SceneViewPost2D::exportVTKScalarView(const QString &fileName)
{
    if (Agros2D::problem()->isSolved())
    {
        QString fn = fileName;

        if (fn.isEmpty())
        {
            // file dialog
            QSettings settings;
            QString dir = settings.value("General/LastVTKDir").toString();

            fn = QFileDialog::getSaveFileName(this, tr("Export VTK file"), dir, tr("VTK files (*.vtk)"));
            if (fn.isEmpty())
                return;

            if (!fn.endsWith(".vtk"))
                fn.append(".vtk");

            // remove existing file
            if (QFile::exists(fn))
                QFile::remove(fn);
        }

        Hermes::Hermes2D::Views::Linearizer linScalarView;
        Hermes::Hermes2D::Filter<double> *slnScalarView = m_postHermes->viewScalarFilter(postHermes()->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString()),
                                                                                         (PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());

        linScalarView.save_solution_vtk(slnScalarView,
                                        fn.toLatin1().data(),
                                        Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().toLatin1().data(),
                                        true);

        delete slnScalarView;

        if (!fn.isEmpty())
        {
            QFileInfo fileInfo(fn);
            if (fileInfo.absoluteDir() != tempProblemDir())
            {
                QSettings settings;
                settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
            }
        }
    }
}

void SceneViewPost2D::selectByMarker()
{
    SceneModePostprocessor mode = (actPostprocessorModeSurfaceIntegral->isChecked()) ? SceneModePostprocessor_SurfaceIntegral : SceneModePostprocessor_VolumeIntegral;

    SceneMarkerSelectDialog sceneMarkerSelectDialog(this, mode, QApplication::activeWindow());
    if (sceneMarkerSelectDialog.exec() == QDialog::Accepted)
        emit mousePressed();
}

void SceneViewPost2D::selectPoint()
{
    LocalPointValueDialog localPointValueDialog(m_selectedPoint);
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        emit mousePressed(localPointValueDialog.point());
        updateGL();
    }
}

void SceneViewPost2D::doPostprocessorModeGroup(QAction *action)
{
    if (actPostprocessorModeNothing->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_Empty);
    if (actPostprocessorModeLocalPointValue->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_LocalValue);
    if (actPostprocessorModeSurfaceIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_SurfaceIntegral);
    if (actPostprocessorModeVolumeIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_VolumeIntegral);

    Agros2D::scene()->selectNone();

    setControls();
    updateGL();
}

void SceneViewPost2D::showGroup(QAction *action)
{
    refresh();
}

void SceneViewPost2D::selectedPoint(const Point &p)
{
    m_selectedPoint = p;
    updateGL();
}
