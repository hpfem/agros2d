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

#include "sceneview_geometry.h"
#include "util.h"
#include "scene.h"
#include "scenesolution.h"
#include "hermes2d/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenebasicselectdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

#include "../lib/gl2ps/gl2ps.h"

SceneViewGeometry::SceneViewGeometry(QWidget *parent): SceneViewCommon2D(parent)
{
    createActionsGeometry();
    createMenuGeometry();
}

SceneViewGeometry::~SceneViewGeometry()
{
}

void SceneViewGeometry::createActionsGeometry()
{
    actSceneModeGeometry = new QAction(iconView(), tr("Geometry"), this);
    actSceneModeGeometry->setShortcut(Qt::Key_F5);
    actSceneModeGeometry->setStatusTip(tr("Geometry editor"));
    actSceneModeGeometry->setCheckable(true);

    // scene - operate on items
    actOperateOnNodes = new QAction(icon("scene-node"), tr("Operate on &nodes"), this);
    actOperateOnNodes->setShortcut(Qt::Key_F2);
    actOperateOnNodes->setStatusTip(tr("Operate on nodes"));
    actOperateOnNodes->setCheckable(true);

    actOperateOnEdges = new QAction(icon("scene-edge"), tr("Operate on &edges"), this);
    actOperateOnEdges->setShortcut(Qt::Key_F3);
    actOperateOnEdges->setStatusTip(tr("Operate on edges"));
    actOperateOnEdges->setCheckable(true);

    actOperateOnLabels = new QAction(icon("scene-label"), tr("Operate on &labels"), this);
    actOperateOnLabels->setShortcut(Qt::Key_F4);
    actOperateOnLabels->setStatusTip(tr("Operate on labels"));
    actOperateOnLabels->setCheckable(true);

    actSceneModeGeometryGroup = new QActionGroup(this);
    actSceneModeGeometryGroup->addAction(actOperateOnNodes);
    actSceneModeGeometryGroup->addAction(actOperateOnEdges);
    actSceneModeGeometryGroup->addAction(actOperateOnLabels);
    connect(actSceneModeGeometryGroup, SIGNAL(triggered(QAction *)), this, SLOT(doSceneGeometryModeSet(QAction *)));

    // select region
    actSceneViewSelectRegion = new QAction(icon("scene-select-region"), tr("&Select region"), this);
    actSceneViewSelectRegion->setStatusTip(tr("Select region"));
    actSceneViewSelectRegion->setCheckable(true);

    // object properties
    actSceneObjectProperties = new QAction(icon("scene-properties"), tr("Object properties"), this);
    actSceneObjectProperties->setShortcut(Qt::Key_Space);
    connect(actSceneObjectProperties, SIGNAL(triggered()), this, SLOT(doSceneObjectProperties()));
}

void SceneViewGeometry::createMenuGeometry()
{
    logMessage("SceneViewCommon::createMenu()");

    mnuScene = new QMenu(this);

    mnuScene->addAction(Util::scene()->actNewNode);
    mnuScene->addAction(Util::scene()->actNewEdge);
    mnuScene->addAction(Util::scene()->actNewLabel);
    mnuScene->addSeparator();
    Util::scene()->addBoundaryAndMaterialMenuItems(mnuScene, this);
    mnuScene->addSeparator();
    mnuScene->addAction(actSceneViewSelectRegion);
    mnuScene->addAction(Util::scene()->actTransform);
    mnuScene->addSeparator();
    mnuScene->addAction(actSceneObjectProperties);
}

void SceneViewGeometry::doSceneObjectProperties()
{
    logMessage("SceneViewCommon::doSceneObjectProperties()");

    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
    {
        if (Util::scene()->selectedCount() > 1)
        {
            SceneBoundarySelectDialog boundaryDialog(this);
            boundaryDialog.exec();
        }
        if (Util::scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Util::scene()->edges->length(); i++)
            {
                if (Util::scene()->edges->at(i)->isSelected)
                    Util::scene()->edges->at(i)->showDialog(this);
            }
        }
    }
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
    {
        if (Util::scene()->selectedCount() > 1)
        {
            SceneMaterialSelectDialog materialDialog(this);
            materialDialog.exec();
        }
        if (Util::scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Util::scene()->labels->length(); i++)
            {
                if (Util::scene()->labels->at(i)->isSelected)
                    Util::scene()->labels->at(i)->showDialog(this);
            }
        }
    }

    Util::scene()->selectNone();
}

void SceneViewGeometry::doSelectBasic()
{
    logMessage("SceneViewCommon::doSelectBasic()");

    SceneBasicSelectDialog sceneBasicSelectDialog(this, QApplication::activeWindow());
    sceneBasicSelectDialog.exec();
}

void SceneViewGeometry::doInvalidated()
{
    // actions
    actSceneViewSelectRegion->setEnabled(actSceneModeGeometry->isChecked());
    actOperateOnNodes->setEnabled(actSceneModeGeometry->isChecked());
    actOperateOnEdges->setEnabled(actSceneModeGeometry->isChecked());
    actOperateOnLabels->setEnabled(actSceneModeGeometry->isChecked());

    SceneViewCommon::doInvalidated();
}

void SceneViewGeometry::clear()
{
    SceneViewCommon2D::clear();

    m_selectRegion = false;

    m_sceneMode = SceneGeometryMode_OperateOnNodes;
}

void SceneViewGeometry::doSceneGeometryModeSet(QAction *action)
{
    if (actOperateOnNodes->isChecked()) m_sceneMode = SceneGeometryMode_OperateOnNodes;
    if (actOperateOnEdges->isChecked()) m_sceneMode = SceneGeometryMode_OperateOnEdges;
    if (actOperateOnLabels->isChecked()) m_sceneMode = SceneGeometryMode_OperateOnLabels;

    switch (m_sceneMode)
    {
    case SceneGeometryMode_OperateOnNodes:
        emit labelCenter(tr("Operate on nodes"));
        break;
    case SceneGeometryMode_OperateOnEdges:
        emit labelCenter(tr("Operate on edges"));
        break;
    case SceneGeometryMode_OperateOnLabels:
        emit labelCenter(tr("Operate on labels"));
        break;
    }

    Util::scene()->highlightNone();
    Util::scene()->selectNone();
    m_nodeLast = NULL;

    doInvalidated();

    emit sceneGeometryModeChanged(m_sceneMode);
}

void SceneViewGeometry::selectRegion(const Point &start, const Point &end)
{
    logMessage("SceneViewCommon::selectRegion()");

    Util::scene()->selectNone();

    switch (m_sceneMode)
    {
    case SceneGeometryMode_OperateOnNodes:
        foreach (SceneNode *node, Util::scene()->nodes->items())
            if (node->point.x >= start.x && node->point.x <= end.x && node->point.y >= start.y && node->point.y <= end.y)
                node->isSelected = true;
        break;
    case SceneGeometryMode_OperateOnEdges:
        foreach (SceneEdge *edge, Util::scene()->edges->items())
            if (edge->nodeStart->point.x >= start.x && edge->nodeStart->point.x <= end.x && edge->nodeStart->point.y >= start.y && edge->nodeStart->point.y <= end.y &&
                    edge->nodeEnd->point.x >= start.x && edge->nodeEnd->point.x <= end.x && edge->nodeEnd->point.y >= start.y && edge->nodeEnd->point.y <= end.y)
                edge->isSelected = true;
        break;
    case SceneGeometryMode_OperateOnLabels:
        foreach (SceneLabel *label, Util::scene()->labels->items())
            if (label->point.x >= start.x && label->point.x <= end.x && label->point.y >= start.y && label->point.y <= end.y)
                label->isSelected = true;
        break;
    }
}

void SceneViewGeometry::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    SceneViewCommon2D::mouseMoveEvent(event);

    m_lastPos = event->pos();

    setToolTip("");

    Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

    // highlight scene objects + hints
    if ((event->modifiers() == 0)
            || ((event->modifiers() & Qt::ControlModifier)
                && (event->modifiers() & Qt::ShiftModifier)
                && (Util::scene()->selectedCount() == 0)))
    {
        // highlight scene objects
        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            // highlight the closest node
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                Util::scene()->highlightNone();
                node->isHighlighted = true;
                setToolTip(tr("<h3>Node</h3>Point: [%1; %2]<br/>Index: %3").
                           arg(node->point.x, 0, 'g', 3).
                           arg(node->point.y, 0, 'g', 3).
                           arg(Util::scene()->nodes->items().indexOf(node)));
                updateGL();
            }
        }
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // highlight the closest edge
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                // assigned boundary conditions
                QString str;
                foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                    str = str + QString("%1 (%2), ").
                            arg(QString::fromStdString(edge->getMarker(fieldInfo)->getName())).
                            arg(QString::fromStdString(fieldInfo->module()->name));
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                Util::scene()->highlightNone();
                edge->isHighlighted = true;
                setToolTip(tr("<h3>Edge</h3><b>Point:</b> [%1; %2] - [%3; %4]<br/><b>Boundary conditions:</b> %5<br/><b>Angle:</b> %6 deg.<br/><b>Refine towards edge:</b> %7<br/><b>Index:</b> %8").
                           arg(edge->nodeStart->point.x, 0, 'g', 3).
                           arg(edge->nodeStart->point.y, 0, 'g', 3).
                           arg(edge->nodeEnd->point.x, 0, 'g', 3).
                           arg(edge->nodeEnd->point.y, 0, 'g', 3).
                           arg(str).
                           arg(edge->angle, 0, 'f', 0).
                           arg(edge->refineTowardsEdge, 0, 'g', 3).
                           arg(Util::scene()->edges->items().indexOf(edge)));
                updateGL();
            }
        }
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // highlight the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                // assigned materials
                QString str;
                foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                    str = str + QString("%1 (%2), ").
                            arg(QString::fromStdString(label->getMarker(fieldInfo)->getName())).
                            arg(QString::fromStdString(fieldInfo->module()->name));
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                Util::scene()->highlightNone();
                label->isHighlighted = true;
                setToolTip(tr("<h3>Label</h3><b>Point:</b> [%1; %2]<br/><b>Materials:</b> %3<br/><b>Triangle area:</b> %4 m<sup>2</sup><br/><b>Polynomial order:</b> %5<br/><b>Index:</b> %6").
                           arg(label->point.x, 0, 'g', 3).
                           arg(label->point.y, 0, 'g', 3).
                           arg(str).
                           arg(label->area, 0, 'g', 3).
                           arg(label->polynomialOrder).
                           arg(Util::scene()->labels->items().indexOf(label)));
                updateGL();
            }
        }
    }

    // add edge by mouse - draw line
    if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
    {
        // add edge directly by mouse click - highlight
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // add edge directly by mouse click
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                Util::scene()->highlightNone();
                node->isHighlighted = true;
                updateGL();
            }
        }
    }

    // snap to grid - nodes
    m_snapToGrid = ((Util::config()->snapToGrid)
                    && ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
                    && (m_sceneMode == SceneGeometryMode_OperateOnNodes));

    if (m_snapToGrid && !(event->modifiers() & Qt::ControlModifier))
    {
        m_snapToGrid = false;
        updateGL();
    }
    if (m_snapToGrid && (event->modifiers() & Qt::ControlModifier))
        updateGL();

    // move nodes and labels directly by mouse - left mouse + control + shift
    if ((event->buttons() & Qt::LeftButton)
            && ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)))
    {
        Point dp = Point(2.0/width() * dx/m_scale2d*aspect(), -2.0/height() * dy/m_scale2d);

        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            // mouse move length memory
            static Point len;
            len = len + dp;

            if (Util::config()->snapToGrid)
            {
                if (fabs(len.x) > Util::config()->gridStep)
                {
                    foreach (SceneNode *node, Util::scene()->nodes->items())
                        if (node->isSelected)
                            node->point.x += (len.x > 0) ? Util::config()->gridStep : -Util::config()->gridStep;
                    len.x = 0;
                }

                if (fabs(len.y) > Util::config()->gridStep)
                {
                    foreach (SceneNode *node, Util::scene()->nodes->items())
                        if (node->isSelected)
                            node->point.y += (len.y > 0) ? Util::config()->gridStep : -Util::config()->gridStep;
                    len.y = 0;
                }
            }
            else
            {
                Util::scene()->transformTranslate(dp, false);
            }
        }
        else if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            static Point len;
            len = len + dp;

            if (Util::config()->snapToGrid)
            {
                if (fabs(len.x) > Util::config()->gridStep)
                {
                    dp.x = (len.x > 0) ? Util::config()->gridStep : -Util::config()->gridStep;
                    dp.y = 0;
                    len.x = 0;

                    Util::scene()->transformTranslate(dp, false);
                }

                if (fabs(len.y) > Util::config()->gridStep)
                {
                    dp.x = 0;
                    dp.y = (len.y > 0) ? Util::config()->gridStep : -Util::config()->gridStep;
                    len.y = 0;

                    Util::scene()->transformTranslate(dp, false);
                }
            }
            else
            {
                Util::scene()->transformTranslate(dp, false);
            }
        }
        else if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            Util::scene()->transformTranslate(dp, false);
        }

        updateGL();
    }

    if (m_snapToGrid)
    {
        Point snapPoint;
        snapPoint.x = floor(p.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
        snapPoint.y = floor(p.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;

        emit mouseMoved(snapPoint);
    }
    else
    {
        emit mouseMoved(p);
    }
}

void SceneViewGeometry::mousePressEvent(QMouseEvent *event)
{
    // select region
    if ((event->button() & Qt::LeftButton)
            && !(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier))
    {
        // select region
        if (actSceneViewSelectRegion->isChecked())
        {
            m_selectRegionPos = m_lastPos;
            actSceneViewSelectRegion->setChecked(false);
            actSceneViewSelectRegion->setData(true);
            m_selectRegion = true;

            return;
        }
    }

    m_lastPos = event->pos();
    Point p = position(Point(event->pos().x(), event->pos().y()));

    // add node, edge or label by mouse click
    // control + left mouse
    if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
    {
        // add node directly by mouse click
        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            Point pointNode;

            // snap to grid
            if (m_snapToGrid)
            {
                Point snapPoint = position(Point(m_lastPos.x(), m_lastPos.y()));

                pointNode.x = floor(snapPoint.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
                pointNode.y = floor(snapPoint.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
            }
            else
            {
                pointNode = p;
            }

            // coordinates must be greater then or equal to 0 (axisymmetric case)
            if ((Util::scene()->problemInfo()->coordinateType == CoordinateType_Axisymmetric) &&
                    (pointNode.x < 0))
            {
                QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));

            }
            else
            {
                SceneNode *node = new SceneNode(pointNode);
                SceneNode *nodeAdded = Util::scene()->addNode(node);
                if (nodeAdded == node) Util::scene()->undoStack()->push(new SceneNodeCommandAdd(node->point));
                updateGL();
            }
        }
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // add edge directly by mouse click
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                if (m_nodeLast == NULL)
                {
                    m_nodeLast = node;
                    m_nodeLast->isSelected = true;
                }
                else
                {
                    if (node != m_nodeLast)
                    {
                        SceneEdge *edge = new SceneEdge(m_nodeLast, node, 0, 0); //TODO - do it better
                        SceneEdge *edgeAdded = Util::scene()->addEdge(edge);
                        if (edgeAdded == edge) Util::scene()->undoStack()->push(new SceneEdgeCommandAdd(edge->nodeStart->point,
                                                                                                  edge->nodeEnd->point,
                                                                                                  "TODO",
                                                                                                  edge->angle,
                                                                                                  edge->refineTowardsEdge));
                    }

                    m_nodeLast->isSelected = false;
                    m_nodeLast = NULL;
                }

                updateGL();
            }
        }
        // add label directly by mouse click
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // coordinates must be greater then or equal to 0 (axisymmetric case)
            if ((Util::scene()->problemInfo()->coordinateType == CoordinateType_Axisymmetric) &&
                    (p.x < 0))
            {
                QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));

            }
            else
            {
                SceneLabel *label = new SceneLabel(p, 0, 0); //TODO - do it better
                SceneLabel *labelAdded = Util::scene()->addLabel(label);
                if (labelAdded == label) Util::scene()->undoStack()->push(new SceneLabelCommandAdd(label->point,
                                                                                             "TODO",
                                                                                             label->area,
                                                                                             label->polynomialOrder));
                updateGL();
            }
        }
    }

    // multiple select or just one node or label due to movement
    // nothing or (shift + control) + left mouse
    if ((event->button() & Qt::LeftButton) && ((event->modifiers() == 0)
                                               || ((event->modifiers() & Qt::ControlModifier)
                                                   && (event->modifiers() & Qt::ShiftModifier)
                                                   && (Util::scene()->selectedCount() == 0))))
    {
        // select scene objects
        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            // select the closest node
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                node->isSelected = !node->isSelected;
                updateGL();
            }
        }

        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // select the closest label
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                edge->isSelected = !edge->isSelected;
                updateGL();
            }
        }

        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // select the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                label->isSelected = !label->isSelected;
                updateGL();
            }
        }
    }

    SceneViewCommon2D::mousePressEvent(event);
}

void SceneViewGeometry::mouseReleaseEvent(QMouseEvent *event)
{
    actSceneViewSelectRegion->setChecked(false);

    if (m_selectRegion)
    {
        Point posStart = position(Point(m_selectRegionPos.x(), m_selectRegionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        if (actSceneViewSelectRegion->data().value<bool>())
            selectRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));

        actSceneViewSelectRegion->setData(false);
    }

    m_selectRegion = false;    
    updateGL();

    // move by mouse - select none
    if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
    {
        Util::scene()->selectNone();
    }

    SceneViewCommon2D::mouseReleaseEvent(event);
}

void SceneViewGeometry::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        Point p = position(Point(event->pos().x(), event->pos().y()));

        if (event->button() & Qt::LeftButton)
        {
            // select scene objects
            Util::scene()->selectNone();
            if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
            {
                // select the closest node
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    node->isSelected = true;
                    updateGL();
                    if (node->showDialog(this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
            {
                // select the closest label
                SceneEdge *edge = findClosestEdge(p);
                if (edge)
                {
                    edge->isSelected = true;
                    updateGL();
                    if (edge->showDialog(this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
            {
                // select the closest label
                SceneLabel *label = findClosestLabel(p);
                if (label)
                {
                    label->isSelected = true;
                    updateGL();
                    if (label->showDialog(this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            Util::scene()->selectNone();
            updateGL();
        }
    }

    SceneViewCommon2D::mouseDoubleClickEvent(event);
}

void SceneViewGeometry::keyPressEvent(QKeyEvent *event)
{
    SceneViewCommon2D::keyPressEvent(event);

    switch (event->key())
    {
    case Qt::Key_Delete:
    {
        Util::scene()->deleteSelected();
    }
        break;
    case Qt::Key_Space:
    {
        doSceneObjectProperties();
    }
        break;
    case Qt::Key_A:
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {
            Util::scene()->selectAll(m_sceneMode);

            refresh();
        }
    }
        break;
    default:
        QGLWidget::keyPressEvent(event);
    }

    // snap to grid
    m_snapToGrid = ((Util::config()->snapToGrid)
                    && (event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier)
                    && (m_sceneMode == SceneGeometryMode_OperateOnNodes));
}

void SceneViewGeometry::keyReleaseEvent(QKeyEvent *event)
{
    m_snapToGrid = false;
    updateGL();

    SceneViewCommon2D::keyReleaseEvent(event);
}

void SceneViewGeometry::contextMenuEvent(QContextMenuEvent *event)
{
    logMessage("SceneViewCommon::contextMenuEvent()");

    actSceneObjectProperties->setEnabled(false);

    // set boundary context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        actSceneObjectProperties->setEnabled(Util::scene()->selectedCount() > 0);

    // set material context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        actSceneObjectProperties->setEnabled(Util::scene()->selectedCount() > 0);

    if (mnuScene)
        delete mnuScene;
    createMenuGeometry();

    mnuScene->exec(event->globalPos());
}

void SceneViewGeometry::paintRulersHintsEdges()
{
    loadProjection2d(true);

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(width(), height()));

    glColor3d(0.0, 0.53, 0.0);

    Point p = position(m_lastPos.x(), m_lastPos.y());

    Point snapPoint = p;
    if (m_snapToGrid)
    {
        snapPoint.x = floor(p.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
        snapPoint.y = floor(p.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;

        // hint line
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x8FFF);

        glLineWidth(1.0);
        glBegin(GL_LINES);
        glVertex2d(snapPoint.x, cornerMax.y - m_rulersAreaWidth.y);
        glVertex2d(snapPoint.x, cornerMin.y);
        glVertex2d(cornerMin.x + m_rulersNumbersWidth + m_rulersAreaWidth.x, snapPoint.y);
        glVertex2d(cornerMax.x, snapPoint.y);
        glEnd();

        glDisable(GL_LINE_STIPPLE);

        renderTextPos(snapPoint.x + m_rulersAreaWidth.x, snapPoint.y - m_rulersAreaWidth.y,
                      QString(tr("%1, %2")).arg(snapPoint.x).arg(snapPoint.y));
    }

    // ticks
    glLineWidth(3.0);
    glBegin(GL_TRIANGLES);
    glVertex2d(snapPoint.x, cornerMax.y - m_rulersAreaWidth.y);
    glVertex2d(snapPoint.x + m_rulersAreaWidth.x * 2.0/7.0, cornerMax.y - m_rulersAreaWidth.y * 2.0/3.0);
    glVertex2d(snapPoint.x - m_rulersAreaWidth.x * 2.0/7.0, cornerMax.y - m_rulersAreaWidth.y * 2.0/3.0);

    glVertex2d(cornerMin.x + m_rulersNumbersWidth + m_rulersAreaWidth.x, snapPoint.y);
    glVertex2d(cornerMin.x + m_rulersNumbersWidth + m_rulersAreaWidth.x * 2.0/3.0, snapPoint.y + m_rulersAreaWidth.y * 2.0/7.0);
    glVertex2d(cornerMin.x + m_rulersNumbersWidth + m_rulersAreaWidth.x * 2.0/3.0, snapPoint.y - m_rulersAreaWidth.y * 2.0/7.0);
    glEnd();
}

void SceneViewGeometry::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Util::config()->colorBackground.redF(),
                 Util::config()->colorBackground.greenF(),
                 Util::config()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // background
    paintBackgroundPixmap();

    // grid
    if (Util::config()->showGrid) paintGrid();

    // geometry
    paintGeometry();

    // rulers
    if (Util::config()->showRulers)
    {
        paintRulers();
        paintRulersHintsEdges();
    }

    // axes
    if (Util::config()->showAxes) paintAxes();

    paintZoomRegion();
    paintSnapToGrid();
    paintEdgeLine();
}

void SceneViewGeometry::paintGeometry()
{
    logMessage("SceneViewCommon::paintGeometry()");

    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // edge without marker
            if (edge->markersCount() == 0)
            {
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0x8FFF);
            }
        }

        glColor3d(Util::config()->colorEdges.redF(),
                  Util::config()->colorEdges.greenF(),
                  Util::config()->colorEdges.blueF());
        glLineWidth(Util::config()->edgeWidth);

        if (edge->isHighlighted)
        {
            glColor3d(Util::config()->colorHighlighted.redF(),
                      Util::config()->colorHighlighted.greenF(),
                      Util::config()->colorHighlighted.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }
        if (edge->isSelected)
        {
            glColor3d(Util::config()->colorSelected.redF(),
                      Util::config()->colorSelected.greenF(),
                      Util::config()->colorSelected.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }

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

        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0);
    }

    // nodes
    foreach (SceneNode *node, Util::scene()->nodes->items())
    {
        glColor3d(Util::config()->colorNodes.redF(),
                  Util::config()->colorNodes.greenF(),
                  Util::config()->colorNodes.blueF());
        glPointSize(Util::config()->nodeSize);

        glBegin(GL_POINTS);
        glVertex2d(node->point.x, node->point.y);
        glEnd();

        glColor3d(Util::config()->colorBackground.redF(),
                  Util::config()->colorBackground.greenF(),
                  Util::config()->colorBackground.blueF());
        glPointSize(Util::config()->nodeSize - 2.0);

        glBegin(GL_POINTS);
        glVertex2d(node->point.x, node->point.y);
        glEnd();

        if ((node->isSelected) || (node->isHighlighted))
        {
            glPointSize(Util::config()->nodeSize - 2.0);

            if (node->isHighlighted)
                glColor3d(Util::config()->colorHighlighted.redF(),
                          Util::config()->colorHighlighted.greenF(),
                          Util::config()->colorHighlighted.blueF());
            if (node->isSelected)
                glColor3d(Util::config()->colorSelected.redF(),
                          Util::config()->colorSelected.greenF(),
                          Util::config()->colorSelected.blueF());

            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();
        }
    }

    glLineWidth(1.0);

    // labels
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        glColor3d(Util::config()->colorLabels.redF(),
                  Util::config()->colorLabels.greenF(),
                  Util::config()->colorLabels.blueF());
        glPointSize(Util::config()->labelSize);
        glBegin(GL_POINTS);
        glVertex2d(label->point.x, label->point.y);
        glEnd();

        glColor3d(Util::config()->colorBackground.redF(),
                  Util::config()->colorBackground.greenF(),
                  Util::config()->colorBackground.blueF());
        glPointSize(Util::config()->labelSize - 2.0);
        glBegin(GL_POINTS);
        glVertex2d(label->point.x, label->point.y);
        glEnd();

        if ((label->isSelected) || (label->isHighlighted))
        {
            if (label->isHighlighted)
                glColor3d(Util::config()->colorHighlighted.redF(),
                          Util::config()->colorHighlighted.greenF(),
                          Util::config()->colorHighlighted.blueF());
            if (label->isSelected)
                glColor3d(Util::config()->colorSelected.redF(),
                          Util::config()->colorSelected.greenF(),
                          Util::config()->colorSelected.blueF());

            glPointSize(Util::config()->labelSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();
        }
        glLineWidth(1.0);

        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            glColor3d(0.1, 0.1, 0.1);

            // assigned materials
            QString str;
            foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                str = str + QString("%1, ").
                        arg(QString::fromStdString(label->getMarker(fieldInfo)->getName()));
            if (str.length() > 0)
                str = str.left(str.length() - 2);

            Point point;
            point.x = 2.0/width()*aspect()*fontMetrics().width(str)/m_scale2d/2.0;
            point.y = 2.0/height()*fontMetrics().height()/m_scale2d;

            renderTextPos(label->point.x-point.x, label->point.y-point.y, str, false);
        }

        // area size
        if ((m_sceneMode == SceneGeometryMode_OperateOnLabels) || (Util::config()->showInitialMeshView))
        {
            double radius = sqrt(label->area/M_PI);
            glColor3d(0, 0.95, 0.9);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i<360; i = i + 10)
            {
                glVertex2d(label->point.x + radius*cos(i/180.0*M_PI), label->point.y + radius*sin(i/180.0*M_PI));
            }
            glEnd();
        }
    }
}

void SceneViewGeometry::paintSnapToGrid()
{
    logMessage("SceneViewCommon::paintSnapToGrid()");

    if (m_snapToGrid)
    {
        loadProjection2d(true);

        Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

        Point snapPoint;
        snapPoint.x = floor(p.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
        snapPoint.y = floor(p.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;

        glColor3d(Util::config()->colorHighlighted.redF(),
                  Util::config()->colorHighlighted.greenF(),
                  Util::config()->colorHighlighted.blueF());
        glPointSize(Util::config()->nodeSize - 1.0);
        glBegin(GL_POINTS);
        glVertex2d(snapPoint.x, snapPoint.y);
        glEnd();
    }
}

void SceneViewGeometry::paintEdgeLine()
{
    logMessage("SceneViewCommon::paintEdgeLine()");

    if (m_nodeLast)
    {
        if (m_nodeLast->isSelected)
        {
            loadProjection2d(true);

            Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

            glColor3d(Util::config()->colorEdges.redF(),
                      Util::config()->colorEdges.greenF(),
                      Util::config()->colorEdges.blueF());

            // check for crossing
            foreach (SceneEdge *edge, Util::scene()->edges->items())
            {
                QList<Point> intersects = intersection(p, m_nodeLast->point,
                                                       m_nodeLast->point, 0, 0,
                                                       edge->nodeStart->point, edge->nodeEnd->point,
                                                       edge->center(), edge->radius(), edge->angle);

                foreach (Point intersect, intersects)
                {
                    // red line and point
                    glColor3d(1.0, 0.0, 0.0);

                    glPointSize(5.0);

                    glBegin(GL_POINTS);
                    glVertex2d(intersect.x, intersect.y);
                    glEnd();
                }
            }

            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x8FFF);

            glLineWidth(Util::config()->edgeWidth);

            glBegin(GL_LINES);
            glVertex2d(m_nodeLast->point.x, m_nodeLast->point.y);
            glVertex2d(p.x, p.y);
            glEnd();

            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
        }
    }
}

ErrorResult SceneViewGeometry::saveGeometryToFile(const QString &fileName, int format)
{
    logMessage("SceneViewCommon::saveImageToFile()");

    // store old value
    SceneGeometryMode sceneMode = m_sceneMode;
    m_sceneMode == SceneGeometryMode_OperateOnNodes;
    actOperateOnNodes->trigger();

    makeCurrent();
    int state = GL2PS_OVERFLOW;
    int buffsize = 0;
    GLint options = GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT;

    FILE *fp = fopen(fileName.toStdString().c_str(), "wb");
    while (state == GL2PS_OVERFLOW)
    {
        buffsize += 1024*1024;
        gl2psBeginPage("Agros2D", "Agros2D - export", NULL, format,
                       GL2PS_BSP_SORT, options,
                       GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, "xxx.pdf");

        glClearColor(Util::config()->colorBackground.redF(),
                     Util::config()->colorBackground.greenF(),
                     Util::config()->colorBackground.blueF(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // paintSolutionMesh();
        // paintInitialMesh();

        paintGeometry();

        state = gl2psEndPage();
    }
    fclose(fp);

    // restore viewport
    m_sceneMode = sceneMode;
    if (m_sceneMode == SceneGeometryMode_OperateOnNodes) actOperateOnNodes->trigger();
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges) actOperateOnEdges->trigger();
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels) actOperateOnLabels->trigger();

    return ErrorResult();
}
