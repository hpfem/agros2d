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

#include "sceneview_common3d.h"
#include "sceneview_data.h"
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

SceneViewCommon3D::SceneViewCommon3D(QWidget *parent): SceneViewPostInterface(parent)
{
    createActions();
}

void SceneViewCommon3D::createActions()
{
    // projection
    actSetProjectionXY = new QAction(tr("Projection to %1%2").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelY()), this);
    actSetProjectionXY->setStatusTip(tr("Projection to %1%2 plane.").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelY()));
    connect(actSetProjectionXY, SIGNAL(triggered()), this, SLOT(doSetProjectionXY()));

    actSetProjectionXZ = new QAction(tr("Projection to %1%2").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelZ()), this);
    actSetProjectionXZ->setStatusTip(tr("Projection to %1%2 plane.").arg(Util::scene()->problemInfo()->labelX()).arg(Util::scene()->problemInfo()->labelZ()));
    connect(actSetProjectionXZ, SIGNAL(triggered()), this, SLOT(doSetProjectionXZ()));

    actSetProjectionYZ = new QAction(tr("Projection to %1%2").arg(Util::scene()->problemInfo()->labelY()).arg(Util::scene()->problemInfo()->labelZ()), this);
    actSetProjectionYZ->setStatusTip(tr("Projection to %1%2 plane.").arg(Util::scene()->problemInfo()->labelY()).arg(Util::scene()->problemInfo()->labelZ()));
    connect(actSetProjectionYZ, SIGNAL(triggered()), this, SLOT(doSetProjectionYZ()));
}

SceneViewCommon3D::~SceneViewCommon3D()
{
}

void SceneViewCommon3D::clear()
{
    logMessage("SceneViewCommon::doDefaultValues()");

    // 3d
    m_scale3d = 0.6;
    m_offset3d = Point();
    m_rotation3d = Point3();
    m_rotation3d.x = 66.0;
    m_rotation3d.y = -35.0;
    m_rotation3d.z = 0.0;

    doInvalidated();
    doZoomBestFit();

    SceneViewCommon::clear();
}

void SceneViewCommon3D::paintBackground()
{
    logMessage("SceneViewCommon::paintBackground()");

    // background
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, -10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBegin(GL_QUADS);
    if (Util::config()->scalarView3DBackground)
        glColor3d(0.99, 0.99, 0.99);
    else
        glColor3d(Util::config()->colorBackground.redF(),
                  Util::config()->colorBackground.greenF(),
                  Util::config()->colorBackground.blueF());
    glVertex3d(-1.0, -1.0, 0.0);
    glVertex3d(1.0, -1.0, 0.0);
    if (Util::config()->scalarView3DBackground)
        glColor3d(0.44, 0.56, 0.89);
    glVertex3d(1.0, 1.0, 0.0);
    glVertex3d(-1.0, 1.0, 0.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    glPopMatrix();
}

void SceneViewCommon3D::loadProjection3d(bool setScene)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 0.5, -0.5, 0.5, 4.0, 15.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (setScene)
    {
        // set max and min zoom
        if (m_scale3d < 1e-9) m_scale3d = 1e-9;
        if (m_scale3d > 1e6) m_scale3d = 1e6;

        glScaled(1.0/aspect(), 1.0, 0.1);

        // move to origin
        glTranslated(-m_offset3d.x, -m_offset3d.y, 1.0);

        glRotated(m_rotation3d.x, 1.0, 0.0, 0.0);
        glRotated(m_rotation3d.z, 0.0, 1.0, 0.0);
        glRotated(m_rotation3d.y, 0.0, 0.0, 1.0);

        RectPoint rect = Util::scene()->boundingBox();
        if (Util::config()->showPost3D == SceneViewPost3DShow_ScalarView3D)
        {
            glTranslated(- m_scale3d * (rect.start.x + rect.end.x) / 2.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
        }
        else
        {
            if (Util::scene()->problemInfo()->coordinateType == CoordinateType_Planar)
            {
                glTranslated(- m_scale3d * (rect.start.x + rect.end.x) / 2.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
            }
            else
            {
                glTranslated(0.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
            }
        }

        glScaled(m_scale3d, m_scale3d, m_scale3d);
    }
}

void SceneViewCommon3D::setZoom(double power)
{
    m_scale3d = m_scale3d * pow(1.2, power);

    updateGL();
}

void SceneViewCommon3D::initLighting()
{
    logMessage("SceneViewCommon::initLighting()");

    if (Util::config()->scalarView3DLighting || Util::config()->showPost3D == SceneViewPost3DShow_Model)
    {
        // environment
        float light_specular[] = {  1.0f, 1.0f, 1.0f, 1.0f };
        float light_ambient[]  = {  0.7f, 0.7f, 0.7f, 1.0f };
        float light_diffuse[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
        float light_position[] = {  1.0f, 0.0f, 1.0f, 0.0f };

        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        // material
        float material_ambient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
        float material_diffuse[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
        float material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

        // glEnable(GL_COLOR_MATERIAL);
#if defined(GL_LIGHT_MODEL_COLOR_CONTROL) && defined(GL_SEPARATE_SPECULAR_COLOR)
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
}

// events

void SceneViewCommon3D::keyPressEvent(QKeyEvent *event)
{
}

void SceneViewCommon3D::keyReleaseEvent(QKeyEvent *event)
{
    setToolTip("");

    emit mouseSceneModeChanged(MouseSceneMode_Nothing);
}

void SceneViewCommon3D::mousePressEvent(QMouseEvent *event)
{
}

void SceneViewCommon3D::mouseReleaseEvent(QMouseEvent *event)
{
}

void SceneViewCommon3D::mouseDoubleClickEvent(QMouseEvent * event)
{
}

void SceneViewCommon3D::mouseMoveEvent(QMouseEvent *event)
{
    logMessage("SceneViewCommon::mouseMoveEvent()");

    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    setToolTip("");

    // pan
    if ((event->buttons() & Qt::MidButton)
            || ((event->buttons() & Qt::LeftButton)
                && (((event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))))

    {
        setCursor(Qt::PointingHandCursor);

        m_offset3d.x -= 2.0/width() * dx*aspect();
        m_offset3d.y += 2.0/height() * dy;

        emit mouseSceneModeChanged(MouseSceneMode_Pan);

        updateGL();
    }

    // rotate
    if ((event->buttons() & Qt::LeftButton)
            && (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        m_rotation3d.x -= dy;
        m_rotation3d.y += dx;

        emit mouseSceneModeChanged(MouseSceneMode_Rotate);

        updateGL();
    }
    if ((event->buttons() & Qt::LeftButton)
            && (!(event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        m_rotation3d.z -= dy;

        emit mouseSceneModeChanged(MouseSceneMode_Rotate);

        updateGL();
    }
}

void SceneViewCommon3D::wheelEvent(QWheelEvent *event)
{
    logMessage("SceneViewCommon::wheelEvent()");

    setZoom(event->delta()/150.0);
}

void SceneViewCommon3D::doSetProjectionXY()
{
    m_rotation3d.x = m_rotation3d.y = m_rotation3d.z = 0.0;
    updateGL();
}

void SceneViewCommon3D::doSetProjectionXZ()
{
    m_rotation3d.y = m_rotation3d.z = 0.0;
    m_rotation3d.x = 90.0;
    updateGL();
}

void SceneViewCommon3D::doSetProjectionYZ()
{
    m_rotation3d.x = m_rotation3d.y = 90.0;
    m_rotation3d.z = 0.0;
    updateGL();
}
