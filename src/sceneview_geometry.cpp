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
#include "hermes2d/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenebasicselectdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
//#include "hermes2d/problem.h"

#include "gl2ps/gl2ps.h"

SceneViewPreprocessor::SceneViewPreprocessor(QWidget *parent)
    : SceneViewCommon2D(NULL, parent)
{
    createActionsGeometry();
    createMenuGeometry();
}

SceneViewPreprocessor::~SceneViewPreprocessor()
{
}

void SceneViewPreprocessor::createActionsGeometry()
{
    actSceneModePreprocessor = new QAction(iconView(), tr("Preprocessor"), this);
    actSceneModePreprocessor->setShortcut(Qt::Key_F5);
    actSceneModePreprocessor->setStatusTip(tr("Preprocessor"));
    actSceneModePreprocessor->setCheckable(true);

    // scene - operate on items
    actOperateOnNodes = new QAction(icon("scene-node"), tr("Operate on &nodes"), this);
    actOperateOnNodes->setShortcut(Qt::Key_F2);
    actOperateOnNodes->setStatusTip(tr("Operate on nodes"));
    actOperateOnNodes->setCheckable(true);
    actOperateOnNodes->setChecked(true);

    actOperateOnEdges = new QAction(icon("scene-edge"), tr("Operate on &edges"), this);
    actOperateOnEdges->setShortcut(Qt::Key_F3);
    actOperateOnEdges->setStatusTip(tr("Operate on edges"));
    actOperateOnEdges->setCheckable(true);

    actOperateOnLabels = new QAction(icon("scene-label"), tr("Operate on &labels"), this);
    actOperateOnLabels->setShortcut(Qt::Key_F4);
    actOperateOnLabels->setStatusTip(tr("Operate on labels"));
    actOperateOnLabels->setCheckable(true);

    actOperateGroup = new QActionGroup(this);
    actOperateGroup->setExclusive(true);
    actOperateGroup->addAction(actOperateOnNodes);
    actOperateGroup->addAction(actOperateOnEdges);
    actOperateGroup->addAction(actOperateOnLabels);
    connect(actOperateGroup, SIGNAL(triggered(QAction *)), this, SLOT(doSceneGeometryModeSet(QAction *)));

    // select region
    actSceneViewSelectRegion = new QAction(icon("scene-select-region"), tr("&Select region"), this);
    actSceneViewSelectRegion->setStatusTip(tr("Select region"));
    actSceneViewSelectRegion->setCheckable(true);

    // object properties
    actSceneObjectProperties = new QAction(icon("scene-properties"), tr("Object properties"), this);
    actSceneObjectProperties->setShortcut(Qt::Key_Space);
    connect(actSceneObjectProperties, SIGNAL(triggered()), this, SLOT(doSceneObjectProperties()));

    // scene edge swap points
    actSceneEdgeSwapDirection = new QAction(icon(""), tr("Swap direction"), this);
    connect(actSceneEdgeSwapDirection, SIGNAL(triggered()), this, SLOT(doSceneEdgeSwapDirection()));
}

void SceneViewPreprocessor::createMenuGeometry()
{
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
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        mnuScene->addAction(actSceneEdgeSwapDirection);
    mnuScene->addAction(actSceneObjectProperties);
}

void SceneViewPreprocessor::doSceneObjectProperties()
{
    if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
    {
        if (Util::scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Util::scene()->nodes->length(); i++)
            {
                if (Util::scene()->nodes->at(i)->isSelected())
                    Util::scene()->nodes->at(i)->showDialog(this);
            }
        }
    }
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
    {
        if (Util::scene()->selectedCount() > 1)
        {
            SceneEdgeSelectDialog *dialog = new SceneEdgeSelectDialog(Util::scene()->edges->selected(), this);
            dialog->exec();
        }
        if (Util::scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Util::scene()->edges->length(); i++)
            {
                if (Util::scene()->edges->at(i)->isSelected())
                    Util::scene()->edges->at(i)->showDialog(this);
            }
        }
    }
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
    {
        if (Util::scene()->selectedCount() > 1)
        {
            SceneLabelSelectDialog *dialog = new SceneLabelSelectDialog(Util::scene()->labels->selected(), this);
            dialog->exec();
        }
        if (Util::scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Util::scene()->labels->length(); i++)
            {
                if (Util::scene()->labels->at(i)->isSelected())
                    Util::scene()->labels->at(i)->showDialog(this);
            }
        }
    }

    Util::scene()->selectNone();
}

void SceneViewPreprocessor::doSceneEdgeSwapDirection()
{
    // swap
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        if (Util::scene()->selectedCount() == 1)
            for (int i = 0; i < Util::scene()->edges->length(); i++)
                if (Util::scene()->edges->at(i)->isSelected())
                {
                    Util::scene()->edges->at(i)->swapDirection();
                    refresh();
                }
}

void SceneViewPreprocessor::doSelectBasic()
{
    SceneBasicSelectDialog sceneBasicSelectDialog(this, QApplication::activeWindow());
    sceneBasicSelectDialog.exec();
}

void SceneViewPreprocessor::refresh()
{
    // actions
    actSceneViewSelectRegion->setEnabled(actSceneModePreprocessor->isChecked());
    actOperateOnNodes->setEnabled(actSceneModePreprocessor->isChecked());
    actOperateOnEdges->setEnabled(actSceneModePreprocessor->isChecked());
    actOperateOnLabels->setEnabled(actSceneModePreprocessor->isChecked());

    SceneViewCommon::refresh();
}

void SceneViewPreprocessor::clear()
{
    SceneViewCommon2D::clear();

    m_selectRegion = false;

    deleteTexture(m_backgroundTexture);
    m_backgroundTexture = -1;

    m_sceneMode = SceneGeometryMode_OperateOnNodes;
}

void SceneViewPreprocessor::doSceneGeometryModeSet(QAction *action)
{
    if (actOperateOnNodes->isChecked())
        m_sceneMode = SceneGeometryMode_OperateOnNodes;
    else if (actOperateOnEdges->isChecked())
        m_sceneMode = SceneGeometryMode_OperateOnEdges;
    else if (actOperateOnLabels->isChecked())
        m_sceneMode = SceneGeometryMode_OperateOnLabels;
    else
    {
        // set default
        actOperateOnNodes->setChecked(true);
        m_sceneMode = SceneGeometryMode_OperateOnNodes;
    }

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

    refresh();

    emit sceneGeometryModeChanged(m_sceneMode);
}

void SceneViewPreprocessor::selectRegion(const Point &start, const Point &end)
{
    Util::scene()->selectNone();

    switch (m_sceneMode)
    {
    case SceneGeometryMode_OperateOnNodes:
        foreach (SceneNode *node, Util::scene()->nodes->items())
            if (node->point().x >= start.x && node->point().x <= end.x && node->point().y >= start.y && node->point().y <= end.y)
                node->setSelected(true);
        break;
    case SceneGeometryMode_OperateOnEdges:
        foreach (SceneEdge *edge, Util::scene()->edges->items())
            if (edge->nodeStart()->point().x >= start.x && edge->nodeStart()->point().x <= end.x && edge->nodeStart()->point().y >= start.y && edge->nodeStart()->point().y <= end.y &&
                    edge->nodeEnd()->point().x >= start.x && edge->nodeEnd()->point().x <= end.x && edge->nodeEnd()->point().y >= start.y && edge->nodeEnd()->point().y <= end.y)
                edge->setSelected(true);
        break;
    case SceneGeometryMode_OperateOnLabels:
        foreach (SceneLabel *label, Util::scene()->labels->items())
            if (label->point().x >= start.x && label->point().x <= end.x && label->point().y >= start.y && label->point().y <= end.y)
                label->setSelected(true);
        break;
    }
}

void SceneViewPreprocessor::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    SceneViewCommon2D::mouseMoveEvent(event);

    m_lastPos = event->pos();

    setToolTip("");

    Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

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
                node->setHighlighted(true);
                setToolTip(tr("<h3>Node</h3>Point: [%1; %2]<br/>Index: %3").
                           arg(node->point().x, 0, 'g', 3).
                           arg(node->point().y, 0, 'g', 3).
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
                QString str, refinement;
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    str = str + QString("%1 (%2), ").
                            arg(edge->marker(fieldInfo)->name()).
                            arg(fieldInfo->name());
                    refinement = refinement + QString("%1 (%2), ").
                            arg(fieldInfo->edgeRefinement(edge)).
                            arg(fieldInfo->name());
                }
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                Util::scene()->highlightNone();
                edge->setHighlighted(true);
                setToolTip(tr("<h3>Edge</h3><b>Point:</b> [%1; %2] - [%3; %4]<br/><b>Boundary conditions:</b> %5<br/><b>Refinement:</b> %6<br/><b>Angle:</b> %7 deg.<br/><b>Index:</b> %8").
                           arg(edge->nodeStart()->point().x, 0, 'g', 3).
                           arg(edge->nodeStart()->point().y, 0, 'g', 3).
                           arg(edge->nodeEnd()->point().x, 0, 'g', 3).
                           arg(edge->nodeEnd()->point().y, 0, 'g', 3).
                           arg(str).
                           arg(refinement).
                           arg(edge->angle(), 0, 'f', 0).
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
                QString str, polynomial_order, area_refinement;
                foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                {
                    str = str + QString("%1 (%2), ").
                            arg(label->marker(fieldInfo)->name()).
                            arg(fieldInfo->name());
                    area_refinement = area_refinement + QString("%1 (%2), ").
                            arg(fieldInfo->labelRefinement(label)).
                            arg(fieldInfo->name());
                    polynomial_order = polynomial_order + QString("%1 (%2), ").
                            arg(fieldInfo->labelPolynomialOrder(label)).
                            arg(fieldInfo->name());
                }
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                Util::scene()->highlightNone();
                label->setHighlighted(true);
                setToolTip(tr("<h3>Label</h3><b>Point:</b> [%1; %2]<br/><b>Materials:</b> %3<br/><b>Area refinement:</b> %4<br/><b>Polynomial order:</b> %5<br/><b>Triangle area:</b> %6 m<sup>2</sup><br /><b>Index:</b> %7").
                           arg(label->point().x, 0, 'g', 3).
                           arg(label->point().y, 0, 'g', 3).
                           arg(str).
                           arg(area_refinement).
                           arg(polynomial_order).
                           arg(label->area(), 0, 'g', 3).
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
                node->setHighlighted(true);
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
            foreach (SceneNode *node, Util::scene()->nodes->items())
            {
                Util::scene()->checkNode(node);
            }

            foreach (SceneEdge *edge, Util::scene()->edges->items())
            {
                Util::scene()->checkEdge(edge);
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
            foreach (SceneNode *node, Util::scene()->nodes->items())
            {
                Util::scene()->checkNode(node);
            }

            foreach (SceneEdge *edge, Util::scene()->edges->items())
            {
                Util::scene()->checkEdge(edge);
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

void SceneViewPreprocessor::mousePressEvent(QMouseEvent *event)
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
    Point p = transform(Point(event->pos().x(), event->pos().y()));

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
                Point snapPoint = transform(Point(m_lastPos.x(), m_lastPos.y()));

                pointNode.x = floor(snapPoint.x / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
                pointNode.y = floor(snapPoint.y / Util::config()->gridStep + 0.5) * Util::config()->gridStep;
            }
            else
            {
                pointNode = p;
            }

            // coordinates must be greater then or equal to 0 (axisymmetric case)
            if ((Util::problem()->config()->coordinateType() == CoordinateType_Axisymmetric) &&
                    (pointNode.x < 0))
            {
                QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));

            }
            else
            {
                SceneNode *node = new SceneNode(pointNode);
                SceneNode *nodeAdded = Util::scene()->addNode(node);
                if (nodeAdded == node) Util::scene()->undoStack()->push(new SceneNodeCommandAdd(node->point()));
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
                    m_nodeLast->setSelected(true);
                }
                else
                {
                    if (node != m_nodeLast)
                    {
                        SceneEdge *edge = new SceneEdge(m_nodeLast, node, 0);
                        SceneEdge *edgeAdded = Util::scene()->addEdge(edge);

                        if (edgeAdded == edge) Util::scene()->undoStack()->push(edge->getAddCommand());
                    }

                    m_nodeLast->setSelected(false);
                    m_nodeLast = NULL;
                }

                updateGL();
            }
        }
        // add label directly by mouse click
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // coordinates must be greater then or equal to 0 (axisymmetric case)
            if ((Util::problem()->config()->coordinateType() == CoordinateType_Axisymmetric) &&
                    (p.x < 0))
            {
                QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));

            }
            else
            {
                SceneLabel *label = new SceneLabel(p, 0);
                SceneLabel *labelAdded = Util::scene()->addLabel(label);

                if (labelAdded == label)
                    Util::scene()->undoStack()->push(label->getAddCommand());

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
                node->setSelected(!node->isSelected());
                updateGL();
            }
        }

        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // select the closest label
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                edge->setSelected(!edge->isSelected());
                updateGL();
            }
        }

        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // select the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                label->setSelected(!label->isSelected());
                updateGL();
            }
        }
    }

    SceneViewCommon2D::mousePressEvent(event);
}

void SceneViewPreprocessor::mouseReleaseEvent(QMouseEvent *event)
{
    actSceneViewSelectRegion->setChecked(false);

    if (m_selectRegion)
    {
        Point posStart = transform(Point(m_selectRegionPos.x(), m_selectRegionPos.y()));
        Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

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

void SceneViewPreprocessor::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        Point p = transform(Point(event->pos().x(), event->pos().y()));

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
                    node->setSelected(true);
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
                    edge->setSelected(true);
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
                    label->setSelected(true);
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

void SceneViewPreprocessor::keyPressEvent(QKeyEvent *event)
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

void SceneViewPreprocessor::keyReleaseEvent(QKeyEvent *event)
{
    m_snapToGrid = false;
    updateGL();

    SceneViewCommon2D::keyReleaseEvent(event);
}

void SceneViewPreprocessor::contextMenuEvent(QContextMenuEvent *event)
{
    actSceneObjectProperties->setEnabled(false);

    // set node context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        actSceneObjectProperties->setEnabled(Util::scene()->selectedCount() == 1);

    // set boundary context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
    {
        actSceneObjectProperties->setEnabled(Util::scene()->selectedCount() > 0);
        actSceneEdgeSwapDirection->setEnabled(Util::scene()->selectedCount() == 1);
    }

    // set material context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        actSceneObjectProperties->setEnabled(Util::scene()->selectedCount() > 0);

    if (mnuScene)
        delete mnuScene;
    createMenuGeometry();

    mnuScene->exec(event->globalPos());
}

void SceneViewPreprocessor::paintRulersHintsEdges()
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
        glVertex2d(snapPoint.x, cornerMax.y - rulersArea.y);
        glVertex2d(snapPoint.x, cornerMin.y);
        glVertex2d(cornerMin.x + rulersArea.x, snapPoint.y);
        glVertex2d(cornerMax.x, snapPoint.y);
        glEnd();

        glDisable(GL_LINE_STIPPLE);
    }

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

    // snap to grid text
    if (m_snapToGrid)
    {
        loadProjectionViewPort();

        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

        Point scr = untransform(snapPoint.x, snapPoint.y);
        printRulersAt(scr.x + m_fontRulers->glyphs[GLYPH_M].width,
                      scr.y + m_fontRulers->height * 0.7,
                      QString(tr("%1, %2")).arg(snapPoint.x).arg(snapPoint.y));
    }
}

void SceneViewPreprocessor::paintBackgroundPixmap()
{
    if (m_backgroundTexture != -1)
    {
        loadProjection2d(true);

        glEnable(GL_TEXTURE_2D);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);

        glColor3d(1.0, 1.0, 1.0);

        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0); glVertex2d(m_backgroundPosition.x(), m_backgroundPosition.y());
        glTexCoord2d(1.0, 0.0); glVertex2d(m_backgroundPosition.x() + m_backgroundPosition.width(), m_backgroundPosition.y());
        glTexCoord2d(1.0, 1.0); glVertex2d(m_backgroundPosition.x() + m_backgroundPosition.width(), m_backgroundPosition.y() + m_backgroundPosition.height());
        glTexCoord2d(0.0, 1.0); glVertex2d(m_backgroundPosition.x(), m_backgroundPosition.y() + m_backgroundPosition.height());
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
}

void SceneViewPreprocessor::loadBackgroundImage(const QString &fileName, double x, double y, double w, double h)
{
    // delete texture
    if (m_backgroundTexture != -1)
    {
        deleteTexture(m_backgroundTexture);
        m_backgroundTexture = -1;
    }

    if (QFile::exists(fileName))
    {
        m_backgroundImage.load(fileName);
        m_backgroundTexture = bindTexture(m_backgroundImage, GL_TEXTURE_2D, GL_RGBA);
        m_backgroundPosition = QRectF(x, y, w, h);
    }
}

void SceneViewPreprocessor::paintGL()
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

void SceneViewPreprocessor::paintGeometry()
{
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

        if (edge->isError())
        {
            glColor3d(Util::config()->colorCrossed.redF(),
                      Util::config()->colorCrossed.greenF(),
                      Util::config()->colorCrossed.blueF());
            glLineWidth(Util::config()->edgeWidth);
        }
        if (edge->isHighlighted())
        {
            glColor3d(Util::config()->colorHighlighted.redF(),
                      Util::config()->colorHighlighted.greenF(),
                      Util::config()->colorHighlighted.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }
        if (edge->isSelected())
        {
            glColor3d(Util::config()->colorSelected.redF(),
                      Util::config()->colorSelected.greenF(),
                      Util::config()->colorSelected.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }

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
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle(), edge->angle()/2.0);
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
        glVertex2d(node->point().x, node->point().y);
        glEnd();

        glColor3d(Util::config()->colorBackground.redF(),
                  Util::config()->colorBackground.greenF(),
                  Util::config()->colorBackground.blueF());
        glPointSize(Util::config()->nodeSize - 2.0);

        glBegin(GL_POINTS);
        glVertex2d(node->point().x, node->point().y);
        glEnd();

        if ((node->isSelected()) || (node->isHighlighted()) || (node->isError()) )
        {
            glPointSize(Util::config()->nodeSize - 2.0);

            if (node->isError())
                glColor3d(Util::config()->colorCrossed.redF(),
                          Util::config()->colorCrossed.greenF(),
                          Util::config()->colorCrossed.blueF());

            if (node->isHighlighted())
                glColor3d(Util::config()->colorHighlighted.redF(),
                          Util::config()->colorHighlighted.greenF(),
                          Util::config()->colorHighlighted.blueF());
            if (node->isSelected())
                glColor3d(Util::config()->colorSelected.redF(),
                          Util::config()->colorSelected.greenF(),
                          Util::config()->colorSelected.blueF());

            glBegin(GL_POINTS);
            glVertex2d(node->point().x, node->point().y);
            glEnd();
        }
    }

    // labels
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        glColor3d(Util::config()->colorLabels.redF(),
                  Util::config()->colorLabels.greenF(),
                  Util::config()->colorLabels.blueF());
        glPointSize(Util::config()->labelSize);
        glBegin(GL_POINTS);
        glVertex2d(label->point().x, label->point().y);
        glEnd();

        glColor3d(Util::config()->colorBackground.redF(),
                  Util::config()->colorBackground.greenF(),
                  Util::config()->colorBackground.blueF());
        glPointSize(Util::config()->labelSize - 2.0);
        glBegin(GL_POINTS);
        glVertex2d(label->point().x, label->point().y);
        glEnd();

        if ((label->isSelected()) || (label->isHighlighted()))
        {
            if (label->isHighlighted())
                glColor3d(Util::config()->colorHighlighted.redF(),
                          Util::config()->colorHighlighted.greenF(),
                          Util::config()->colorHighlighted.blueF());
            if (label->isSelected())
                glColor3d(Util::config()->colorSelected.redF(),
                          Util::config()->colorSelected.greenF(),
                          Util::config()->colorSelected.blueF());

            glPointSize(Util::config()->labelSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point().x, label->point().y);
            glEnd();
        }

        // area size
        if ((m_sceneMode == SceneGeometryMode_OperateOnLabels) || (Util::config()->showInitialMeshView))
        {
            double radius = sqrt(label->area()/M_PI);
            glColor3d(0, 0.95, 0.9);

            glLineWidth(1.0);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i<360; i = i + 10)
            {
                glVertex2d(label->point().x + radius*cos(i/180.0*M_PI), label->point().y + radius*sin(i/180.0*M_PI));
            }
            glEnd();
        }
    }

    // labels hints
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            glColor3d(0.1, 0.1, 0.1);

            // assigned materials
            QString str;
            foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
                str = str + QString("%1, ").
                        arg(label->marker(fieldInfo)->name());
            if (str.length() > 0)
                str = str.left(str.length() - 2);

            Point scr = untransform(label->point().x, label->point().y);

            printRulersAt(scr.x - m_fontRulers->glyphs[GLYPH_M].width * str.length() / 2.0, scr.y - m_fontRulers->height * 1.2, str);
        }
    }
}

void SceneViewPreprocessor::paintSnapToGrid()
{
    if (m_snapToGrid)
    {
        loadProjection2d(true);

        Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

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

void SceneViewPreprocessor::paintEdgeLine()
{
    if (m_nodeLast)
    {
        if (m_nodeLast->isSelected())
        {
            loadProjection2d(true);

            Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

            glColor3d(Util::config()->colorEdges.redF(),
                      Util::config()->colorEdges.greenF(),
                      Util::config()->colorEdges.blueF());

            // check for crossing
            foreach (SceneEdge *edge, Util::scene()->edges->items())
            {
                QList<Point> intersects = intersection(p, m_nodeLast->point(),
                                                       m_nodeLast->point(), 0, 0,
                                                       edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                                       edge->center(), edge->radius(), edge->angle());

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
            glVertex2d(m_nodeLast->point().x, m_nodeLast->point().y);
            glVertex2d(p.x, p.y);
            glEnd();

            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
        }
    }
}

ErrorResult SceneViewPreprocessor::saveGeometryToFile(const QString &fileName, int format)
{
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
