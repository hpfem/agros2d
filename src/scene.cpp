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
#include "sceneview.h"

#include "util.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenefunction.h"
#include "scenesolution.h"

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

#include "problemdialog.h"
#include "scenetransformdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"
#include "progressdialog.h"

#include "hermes2d/hermes_field.h"

#include "scripteditordialog.h"

 PhysicField ProblemInfo::physicField()
 {
     return (m_hermes) ? m_hermes->physicField() : PhysicField_Undefined;
 }

void ProblemInfo::clear()
{
    problemType = ProblemType_Planar;
    analysisType = AnalysisType_SteadyState;

    // hermes object
    if (m_hermes) delete m_hermes;

    // read default field (Util::config() is not set)
    QSettings settings;
    PhysicField defaultPhysicField = (PhysicField) settings.value("General/DefaultPhysicField", PhysicField_Electrostatic).toInt();
    m_hermes = hermesFieldFactory(defaultPhysicField);

    name = QObject::tr("unnamed");
    date = QDate::currentDate();
    fileName = "";
    scriptStartup = "";
    description = "";
    numberOfRefinements = 1;
    polynomialOrder = 2;
    adaptivityType = AdaptivityType_None;
    adaptivitySteps = 0;
    adaptivityTolerance = 1.0;

    // harmonic
    frequency = 0.0;

    // transient
    timeStep = Value("1.0", false);
    timeTotal = Value("1.0", false);
    initialCondition = Value("0.0", false);

    // matrix solver
    matrixSolver = SOLVER_UMFPACK;
}

DxfFilter::DxfFilter(Scene *scene)
{
    logMessage("DxfFilter::DxfFilter()");

    this->m_scene = scene;
}

void DxfFilter::addLine(const DL_LineData &l)
{
    logMessage("DxfFilter::addLine()");

    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(l.x1, l.y1)));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(l.x2, l.y2)));

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->edgeMarkers[0], 0, 0));
}

void DxfFilter::addArc(const DL_ArcData& a)
{
    logMessage("DxfFilter::addArc()");

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
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->edgeMarkers[0], (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1, 0));
}

void DxfFilter::addCircle(const DL_CircleData& c)
{
    logMessage("DxfFilter::addCircle()");

    // nodes
    SceneNode *node1 = m_scene->addNode(new SceneNode(Point(c.cx + c.radius, c.cy)));
    SceneNode *node2 = m_scene->addNode(new SceneNode(Point(c.cx, c.cy + c.radius)));
    SceneNode *node3 = m_scene->addNode(new SceneNode(Point(c.cx - c.radius, c.cy)));
    SceneNode *node4 = m_scene->addNode(new SceneNode(Point(c.cx, c.cy - c.radius)));

    // edges
    m_scene->addEdge(new SceneEdge(node1, node2, m_scene->edgeMarkers[0], 90, 0));
    m_scene->addEdge(new SceneEdge(node2, node3, m_scene->edgeMarkers[0], 90, 0));
    m_scene->addEdge(new SceneEdge(node3, node4, m_scene->edgeMarkers[0], 90, 0));
    m_scene->addEdge(new SceneEdge(node4, node1, m_scene->edgeMarkers[0], 90, 0));
}

// ************************************************************************************************************************

// initialize pointer
Util *Util::m_singleton = NULL;

Util::Util()
{
    logMessage("Util::Util()");

    m_scene = new Scene();

    // script remote
    m_scriptEngineRemote = new ScriptEngineRemote();

    // completer
    m_completer = new QCompleter();
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::InlineCompletion);
    m_completer->setModelSorting(QCompleter::UnsortedModel);
    m_completer->setModel(new QStringListModel());

    QSettings settings;
    QStringList list = settings.value("CommandDialog/RecentCommands").value<QStringList>();
    QStringListModel *model = dynamic_cast<QStringListModel *>(m_completer->model());
    model->setStringList(list);

    // config
    m_config = new Config();
    m_config->load();

    initLists();
}

Util::~Util()
{
    logMessage("Util::~Util()");

    delete m_scene;
    delete m_completer;
    delete m_config;
    delete m_scriptEngineRemote;
}

void Util::createSingleton()
{
    m_singleton = new Util();
}

Util *Util::singleton()
{    
    return m_singleton;
}

// ************************************************************************************************************************

Scene::Scene()
{
    logMessage("Scene::Scene()");

    createActions();

    m_problemInfo = new ProblemInfo();
    m_undoStack = new QUndoStack(this);
    m_sceneSolution = new SceneSolution();

    connect(this, SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(m_sceneSolution, SIGNAL(solved()), this, SLOT(doInvalidated()));

    clear();
}

Scene::~Scene()
{
    logMessage("Scene::~Scene()");

    clear();

    delete m_sceneSolution;
    delete m_undoStack;
}

void Scene::createActions()
{
    logMessage("Scene::createActions()");

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

    actNewEdgeMarker = new QAction(icon("scene-edgemarker"), tr("New &boundary condition..."), this);
    actNewEdgeMarker->setShortcut(tr("Alt+B"));
    actNewEdgeMarker->setStatusTip(tr("New boundary condition"));
    connect(actNewEdgeMarker, SIGNAL(triggered()), this, SLOT(doNewEdgeMarker()));

    actNewLabelMarker = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewLabelMarker->setShortcut(tr("Alt+M"));
    actNewLabelMarker->setStatusTip(tr("New material"));
    connect(actNewLabelMarker, SIGNAL(triggered()), this, SLOT(doNewLabelMarker()));

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    actClearSolution = new QAction(icon(""), tr("Clear solution"), this);
    actClearSolution->setStatusTip(tr("Clear solution"));
    connect(actClearSolution, SIGNAL(triggered()), this, SLOT(doClearSolution()));

    actProblemProperties = new QAction(icon("document-properties"), tr("&Problem properties"), this);
    actProblemProperties->setStatusTip(tr("Problem properties"));
    connect(actProblemProperties, SIGNAL(triggered()), this, SLOT(doProblemProperties()));
}

SceneNode *Scene::addNode(SceneNode *node)
{
    logMessage("SceneNode *Scene::addNode()");

    // clear solution
    m_sceneSolution->clear();

    // check if node doesn't exists
    foreach (SceneNode *nodeCheck, nodes)
    {
        if (nodeCheck->point == node->point)
        {
            delete node;
            return nodeCheck;
        }
    }
    nodes.append(node);
    if (!scriptIsRunning()) emit invalidated();

    return node;
}

void Scene::removeNode(SceneNode *node)
{
    logMessage("Scene::removeNode()");

    // clear solution
    m_sceneSolution->clear();

    // remove all edges connected to this node
    foreach (SceneEdge *edge, edges)
    {
        if ((edge->nodeStart == node) || (edge->nodeEnd == node))
        {
            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->marker->name,
                                                         edge->angle, edge->refineTowardsEdge));
            removeEdge(edge);
        }
    }

    nodes.removeOne(node);
    // delete node;

    emit invalidated();
}

SceneNode *Scene::getNode(const Point &point)
{
    logMessage("SceneNode *Scene::getNode()");

    foreach (SceneNode *nodeCheck, nodes)
    {
        if (nodeCheck->point == point)
            return nodeCheck;
    }

    return NULL;
}


SceneEdge *Scene::addEdge(SceneEdge *edge)
{
    logMessage("SceneEdge *Scene::addEdge");

    // clear solution
    m_sceneSolution->clear();

    // check if edge doesn't exists
    foreach (SceneEdge *edgeCheck, edges)
    {
        if (((((edgeCheck->nodeStart == edge->nodeStart) && (edgeCheck->nodeEnd == edge->nodeEnd)) &&
              (fabs(edgeCheck->angle - edge->angle) < EPS_ZERO)) ||
             (((edgeCheck->nodeStart == edge->nodeEnd) && (edgeCheck->nodeEnd == edge->nodeStart))) &&
             (fabs(edgeCheck->angle + edge->angle) < EPS_ZERO)))
        {
            delete edge;
            return edgeCheck;
        }
    }

    edges.append(edge);
    if (!scriptIsRunning()) emit invalidated();

    return edge;
}

void Scene::removeEdge(SceneEdge *edge)
{
    logMessage("Scene::removeEdge()");

    // clear solution
    m_sceneSolution->clear();

    edges.removeOne(edge);
    // delete edge;

    emit invalidated();
}

SceneEdge *Scene::getEdge(const Point &pointStart, const Point &pointEnd, double angle)
{
    logMessage("SceneEdge *Scene::getEdge()");

    foreach (SceneEdge *edgeCheck, edges)
    {
        if (((edgeCheck->nodeStart->point == pointStart) && (edgeCheck->nodeEnd->point == pointEnd)) && (edgeCheck->angle == angle))
            return edgeCheck;
    }

    return NULL;
}

void Scene::setEdgeEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    logMessage("setEdgeEdgeMarker()");

    for (int i = 0; i<edges.count(); i++)
    {
        if (edges[i]->isSelected)
            edges[i]->marker = edgeMarker;
    }
    selectNone();
}

SceneLabel *Scene::addLabel(SceneLabel *label)
{
    logMessage("SceneLabel *Scene::addLabel");

    // clear solution
    m_sceneSolution->clear();

    // check if label doesn't exists
    foreach (SceneLabel *labelCheck, labels)
    {
        if (labelCheck->point == label->point)
        {
            delete label;
            return labelCheck;
        }
    }

    labels.append(label);
    if (!scriptIsRunning()) emit invalidated();

    return label;
}

void Scene::removeLabel(SceneLabel *label)
{
    logMessage("Scene::removeLabel()");

    // clear solution
    m_sceneSolution->clear();

    labels.removeOne(label);
    // delete label;

    emit invalidated();
}

SceneLabel *Scene::getLabel(const Point &point)
{
    logMessage("SceneLabel *Scene::getLabel()");

    foreach (SceneLabel *labelCheck, labels)
    {
        if (labelCheck->point == point)
            return labelCheck;
    }
    return NULL;
}

void Scene::setLabelLabelMarker(SceneLabelMarker *labelMarker)
{
    logMessage("Scene::setLabelLabelMarker()");

    for (int i = 0; i<labels.count(); i++)
    {
        if (labels[i]->isSelected)
            labels[i]->marker = labelMarker;
    }
    selectNone();
}

void Scene::addEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    logMessage("Scene::addEdgeMarker()");

    edgeMarkers.append(edgeMarker);
    if (!scriptIsRunning()) emit invalidated();
}

void Scene::removeEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    logMessage("Scene::removeEdgeMarker()");

    // set none marker
    foreach (SceneEdge *edge, edges)
    {
        if (edge->marker == edgeMarker)
            edge->marker = edgeMarkers[0];
    }
    this->edgeMarkers.removeOne(edgeMarker);
    // delete edgeMarker;

    emit invalidated();
}

SceneEdgeMarker *Scene::getEdgeMarker(const QString &name)
{
    logMessage("SceneEdgeMarker *Scene::getEdgeMarker()");

    for (int i = 0; i<edgeMarkers.count(); i++)
    {
        if (edgeMarkers[i]->name == name)
            return edgeMarkers[i];
    }
    return NULL;
}

bool Scene::setEdgeMarker(const QString &name, SceneEdgeMarker *edgeMarker)
{
    logMessage("Scene::setEdgeMarker()");

    for (int i = 1; i<edgeMarkers.count(); i++)
    {
        if (edgeMarkers[i]->name == name)
        {
            SceneEdgeMarker *markerTemp = edgeMarkers[i];

            // set new marker
            foreach (SceneEdge *edge, edges)
            {
                if (edge->marker == edgeMarkers[i])
                    edge->marker = edgeMarker;
            }

            // replace and delete old marker
            edgeMarkers.replace(i, edgeMarker);
            delete markerTemp;

            return true;
        }
    }

    return false;
}

void Scene::replaceEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    // store original name
    QString name = edgeMarker->name;

    // add new marker
    SceneEdgeMarker *markerNew = Util::scene()->problemInfo()->hermes()->newEdgeMarker();
    Util::scene()->addEdgeMarker(markerNew);

    // set edges to the new marker
    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        SceneEdge *edge = Util::scene()->edges[i];
        if (edge->marker == edgeMarker)
        {
            edge->marker = markerNew;
        }
    }

    // remove old marker
    Util::scene()->removeEdgeMarker(edgeMarker);

    // set original name
    markerNew->name = name;
}

void Scene::addLabelMarker(SceneLabelMarker *labelMarker)
{
    logMessage("Scene::addLabelMarker()");

    this->labelMarkers.append(labelMarker);
    if (!scriptIsRunning()) emit invalidated();
}

bool Scene::setLabelMarker(const QString &name, SceneLabelMarker *labelMarker)
{
    logMessage("Scene::setLabelMarker()");

    for (int i = 1; i<labelMarkers.count(); i++)
    {
        if (labelMarkers[i]->name == name)
        {
            SceneLabelMarker *markerTemp = labelMarkers[i];

            // set new marker
            foreach (SceneLabel *label, labels)
            {
                if (label->marker == labelMarkers[i])
                    label->marker = labelMarker;
            }

            // replace and delete old marker
            labelMarkers.replace(i, labelMarker);
            delete markerTemp;

            return true;
        }
    }

    return false;
}

SceneLabelMarker *Scene::getLabelMarker(const QString &name)
{
    logMessage("Scene::SceneLabelMarker *Scene::getLabelMarker()");

    for (int i = 0; i<labelMarkers.count(); i++)
    {
        if (labelMarkers[i]->name == name)
            return labelMarkers[i];
    }
    return NULL;
}

void Scene::removeLabelMarker(SceneLabelMarker *labelMarker)
{
    logMessage("Scene::removeLabelMarker()");

    // set none marker
    foreach (SceneLabel *label, labels)
    {
        if (label->marker == labelMarker)
            label->marker = labelMarkers[0];
    }
    this->labelMarkers.removeOne(labelMarker);
    // delete labelMarker;

    emit invalidated();
}

void Scene::replaceLabelMarker(SceneLabelMarker *labelMarker)
{
    // store original name
    QString name = labelMarker->name;

    // add new marker
    SceneLabelMarker *markerNew = Util::scene()->problemInfo()->hermes()->newLabelMarker();
    Util::scene()->addLabelMarker(markerNew);

    // set labels to the new marker
    for (int i = 0; i < Util::scene()->labels.count(); i++)
    {
        SceneLabel *label = Util::scene()->labels[i];
        if (label->marker == labelMarker)
        {
            label->marker = markerNew;
        }
    }

    // remove old marker
    Util::scene()->removeLabelMarker(labelMarker);

    // set original name
    markerNew->name = name;
}

void Scene::clear()
{
    logMessage("Scene::clear()");

    blockSignals(true);

    m_undoStack->clear();

    m_sceneSolution->clear();
    m_problemInfo->clear();

    // geometry
    for (int i = 0; i < nodes.count(); i++) delete nodes[i];
    nodes.clear();
    for (int i = 0; i < edges.count(); i++) delete edges[i];
    edges.clear();
    for (int i = 0; i < labels.count(); i++) delete labels[i];
    labels.clear();

    // markers
    for (int i = 0; i < edgeMarkers.count(); i++) delete edgeMarkers[i];
    edgeMarkers.clear();
    for (int i = 0; i < labelMarkers.count(); i++) delete labelMarkers[i];
    labelMarkers.clear();

    // none edge
    addEdgeMarker(new SceneEdgeMarkerNone());
    // none label
    addLabelMarker(new SceneLabelMarkerNone());

    blockSignals(false);

    emit fileNameChanged(tr("unnamed"));
    emit invalidated();
}

RectPoint Scene::boundingBox() const
{
    logMessage("RectPoint Scene::boundingBox()");

    Point min( CONST_DOUBLE,  CONST_DOUBLE);
    Point max(-CONST_DOUBLE, -CONST_DOUBLE);

    foreach (SceneNode *node, nodes) {
        if (node->point.x<min.x) min.x = node->point.x;
        if (node->point.x>max.x) max.x = node->point.x;
        if (node->point.y<min.y) min.y = node->point.y;
        if (node->point.y>max.y) max.y = node->point.y;
    }

    RectPoint rect;
    if (nodes.length() > 0)
        rect.set(min, max);
    else
        rect.set(Point(-0.5, -0.5), Point(0.5, 0.5));

    return rect;
}

void Scene::selectNone()
{
    logMessage("Scene::selectNone()");

    foreach (SceneNode *node, nodes)
        node->isSelected = false;

    foreach (SceneEdge *edge, edges)
        edge->isSelected = false;

    foreach (SceneLabel *label, labels)
        label->isSelected = false;
}

void Scene::selectAll(SceneMode sceneMode)
{
    logMessage("Scene::selectAll()");

    selectNone();

    switch (sceneMode)
    {
    case SceneMode_OperateOnNodes:
        foreach (SceneNode *node, nodes)
            node->isSelected = true;
        break;
    case SceneMode_OperateOnEdges:
        foreach (SceneEdge *edge, edges)
            edge->isSelected = true;
        break;
    case SceneMode_OperateOnLabels:
        foreach (SceneLabel *label, labels)
            label->isSelected = true;
        break;
    }
}

void Scene::deleteSelected()
{
    logMessage("Scene::deleteSelected()");

    m_undoStack->beginMacro(tr("Delete selected"));

    foreach (SceneNode *node, nodes)
    {
        if (node->isSelected)
        {
            m_undoStack->beginMacro(tr("Remove node"));
            m_undoStack->push(new SceneNodeCommandRemove(node->point));
            removeNode(node);
            m_undoStack->endMacro();
        }
    }

    foreach (SceneEdge *edge, edges)
    {
        if (edge->isSelected)
        {
            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->marker->name,
                                                         edge->angle, edge->refineTowardsEdge));
            removeEdge(edge);
        }
    }

    foreach (SceneLabel *label, labels)
    {
        if (label->isSelected)
        {
            m_undoStack->push(new SceneLabelCommandRemove(label->point, label->marker->name, label->area, label->polynomialOrder));
            removeLabel(label);
        }
    }

    m_undoStack->endMacro();

    emit invalidated();
}

int Scene::selectedCount()
{
    logMessage("Scene::selectedCount()");

    int count = 0;
    foreach (SceneNode *node, nodes)
        if (node->isSelected) count++;

    foreach (SceneEdge *edge, edges)
        if (edge->isSelected) count++;

    foreach (SceneLabel *label, labels)
        if (label->isSelected) count++;

    return count;
}

void Scene::highlightNone()
{
    logMessage("Scene::highlightNone()");

    foreach (SceneNode *node, nodes)
        node->isHighlighted = false;

    foreach (SceneEdge *edge, edges)
        edge->isHighlighted = false;

    foreach (SceneLabel *label, labels)
        label->isHighlighted = false;
}

void Scene::transformTranslate(const Point &point, bool copy)
{
    logMessage("Scene::transformTranslate()");

    // clear solution
    m_sceneSolution->clear();

    m_undoStack->beginMacro(tr("Translation"));

    foreach (SceneEdge *edge, edges)
    {
        if (edge->isSelected)
        {
            edge->nodeStart->isSelected = true;
            edge->nodeEnd->isSelected = true;
        }
    }

    foreach (SceneNode *node, nodes)
    {
        if (node->isSelected)
        {
            Point pointNew = node->point + point;
            if (!copy)
            {
                if (!getNode(pointNew))
                {
                    m_undoStack->push(new SceneNodeCommandEdit(node->point, pointNew));
                    node->point = pointNew;
                }
            }
            else
            {
                SceneNode *nodeNew = new SceneNode(pointNew);
                SceneNode *nodeAdded = addNode(nodeNew);
                if (nodeAdded == nodeNew) m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));
            }
        }
    }

    foreach (SceneLabel *label, labels)
    {
        if (label->isSelected)
        {
            Point pointNew = label->point + point;
            if (!copy)
            {
                if (!getLabel(pointNew))
                {
                    m_undoStack->push(new SceneLabelCommandEdit(label->point, pointNew));
                    label->point = pointNew;
                }
            }
            else
            {
                SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area, label->polynomialOrder);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->marker->name, labelNew->area, label->polynomialOrder));
            }
        }
    }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::transformRotate(const Point &point, double angle, bool copy)
{
    logMessage("Scene::transformRotate()");

    // clear solution
    m_sceneSolution->clear();

    m_undoStack->beginMacro(tr("Rotation"));

    foreach (SceneEdge *edge, edges)
    {
        if (edge->isSelected)
        {
            edge->nodeStart->isSelected = true;
            edge->nodeEnd->isSelected = true;
        }
    }

    foreach (SceneNode *node, nodes)
        if (node->isSelected)
        {
            double distanceNode = (node->point - point).magnitude();
            double angleNode = (node->point - point).angle()/M_PI*180;

            Point pointNew = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
            if (!copy)
            {
                node->point = pointNew;
            }
            else
            {
                SceneNode *nodeNew = new SceneNode(pointNew);
                SceneNode *nodeAdded = addNode(nodeNew);
                if (nodeAdded == nodeNew) m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));
            }
        }

    foreach (SceneLabel *label, labels)
        if (label->isSelected)
        {
            double distanceNode = (label->point - point).magnitude();
            double angleNode = (label->point - point).angle()/M_PI*180;

            Point pointNew = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
            if (!copy)
            {
                label->point = pointNew;
            }
            else
            {
                SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area, label->polynomialOrder);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->marker->name, labelNew->area, labelNew->polynomialOrder));
            }
        }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy)
{
    logMessage("Scene::transformScale()");

    // clear solution
    m_sceneSolution->clear();

    m_undoStack->beginMacro(tr("Scale"));

    foreach (SceneEdge *edge, edges)
    {
        if (edge->isSelected)
        {
            edge->nodeStart->isSelected = true;
            edge->nodeEnd->isSelected = true;
        }
    }

    foreach (SceneNode *node, nodes)
        if (node->isSelected)
        {
            Point pointNew = point + (node->point - point) * scaleFactor;
            if (!copy)
            {
                node->point = pointNew;
            }
            else
            {
                SceneNode *nodeNew = new SceneNode(pointNew);
                SceneNode *nodeAdded = addNode(nodeNew);
                if (nodeAdded == nodeNew) m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));
            }
        }

    foreach (SceneLabel *label, labels)
        if (label->isSelected)
        {
            Point pointNew = point + (label->point - point) * scaleFactor;
            if (!copy)
            {
                label->point = pointNew;
            }
            else
            {
                SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area, label->polynomialOrder);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->marker->name, labelNew->area, labelNew->polynomialOrder));
            }
        }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::doInvalidated()
{
    logMessage("Scene::doInvalidated()");

    actNewEdge->setEnabled((nodes.count() >= 2) && (edgeMarkers.count() >= 1));
    actNewLabel->setEnabled(labelMarkers.count() >= 1);
    actClearSolution->setEnabled(m_sceneSolution->isSolved());
}

void Scene::doNewNode(const Point &point)
{
    logMessage("Scene::doNewNode()");

    SceneNode *node = new SceneNode(point);
    if (node->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneNode *nodeAdded = addNode(node);
        if (nodeAdded == node) m_undoStack->push(new SceneNodeCommandAdd(node->point));
    }
    else
        delete node;
}

void Scene::doNewEdge()
{
    logMessage("Scene::doNewEdge()");

    SceneEdge *edge = new SceneEdge(nodes[0], nodes[1], edgeMarkers[0], 0, 0);
    if (edge->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneEdge *edgeAdded = addEdge(edge);
        if (edgeAdded == edge)
            m_undoStack->push(new SceneEdgeCommandAdd(edge->nodeStart->point,
                                                      edge->nodeEnd->point,
                                                      edge->marker->name,
                                                      edge->angle,
                                                      edge->refineTowardsEdge));
    }
    else
        delete edge;
}

void Scene::doNewLabel(const Point &point)
{
    logMessage("Scene::doNewLabel()");

    SceneLabel *label = new SceneLabel(point, labelMarkers[0], 0.0, 0);
    if (label->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneLabel *labelAdded = addLabel(label);
        if (labelAdded == label) m_undoStack->push(new SceneLabelCommandAdd(label->point,
                                                                            label->marker->name,
                                                                            label->area,
                                                                            label->polynomialOrder));
    }
    else
        delete label;
}

void Scene::doDeleteSelected()
{
    logMessage("doDeleteSelected()");

    deleteSelected();
}

void Scene::doNewEdgeMarker()
{
    logMessage("Scene::doNewEdgeMarker()");

    SceneEdgeMarker *marker = Util::scene()->problemInfo()->hermes()->newEdgeMarker();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addEdgeMarker(marker);
    else
        delete marker;
}

void Scene::doNewLabelMarker()
{
    logMessage("Scene::doNewLabelMarker()");

    SceneLabelMarker *marker = Util::scene()->problemInfo()->hermes()->newLabelMarker();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addLabelMarker(marker);
    else
        delete marker;
}

void Scene::doTransform()
{
    logMessage("Scene::doTransform()");

    SceneTransformDialog sceneTransformDialog(QApplication::activeWindow());
    sceneTransformDialog.exec();
}

void Scene::doClearSolution()
{
    logMessage("Scene::doClearSolution()");

    m_sceneSolution->clear();
    emit invalidated();
}

void Scene::doProblemProperties()
{
    logMessage("Scene::doProblemProperties()");

    ProblemDialog problemDialog(m_problemInfo, false, QApplication::activeWindow());
    if (problemDialog.showDialog() == QDialog::Accepted)
    {
        // FIXME - add QList of variables
        QComboBox *cmbScalarFieldVariable = new QComboBox();
        Util::scene()->problemInfo()->hermes()->fillComboBoxScalarVariable(cmbScalarFieldVariable);
        QComboBox *cmbVectorFieldVariable = new QComboBox();
        Util::scene()->problemInfo()->hermes()->fillComboBoxVectorVariable(cmbVectorFieldVariable);

        // determines whether the selected field exists
        if (cmbScalarFieldVariable->findData(sceneView()->sceneViewSettings().contourPhysicFieldVariable) == -1)
            sceneView()->sceneViewSettings().contourPhysicFieldVariable = Util::scene()->problemInfo()->hermes()->contourPhysicFieldVariable();

        if (cmbScalarFieldVariable->findData(sceneView()->sceneViewSettings().scalarPhysicFieldVariable) == -1)
        {
            sceneView()->sceneViewSettings().scalarPhysicFieldVariable = Util::scene()->problemInfo()->hermes()->scalarPhysicFieldVariable();
            sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = Util::scene()->problemInfo()->hermes()->scalarPhysicFieldVariableComp();
            sceneView()->sceneViewSettings().scalarRangeAuto = true;
        }

        if (cmbVectorFieldVariable->findData(sceneView()->sceneViewSettings().vectorPhysicFieldVariable) == -1)
            sceneView()->sceneViewSettings().vectorPhysicFieldVariable = Util::scene()->problemInfo()->hermes()->vectorPhysicFieldVariable();

        delete cmbScalarFieldVariable;
        delete cmbVectorFieldVariable;


        emit invalidated();
    }
}

void Scene::writeToDxf(const QString &fileName)
{
    logMessage("Scene::writeToDxf()");

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
    for (int i = 0; i<edges.length(); i++)
    {
        if (edges[i]->angle == 0)
        {
            // line
            double x1 = edges[i]->nodeStart->point.x;
            double y1 = edges[i]->nodeStart->point.y;
            double x2 = edges[i]->nodeEnd->point.x;
            double y2 = edges[i]->nodeEnd->point.y;

            dxf->writeLine(*dw, DL_LineData(x1, y1, 0.0, x2, y2, 0.0), DL_Attributes("main", 256, -1, "BYLAYER"));
        }
        else
        {
            // arc
            double cx = edges[i]->center().x;
            double cy = edges[i]->center().y;
            double radius = edges[i]->radius();
            double angle1 = atan2(cy - edges[i]->nodeStart->point.y, cx - edges[i]->nodeStart->point.x)/M_PI*180.0 + 180.0;
            double angle2 = atan2(cy - edges[i]->nodeEnd->point.y, cx - edges[i]->nodeEnd->point.x)/M_PI*180.0 + 180.0;

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
    logMessage("Scene::readFromDxf()");

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
    logMessage("Scene::readFromFile()");

    QSettings settings;
    QFileInfo fileInfo(fileName);
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
    m_problemInfo->fileName = fileName;
    emit fileNameChanged(fileInfo.absoluteFilePath());

    blockSignals(true);

    if (!doc.setContent(&file)) {
        file.close();
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' is not valid Agros2D file.").arg(fileName));
    }
    file.close();

    QDomNode n;
    QDomElement element;

    // main document
    QDomElement eleDoc = doc.documentElement();

    // problems
    QDomNode eleProblems = eleDoc.elementsByTagName("problems").at(0);
    // first problem
    QDomNode eleProblem = eleProblems.toElement().elementsByTagName("problem").at(0);
    // name
    m_problemInfo->name = eleProblem.toElement().attribute("name");
    // problem type                                                                                                                                                                                                                             `
    m_problemInfo->problemType = problemTypeFromStringKey(eleProblem.toElement().attribute("problemtype"));
    // analysis type
    m_problemInfo->analysisType = analysisTypeFromStringKey(eleProblem.toElement().attribute("analysistype",
                                                                                             analysisTypeToStringKey(AnalysisType_SteadyState)));
    // physic field
    m_problemInfo->setHermes(hermesFieldFactory(physicFieldFromStringKey(eleProblem.toElement().attribute("type"))));
    // number of refinements
    m_problemInfo->numberOfRefinements = eleProblem.toElement().attribute("numberofrefinements").toInt();
    // polynomial order
    m_problemInfo->polynomialOrder = eleProblem.toElement().attribute("polynomialorder").toInt();
    // adaptivity
    m_problemInfo->adaptivityType = adaptivityTypeFromStringKey(eleProblem.toElement().attribute("adaptivitytype"));
    m_problemInfo->adaptivitySteps = eleProblem.toElement().attribute("adaptivitysteps").toInt();
    m_problemInfo->adaptivityTolerance = eleProblem.toElement().attribute("adaptivitytolerance").toDouble();

    // harmonic
    m_problemInfo->frequency = eleProblem.toElement().attribute("frequency", "0").toDouble();

    // transient
    m_problemInfo->timeStep.text = eleProblem.toElement().attribute("timestep", "1");
    m_problemInfo->timeTotal.text = eleProblem.toElement().attribute("timetotal", "1");
    m_problemInfo->initialCondition.text = eleProblem.toElement().attribute("initialcondition", "0");

    // matrix solver
    m_problemInfo->matrixSolver = matrixSolverTypeFromStringKey(eleProblem.toElement().attribute("matrix_solver",
                                                                                                 matrixSolverTypeToStringKey(SOLVER_UMFPACK)));

    // startup script
    QDomNode eleScriptStartup = eleProblem.toElement().elementsByTagName("scriptstartup").at(0);
    m_problemInfo->scriptStartup = eleScriptStartup.toElement().text();

    // FIX ME - EOL conversion
    QPlainTextEdit textEdit;
    textEdit.setPlainText(m_problemInfo->scriptStartup);
    m_problemInfo->scriptStartup = textEdit.toPlainText();

    // description
    QDomNode eleDescription = eleProblem.toElement().elementsByTagName("description").at(0);
    m_problemInfo->description = eleDescription.toElement().text();

    // markers ***************************************************************************************************************

    // edge marker
    QDomNode eleEdgeMarkers = eleProblem.toElement().elementsByTagName("edges").at(0);
    n = eleEdgeMarkers.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        QString name = element.toElement().attribute("name");

        if (element.toElement().attribute("id") == 0)
        {
            // none marker
            addEdgeMarker(new SceneEdgeMarkerNone());
        }
        else
        {
            // read marker
            m_problemInfo->hermes()->readEdgeMarkerFromDomElement(&element.toElement());
        }

        n = n.nextSibling();
    }

    // label marker
    QDomNode eleLabelMarkers = eleProblem.toElement().elementsByTagName("labels").at(0);
    n = eleLabelMarkers.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        QString name = element.toElement().attribute("name");

        if (element.toElement().attribute("id") == 0)
        {
            // none marker
            addLabelMarker(new SceneLabelMarkerNone());
        }
        else
        {
            // read marker
            m_problemInfo->hermes()->readLabelMarkerFromDomElement(&element.toElement());
        }

        n = n.nextSibling();
    }

    // geometry ***************************************************************************************************************

    // geometry
    QDomNode eleGeometry = eleDoc.elementsByTagName("geometry").at(0);

    // nodes
    QDomNode eleNodes = eleGeometry.toElement().elementsByTagName("nodes").at(0);
    n = eleNodes.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        Point point = Point(element.attribute("x").toDouble(), element.attribute("y").toDouble());

        addNode(new SceneNode(point));
        n = n.nextSibling();
    }

    // edges
    QDomNode eleEdges = eleGeometry.toElement().elementsByTagName("edges").at(0);
    n = eleEdges.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        SceneNode *nodeFrom = nodes[element.attribute("start").toInt()];
        SceneNode *nodeTo = nodes[element.attribute("end").toInt()];
        SceneEdgeMarker *marker = edgeMarkers[element.attribute("marker").toInt()];
        double angle = element.attribute("angle", "0").toDouble();
        int refineTowardsEdge = element.attribute("refine_towards", "0").toInt();

        addEdge(new SceneEdge(nodeFrom, nodeTo, marker, angle, refineTowardsEdge));
        n = n.nextSibling();
    }

    // labels
    QDomNode eleLabels = eleGeometry.toElement().elementsByTagName("labels").at(0);
    n = eleLabels.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        Point point = Point(element.attribute("x").toDouble(), element.attribute("y").toDouble());
        SceneLabelMarker *marker = labelMarkers[element.attribute("marker").toInt()];
        double area = element.attribute("area", "0").toDouble();
        int polynomialOrder = element.attribute("polynomialorder", "0").toInt();

        addLabel(new SceneLabel(point, marker, area, polynomialOrder));
        n = n.nextSibling();
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    blockSignals(false);

    // default values
    emit invalidated();
    emit defaultValues();

    // mesh
    if (eleDoc.elementsByTagName("mesh").count() > 0)
    {
        QDomNode eleMesh = eleDoc.elementsByTagName("mesh").at(0);
        Util::scene()->sceneSolution()->loadMeshInitial(&eleMesh.toElement());
    }

    // solutions
    if (eleDoc.elementsByTagName("solutions").count() > 0)
    {
        QDomNode eleSolutions = eleDoc.elementsByTagName("solutions").at(0);
        Util::scene()->sceneSolution()->loadSolution(&eleSolutions.toElement());
        emit invalidated();
    }

    // run script
    runPythonScript(m_problemInfo->scriptStartup);

    return ErrorResult();
}

ErrorResult Scene::writeToFile(const QString &fileName)
{
    logMessage("Scene::writeToFile()");

    QSettings settings;

    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
    {
        QFileInfo fileInfo(fileName);
        settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
        m_problemInfo->fileName = fileName;
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDomDocument doc;

    // main document
    QDomElement eleDoc = doc.createElement("document");
    doc.appendChild(eleDoc);

    // problems
    QDomNode eleProblems = doc.createElement("problems");
    eleDoc.appendChild(eleProblems);
    // first problem
    QDomElement eleProblem = doc.createElement("problem");
    eleProblems.appendChild(eleProblem);
    // id
    eleProblem.setAttribute("id", 0);
    // name
    eleProblem.setAttribute("name", m_problemInfo->name);
    // problem type
    eleProblem.toElement().setAttribute("problemtype", problemTypeToStringKey(m_problemInfo->problemType));
    // analysis type
    eleProblem.setAttribute("analysistype", analysisTypeToStringKey(m_problemInfo->analysisType));
    // type
    eleProblem.setAttribute("type", physicFieldToStringKey(m_problemInfo->physicField()));
    // number of refinements
    eleProblem.setAttribute("numberofrefinements", m_problemInfo->numberOfRefinements);
    // polynomial order
    eleProblem.setAttribute("polynomialorder", m_problemInfo->polynomialOrder);
    // adaptivity
    eleProblem.setAttribute("adaptivitytype", adaptivityTypeToStringKey(m_problemInfo->adaptivityType));
    eleProblem.setAttribute("adaptivitysteps", m_problemInfo->adaptivitySteps);
    eleProblem.setAttribute("adaptivitytolerance", m_problemInfo->adaptivityTolerance);
    // harmonic magnetic
    eleProblem.setAttribute("frequency", m_problemInfo->frequency);
    // transient
    eleProblem.setAttribute("timestep", m_problemInfo->timeStep.text);
    eleProblem.setAttribute("timetotal", m_problemInfo->timeTotal.text);
    eleProblem.setAttribute("initialcondition", m_problemInfo->initialCondition.text);

    // matrix solver
    eleProblem.setAttribute("matrix_solver", matrixSolverTypeToStringKey(m_problemInfo->matrixSolver));

    // startup script
    QDomElement eleScriptStartup = doc.createElement("scriptstartup");
    eleScriptStartup.appendChild(doc.createTextNode(m_problemInfo->scriptStartup));
    eleProblem.appendChild(eleScriptStartup);

    // description
    QDomElement eleDescription = doc.createElement("description");
    eleDescription.appendChild(doc.createTextNode(m_problemInfo->description));
    eleProblem.appendChild(eleDescription);

    // geometry
    QDomNode eleGeometry = doc.createElement("geometry");
    eleDoc.appendChild(eleGeometry);

    // geometry ***************************************************************************************************************

    // nodes
    QDomNode eleNodes = doc.createElement("nodes");
    eleGeometry.appendChild(eleNodes);
    for (int i = 0; i<nodes.length(); i++)
    {
        QDomElement eleNode = doc.createElement("node");

        eleNode.setAttribute("id", i);
        eleNode.setAttribute("x", nodes[i]->point.x);
        eleNode.setAttribute("y", nodes[i]->point.y);

        eleNodes.appendChild(eleNode);
    }

    // edges
    QDomNode eleEdges = doc.createElement("edges");
    eleGeometry.appendChild(eleEdges);
    for (int i = 0; i<edges.length(); i++)
    {
        QDomElement eleEdge = doc.createElement("edge");

        eleEdge.setAttribute("id", i);
        eleEdge.setAttribute("start", nodes.indexOf(edges[i]->nodeStart));
        eleEdge.setAttribute("end", nodes.indexOf(edges[i]->nodeEnd));
        eleEdge.setAttribute("angle", edges[i]->angle);
        eleEdge.setAttribute("refine_towards", edges[i]->refineTowardsEdge);
        eleEdge.setAttribute("marker", edgeMarkers.indexOf(edges[i]->marker));

        eleEdges.appendChild(eleEdge);
    }

    // labels
    QDomNode eleLabels = doc.createElement("labels");
    eleGeometry.appendChild(eleLabels);
    for (int i = 0; i<labels.length(); i++)
    {
        QDomElement eleLabel = doc.createElement("label");

        eleLabel.setAttribute("id", i);
        eleLabel.setAttribute("x", labels[i]->point.x);
        eleLabel.setAttribute("y", labels[i]->point.y);
        eleLabel.setAttribute("area", labels[i]->area);
        eleLabel.setAttribute("polynomialorder", labels[i]->polynomialOrder);
        eleLabel.setAttribute("marker", labelMarkers.indexOf(labels[i]->marker));

        eleLabels.appendChild(eleLabel);
    }

    // markers ***************************************************************************************************************

    // edge markers
    QDomNode eleEdgeMarkers = doc.createElement("edges");
    eleProblem.appendChild(eleEdgeMarkers);
    for (int i = 1; i<edgeMarkers.length(); i++)
    {
        QDomElement eleEdgeMarker = doc.createElement("edge");

        eleEdgeMarker.setAttribute("id", i);
        eleEdgeMarker.setAttribute("name", edgeMarkers[i]->name);
        if (edgeMarkers[i]->type == PhysicFieldBC_None)
            eleEdgeMarker.setAttribute("type", "none");

        if (i > 0)
        {
            // write marker
            m_problemInfo->hermes()->writeEdgeMarkerToDomElement(&eleEdgeMarker, edgeMarkers[i]);
        }

        eleEdgeMarkers.appendChild(eleEdgeMarker);
    }

    // label markers
    QDomNode eleLabelMarkers = doc.createElement("labels");
    eleProblem.appendChild(eleLabelMarkers);
    for (int i = 1; i<labelMarkers.length(); i++)
    {
        QDomElement eleLabelMarker = doc.createElement("label");

        eleLabelMarker.setAttribute("id", i);
        eleLabelMarker.setAttribute("name", labelMarkers[i]->name);

        if (i > 0)
        {
            // write marker
            m_problemInfo->hermes()->writeLabelMarkerToDomElement(&eleLabelMarker, labelMarkers[i]);
        }

        eleLabelMarkers.appendChild(eleLabelMarker);
    }

    if (settings.value("Solver/SaveProblemWithSolution", false).value<bool>())
    {
        // mesh
        QDomNode eleMesh = doc.createElement("mesh");
        Util::scene()->sceneSolution()->saveMeshInitial(&doc, &eleMesh.toElement());
        eleDoc.appendChild(eleMesh);

        // solution
        QDomNode eleSolutions = doc.createElement("solutions");
        Util::scene()->sceneSolution()->saveSolution(&doc, &eleSolutions.toElement());
        eleDoc.appendChild(eleSolutions);
    }

    // save to file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved (%2).").
                           arg(fileName).
                           arg(file.errorString()));

    QTextStream out(&file);
    doc.save(out, 4);

    file.waitForBytesWritten(0);
    file.close();

    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
        emit fileNameChanged(QFileInfo(fileName).absoluteFilePath());

    emit invalidated();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return ErrorResult();
}
