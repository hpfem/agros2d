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

Point SceneViewCommon2D::position(const Point &point) const
{
    return Point((2.0/width()*point.x-1)/m_scale2d*aspect()+m_offset2d.x,
                 -(2.0/height()*point.y-1)/m_scale2d+m_offset2d.y);
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

SceneNode *SceneViewCommon2D::findClosestNode(const Point &point)
{
    SceneNode *nodeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneNode *node, Util::scene()->nodes->items())
    {
        double nodeDistance = node->distance(point);
        if (node->distance(point) < distance)
        {
            distance = nodeDistance;
            nodeClosest = node;
        }
    }

    return nodeClosest;
}

SceneEdge *SceneViewCommon2D::findClosestEdge(const Point &point)
{
    SceneEdge *edgeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        double edgeDistance = edge->distance(point);
        if (edge->distance(point) < distance)
        {
            distance = edgeDistance;
            edgeClosest = edge;
        }
    }

    return edgeClosest;
}

SceneLabel *SceneViewCommon2D::findClosestLabel(const Point &point)
{
    SceneLabel *labelClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        double labelDistance = label->distance(point);
        if (label->distance(point) < distance)
        {
            distance = labelDistance;
            labelClosest = label;
        }
    }

    return labelClosest;
}

void SceneViewCommon2D::paintGrid()
{
    loadProjection2d(true);

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(width(), height()));

    glDisable(GL_DEPTH_TEST);

    // heavy line
    int heavyLine = 5;

    glLineWidth(1.0);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x1C47);
    glBegin(GL_LINES);

    if ((((cornerMax.x-cornerMin.x)/Util::config()->gridStep + (cornerMin.y-cornerMax.y)/Util::config()->gridStep) < 200) &&
            ((cornerMax.x-cornerMin.x)/Util::config()->gridStep > 0) && ((cornerMin.y-cornerMax.y)/Util::config()->gridStep > 0))
    {
        // vertical lines
        for (int i = cornerMin.x/Util::config()->gridStep - 1; i < cornerMax.x/Util::config()->gridStep + 1; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(Util::config()->colorCross.redF(),
                          Util::config()->colorCross.greenF(),
                          Util::config()->colorCross.blueF());
            else
                glColor3d(Util::config()->colorGrid.redF(),
                          Util::config()->colorGrid.greenF(),
                          Util::config()->colorGrid.blueF());
            glVertex2d(i*Util::config()->gridStep, cornerMin.y);
            glVertex2d(i*Util::config()->gridStep, cornerMax.y);
        }

        // horizontal lines
        for (int i = cornerMax.y/Util::config()->gridStep - 1; i < cornerMin.y/Util::config()->gridStep + 1; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(Util::config()->colorCross.redF(),
                          Util::config()->colorCross.greenF(),
                          Util::config()->colorCross.blueF());
            else
                glColor3d(Util::config()->colorGrid.redF(),
                          Util::config()->colorGrid.greenF(),
                          Util::config()->colorGrid.blueF());
            glVertex2d(cornerMin.x, i*Util::config()->gridStep);
            glVertex2d(cornerMax.x, i*Util::config()->gridStep);
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    if (Util::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
    {
        drawBlend(cornerMin,
                  Point(0, cornerMax.y),
                  Util::config()->colorGrid.redF(),
                  Util::config()->colorGrid.greenF(),
                  Util::config()->colorGrid.blueF(), 0.25);
    }

    // axes
    glColor3d(Util::config()->colorCross.redF(),
              Util::config()->colorCross.greenF(),
              Util::config()->colorCross.blueF());
    glLineWidth(1.5);
    glBegin(GL_LINES);
    // y axis
    glVertex2d(0, cornerMin.y);
    glVertex2d(0, cornerMax.y);
    // x axis
    glVertex2d(((Util::problem()->config()->coordinateType() == CoordinateType_Axisymmetric) ? 0 : cornerMin.x), 0);
    glVertex2d(cornerMax.x, 0);
    glEnd();
}

void SceneViewCommon2D::paintAxes()
{
    loadProjection2d();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glColor3d(Util::config()->colorCross.redF(),
              Util::config()->colorCross.greenF(),
              Util::config()->colorCross.blueF());

    Point rulersArea = rulersAreaWidth();
    double rulersNumbers = rulersNumbersWidth();

    Point border = (Util::config()->showRulers) ? Point((rulersArea.x/4.0 + rulersNumbers/2.0)*m_scale2d/aspect()*width() + 20.0,
                                                        - (rulersArea.y/4.0)*m_scale2d*height() + 20.0)
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

    renderText(border.x + 38, height() - border.y + 2 + fontMetrics().height() / 3,
               Util::problem()->config()->labelX());

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

    renderText(border.x + 2 - fontMetrics().width(Util::problem()->config()->labelY()) / 2, height() - border.y - 38,
               Util::problem()->config()->labelY());

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneViewCommon2D::paintRulers()
{
    loadProjection2d(true);

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(width(), height()));

    double gridStep = Util::config()->gridStep;
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
        double rulersNumbers = rulersNumbersWidth();
        Point rulersArea = rulersAreaWidth();

        // area background
        drawBlend(Point(cornerMin.x, cornerMax.y - rulersArea.y),
                  Point(cornerMax.x, cornerMax.y), 0.95, 0.95, 0.95, 1.0);
        drawBlend(Point(cornerMin.x + rulersNumbers + rulersArea.x, cornerMax.y),
                  Point(cornerMin.x, cornerMin.y), 0.95, 0.95, 0.95, 1.0);

        // area lines
        glColor3d(0.5, 0.5, 0.5);
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, cornerMax.y - rulersArea.y);
        glVertex2d(cornerMax.x, cornerMax.y - rulersArea.y);
        glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, cornerMax.y - rulersArea.y);
        glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, cornerMin.y);
        glEnd();

        // lines
        glLineWidth(1.0);
        glBegin(GL_LINES);

        // horizontal ticks
        for (int i = cornerMin.x/gridStep - 1; i < cornerMax.x/gridStep + 1; i++)
        {
            if ((i*gridStep > cornerMin.x + rulersNumbers + rulersArea.x) && (i*gridStep < cornerMax.x))
            {
                if (i % heavyLine == 0)
                {
                    glVertex2d(i*gridStep, cornerMax.y - rulersArea.y);
                    glVertex2d(i*gridStep, cornerMax.y - rulersArea.y * 1.0/7.0);
                }
                else
                {
                    glVertex2d(i*gridStep, cornerMax.y - rulersArea.y);
                    glVertex2d(i*gridStep, cornerMax.y - rulersArea.y * 2.0/3.0);
                }
            }
        }

        // vertical ticks
        for (int i = cornerMax.y/gridStep - 1; i < cornerMin.y/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMax.y - rulersArea.y) || (i*gridStep > cornerMin.y))
                continue;

            if (i % heavyLine == 0)
            {
                glVertex2d(cornerMin.x + rulersArea.x * 1.0/7.0, i*gridStep);
                glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, i*gridStep);
            }
            else
            {
                glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x * 2.0/3.0, i*gridStep);
                glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, i*gridStep);
            }
        }
        glEnd();

        // zero axes
        glColor3d(Util::config()->colorCross.redF(),
                  Util::config()->colorCross.greenF(),
                  Util::config()->colorCross.blueF());

        glLineWidth(1.5);
        glBegin(GL_LINES);

        glVertex2d(0.0, cornerMax.y - rulersArea.y);
        glVertex2d(0.0, cornerMax.y - rulersArea.y * 1.0/7.0);

        glVertex2d(cornerMin.x + rulersArea.x * 1.0/7.0, 0.0);
        glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, 0.0);

        glEnd();

        QFont font = fontLabel();

        // horizontal labels
        QString textsizehor = QString::number(gridStep);
        double sizehor = 2.0/width()*(QFontMetrics(font).width(textsizehor) / 6.0)/m_scale2d*aspect();

        for (int i = cornerMin.x/gridStep - 1; i < cornerMax.x/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMin.x + rulersNumbers + rulersArea.x) || (i*gridStep > cornerMax.x))
                continue;

            if (i % heavyLine == 0)
            {
                QString text;
                if ((abs(gridStep) > 1e3 || abs(gridStep) < 1e-3) && i != 0)
                    text = QString::number(i*gridStep, 'e', 2);
                else
                    text = QString::number(i*gridStep, 'f', 6);
                renderTextPos(i*gridStep + sizehor, cornerMax.y, QString(text + "        ").left(9), false, font);
            }
        }

        // vertical labels
        double sizever = 2.0/width()*(QFontMetrics(font).height() * 7.0 / 6.0)/m_scale2d;

        for (int i = cornerMax.y/gridStep - 1; i < cornerMin.y/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMax.y - rulersArea.y) || (i*gridStep > cornerMin.y))
                continue;

            if (i % heavyLine == 0)
            {
                QString text;
                if ((abs(gridStep) > 1e3 || abs(gridStep) < 1e-3) && i != 0)
                    text = QString::number(i*gridStep, 'e', 2);
                else
                    text = QString::number(i*gridStep, 'f', 7);
                renderTextPos(cornerMin.x + rulersArea.x / 20.0, i*gridStep - sizever, QString(((i >= 0) ? " " : "") + text + "        ").left(9), false, font);
            }

        }
    }
}

void SceneViewCommon2D::paintRulersHints()
{
    loadProjection2d(true);

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(width(), height()));

    glColor3d(0.0, 0.53, 0.0);

    Point p = position(m_lastPos.x(), m_lastPos.y());
    Point rulersArea = rulersAreaWidth();
    double rulersNumbers = rulersNumbersWidth();

    // ticks
    glLineWidth(3.0);
    glBegin(GL_TRIANGLES);
    glVertex2d(p.x, cornerMax.y - rulersArea.y);
    glVertex2d(p.x + rulersArea.x * 2.0/7.0, cornerMax.y - rulersArea.y * 2.0/3.0);
    glVertex2d(p.x - rulersArea.x * 2.0/7.0, cornerMax.y - rulersArea.y * 2.0/3.0);

    glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x, p.y);
    glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x * 2.0/3.0, p.y + rulersArea.y * 2.0/7.0);
    glVertex2d(cornerMin.x + rulersNumbers + rulersArea.x * 2.0/3.0, p.y - rulersArea.y * 2.0/7.0);
    glEnd();
}

void SceneViewCommon2D::paintZoomRegion()
{
    loadProjection2d(true);

    // zoom region
    if (m_zoomRegion)
    {
        Point posStart = position(Point(m_zoomRegionPos.x(), m_zoomRegionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        drawBlend(posStart, posEnd,
                  Util::config()->colorHighlighted.redF(),
                  Util::config()->colorHighlighted.greenF(),
                  Util::config()->colorHighlighted.blueF());
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

    Point stepTemp = position(Point(width(), height()));
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
        Util::scene()->selectNone();
        emit mousePressed();
        refresh();
    }
        break;
    case Qt::Key_N:
    {
        // add node with coordinates under mouse pointer
        if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
        {
            Point p = position(Point(m_lastPos.x(), m_lastPos.y()));
            Util::scene()->doNewNode(p);
        }
    }
        break;
    case Qt::Key_L:
    {
        // add label with coordinates under mouse pointer
        if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
        {
            Point p = position(Point(m_lastPos.x(), m_lastPos.y()));
            Util::scene()->doNewLabel(p);
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

void SceneViewCommon2D::renderTextPos(double x, double y,
                                      const QString &str, bool blend, QFont fnt, bool horizontal)
{
    QFont fontLocal = font();
    if (fnt != QFont())
        fontLocal = fnt;

    Point size(2.0/width()*QFontMetrics(fontLocal).width(" ")/m_scale2d*aspect(),
               2.0/height()*QFontMetrics(fontLocal).height()/m_scale2d);

    if (blend)
    {

        double xs = x - size.x / 2.0;
        double ys = y - size.y * 1.15 / 3.2;
        double xe = xs + size.x * (str.size() + 1);
        double ye = ys + size.y * 1.15;

        drawBlend(Point(xs, ys), Point(xe, ye));
    }

    if (horizontal)
        renderText(x, y, 0.0, str, fontLocal);
    else
        for (int i = 0; i < str.length(); i++)
            renderText(x, y - i*size.y, 0.0, str[i], fontLocal);
}

// rulers
// m_rulersNumbersWidth = 0.0;
// m_rulersAreaWidth = Point();
QFont SceneViewCommon2D::fontLabel()
{
    QFont fontLabel = font();
    fontLabel.setPointSize(fontLabel.pointSize() - 1);

    return fontLabel;
}

Point SceneViewCommon2D::rulersAreaWidth()
{
    // labels
    QFont font = fontLabel();

    return Point((2.0/width()*font.pointSize()*2.0)/m_scale2d*aspect(),
                 -(2.0/height()*font.pointSize()*2.0)/m_scale2d);
}

double SceneViewCommon2D::rulersNumbersWidth()
{
    // labels
    QFont font = fontLabel();

    return (2.0/width()*QFontMetrics(font).width("MMMMMMMM"))/m_scale2d*aspect();
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

    Point rulersArea = rulersAreaWidth();
    double rulersNumbers = rulersNumbersWidth();

    m_offset2d.x = ((Util::config()->showRulers) ? start.x+end.x - rulersNumbers - rulersArea.x : start.x + end.x) / 2.0;
    m_offset2d.y = (start.y + end.y)/2.0;

    double sceneWidth = end.x-start.x;
    double sceneHeight = end.y-start.y;

    double maxScene = (((double) ((Util::config()->showRulers) ? width() - rulersNumbers - rulersArea.x :
                                                                 width()) / (double) height()) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale2d = 1.8/maxScene;

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
            Point posStart = position(Point(m_zoomRegionPos.x(), m_zoomRegionPos.y()));
            Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

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

    Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

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

    if (Util::config()->showRulers)
        updateGL();
}

void SceneViewCommon2D::wheelEvent(QWheelEvent *event)
{
    if (Util::config()->zoomToMouse)
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

