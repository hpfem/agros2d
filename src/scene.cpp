#include "scene.h"

DxfFilter::DxfFilter(Scene *scene)
{
    this->m_scene = scene;
}

void DxfFilter::addLine(const DL_LineData &d)
{
    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(d.x1, d.y1)));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(d.x2, d.y2)));
    
    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->edgeMarkers[0], 0));
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
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->edgeMarkers[0], (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1));
}

// ************************************************************************************************************************

// initialize pointer
Util *Util::m_singleton = NULL;

Util::Util()
{
    m_scene = new Scene();
    m_helpDialog = new HelpDialog(QApplication::activeWindow());
}

Util::~Util()
{
    delete m_scene;
    delete m_helpDialog;
}

Util *Util::singleton()
{
    if (!m_singleton)
    {
        static QMutex mutex;
        mutex.lock();

        if (!m_singleton)
            m_singleton = new Util();

        mutex.unlock();
    }

    return m_singleton;
}

// ************************************************************************************************************************

Scene::Scene() {
    createActions();

    m_undoStack = new QUndoStack(this);
    m_sceneSolution = new SceneSolution(this);
    solverDialog = new SolverDialog(QApplication::activeWindow());
    connect(solverDialog, SIGNAL(solved()), this, SLOT(doSolved()));

    connect(this, SIGNAL(invalidated()), this, SLOT(doInvalidated()));

    clear();
}

Scene::~Scene() {
    // delete solverDialog;
    // delete m_sceneSolution;
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

    actNewEdgeMarker = new QAction(icon("scene-edgemarker"), tr("New &boundary condition..."), this);
    actNewEdgeMarker->setShortcut(tr("Alt+B"));
    actNewEdgeMarker->setStatusTip(tr("New boundary condition"));
    connect(actNewEdgeMarker, SIGNAL(triggered()), this, SLOT(doNewEdgeMarker()));

    actNewLabelMarker = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewLabelMarker->setShortcut(tr("Alt+M"));
    actNewLabelMarker->setStatusTip(tr("New material"));
    connect(actNewLabelMarker, SIGNAL(triggered()), this, SLOT(doNewLabelMarker()));

    actNewFunction = new QAction(icon(""), tr("New &function..."), this);
    actNewFunction->setShortcut(tr("Alt+F"));
    actNewFunction->setStatusTip(tr("New function"));
    connect(actNewFunction, SIGNAL(triggered()), this, SLOT(doNewFunction()));

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    actProblemProperties = new QAction(icon("scene-properties"), tr("&Problem properties"), this);
    actProblemProperties->setStatusTip(tr("Problem properties"));
    connect(actProblemProperties, SIGNAL(triggered()), this, SLOT(doProblemProperties()));
}

SceneNode *Scene::addNode(SceneNode *node)
{
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
    emit invalidated();

    return node;
}

void Scene::removeNode(SceneNode *node)
{
    // remove all edges connected to this node
    foreach (SceneEdge *edge, edges)
    {
        if ((edge->nodeStart == node) || (edge->nodeEnd == node))
        {
            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->marker->name, edge->angle));
            removeEdge(edge);
        }
    }

    nodes.removeOne(node);
    emit invalidated();
}

SceneNode *Scene::getNode(const Point &point)
{
    foreach (SceneNode *nodeCheck, nodes)
    {
        if (nodeCheck->point == point)
            return nodeCheck;
    }

    return NULL;
}


SceneEdge *Scene::addEdge(SceneEdge *edge)
{
    // check if edge doesn't exists
    foreach (SceneEdge *edgeCheck, edges)
    {
        if (((edgeCheck->nodeStart == edge->nodeStart) && (edgeCheck->nodeEnd == edge->nodeEnd)) ||
            ((edgeCheck->nodeStart == edge->nodeEnd) && (edgeCheck->nodeEnd == edge->nodeStart)) &&
            (fabs(edgeCheck->angle-edge->angle) < EPS_ZERO))
        {
            delete edge;
            return edgeCheck;
        }
    }

    edges.append(edge);
    emit invalidated();

    return edge;
}

void Scene::removeEdge(SceneEdge *edge)
{
    edges.removeOne(edge);
    emit invalidated();
}

SceneEdge *Scene::getEdge(const Point &pointStart, const Point &pointEnd)
{
    foreach (SceneEdge *edgeCheck, edges)
    {
        if (((edgeCheck->nodeStart->point == pointStart) && (edgeCheck->nodeEnd->point == pointEnd)) ||
            ((edgeCheck->nodeStart->point == pointEnd) && (edgeCheck->nodeEnd->point == pointStart)))
            return edgeCheck;
    }

    return NULL;
}

SceneLabel *Scene::addLabel(SceneLabel *label)
{
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
    emit invalidated();

    return label;
}

void Scene::removeLabel(SceneLabel *label)
{
    labels.removeOne(label);
    emit invalidated();
}

SceneLabel *Scene::getLabel(const Point &point)
{
    foreach (SceneLabel *labelCheck, labels)
    {
        if (labelCheck->point == point)
            return labelCheck;
    }
    return NULL;
}

void Scene::addEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    edgeMarkers.append(edgeMarker);
    emit invalidated();
}

void Scene::removeEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    // set none marker
    foreach (SceneEdge *edge, edges)
    {
        if (edge->marker == edgeMarker)
            edge->marker = edgeMarkers[0];
    }
    this->edgeMarkers.removeOne(edgeMarker);
    emit invalidated();
}

void Scene::setEdgeMarker(SceneEdgeMarker *edgeMarker)
{
    for (int i = 0; i<edges.count(); i++)
    {
        if (edges[i]->isSelected)
            edges[i]->marker = edgeMarker;
    }
    selectNone();
}

SceneEdgeMarker *Scene::getEdgeMarker(const QString &name)
{
    for (int i = 1; i<edgeMarkers.count(); i++)
    {
        if (edgeMarkers[i]->name == name)
            return edgeMarkers[i];
    }
    return NULL;
}

void Scene::addLabelMarker(SceneLabelMarker *labelMarker)
{
    this->labelMarkers.append(labelMarker);
    emit invalidated();
}

void Scene::removeLabelMarker(SceneLabelMarker *labelMarker)
{
    // set none marker
    foreach (SceneLabel *label, labels)
    {
        if (label->marker == labelMarker)
            label->marker = labelMarkers[0];
    }
    this->labelMarkers.removeOne(labelMarker);
    emit invalidated();
}

void Scene::setLabelMarker(SceneLabelMarker *labelMarker)
{
    for (int i = 0; i<labels.count(); i++)
    {
        if (labels[i]->isSelected)
            labels[i]->marker = labelMarker;
    }
    selectNone();
}

SceneLabelMarker *Scene::getLabelMarker(const QString &name)
{
    for (int i = 1; i<labelMarkers.count(); i++)
    {
        if (labelMarkers[i]->name == name)
            return labelMarkers[i];
    }
    return NULL;
}

SceneFunction *Scene::addFunction(SceneFunction *function)
{
    // check if function doesn't exists
    foreach (SceneFunction *functionCheck, functions)
    {
        if ((functionCheck->name == function->name) && (functionCheck->function == function->function))
            return functionCheck;
    }

    functions.append(function);
    emit invalidated();

    return function;
}

void Scene::removeFunction(SceneFunction *function)
{
    functions.removeOne(function);
    emit invalidated();
}

void Scene::clear()
{
    blockSignals(true);

    m_undoStack->clear();

    m_sceneSolution->clear();
    m_problemInfo.clear();

    // geometry
    nodes.clear();
    edges.clear();
    labels.clear();

    // markers
    edgeMarkers.clear();
    labelMarkers.clear();

    // functions
    functions.clear();

    // none edge
    addEdgeMarker(new SceneEdgeMarkerNone());
    // none label
    addLabelMarker(new SceneLabelMarkerNone());

    blockSignals(false);

    emit invalidated();
}

RectPoint Scene::boundingBox()
{
    Point min( 1e100,  1e100);
    Point max(-1e100, -1e100);

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
    foreach (SceneNode *node, nodes)
        node->isSelected = false;

    foreach (SceneEdge *edge, edges)
        edge->isSelected = false;

    foreach (SceneLabel *label, labels)
        label->isSelected = false;
}

void Scene::selectAll(SceneMode sceneMode)
{
    selectNone();

    switch (sceneMode)
    {
    case SCENEMODE_OPERATE_ON_NODES:
        foreach (SceneNode *node, nodes)
            node->isSelected = true;
        break;
    case SCENEMODE_OPERATE_ON_EDGES:
        foreach (SceneEdge *edge, edges)
            edge->isSelected = true;
        break;
    case SCENEMODE_OPERATE_ON_LABELS:
        foreach (SceneLabel *label, labels)
            label->isSelected = true;
        break;
    }
}

void Scene::deleteSelected()
{
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
            m_undoStack->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, edge->marker->name, edge->angle));
            removeEdge(edge);
        }
    }

    foreach (SceneLabel *label, labels)
    {
        if (label->isSelected)
        {
            m_undoStack->push(new SceneLabelCommandRemove(label->point, label->marker->name, label->area));
            removeLabel(label);
        }
    }

    m_undoStack->endMacro();
}

int Scene::selectedCount()
{
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
    foreach (SceneNode *node, nodes)
        node->isHighlighted = false;

    foreach (SceneEdge *edge, edges)
        edge->isHighlighted = false;

    foreach (SceneLabel *label, labels)
        label->isHighlighted = false;
}

void Scene::transformTranslate(const Point &point, bool copy)
{
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
                node->point = pointNew;
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
                label->point = pointNew;
            }
            else
            {
                SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area);
                SceneLabel *labelAdded = addLabel(labelNew);
                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->marker->name, labelNew->area));
            }
        }
    }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::transformRotate(const Point &point, double angle, bool copy)
{
    m_undoStack->beginMacro(tr("Rotation"));

    foreach (SceneNode *node, nodes)
        if (node->isSelected)
        {
        double distanceNode = (node->point - point).magnitude();
        double angleNode = (node->point - point).angle()/M_PI*180;

        Point pointNew = point + Point(distanceNode * cos((angleNode - angle)/180.0*M_PI), distanceNode * sin((angleNode - angle)/180.0*M_PI));
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

        Point pointNew = point + Point(distanceNode * cos((angleNode - angle)/180.0*M_PI), distanceNode * sin((angleNode - angle)/180.0*M_PI));
        if (!copy)
        {
            label->point = pointNew;
        }
        else
        {
            SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area);
            SceneLabel *labelAdded = addLabel(labelNew);
            if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->marker->name, labelNew->area));
        }
    }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy)
{
    m_undoStack->beginMacro(tr("Scale"));

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
            SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area);
            SceneLabel *labelAdded = addLabel(labelNew);
            if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, labelNew->marker->name, labelNew->area));
        }
    }

    m_undoStack->endMacro();

    emit invalidated();
}

void Scene::createMeshAndSolve(SolverMode solverMode)
{
    // check boundary conditions
    /*
    foreach (SceneEdge *edge, edges)
    {
        if (edge->marker->type == PHYSICFIELDBC_NONE)
        {
            selectNone();
            edge->isSelected = true;
            QMessageBox::warning(QApplication::activeWindow(),
                                 tr("Missing marker"),
                                 tr("Edge [%1; %2] - [%3; %4] has no marker.").arg(edge->nodeStart->point.x, 0, 'f', 3).arg(edge->nodeStart->point.y, 0, 'f', 3).arg(edge->nodeEnd->point.x, 0, 'f', 3).arg(edge->nodeEnd->point.y, 0, 'f', 3));
        }
    }
    */

    // clear problem
    sceneSolution()->clear();

    // store orig name
    QString fileNameOrig = m_problemInfo.fileName;

    // save as temp name
    m_problemInfo.fileName = tempProjectFileName() + ".a2d";

    // save problem
    writeToFile(m_problemInfo.fileName);

    // solve
    QFileInfo fileInfoOrig(fileNameOrig);
    solverDialog->setFileNameOrig(fileInfoOrig.absoluteFilePath());
    solverDialog->setMode(solverMode);
    solverDialog->show();
    solverDialog->solve();

    // restore orig name
    m_problemInfo.fileName = fileNameOrig;
}

void Scene::doSolved()
{
    solverDialog->hide();

    // set solver results
    if (m_sceneSolution->isSolved())
        emit solved();

    emit invalidated();

    // delete temp file
    if (m_problemInfo.fileName == tempProjectFileName() + ".a2d")
    {
        QFile::remove(m_problemInfo.fileName);
        m_problemInfo.fileName = "";
    }
}

void Scene::doInvalidated()
{
    actNewEdge->setEnabled((nodes.count() >= 2) && (edgeMarkers.count() >= 1));
    actNewLabel->setEnabled(labelMarkers.count() >= 1);
}

void Scene::doNewNode(const Point &point)
{
    SceneNode *node = new SceneNode(point);
    if (node->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
    {
        SceneNode *nodeAdded = addNode(node);
        if (nodeAdded == node) m_undoStack->push(new SceneNodeCommandAdd(node->point));
    }
    else
        delete node;
}

void Scene::doNewEdge()
{
    SceneEdge *edge = new SceneEdge(nodes[0], nodes[1], edgeMarkers[0], 0);
    if (edge->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
    {
        addEdge(edge);
    }
    else
        delete edge;
}

void Scene::doNewLabel()
{
    SceneLabel *label = new SceneLabel(Point(), labelMarkers[0], 0);
    if (label->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
    {
        addLabel(label);
    }
    else
        delete label;
}

void Scene::doDeleteSelected()
{
    deleteSelected();
}

void Scene::doNewEdgeMarker()
{
    SceneEdgeMarker *marker;
    switch (m_problemInfo.physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        marker = new SceneEdgeElectrostaticMarker("new boundary", PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL, Value("0"));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        marker = new SceneEdgeMagnetostaticMarker("new boundary", PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL, Value("0"));
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        marker = new SceneEdgeHarmonicMagneticMarker("new boundary", PHYSICFIELDBC_HARMONIC_MAGNETIC_VECTOR_POTENTIAL, Value("0"));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        marker = new SceneEdgeHeatMarker("new boundary", PHYSICFIELDBC_HEAT_TEMPERATURE, Value("0"));
        break;
    case PHYSICFIELD_CURRENT:
        marker = new SceneEdgeCurrentMarker("new boundary", PHYSICFIELDBC_CURRENT_POTENTIAL, Value("0"));
        break;
    case PHYSICFIELD_ELASTICITY:
        marker = new SceneEdgeElasticityMarker("new boundary", PHYSICFIELDBC_ELASTICITY_FREE, PHYSICFIELDBC_ELASTICITY_FREE, 0, 0);
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(m_problemInfo.physicField).toStdString() + "' is not implemented. Scene::doNewEdgeMarker()" << endl;
        throw;
        break;
    }

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
    {
        addEdgeMarker(marker);
    }
    else
        delete marker;
}

void Scene::doNewLabelMarker()
{
    SceneLabelMarker *marker;
    switch (m_problemInfo.physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        marker = new SceneLabelElectrostaticMarker("new material",  Value("0"), Value("1"));
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        marker = new SceneLabelMagnetostaticMarker("new material", Value("0"), Value("1"));
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        marker = new SceneLabelHarmonicMagneticMarker("new material", Value("0"), Value("0"), Value("1"), Value("0"));
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        marker = new SceneLabelHeatMarker("new material", Value("0"), Value("385"));
        break;
    case PHYSICFIELD_CURRENT:
        marker = new SceneLabelCurrentMarker("new material", Value("57e6"));
        break;
    case PHYSICFIELD_ELASTICITY:
        marker = new SceneLabelElasticityMarker("new material", 2e11, 0.33);
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(m_problemInfo.physicField).toStdString() + "' is not implemented. Scene::doNewLabelMarker()" << endl;
        throw;
        break;
    }

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
    {
        addLabelMarker(marker);
    }
    else
        delete marker;
}

void Scene::doNewFunction()
{
    SceneFunction *function = new SceneFunction(tr("unnamed function"), "x");
    if (function->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
    {
        addFunction(function);
    }
    else
        delete function;
}

void Scene::doTransform()
{
    SceneTransformDialog *sceneTransformDialog = new SceneTransformDialog(QApplication::activeWindow());
    sceneTransformDialog->exec();
    delete sceneTransformDialog;
}

void Scene::doProblemProperties()
{
    ProblemDialog *problemDialog = new ProblemDialog(m_problemInfo, false, QApplication::activeWindow());
    if (problemDialog->showDialog() == QDialog::Accepted)
    {
        emit invalidated();
    }
    delete problemDialog;
}



void Scene::writeToDxf(const QString &fileName)
{
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    DL_Dxf* dxf = new DL_Dxf();
    DL_Codes::version exportVersion = DL_Codes::AC1015;
    DL_WriterA *dw = dxf->out(fileName.toStdString().c_str(), exportVersion);
    if (dw == NULL) {
        cerr << fileName.toStdString() << " could not be opened." << endl;
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
                            std::string(""),      // leave empty
                            DL_Codes::black,      // default color
                            100,                  // default width
                            "CONTINUOUS"));       // default line style

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
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    blockSignals(true);

    DxfFilter *filter = new DxfFilter(this);
    DL_Dxf* dxf = new DL_Dxf();
    if (!dxf->in(fileName.toStdString(), filter)) {
        cerr << fileName.toStdString() << " could not be opened." << endl;
        return;
    }

    delete dxf;
    delete filter;

    blockSignals(false);

    emit invalidated();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void Scene::readFromFile(const QString &fileName)
{
    QSettings settings;
    QFileInfo fileInfo(fileName);
    settings.setValue("General/LastDataDir", fileInfo.absolutePath());

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    clear();
    m_problemInfo.fileName = fileName;

    blockSignals(true);

    QDomDocument doc;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file)) {
        file.close();
        return;
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
    m_problemInfo.name = eleProblem.toElement().attribute("name");
    // problem type                                                                                                                                                                                                                             `
    if (eleProblem.toElement().attribute("problemtype") == problemTypeStringKey(PROBLEMTYPE_PLANAR)) m_problemInfo.problemType = PROBLEMTYPE_PLANAR;
    if (eleProblem.toElement().attribute("problemtype") == problemTypeStringKey(PROBLEMTYPE_AXISYMMETRIC)) m_problemInfo.problemType = PROBLEMTYPE_AXISYMMETRIC;
    // physic field
    m_problemInfo.physicField = physicFieldFromStringKey(eleProblem.toElement().attribute("type"));
    // number of refinements
    m_problemInfo.numberOfRefinements = eleProblem.toElement().attribute("numberofrefinements").toInt();
    // polynomial order
    m_problemInfo.polynomialOrder = eleProblem.toElement().attribute("polynomialorder").toInt();
    // adaptivity
    if (eleProblem.toElement().attribute("adaptivitytype") == adaptivityTypeStringKey(ADAPTIVITYTYPE_NONE)) m_problemInfo.adaptivityType = ADAPTIVITYTYPE_NONE;
    if (eleProblem.toElement().attribute("adaptivitytype") == adaptivityTypeStringKey(ADAPTIVITYTYPE_H)) m_problemInfo.adaptivityType = ADAPTIVITYTYPE_H;
    if (eleProblem.toElement().attribute("adaptivitytype") == adaptivityTypeStringKey(ADAPTIVITYTYPE_P)) m_problemInfo.adaptivityType = ADAPTIVITYTYPE_P;
    if (eleProblem.toElement().attribute("adaptivitytype") == adaptivityTypeStringKey(ADAPTIVITYTYPE_HP)) m_problemInfo.adaptivityType = ADAPTIVITYTYPE_HP;
    m_problemInfo.adaptivitySteps = eleProblem.toElement().attribute("adaptivitysteps").toInt();
    m_problemInfo.adaptivityTolerance = eleProblem.toElement().attribute("adaptivitytolerance").toDouble();
    // time harmonic
    m_problemInfo.frequency = eleProblem.toElement().attribute("frequency").toDouble();

    // startup script
    QDomNode eleSriptStartup = eleProblem.toElement().elementsByTagName("scriptstartup").at(0);
    m_problemInfo.scriptStartup = eleSriptStartup.toElement().text();

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
            PhysicFieldBC type;
            PhysicFieldBC typeX;
            PhysicFieldBC typeY;
            switch (m_problemInfo.physicField)
            {
            case PHYSICFIELD_ELECTROSTATIC:
                // electrostatic markers
                if (element.toElement().attribute("type") == "none") type = PHYSICFIELDBC_NONE;
                if (element.toElement().attribute("type") == "potential") type = PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL;
                if (element.toElement().attribute("type") == "surface_charge_density") type = PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE;
                addEdgeMarker(new SceneEdgeElectrostaticMarker(name,
                                                               type,
                                                               Value(element.toElement().attribute("value"))));
                break;
            case PHYSICFIELD_MAGNETOSTATIC:
                // magnetostatic markers
                if (element.toElement().attribute("type") == "none") type = PHYSICFIELDBC_NONE;
                if (element.toElement().attribute("type") == "vector_potential") type = PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL;
                if (element.toElement().attribute("type") == "surface_current_density") type = PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT;
                addEdgeMarker(new SceneEdgeMagnetostaticMarker(name,
                                                               type,
                                                               Value(element.toElement().attribute("value"))));
                break;
            case PHYSICFIELD_HARMONIC_MAGNETIC:
                // harmonic magnetic markers
                if (element.toElement().attribute("type") == "none") type = PHYSICFIELDBC_NONE;
                if (element.toElement().attribute("type") == "vector_potential") type = PHYSICFIELDBC_HARMONIC_MAGNETIC_VECTOR_POTENTIAL;
                if (element.toElement().attribute("type") == "surface_current_density") type = PHYSICFIELDBC_HARMONIC_MAGNETIC_SURFACE_CURRENT;
                addEdgeMarker(new SceneEdgeHarmonicMagneticMarker(name,
                                                                  type,
                                                                  Value(element.toElement().attribute("value"))));
                break;
            case PHYSICFIELD_HEAT_TRANSFER:
                // heat markers
                if (element.toElement().attribute("type") == "none") type = PHYSICFIELDBC_NONE;
                if (element.toElement().attribute("type") == "temperature")
                {
                    type = PHYSICFIELDBC_HEAT_TEMPERATURE;
                    addEdgeMarker(new SceneEdgeHeatMarker(name,
                                                          type,
                                                          Value(element.toElement().attribute("temperature"))));
                }
                if (element.toElement().attribute("type") == "heat_flux")
                {
                    type = PHYSICFIELDBC_HEAT_HEAT_FLUX;
                    addEdgeMarker(new SceneEdgeHeatMarker(name, type,
                                                          Value(element.toElement().attribute("heat_flux")),
                                                          Value(element.toElement().attribute("h")),
                                                          Value(element.toElement().attribute("external_temperature"))));
                }
                break;
            case PHYSICFIELD_CURRENT:
                // current markers
                if (element.toElement().attribute("type") == "none") type = PHYSICFIELDBC_NONE;
                if (element.toElement().attribute("type") == "potential") type = PHYSICFIELDBC_CURRENT_POTENTIAL;
                if (element.toElement().attribute("type") == "inward_current_flow") type = PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW;
                addEdgeMarker(new SceneEdgeCurrentMarker(name,
                                                         type,
                                                         Value(element.toElement().attribute("value"))));
                break;
            case PHYSICFIELD_ELASTICITY:
                {
                    // elasticity markers
                    if (element.toElement().attribute("typex") == "none") typeX = PHYSICFIELDBC_NONE;
                    if (element.toElement().attribute("typey") == "none") typeY = PHYSICFIELDBC_NONE;

                    if (element.toElement().attribute("typex") == "fixed") typeX = PHYSICFIELDBC_ELASTICITY_FIXED;
                    if (element.toElement().attribute("typex") == "free") typeX = PHYSICFIELDBC_ELASTICITY_FREE;
                    if (element.toElement().attribute("typey") == "fixed") typeY = PHYSICFIELDBC_ELASTICITY_FIXED;
                    if (element.toElement().attribute("typey") == "free") typeY = PHYSICFIELDBC_ELASTICITY_FREE;

                    addEdgeMarker(new SceneEdgeElasticityMarker(name, typeX, typeY,
                                                                element.toElement().attribute("forcex").toDouble(),
                                                                element.toElement().attribute("forcey").toDouble()));
                }
                break;
            default:
                cerr << "Physical field '" + physicFieldStringKey(m_problemInfo.physicField).toStdString() + "' is not implemented. Scene::readFromFile(const QString &fileName)" << endl;
                throw;
                break;
            }
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
            switch (m_problemInfo.physicField)
            {
            case PHYSICFIELD_ELECTROSTATIC:
                // electrostatic markers
                addLabelMarker(new SceneLabelElectrostaticMarker(name,
                                                                 Value(element.toElement().attribute("charge_density")),
                                                                 Value(element.toElement().attribute("permittivity"))));
                break;
            case PHYSICFIELD_MAGNETOSTATIC:
                // magnetostatic markers
                addLabelMarker(new SceneLabelMagnetostaticMarker(name,
                                                                 Value(element.toElement().attribute("current_density")),
                                                                 Value(element.toElement().attribute("permeability"))));
                break;
            case PHYSICFIELD_HARMONIC_MAGNETIC:
                // magnetostatic markers
                addLabelMarker(new SceneLabelHarmonicMagneticMarker(name,
                                                                    Value(element.toElement().attribute("current_density_real")),
                                                                    Value(element.toElement().attribute("current_density_imag")),
                                                                    Value(element.toElement().attribute("permeability")),
                                                                    Value(element.toElement().attribute("conductivity"))));
                break;
            case PHYSICFIELD_HEAT_TRANSFER:
                // heat markers
                addLabelMarker(new SceneLabelHeatMarker(name,
                                                        Value(element.toElement().attribute("volume_heat")),
                                                        Value(element.toElement().attribute("thermal_conductivity"))));
                break;
            case PHYSICFIELD_CURRENT:
                // current markers
                addLabelMarker(new SceneLabelCurrentMarker(name,
                                                           Value(element.toElement().attribute("conductivity"))));
                break;
            case PHYSICFIELD_ELASTICITY:
                // heat markers
                addLabelMarker(new SceneLabelElasticityMarker(name,
                                                              element.toElement().attribute("young_modulus").toDouble(),
                                                              element.toElement().attribute("poisson_ratio").toDouble()));
                break;            default:
                        cerr << "Physical field '" + physicFieldStringKey(m_problemInfo.physicField).toStdString() + "' is not implemented. Scene::readFromFile(const QString &fileName)" << endl;
                throw;
                break;
            }
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
        double angle = element.attribute("angle").toDouble();

        addEdge(new SceneEdge(nodeFrom, nodeTo, marker, angle));
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
        double area = element.attribute("area").toDouble();

        addLabel(new SceneLabel(point, marker, area));
        n = n.nextSibling();
    }

    // functions
    QDomNode eleFunctions = eleDoc.elementsByTagName("functions").at(0);
    n = eleFunctions.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();

        addFunction(new SceneFunction(element.attribute("name"), element.attribute("function")));
        n = n.nextSibling();
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    blockSignals(false);

    emit invalidated();
}

void Scene::writeToFile(const QString &fileName) {
    if (problemInfo().fileName != QDir::temp().absolutePath() + "/agros2d/temp.a2d")
    {
        QSettings settings;
        QFileInfo fileInfo(fileName);
        settings.setValue("General/LastDataDir", fileInfo.absoluteFilePath());
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    m_problemInfo.fileName = fileName;

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
    eleProblem.setAttribute("name", m_problemInfo.name);
    // problem type                                                                                                                                                                                                                             `
    if (m_problemInfo.problemType == PROBLEMTYPE_PLANAR) eleProblem.toElement().setAttribute("problemtype", "planar");
    if (m_problemInfo.problemType == PROBLEMTYPE_AXISYMMETRIC) eleProblem.toElement().setAttribute("problemtype", "axisymmetric");
    // name
    eleProblem.setAttribute("type", physicFieldStringKey(m_problemInfo.physicField));
    // number of refinements
    eleProblem.setAttribute("numberofrefinements", m_problemInfo.numberOfRefinements);
    // polynomial order
    eleProblem.setAttribute("polynomialorder", m_problemInfo.polynomialOrder);
    // adaptivity
    eleProblem.setAttribute("adaptivitytype", adaptivityTypeStringKey(m_problemInfo.adaptivityType));
    eleProblem.setAttribute("adaptivitysteps", m_problemInfo.adaptivitySteps);
    eleProblem.setAttribute("adaptivitytolerance", m_problemInfo.adaptivityTolerance);
    // time harmonic
    eleProblem.setAttribute("frequency", m_problemInfo.frequency);

    // startup script
    QDomElement eleSriptStartup = doc.createElement("scriptstartup");
    eleSriptStartup.appendChild(doc.createTextNode(m_problemInfo.scriptStartup));
    eleProblem.appendChild(eleSriptStartup);

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
        if (edgeMarkers[i]->type == PHYSICFIELDBC_NONE) eleEdgeMarker.setAttribute("type", "none");

        if (i > 0)
        {
            // electrostatic
            if (SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(edgeMarkers[i]))
            {
                eleEdgeMarker.setAttribute("type", physicFieldBCStringKey(edgeElectrostaticMarker->type));
                eleEdgeMarker.setAttribute("value", edgeElectrostaticMarker->value.text);
            }
            // magnetostatic
            if (SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(edgeMarkers[i]))
            {
                eleEdgeMarker.setAttribute("type", physicFieldBCStringKey(edgeMagnetostaticMarker->type));
                eleEdgeMarker.setAttribute("value", edgeMagnetostaticMarker->value.text);
            }
            // harmonic magnetic
            if (SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker = dynamic_cast<SceneEdgeHarmonicMagneticMarker *>(edgeMarkers[i]))
            {
                eleEdgeMarker.setAttribute("type", physicFieldBCStringKey(edgeHarmonicMagneticMarker->type));
                eleEdgeMarker.setAttribute("value", edgeHarmonicMagneticMarker->value.text);
            }
            // heat transfer
            if (SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(edgeMarkers[i]))
            {
                eleEdgeMarker.toElement().setAttribute("type", physicFieldBCStringKey(edgeHeatMarker->type));
                if (edgeHeatMarker->type == PHYSICFIELDBC_HEAT_TEMPERATURE)
                {
                    eleEdgeMarker.setAttribute("temperature", edgeHeatMarker->temperature.text);
                }
                if (edgeHeatMarker->type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
                {
                    eleEdgeMarker.setAttribute("heat_flux", edgeHeatMarker->heatFlux.text);
                    eleEdgeMarker.setAttribute("h", edgeHeatMarker->h.text);
                    eleEdgeMarker.setAttribute("external_temperature", edgeHeatMarker->externalTemperature.text);
                }
            }
            // current
            if (SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(edgeMarkers[i]))
            {
                eleEdgeMarker.setAttribute("type", physicFieldBCStringKey(edgeCurrentMarker->type));
                eleEdgeMarker.setAttribute("value", edgeCurrentMarker->value.text);
            }
            // elasticity
            if (SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(edgeMarkers[i]))
            {
                eleEdgeMarker.setAttribute("typex", physicFieldBCStringKey(edgeElasticityMarker->typeX));
                eleEdgeMarker.setAttribute("typey", physicFieldBCStringKey(edgeElasticityMarker->typeY));
                eleEdgeMarker.setAttribute("forcex", edgeElasticityMarker->forceX);
                eleEdgeMarker.setAttribute("forcey", edgeElasticityMarker->forceY);
            }
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
            // electrostatic
            if (SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarkers[i]))
            {
                eleLabelMarker.setAttribute("charge_density", labelElectrostaticMarker->charge_density.text);
                eleLabelMarker.setAttribute("permittivity", labelElectrostaticMarker->permittivity.text);
            }
            // magnetostatic
            if (SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarkers[i]))
            {
                eleLabelMarker.setAttribute("current_density", labelMagnetostaticMarker->current_density.text);
                eleLabelMarker.setAttribute("permeability", labelMagnetostaticMarker->permeability.text);
            }
            // harmonic magnetic
            if (SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarkers[i]))
            {
                eleLabelMarker.setAttribute("current_density_real", labelHarmonicMagneticMarker->current_density_real.text);
                eleLabelMarker.setAttribute("current_density_imag", labelHarmonicMagneticMarker->current_density_imag.text);
                eleLabelMarker.setAttribute("permeability", labelHarmonicMagneticMarker->permeability.text);
                eleLabelMarker.setAttribute("conductivity", labelHarmonicMagneticMarker->conductivity.text);
            }
            // heat
            if (SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(labelMarkers[i]))
            {
                eleLabelMarker.setAttribute("thermal_conductivity", labelHeatMarker->thermal_conductivity.text);
                eleLabelMarker.setAttribute("volume_heat", labelHeatMarker->volume_heat.text);
            }
            // current
            if (SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarkers[i]))
            {
                eleLabelMarker.setAttribute("conductivity", labelCurrentMarker->conductivity.text);
            }
            // elasticity
            if (SceneLabelElasticityMarker *labelHeatMarker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarkers[i]))
            {
                eleLabelMarker.setAttribute("young_modulus", labelHeatMarker->young_modulus);
                eleLabelMarker.setAttribute("poisson_ratio", labelHeatMarker->poisson_ratio);
            }
        }

        eleLabelMarkers.appendChild(eleLabelMarker);
    }

    // functions
    QDomNode eleFunctions = doc.createElement("functions");
    eleDoc.appendChild(eleFunctions);
    for (int i = 0; i<functions.length(); i++)
    {
        QDomElement eleFunction = doc.createElement("function");

        eleFunction.setAttribute("name", functions[i]->name);
        eleFunction.setAttribute("function", functions[i]->function);

        eleFunctions.appendChild(eleFunction);
    }

    // save to file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    doc.save(out, 4);

    file.waitForBytesWritten(0);
    file.close();

    emit invalidated();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}
