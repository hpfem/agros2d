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

#include "sceneview_common2d.h"

#include "util/global.h"

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

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "util/constants.h"

SceneViewCommon2D::SceneViewCommon2D(PostHermes *postHermes, QWidget *parent)
    : SceneViewPostInterface(postHermes, parent)
{
    connect(this, SIGNAL(mouseMoved(Point)), this, SLOT(updatePosition(Point)));
}

SceneViewCommon2D::~SceneViewCommon2D()
{
}

void SceneViewCommon2D::clear()
{
    m_zoomRegion = false;

    // 2d
    m_scale2d = 1.0;
    m_offset2d = Point();

    m_nodeLast = NULL;

    SceneViewCommon::clear();
}

void SceneViewCommon2D::updatePosition(const Point &point)
{
    emit labelRight(tr("Position: [%1; %2]").arg(point.x, 8, 'f', 5).arg(point.y, 8, 'f', 5));
}

Point SceneViewCommon2D::transform(const Point &point) const
{
    return Point((2.0 / width() * point.x - 1) / m_scale2d*aspect() + m_offset2d.x,
                 - (2.0 / height() * point.y - 1) / m_scale2d + m_offset2d.y);
}

Point SceneViewCommon2D::untransform(const Point &point) const
{
    return Point((1.0 + (point.x - m_offset2d.x) * m_scale2d/aspect()) * width() / 2.0,
                 (1.0 + (point.y - m_offset2d.y) * m_scale2d) * height() / 2.0);
}

void SceneViewCommon2D::loadProjection2d(bool setScene)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-0.5, 0.5, -0.5, 0.5, -10.0, -10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (setScene)
    {
        // set max and min zoom
        if (m_scale2d < 1e-9) m_scale2d = 1e-9;
        if (m_scale2d > 1e6) m_scale2d = 1e6;

        glScaled(m_scale2d/aspect(), m_scale2d, m_scale2d);

        glTranslated(-m_offset2d.x, -m_offset2d.y, 0.0);
    }
}

void SceneViewCommon2D::paintGrid()
{
    loadProjection2d(true);

    Point cornerMin = transform(Point(0, 0));
    Point cornerMax = transform(Point(width(), height()));

    glDisable(GL_DEPTH_TEST);

    // heavy line
    int heavyLine = 5;

    glLineWidth(1.0);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x1C47);
    glBegin(GL_LINES);

    if ((((cornerMax.x-cornerMin.x)/Agros2D::problem()->configView()->gridStep + (cornerMin.y-cornerMax.y)/Agros2D::problem()->configView()->gridStep) < 200) &&
            ((cornerMax.x-cornerMin.x)/Agros2D::problem()->configView()->gridStep > 0) && ((cornerMin.y-cornerMax.y)/Agros2D::problem()->configView()->gridStep > 0))
    {
        // vertical lines
        for (int i = cornerMin.x/Agros2D::problem()->configView()->gridStep - 1; i < cornerMax.x/Agros2D::problem()->configView()->gridStep + 1; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(Agros2D::problem()->configView()->colorCross.redF(),
                          Agros2D::problem()->configView()->colorCross.greenF(),
                          Agros2D::problem()->configView()->colorCross.blueF());
            else
                glColor3d(Agros2D::problem()->configView()->colorGrid.redF(),
                          Agros2D::problem()->configView()->colorGrid.greenF(),
                          Agros2D::problem()->configView()->colorGrid.blueF());
            glVertex2d(i*Agros2D::problem()->configView()->gridStep, cornerMin.y);
            glVertex2d(i*Agros2D::problem()->configView()->gridStep, cornerMax.y);
        }

        // horizontal lines
        for (int i = cornerMax.y/Agros2D::problem()->configView()->gridStep - 1; i < cornerMin.y/Agros2D::problem()->configView()->gridStep + 1; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(Agros2D::problem()->configView()->colorCross.redF(),
                          Agros2D::problem()->configView()->colorCross.greenF(),
                          Agros2D::problem()->configView()->colorCross.blueF());
            else
                glColor3d(Agros2D::problem()->configView()->colorGrid.redF(),
                          Agros2D::problem()->configView()->colorGrid.greenF(),
                          Agros2D::problem()->configView()->colorGrid.blueF());
            glVertex2d(cornerMin.x, i*Agros2D::problem()->configView()->gridStep);
            glVertex2d(cornerMax.x, i*Agros2D::problem()->configView()->gridStep);
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
    {
        drawBlend(cornerMin,
                  Point(0, cornerMax.y),
                  Agros2D::problem()->configView()->colorGrid.redF(),
                  Agros2D::problem()->configView()->colorGrid.greenF(),
                  Agros2D::problem()->configView()->colorGrid.blueF(), 0.25);
    }

    glColor3d(Agros2D::problem()->configView()->colorCross.redF(),
              Agros2D::problem()->configView()->colorCross.greenF(),
              Agros2D::problem()->configView()->colorCross.blueF());
    glLineWidth(1.5);
    glBegin(GL_LINES);
    // y axis
    glVertex2d(0, cornerMin.y);
    glVertex2d(0, cornerMax.y);
    // x axis
    glVertex2d(((Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric) ? 0 : cornerMin.x), 0);
    glVertex2d(cornerMax.x, 0);
    glEnd();
}

void SceneViewCommon2D::paintAxes()
{
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glColor3d(Agros2D::problem()->configView()->colorCross.redF(),
              Agros2D::problem()->configView()->colorCross.greenF(),
              Agros2D::problem()->configView()->colorCross.blueF());

    Point rulersArea = rulersAreaSize();
    Point border = (Agros2D::problem()->configView()->showRulers) ? Point(rulersArea.x + 10.0, rulersArea.y + 10.0)
                                                                  : Point(10.0, 10.0);

    // x-axis
    glBegin(GL_QUADS);
    glVertex2d(border.x, border.y);
    glVertex2d(border.x + 16, border.y);
    glVertex2d(border.x + 16, border.y + 2);
    glVertex2d(border.x, border.y + 2);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(border.x + 16, border.y - 4);
    glVertex2d(border.x + 16, border.y + 6);
    glVertex2d(border.x + 35, border.y + 1);
    glEnd();

    printRulersAt(border.x + 38, border.y + 1 - (m_charDataRulers[GLYPH_M].x1 - m_charDataRulers[GLYPH_M].x0) / 2.0, Agros2D::problem()->config()->labelX());

    // y-axis
    glBegin(GL_QUADS);
    glVertex2d(border.x, border.y);
    glVertex2d(border.x, border.y + 16);
    glVertex2d(border.x + 2, border.y + 16);
    glVertex2d(border.x + 2, border.y);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(border.x - 4, border.y + 16);
    glVertex2d(border.x + 6, border.y + 16);
    glVertex2d(border.x + 1, border.y + 35);
    glEnd();

    printRulersAt(border.x + 1 - (m_charDataRulers[GLYPH_M].x1 - m_charDataRulers[GLYPH_M].x0) / 2.0, border.y + 38, Agros2D::problem()->config()->labelY());

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneViewCommon2D::paintRulers()
{
    loadProjection2d(true);

    Point cornerMin = transform(Point(0, 0));
    Point cornerMax = transform(Point(width(), height()));

    double gridStep = Agros2D::problem()->configView()->gridStep;
    if (gridStep < EPS_ZERO)
        return;

    while (((cornerMax.x-cornerMin.x)/gridStep + (cornerMin.y-cornerMax.y)/gridStep) > 200)
        gridStep *= 2.0;
    while (((cornerMax.x-cornerMin.x)/gridStep + (cornerMin.y-cornerMax.y)/gridStep) < 80)
        gridStep /= 2.0;

    if (((cornerMax.x-cornerMin.x)/gridStep > 0) && ((cornerMin.y-cornerMax.y)/gridStep > 0))
    {
        int heavyLine = 5;

        // labels
        Point rulersAreaScreen = rulersAreaSize();
        Point rulersArea(2.0/width()*rulersAreaScreen.x/m_scale2d*aspect(),
                         2.0/height()*rulersAreaScreen.y/m_scale2d);

        double tickSize = rulersArea.y / 3.0;

        // area background
        drawBlend(Point(cornerMin.x, cornerMax.y + rulersArea.y),
                  Point(cornerMax.x, cornerMax.y), 0.95, 0.95, 0.95, 1.0);
        drawBlend(Point(cornerMin.x + rulersArea.x, cornerMax.y),
                  Point(cornerMin.x, cornerMin.y), 0.95, 0.95, 0.95, 1.0);

        // area lines
        glColor3d(0.5, 0.5, 0.5);
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMax.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMin.y);
        glEnd();

        // lines
        glLineWidth(1.0);
        glBegin(GL_LINES);

        // horizontal ticks
        for (int i = cornerMin.x/gridStep - 1; i < cornerMax.x/gridStep + 1; i++)
        {
            if ((i*gridStep > cornerMin.x + rulersArea.x) && (i*gridStep < cornerMax.x))
            {
                if (i % heavyLine == 0)
                {
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y);
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y - 2.0 * tickSize);
                }
                else
                {
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y);
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y - tickSize);
                }
            }
        }

        // vertical ticks
        for (int i = cornerMax.y/gridStep - 1; i < cornerMin.y/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMax.y + rulersArea.y) || (i*gridStep > cornerMin.y))
                continue;

            if (i % heavyLine == 0)
            {
                glVertex2d(cornerMin.x + rulersArea.x - 2.0 * tickSize, i*gridStep);
                glVertex2d(cornerMin.x + rulersArea.x, i*gridStep);
            }
            else
            {
                glVertex2d(cornerMin.x + rulersArea.x - tickSize, i*gridStep);
                glVertex2d(cornerMin.x + rulersArea.x, i*gridStep);
            }
        }
        glEnd();

        // zero axes
        glColor3d(Agros2D::problem()->configView()->colorCross.redF(),
                  Agros2D::problem()->configView()->colorCross.greenF(),
                  Agros2D::problem()->configView()->colorCross.blueF());

        glLineWidth(1.5);
        glBegin(GL_LINES);

        glVertex2d(0.0, cornerMax.y + rulersArea.y);
        glVertex2d(0.0, cornerMax.y + rulersArea.y - 2.0 * tickSize);

        glVertex2d(cornerMin.x + rulersArea.x - tickSize, 0.0);
        glVertex2d(cornerMin.x + rulersArea.x, 0.0);

        glEnd();

        // labels
        loadProjectionViewPort();

        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

        // horizontal labels
        for (int i = cornerMin.x/gridStep - 1; i < cornerMax.x/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMin.x + rulersArea.x) || (i*gridStep > cornerMax.x))
                continue;

            if (i % heavyLine == 0)
            {
                QString text;
                if ((abs(gridStep) > 1e3 || abs(gridStep) < 1e-3) && i != 0)
                    text = QString::number(i*gridStep, 'e', 2);
                else
                    text = QString::number(i*gridStep, 'f', 6);

                Point scr = untransform(i*gridStep, cornerMax.y);
                printRulersAt(scr.x + (m_charDataRulers[GLYPH_M].x1 - m_charDataRulers[GLYPH_M].x0) / 2.0,
                              scr.y + 2, QString(text + "        ").left(9));
            }
        }

        // vertical labels
        for (int i = cornerMax.y/gridStep - 1; i < cornerMin.y/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMax.y + rulersArea.y) || (i*gridStep > cornerMin.y))
                continue;

            if (i % heavyLine == 0)
            {
                QString text;
                if ((abs(gridStep) > 1e3 || abs(gridStep) < 1e-3) && i != 0)
                    text = QString::number(i*gridStep, 'e', 2);
                else
                    text = QString::number(i*gridStep, 'f', 7);

                Point scr = untransform(cornerMin.x + rulersArea.x / 20.0, i*gridStep);
                printRulersAt(scr.x, scr.y - 2 * (m_charDataRulers[GLYPH_M].y1 - m_charDataRulers[GLYPH_M].y0) * 1.1,
                              QString(((i >= 0) ? " " : "") + text + "        ").left(9));
            }
        }
    }
}

void SceneViewCommon2D::paintRulersHints()
{
    loadProjection2d(true);

    Point cornerMin = transform(Point(0, 0));
    Point cornerMax = transform(Point(width(), height()));

    glColor3d(0.0, 0.53, 0.0);

    Point p = transform(m_lastPos.x(), m_lastPos.y());
    Point rulersAreaScreen = rulersAreaSize();
    Point rulersArea(2.0/width()*rulersAreaScreen.x/m_scale2d*aspect(),
                     2.0/height()*rulersAreaScreen.y/m_scale2d);

    double tickSize = rulersArea.y / 3.0;

    // ticks
    glLineWidth(3.0);
    glBegin(GL_TRIANGLES);
    // horizontal
    glVertex2d(p.x, cornerMax.y + rulersArea.y);
    glVertex2d(p.x + tickSize / 2.0, cornerMax.y + rulersArea.y - tickSize);
    glVertex2d(p.x - tickSize / 2.0, cornerMax.y + rulersArea.y - tickSize);

    // vertical
    glVertex2d(cornerMin.x + rulersArea.x, p.y);
    glVertex2d(cornerMin.x + rulersArea.x - tickSize, p.y + tickSize / 2.0);
    glVertex2d(cornerMin.x + rulersArea.x - tickSize, p.y - tickSize / 2.0);
    glEnd();
}

void SceneViewCommon2D::paintZoomRegion()
{
    // zoom region
    if (m_zoomRegion)
    {
        loadProjection2d(true);

        Point posStart = transform(Point(m_zoomRegionPos.x(), m_zoomRegionPos.y()));
        Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

        drawBlend(posStart, posEnd,
                  Agros2D::problem()->configView()->colorHighlighted.redF(),
                  Agros2D::problem()->configView()->colorHighlighted.greenF(),
                  Agros2D::problem()->configView()->colorHighlighted.blueF());
    }
}

// events

void SceneViewCommon2D::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
        emit mouseSceneModeChanged(MouseSceneMode_Add);
    if (!(event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
        emit mouseSceneModeChanged(MouseSceneMode_Pan);
    if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
        emit mouseSceneModeChanged(MouseSceneMode_Move);

    Point stepTemp = transform(Point(width(), height()));
    stepTemp.x = stepTemp.x - m_offset2d.x;
    stepTemp.y = stepTemp.y - m_offset2d.y;
    double step = qMin(stepTemp.x, stepTemp.y) / 10.0;

    switch (event->key())
    {
    case Qt::Key_Up:
    {
        m_offset2d.y += step;
        refresh();
    }
        break;
    case Qt::Key_Down:
    {
        m_offset2d.y -= step;
        refresh();
    }
        break;
    case Qt::Key_Left:
    {
        m_offset2d.x -= step;
        refresh();
    }
        break;
    case Qt::Key_Right:
    {
        m_offset2d.x += step;
        refresh();
    }
        break;
    case Qt::Key_Plus:
    {
        doZoomIn();
    }
        break;
    case Qt::Key_Minus:
    {
        doZoomOut();
    }
        break;
    case Qt::Key_Escape:
    {
        m_nodeLast = NULL;
        Agros2D::scene()->selectNone();
        emit mousePressed();
        refresh();
    }
        break;
    case Qt::Key_N:
    {
        // add node with coordinates under mouse pointer
        if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
        {
            Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));
            Agros2D::scene()->doNewNode(p);
        }
    }
        break;
    case Qt::Key_L:
    {
        // add label with coordinates under mouse pointer
        if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
        {
            Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));
            Agros2D::scene()->doNewLabel(p);
        }
    }
        break;
    default:
        QGLWidget::keyPressEvent(event);
    }
}

void SceneViewCommon2D::keyReleaseEvent(QKeyEvent *event)
{
    setToolTip("");

    if (!(event->modifiers() & Qt::ControlModifier))
    {
        m_nodeLast = NULL;
        updateGL();
    }

    emit mouseSceneModeChanged(MouseSceneMode_Nothing);
}

// rulers
Point SceneViewCommon2D::rulersAreaSize()
{
    return Point((m_charDataRulers[GLYPH_M].x1 - m_charDataRulers[GLYPH_M].x0) * 11,
                 (m_charDataRulers[GLYPH_M].y1 - m_charDataRulers[GLYPH_M].y0) * 3);
}

void SceneViewCommon2D::setZoom(double power)
{
    m_scale2d = m_scale2d * pow(1.2, power);

    updateGL();
}

void SceneViewCommon2D::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO)
        return;

    Point rulersAreaScreen = rulersAreaSize();

    double sceneWidth = end.x - start.x;
    double sceneHeight = end.y - start.y;

    double w = (Agros2D::problem()->configView()->showRulers) ? width() - rulersAreaScreen.x : width();
    double h = (Agros2D::problem()->configView()->showRulers) ? height() - rulersAreaScreen.y : height();
    double maxScene = ((w / h) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale2d = 1.8/maxScene;

    Point rulersArea(2.0/width()*rulersAreaScreen.x/m_scale2d*aspect(),
                     2.0/height()*rulersAreaScreen.y/m_scale2d);

    m_offset2d.x = ((Agros2D::problem()->configView()->showRulers) ? start.x + end.x - rulersArea.x : start.x + end.x) / 2.0;
    m_offset2d.y = ((Agros2D::problem()->configView()->showRulers) ? start.y + end.y - rulersArea.y : start.y + end.y) / 2.0;

    setZoom(0);
}

void SceneViewCommon2D::mousePressEvent(QMouseEvent *event)
{
    // zoom region
    if ((event->button() & Qt::LeftButton)
            && !(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier))
    {
        // zoom region
        if (actSceneZoomRegion)
        {
            if (actSceneZoomRegion->isChecked())
            {
                m_zoomRegionPos = m_lastPos;
                actSceneZoomRegion->setChecked(false);
                actSceneZoomRegion->setData(true);
                m_zoomRegion = true;

                return;
            }
        }
    }
}

void SceneViewCommon2D::mouseDoubleClickEvent(QMouseEvent * event)
{

    if (!(event->modifiers() & Qt::ControlModifier))
    {
        // zoom best fit
        if ((event->buttons() & Qt::MidButton)
                || ((event->buttons() & Qt::LeftButton)
                    && ((!(event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)))))
        {
            doZoomBestFit();
            return;
        }
    }
}

void SceneViewCommon2D::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);

    // zoom region
    if (actSceneZoomRegion)
    {
        actSceneZoomRegion->setChecked(false);

        if (m_zoomRegion)
        {
            Point posStart = transform(Point(m_zoomRegionPos.x(), m_zoomRegionPos.y()));
            Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

            if (actSceneZoomRegion->data().value<bool>())
                doZoomRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));

            actSceneZoomRegion->setData(false);
        }
    }

    m_zoomRegion = false;
    updateGL();

    emit mouseSceneModeChanged(MouseSceneMode_Nothing);
}

void SceneViewCommon2D::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    setToolTip("");

    Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

    // zoom or select region
    if (m_zoomRegion)
        updateGL();

    // pan - middle button or shift + left mouse
    if ((event->buttons() & Qt::MidButton)
            || ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        m_offset2d.x -= 2.0/width() * dx/m_scale2d*aspect();
        m_offset2d.y += 2.0/height() * dy/m_scale2d;

        emit mouseSceneModeChanged(MouseSceneMode_Pan);

        updateGL();
    }

    emit mouseMoved(p);

    if (Agros2D::problem()->configView()->showRulers)
        updateGL();
}

void SceneViewCommon2D::wheelEvent(QWheelEvent *event)
{
    if (Agros2D::problem()->configView()->zoomToMouse)
    {
        Point posMouse;
        posMouse = Point((2.0/width()*(event->pos().x() - width()/2.0))/m_scale2d*aspect(),
                         -(2.0/height()*(event->pos().y() - height()/2.0))/m_scale2d);

        m_offset2d.x += posMouse.x;
        m_offset2d.y += posMouse.y;

        m_scale2d = m_scale2d * pow(1.2, event->delta()/150.0);

        posMouse = Point((2.0/width()*(event->pos().x() - width()/2.0))/m_scale2d*aspect(),
                         -(2.0/height()*(event->pos().y() - height()/2.0))/m_scale2d);

        m_offset2d.x -= posMouse.x;
        m_offset2d.y -= posMouse.y;

        updateGL();
    }
    else
    {
        setZoom(event->delta()/150.0);
    }
}

