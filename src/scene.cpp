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

#include "scene.h"

#include "util/xml.h"
#include "util/constants.h"
#include "util/global.h"
#include "util/loops.h"

#include "util.h"
#include "value.h"
#include "logview.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenefunction.h"
#include "hermes2d/problem.h"
#include "hermes2d/plugin_interface.h"

#include "problemdialog.h"
#include "scenetransformdialog.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "pythonlab/pythonengine_agros.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/plugin_interface.h"

QString generateSvgGeometry(QList<SceneEdge*> edges)
{
    RectPoint boundingBox = SceneEdgeContainer::boundingBox(edges);

    double size = 200;
    double stroke_width = max(boundingBox.width(), boundingBox.height()) / size / 2.0;

    // svg
    QString str;
    str += QString("<svg width=\"%1px\" height=\"%2px\" viewBox=\"%3 %4 %5 %6\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n").
            arg(size).
            arg(size).
            arg(boundingBox.start.x).
            arg(0).
            arg(boundingBox.width()).
            arg(boundingBox.height());

    str += QString("<g stroke=\"black\" stroke-width=\"%1\" fill=\"none\">\n").arg(stroke_width);

    foreach (SceneEdge *edge, edges)
    {
        if (edge->angle() > 0.0)
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            int segments = edge->angle() / 5.0;
            if (segments < 2) segments = 2;
            double theta = edge->angle() / double(segments - 1);

            for (int i = 0; i < segments-1; i++)
            {
                double arc1 = (startAngle + i*theta)/180.0*M_PI;
                double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                double x1 = radius * cos(arc1);
                double y1 = radius * sin(arc1);
                double x2 = radius * cos(arc2);
                double y2 = radius * sin(arc2);

                str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                        arg(center.x + x1).
                        arg(boundingBox.end.y - (center.y + y1)).
                        arg(center.x + x2).
                        arg(boundingBox.end.y - (center.y + y2));
            }
        }
        else
        {
            str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                    arg(edge->nodeStart()->point().x).
                    arg(boundingBox.end.y - edge->nodeStart()->point().y).
                    arg(edge->nodeEnd()->point().x).
                    arg(boundingBox.end.y - edge->nodeEnd()->point().y);
        }
    }
    str += "</g>\n";
    str += "</svg>\n";

    return str;
}

ostream& operator<<(ostream& output, FieldInfo& id)
{
    output << "FieldInfo " << id.fieldId().toStdString();
    return output;
}

DxfFilter::DxfFilter(Scene *scene)
{
    this->m_scene = scene;
}

void DxfFilter::addLine(const DL_LineData &l)
{
    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(l.x1, l.y1)));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(l.x2, l.y2)));

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd,  0)); // TODO: do it better
}

void DxfFilter::addArc(const DL_ArcData& a)
{
    double angle1 = a.angle1;
    double angle2 = a.angle2;

    while (angle1 < 0.0) angle1 += 360.0;
    while (angle1 >= 360.0) angle1 -= 360.0;
    while (angle2 < 0.0) angle2 += 360.0;
    while (angle2 >= 360.0) angle2 -= 360.0;

    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(a.cx + a.radius*cos(angle1/180*M_PI), a.cy + a.radius*sin(angle1/180*M_PI))));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(a.cx + a.radius*cos(angle2/180*M_PI), a.cy + a.radius*sin(angle2/180*M_PI))));

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1)); // TODO: do it better
}

void DxfFilter::addCircle(const DL_CircleData& c)
{
    // nodes
    SceneNode *node1 = m_scene->addNode(new SceneNode(Point(c.cx + c.radius, c.cy)));
    SceneNode *node2 = m_scene->addNode(new SceneNode(Point(c.cx, c.cy + c.radius)));
    SceneNode *node3 = m_scene->addNode(new SceneNode(Point(c.cx - c.radius, c.cy)));
    SceneNode *node4 = m_scene->addNode(new SceneNode(Point(c.cx, c.cy - c.radius)));

    // edges
    m_scene->addEdge(new SceneEdge(node1, node2, 90));
    m_scene->addEdge(new SceneEdge(node2, node3, 90));
    m_scene->addEdge(new SceneEdge(node3, node4, 90));
    m_scene->addEdge(new SceneEdge(node4, node1, 90));
}

// ************************************************************************************************************************

NewMarkerAction::NewMarkerAction(QIcon icon, QObject* parent, QString field) :
    QAction(icon, availableModules()[field], parent),
    field(field)
{
    setStatusTip(tr("New boundary condition"));
    connect(this, SIGNAL(triggered()), this, SLOT(doTriggered()));
}

void NewMarkerAction::doTriggered()
{
    emit triggered(field);
}

// ************************************************************************************************************************

Scene::Scene()
{
    createActions();

    m_undoStack = new QUndoStack(this);
    //m_sceneSolution = new SceneSolution<double>();
    m_activeViewField = NULL;
    m_activeTimeStep = 0;

    connect(this, SIGNAL(invalidated()), this, SLOT(doInvalidated()));

    boundaries = new SceneBoundaryContainer();
    materials = new SceneMaterialContainer();

    nodes = new SceneNodeContainer();
    edges = new SceneEdgeContainer();
    labels = new SceneLabelContainer();

    clear();
}

Scene::~Scene()
{
    clear();

    delete m_undoStack;

    // TODO write destructors or use QSharedPointers...
    delete boundaries;
    delete materials;
    delete nodes;
    delete edges;
    delete labels;
}

void Scene::createActions()
{
    // scene - add items
    actNewNode = new QAction(icon("scene-node"), tr("New &node..."), this);
    actNewNode->setShortcut(tr("Alt+N"));
    actNewNode->setStatusTip(tr("New node"));
    connect(actNewNode, SIGNAL(triggered()), this, SLOT(doNewNode()));

    actNewEdge = new QAction(icon("scene-edge"), tr("New &edge..."), this);
    actNewEdge->setShortcut(tr("Alt+E"));
    actNewEdge->setStatusTip(tr("New edge"));
    connect(actNewEdge, SIGNAL(triggered()), this, SLOT(doNewEdge()));

    actNewLabel = new QAction(icon("scene-label"), tr("New &label..."), this);
    actNewLabel->setShortcut(tr("Alt+L"));
    actNewLabel->setStatusTip(tr("New label"));
    connect(actNewLabel, SIGNAL(triggered()), this, SLOT(doNewLabel()));

    actDeleteSelected = new QAction(icon("edit-delete"), tr("Delete selected objects"), this);
    actDeleteSelected->setStatusTip(tr("Delete selected objects"));
    connect(actDeleteSelected, SIGNAL(triggered()), this, SLOT(doDeleteSelected()));

    actNewBoundary = new QAction(icon("scene-edgemarker"), tr("New &boundary condition..."), this);
    actNewBoundary->setShortcut(tr("Alt+B"));
    actNewBoundary->setStatusTip(tr("New boundary condition"));
    connect(actNewBoundary, SIGNAL(triggered()), this, SLOT(doNewBoundary()));

    QMapIterator<QString, QString> iEdge(availableModules());
    while (iEdge.hasNext())
    {
        iEdge.next();

        NewMarkerAction* action = new NewMarkerAction(icon("scene-edgemarker"), this, iEdge.key());
        connect(action, SIGNAL(triggered(QString)), this, SLOT(doNewBoundary(QString)));
        actNewBoundaries[iEdge.key()] = action;
    }

    actNewMaterial = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewMaterial->setShortcut(tr("Alt+M"));
    actNewMaterial->setStatusTip(tr("New material"));
    connect(actNewMaterial, SIGNAL(triggered()), this, SLOT(doNewMaterial()));

    QMapIterator<QString, QString> iLabel(availableModules());
    while (iLabel.hasNext())
    {
        iLabel.next();

        NewMarkerAction* action = new NewMarkerAction(icon("scene-labelmarker"), this, iLabel.key());
        connect(action, SIGNAL(triggered(QString)), this, SLOT(doNewMaterial(QString)));
        actNewMaterials[iLabel.key()] = action;
    }

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
}

SceneNode *Scene::addNode(SceneNode *node)
{
    // clear solution
    Agros2D::problem()->clearSolution();

    // check if node doesn't exists
    if (SceneNode* existing = nodes->get(node))
    {
        delete node;
        return existing;
    }

    nodes->add(node);
    if (!currentPythonEngine()->isRunning()) emit invalidated();

    checkNodeConnect(node);
    checkNode(node);

    return node;
}

void Scene::removeNode(SceneNode *node)
{
    // clear solution
    Agros2D::problem()->clearSolution();

    nodes->remove(node);
    // delete node;

    // clear lying nodes
    foreach (SceneEdge *edge, edges->items())
        edge->lyingNodes().removeOne(node);

    // remove all edges connected to this node
    foreach (SceneEdge *edge, edges->items())
    {
        if ((edge->nodeStart() == node) || (edge->nodeEnd() == node))
        {
            // ToDo: add markers
            //            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->markers,
            //                                             edge->angle, edge->refineTowardsEdge));
            removeEdge(edge);
        }
    }

    nodes->items().removeOne(node);
    // delete node;

    emit invalidated();
}

SceneNode *Scene::getNode(const Point &point)
{
    return nodes->get(point);
}

SceneEdge *Scene::addEdge(SceneEdge *edge)
{
    // clear solution
    Agros2D::problem()->clearSolution();

    // check if edge doesn't exists
    if (SceneEdge* existing = edges->get(edge)){
        delete edge;
        return existing;
    }

    // check of crossings
    // ToDo: Zjistit proč se funkce addEdge volá dvakrát pro stejnou hranu. BUG?

    this->checkEdge(edge);

    edge->nodeStart()->connectedEdges().append(edge);
    edge->nodeEnd()->connectedEdges().append(edge);

    foreach(SceneNode *node, nodes->items())
    {
        if ((edge->nodeStart() == node) || (edge->nodeEnd() == node))
            continue;

        if (edge->distance(node->point()) < EPS_ZERO)
        {
            node->lyingEdges().append(edge);
            edge->lyingNodes().append(node);
        }
    }

    edges->add(edge);
    if (!currentPythonEngine()->isRunning()) emit invalidated();

    return edge;
}

void Scene::removeEdge(SceneEdge *edge)
{
    // clear solution
    Agros2D::problem()->clearSolution();

    // clear crosses
    foreach(SceneEdge *edgeCheck, edge->crossedEdges())
        edgeCheck->crossedEdges().removeOne(edge);

    // clear lying edges
    foreach (SceneNode *node, nodes->items())
        node->lyingEdges().removeOne(edge);

    edges->items().removeOne(edge);

    edge->nodeStart()->connectedEdges().removeOne(edge);
    edge->nodeEnd()->connectedEdges().removeOne(edge);

    emit invalidated();
}

SceneEdge *Scene::getEdge(const Point &pointStart, const Point &pointEnd, double angle)
{
    return edges->get(pointStart, pointEnd, angle);
}

SceneLabel *Scene::addLabel(SceneLabel *label)
{
    // clear solution
    Agros2D::problem()->clearSolution();

    // check if label doesn't exists
    if(SceneLabel* existing = labels->get(label)){
        delete label;
        return existing;
    }

    labels->add(label);
    if (!currentPythonEngine()->isRunning()) emit invalidated();

    return label;
}

void Scene::removeLabel(SceneLabel *label)
{
    // clear solution
    Agros2D::problem()->clearSolution();

    labels->remove(label);
    // delete label;

    emit invalidated();
}

SceneLabel *Scene::getLabel(const Point &point)
{
    return labels->get(point);
}

void Scene::addBoundary(SceneBoundary *boundary)
{
    boundaries->add(boundary);
    if (!currentPythonEngine()->isRunning()) emit invalidated();
}

void Scene::removeBoundary(SceneBoundary *boundary)
{
    //TODO instead of setting NoneBoundary we now remove... rething
    edges->removeMarkerFromAll(boundary);
    boundaries->remove(boundary);
    // delete boundary;

    emit invalidated();
}

void Scene::setBoundary(SceneBoundary *boundary)
{
    edges->selected().addMarkerToAll(boundary);
    selectNone();
}

SceneBoundary *Scene::getBoundary(FieldInfo *field, const QString &name)
{
    return boundaries->filter(field).get(name);
}

void Scene::addMaterial(SceneMaterial *material)
{
    this->materials->add(material);
    if (!currentPythonEngine()->isRunning()) emit invalidated();
}


SceneMaterial *Scene::getMaterial(FieldInfo *field, const QString &name)
{
    return materials->filter(field).get(name);
}

void Scene::removeMaterial(SceneMaterial *material)
{
    labels->removeMarkerFromAll(material);
    materials->remove(material);

    // delete material;

    emit invalidated();
}

void Scene::setMaterial(SceneMaterial *material)
{
    labels->selected().addMarkerToAll(material);
    selectNone();
}

bool Scene::checkGeometryAssignement()
{
    if (Agros2D::scene()->edges->length() > 2)
    {
        // at least one boundary condition has to be assigned
        int count = 0;
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
            if (edge->markersCount() > 0)
                count++;

        if (count == 0)
        {
            Agros2D::log()->printError(tr("Geometry"), tr("at least one boundary condition has to be assigned"));
            return false;
        }
    }
    if (Agros2D::scene()->labels->length() < 1)
    {
        Agros2D::log()->printError(tr("Geometry"), tr("invalid number of labels (%1 < 1)").arg(Agros2D::scene()->labels->length()));
        return false;
    }
    else
    {
        // at least one material has to be assigned
        int count = 0;
        foreach (SceneLabel *label, Agros2D::scene()->labels->items())
            if (label->markersCount() > 0)
                count++;

        if (count == 0)
        {
            Agros2D::log()->printError(tr("Geometry"), tr("at least one material has to be assigned"));
            return false;
        }
    }
    if (Agros2D::scene()->boundaries->length() < 2) // + none marker
    {
        Agros2D::log()->printError(tr("Geometry"), tr("invalid number of boundary conditions (%1 < 1)").arg(Agros2D::scene()->boundaries->length()));
        return false;
    }
    if (Agros2D::scene()->materials->length() < 2) // + none marker
    {
        Agros2D::log()->printError(tr("Geometry"), tr("invalid number of materials (%1 < 1)").arg(Agros2D::scene()->materials->length()));
        return false;
    }

    return true;
}

void Scene::clear()
{
    blockSignals(true);

    m_undoStack->clear();

    // TODO: - not good
    // clear problem
    if (Agros2D::singleton() && Agros2D::problem())
    {
        Agros2D::problem()->clearSolution();
        Agros2D::problem()->clearFieldsAndConfig();
    }

    m_activeViewField = NULL;
    m_activeTimeStep = 0;
    m_activeAdaptivityStep = 0;
    m_activeSolutionMode = SolutionMode_Normal;

    // geometry
    nodes->clear();
    edges->clear();
    labels->clear();

    // markers
    boundaries->clear();
    materials->clear();

    // none edge
    boundaries->add(new SceneBoundaryNone());
    // none label
    materials->add(new SceneMaterialNone());

    blockSignals(false);

    emit cleared();

    emit fileNameChanged(tr("unnamed"));
    emit invalidated();
}

RectPoint Scene::boundingBox() const
{
    if (nodes->isEmpty() && edges->isEmpty() && labels->isEmpty())
    {
        return RectPoint(Point(-0.5, -0.5), Point(0.5, 0.5));
    }
    else
    {
        // nodes bounding box
        RectPoint nodesBoundingBox = nodes->boundingBox();
        // edges bounding box
        RectPoint edgesBoundingBox = edges->boundingBox();
        // labels bounding box
        RectPoint labelsBoundingBox = labels->boundingBox();

        return RectPoint(Point(qMin(qMin(nodesBoundingBox.start.x, edgesBoundingBox.start.x), labelsBoundingBox.start.x),
                               qMin(qMin(nodesBoundingBox.start.y, edgesBoundingBox.start.y), labelsBoundingBox.start.y)),
                         Point(qMax(qMax(nodesBoundingBox.end.x, edgesBoundingBox.end.x), labelsBoundingBox.end.x),
                               qMax(qMax(nodesBoundingBox.end.y, edgesBoundingBox.end.y), labelsBoundingBox.end.y)));
    }
}

void Scene::selectNone()
{
    nodes->setSelected(false);
    edges->setSelected(false);
    labels->setSelected(false);
}

void Scene::selectAll(SceneGeometryMode sceneMode)
{
    selectNone();

    switch (sceneMode)
    {
    case SceneGeometryMode_OperateOnNodes:
        nodes->setSelected();
        break;
    case SceneGeometryMode_OperateOnEdges:
        edges->setSelected();
        break;
    case SceneGeometryMode_OperateOnLabels:
        labels->setSelected();
        break;
    }
}

void Scene::deleteSelected()
{
    m_undoStack->beginMacro(tr("Delete selected"));

    nodes->selected().deleteWithUndo(tr("Remove node"));
    edges->selected().deleteWithUndo(tr("Remove edge"));
    labels->selected().deleteWithUndo(tr("Remove label"));

    m_undoStack->endMacro();
    checkGeometry();
    emit invalidated();
}

int Scene::selectedCount()
{
    return nodes->selected().length() +
            edges->selected().length() +
            labels->selected().length();
}

void Scene::highlightNone()
{
    nodes->setHighlighted(false);
    edges->setHighlighted(false);
    labels->setHighlighted(false);
}

int Scene::highlightedCount()
{
    return nodes->highlighted().length() +
            edges->highlighted().length() +
            labels->highlighted().length();
}

void Scene::moveSelectedNodesAndEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy)
{
    QList<SceneEdge *> selectedEdges;
    QList<QPair<double, SceneNode *> > selectedNodes;

    foreach (SceneEdge *edge, edges->items())
    {
        if (edge->isSelected())
        {
            edge->nodeStart()->setSelected(true);
            edge->nodeEnd()->setSelected(true);
            selectedEdges.append(edge);
        }
    }

    foreach (SceneNode *node, nodes->items())
    {
        if (node->isSelected())
        {
            QPair<double, SceneNode *> pair;

            Point newPoint;
            if (mode == SceneTransformMode_Translate)
            {
                newPoint = node->point() + point;
                // projection of the point to the real axis of the displacement vector
                pair.first = node->point().x * cos(point.angle()) + node->point().y * sin(point.angle());
            }
            else if (mode == SceneTransformMode_Rotate)
            {
                double distanceNode = (node->point() - point).magnitude();
                double angleNode = (node->point() - point).angle()/M_PI*180;

                newPoint = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));

                // projection of the point to the tangential axis of the displacement vector
                pair.first = 0;
            }
            else if (mode == SceneTransformMode_Scale)
            {
                newPoint = point + (node->point() - point) * scaleFactor;
                //
                pair.first = ((abs(scaleFactor) > 1) ? 1.0 : -1.0) * (node->point() - point).magnitude();
            }

            SceneNode *obstructNode = getNode(newPoint);
            if (obstructNode && !obstructNode->isSelected())
                return;

            pair.second = node;
            selectedNodes.append(pair);
        }
    }

    // sort selected nodes
    qSort(selectedNodes.begin(), selectedNodes.end(), qGreater<QPair<double, SceneNode *> >());

    for (int i = 0; i < selectedNodes.count(); i++)
    {
        SceneNode *node = selectedNodes[i].second;

        Point newPoint;
        if (mode == SceneTransformMode_Translate)
        {
            newPoint = node->point() + point;
        }
        else if (mode == SceneTransformMode_Rotate)
        {
            double distanceNode = (node->point() - point).magnitude();
            double angleNode = (node->point() - point).angle()/M_PI*180;

            newPoint = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
        }
        else if (mode == SceneTransformMode_Scale)
        {
            newPoint = point + (node->point() - point) * scaleFactor;
        }

        if (!copy)
        {
            m_undoStack->push(new SceneNodeCommandEdit(node->point(), newPoint));
            node->setPoint(newPoint);
        }
        else
        {
            SceneNode *nodeNew = new SceneNode(newPoint);
            SceneNode *nodeAdded = addNode(nodeNew);

            if (nodeAdded == nodeNew)
                m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point()));

            nodeAdded->setSelected(true);
            node->setSelected(false);
        }
    }

    foreach (SceneEdge *edge, selectedEdges)
    {
        if (edge->isSelected())
        {
            edge->nodeStart()->setSelected(false);
            edge->nodeEnd()->setSelected(false);
            // this->checkEdge(edge);

            // add new edge
            if (copy)
            {
                Point newPointStart;
                Point newPointEnd;
                if (mode == SceneTransformMode_Translate)
                {
                    newPointStart = edge->nodeStart()->point() + point;
                    newPointEnd = edge->nodeEnd()->point() + point;
                }
                else if (mode == SceneTransformMode_Rotate)
                {
                    double distanceNodeStart = (edge->nodeStart()->point() - point).magnitude();
                    double angleNodeStart = (edge->nodeStart()->point() - point).angle()/M_PI*180;

                    newPointStart = point + Point(distanceNodeStart * cos((angleNodeStart + angle)/180.0*M_PI), distanceNodeStart * sin((angleNodeStart + angle)/180.0*M_PI));

                    double distanceNodeEnd = (edge->nodeEnd()->point() - point).magnitude();
                    double angleNodeEnd = (edge->nodeEnd()->point() - point).angle()/M_PI*180;

                    newPointEnd = point + Point(distanceNodeEnd * cos((angleNodeEnd + angle)/180.0*M_PI), distanceNodeEnd * sin((angleNodeEnd + angle)/180.0*M_PI));
                }
                else if (mode == SceneTransformMode_Scale)
                {
                    newPointStart = point + (edge->nodeStart()->point() - point) * scaleFactor;
                    newPointEnd = point + (edge->nodeEnd()->point() - point) * scaleFactor;
                }

                // add new edge
                SceneNode *newNodeStart = getNode(newPointStart);
                SceneNode *newNodeEnd = getNode(newPointEnd);
                if (newNodeStart && newNodeEnd)
                {
                    SceneEdge *newEdge = new SceneEdge(newNodeStart, newNodeEnd,
                                                       edge->angle());
                    addEdge(newEdge);

                    m_undoStack->push(newEdge->getAddCommand());
                }
            }
        }
    }

    selectedNodes.clear();
    selectedEdges.clear();
}

void Scene::moveSelectedLabels(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy)
{
    QList<QPair<double, SceneLabel *> > selectedLabels;

    foreach (SceneLabel *label, labels->items())
    {
        if (label->isSelected())
        {
            QPair<double, SceneLabel *> pair;

            Point newPoint;
            if (mode == SceneTransformMode_Translate)
            {
                newPoint = label->point() + point;
                // projection of the point to the real axis of the displacement vector
                pair.first = label->point().x * cos(point.angle()) + label->point().y * sin(point.angle());
            }
            else if (mode == SceneTransformMode_Rotate)
            {
                double distanceLabel = (label->point() - point).magnitude();
                double angleLabel = (label->point() - point).angle()/M_PI*180;

                newPoint = point + Point(distanceLabel * cos((angleLabel + angle)/180.0*M_PI), distanceLabel * sin((angleLabel + angle)/180.0*M_PI));

                // projection of the point to the tangential axis of the displacement vector
                pair.first = 0;
            }
            else if (mode == SceneTransformMode_Scale)
            {
                newPoint = point + (label->point() - point) * scaleFactor;
                pair.first = ((abs(scaleFactor) > 1) ? 1.0 : -1.0) * (label->point() - point).magnitude();
            }

            SceneLabel *obstructLabel = getLabel(newPoint);
            if (obstructLabel && !obstructLabel->isSelected())
                return;

            pair.second = label;
            selectedLabels.append(pair);
        }
    }

    qSort(selectedLabels.begin(), selectedLabels.end(), qGreater<QPair<double, SceneLabel *> >());

    for (int i = 0; i < selectedLabels.count(); i++)
    {
        SceneLabel *label = selectedLabels[i].second;

        Point newPoint;
        if (mode == SceneTransformMode_Translate)
        {
            newPoint = label->point() + point;
        }
        else if (mode == SceneTransformMode_Rotate)
        {
            double distanceLabel = (label->point() - point).magnitude();
            double angleLabel = (label->point() - point).angle()/M_PI*180;

            newPoint = point + Point(distanceLabel * cos((angleLabel + angle)/180.0*M_PI), distanceLabel * sin((angleLabel + angle)/180.0*M_PI));
        }
        else if (mode == SceneTransformMode_Scale)
        {
            newPoint = point + (label->point() - point) * scaleFactor;
        }

        if (!copy)
        {
            m_undoStack->push(new SceneLabelCommandEdit(label->point(), newPoint));
            label->setPoint(newPoint);
        }
        else
        {
            SceneLabel *labelNew = new SceneLabel(newPoint,
                                                  label->area());
            SceneLabel *labelAdded = addLabel(labelNew);

            if (labelAdded == labelNew)
                m_undoStack->push(label->getAddCommand());

            labelAdded->setSelected(true);
            label->setSelected(false);
        }
    }

    selectedLabels.clear();
}

void Scene::transformTranslate(const Point &point, bool copy)
{
    m_undoStack->beginMacro(tr("Translation"));

    moveSelectedNodesAndEdges(SceneTransformMode_Translate, point, 0.0, 0.0, copy);
    moveSelectedLabels(SceneTransformMode_Translate, point, 0.0, 0.0, copy);

    m_undoStack->endMacro();
    emit invalidated();
}

void Scene::transformRotate(const Point &point, double angle, bool copy)
{
    m_undoStack->beginMacro(tr("Rotation"));

    moveSelectedNodesAndEdges(SceneTransformMode_Rotate, point, angle, 0.0, copy);
    moveSelectedLabels(SceneTransformMode_Rotate, point, angle, 0.0, copy);

    m_undoStack->endMacro();
    emit invalidated();
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy)
{
    m_undoStack->beginMacro(tr("Scale"));

    moveSelectedNodesAndEdges(SceneTransformMode_Scale, point, 0.0, scaleFactor, copy);
    moveSelectedLabels(SceneTransformMode_Scale, point, 0.0, scaleFactor, copy);

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::doInvalidated()
{
    actNewEdge->setEnabled((nodes->length() >= 2) && (boundaries->length() >= 1));
    actNewLabel->setEnabled(materials->length() >= 1);    
}

void Scene::doNewNode(const Point &point)
{
    SceneNode *node = new SceneNode(point);
    if (node->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneNode *nodeAdded = addNode(node);
        if (nodeAdded == node) m_undoStack->push(new SceneNodeCommandAdd(node->point()));
    }
    else
        delete node;
}

void Scene::doNewEdge()
{
    SceneEdge *edge = new SceneEdge(nodes->at(0), nodes->at(1), 0);
    if (edge->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneEdge *edgeAdded = addEdge(edge);
        if (edgeAdded == edge)
            m_undoStack->push(edge->getAddCommand());
    }
    else
        delete edge;
}

void Scene::doNewLabel(const Point &point)
{
    SceneLabel *label = new SceneLabel(point, 0.0);
    if (label->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneLabel *labelAdded = addLabel(label);

        if (labelAdded == label)
            m_undoStack->push(label->getAddCommand());
    }
    else
        delete label;
}

void Scene::doDeleteSelected()
{
    deleteSelected();
}

void Scene::doNewBoundary()
{
    doNewBoundary(Agros2D::scene()->activeViewField()->fieldId());
}

void Scene::doNewBoundary(QString field)
{
    SceneBoundary *marker = Agros2D::problem()->fieldInfo(field)->module()->newBoundary();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addBoundary(marker);
    else
        delete marker;
}

void Scene::doNewMaterial()
{
    doNewMaterial(Agros2D::scene()->activeViewField()->fieldId());
}

void Scene::doNewMaterial(QString field)
{
    SceneMaterial *marker = Agros2D::problem()->fieldInfo(field)->module()->newMaterial();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addMaterial(marker);
    else
        delete marker;
}

void Scene::addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent)
{
    if (Agros2D::problem()->fieldInfos().count() == 1)
    {
        // one material and boundary
        menu->addAction(actNewBoundary);
        menu->addAction(actNewMaterial);
    }
    else
    {
        // multiple materials and boundaries
        QMenu* mnuSubBoundaries = new QMenu("New boundary condition", parent);
        menu->addMenu(mnuSubBoundaries);
        foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
            mnuSubBoundaries->addAction(actNewBoundaries[fieldInfo->fieldId()]);

        QMenu* mnuSubMaterials = new QMenu("New material", parent);
        menu->addMenu(mnuSubMaterials);
        foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
            mnuSubMaterials->addAction(actNewMaterials[fieldInfo->fieldId()]);
    }
}

void Scene::doFieldsChanged()
{
    edges->doFieldsChanged();
    labels->doFieldsChanged();
    boundaries->doFieldsChanged();
    materials->doFieldsChanged();
}

void Scene::writeToDxf(const QString &fileName)
{
    RectPoint box = boundingBox();

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    DL_Dxf* dxf = new DL_Dxf();
    DL_Codes::version exportVersion = DL_Codes::AC1015;
    DL_WriterA *dw = dxf->out(fileName.toStdString().c_str(), exportVersion);
    if (dw == NULL) {
        qCritical() << fileName << " could not be opened.";
        return;
    }

    dxf->writeHeader(*dw);
    // int variable:
    dw->dxfString(9, "$INSUNITS");
    dw->dxfInt(70, 4);
    // real (double, float) variable:
    dw->dxfString(9, "$DIMEXE");
    dw->dxfReal(40, 1.25);
    // string variable:
    dw->dxfString(9, "$TEXTSTYLE");
    dw->dxfString(7, "Standard");
    // vector variable:
    dw->dxfString(9, "$LIMMIN");
    dw->dxfReal(10, 0.0);
    dw->dxfReal(20, 0.0);
    dw->sectionEnd();
    dw->sectionTables();
    dxf->writeVPort(*dw);
    dw->tableLineTypes(25);
    dxf->writeLineType(*dw, DL_LineTypeData("BYBLOCK", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BYLAYER", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CONTINUOUS", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO02W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO03W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO04W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO05W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BORDER", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BORDER2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BORDERX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CENTER", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CENTER2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CENTERX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHDOT", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHDOT2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHDOTX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHED", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHED2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHEDX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DIVIDE", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DIVIDE2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DIVIDEX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DOT", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DOT2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DOTX2", 0));
    dw->tableEnd();

    int numberOfLayers = 1;
    dw->tableLayers(numberOfLayers);

    dxf->writeLayer(*dw,
                    DL_LayerData("main", 0),
                    DL_Attributes(
                        std::string(""),            // leave empty
                        DL_Codes::black,            // default color
                        qMax(box.width(), box.height())/100.0,   // default width
                        "CONTINUOUS"));             // default line style

    dw->tableEnd();
    dxf->writeStyle(*dw);
    dxf->writeView(*dw);
    dxf->writeUcs(*dw);

    dw->tableAppid(1);
    dw->tableAppidEntry(0x12);
    dw->dxfString(2, "ACAD");
    dw->dxfInt(70, 0);
    dw->tableEnd();
    dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);
    dxf->writeBlockRecord(*dw);
    dw->tableEnd();
    dw->sectionEnd();
    dw->sectionBlocks();

    dw->sectionEnd();
    dw->sectionEntities();

    // edges
    foreach (SceneEdge *edge, edges->items())
    {
        if (fabs(edge->angle()) < EPS_ZERO)
        {
            // line
            double x1 = edge->nodeStart()->point().x;
            double y1 = edge->nodeStart()->point().y;
            double x2 = edge->nodeEnd()->point().x;
            double y2 = edge->nodeEnd()->point().y;

            dxf->writeLine(*dw, DL_LineData(x1, y1, 0.0, x2, y2, 0.0), DL_Attributes("main", 256, -1, "BYLAYER"));
        }
        else
        {
            // arc
            double cx = edge->center().x;
            double cy = edge->center().y;
            double radius = edge->radius();
            double angle1 = atan2(cy - edge->nodeStart()->point().y, cx - edge->nodeStart()->point().x)/M_PI*180.0 + 180.0;
            double angle2 = atan2(cy - edge->nodeEnd()->point().y, cx - edge->nodeEnd()->point().x)/M_PI*180.0 + 180.0;

            while (angle1 < 0.0) angle1 += 360.0;
            while (angle1 >= 360.0) angle1 -= 360.0;
            while (angle2 < 0.0) angle2 += 360.0;
            while (angle2 >= 360.0) angle2 -= 360.0;

            dxf->writeArc(*dw, DL_ArcData(cx, cy, 0.0, radius, angle1, angle2), DL_Attributes("main", 256, -1, "BYLAYER"));
        }
    }

    dw->sectionEnd();
    dxf->writeObjects(*dw);
    dxf->writeObjectsEnd(*dw);
    dw->dxfEOF();
    dw->close();

    delete dw;
    delete dxf;

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}


void Scene::readFromDxf(const QString &fileName)
{
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    blockSignals(true);

    DxfFilter *filter = new DxfFilter(this);
    DL_Dxf* dxf = new DL_Dxf();
    if (!dxf->in(fileName.toStdString(), filter))
    {
        qCritical() << fileName << " could not be opened.";
        return;
    }

    delete dxf;
    delete filter;

    blockSignals(false);

    emit invalidated();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

ErrorResult Scene::readFromFile(const QString &fileName)
{
    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir())
        settings.setValue("General/LastProblemDir", fileInfo.absolutePath());

    QDomDocument doc;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be opened (%2).").
                           arg(fileName).
                           arg(file.errorString()));

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    clear();

    Agros2D::problem()->config()->setFileName(fileName);
    emit fileNameChanged(fileInfo.absoluteFilePath());

    blockSignals(true);

    if (!doc.setContent(&file))
    {
        file.close();
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' is not valid Agros2D file.").arg(fileName));
    }
    file.close();

    // main document
    QDomElement eleDoc = doc.documentElement();
    QString version = eleDoc.attribute("version");

    // convert document
    if (version.isEmpty() || version == "2.0")
    {
        if (QMessageBox::question(QApplication::activeWindow(), tr("Convert file?"),
                                  tr("File %1 must be converted to the new version. Do you want to convert and replace current file?").arg(fileName),
                                  tr("&Yes"), tr("&No")) == 0)
        {
            QString out = transformXML(fileName, datadir() + "/resources/xslt/problem_a2d_xml.xsl");

            doc.setContent(out);
            eleDoc = doc.documentElement();

            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly))
                return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved (%2).").
                                   arg(fileName).
                                   arg(file.errorString()));

            QTextStream stream(&file);
            doc.save(stream, 4);

            file.waitForBytesWritten(0);
            file.close();
        }
        else
            return ErrorResult();
    }

    // validation
    /*
    ErrorResult error = validateXML(fileName, datadir() + "/resources/xsd/problem_a2d_xml.xsd");
    if (error.isError())
    {
        //qDebug() << error.message();
        return error;
    }
    */

    // geometry ***************************************************************************************************************

    QDomNode eleGeometry = eleDoc.elementsByTagName("geometry").at(0);

    // nodes
    QDomNode eleNodes = eleGeometry.toElement().elementsByTagName("nodes").at(0);
    QDomNode nodeNode = eleNodes.firstChild();
    while (!nodeNode.isNull())
    {
        QDomElement element = nodeNode.toElement();

        Point point = Point(element.attribute("x").toDouble(),
                            element.attribute("y").toDouble());

        addNode(new SceneNode(point));
        nodeNode = nodeNode.nextSibling();
    }

    // edges
    QDomNode eleEdges = eleGeometry.toElement().elementsByTagName("edges").at(0);
    QDomNode nodeEdge = eleEdges.firstChild();
    while (!nodeEdge.isNull())
    {
        QDomElement element = nodeEdge.toElement();

        SceneNode *nodeFrom = nodes->at(element.attribute("start").toInt());
        SceneNode *nodeTo = nodes->at(element.attribute("end").toInt());
        double angle = element.attribute("angle", "0").toDouble();

        SceneEdge *edge = new SceneEdge(nodeFrom, nodeTo, angle);
        addEdge(edge);

        nodeEdge = nodeEdge.nextSibling();
    }

    // labels
    QDomNode eleLabels = eleGeometry.toElement().elementsByTagName("labels").at(0);
    QDomNode nodeLabel = eleLabels.firstChild();
    while (!nodeLabel.isNull())
    {
        QDomElement element = nodeLabel.toElement();
        Point point = Point(element.attribute("x").toDouble(), element.attribute("y").toDouble());
        double area = element.attribute("area", "0").toDouble();

        SceneLabel *label = new SceneLabel(point, area);
        addLabel(label);

        nodeLabel = nodeLabel.nextSibling();
    }

    // problem info
    QDomNode eleProblemInfo = eleDoc.elementsByTagName("problem").at(0);

    // name
    Agros2D::problem()->config()->setName(eleProblemInfo.toElement().attribute("name"));
    // coordinate type
    Agros2D::problem()->config()->setCoordinateType(coordinateTypeFromStringKey(eleProblemInfo.toElement().attribute("coordinate_type")));
    // mesh type
    Agros2D::problem()->config()->setMeshType(meshTypeFromStringKey(eleProblemInfo.toElement().attribute("mesh_type",
                                                                                                      meshTypeToStringKey(MeshType_Triangle))));

    // harmonic
    Agros2D::problem()->config()->setFrequency(eleProblemInfo.toElement().attribute("frequency", "0").toDouble());

    // transient
    Agros2D::problem()->config()->setNumConstantTimeSteps(eleProblemInfo.toElement().attribute("time_steps", "2").toInt());
    Agros2D::problem()->config()->setTimeTotal(Value(eleProblemInfo.toElement().attribute("time_total", "1.0")));
    Agros2D::problem()->config()->setTimeOrder(eleProblemInfo.toElement().attribute("time_order", "1").toInt());
    Agros2D::problem()->config()->setTimeStepMethod(timeStepMethodFromStringKey(
                                                     eleProblemInfo.toElement().attribute("time_method", timeStepMethodToStringKey(TimeStepMethod_Fixed))));
    Agros2D::problem()->config()->setTimeMethodTolerance(eleProblemInfo.toElement().attribute("time_method_tolerance", "0.05"));

    // matrix solver
    Agros2D::problem()->config()->setMatrixSolver(matrixSolverTypeFromStringKey(eleProblemInfo.toElement().attribute("matrix_solver",
                                                                                                                  matrixSolverTypeToStringKey(Hermes::SOLVER_UMFPACK))));

    // startup script
    QDomNode eleScriptStartup = eleProblemInfo.toElement().elementsByTagName("startup_script").at(0);
    Agros2D::problem()->config()->setStartupScript(eleScriptStartup.toElement().text());

    // FIX ME - EOL conversion
    QPlainTextEdit textEdit;
    textEdit.setPlainText(Agros2D::problem()->config()->startupscript());
    Agros2D::problem()->config()->setStartupScript(textEdit.toPlainText());

    // description
    QDomNode eleDescription = eleProblemInfo.toElement().elementsByTagName("description").at(0);
    Agros2D::problem()->config()->setDescription(eleDescription.toElement().text());

    // field ***************************************************************************************************************

    QDomNode eleFields = eleProblemInfo.toElement().elementsByTagName("fields").at(0);
    QDomNode nodeField = eleFields.firstChild();
    while (!nodeField.isNull())
    {
        QDomNode eleField = nodeField.toElement();

        FieldInfo *field = new FieldInfo(eleField.toElement().attribute("field_id"));

        // analysis type
        field->setAnalysisType(analysisTypeFromStringKey(eleField.toElement().attribute("analysis_type",
                                                                                        analysisTypeToStringKey(AnalysisType_SteadyState))));

        // initial condition
        field->setInitialCondition(eleField.toElement().attribute("initial_condition", "0.0"));

        // polynomial order
        field->setPolynomialOrder(eleField.toElement().attribute("polynomial_order").toInt());

        // number of refinements
        field->setNumberOfRefinements(eleField.toElement().attribute("number_of_refinements").toInt());

        // edges refinement
        QDomNode eleRefinement = eleField.toElement().elementsByTagName("refinement").at(0);

        QDomNode eleEdgesRefinement = eleRefinement.toElement().elementsByTagName("edges").at(0);
        QDomNode nodeEdgeRefinement = eleEdgesRefinement.firstChild();
        while (!nodeEdgeRefinement.isNull())
        {
            QDomElement eleEdge = nodeEdgeRefinement.toElement();
            int edge = eleEdge.toElement().attribute("edge").toInt();
            int refinement = eleEdge.toElement().attribute("refinement").toInt();
            field->setEdgeRefinement(Agros2D::scene()->edges->items().at(edge), refinement);

            nodeEdgeRefinement = nodeEdgeRefinement.nextSibling();
        }

        // labels refinement
        QDomNode eleLabelsRefinement = eleRefinement.toElement().elementsByTagName("labels").at(0);
        QDomNode nodeLabelRefinement = eleLabelsRefinement.firstChild();
        while (!nodeLabelRefinement.isNull())
        {
            QDomElement eleLabel = nodeLabelRefinement.toElement();
            int label = eleLabel.toElement().attribute("label").toInt();
            int refinement = eleLabel.toElement().attribute("refinement").toInt();
            field->setLabelRefinement(Agros2D::scene()->labels->items().at(label), refinement);

            nodeLabelRefinement = nodeLabelRefinement.nextSibling();
        }

        // polynomial order
        // labels refinement
        QDomNode eleLabelsPolynomialOrder = eleField.toElement().elementsByTagName("polynomial_order").at(0);
        QDomNode nodeLabelPolynomialOrder = eleLabelsPolynomialOrder.firstChild();
        while (!nodeLabelPolynomialOrder.isNull())
        {
            QDomElement eleLabel = nodeLabelPolynomialOrder.toElement();
            int label = eleLabel.toElement().attribute("label").toInt();
            int order = eleLabel.toElement().attribute("order").toInt();
            field->setLabelPolynomialOrder(Agros2D::scene()->labels->items().at(label), order);

            nodeLabelPolynomialOrder = nodeLabelPolynomialOrder.nextSibling();
        }

        // adaptivity
        QDomNode eleFieldAdaptivity = eleField.toElement().elementsByTagName("adaptivity").at(0);

        field->setAdaptivityType(adaptivityTypeFromStringKey(eleFieldAdaptivity.toElement().attribute("adaptivity_type")));
        field->setAdaptivitySteps(eleFieldAdaptivity.toElement().attribute("adaptivity_steps").toInt());
        field->setAdaptivityTolerance(eleFieldAdaptivity.toElement().attribute("adaptivity_tolerance").toDouble());

        // linearity
        QDomNode eleFieldLinearity = eleField.toElement().elementsByTagName("solver").at(0);

        field->setLinearityType(linearityTypeFromStringKey(eleFieldLinearity.toElement().attribute("linearity_type",
                                                                                                   linearityTypeToStringKey(LinearityType_Linear))));
        field->setNonlinearSteps(eleFieldLinearity.toElement().attribute("nonlinear_steps", "10").toInt());
        field->setNonlinearTolerance(eleFieldLinearity.toElement().attribute("nonlinear_tolerance", "1e-3").toDouble());
        field->setNewtonDampingCoeff(eleFieldLinearity.toElement().attribute("newton_damping_coeff", "1").toDouble());
        field->setNewtonAutomaticDamping(eleFieldLinearity.toElement().attribute("newton_automatic_damping", "1").toInt());
        field->setNewtonDampingNumberToIncrease(eleFieldLinearity.toElement().attribute("newton_damping_number_to_increase", "1").toInt());
        field->setPicardAndersonAcceleration(eleFieldLinearity.toElement().attribute("picard_anderson_acceleration", "1").toInt());
        field->setPicardAndersonBeta(eleFieldLinearity.toElement().attribute("picard_anderson_beta", "0.2").toDouble());
        field->setPicardAndersonNumberOfLastVectors(eleFieldLinearity.toElement().attribute("picard_anderson_vectors", "3").toInt());

        // time steps skip
        field->setTimeSkip(eleField.toElement().attribute("time_skip", "0.0"));

        // boundary conditions
        QDomNode eleBoundaries = eleField.toElement().elementsByTagName("boundaries").at(0);
        QDomNode nodeBoundary = eleBoundaries.firstChild();
        while (!nodeBoundary.isNull())
        {
            QDomElement element = nodeBoundary.toElement();
            QString name = element.toElement().attribute("name");
            QString type = element.toElement().attribute("type");

            // read marker
            SceneBoundary *boundary = new SceneBoundary(field,
                                                        name,
                                                        type);

            Module::BoundaryType *boundary_type = field->module()->boundaryType(type);
            foreach (Module::BoundaryTypeVariable *variable, boundary_type->variables())
                boundary->setValue(variable->id(),
                                   Value(field, element.toElement().attribute(variable->id(), "0")));

            Agros2D::scene()->addBoundary(boundary);

            // add boundary to the edge marker
            QDomNode nodeEdge = element.firstChild();
            while (!nodeEdge.isNull())
            {
                QDomElement eleEdge = nodeEdge.toElement();

                int id = eleEdge.toElement().attribute("edge").toInt();
                edges->at(id)->addMarker(boundary);

                nodeEdge = nodeEdge.nextSibling();
            }

            // add missing none markers
            edges->addMissingFieldMarkers(field);

            nodeBoundary = nodeBoundary.nextSibling();
        }

        // materials
        QDomNode eleMaterials = eleField.toElement().elementsByTagName("materials").at(0);
        QDomNode nodeMaterial = eleMaterials.firstChild();
        while (!nodeMaterial.isNull())
        {
            QDomElement element = nodeMaterial.toElement();
            QString name = element.toElement().attribute("name");

            // read marker
            SceneMaterial *material = new SceneMaterial(field, name);

            foreach (Module::MaterialTypeVariable *variable, field->module()->materialTypeVariables())
            {
                material->setValue(variable->id(),
                                   Value(field, element.toElement().attribute(variable->id(), "0")));
            }

            // add material
            Agros2D::scene()->addMaterial(material);

            // add material to the label marker
            QDomNode nodeLabel = element.firstChild();
            while(!nodeLabel.isNull())
            {
                QDomElement eleLabel = nodeLabel.toElement();

                int id = eleLabel.toElement().attribute("label").toInt();
                labels->at(id)->addMarker(material);

                nodeLabel = nodeLabel.nextSibling();
            }

            // add missing none markers
            labels->addMissingFieldMarkers(field);

            nodeMaterial = nodeMaterial.nextSibling();
        }

        // add field
        Agros2D::problem()->addField(field);

        // next field
        nodeField = nodeField.nextSibling();
    }

    // couplings
    Agros2D::problem()->synchronizeCouplings();

    // coupling
    QDomNode eleCouplings = eleProblemInfo.toElement().elementsByTagName("couplings").at(0);
    QDomNode nodeCoupling = eleCouplings.firstChild();
    while (!nodeCoupling.isNull())
    {
        QDomElement element = nodeCoupling.toElement();

        if (Agros2D::problem()->hasCoupling(element.toElement().attribute("source_fieldid"),
                                         element.toElement().attribute("target_fieldid")))
        {
            CouplingInfo *couplingInfo = Agros2D::problem()->couplingInfo(element.toElement().attribute("source_fieldid"),
                                                                       element.toElement().attribute("target_fieldid"));
            couplingInfo->setCouplingType(couplingTypeFromStringKey(element.toElement().attribute("type")));
        }

        nodeCoupling = nodeCoupling.nextSibling();
    }

    // read config
    QDomElement config = eleDoc.elementsByTagName("config").at(0).toElement();
    Agros2D::config()->load(&config);

    blockSignals(false);

    // default values
    emit invalidated();
    emit defaultValues();

    // run script
    currentPythonEngineAgros()->runScript(Agros2D::problem()->config()->startupscript());

    return ErrorResult();
}

ErrorResult Scene::writeToFile(const QString &fileName)
{
    QSettings settings;

    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
        {
            settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
            Agros2D::problem()->config()->setFileName(fileName);
        }
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDomDocument doc;

    // xml version
    QDomProcessingInstruction xmlVersion = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    doc.appendChild(xmlVersion);

    // main document
    QDomElement eleDoc = doc.createElement("document");
    doc.appendChild(eleDoc);
    eleDoc.setAttribute("version", "2.1");

    // geometry ***************************************************************************************************************

    QDomNode eleGeometry = doc.createElement("geometry");
    eleDoc.appendChild(eleGeometry);

    // nodes
    QDomNode eleNodes = doc.createElement("nodes");
    eleGeometry.appendChild(eleNodes);
    int inode = 0;
    foreach (SceneNode *node, nodes->items())
    {
        QDomElement eleNode = doc.createElement("node");

        eleNode.setAttribute("id", inode);
        eleNode.setAttribute("x", node->point().x);
        eleNode.setAttribute("y", node->point().y);

        eleNodes.appendChild(eleNode);

        inode++;
    }

    // edges
    QDomNode eleEdges = doc.createElement("edges");
    eleGeometry.appendChild(eleEdges);
    int iedge = 0;
    foreach (SceneEdge *edge, edges->items())
    {
        QDomElement eleEdge = doc.createElement("edge");

        eleEdge.setAttribute("id", iedge);
        eleEdge.setAttribute("start", nodes->items().indexOf(edge->nodeStart()));
        eleEdge.setAttribute("end", nodes->items().indexOf(edge->nodeEnd()));
        eleEdge.setAttribute("angle", edge->angle());

        eleEdges.appendChild(eleEdge);

        iedge++;
    }

    // labels
    QDomNode eleLabels = doc.createElement("labels");
    eleGeometry.appendChild(eleLabels);
    int ilabel = 0;
    foreach (SceneLabel *label, labels->items())
    {
        QDomElement eleLabel = doc.createElement("label");

        eleLabel.setAttribute("id", ilabel);
        eleLabel.setAttribute("x", label->point().x);
        eleLabel.setAttribute("y", label->point().y);
        eleLabel.setAttribute("area", label->area());

        eleLabels.appendChild(eleLabel);

        ilabel++;
    }

    // problem info
    QDomElement eleProblem = doc.createElement("problem");
    eleDoc.appendChild(eleProblem);

    // name
    eleProblem.setAttribute("name", Agros2D::problem()->config()->name());
    // coordinate type
    eleProblem.setAttribute("coordinate_type", coordinateTypeToStringKey(Agros2D::problem()->config()->coordinateType()));
    // mesh type
    eleProblem.setAttribute("mesh_type", meshTypeToStringKey(Agros2D::problem()->config()->meshType()));

    // harmonic
    eleProblem.setAttribute("frequency", Agros2D::problem()->config()->frequency());

    // transient
    eleProblem.setAttribute("time_steps", Agros2D::problem()->config()->numConstantTimeSteps());
    eleProblem.setAttribute("time_total", Agros2D::problem()->config()->timeTotal().text());
    eleProblem.setAttribute("time_order", QString::number(Agros2D::problem()->config()->timeOrder()));
    eleProblem.setAttribute("time_method", timeStepMethodToStringKey(Agros2D::problem()->config()->timeStepMethod()));
    eleProblem.setAttribute("time_method_tolerance", Agros2D::problem()->config()->timeMethodTolerance().text());

    // matrix solver
    eleProblem.setAttribute("matrix_solver", matrixSolverTypeToStringKey(Agros2D::problem()->config()->matrixSolver()));

    // startup script
    QDomElement eleScriptStartup = doc.createElement("startup_script");
    eleScriptStartup.appendChild(doc.createTextNode(Agros2D::problem()->config()->startupscript()));
    eleProblem.appendChild(eleScriptStartup);

    // description
    QDomElement eleDescription = doc.createElement("description");
    eleDescription.appendChild(doc.createTextNode(Agros2D::problem()->config()->description()));
    eleProblem.appendChild(eleDescription);

    // field ***************************************************************************************************************
    QDomNode eleFields = doc.createElement("fields");
    eleProblem.appendChild(eleFields);
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QDomElement eleField = doc.createElement("field");
        eleFields.appendChild(eleField);

        // fieldid
        eleField.setAttribute("field_id", fieldInfo->fieldId());
        // analysis type
        eleField.setAttribute("analysis_type", analysisTypeToStringKey(fieldInfo->analysisType()));
        // initial condition
        eleField.setAttribute("initial_condition", fieldInfo->initialCondition().text());
        // polynomial order
        eleField.setAttribute("polynomial_order", fieldInfo->polynomialOrder());
        // time steps skip
        eleField.setAttribute("time_skip", fieldInfo->timeSkip().text());

        // number of refinements
        eleField.setAttribute("number_of_refinements", fieldInfo->numberOfRefinements());

        eleField.setAttribute("number_of_refinements", fieldInfo->numberOfRefinements());

        // refinement
        QDomElement eleRefinement = doc.createElement("refinement");

        // edges
        QDomElement eleEdgesRefinement = doc.createElement("edges");
        QMapIterator<SceneEdge *, int> edgeIterator(fieldInfo->edgesRefinement());
        while (edgeIterator.hasNext()) {
            edgeIterator.next();
            QDomElement eleEdge = doc.createElement("edge");

            eleEdge.setAttribute("edge", QString::number(Agros2D::scene()->edges->items().indexOf(edgeIterator.key())));
            eleEdge.setAttribute("refinement", QString::number(edgeIterator.value()));

            eleEdgesRefinement.appendChild(eleEdge);
        }

        eleRefinement.appendChild(eleEdgesRefinement);

        // labels
        QDomElement eleLabelsRefinement = doc.createElement("labels");
        QMapIterator<SceneLabel *, int> labelIterator(fieldInfo->labelsRefinement());
        while (labelIterator.hasNext()) {
            labelIterator.next();
            QDomElement eleLabel = doc.createElement("label");

            eleLabel.setAttribute("label", QString::number(Agros2D::scene()->labels->items().indexOf(labelIterator.key())));
            eleLabel.setAttribute("refinement", QString::number(labelIterator.value()));

            eleLabelsRefinement.appendChild(eleLabel);
        }

        eleRefinement.appendChild(eleLabelsRefinement);
        eleField.appendChild(eleRefinement);

        // polynomial order
        QDomElement eleLabelPolynomialOrder = doc.createElement("polynomial_order");

        QMapIterator<SceneLabel *, int> labelOrderIterator(fieldInfo->labelsPolynomialOrder());
        while (labelOrderIterator.hasNext()) {
            labelOrderIterator.next();
            QDomElement eleLabel = doc.createElement("label");

            eleLabel.setAttribute("label", QString::number(Agros2D::scene()->labels->items().indexOf(labelOrderIterator.key())));
            eleLabel.setAttribute("order", QString::number(labelOrderIterator.value()));

            eleLabelPolynomialOrder.appendChild(eleLabel);
        }

        eleField.appendChild(eleLabelPolynomialOrder);

        // adaptivity
        QDomElement eleAdaptivity = doc.createElement("adaptivity");
        eleField.appendChild(eleAdaptivity);
        eleAdaptivity.setAttribute("adaptivity_type", adaptivityTypeToStringKey(fieldInfo->adaptivityType()));
        eleAdaptivity.setAttribute("adaptivity_steps", fieldInfo->adaptivitySteps());
        eleAdaptivity.setAttribute("adaptivity_tolerance", fieldInfo->adaptivityTolerance());

        // linearity
        QDomElement eleLinearity = doc.createElement("solver");
        eleField.appendChild(eleLinearity);
        eleLinearity.setAttribute("linearity_type", linearityTypeToStringKey(fieldInfo->linearityType()));
        eleLinearity.setAttribute("nonlinear_steps", fieldInfo->nonlinearSteps());
        eleLinearity.setAttribute("nonlinear_tolerance", fieldInfo->nonlinearTolerance());
        eleLinearity.setAttribute("newton_damping_coeff", fieldInfo->newtonDampingCoeff());
        eleLinearity.setAttribute("newton_automatic_damping", fieldInfo->newtonAutomaticDamping());
        eleLinearity.setAttribute("newton_damping_number_to_increase",fieldInfo->newtonDampingNumberToIncrease());
        eleLinearity.setAttribute("picard_anderson_acceleration", fieldInfo->picardAndersonAcceleration());
        eleLinearity.setAttribute("picard_anderson_beta", fieldInfo->picardAndersonBeta());
        eleLinearity.setAttribute("picard_anderson_vectors", fieldInfo->picardAndersonNumberOfLastVectors());

        // boundaries
        QDomNode eleBoundaries = doc.createElement("boundaries");
        eleField.appendChild(eleBoundaries);
        int iboundary = 1;
        foreach (SceneBoundary *boundary, boundaries->filter(fieldInfo).items())
        {
            QDomElement eleBoundary = doc.createElement("boundary");

            eleBoundary.setAttribute("name", boundary->name());
            if (boundary->type() == "")
                eleBoundary.setAttribute("type", "none");

            // if (iboundary > 0)
            {
                // write marker
                eleBoundary.setAttribute("id", iboundary);
                eleBoundary.setAttribute("type", boundary->type());

                const QHash<QString, Value> values = boundary->values();
                for (QHash<QString, Value>::const_iterator it = values.begin(); it != values.end(); ++it)
                    eleBoundary.setAttribute(it.key(), it.value().toString());

                // add edges
                foreach (SceneEdge *edge, edges->items())
                {
                    if (edge->hasMarker(boundary))
                    {
                        QDomElement eleEdge = doc.createElement("edge");
                        eleEdge.setAttribute("edge", edges->items().indexOf(edge));

                        eleBoundary.appendChild(eleEdge);
                    }
                }
            }

            eleBoundaries.appendChild(eleBoundary);

            iboundary++;
        }

        // materials
        QDomNode eleMaterials = doc.createElement("materials");
        eleField.appendChild(eleMaterials);
        int imaterial = 1;
        foreach (SceneMaterial *material, materials->filter(fieldInfo).items())
        {
            QDomElement eleMaterial = doc.createElement("material");

            // write marker
            eleMaterial.setAttribute("id", imaterial);
            eleMaterial.setAttribute("name", material->name());

            const QHash<QString, Value> values = material->values();
            for (QHash<QString, Value>::const_iterator it = values.begin(); it != values.end(); ++it)
                eleMaterial.setAttribute(it.key(), it.value().toString());

            // add labels
            foreach (SceneLabel *label, labels->items())
            {
                if (label->hasMarker(material))
                {
                    QDomElement eleLabel = doc.createElement("label");
                    eleLabel.setAttribute("label", labels->items().indexOf(label));

                    eleMaterial.appendChild(eleLabel);
                }
            }

            eleMaterials.appendChild(eleMaterial);

            imaterial++;
        }
    }

    // coupling
    QDomNode eleCouplings = doc.createElement("couplings");
    eleProblem.appendChild(eleCouplings);
    foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
    {
        QDomElement eleCoupling = doc.createElement("coupling");
        eleCoupling.setAttribute("id", couplingInfo->couplingId());
        eleCoupling.setAttribute("source_fieldid", couplingInfo->sourceField()->fieldId());
        eleCoupling.setAttribute("target_fieldid", couplingInfo->targetField()->fieldId());
        eleCoupling.setAttribute("type", couplingTypeToStringKey(couplingInfo->couplingType()));
        eleCouplings.appendChild(eleCoupling);
    }

    // save to file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved (%2).").
                           arg(fileName).
                           arg(file.errorString()));

    // save config
    QDomElement eleConfig = doc.createElement("config");
    eleDoc.appendChild(eleConfig);
    Agros2D::config()->save(&eleConfig);

    QTextStream out(&file);
    doc.save(out, 4);

    file.waitForBytesWritten(0);
    file.close();

    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
        emit fileNameChanged(QFileInfo(fileName).absoluteFilePath());

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return ErrorResult();
}

MultiSolutionArray<double> Scene::activeMultiSolutionArray()
{
    return Agros2D::solutionStore()->multiSolution(FieldSolutionID(activeViewField(), activeTimeStep(), activeAdaptivityStep(), activeSolutionType()));
}

void Scene::checkNodeConnect(SceneNode *node)
{
    bool isConnected = false;
    foreach (SceneNode *nodeCheck, this->nodes->items())
    {
        if ((nodeCheck->distance(node->point()) < EPS_ZERO) && (nodeCheck != node))
        {
            isConnected = true;
            foreach (SceneEdge *edgeCheck, node->connectedEdges())
            {

                SceneNode * nodeStart;
                SceneNode * nodeEnd;
                if (edgeCheck->nodeStart()->point() == node->point())
                {
                    nodeStart = nodeCheck;
                    nodeEnd = edgeCheck->nodeEnd();
                }
                if (edgeCheck->nodeEnd()->point() == node->point())
                {
                    nodeStart = edgeCheck->nodeStart();
                    nodeEnd = nodeCheck;
                }

                double edgeAngle = edgeCheck->angle();
                removeEdge(edgeCheck);
                SceneEdge *edge = new SceneEdge(nodeStart, nodeEnd, 0.0);
                edge->setAngle(edgeAngle);
                edge->lyingNodes().clear();
            }
        }
    }

    if(isConnected)
    {
        removeNode(node);
    }
}

void Scene::checkNode(SceneNode *node)
// control if node is not lying on the edge
{

    foreach (SceneEdge *edge, node->lyingEdges())
    {
        edge->lyingNodes().removeOne(node);
    }

    node->lyingEdges().clear();

    foreach (SceneEdge *edge, edges->items())
    {
        if ((edge->nodeStart() == node) || (edge->nodeEnd() == node))
            continue;

        if ((edge->distance(node->point()) < EPS_ZERO))
        {
            node->lyingEdges().append(edge);
            edge->lyingNodes().append(node);
        }

    }
}

void Scene::checkEdge(SceneEdge *edge)
{
    // clear all crossings

    foreach (SceneEdge *edgeCheck, edge->crossedEdges())
    {
        edgeCheck->crossedEdges().removeOne(edge);
    }

    edge->crossedEdges().clear();

    foreach (SceneEdge *edgeCheck, this->edges->items())
    {
        if (edgeCheck != edge)
        {
            QList<Point> intersects;

            // ToDo: Improve
            // ToDo: Add check of crossings of two arcs
            if (edge->angle() > 0)
                intersects = intersection(edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle(),
                                          edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                          edge->center(), edge->radius(), edge->angle());

            else
                intersects = intersection(edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                          edgeCheck->center(), edge->radius(), edge->angle(),
                                          edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle());

            edgeCheck->crossedEdges().removeOne(edge);

            if (intersects.count() > 0)
            {
                edgeCheck->crossedEdges().append(edge);
                edge->crossedEdges().append(edgeCheck);

            }
        }
    }
}

void Scene::checkGeometry()
{
    foreach (SceneNode *node, this->nodes->items())
    {
        this->checkNode(node);
    }

    foreach (SceneEdge *edge, this->edges->items())
    {
        this->checkEdge(edge);
    }
}

ErrorResult Scene::checkGeometryResult()
{
    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
    {
        // check for nodes with r < 0
        QSet<int> nodes;
        foreach (SceneNode *node, this->nodes->items())
        {
            if (node->point().x < - EPS_ZERO)
                nodes.insert(this->nodes->items().indexOf(node));
        }

        if (nodes.count() > 0)
        {
            QString indices;
            foreach (int index, nodes)
                indices += QString::number(index) + ", ";
            indices = indices.left(indices.length() - 2);

            nodes.clear();
            return ErrorResult(ErrorResultType_Critical, tr("There are nodes '%1' with negative radial component.").arg(indices));
        }
        nodes.clear();
    }

    checkGeometry();

    foreach (SceneEdge *edge, this->edges->items())
    {
        if (edge->crossedEdges().count() != 0)
        {
            return ErrorResult(ErrorResultType_Critical, tr("There are crossings in the geometry (red highlighted). Remove the crossings first."));
        }
    }

    foreach (SceneNode *node, this->nodes->items())
    {
        if (!node->isConnected())
        {
            return ErrorResult(ErrorResultType_Critical, tr("There are nodes which are not connected to any edge (red highlighted). All nodes should be connected."));
        }

        if (node->isLyingOnEdges())
        {
            return ErrorResult(ErrorResultType_Critical, tr("There are nodes which lie on the edge but they are not connected to the edge. Remove these nodes first."));
        }
    }

    return ErrorResult();
}

void Scene::setActiveViewField(FieldInfo* fieldInfo)
{
    m_activeViewField = fieldInfo;

    int newTimeStep = Agros2D::solutionStore()->nearestTimeStep(fieldInfo, m_activeTimeStep);
    setActiveTimeStep(newTimeStep);

    int lastAdaptiveStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal, newTimeStep);
    setActiveAdaptivityStep(min(lastAdaptiveStep, activeAdaptivityStep()));
    setActiveSolutionType(SolutionMode_Normal);
}

void Scene::setActiveTimeStep(int ts)
{
    m_activeTimeStep = ts;
}
