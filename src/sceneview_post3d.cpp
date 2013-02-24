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

#include "sceneview_post3d.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d/problem.h"
#include "logview.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem_config.h"

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

SceneViewPost3D::SceneViewPost3D(PostHermes *postHermes, QWidget *parent)
    : SceneViewCommon3D(postHermes, parent),
      m_listScalarField3D(-1),
      m_listScalarField3DSolid(-1),
      m_listModel(-1)
{
    createActionsPost3D();

    connect(Agros2D::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postHermes, SIGNAL(processed()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

}

SceneViewPost3D::~SceneViewPost3D()
{
}

void SceneViewPost3D::createActionsPost3D()
{
    actSceneModePost3D = new QAction(iconView(), tr("Post 3D"), this);
    actSceneModePost3D->setShortcut(tr("Ctrl+5"));
    actSceneModePost3D->setStatusTip(tr("Postprocessor 3D"));
    actSceneModePost3D->setCheckable(true);
}

void SceneViewPost3D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon3D::mousePressEvent(event);
}

void SceneViewPost3D::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Agros2D::problem()->configView()->colorBackground.redF(),
                 Agros2D::problem()->configView()->colorBackground.greenF(),
                 Agros2D::problem()->configView()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Agros2D::problem()->isMeshed())
    {
        if (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_Model) paintScalarField3DSolid();
    }

    if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
    {
        if (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3D) paintScalarField3D();
        if (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3DSolid) paintScalarField3DSolid();

        // bars
        if (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3D ||
                Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3DSolid)
            paintScalarFieldColorBar(Agros2D::problem()->configView()->scalarRangeMin, Agros2D::problem()->configView()->scalarRangeMax);
    }

    switch (Agros2D::problem()->configView()->showPost3D)
    {
    case SceneViewPost3DMode_ScalarView3D:
    case SceneViewPost3DMode_ScalarView3DSolid:
    {
        if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
        {
            Module::LocalVariable localVariable = postHermes()->activeViewField()->localVariable(Agros2D::problem()->configView()->scalarVariable);
            QString text = Agros2D::problem()->configView()->scalarVariable != "" ? localVariable.name() : "";
            if (Agros2D::problem()->configView()->scalarVariableComp != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString(Agros2D::problem()->configView()->scalarVariableComp);

            emit labelCenter(text);
        }
    }
        break;
    case SceneViewPost3DMode_Model:
        emit labelCenter(tr("Model"));
        break;
    default:
        emit labelCenter(tr("Postprocessor 3D"));
    }

    if (Agros2D::problem()->configView()->showAxes) paintAxes();
}

void SceneViewPost3D::resizeGL(int w, int h)
{
    if (Agros2D::problem()->isSolved())
    {
        paletteCreate();
    }

    SceneViewCommon::resizeGL(w, h);
}

void SceneViewPost3D::paintScalarField3D()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->scalarIsPrepared()) return;

    loadProjection3d(true, Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3D);

    if (m_listScalarField3D == -1)
    {
        paletteCreate();

        m_listScalarField3D = glGenLists(1);
        glNewList(m_listScalarField3D, GL_COMPILE);

        // gradient background
        paintBackground();
        glEnable(GL_DEPTH_TEST);

        // range
        double irange = 1.0 / (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin);
        // special case: constant solution
        if (fabs(Agros2D::problem()->configView()->scalarRangeMin - Agros2D::problem()->configView()->scalarRangeMax) < EPS_ZERO)
        {
            irange = 1.0;
        }

        m_postHermes->linScalarView().lock_data();

        double3* linVert = m_postHermes->linScalarView().get_vertices();
        int3* linTris = m_postHermes->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        RectPoint rect = Agros2D::scene()->boundingBox();

        double max = qMax(rect.width(), rect.height());

        if (Agros2D::problem()->configView()->scalarView3DLighting)
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        else
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        glPushMatrix();
        glScaled(1.0, 1.0, max / Agros2D::problem()->configView()->scalarView3DHeight * fabs(irange));

        // scalar view
        initLighting();
        // init normal
        double *normal = new double[3];

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
            point[0].x = linVert[linTris[i][0]][0];
            point[0].y = linVert[linTris[i][0]][1];
            value[0]   = linVert[linTris[i][0]][2];
            point[1].x = linVert[linTris[i][1]][0];
            point[1].y = linVert[linTris[i][1]][1];
            value[1]   = linVert[linTris[i][1]][2];
            point[2].x = linVert[linTris[i][2]][0];
            point[2].y = linVert[linTris[i][2]][1];
            value[2]   = linVert[linTris[i][2]][2];

            if (!Agros2D::problem()->configView()->scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < Agros2D::problem()->configView()->scalarRangeMin || avgValue > Agros2D::problem()->configView()->scalarRangeMax)
                    continue;
            }

            double delta = 0.0;

            if (Agros2D::problem()->configView()->scalarView3DLighting)
            {
                computeNormal(point[0].x, point[0].y, - delta - (value[0] - Agros2D::problem()->configView()->scalarRangeMin),
                        point[1].x, point[1].y, - delta - (value[1] - Agros2D::problem()->configView()->scalarRangeMin),
                        point[2].x, point[2].y, - delta - (value[2] - Agros2D::problem()->configView()->scalarRangeMin),
                        normal);

                glNormal3d(normal[0], normal[1], normal[2]);
            }
            for (int j = 0; j < 3; j++)
            {
                glTexCoord1d((value[j] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                glVertex3d(point[j].x, point[j].y, - delta - (value[j] - Agros2D::problem()->configView()->scalarRangeMin));
            }
        }
        glEnd();

        // remove normal
        delete [] normal;

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glDisable(GL_TEXTURE_1D);
        glDisable(GL_LIGHTING);

        // draw blended mesh
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4d(0.5, 0.5, 0.5, 0.3);

        m_postHermes->linInitialMeshView().lock_data();

        double3* linVertMesh = m_postHermes->linInitialMeshView().get_vertices();
        int3* linTrisMesh = m_postHermes->linInitialMeshView().get_triangles();

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postHermes->linInitialMeshView().get_num_triangles(); i++)
        {
            glVertex2d(linVertMesh[linTrisMesh[i][0]][0], linVertMesh[linTrisMesh[i][0]][1]);
            glVertex2d(linVertMesh[linTrisMesh[i][1]][0], linVertMesh[linTrisMesh[i][1]][1]);
            glVertex2d(linVertMesh[linTrisMesh[i][2]][0], linVertMesh[linTrisMesh[i][2]][1]);
        }
        glEnd();

        m_postHermes->linInitialMeshView().unlock_data();

        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_OFFSET_FILL);

        // bounding box
        if (Agros2D::problem()->configView()->scalarView3DBoundingBox)
        {
            double borderXY = max * 0.05;
            double borderZ = (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) * 0.05;

            glBegin(GL_LINES);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, borderZ);

            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);

            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, - (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin) - borderZ);
            glEnd();
        }

        // geometry - edges
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {

            glColor3d(Agros2D::problem()->configView()->colorEdges.redF(),
                      Agros2D::problem()->configView()->colorEdges.greenF(),
                      Agros2D::problem()->configView()->colorEdges.blueF());
            glLineWidth(Agros2D::problem()->configView()->edgeWidth);

            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, 0.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, 0.0);
                glEnd();
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                drawArc(center, radius, startAngle, edge->angle());
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

        m_postHermes->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField3D);
    }
    else
    {
        glCallList(m_listScalarField3D);
    }
}

void SceneViewPost3D::paintScalarField3DSolid()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3DSolid && !m_postHermes->scalarIsPrepared()) return;

    loadProjection3d(true, Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3D);

    if (m_listScalarField3DSolid == -1)
    {
        paletteCreate();

        m_listScalarField3DSolid = glGenLists(1);
        glNewList(m_listScalarField3DSolid, GL_COMPILE);

        bool isModel = (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_Model);

        // gradient background
        paintBackground();
        glEnable(GL_DEPTH_TEST);

        RectPoint rect = Agros2D::scene()->boundingBox();
        double max = qMax(rect.width(), rect.height());
        double depth = max / Agros2D::problem()->configView()->scalarView3DHeight;

        // range
        double irange = 1.0 / (Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin);
        // special case: constant solution
        if (fabs(Agros2D::problem()->configView()->scalarRangeMin - Agros2D::problem()->configView()->scalarRangeMax) < EPS_ZERO)
        {
            irange = 1.0;
        }

        double phi = Agros2D::problem()->configView()->scalarView3DAngle;

        m_postHermes->linScalarView().lock_data();

        double3* linVert = m_postHermes->linScalarView().get_vertices();
        int3* linTris = m_postHermes->linScalarView().get_triangles();
        int* linTrisMarkers = m_postHermes->linScalarView().get_triangle_markers();
        int2* linEdges = m_postHermes->linScalarView().get_edges();
        int* linEdgesMarkers = m_postHermes->linScalarView().get_edge_markers();

        QVector<int> linTrisBoundaries;
        for (int edge = 0; edge < m_postHermes->linScalarView().get_num_edges(); edge++)
        {
            if (linEdgesMarkers[edge] > 0)
            {
                if (!linTrisBoundaries.contains(linEdges[edge][0]))
                    linTrisBoundaries.append(linEdges[edge][0]);
                if (!linTrisBoundaries.contains(linEdges[edge][1]))
                    linTrisBoundaries.append(linEdges[edge][1]);
            }
        }

        Point point[3];
        double value[3];

        glPushMatrix();

        // set texture for coloring
        if (!isModel)
        {
            glEnable(GL_TEXTURE_1D);
            glBindTexture(GL_TEXTURE_1D, m_textureScalar);
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

        if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
        {
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    point[j].x = linVert[linTris[i][j]][0];
                    point[j].y = linVert[linTris[i][j]][1];
                    value[j]   = linVert[linTris[i][j]][2];
                }

                // find marker
                SceneLabel *label = Agros2D::scene()->labels->at(atoi(postHermes()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(linTrisMarkers[i]).marker.c_str()));
                SceneMaterial *material = label->marker(postHermes()->activeViewField());

                // hide material
                if (Agros2D::problem()->configView()->solidViewHide.contains(material->name()))
                    continue;

                if (!Agros2D::problem()->configView()->scalarRangeAuto)
                {
                    double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                    if (avgValue < Agros2D::problem()->configView()->scalarRangeMin || avgValue > Agros2D::problem()->configView()->scalarRangeMax)
                        continue;
                }

                // z = - depth / 2.0
                if (Agros2D::problem()->configView()->scalarView3DLighting || isModel)
                {
                    computeNormal(point[0].x, point[0].y, -depth/2.0,
                            point[1].x, point[1].y, -depth/2.0,
                            point[2].x, point[2].y, -depth/2.0,
                            normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                for (int j = 0; j < 3; j++)
                {
                    if (!isModel) glTexCoord1d((value[j] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                    glVertex3d(point[j].x, point[j].y, -depth/2.0);
                }

                // z = + depth / 2.0
                if (Agros2D::problem()->configView()->scalarView3DLighting || isModel)
                {
                    computeNormal(point[0].x, point[0].y, depth/2.0,
                            point[1].x, point[1].y, depth/2.0,
                            point[2].x, point[2].y, depth/2.0,
                            normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                for (int j = 0; j < 3; j++)
                {
                    if (!isModel) glTexCoord1d((value[j] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                    glVertex3d(point[j].x, point[j].y, depth/2.0);
                }
            }
            glEnd();

            glBegin(GL_QUADS);
            for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
            {
                // boundary element
                if ((linTrisBoundaries.contains(linTris[i][0]) || linTrisBoundaries.contains(linTris[i][1]) || linTrisBoundaries.contains(linTris[i][2])))
                {
                    for (int j = 0; j < 3; j++)
                    {
                        point[j].x = linVert[linTris[i][j]][0];
                        point[j].y = linVert[linTris[i][j]][1];
                        value[j]   = linVert[linTris[i][j]][2];
                    }

                    // find marker
                    SceneLabel *label = Agros2D::scene()->labels->at(atoi(postHermes()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(linTrisMarkers[i]).marker.c_str()));
                    SceneMaterial *material = label->marker(postHermes()->activeViewField());

                    // hide material
                    if (Agros2D::problem()->configView()->solidViewHide.contains(material->name()))
                        continue;

                    // length
                    for (int k = 0; k < 3; k++)
                    {
                        if (Agros2D::problem()->configView()->scalarView3DLighting || isModel)
                        {
                            computeNormal(point[k].x, point[k].y, -depth/2.0,
                                          point[(k + 1) % 3].x, point[(k + 1) % 3].y, -depth/2.0,
                                    point[(k + 1) % 3].x, point[(k + 1) % 3].y,  depth/2.0,
                                    normal);
                            glNormal3d(normal[0], normal[1], normal[2]);
                        }

                        if (!isModel) glTexCoord1d((value[k] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                        glVertex3d(point[k].x, point[k].y, -depth/2.0);
                        if (!isModel) glTexCoord1d((value[(k + 1) % 3] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                        glVertex3d(point[(k + 1) % 3].x, point[(k + 1) % 3].y, -depth/2.0);

                        if (!isModel) glTexCoord1d((value[(k + 1) % 3] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                        glVertex3d(point[(k + 1) % 3].x, point[(k + 1) % 3].y, depth/2.0);
                        if (!isModel) glTexCoord1d((value[k] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                        glVertex3d(point[k].x, point[k].y, depth/2.0);
                    }
                }
            }
            glEnd();
        }
        else
        {
            // side
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    point[j].x = linVert[linTris[i][j]][0];
                    point[j].y = linVert[linTris[i][j]][1];
                    value[j]   = linVert[linTris[i][j]][2];
                }

                // find marker
                SceneLabel *label = Agros2D::scene()->labels->at(atoi(postHermes()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(linTrisMarkers[i]).marker.c_str()));
                SceneMaterial *material = label->marker(postHermes()->activeViewField());

                // hide material
                if (Agros2D::problem()->configView()->solidViewHide.contains(material->name()))
                    continue;

                if (!Agros2D::problem()->configView()->scalarRangeAuto)
                {
                    double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                    if (avgValue < Agros2D::problem()->configView()->scalarRangeMin || avgValue > Agros2D::problem()->configView()->scalarRangeMax)
                        continue;
                }

                for (int j = 0; j < 2; j++)
                {
                    if (Agros2D::problem()->configView()->scalarView3DLighting || isModel)
                    {
                        computeNormal(point[0].x * cos(j*phi/180.0*M_PI), point[0].y, point[0].x * sin(j*phi/180.0*M_PI),
                                point[1].x * cos(j*phi/180.0*M_PI), point[1].y, point[1].x * sin(j*phi/180.0*M_PI),
                                point[2].x * cos(j*phi/180.0*M_PI), point[2].y, point[2].x * sin(j*phi/180.0*M_PI),
                                normal);
                        glNormal3d(normal[0], normal[1], normal[2]);
                    }

                    glTexCoord1d((value[0] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                    glVertex3d(point[0].x * cos(j*phi/180.0*M_PI), point[0].y, point[0].x * sin(j*phi/180.0*M_PI));
                    glTexCoord1d((value[1] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                    glVertex3d(point[1].x * cos(j*phi/180.0*M_PI), point[1].y, point[1].x * sin(j*phi/180.0*M_PI));
                    glTexCoord1d((value[2] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                    glVertex3d(point[2].x * cos(j*phi/180.0*M_PI), point[2].y, point[2].x * sin(j*phi/180.0*M_PI));
                }
            }
            glEnd();

            for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
            {
                // boundary element
                if ((linTrisBoundaries.contains(linTris[i][0]) || linTrisBoundaries.contains(linTris[i][1]) || linTrisBoundaries.contains(linTris[i][2])))
                {
                    for (int j = 0; j < 3; j++)
                    {
                        point[j].x = linVert[linTris[i][j]][0];
                        point[j].y = linVert[linTris[i][j]][1];
                        value[j]   = linVert[linTris[i][j]][2];
                    }

                    // find marker
                    SceneLabel *label = Agros2D::scene()->labels->at(atoi(postHermes()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(linTrisMarkers[i]).marker.c_str()));
                    SceneMaterial *material = label->marker(postHermes()->activeViewField());

                    // hide material
                    if (Agros2D::problem()->configView()->solidViewHide.contains(material->name()))
                        continue;

                    if (!Agros2D::problem()->configView()->scalarRangeAuto)
                    {
                        double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                        if (avgValue < Agros2D::problem()->configView()->scalarRangeMin || avgValue > Agros2D::problem()->configView()->scalarRangeMax)
                            continue;
                    }

                    // sides
                    int count = 29.0 * phi / 360.0;
                    double step = phi/count;
                    for (int k = 0; k < 3; k++)
                    {
                        glBegin(GL_TRIANGLE_STRIP);
                        for (int j = 0; j < count + 1; j++)
                        {

                            if (Agros2D::problem()->configView()->scalarView3DLighting || isModel)
                            {
                                computeNormal(point[k].x * cos((j+0)*step/180.0*M_PI), point[k].y, point[k].x * sin((j+0)*step/180.0*M_PI),
                                              point[(k + 1) % 3].x * cos((j+0)*step/180.0*M_PI), point[(k + 1) % 3].y, point[(k + 1) % 3].x * sin((j+0)*step/180.0*M_PI),
                                        point[(k + 1) % 3].x * cos((j+1)*step/180.0*M_PI), point[(k + 1) % 3].y, point[(k + 1) % 3].x * sin((j+1)*step/180.0*M_PI),
                                        normal);
                                glNormal3d(normal[0], normal[1], normal[2]);
                            }

                            if (!isModel) glTexCoord1d((value[k] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                            glVertex3d(point[k].x * cos((j+0)*step/180.0*M_PI), point[k].y, point[k].x * sin((j+0)*step/180.0*M_PI));
                            if (!isModel) glTexCoord1d((value[(k + 1) % 3] - Agros2D::problem()->configView()->scalarRangeMin) * irange);
                            glVertex3d(point[(k + 1) % 3].x * cos((j+0)*step/180.0*M_PI), point[(k + 1) % 3].y, point[(k + 1) % 3].x * sin((j+0)*step/180.0*M_PI));
                        }
                        glEnd();
                    }
                }
            }
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
        if (Agros2D::problem()->configView()->scalarView3DSolidGeometry)
        {
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glColor3d(0.1, 0.1, 0.1);
            glLineWidth(2.0);

            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
            {
                // top and bottom
                foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
                {
                    glBegin(GL_LINES);
                    if (edge->isStraight())
                    {
                        glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                        glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);

                        glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);
                        glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
                    }
                    else
                    {
                        Point center = edge->center();
                        double radius = edge->radius();
                        double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                                  center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                        int segments = edge->angle() / 5.0;
                        if (segments < 2) segments = 2;

                        double theta = edge->angle() / double(segments);

                        for (int i = 0; i < segments; i++)
                        {
                            double arc1 = (startAngle + i*theta)/180.0*M_PI;
                            double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                            double x1 = radius * cos(arc1);
                            double y1 = radius * sin(arc1);
                            double x2 = radius * cos(arc2);
                            double y2 = radius * sin(arc2);

                            glVertex3d(center.x + x1, center.y + y1, depth/2.0);
                            glVertex3d(center.x + x2, center.y + y2, depth/2.0);

                            glVertex3d(center.x + x1, center.y + y1, -depth/2.0);
                            glVertex3d(center.x + x2, center.y + y2, -depth/2.0);
                        }
                    }
                    glEnd();
                }

                // side
                glBegin(GL_LINES);
                foreach (SceneNode *node, Agros2D::scene()->nodes->items())
                {
                    glVertex3d(node->point().x, node->point().y,  depth/2.0);
                    glVertex3d(node->point().x, node->point().y, -depth/2.0);
                }
                glEnd();

                glLineWidth(1.0);
            }
            else
            {
                // top
                foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
                {
                    for (int j = 0; j < 2; j++)
                    {
                        if (edge->isStraight())
                        {
                            glBegin(GL_LINES);
                            glVertex3d(edge->nodeStart()->point().x * cos(j*phi/180.0*M_PI), edge->nodeStart()->point().y, edge->nodeStart()->point().x * sin(j*phi/180.0*M_PI));
                            glVertex3d(edge->nodeEnd()->point().x * cos(j*phi/180.0*M_PI), edge->nodeEnd()->point().y, edge->nodeEnd()->point().x * sin(j*phi/180.0*M_PI));
                            glEnd();
                        }
                        else
                        {
                            Point center = edge->center();
                            double radius = edge->radius();
                            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                            double theta = edge->angle() / double(edge->angle()/2 - 1);

                            glBegin(GL_LINE_STRIP);
                            for (int i = 0; i < edge->angle()/2; i++)
                            {
                                double arc = (startAngle + i*theta)/180.0*M_PI;

                                double x = radius * cos(arc);
                                double y = radius * sin(arc);

                                glVertex3d((center.x + x) * cos(j*phi/180.0*M_PI),
                                           center.y + y,
                                           (center.x + x) * sin(j*phi/180.0*M_PI));
                            }
                            glEnd();
                        }
                    }
                }

                // side
                foreach (SceneNode *node, Agros2D::scene()->nodes->items())
                {
                    int count = 29.0 * phi / 360.0;
                    double step = phi/count;

                    glBegin(GL_LINE_STRIP);
                    for (int j = 0; j < count; j++)
                    {
                        glVertex3d(node->point().x * cos((j+0)*step/180.0*M_PI),
                                   node->point().y,
                                   node->point().x * sin((j+0)*step/180.0*M_PI));
                        glVertex3d(node->point().x * cos((j+1)*step/180.0*M_PI),
                                   node->point().y,
                                   node->point().x * sin((j+1)*step/180.0*M_PI));
                    }
                    glEnd();
                }

                glLineWidth(1.0);
            }

            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }

        glDisable(GL_DEPTH_TEST);

        glPopMatrix();

        m_postHermes->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField3DSolid);
    }
    else
    {
        glCallList(m_listScalarField3DSolid);
    }
}

void SceneViewPost3D::clearGLLists()
{
    if (m_listScalarField3D != -1) glDeleteLists(m_listScalarField3D, 1);
    if (m_listScalarField3DSolid != -1) glDeleteLists(m_listScalarField3DSolid, 1);
    if (m_listModel != -1) glDeleteLists(m_listModel, 1);

    m_listScalarField3D = -1;
    m_listScalarField3DSolid = -1;
    m_listModel = -1;
}

void SceneViewPost3D::refresh()
{
    clearGLLists();

    setControls();

    SceneViewCommon::refresh();
}

void SceneViewPost3D::setControls()
{
    // actions
    actSceneModePost3D->setEnabled(Agros2D::problem()->isSolved());
    actSetProjectionXY->setEnabled(Agros2D::problem()->isSolved());
    actSetProjectionXZ->setEnabled(Agros2D::problem()->isSolved());
    actSetProjectionYZ->setEnabled(Agros2D::problem()->isSolved());
}

void SceneViewPost3D::clear()
{
    SceneViewCommon3D::clear();
}
