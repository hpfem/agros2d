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
#include "sceneview_common.h"

#include "util.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenefunction.h"
#include "scenesolution.h"

#include "problemdialog.h"
#include "scenetransformdialog.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"
#include "progressdialog.h"

#include "pythonlabagros.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"

void ProblemInfo::clear()
{
    coordinateType = CoordinateType_Planar;
    name = QObject::tr("unnamed");
    date = QDate::currentDate();
    fileName = "";
    startupscript = "";
    description = "";

    // matrix solver
    matrixSolver = Hermes::SOLVER_UMFPACK;

    // mesh type
    meshType = MeshType_Triangle;

    // harmonic
    frequency = 0.0;

    // transient
    timeStep = Value("1.0", false);
    timeTotal = Value("1.0", false);
}

FieldInfo::FieldInfo(ProblemInfo* parent, QString fieldId)
{
    m_module = NULL;
    m_parent = parent;

    if (fieldId.isEmpty())
    {
        // default
        // read default field (Util::config() is not set)
        QSettings settings;
        m_fieldId = settings.value("General/DefaultPhysicField", "electrostatic").toString();

        bool check = false;
        std::map<std::string, std::string> modules = availableModules();
        for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
            if (m_fieldId.toStdString() == it->first)
            {
                check = true;
                break;
            }
        if (!check)
            m_fieldId = "electrostatic";
    }
    else
    {
        m_fieldId = fieldId;
    }

    clear();
}

FieldInfo::~FieldInfo()
{
    if (m_module) delete m_module;
}

void FieldInfo::clear()
{
    // module object
    setAnalysisType(AnalysisType_SteadyState);

    numberOfRefinements = 1;
    polynomialOrder = 2;
    adaptivityType = AdaptivityType_None;
    adaptivitySteps = 0;
    adaptivityTolerance = 1.0;

    initialCondition = Value("0.0", false);

    // weakforms
    weakFormsType = WeakFormsType_Compiled;

    // linearity
    linearityType = LinearityType_Linear;
    nonlinearTolerance = 1e-3;
    nonlinearSteps = 10;
}

void FieldInfo::setAnalysisType(AnalysisType analysisType)
{
    m_analysisType = analysisType;

    if (m_module) delete m_module;
    m_module = moduleFactory(m_fieldId.toStdString(),
                             coordinateType(),
                             m_analysisType);
}

//int FieldInfo::numberOfSolutions() const
//{
//    if (m_analysisType == AnalysisType_SteadyState)
//        return module()->steady_state_solutions;
//    else if (m_analysisType == AnalysisType_Transient)
//        return module()->transient_solutions;
//    else if (m_analysisType == AnalysisType_Harmonic)
//        return module()->harmonic_solutions;
//    else
//        return 0;
//}

ostream& operator<<(ostream& output, FieldInfo& id)
{
    output << "FieldInfo " << id.fieldId().toStdString();
    return output;
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
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd,  0, 0)); //TODO - do it better
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
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1, 0)); //TODO - do it better
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
    m_scene->addEdge(new SceneEdge(node1, node2, 90, 0)); // do it better
    m_scene->addEdge(new SceneEdge(node2, node3, 90, 0));
    m_scene->addEdge(new SceneEdge(node3, node4, 90, 0));
    m_scene->addEdge(new SceneEdge(node4, node1, 90, 0));
}

// ************************************************************************************************************************

NewMarkerAction::NewMarkerAction(QIcon icon, QObject* parent, QString field) :
    QAction(icon, tr(availableModules()[field.toStdString()].c_str()), parent),
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

// initialize pointer
Util *Util::m_singleton = NULL;

Util::Util()
{
    logMessage("Util::Util()");

    m_scene = new Scene();

    // script remote
    m_scriptEngineRemote = new ScriptEngineRemote();

    m_problem = new Problem();
    m_solutionStore = new SolutionStore();

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
    delete m_problem;
    delete m_solutionStore;
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
    //m_sceneSolution = new SceneSolution<double>();
    m_activeViewField = NULL;
    m_activeTimeStep = 0;

    connect(this, SIGNAL(invalidated()), this, SLOT(doInvalidated()));

    /// TODO
    //connect(m_sceneSolution, SIGNAL(solved()), this, SLOT(doInvalidated()));
    connect(this, SIGNAL(fieldsChanged()), this, SLOT(doFieldsChanged()));

    boundaries = new SceneBoundaryContainer();
    materials = new SceneMaterialContainer();

    nodes = new SceneNodeContainer();
    edges = new SceneEdgeContainer();
    labels = new SceneLabelContainer();

    clear();
}

Scene::~Scene()
{
    logMessage("Scene::~Scene()");

    clear();

    delete m_undoStack;

    // TODO write destructors or use shared_ptrs...
    delete boundaries;
    delete materials;
    delete nodes;
    delete edges;
    delete labels;
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

    std::map<std::string, std::string> modules = availableModules();

    for(std::map<std::string, std::string>::iterator iter = modules.begin(); iter!= modules.end(); ++iter){
        NewMarkerAction* action = new NewMarkerAction(icon("scene-edgemarker"), this, iter->first.c_str());
        connect(action, SIGNAL(triggered(QString)), this, SLOT(doNewBoundary(QString)));
        actNewBoundaries[iter->first.c_str()] = action;
    }

    actNewMaterial = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewMaterial->setShortcut(tr("Alt+M"));
    actNewMaterial->setStatusTip(tr("New material"));
    connect(actNewMaterial, SIGNAL(triggered()), this, SLOT(doNewMaterial()));

    for(std::map<std::string, std::string>::iterator iter = modules.begin(); iter!= modules.end(); ++iter)
    {
        NewMarkerAction* action = new NewMarkerAction(icon("scene-labelmarker"), this, iter->first.c_str());
        connect(action, SIGNAL(triggered(QString)), this, SLOT(doNewMaterial(QString)));
        actNewMaterials[iter->first.c_str()] = action;
    }

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    actClearSolutions = new QAction(icon(""), tr("Clear solution"), this);
    actClearSolutions->setStatusTip(tr("Clear solution"));
    connect(actClearSolutions, SIGNAL(triggered()), this, SLOT(doClearSolution()));

    actProblemProperties = new QAction(icon("document-properties"), tr("&Problem properties"), this);
    actProblemProperties->setShortcut(tr("F12"));
    actProblemProperties->setStatusTip(tr("Problem properties"));
    connect(actProblemProperties, SIGNAL(triggered()), this, SLOT(doProblemProperties()));
}

void Scene::clearSolutions()
{
    foreach (SceneSolution<double>* sceneSolution, m_sceneSolutions)
    {
        delete sceneSolution;
    }
    m_sceneSolutions.clear();
}

//void Scene::createSolutions()
//{
//    clearSolutions();
//    foreach(FieldInfo* fi, fieldInfos())
//    {
//        m_sceneSolutions[fi] = new SceneSolution<double>(fi);
//    }
//}

SceneNode *Scene::addNode(SceneNode *node)
{
    logMessage("SceneNode *Scene::addNode()");

    // clear solution
    clearSolutions();

    // check if node doesn't exists
    if(SceneNode* existing = nodes->get(node)){
        delete node;
        return existing;
    }

    nodes->add(node);
    if (!scriptIsRunning()) emit invalidated();

    return node;
}

void Scene::removeNode(SceneNode *node)
{
    logMessage("Scene::nodes->remove()");

    // clear solution
    clearSolutions();

    nodes->remove(node);
    // delete node;

    emit invalidated();
}

SceneNode *Scene::getNode(const Point &point)
{
    logMessage("SceneNode *Scene::getNode()");

    nodes->get(point);
}


SceneEdge *Scene::addEdge(SceneEdge *edge)
{
    logMessage("SceneEdge *Scene::addEdge");

    // clear solution
    clearSolutions();

    // check if edge doesn't exists
    if (SceneEdge* existing = edges->get(edge)){
        delete edge;
        return existing;
    }

    edges->add(edge);
    if (!scriptIsRunning()) emit invalidated();

    return edge;
}

void Scene::removeEdge(SceneEdge *edge)
{
    logMessage("Scene::edges->remove()");

    // clear solution
    clearSolutions();

    edges->remove(edge);
    // delete edge;

    emit invalidated();
}

SceneEdge *Scene::getEdge(const Point &pointStart, const Point &pointEnd, double angle)
{
    logMessage("SceneEdge *Scene::getEdge()");

    edges->get(pointStart, pointEnd, angle);
}

SceneLabel *Scene::addLabel(SceneLabel *label)
{
    logMessage("SceneLabel *Scene::addLabel");

    // clear solution
    clearSolutions();

    // check if label doesn't exists
    if(SceneLabel* existing = labels->get(label)){
        delete label;
        return existing;
    }

    labels->add(label);
    if (!scriptIsRunning()) emit invalidated();

    return label;
}

void Scene::removeLabel(SceneLabel *label)
{
    logMessage("Scene::label->remove()");

    // clear solution
    clearSolutions();

    labels->remove(label);
    // delete label;

    emit invalidated();
}

SceneLabel *Scene::getLabel(const Point &point)
{
    logMessage("SceneLabel *Scene::getLabel()");

    labels->get(point);
}

void Scene::addBoundary(SceneBoundary *boundary)
{
    logMessage("Scene::addBoundary()");

    boundaries->add(boundary);
    if (!scriptIsRunning()) emit invalidated();
}

void Scene::removeBoundary(SceneBoundary *boundary)
{
    logMessage("Scene::removeBoundary()");

    //TODO instead of setting NoneBoundary we now remove... rething
    edges->removeMarkerFromAll(boundary);
    boundaries->remove(boundary);
    // delete boundary;

    emit invalidated();
}

void Scene::setBoundary(SceneBoundary *boundary)
{
    logMessage("setEdgeBoundary()");

    edges->selected().addMarkerToAll(boundary);
    selectNone();
}

SceneBoundary *Scene::getBoundary(const QString &name)
{
    logMessage("SceneBoundary *Scene::getBoundary()");

    return boundaries->get(name);
}

void Scene::addMaterial(SceneMaterial *material)
{
    logMessage("Scene::addMaterial()");

    this->materials->add(material);
    if (!scriptIsRunning()) emit invalidated();
}


SceneMaterial *Scene::getMaterial(const QString &name)
{
    logMessage("Scene::SceneMaterial *Scene::getMaterial()");

    return materials->get(name);
}

void Scene::removeMaterial(SceneMaterial *material)
{
    logMessage("Scene::removeMaterial()");

    //TODO instead of setting NoneMaterial we now remove... rething
    labels->removeMarkerFromAll(material);
    this->materials->remove(material);
    // delete material;

    emit invalidated();
}

void Scene::setMaterial(SceneMaterial *material)
{
    logMessage("Scene::setLabelMaterial()");

    labels->selected().addMarkerToAll(material);
    selectNone();
}


void Scene::clear()
{
    logMessage("Scene::clear()");

    blockSignals(true);

    // clear problem
    //TODO - not good
    if (Util::singleton() && Util::problem())
        Util::problem()->clear();

    // clear couplings
    foreach (CouplingInfo* couplingInfo, m_couplingInfos)
        delete couplingInfo;
    m_couplingInfos.clear();

    m_undoStack->clear();

    clearSolutions();
    m_problemInfo->clear();

    QMapIterator<QString, FieldInfo *> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
    m_fieldInfos.clear();

    //TODO - allow "no field"
    m_activeViewField = new FieldInfo(m_problemInfo);
    addField(m_activeViewField);

    // geometry
    nodes->clear();
    edges->clear();
    labels->clear();

    // markers
    boundaries->clear();
    materials->clear();

    // none edge
    addBoundary(new SceneBoundaryNone());
    // none label
    addMaterial(new SceneMaterialNone());

    blockSignals(false);

    emit cleared();

    emit fileNameChanged(tr("unnamed"));
    emit invalidated();
}

RectPoint Scene::boundingBox() const
{
    logMessage("RectPoint Scene::boundingBox()");

    if (nodes->isEmpty())
    {
        return RectPoint(Point(-0.5, -0.5), Point(0.5, 0.5));
    }
    else
    {
        return nodes->boundingBox();
    }
}

void Scene::selectNone()
{
    logMessage("Scene::selectNone()");

    nodes->setSelected(false);
    edges->setSelected(false);
    labels->setSelected(false);
}

void Scene::selectAll(SceneGeometryMode sceneMode)
{
    logMessage("Scene::selectAll()");

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
    logMessage("Scene::deleteSelected()");

    m_undoStack->beginMacro(tr("Delete selected"));

    nodes->selected().deleteWithUndo(tr("Remove node"));
    edges->selected().deleteWithUndo(tr("Remove edge"));
    labels->selected().deleteWithUndo(tr("Remove label"));

    m_undoStack->endMacro();

    emit invalidated();
}

int Scene::selectedCount()
{
    logMessage("Scene::selectedCount()");

    return nodes->selected().length() +
            edges->selected().length() +
            labels->selected().length();
}

void Scene::highlightNone()
{
    logMessage("Scene::highlightNone()");

    nodes->setHighlighted(false);
    edges->setHighlighted(false);
    labels->setHighlighted(false);
}

void Scene::moveSelectedNodesAndEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy)
{
    QList<SceneEdge *> selectedEdges;
    QList<QPair<double, SceneNode *> > selectedNodes;

    foreach (SceneEdge *edge, edges->items())
    {
        if (edge->isSelected)
        {
            edge->nodeStart->isSelected = true;
            edge->nodeEnd->isSelected = true;
            selectedEdges.append(edge);
        }
    }

    foreach (SceneNode *node, nodes->items())
    {
        if (node->isSelected)
        {
            QPair<double, SceneNode *> pair;

            Point newPoint;
            if (mode == SceneTransformMode_Translate)
            {
                newPoint = node->point + point;
                // projection of the point to the real axis of the displacement vector
                pair.first = node->point.x * cos(point.angle()) + node->point.y * sin(point.angle());
            }
            else if (mode == SceneTransformMode_Rotate)
            {
                double distanceNode = (node->point - point).magnitude();
                double angleNode = (node->point - point).angle()/M_PI*180;

                newPoint = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));

                // projection of the point to the tangential axis of the displacement vector
                pair.first = 0;
            }
            else if (mode == SceneTransformMode_Scale)
            {
                newPoint = point + (node->point - point) * scaleFactor;
                //
                pair.first = ((abs(scaleFactor) > 1) ? 1.0 : -1.0) * (node->point - point).magnitude();
            }

            SceneNode *obstructNode = getNode(newPoint);
            if (obstructNode && !obstructNode->isSelected)
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
            newPoint = node->point + point;
        }
        else if (mode == SceneTransformMode_Rotate)
        {
            double distanceNode = (node->point - point).magnitude();
            double angleNode = (node->point - point).angle()/M_PI*180;

            newPoint = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
        }
        else if (mode == SceneTransformMode_Scale)
        {
            newPoint = point + (node->point - point) * scaleFactor;
        }

        if (!copy)
        {
            m_undoStack->push(new SceneNodeCommandEdit(node->point, newPoint));
            node->point = newPoint;
        }
        else
        {
            SceneNode *nodeNew = new SceneNode(newPoint);
            SceneNode *nodeAdded = addNode(nodeNew);

            if (nodeAdded == nodeNew)
                m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));

            nodeAdded->isSelected = true;
            node->isSelected = false;
        }
    }

    foreach (SceneEdge *edge, selectedEdges)
    {
        if (edge->isSelected)
        {
            edge->nodeStart->isSelected = false;
            edge->nodeEnd->isSelected = false;
            // this->checkEdge(edge);

            // add new edge
            if (copy)
            {
                Point newPointStart;
                Point newPointEnd;
                if (mode == SceneTransformMode_Translate)
                {
                    newPointStart = edge->nodeStart->point + point;
                    newPointEnd = edge->nodeEnd->point + point;
                }
                else if (mode == SceneTransformMode_Rotate)
                {
                    double distanceNodeStart = (edge->nodeStart->point - point).magnitude();
                    double angleNodeStart = (edge->nodeStart->point - point).angle()/M_PI*180;

                    newPointStart = point + Point(distanceNodeStart * cos((angleNodeStart + angle)/180.0*M_PI), distanceNodeStart * sin((angleNodeStart + angle)/180.0*M_PI));

                    double distanceNodeEnd = (edge->nodeEnd->point - point).magnitude();
                    double angleNodeEnd = (edge->nodeEnd->point - point).angle()/M_PI*180;

                    newPointEnd = point + Point(distanceNodeEnd * cos((angleNodeEnd + angle)/180.0*M_PI), distanceNodeEnd * sin((angleNodeEnd + angle)/180.0*M_PI));
                }
                else if (mode == SceneTransformMode_Scale)
                {
                    newPointStart = point + (edge->nodeStart->point - point) * scaleFactor;
                    newPointEnd = point + (edge->nodeEnd->point - point) * scaleFactor;
                }

                // add new edge
                SceneNode *newNodeStart = getNode(newPointStart);
                SceneNode *newNodeEnd = getNode(newPointEnd);
                if (newNodeStart && newNodeEnd)
                {
                    SceneEdge *newEdge = new SceneEdge(newNodeStart, newNodeEnd,
                                                       edge->angle, edge->refineTowardsEdge);
                    addEdge(newEdge);

                    m_undoStack->push(new SceneEdgeCommandAdd(newEdge->nodeStart->point,
                                                              newEdge->nodeEnd->point,
                                                              "TODO",
                                                              newEdge->angle,
                                                              newEdge->refineTowardsEdge));
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
        if (label->isSelected)
        {
            QPair<double, SceneLabel *> pair;

            Point newPoint;
            if (mode == SceneTransformMode_Translate)
            {
                newPoint = label->point + point;
                // projection of the point to the real axis of the displacement vector
                pair.first = label->point.x * cos(point.angle()) + label->point.y * sin(point.angle());
            }
            else if (mode == SceneTransformMode_Rotate)
            {
                double distanceLabel = (label->point - point).magnitude();
                double angleLabel = (label->point - point).angle()/M_PI*180;

                newPoint = point + Point(distanceLabel * cos((angleLabel + angle)/180.0*M_PI), distanceLabel * sin((angleLabel + angle)/180.0*M_PI));

                // projection of the point to the tangential axis of the displacement vector
                pair.first = 0;
            }
            else if (mode == SceneTransformMode_Scale)
            {
                newPoint = point + (label->point - point) * scaleFactor;
                pair.first = ((abs(scaleFactor) > 1) ? 1.0 : -1.0) * (label->point - point).magnitude();
            }

            SceneLabel *obstructLabel = getLabel(newPoint);
            if (obstructLabel && !obstructLabel->isSelected)
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
            newPoint = label->point + point;
        }
        else if (mode == SceneTransformMode_Rotate)
        {
            double distanceLabel = (label->point - point).magnitude();
            double angleLabel = (label->point - point).angle()/M_PI*180;

            newPoint = point + Point(distanceLabel * cos((angleLabel + angle)/180.0*M_PI), distanceLabel * sin((angleLabel + angle)/180.0*M_PI));
        }
        else if (mode == SceneTransformMode_Scale)
        {
            newPoint = point + (label->point - point) * scaleFactor;
        }

        if (!copy)
        {
            m_undoStack->push(new SceneLabelCommandEdit(label->point, newPoint));
            label->point = newPoint;
        }
        else
        {
            SceneLabel *labelNew = new SceneLabel(newPoint,
                                                  label->area,
                                                  label->polynomialOrder);
            SceneLabel *labelAdded = addLabel(labelNew);

            if (labelAdded == labelNew)
                m_undoStack->push(new SceneLabelCommandAdd(labelNew->point,
                                                           "TODO",
                                                           labelNew->area,
                                                           label->polynomialOrder));

            labelAdded->isSelected = true;
            label->isSelected = false;
        }
    }

    selectedLabels.clear();
}

void Scene::transformTranslate(const Point &point, bool copy)
{
    logMessage("Scene::transformTranslate()");

    m_undoStack->beginMacro(tr("Translation"));

    moveSelectedNodesAndEdges(SceneTransformMode_Translate, point, 0.0, 0.0, copy);
    moveSelectedLabels(SceneTransformMode_Translate, point, 0.0, 0.0, copy);

    m_undoStack->endMacro();
    emit invalidated();
}

void Scene::transformRotate(const Point &point, double angle, bool copy)
{
    logMessage("Scene::transformRotate()");

    m_undoStack->beginMacro(tr("Rotation"));

    moveSelectedNodesAndEdges(SceneTransformMode_Rotate, point, angle, 0.0, copy);
    moveSelectedLabels(SceneTransformMode_Rotate, point, angle, 0.0, copy);

    m_undoStack->endMacro();
    emit invalidated();
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy)
{
    logMessage("Scene::transformScale()");

    m_undoStack->beginMacro(tr("Scale"));

    moveSelectedNodesAndEdges(SceneTransformMode_Scale, point, 0.0, scaleFactor, copy);
    moveSelectedLabels(SceneTransformMode_Scale, point, 0.0, scaleFactor, copy);

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::doInvalidated()
{
    logMessage("Scene::doInvalidated()");

    actNewEdge->setEnabled((nodes->length() >= 2) && (boundaries->length() >= 1));
    actNewLabel->setEnabled(materials->length() >= 1);
    //actClearSolutions->setEnabled(m_sceneSolution->isSolved());  //TODO kdy umoznit mazani?
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

    SceneEdge *edge = new SceneEdge(nodes->at(0), nodes->at(1), 0, 0); //TODO - do it better
    if (edge->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneEdge *edgeAdded = addEdge(edge);
        if (edgeAdded == edge)
            m_undoStack->push(new SceneEdgeCommandAdd(edge->nodeStart->point,
                                                      edge->nodeEnd->point,
                                                      "TODO",
                                                      edge->angle,
                                                      edge->refineTowardsEdge));
    }
    else
        delete edge;
}

void Scene::doNewLabel(const Point &point)
{
    logMessage("Scene::doNewLabel()");

    SceneLabel *label = new SceneLabel(point, 0.0, 0); //TODO - do it better
    if (label->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneLabel *labelAdded = addLabel(label);
        if (labelAdded == label) m_undoStack->push(new SceneLabelCommandAdd(label->point,
                                                                            "TODO",
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
    doNewBoundary(Util::scene()->fieldInfo()->fieldId());
    }

void Scene::doNewBoundary(QString field)
{
    logMessage("Scene::doNewBoundary()");

    SceneBoundary *marker = Util::scene()->fieldInfo(field)->module()->newBoundary();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addBoundary(marker);
    else
        delete marker;
}

void Scene::doNewMaterial()
{
    doNewMaterial(Util::scene()->fieldInfo()->fieldId());
}

void Scene::doNewMaterial(QString field)
{
    logMessage("Scene::doNewMaterial()");

    SceneMaterial *marker = Util::scene()->fieldInfo(field)->module()->newMaterial();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addMaterial(marker);
    else
        delete marker;
}

void Scene::addField(FieldInfo *field)
{
    // add to the collection
    m_fieldInfos[field->fieldId()] = field;

    emit fieldsChanged();
    emit invalidated();
}

void Scene::removeField(FieldInfo *field)
{
    // first remove references to markers of this field from all edges and labels
    edges->removeFieldMarkers(field);
    labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    boundaries->removeFieldMarkers(field);
    materials->removeFieldMarkers(field);

//    // remove boundary conditions
//    foreach (SceneBoundary *boundary, field->module()->boundaries().items())
//        removeBoundary(boundary);

//    // remove materials
//    foreach (SceneMaterial *material, field->module()->materials().items())
//        removeMaterial(material);

    // remove from the collection
    m_fieldInfos.remove(field->fieldId());

    emit fieldsChanged();
    emit invalidated();
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

    clearSolutions();
    emit invalidated();
}

void Scene::doProblemProperties()
{
    logMessage("Scene::doProblemProperties()");

    ProblemDialog problemDialog(m_problemInfo, m_fieldInfos, m_couplingInfos,
                                false, QApplication::activeWindow());
    if (problemDialog.showDialog() == QDialog::Accepted)
    {
        emit invalidated();
    }

    /*
    //TODO - allow "no field"
    FieldInfo *fieldInfo = Util::scene()->fieldInfos().values().at(0);
    */
}

void Scene::addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent)
{
    if (Util::scene()->fieldInfos().count() == 1)
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
        foreach(FieldInfo* fieldInfo, fieldInfos())
            mnuSubBoundaries->addAction(actNewBoundaries[fieldInfo->fieldId()]);

        QMenu* mnuSubMaterials = new QMenu("New material", parent);
        menu->addMenu(mnuSubMaterials);
        foreach(FieldInfo* fieldInfo, fieldInfos())
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
    for (int i = 0; i<edges->length(); i++)
    {
        if (fabs(edges->at(i)->angle) < EPS_ZERO)
        {
            // line
            double x1 = edges->at(i)->nodeStart->point.x;
            double y1 = edges->at(i)->nodeStart->point.y;
            double x2 = edges->at(i)->nodeEnd->point.x;
            double y2 = edges->at(i)->nodeEnd->point.y;

            dxf->writeLine(*dw, DL_LineData(x1, y1, 0.0, x2, y2, 0.0), DL_Attributes("main", 256, -1, "BYLAYER"));
        }
        else
        {
            // arc
            double cx = edges->at(i)->center().x;
            double cy = edges->at(i)->center().y;
            double radius = edges->at(i)->radius();
            double angle1 = atan2(cy - edges->at(i)->nodeStart->point.y, cx - edges->at(i)->nodeStart->point.x)/M_PI*180.0 + 180.0;
            double angle2 = atan2(cy - edges->at(i)->nodeEnd->point.y, cx - edges->at(i)->nodeEnd->point.x)/M_PI*180.0 + 180.0;

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
    //TODO - allow "no field"
    m_fieldInfos.clear();

    m_problemInfo->fileName = fileName;
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
    ErrorResult error = validateXML(fileName, datadir() + "/resources/xsd/problem_a2d_xml.xsd");
    if (error.isError())
    {
        //qDebug() << error.message();
        return error;
    }

    // geometry ***************************************************************************************************************

    QDomNode eleGeometry = eleDoc.elementsByTagName("geometry").at(0);

    // nodes
    QDomNode eleNodes = eleGeometry.toElement().elementsByTagName("nodes").at(0);
    QDomNode nodeNode = eleNodes.firstChild();
    while (!nodeNode.isNull())
    {
        QDomElement element = nodeNode.toElement();

        Point point = Point(element.attribute("x").toDouble(), element.attribute("y").toDouble());

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
        int refineTowardsEdge = element.attribute("refine_towards", "0").toInt();

        SceneEdge *edge = new SceneEdge(nodeFrom, nodeTo, angle, refineTowardsEdge);
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
        int polynomialOrder = element.attribute("polynomial_order", "0").toInt();

        SceneLabel *label = new SceneLabel(point, area, polynomialOrder);
        addLabel(label);

        nodeLabel = nodeLabel.nextSibling();
    }

    // problem info
    QDomNode eleProblemInfo = eleDoc.elementsByTagName("problem").at(0);

    // name
    m_problemInfo->name = eleProblemInfo.toElement().attribute("name");
    // date
    m_problemInfo->date = QDate::fromString(eleProblemInfo.toElement().attribute("date", QDate::currentDate().toString(Qt::ISODate)), Qt::ISODate);
    // coordinate type
    m_problemInfo->coordinateType = coordinateTypeFromStringKey(eleProblemInfo.toElement().attribute("coordinate_type"));
    // mesh type
    m_problemInfo->meshType = meshTypeFromStringKey(eleProblemInfo.toElement().attribute("mesh_type",
                                                                               meshTypeToStringKey(MeshType_Triangle)));

    // harmonic
    m_problemInfo->frequency = eleProblemInfo.toElement().attribute("frequency", "0").toDouble();

    // transient
    m_problemInfo->timeStep.setText(eleProblemInfo.toElement().attribute("time_step", "1"));
    m_problemInfo->timeTotal.setText(eleProblemInfo.toElement().attribute("time_total", "1"));

    // matrix solver
    m_problemInfo->matrixSolver = matrixSolverTypeFromStringKey(eleProblemInfo.toElement().attribute("matrix_solver",
                                                                                                     matrixSolverTypeToStringKey(Hermes::SOLVER_UMFPACK)));

    // startup script
    QDomNode eleScriptStartup = eleProblemInfo.toElement().elementsByTagName("startup_script").at(0);
    m_problemInfo->startupscript = eleScriptStartup.toElement().text();

    // FIX ME - EOL conversion
    QPlainTextEdit textEdit;
    textEdit.setPlainText(m_problemInfo->startupscript);
    m_problemInfo->startupscript = textEdit.toPlainText();

    // description
    QDomNode eleDescription = eleProblemInfo.toElement().elementsByTagName("description").at(0);
    m_problemInfo->description = eleDescription.toElement().text();

    // field ***************************************************************************************************************

    QDomNode eleFields = eleProblemInfo.toElement().elementsByTagName("fields").at(0);
    QDomNode nodeField = eleFields.firstChild();
    while (!nodeField.isNull())
    {
        QDomNode eleField = nodeField.toElement();

        FieldInfo *field = new FieldInfo(m_problemInfo, eleField.toElement().attribute("field_id"));

        // analysis type
        field->setAnalysisType(analysisTypeFromStringKey(eleField.toElement().attribute("analysis_type",
                                                                                        analysisTypeToStringKey(AnalysisType_SteadyState))));

        // initial condition
        field->initialCondition.setText(eleField.toElement().attribute("initial_condition", "0"));

        // weakforms
        field->weakFormsType = weakFormsTypeFromStringKey(eleField.toElement().attribute("weak_forms",
                                                                                         weakFormsTypeToStringKey(WeakFormsType_Compiled)));

        // polynomial order
        field->polynomialOrder = eleField.toElement().attribute("polynomial_order").toInt();

        // number of refinements
        field->numberOfRefinements = eleField.toElement().attribute("number_of_refinements").toInt();

        // adaptivity
        QDomNode eleFieldAdaptivity = eleField.toElement().elementsByTagName("adaptivity").at(0);

        field->adaptivityType = adaptivityTypeFromStringKey(eleFieldAdaptivity.toElement().attribute("adaptivity_type"));
        field->adaptivitySteps = eleFieldAdaptivity.toElement().attribute("adaptivity_steps").toInt();
        field->adaptivityTolerance = eleFieldAdaptivity.toElement().attribute("adaptivity_tolerance").toDouble();

        // linearity
        QDomNode eleFieldLinearity = eleField.toElement().elementsByTagName("solver").at(0);

        field->linearityType = linearityTypeFromStringKey(eleFieldLinearity.toElement().attribute("linearity_type",
                                                                                                  linearityTypeToStringKey(LinearityType_Linear)));
        field->nonlinearSteps = eleFieldLinearity.toElement().attribute("nonlinear_steps", "10").toInt();
        field->nonlinearTolerance = eleFieldLinearity.toElement().attribute("nonlinear_tolerance", "1e-3").toDouble();

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
                                                        name.toStdString(),
                                                        type.toStdString());

            cout << "reading bc " << name.toStdString() << ", type " << type.toStdString() << endl;

            Hermes::Module::BoundaryType *boundary_type = field->module()->get_boundary_type(type.toStdString());
            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin();
                 it < boundary_type->variables.end(); ++it)
            {
                Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);

                cout << "setting variable " << variable->id << " to " <<  (element.toElement().attribute(QString::fromStdString(variable->id), "0")).toStdString() << endl;
                boundary->setValue(variable->id,
                                   Value(element.toElement().attribute(QString::fromStdString(variable->id), "0")));
            }

            Util::scene()->addBoundary(boundary);

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
            SceneMaterial *material = new SceneMaterial(field,
                                                        name.toStdString());
            Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = field->module()->material_type_variables;
            for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
            {
                Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);

                material->setValue(variable->id,
                                   Value(element.toElement().attribute(QString::fromStdString(variable->id),
                                                                       QString::number(variable->default_value))));
            }

            // add material
            Util::scene()->addMaterial(material);

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
        addField(field);

        // next field
        nodeField = nodeField.nextSibling();
    }

    // read config
    QDomElement config = eleDoc.elementsByTagName("config").at(0).toElement();
    Util::config()->loadPostprocessor(&config);

    blockSignals(false);

    // default values
    emit invalidated();
    emit defaultValues();

    // mesh
    if (eleDoc.elementsByTagName("mesh").count() > 0)
    {
        QDomNode eleMesh = eleDoc.elementsByTagName("mesh").at(0);
        Util::scene()->activeSceneSolution()->loadMeshInitial(eleMesh.toElement());
    }
    /*
    // solutions
    if (eleDoc.elementsByTagName("solutions").count() > 0)
    {
        QDomNode eleSolutions = eleDoc.elementsByTagName("solutions").at(0);
        Util::scene()->sceneSolution()->loadSolution(eleSolutions.toElement());
        emit invalidated();
    }
    */

    // run script
    runPythonScript(m_problemInfo->startupscript);

    return ErrorResult();
}

ErrorResult Scene::writeToFile(const QString &fileName)
{
    logMessage("Scene::writeToFile()");

    QSettings settings;

    // custom form
    /*
        if (m_problemInfo->fieldId() == "custom")
            if (!QFile::exists(m_problemInfo->fileName.left(m_problemInfo->fileName.size() - 4) + ".xml"))
            {
                QFile::remove(fileName.left(fileName.size() - 4) + ".xml");
                QFile::copy(m_problemInfo->fileName.left(m_problemInfo->fileName.size() - 4) + ".xml",
                            fileName.left(fileName.size() - 4) + ".xml");
            }
        */

    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
        {
            settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
            m_problemInfo->fileName = fileName;
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
        eleNode.setAttribute("x", node->point.x);
        eleNode.setAttribute("y", node->point.y);

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
        eleEdge.setAttribute("start", nodes->items().indexOf(edge->nodeStart));
        eleEdge.setAttribute("end", nodes->items().indexOf(edge->nodeEnd));
        eleEdge.setAttribute("angle", edge->angle);
        eleEdge.setAttribute("refine_towards", edge->refineTowardsEdge);

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
        eleLabel.setAttribute("x", label->point.x);
        eleLabel.setAttribute("y", label->point.y);
        eleLabel.setAttribute("area", label->area);
        eleLabel.setAttribute("polynomial_order", label->polynomialOrder);

        eleLabels.appendChild(eleLabel);

        ilabel++;
    }

    // problem info
    QDomElement eleProblem = doc.createElement("problem");
    eleDoc.appendChild(eleProblem);

    // name
    eleProblem.setAttribute("name", m_problemInfo->name);
    // date
    eleProblem.setAttribute("date", m_problemInfo->date.toString(Qt::ISODate));
    // coordinate type
    eleProblem.setAttribute("coordinate_type", coordinateTypeToStringKey(m_problemInfo->coordinateType));
    // mesh type
    eleProblem.setAttribute("mesh_type", meshTypeToStringKey(m_problemInfo->meshType));

    // harmonic
    eleProblem.setAttribute("frequency", m_problemInfo->frequency);

    // transient
    eleProblem.setAttribute("time_step", m_problemInfo->timeStep.text());
    eleProblem.setAttribute("time_total", m_problemInfo->timeTotal.text());

    // matrix solver
    eleProblem.setAttribute("matrix_solver", matrixSolverTypeToStringKey(m_problemInfo->matrixSolver));

    // startup script
    QDomElement eleScriptStartup = doc.createElement("startup_script");
    eleScriptStartup.appendChild(doc.createTextNode(m_problemInfo->startupscript));
    eleProblem.appendChild(eleScriptStartup);

    // description
    QDomElement eleDescription = doc.createElement("description");
    eleDescription.appendChild(doc.createTextNode(m_problemInfo->description));
    eleProblem.appendChild(eleDescription);

    // field ***************************************************************************************************************
    QDomNode eleFields = doc.createElement("fields");
    eleProblem.appendChild(eleFields);
    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        QDomElement eleField = doc.createElement("field");
        eleFields.appendChild(eleField);

        // fieldid
        eleField.setAttribute("field_id", fieldInfo->fieldId());
        // analysis type
        eleField.setAttribute("analysis_type", analysisTypeToStringKey(fieldInfo->analysisType()));
        // initial condition
        eleField.setAttribute("initial_condition", fieldInfo->initialCondition.text());
        // weakforms
        eleField.setAttribute("weak_forms", weakFormsTypeToStringKey(fieldInfo->weakFormsType));
        // polynomial order
        eleField.setAttribute("polynomial_order", fieldInfo->polynomialOrder);

        // number of refinements
        eleField.setAttribute("number_of_refinements", fieldInfo->numberOfRefinements);

        // adaptivity
        QDomElement eleAdaptivity = doc.createElement("adaptivity");
        eleField.appendChild(eleAdaptivity);
        eleAdaptivity.setAttribute("adaptivity_type", adaptivityTypeToStringKey(fieldInfo->adaptivityType));
        eleAdaptivity.setAttribute("adaptivity_steps", fieldInfo->adaptivitySteps);
        eleAdaptivity.setAttribute("adaptivity_tolerance", fieldInfo->adaptivityTolerance);

        // linearity
        QDomElement eleLinearity = doc.createElement("solver");
        eleField.appendChild(eleLinearity);
        eleLinearity.setAttribute("linearity_type", linearityTypeToStringKey(fieldInfo->linearityType));
        eleLinearity.setAttribute("nonlinear_steps", fieldInfo->nonlinearSteps);
        eleLinearity.setAttribute("nonlinear_tolerance", fieldInfo->nonlinearTolerance);

        // boundaries
        QDomNode eleBoundaries = doc.createElement("boundaries");
        eleField.appendChild(eleBoundaries);
        int iboundary = 1;
        foreach (SceneBoundary *boundary, boundaries->filter(fieldInfo).items())
        {
            QDomElement eleBoundary = doc.createElement("boundary");

            eleBoundary.setAttribute("name", QString::fromStdString(boundary->getName()));
            if (boundary->getType() == "")
                eleBoundary.setAttribute("type", "none");

            // if (iboundary > 0)
            {
                // write marker
                eleBoundary.setAttribute("id", iboundary);
                eleBoundary.setAttribute("type", QString::fromStdString(boundary->getType()));

                const std::map<std::string, Value> values = boundary->getValues();
                for (std::map<std::string, Value>::const_iterator it = values.begin(); it != values.end(); ++it)
                    eleBoundary.setAttribute(QString::fromStdString(it->first), it->second.toString());

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
            eleMaterial.setAttribute("name", QString::fromStdString(material->getName()));\

            const std::map<std::string, Value> values = material->getValues();
            for (std::map<std::string, Value>::const_iterator it = values.begin(); it != values.end(); ++it)
                eleMaterial.setAttribute(QString::fromStdString(it->first), it->second.toString());

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


    //    if (settings.value("Solver/SaveProblemWithSolution", false).value<bool>())
    //    {
    //        // mesh
    //        QDomNode eleMesh = doc.createElement("mesh");
    //        Util::scene()->sceneSolution()->saveMeshInitial(&doc, eleMesh.toElement());
    //        eleDoc.appendChild(eleMesh);

    //        // solution
    //        QDomNode eleSolutions = doc.createElement("solutions");
    //        Util::scene()->sceneSolution()->saveSolution(&doc, eleSolutions.toElement());
    //        eleDoc.appendChild(eleSolutions);
    //    }

    // save to file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved (%2).").
                           arg(fileName).
                           arg(file.errorString()));

    // save config
    QDomElement eleConfig = doc.createElement("config");
    eleDoc.appendChild(eleConfig);
    Util::config()->savePostprocessor(&eleConfig);

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

void Scene::synchronizeCouplings()
{
    CouplingInfo::synchronizeCouplings(m_fieldInfos, m_couplingInfos);
}

SceneSolution<double> *Scene::activeSceneSolution()
{
    return sceneSolution(FieldSolutionID(activeViewField(), activeTimeStep(), 0, SolutionType_Normal));
}

SceneSolution<double>* Scene::sceneSolution(FieldSolutionID fsid)
{
    if(! m_sceneSolutions.contains(fsid))
    {
        MultiSolutionArray<double> msa;
        if(Util::solutionStore()->contains(fsid))
            msa = Util::solutionStore()->multiSolution(fsid);
        SceneSolution<double>* sceneSolution = new SceneSolution<double>(fsid.group, msa);
        m_sceneSolutions.insert(fsid, sceneSolution);
    }
    else
    {
        m_sceneSolutions[fsid]->updateSolutionArray(Util::solutionStore()->multiSolution(fsid));
    }

    return m_sceneSolutions[fsid];
}
