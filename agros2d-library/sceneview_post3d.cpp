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
#include "util/constants.h"

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

SceneViewPost3D::SceneViewPost3D(PostDeal *postDeal, QWidget *parent)
    : SceneViewCommon3D(postDeal, parent),
      m_listScalarField3D(-1),
      m_listScalarField3DSolid(-1),
      m_listModel(-1)
{
    createActionsPost3D();

    connect(Agros2D::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postDeal, SIGNAL(processed()), this, SLOT(refresh()));

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

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // gradient background
    paintBackground();

    if (Agros2D::problem()->isMeshed())
    {
        if (((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_Model) paintScalarField3DSolid();
    }

    if (Agros2D::problem()->isSolved() && m_postDeal->isProcessed())
    {
        if (((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3D) paintScalarField3D();
        if (((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3DSolid) paintScalarField3DSolid();

        // bars
        if (((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3D ||
                ((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3DSolid)
            paintScalarFieldColorBar(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble(), Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble());
    }

    switch ((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt())
    {
    case SceneViewPost3DMode_ScalarView3D:
    case SceneViewPost3DMode_ScalarView3DSolid:
    {
        if (Agros2D::problem()->isSolved() && m_postDeal->isProcessed())
        {
            Module::LocalVariable localVariable = postDeal()->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString());
            QString text = Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.name();
            if ((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt() != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());

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

    if (Agros2D::configComputer()->value(Config::Config_ShowAxes).toBool()) paintAxes();
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

    loadProjection3d(true, ((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3D);

    if (m_listScalarField3D == -1)
    {
        if (m_postDeal->scalarValues().isEmpty()) return;

        paletteCreate();

        m_listScalarField3D = glGenLists(1);
        glNewList(m_listScalarField3D, GL_COMPILE);

        glPushMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPopMatrix();

        glEnable(GL_DEPTH_TEST);

        // range
        double irange = 1.0 / (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble());
        // special case: constant solution
        if (fabs(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble()) < EPS_ZERO)
        {
            irange = 1.0;
        }

        RectPoint rect = Agros2D::scene()->boundingBox();

        double max = qMax(rect.width(), rect.height());

        glPushMatrix();
        glScaled(1.0, 1.0, max / Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble() * fabs(irange));

        // scalar view
        initLighting();
        // init normal
        double *normal = new double[3];

        // set texture for coloring
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool())
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        else
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, m_textureScalar);

        // set texture transformation matrix
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(m_texShift, 0.0, 0.0);
        glScaled(m_texScale, 0.0, 0.0);

        glBegin(GL_TRIANGLES);
        foreach (PostTriangle triangle, m_postDeal->scalarValues())
        {
            if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
            {
                double avgValue = (triangle.values[0] + triangle.values[1] + triangle.values[2]) / 3.0;
                if (avgValue < Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() || avgValue > Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble())
                    continue;
            }

            double delta = 0.0;

            if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool())
            {
                computeNormal(triangle.vertices[0][0], triangle.vertices[0][1], - delta - (triangle.values[0] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()),
                        triangle.vertices[1][0], triangle.vertices[1][1], - delta - (triangle.values[1] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()),
                        triangle.vertices[2][0], triangle.vertices[2][1], - delta - (triangle.values[2] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()),
                        normal);

                glNormal3d(normal[0], normal[1], normal[2]);
            }
            for (int j = 0; j < 3; j++)
            {
                glTexCoord1d((triangle.values[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                glVertex3d(triangle.vertices[j][0], triangle.vertices[j][1], - delta - (triangle.values[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()));
            }
        }
        glEnd();

        // remove normal
        delete [] normal;

        glDisable(GL_TEXTURE_1D);
        glDisable(GL_LIGHTING);

        // draw blended mesh
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4d(0.5, 0.5, 0.5, 0.3);

        // triangles
        glBegin(GL_TRIANGLES);
        for (Hermes::Hermes2D::Views::Linearizer::Iterator<Hermes::Hermes2D::Views::ScalarLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::triangle_t>
             it = m_postDeal->linInitialMeshView()->triangles_begin(); !it.end; ++it)
        {
            Hermes::Hermes2D::Views::ScalarLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::triangle_t& triangle = it.get();

            glVertex2d(triangle[0][0], triangle[0][1]);
            glVertex2d(triangle[1][0], triangle[1][1]);
            glVertex2d(triangle[2][0], triangle[2][1]);
        }
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_OFFSET_FILL);

        // bounding box
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DBoundingBox).toBool())
        {
            double borderXY = max * 0.05;
            double borderZ = (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * 0.05;

            glBegin(GL_LINES);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, borderZ);

            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);

            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.start.y - borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.start.y - borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.end.x + borderXY, rect.end.y + borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, borderZ);
            glVertex3d(rect.start.x - borderXY, rect.end.y + borderXY, - (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) - borderZ);
            glEnd();
        }

        // geometry - edges
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {

            glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
            glLineWidth(EDGEWIDTH);

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

            glLineWidth(1.0);
        }

        glDisable(GL_DEPTH_TEST);

        // switch-off texture transform
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glPopMatrix();

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

    loadProjection3d(true, ((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3D);

    if (m_listScalarField3DSolid == -1)
    {
        if (m_postDeal->scalarValues().isEmpty()) return;

        paletteCreate();

        m_listScalarField3DSolid = glGenLists(1);
        glNewList(m_listScalarField3DSolid, GL_COMPILE);

        bool isModel = (((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_Model);

        glPushMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPopMatrix();

        glEnable(GL_DEPTH_TEST);

        RectPoint rect = Agros2D::scene()->boundingBox();
        double max = qMax(rect.width(), rect.height());
        double depth = max / Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble();

        // range
        double irange = 1.0 / (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble());
        // special case: constant solution
        if (fabs(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble()) < EPS_ZERO)
        {
            irange = 1.0;
        }

        double phi = Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DAngle).toDouble();

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
            foreach (PostTriangle triangle, m_postDeal->scalarValues())
            {
                // int& elem_marker = it.get_marker();

                // find marker
                // SceneLabel *label = Agros2D::scene()->labels->at(atoi(postDeal()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(elem_marker).marker.c_str()));
                SceneLabel *label = Agros2D::scene()->labels->at(0);
                SceneMaterial *material = label->marker(postDeal()->activeViewField());

                // hide material
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_SolidViewHide).toStringList().contains(material->name()))
                    continue;

                if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
                {
                    double avgValue = (triangle.values[0] + triangle.values[1] + triangle.values[2]) / 3.0;
                    if (avgValue < Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() || avgValue > Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble())
                        continue;
                }

                // z = - depth / 2.0
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool() || isModel)
                {
                    computeNormal(triangle.vertices[0][0], triangle.vertices[0][1], -depth/2.0,
                            triangle.vertices[1][0], triangle.vertices[1][1], -depth/2.0,
                            triangle.vertices[2][0], triangle.vertices[2][1], -depth/2.0,
                            normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                for (int j = 0; j < 3; j++)
                {
                    if (!isModel) glTexCoord1d((triangle.values[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                    glVertex3d(triangle.vertices[j][0], triangle.vertices[j][1], -depth/2.0);
                }

                // z = + depth / 2.0
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool() || isModel)
                {
                    computeNormal(triangle.vertices[0][0], triangle.vertices[0][1], depth/2.0,
                            triangle.vertices[1][0], triangle.vertices[1][1], depth/2.0,
                            triangle.vertices[2][0], triangle.vertices[2][1], depth/2.0,
                            normal);
                    glNormal3d(normal[0], normal[1], normal[2]);
                }

                for (int j = 0; j < 3; j++)
                {
                    if (!isModel) glTexCoord1d((triangle.values[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                    glVertex3d(triangle.vertices[j][0], triangle.vertices[j][1], depth/2.0);
                }
            }
            glEnd();

            glBegin(GL_QUADS);
            foreach (PostTriangle triangle, m_postDeal->scalarValues())
            {
                // boundary element
                // if ((linTrisBoundaries.contains(linTris[i][0]) || linTrisBoundaries.contains(linTris[i][1]) || linTrisBoundaries.contains(linTris[i][2])))
                {
                    // find marker
                    SceneLabel *label = Agros2D::scene()->labels->at(0); // Agros2D::scene()->labels->at(atoi(postDeal()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(linTrisMarkers[i]).marker.c_str()));
                    SceneMaterial *material = label->marker(postDeal()->activeViewField());

                    // hide material
                    if (Agros2D::problem()->setting()->value(ProblemSetting::View_SolidViewHide).toStringList().contains(material->name()))
                        continue;

                    // length
                    for (int k = 0; k < 3; k++)
                    {
                        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool() || isModel)
                        {
                            computeNormal(triangle.vertices[k][0], triangle.vertices[k][1], -depth/2.0,
                                    triangle.vertices[(k + 1) % 3][0], triangle.vertices[(k + 1) % 3][1], -depth/2.0,
                                    triangle.vertices[(k + 1) % 3][0], triangle.vertices[(k + 1) % 3][1],  depth/2.0,
                                    normal);
                            glNormal3d(normal[0], normal[1], normal[2]);
                        }

                        if (!isModel) glTexCoord1d((triangle.values[k] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                        glVertex3d(triangle.vertices[k][0], triangle.vertices[k][1], -depth/2.0);
                        if (!isModel) glTexCoord1d((triangle.values[(k + 1) % 3] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                        glVertex3d(triangle.vertices[(k + 1) % 3][0], triangle.vertices[(k + 1) % 3][1], -depth/2.0);

                        if (!isModel) glTexCoord1d((triangle.values[(k + 1) % 3] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                        glVertex3d(triangle.vertices[(k + 1) % 3][0], triangle.vertices[(k + 1) % 3][1], depth/2.0);
                        if (!isModel) glTexCoord1d((triangle.values[k] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                        glVertex3d(triangle.vertices[k][0], triangle.vertices[k][1], depth/2.0);
                    }
                }
            }
            glEnd();
        }
        else
        {
            // side
            glBegin(GL_TRIANGLES);
            foreach (PostTriangle triangle, m_postDeal->scalarValues())
            {
                // int& elem_marker = it.get_marker();
                int elem_marker = 0;

                // find marker
                SceneLabel *label = Agros2D::scene()->labels->at(atoi(postDeal()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(elem_marker).marker.c_str()));
                SceneMaterial *material = label->marker(postDeal()->activeViewField());

                // hide material
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_SolidViewHide).toStringList().contains(material->name()))
                    continue;

                if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
                {
                    double avgValue = (triangle.values[0] + triangle.values[1] + triangle.values[2]) / 3.0;
                    if (avgValue < Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() || avgValue > Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble())
                        continue;
                }

                for (int j = 0; j < 2; j++)
                {
                    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool() || isModel)
                    {
                        computeNormal(triangle.vertices[0][0] * cos(j*phi/180.0*M_PI), triangle.vertices[0][1], triangle.vertices[0][0] * sin(j*phi/180.0*M_PI),
                                triangle.vertices[1][0] * cos(j*phi/180.0*M_PI), triangle.vertices[1][1], triangle.vertices[1][0] * sin(j*phi/180.0*M_PI),
                                triangle.vertices[2][0] * cos(j*phi/180.0*M_PI), triangle.vertices[2][1], triangle.vertices[2][0] * sin(j*phi/180.0*M_PI),
                                normal);
                        glNormal3d(normal[0], normal[1], normal[2]);
                    }

                    glTexCoord1d((triangle.values[0] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                    glVertex3d(triangle.vertices[0][0] * cos(j*phi/180.0*M_PI), triangle.vertices[0][1], triangle.vertices[0][0] * sin(j*phi/180.0*M_PI));
                    glTexCoord1d((triangle.values[1] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                    glVertex3d(triangle.vertices[1][0] * cos(j*phi/180.0*M_PI), triangle.vertices[1][1], triangle.vertices[1][0] * sin(j*phi/180.0*M_PI));
                    glTexCoord1d((triangle.values[2] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                    glVertex3d(triangle.vertices[2][0] * cos(j*phi/180.0*M_PI), triangle.vertices[2][1], triangle.vertices[2][0] * sin(j*phi/180.0*M_PI));
                }
            }
            glEnd();

            foreach (PostTriangle triangle, m_postDeal->scalarValues())
            {
                // int& elem_marker = it.get_marker();
                int elem_marker = 0;

                // boundary element
                // if ((linTrisBoundaries.contains(linTris[i][0]) || linTrisBoundaries.contains(linTris[i][1]) || linTrisBoundaries.contains(linTris[i][2])))
                {
                    // find marker
                    SceneLabel *label = Agros2D::scene()->labels->at(atoi(postDeal()->activeViewField()->initialMesh()->get_element_markers_conversion().get_user_marker(elem_marker).marker.c_str()));
                    SceneMaterial *material = label->marker(postDeal()->activeViewField());

                    // hide material
                    if (Agros2D::problem()->setting()->value(ProblemSetting::View_SolidViewHide).toStringList().contains(material->name()))
                        continue;

                    if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
                    {
                        double avgValue = (triangle.values[0] + triangle.values[1] + triangle.values[2]) / 3.0;
                        if (avgValue < Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() || avgValue > Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble())
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

                            if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool() || isModel)
                            {
                                computeNormal(triangle.vertices[k][0] * cos((j+0)*step/180.0*M_PI), triangle.vertices[k][1], triangle.vertices[k][0] * sin((j+0)*step/180.0*M_PI),
                                        triangle.vertices[(k + 1) % 3][0] * cos((j+0)*step/180.0*M_PI), triangle.vertices[(k + 1) % 3][1], triangle.vertices[(k + 1) % 3][0] * sin((j+0)*step/180.0*M_PI),
                                        triangle.vertices[(k + 1) % 3][0] * cos((j+1)*step/180.0*M_PI), triangle.vertices[(k + 1) % 3][1], triangle.vertices[(k + 1) % 3][0] * sin((j+1)*step/180.0*M_PI),
                                        normal);
                                glNormal3d(normal[0], normal[1], normal[2]);
                            }

                            if (!isModel) glTexCoord1d((triangle.values[k] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                            glVertex3d(triangle.vertices[k][0] * cos((j+0)*step/180.0*M_PI),
                                    triangle.vertices[k][1],
                                    triangle.vertices[k][0] * sin((j+0)*step/180.0*M_PI));
                            if (!isModel) glTexCoord1d((triangle.values[(k + 1) % 3] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);
                            glVertex3d(triangle.vertices[(k + 1) % 3][0] * cos((j+0)*step/180.0*M_PI),
                                    triangle.vertices[(k + 1) % 3][1],
                                    triangle.vertices[(k + 1) % 3][0] * sin((j+0)*step/180.0*M_PI));
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
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DSolidGeometry).toBool())
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

    if (Agros2D::problem()->isSolved())
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
    if (Agros2D::problem()->isSolved())
        doZoomBestFit();
}
