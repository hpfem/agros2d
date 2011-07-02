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

#include "scripteditordialog.h"

#include "hermes2d/module.h"

void ProblemInfo::clear()
{
    problemType = ProblemType_Planar;
    analysisType = AnalysisType_SteadyState;

    // module object
    if (m_module) delete m_module;
    m_module = NULL;

    // read default field (Util::config() is not set)
    QSettings settings;
    QString defaultPhysicField = settings.value("General/DefaultPhysicField", "electrostatic").toString();

    bool check = false;
    std::map<std::string, std::string> modules = availableModules();
    for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
        if (defaultPhysicField.toStdString() == it->first)
        {
            check = true;
            break;
        }
    if (!check)
        defaultPhysicField = "electrostatic";

    setModule(moduleFactory(defaultPhysicField.toStdString(), problemType, analysisType));

    name = QObject::tr("unnamed");
    date = QDate::currentDate();
    fileName = "";
    scriptStartup = "";
    description = "";
    numberOfRefinements = 1;
    polynomialOrder = 2;
    meshType = MeshType_Triangle;
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

    // linearity
    linearityType = LinearityType_Linear;
    linearityNonlinearTolerance = 1e-3;
    linearityNonlinearSteps = 10;
}

void ProblemInfo::setModule(Hermes::Module::ModuleAgros *module)
{
    if (m_module) delete m_module;
    m_module = module;
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
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->boundaries[0], 0, 0));
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
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->boundaries[0], (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1, 0));
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
    m_scene->addEdge(new SceneEdge(node1, node2, m_scene->boundaries[0], 90, 0));
    m_scene->addEdge(new SceneEdge(node2, node3, m_scene->boundaries[0], 90, 0));
    m_scene->addEdge(new SceneEdge(node3, node4, m_scene->boundaries[0], 90, 0));
    m_scene->addEdge(new SceneEdge(node4, node1, m_scene->boundaries[0], 90, 0));
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

    actNewBoundary = new QAction(icon("scene-edgemarker"), tr("New &boundary condition..."), this);
    actNewBoundary->setShortcut(tr("Alt+B"));
    actNewBoundary->setStatusTip(tr("New boundary condition"));
    connect(actNewBoundary, SIGNAL(triggered()), this, SLOT(doNewBoundary()));

    actNewMaterial = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewMaterial->setShortcut(tr("Alt+M"));
    actNewMaterial->setStatusTip(tr("New material"));
    connect(actNewMaterial, SIGNAL(triggered()), this, SLOT(doNewMaterial()));

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    actClearSolution = new QAction(icon(""), tr("Clear solution"), this);
    actClearSolution->setStatusTip(tr("Clear solution"));
    connect(actClearSolution, SIGNAL(triggered()), this, SLOT(doClearSolution()));

    actProblemProperties = new QAction(icon("document-properties"), tr("&Problem properties"), this);
    actProblemProperties->setShortcut(tr("F12"));
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
            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->boundary->name,
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

void Scene::setBoundary(SceneBoundary *boundary)
{
    logMessage("setEdgeBoundary()");

    for (int i = 0; i<edges.count(); i++)
    {
        if (edges[i]->isSelected)
            edges[i]->boundary = boundary;
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

void Scene::setMaterial(SceneMaterial *material)
{
    logMessage("Scene::setLabelMaterial()");

    for (int i = 0; i<labels.count(); i++)
    {
        if (labels[i]->isSelected)
            labels[i]->material = material;
    }
    selectNone();
}

void Scene::addBoundary(SceneBoundary *boundary)
{
    logMessage("Scene::addBoundary()");

    boundaries.append(boundary);
    if (!scriptIsRunning()) emit invalidated();
}

void Scene::removeBoundary(SceneBoundary *boundary)
{
    logMessage("Scene::removeBoundary()");

    // set none marker
    foreach (SceneEdge *edge, edges)
    {
        if (edge->boundary == boundary)
            edge->boundary = boundaries[0];
    }
    this->boundaries.removeOne(boundary);
    // delete boundary;

    emit invalidated();
}

SceneBoundary *Scene::getBoundary(const QString &name)
{
    logMessage("SceneBoundary *Scene::getBoundary()");

    for (int i = 0; i<boundaries.count(); i++)
    {
        if (boundaries[i]->name == name)
            return boundaries[i];
    }
    return NULL;
}

bool Scene::setBoundary(const QString &name, SceneBoundary *boundary)
{
    logMessage("Scene::setBoundary()");

    for (int i = 1; i<boundaries.count(); i++)
    {
        if (boundaries[i]->name == name)
        {
            SceneBoundary *markerTemp = boundaries[i];

            // set new marker
            foreach (SceneEdge *edge, edges)
            {
                if (edge->boundary == boundaries[i])
                    edge->boundary = boundary;
            }

            // replace and delete old marker
            boundaries.replace(i, boundary);
            delete markerTemp;

            return true;
        }
    }

    return false;
}

void Scene::replaceBoundary(SceneBoundary *boundary)
{
    // store original name
    QString name = boundary->name;

    // add new marker
    SceneBoundary *markerNew = Util::scene()->problemInfo()->module()->newBoundary();
    Util::scene()->addBoundary(markerNew);

    // set edges to the new marker
    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        SceneEdge *edge = Util::scene()->edges[i];
        if (edge->boundary == boundary)
        {
            edge->boundary = markerNew;
        }
    }

    // remove old marker
    Util::scene()->removeBoundary(boundary);

    // set original name
    markerNew->name = name;
}

void Scene::addMaterial(SceneMaterial *material)
{
    logMessage("Scene::addMaterial()");

    this->materials.append(material);
    if (!scriptIsRunning()) emit invalidated();
}

bool Scene::setMaterial(const QString &name, SceneMaterial *material)
{
    logMessage("Scene::setMaterial()");

    for (int i = 1; i<materials.count(); i++)
    {
        if (materials[i]->name == name)
        {
            SceneMaterial *markerTemp = materials[i];

            // set new marker
            foreach (SceneLabel *label, labels)
            {
                if (label->material == materials[i])
                    label->material = material;
            }

            // replace and delete old marker
            materials.replace(i, material);
            delete markerTemp;

            return true;
        }
    }

    return false;
}

SceneMaterial *Scene::getMaterial(const QString &name)
{
    logMessage("Scene::SceneMaterial *Scene::getMaterial()");

    for (int i = 0; i<materials.count(); i++)
    {
        if (materials[i]->name == name)
            return materials[i];
    }
    return NULL;
}

void Scene::removeMaterial(SceneMaterial *material)
{
    logMessage("Scene::removeMaterial()");

    // set none marker
    foreach (SceneLabel *label, labels)
    {
        if (label->material == material)
            label->material = materials[0];
    }
    this->materials.removeOne(material);
    // delete material;

    emit invalidated();
}

void Scene::replaceMaterial(SceneMaterial *material)
{
    // store original name
    QString name = material->name;

    // add new marker
    SceneMaterial *markerNew = Util::scene()->problemInfo()->module()->newMaterial();
    Util::scene()->addMaterial(markerNew);

    // set labels to the new marker
    for (int i = 0; i < Util::scene()->labels.count(); i++)
    {
        SceneLabel *label = Util::scene()->labels[i];
        if (label->material == material)
        {
            label->material = markerNew;
        }
    }

    // remove old marker
    Util::scene()->removeMaterial(material);

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
    for (int i = 0; i < boundaries.count(); i++) delete boundaries[i];
    boundaries.clear();
    for (int i = 0; i < materials.count(); i++) delete materials[i];
    materials.clear();

    // none edge
    addBoundary(new SceneBoundaryNone());
    // none label
    addMaterial(new SceneMaterialNone());

    blockSignals(false);

    emit fileNameChanged(tr("unnamed"));
    emit invalidated();
}

RectPoint Scene::boundingBox() const
{
    logMessage("RectPoint Scene::boundingBox()");

    if (nodes.isEmpty())
    {
        return RectPoint(Point(-0.5, -0.5), Point(0.5, 0.5));
    }
    else
    {
        Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
        Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

        foreach (SceneNode *node, nodes)
        {
            min.x = qMin(min.x, node->point.x);
            max.x = qMax(max.x, node->point.x);
            min.y = qMin(min.y, node->point.y);
            max.y = qMax(max.y, node->point.y);
        }

        return RectPoint(min, max);
    }
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
            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->boundary->name,
                                                         edge->angle, edge->refineTowardsEdge));
            removeEdge(edge);
        }
    }

    foreach (SceneLabel *label, labels)
    {
        if (label->isSelected)
        {
            m_undoStack->push(new SceneLabelCommandRemove(label->point, label->material->name, label->area, label->polynomialOrder));
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
                SceneLabel *labelNew = new SceneLabel(pointNew, label->material, label->area, label->polynomialOrder);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->material->name, labelNew->area, label->polynomialOrder));
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
                SceneLabel *labelNew = new SceneLabel(pointNew, label->material, label->area, label->polynomialOrder);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->material->name, labelNew->area, labelNew->polynomialOrder));
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
                SceneLabel *labelNew = new SceneLabel(pointNew, label->material, label->area, label->polynomialOrder);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->material->name, labelNew->area, labelNew->polynomialOrder));
            }
        }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::doInvalidated()
{
    logMessage("Scene::doInvalidated()");

    actNewEdge->setEnabled((nodes.count() >= 2) && (boundaries.count() >= 1));
    actNewLabel->setEnabled(materials.count() >= 1);
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

    SceneEdge *edge = new SceneEdge(nodes[0], nodes[1], boundaries[0], 0, 0);
    if (edge->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneEdge *edgeAdded = addEdge(edge);
        if (edgeAdded == edge)
            m_undoStack->push(new SceneEdgeCommandAdd(edge->nodeStart->point,
                                                      edge->nodeEnd->point,
                                                      edge->boundary->name,
                                                      edge->angle,
                                                      edge->refineTowardsEdge));
    }
    else
        delete edge;
}

void Scene::doNewLabel(const Point &point)
{
    logMessage("Scene::doNewLabel()");

    SceneLabel *label = new SceneLabel(point, materials[0], 0.0, 0);
    if (label->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneLabel *labelAdded = addLabel(label);
        if (labelAdded == label) m_undoStack->push(new SceneLabelCommandAdd(label->point,
                                                                            label->material->name,
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

void Scene::doNewBoundary()
{
    logMessage("Scene::doNewBoundary()");

    SceneBoundary *marker = Util::scene()->problemInfo()->module()->newBoundary();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addBoundary(marker);
    else
        delete marker;
}

void Scene::doNewMaterial()
{
    logMessage("Scene::doNewMaterial()");

    SceneMaterial *marker = Util::scene()->problemInfo()->module()->newMaterial();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addMaterial(marker);
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

    QString scalar = "";
    QString vector = "";
    PhysicFieldVariableComp scalarComp = PhysicFieldVariableComp_Undefined;

    // previous value

    scalar = QString::fromStdString(sceneView()->sceneViewSettings().scalarPhysicFieldVariable);
    scalarComp = sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp;
    vector = QString::fromStdString(sceneView()->sceneViewSettings().vectorPhysicFieldVariable);

    ProblemDialog problemDialog(m_problemInfo, false, QApplication::activeWindow());
    // if (problemDialog.showDialog() == QDialog::Accepted)

    problemDialog.showDialog();

    // contour
    sceneView()->sceneViewSettings().contourPhysicFieldVariable = Util::scene()->problemInfo()->module()->view_default_scalar_variable->id;

    // scalar view
    // determines whether the selected field exists
    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = Util::scene()->problemInfo()->module()->view_scalar_variables.begin();
         it < Util::scene()->problemInfo()->module()->view_scalar_variables.end(); ++it )
    {
        Hermes::Module::LocalVariable *variable = ((Hermes::Module::LocalVariable *) *it);
        if (variable->id == scalar.toStdString())
        {
            sceneView()->sceneViewSettings().scalarPhysicFieldVariable = variable->id;
            sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = scalarComp;
        }
    }
    if (sceneView()->sceneViewSettings().scalarPhysicFieldVariable == "")
    {
        sceneView()->sceneViewSettings().scalarPhysicFieldVariable = Util::scene()->problemInfo()->module()->view_default_scalar_variable->id;
        sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = Util::scene()->problemInfo()->module()->view_default_scalar_variable_comp();
        sceneView()->sceneViewSettings().scalarRangeAuto = true;
    }

    // vector view
    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = Util::scene()->problemInfo()->module()->view_vector_variables.begin();
         it < Util::scene()->problemInfo()->module()->view_vector_variables.end(); ++it )
    {
        Hermes::Module::LocalVariable *variable = ((Hermes::Module::LocalVariable *) *it);
        if (variable->id == vector.toStdString())
            sceneView()->sceneViewSettings().vectorPhysicFieldVariable = variable->id;
    }
    if (sceneView()->sceneViewSettings().vectorPhysicFieldVariable == "")
        sceneView()->sceneViewSettings().vectorPhysicFieldVariable = Util::scene()->problemInfo()->module()->view_default_vector_variable->id;

    emit invalidated();
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
        if (fabs(edges[i]->angle) < EPS_ZERO)
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

    // check version of a2d file
    QString version = eleDoc.attribute("version");

    if (!version.isEmpty())
        return ErrorResult(ErrorResultType_Critical, tr("For opening file '%1' is required Agros2D version 2.0 or higher.").arg(fileName));

    // problems
    QDomNode eleProblems = eleDoc.elementsByTagName("problems").at(0);
    // first problem
    QDomNode eleProblem = eleProblems.toElement().elementsByTagName("problem").at(0);
    // name
    m_problemInfo->name = eleProblem.toElement().attribute("name");
    // date
    m_problemInfo->date = QDate::fromString(eleProblem.toElement().attribute("date", QDate::currentDate().toString(Qt::ISODate)), Qt::ISODate);
    // problem type                                                                                                                                                                                                                             `
    m_problemInfo->problemType = problemTypeFromStringKey(eleProblem.toElement().attribute("problemtype"));
    // analysis type
    m_problemInfo->analysisType = analysisTypeFromStringKey(eleProblem.toElement().attribute("analysistype",
                                                                                             analysisTypeToStringKey(AnalysisType_SteadyState)));
    // physic field
    m_problemInfo->setModule(moduleFactory(eleProblem.toElement().attribute("type").toStdString(),
                                           problemTypeFromStringKey(eleProblem.toElement().attribute("problemtype")),
                                           analysisTypeFromStringKey(eleProblem.toElement().attribute("analysistype", analysisTypeToStringKey(AnalysisType_SteadyState)))));
    // number of refinements
    m_problemInfo->numberOfRefinements = eleProblem.toElement().attribute("numberofrefinements").toInt();
    // polynomial order
    m_problemInfo->polynomialOrder = eleProblem.toElement().attribute("polynomialorder").toInt();
    // mesh type
    m_problemInfo->meshType = meshTypeFromStringKey(eleProblem.toElement().attribute("meshtype",
                                                                                     meshTypeToStringKey(MeshType_Triangle)));
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

    // linearity
    m_problemInfo->linearityType = linearityTypeFromStringKey(eleProblem.toElement().attribute("linearity",
                                                                                               linearityTypeToStringKey(LinearityType_Linear)));
    m_problemInfo->linearityNonlinearSteps = eleProblem.toElement().attribute("linearitysteps", "10").toInt();
    m_problemInfo->linearityNonlinearTolerance = eleProblem.toElement().attribute("linearitytolerance", "1e-3").toDouble();

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
    QDomNode eleBoundaries = eleProblem.toElement().elementsByTagName("edges").at(0);
    n = eleBoundaries.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        QString name = element.toElement().attribute("name");

        if (element.toElement().attribute("id") == 0)
        {
            // none marker
            addBoundary(new SceneBoundaryNone());
        }
        else
        {
            // read marker
            m_problemInfo->module()->readBoundaryFromDomElement(&element.toElement());
        }

        n = n.nextSibling();
    }

    // label marker
    QDomNode eleMaterials = eleProblem.toElement().elementsByTagName("labels").at(0);
    n = eleMaterials.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        QString name = element.toElement().attribute("name");

        if (element.toElement().attribute("id") == 0)
        {
            // none marker
            addMaterial(new SceneMaterialNone());
        }
        else
        {
            // read marker
            m_problemInfo->module()->readMaterialFromDomElement(&element.toElement());
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
        SceneBoundary *marker = boundaries[element.attribute("marker").toInt()];
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
        SceneMaterial *marker = materials[element.attribute("marker").toInt()];
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
        if (fileInfo.absoluteDir() != tempProblemDir())
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
    // date
    eleProblem.setAttribute("date", m_problemInfo->date.toString(Qt::ISODate));
    // problem type
    eleProblem.toElement().setAttribute("problemtype", problemTypeToStringKey(m_problemInfo->problemType));
    // analysis type
    eleProblem.setAttribute("analysistype", analysisTypeToStringKey(m_problemInfo->analysisType));
    // type
    eleProblem.setAttribute("type", QString::fromStdString(m_problemInfo->module()->id));
    // number of refinements
    eleProblem.setAttribute("numberofrefinements", m_problemInfo->numberOfRefinements);
    // polynomial order
    eleProblem.setAttribute("polynomialorder", m_problemInfo->polynomialOrder);
    // mesh type
    eleProblem.setAttribute("meshtype", meshTypeToStringKey(m_problemInfo->meshType));
    // adaptivity
    eleProblem.setAttribute("adaptivitytype", adaptivityTypeToStringKey(m_problemInfo->adaptivityType));
    eleProblem.setAttribute("adaptivitysteps", m_problemInfo->adaptivitySteps);
    eleProblem.setAttribute("adaptivitytolerance", m_problemInfo->adaptivityTolerance);
    eleProblem.setAttribute("maxdofs", Util::config()->maxDofs);
    // harmonic magnetic
    eleProblem.setAttribute("frequency", m_problemInfo->frequency);
    // transient
    eleProblem.setAttribute("timestep", m_problemInfo->timeStep.text);
    eleProblem.setAttribute("timetotal", m_problemInfo->timeTotal.text);
    eleProblem.setAttribute("initialcondition", m_problemInfo->initialCondition.text);
    // linearity
    eleProblem.setAttribute("linearity", linearityTypeToStringKey(m_problemInfo->linearityType));
    eleProblem.setAttribute("linearitysteps", m_problemInfo->linearityNonlinearSteps);
    eleProblem.setAttribute("linearitytolerance", m_problemInfo->linearityNonlinearTolerance);

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
        eleEdge.setAttribute("marker", boundaries.indexOf(edges[i]->boundary));

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
        eleLabel.setAttribute("marker", materials.indexOf(labels[i]->material));

        eleLabels.appendChild(eleLabel);
    }

    // markers ***************************************************************************************************************

    // edge markers
    QDomNode eleBoundarys = doc.createElement("edges");
    eleProblem.appendChild(eleBoundarys);
    for (int i = 1; i<boundaries.length(); i++)
    {
        QDomElement eleBoundary = doc.createElement("edge");

        eleBoundary.setAttribute("id", i);
        eleBoundary.setAttribute("name", boundaries[i]->name);
        if (boundaries[i]->type == PhysicFieldBC_None)
            eleBoundary.setAttribute("type", "none");

        if (i > 0)
        {
            // write marker
            m_problemInfo->module()->writeBoundaryToDomElement(&eleBoundary, boundaries[i]);
        }

        eleBoundarys.appendChild(eleBoundary);
    }

    // label markers
    QDomNode eleMaterials = doc.createElement("labels");
    eleProblem.appendChild(eleMaterials);
    for (int i = 1; i<materials.length(); i++)
    {
        QDomElement eleMaterial = doc.createElement("label");

        eleMaterial.setAttribute("id", i);
        eleMaterial.setAttribute("name", materials[i]->name);

        if (i > 0)
        {
            // write marker
            m_problemInfo->module()->writeMaterialToDomElement(&eleMaterial, materials[i]);
        }

        eleMaterials.appendChild(eleMaterial);
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
