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
    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(a.cx + a.radius*cos(a.angle1/180*M_PI), a.cy + a.radius*sin(a.angle1/180*M_PI))));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(a.cx + a.radius*cos(a.angle2/180*M_PI), a.cy + a.radius*sin(a.angle2/180*M_PI))));

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, m_scene->edgeMarkers[0], a.angle2-a.angle1));
}

// ************************************************************************************************************************

Scene::Scene() {
    createActions();

    m_sceneSolution = new SceneSolution(this);
    solverDialog = new SolverDialog(QApplication::activeWindow());
    connect(solverDialog, SIGNAL(solved()), this, SLOT(doSolved()));

    connect(this, SIGNAL(invalidated()), this, SLOT(doInvalidated()));

    clear();
}

Scene::~Scene() {
    delete m_sceneSolution;
    delete solverDialog;
}

void Scene::createActions()
{
    // scene - add items
    actNewNode = new QAction(icon("scene-node"), tr("New &node"), this);
    actNewNode->setShortcut(tr("Alt+N"));
    actNewNode->setStatusTip(tr("New node"));
    connect(actNewNode, SIGNAL(triggered()), this, SLOT(doNewNode()));

    actNewEdge = new QAction(icon("scene-edge"), tr("New &edge"), this);
    actNewEdge->setShortcut(tr("Alt+E"));
    actNewEdge->setStatusTip(tr("New edge"));
    connect(actNewEdge, SIGNAL(triggered()), this, SLOT(doNewEdge()));

    actNewLabel = new QAction(icon("scene-label"), tr("New &label"), this);
    actNewLabel->setShortcut(tr("Alt+L"));
    actNewLabel->setStatusTip(tr("New label"));
    connect(actNewLabel, SIGNAL(triggered()), this, SLOT(doNewLabel()));

    actNewEdgeMarker = new QAction(icon("scene-edgemarker"), tr("New &boundary condition"), this);
    actNewEdgeMarker->setShortcut(tr("Alt+B"));
    actNewEdgeMarker->setStatusTip(tr("New boundary condition"));
    connect(actNewEdgeMarker, SIGNAL(triggered()), this, SLOT(doNewEdgeMarker()));

    actNewLabelMarker = new QAction(icon("scene-labelmarker"), tr("New &material"), this);
    actNewLabelMarker->setShortcut(tr("Alt+M"));
    actNewLabelMarker->setStatusTip(tr("New material"));
    connect(actNewLabelMarker, SIGNAL(triggered()), this, SLOT(doNewLabelMarker()));

    actTransform = new QAction(icon("scene-transform"), tr("Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    actProjectProperties = new QAction(icon("scene-properties"), tr("Project properties"), this);
    actProjectProperties->setStatusTip(tr("Project properties"));
    connect(actProjectProperties, SIGNAL(triggered()), this, SLOT(doProjectProperties()));
}

SceneNode *Scene::addNode(SceneNode *node) {
    // check if node doesn't exists
    foreach (SceneNode *nodeCheck, nodes)
    {
        if ((fabs(nodeCheck->point.x-node->point.x) < EPS_ZERO) && (fabs(nodeCheck->point.y-node->point.y) < EPS_ZERO))
            return nodeCheck;
    }

    nodes.append(node);
    emit invalidated();

    return node;
}

void Scene::removeNode(SceneNode *node) {
    // remove all edges connected to this node
    foreach (SceneEdge *edge, edges)
    {
        if ((edge->nodeStart == node) || (edge->nodeEnd == node))
            removeEdge(edge);
    }
    nodes.removeOne(node);
    emit invalidated();
}

SceneEdge *Scene::addEdge(SceneEdge *edge) {
    // check if edge doesn't exists
    foreach (SceneEdge *edgeCheck, edges)
    {
        if (((edgeCheck->nodeStart == edge->nodeStart) && (edgeCheck->nodeEnd == edge->nodeEnd)) ||
            ((edgeCheck->nodeStart == edge->nodeEnd) && (edgeCheck->nodeEnd == edge->nodeStart)) &&
            (fabs(edgeCheck->angle-edge->angle) < EPS_ZERO))
            return edgeCheck;
    }

    edges.append(edge);
    emit invalidated();

    return edge;
}

void Scene::removeEdge(SceneEdge *edge) {
    edges.removeOne(edge);
    emit invalidated();
}

SceneLabel *Scene::addLabel(SceneLabel *label) {
    // check if label doesn't exists
    foreach (SceneLabel *labelCheck, labels)
    {
        if ((fabs(labelCheck->point.x-label->point.x) < EPS_ZERO) && (fabs(labelCheck->point.y-label->point.y) < EPS_ZERO))
            return labelCheck;
    }

    labels.append(label);
    emit invalidated();

    return label;
}

void Scene::removeLabel(SceneLabel *label) {
    labels.removeOne(label);
    emit invalidated();
}

void Scene::addEdgeMarker(SceneEdgeMarker *edgeMarker) {
    edgeMarkers.append(edgeMarker);
    emit invalidated();
}

void Scene::removeEdgeMarker(SceneEdgeMarker *edgeMarker) {
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

void Scene::addLabelMarker(SceneLabelMarker *labelMarker) {
    this->labelMarkers.append(labelMarker);
    emit invalidated();
}

void Scene::removeLabelMarker(SceneLabelMarker *labelMarker) {
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

void Scene::clear() {
    blockSignals(true);

    m_sceneSolution->clear();
    m_projectInfo.clear();

    nodes.clear();
    edges.clear();
    labels.clear();

    edgeMarkers.clear();
    labelMarkers.clear();

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
    foreach (SceneNode *node, nodes)
        if (node->isSelected) removeNode(node);

    foreach (SceneEdge *edge, edges)
        if (edge->isSelected) removeEdge(edge);

    foreach (SceneLabel *label, labels)
        if (label->isSelected) removeLabel(label);
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
            Point newPoint = node->point + point;
            if (!copy)
                node->point = newPoint;
            else
                addNode(new SceneNode(newPoint));
        }
    }

    foreach (SceneLabel *label, labels)
    {
        if (label->isSelected)
        {
            Point newPoint = label->point + point;
            if (!copy)
                label->point = newPoint;
            else
                addLabel(new SceneLabel(newPoint, label->marker, label->area));
        }
    }

    emit invalidated();
}

void Scene::transformRotate(const Point &point, double angle, bool copy)
{
    foreach (SceneNode *node, nodes)
        if (node->isSelected)
        {
        double distanceNode = (node->point - point).magnitude();
        double angleNode = (node->point - point).angle()/M_PI*180;

        Point newPoint = point + Point(distanceNode * cos((angleNode - angle)/180.0*M_PI), distanceNode * sin((angleNode - angle)/180.0*M_PI));
        if (!copy)
            node->point = newPoint;
        else
            addNode(new SceneNode(newPoint));
    }

    foreach (SceneLabel *label, labels)
        if (label->isSelected)
        {
        double distanceNode = (label->point - point).magnitude();
        double angleNode = (label->point - point).angle()/M_PI*180;

        Point newPoint = point + Point(distanceNode * cos((angleNode - angle)/180.0*M_PI), distanceNode * sin((angleNode - angle)/180.0*M_PI));
        if (!copy)
            label->point = newPoint;
        else
            addLabel(new SceneLabel(newPoint, label->marker, label->area));
    }

    emit invalidated();
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy)
{
    foreach (SceneNode *node, nodes)
        if (node->isSelected)
        {
        Point newPoint = point + (node->point - point) * scaleFactor;
        if (!copy)
            node->point = newPoint;
        else
            addNode(new SceneNode(newPoint));
    }

    foreach (SceneLabel *label, labels)
        if (label->isSelected)
        {
        Point newPoint = point + (label->point - point) * scaleFactor;
        if (!copy)
            label->point = newPoint;
        else
            addLabel(new SceneLabel(newPoint, label->marker, label->area));
    }

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

    // clear project
    sceneSolution()->clear();

    // save as temp name
    if (m_projectInfo.fileName.isEmpty())
        m_projectInfo.fileName = QDesktopServices::TempLocation + "/agros_temp.h2d";

    // save project
    writeToFile(m_projectInfo.fileName);

    // solve
    solverDialog->setMode(solverMode);
    solverDialog->show();
    solverDialog->solve();
}

void Scene::doSolved()
{
    solverDialog->hide();

    // file info
    QFileInfo fileInfo(m_projectInfo.fileName);

    // this slot is called after triangle and solve process is finished
    // linearizer only for mesh (on empty solution)
    if (QFile::exists(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".mesh"))
    {
        // save locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        m_sceneSolution->mesh().load((QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".mesh").toStdString().c_str());

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }

    // set solver results
    if (m_sceneSolution->isSolved())
        emit solved();

    emit invalidated();

    // delete temp file
    if (m_projectInfo.fileName == QDesktopServices::TempLocation + "/agros_temp.h2d")
    {
        QFile::remove(m_projectInfo.fileName);
        m_projectInfo.fileName = "";
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
        addNode(node);
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

void Scene::doNewEdgeMarker()
{
    SceneEdgeMarker *marker;
    switch (m_projectInfo.physicField)
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
        cerr << "Physical field '" + physicFieldStringKey(m_projectInfo.physicField).toStdString() + "' is not implemented. Scene::doNewEdgeMarker()" << endl;
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
    switch (m_projectInfo.physicField)
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
        cerr << "Physical field '" + physicFieldStringKey(m_projectInfo.physicField).toStdString() + "' is not implemented. Scene::doNewLabelMarker()" << endl;
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

void Scene::doTransform()
{
    SceneTransformDialog *sceneTransformDialog = new SceneTransformDialog(QApplication::activeWindow());
    sceneTransformDialog->exec();
    delete sceneTransformDialog;
}

void Scene::doProjectProperties()
{
    ProjectDialog *projectDialog = new ProjectDialog(m_projectInfo, false, QApplication::activeWindow());
    if (projectDialog->showDialog() == QDialog::Accepted)
    {
        emit invalidated();
    }
    delete projectDialog;
}

int Scene::writeToTriangle()
{
    // file info
    QFileInfo fileInfo(m_projectInfo.fileName);

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".poly");

    if (!file.open(QIODevice::WriteOnly))
    {
        cerr << "Could not create triangle poly mesh file." << endl;
        return 0;
    }
    QTextStream out(&file);


    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<nodes.count(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").arg(i).arg(nodes[i]->point.x, 0, 'f', 10).arg(nodes[i]->point.y, 0, 'f', 10).arg(0);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<edges.count(); i++)
    {
        if (edges[i]->angle == 0)
        {
            // line
            outEdges += QString("%1  %2  %3  %4\n").arg(edgesCount).arg(nodes.indexOf(edges[i]->nodeStart)).arg(nodes.indexOf(edges[i]->nodeEnd)).arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudonodes
            Point center = edges[i]->center();
            double radius = edges[i]->radius();
            double startAngle = atan2(center.y - edges[i]->nodeStart->point.y, center.x - edges[i]->nodeStart->point.x) / M_PI*180 - 180;
            int segments = edges[i]->angle/5.0 + 1;
            if (segments < 5) segments = 5; // minimum segments

            double theta = edges[i]->angle / float(segments - 1);

            int nodeStartIndex = 0;
            int nodeEndIndex = 0;
            for (int j = 0; j < segments; j++)
            {
                double arc = (startAngle + j*theta)/180.0*M_PI;
                double x = radius * cos(arc);
                double y = radius * sin(arc);

                nodeEndIndex = nodesCount+1;
                if (j == 0)
                {
                    nodeStartIndex = nodes.indexOf(edges[i]->nodeStart);
                    nodeEndIndex = nodesCount;
                }
                if (j == segments-1)
                {
                    nodeEndIndex = nodes.indexOf(edges[i]->nodeEnd);
                }
                if ((j > 0) && (j < segments))
                {
                    outNodes += QString("%1  %2  %3  %4\n").arg(nodesCount).arg(center.x + x, 0, 'f', 10).arg(center.y + y, 0, 'f', 10).arg(0);
                    nodesCount++;
                }
                outEdges += QString("%1  %2  %3  %4\n").arg(edgesCount).arg(nodeStartIndex).arg(nodeEndIndex).arg(i+1);
                edgesCount++;
                nodeStartIndex = nodeEndIndex;
            }
        }
    }

    // holes
    int holesCount = 0;
    for (int i = 0; i<labels.count(); i++) if (labelMarkers.indexOf(labels[i]->marker) == 0) holesCount++;
    QString outHoles = QString("%1\n").arg(holesCount);
    for (int i = 0; i<labels.count(); i++)
        if (labelMarkers.indexOf(labels[i]->marker) == 0)
            outHoles += QString("%1  %2  %3\n").arg(i).arg(labels[i]->point.x, 0, 'f', 10).arg(labels[i]->point.y, 0, 'f', 10);

    // labels
    QString outLabels;
    int labelsCount = 0;
    for(int i = 0; i<labels.count(); i++)
        if (labelMarkers.indexOf(labels[i]->marker) > 0) 
        {
        outLabels += QString("%1  %2  %3  %4  %5\n").arg(labelsCount).arg(labels[i]->point.x, 0, 'f', 10).arg(labels[i]->point.y, 0, 'f', 10).arg(i).arg(labels[i]->area);
        labelsCount++;
    }


    outNodes.insert(0, QString("%1 2 0 1\n").arg(nodesCount)); // + additional nodes
    out << outNodes;
    outEdges.insert(0, QString("%1 1\n").arg(edgesCount)); // + additional edges
    out << outEdges;
    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").arg(labelsCount)); // - holes
    out << outLabels;

    file.waitForBytesWritten(0);
    file.close();

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
            double angle1 = atan2(cy - edges[i]->nodeStart->point.y, cx - edges[i]->nodeStart->point.x)/M_PI*180.0 - 180.0;
            double angle2 = atan2(cy - edges[i]->nodeEnd->point.y, cx - edges[i]->nodeEnd->point.x)/M_PI*180.0 + 180.0;

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

void Scene::readFromFile(const QString &fileName)
{
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    clear();
    this->m_projectInfo.fileName = fileName;

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

    // projects
    QDomNode eleProjects = eleDoc.elementsByTagName("projects").at(0);
    // first project
    QDomNode eleProject = eleProjects.toElement().elementsByTagName("project").at(0);
    // name
    m_projectInfo.name = eleProject.toElement().attribute("name");
    // problem type                                                                                                                                                                                                                             `
    if (eleProject.toElement().attribute("problemtype") == problemTypeStringKey(PROBLEMTYPE_PLANAR)) m_projectInfo.problemType = PROBLEMTYPE_PLANAR;
    if (eleProject.toElement().attribute("problemtype") == problemTypeStringKey(PROBLEMTYPE_AXISYMMETRIC)) m_projectInfo.problemType = PROBLEMTYPE_AXISYMMETRIC;
    // physic field
    m_projectInfo.physicField = physicFieldFromStringKey(eleProject.toElement().attribute("type"));
    // number of refinements
    m_projectInfo.numberOfRefinements = eleProject.toElement().attribute("numberofrefinements").toInt();
    // polynomial order
    m_projectInfo.polynomialOrder = eleProject.toElement().attribute("polynomialorder").toInt();
    // adaptivity
    m_projectInfo.adaptivitySteps = eleProject.toElement().attribute("adaptivitysteps").toInt();
    m_projectInfo.adaptivityTolerance = eleProject.toElement().attribute("adaptivitytolerance").toDouble();
    // time harmonic
    m_projectInfo.frequency = eleProject.toElement().attribute("frequency").toDouble();

    // startup script
    QDomNode eleSriptStartup = eleProject.toElement().elementsByTagName("scriptstartup").at(0);
    m_projectInfo.scriptStartup = eleSriptStartup.toElement().text();

    // markers ***************************************************************************************************************

    // edge marker
    QDomNode eleEdgeMarkers = eleProject.toElement().elementsByTagName("edges").at(0);
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
            switch (m_projectInfo.physicField)
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
                cerr << "Physical field '" + physicFieldStringKey(m_projectInfo.physicField).toStdString() + "' is not implemented. Scene::readFromFile(const QString &fileName)" << endl;
                throw;
                break;
            }
        }

        n = n.nextSibling();
    }

    // label marker
    QDomNode eleLabelMarkers = eleProject.toElement().elementsByTagName("labels").at(0);
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
            switch (m_projectInfo.physicField)
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
                        cerr << "Physical field '" + physicFieldStringKey(m_projectInfo.physicField).toStdString() + "' is not implemented. Scene::readFromFile(const QString &fileName)" << endl;
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

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    blockSignals(false);

    emit invalidated();
}

void Scene::writeToFile(const QString &fileName) {
    if (projectInfo().fileName != QDesktopServices::TempLocation + "/agros_temp.h2d")
    {
        QSettings settings;
        QFileInfo fileInfo(fileName);
        settings.setValue("LastDataDir", fileInfo.absoluteFilePath());
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    m_projectInfo.fileName = fileName;

    QDomDocument doc;

    // main document
    QDomElement eleDoc = doc.createElement("document");
    doc.appendChild(eleDoc);

    // projects
    QDomNode eleProjects = doc.createElement("projects");
    eleDoc.appendChild(eleProjects);
    // first project
    QDomElement eleProject = doc.createElement("project");
    eleProjects.appendChild(eleProject);
    // id
    eleProject.setAttribute("id", 0);
    // name
    eleProject.setAttribute("name", m_projectInfo.name);
    // problem type                                                                                                                                                                                                                             `
    if (m_projectInfo.problemType == PROBLEMTYPE_PLANAR) eleProject.toElement().setAttribute("problemtype", "planar");
    if (m_projectInfo.problemType == PROBLEMTYPE_AXISYMMETRIC) eleProject.toElement().setAttribute("problemtype", "axisymmetric");
    // name
    eleProject.setAttribute("type", physicFieldStringKey(m_projectInfo.physicField));
    // number of refinements
    eleProject.setAttribute("numberofrefinements", m_projectInfo.numberOfRefinements);
    // polynomial order
    eleProject.setAttribute("polynomialorder", m_projectInfo.polynomialOrder);
    // adaptivity
    eleProject.setAttribute("adaptivitysteps", m_projectInfo.adaptivitySteps);
    eleProject.setAttribute("adaptivitytolerance", m_projectInfo.adaptivityTolerance);
    // time harmonic
    eleProject.setAttribute("frequency", m_projectInfo.frequency);

    // startup script
    QDomElement eleSriptStartup = doc.createElement("scriptstartup");
    eleSriptStartup.appendChild(doc.createTextNode(m_projectInfo.scriptStartup));
    eleProject.appendChild(eleSriptStartup);

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
    eleProject.appendChild(eleEdgeMarkers);
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
    eleProject.appendChild(eleLabelMarkers);
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

bool Scene::triangle2mesh(const QString &source, const QString &destination)
{
    bool returnValue = true;

    int i, n, k, l, marker, node_1, node_2, node_3;
    double x, y;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QFile fileMesh(destination + ".mesh");
    if (!fileMesh.open(QIODevice::WriteOnly))
    {
        cerr << "Could not create hermes2d mesh file." << endl;
        return 0;
    }
    QTextStream outMesh(&fileMesh);

    QFile fileNode(source + ".node");
    if (!fileNode.open(QIODevice::ReadOnly))
    {
        cerr << "Could not create triangle node file." << endl;
        return 0;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(source + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly))
    {
        cerr << "Could not create triangle edge file." << endl;
        return 0;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(source + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly))
    {
        cerr << "Could not create triangle ele file." << endl;
        return 0;
    }
    QTextStream inEle(&fileEle);

    // nodes
    QString outNodes;
    outNodes += "vertices = \n";
    outNodes += "{ \n";
    sscanf(inNode.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i<k; i++)
    {
        sscanf(inNode.readLine().toStdString().c_str(), "%i   %lf %lf %i", &n, &x, &y, &marker);
        outNodes += QString("\t{ %1,  %2 }, \n").arg(x, 0, 'f', 10).arg(y, 0, 'f', 10);
    }
    outNodes.truncate(outNodes.length()-3);
    outNodes += "\n} \n\n";

    // edges and curves
    QString outEdges;
    outEdges += "boundaries = \n";
    outEdges += "{ \n";
    sscanf(inEdge.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i<k; i++)
    {
        sscanf(inEdge.readLine().toStdString().c_str(), "%i	%i	%i	%i", &n, &node_1, &node_2, &marker);
        if (marker != 0)
        {
            if (edges[marker-1]->marker->type != PHYSICFIELDBC_NONE)
                outEdges += QString("\t{ %1, %2, %3 }, \n").arg(node_1).arg(node_2).arg(abs(marker));
        }
    }
    outEdges.truncate(outEdges.length()-3);
    outEdges += "\n} \n\n";
    
    // elements
    QString outElements;
    outElements += "elements = \n";
    outElements += "{ \n";
    sscanf(inEle.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i<k; i++)
    {
        sscanf(inEle.readLine().toStdString().c_str(), "%i	%i	%i	%i	%i", &n, &node_1, &node_2, &node_3, &marker);
        outElements += QString("\t{ %1, %2, %3, %4  }, \n").arg(node_1).arg(node_2).arg(node_3).arg(abs(marker));
    }
    outElements.truncate(outElements.length()-3);
    outElements += "\n} \n\n";

    outMesh << outNodes;
    outMesh << outElements;
    outMesh << outEdges;

    fileNode.close();
    fileEdge.close();
    fileEle.close();

    fileMesh.waitForBytesWritten(0);
    fileMesh.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return returnValue;
}
