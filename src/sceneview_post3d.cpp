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
#include "scene.h"
#include "scenesolution.h"
#include "hermes2d/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

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

SceneViewPost3D::SceneViewPost3D(QWidget *parent)
    : SceneViewCommon3D(parent)
{
    createActionsPost3D();

    connect(m_scene, SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(m_scene, SIGNAL(defaultValues()), this, SLOT(doDefaultValues()));
}

SceneViewPost3D::~SceneViewPost3D()
{
}

void SceneViewPost3D::createActionsPost3D()
{

}

void SceneViewPost3D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon3D::mousePressEvent(event);


}

void SceneViewPost3D::paintGL()
{
    logMessage("SceneViewCommon::paintGL()");

    glClearColor(Util::config()->colorBackground.redF(),
                 Util::config()->colorBackground.greenF(),
                 Util::config()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Util::problem()->isMeshed())
    {
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_Model) paintScalarField3DSolid();
    }

    if (Util::problem()->isSolved())
    {
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D) paintScalarField3D();
        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid) paintScalarField3DSolid();

        if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
                m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
            paintScalarFieldColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
    }

    if (Util::config()->showLabel)
    {
        switch (m_sceneViewSettings.postprocessorShow)
        {
        case SceneViewPostprocessorShow_ScalarView3D:
        case SceneViewPostprocessorShow_ScalarView3DSolid:
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
        case SceneViewPostprocessorShow_Model:
            paintSceneModeLabel(tr("Model"));
            break;
        case SceneViewPostprocessorShow_ParticleTracing:
            paintSceneModeLabel(tr("Particle tracing"));
            break;
        default:
            paintSceneModeLabel(tr("Postprocessor 3D"));
        }
    }
}

void SceneViewPost3D::resizeGL(int w, int h)
{
    SceneViewCommon::resizeGL(w, h);

    if (Util::problem()->isSolved())
    {
        paletteFilter();
        paletteUpdateTexAdjust();
        paletteCreate();
    }
}

void SceneViewPost3D::paintScalarField3D()
{
    logMessage("SceneViewCommon::paintScalarField3D()");

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

        m_scene->activeSceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->activeSceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->activeSceneSolution()->linScalarView().get_triangles();
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
        for (int i = 0; i < m_scene->activeSceneSolution()->linScalarView().get_num_triangles(); i++)
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

        m_scene->activeSceneSolution()->linInitialMeshView().lock_data();

        double3* linVertMesh = m_scene->activeSceneSolution()->linInitialMeshView().get_vertices();
        int3* linTrisMesh = m_scene->activeSceneSolution()->linInitialMeshView().get_triangles();

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->activeSceneSolution()->linInitialMeshView().get_num_triangles(); i++)
        {
            glVertex2d(linVertMesh[linTrisMesh[i][0]][0], linVertMesh[linTrisMesh[i][0]][1]);
            glVertex2d(linVertMesh[linTrisMesh[i][1]][0], linVertMesh[linTrisMesh[i][1]][1]);
            glVertex2d(linVertMesh[linTrisMesh[i][2]][0], linVertMesh[linTrisMesh[i][2]][1]);
        }
        glEnd();

        m_scene->activeSceneSolution()->linInitialMeshView().unlock_data();

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

        m_scene->activeSceneSolution()->linScalarView().unlock_data();

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
    logMessage("SceneViewCommon::paintScalarField3DSolid()");

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

        m_scene->activeSceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->activeSceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->activeSceneSolution()->linScalarView().get_triangles();
        int3* linEdges = m_scene->activeSceneSolution()->linScalarView().get_edges();
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
            for (int i = 0; i < m_scene->activeSceneSolution()->linScalarView().get_num_edges(); i++)
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
            for (int i = 0; i < m_scene->activeSceneSolution()->linScalarView().get_num_edges(); i++)
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

        m_scene->activeSceneSolution()->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField3DSolid);
    }
    else
    {
        glCallList(m_listScalarField3DSolid);
    }
}

void SceneViewPost3D::doDefaultValues()
{
    SceneViewCommon3D::doDefaultValues();
}
