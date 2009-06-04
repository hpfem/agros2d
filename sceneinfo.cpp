#include "sceneinfo.h"

SceneInfoView::SceneInfoView(SceneView *sceneView, QWidget *parent): QDockWidget(tr("Project"), parent)
{
    this->m_sceneView = sceneView;

    setMinimumWidth(160);
    setObjectName("SceneInfoView");

    createActions();

    createMenu();
    createTreeView();

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    setWidget(trvWidget);

    doItemSelected(NULL, Qt::UserRole);
}

SceneInfoView::~SceneInfoView()
{
}

void SceneInfoView::createActions()
{
    actProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    actProperties->setStatusTip(tr("Properties"));
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(icon("scene-delete"), tr("&Delete"), this);
    actDelete->setStatusTip(tr("Delete item"));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));
}

void SceneInfoView::createMenu()
{
    mnuInfo = new QMenu(this);

    mnuInfo->addAction(m_sceneView->scene()->actNewNode);
    mnuInfo->addAction(m_sceneView->scene()->actNewEdge);
    mnuInfo->addAction(m_sceneView->scene()->actNewLabel);
    mnuInfo->addSeparator();
    mnuInfo->addAction(m_sceneView->scene()->actNewEdgeMarker);
    mnuInfo->addAction(m_sceneView->scene()->actNewLabelMarker);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actProperties);
    mnuInfo->addAction(actDelete);
}

void SceneInfoView::createTreeView()
{
    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(1);
    trvWidget->setColumnWidth(0, 150);

    // problem
    problemNode = new QTreeWidgetItem(trvWidget);
    problemNode->setIcon(0, icon("project"));
    problemNode->setText(0, tr("Problem"));
    problemNode->setExpanded(true);
    // general
    problemInfoGeneralNode = new QTreeWidgetItem(problemNode);
    problemInfoGeneralNode->setText(0, tr("General"));
    // solver
    problemInfoSolverNode = new QTreeWidgetItem(problemNode);
    problemInfoSolverNode->setText(0, tr("Solver"));
    problemInfoSolverNode->setExpanded(true);

    // boundary conditions
    boundaryConditionsNode = new QTreeWidgetItem(trvWidget);
    boundaryConditionsNode->setIcon(0, icon("sceneedgemarker"));
    boundaryConditionsNode->setText(0, tr("Boundary conditions"));
    boundaryConditionsNode->setExpanded(true);

    // materials
    materialsNode = new QTreeWidgetItem(trvWidget);
    materialsNode->setIcon(0, icon("scenelabelmarker"));
    materialsNode->setText(0, tr("Materials"));
    materialsNode->setExpanded(true);

    // geometry
    geometryNode = new QTreeWidgetItem(trvWidget);
    geometryNode->setIcon(0, icon("geometry"));
    geometryNode->setText(0, tr("Geometry"));
    geometryNode->setExpanded(false);

    // nodes
    nodesNode = new QTreeWidgetItem(geometryNode);
    nodesNode->setText(0, "Nodes");
    nodesNode->setIcon(0, icon("scenenode"));

    // edges
    edgesNode = new QTreeWidgetItem(geometryNode);
    edgesNode->setText(0, tr("Edges"));
    edgesNode->setIcon(0, icon("sceneedge"));

    // labels
    labelsNode = new QTreeWidgetItem(geometryNode);
    labelsNode->setText(0, tr("Labels"));
    labelsNode->setIcon(0, icon("scenelabel"));

    trvWidget->addTopLevelItem(problemNode);
    trvWidget->addTopLevelItem(geometryNode);
    trvWidget->addTopLevelItem(boundaryConditionsNode);
    trvWidget->addTopLevelItem(materialsNode);
}

void SceneInfoView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        doDelete();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void SceneInfoView::doInvalidated()
{
    blockSignals(true);

    clearNodes();

    Scene *scene = m_sceneView->scene();

    // problem name
    QTreeWidgetItem *itemProblemName = new QTreeWidgetItem(problemInfoGeneralNode);
    itemProblemName->setText(0, tr("Name: ") + scene->projectInfo().name);
    // problem filename
    QTreeWidgetItem *itemProblemFileName = new QTreeWidgetItem(problemInfoGeneralNode);
    QFileInfo fileInfo(scene->projectInfo().fileName);
    itemProblemFileName->setText(0, tr("Filename: ") + fileInfo.fileName());
    // problem type
    QTreeWidgetItem *itemProblemType = new QTreeWidgetItem(problemInfoGeneralNode);
    itemProblemType->setText(0, tr("Type: ") + scene->projectInfo().problemTypeString());
    // physic field
    QTreeWidgetItem *itemPhysicField = new QTreeWidgetItem(problemInfoGeneralNode);
    itemPhysicField->setText(0, tr("Field: ") + scene->projectInfo().physicFieldString());

    // solver
    if (scene->isMeshed())
    {
        QTreeWidgetItem *itemSolverVertices = new QTreeWidgetItem(problemInfoSolverNode);
        itemSolverVertices->setText(0, tr("Nodes: ") + QString::number(scene->sceneSolution()->mesh().get_num_nodes()));
        QTreeWidgetItem *itemSolverEdges = new QTreeWidgetItem(problemInfoSolverNode);
        itemSolverEdges->setText(0, tr("Elements: ") + QString::number(scene->sceneSolution()->mesh().get_num_elements()));
        if (scene->sceneSolution()->sln())
        {
            QTreeWidgetItem *itemSolverDOFs = new QTreeWidgetItem(problemInfoSolverNode);
            itemSolverDOFs->setText(0, tr("DOFs: ") + QString::number(scene->sceneSolution()->sln()->get_num_dofs()));

            QTime time(0, scene->sceneSolution()->timeElapsed() / (60*100), scene->sceneSolution()->timeElapsed() / 1000, scene->sceneSolution()->timeElapsed() % 1000);
            QTreeWidgetItem *itemSolverTimeElapsed = new QTreeWidgetItem(problemInfoSolverNode);
            itemSolverTimeElapsed->setText(0, tr("Time elapsed: ") + time.toString("mm:ss.zzz"));

            if (scene->sceneSolution()->adaptiveSteps() > 0)
            {
                QTreeWidgetItem *itemSolverAdaptiveError = new QTreeWidgetItem(problemInfoSolverNode);
                itemSolverAdaptiveError->setText(0, tr("Adaptive error: ") + QString::number(scene->sceneSolution()->adaptiveError(), 'f', 3) + " %");

                QTreeWidgetItem *itemSolverAdaptiveSteps = new QTreeWidgetItem(problemInfoSolverNode);
                itemSolverAdaptiveSteps->setText(0, tr("Adaptive steps: ") + QString::number(scene->sceneSolution()->adaptiveSteps()));
            }
        }
    }

    // boundary conditions
    for (int i = 1; i<scene->edgeMarkers.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

        item->setText(0, scene->edgeMarkers[i]->name);
        item->setIcon(0, icon("scene-edgemarker"));
        item->setData(0, Qt::UserRole, scene->edgeMarkers[i]->variant());
    }

    // materials
    for (int i = 1; i<scene->labelMarkers.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(materialsNode);

        item->setText(0, scene->labelMarkers[i]->name);
        item->setIcon(0, icon("scene-labelmarker"));
        item->setData(0, Qt::UserRole, scene->labelMarkers[i]->variant());
    }

    // geometry
    // nodes
    for (int i = 0; i<scene->nodes.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(nodesNode);

        item->setText(0, QString("[%1; %2]").arg(scene->nodes[i]->point.x, 0, 'f', 3).arg(scene->nodes[i]->point.y, 0, 'f', 3));
        item->setIcon(0, icon("scene-node"));
        item->setData(0, Qt::UserRole, scene->nodes[i]->variant());
    }

    // edges
    for (int i = 0; i<scene->edges.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(edgesNode);

        item->setText(0, QString("[%1; %2] - [%3; %4]").arg(scene->edges[i]->nodeStart->point.x, 0, 'f', 3).arg(scene->edges[i]->nodeStart->point.y, 0, 'f', 3).arg(scene->edges[i]->nodeEnd->point.x, 0, 'f', 3).arg(scene->edges[i]->nodeEnd->point.y, 0, 'f', 3));
        item->setIcon(0, icon("scene-edge"));
        item->setData(0, Qt::UserRole, scene->edges[i]->variant());
    }    

    // labels
    for (int i = 0; i<scene->labels.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(labelsNode);

        item->setText(0, QString("[%1; %2]").arg(scene->labels[i]->point.x, 0, 'f', 3).arg(scene->labels[i]->point.y, 0, 'f', 3));
        item->setIcon(0, icon("scene-label"));
        item->setData(0, Qt::UserRole, scene->labels[i]->variant());
    }

    blockSignals(false);
}

void SceneInfoView::clearNodes()
{
    blockSignals(true);

    // problem info general
    while (problemInfoGeneralNode->childCount() > 0)
    {
        QTreeWidgetItem *item = problemInfoGeneralNode->child(0);
        problemInfoGeneralNode->removeChild(item);
        delete item;
    }

    // problem info solver
    while (problemInfoSolverNode->childCount() > 0)
    {
        QTreeWidgetItem *item = problemInfoSolverNode->child(0);
        problemInfoSolverNode->removeChild(item);
        delete item;
    }

    // boundary conditions
    while (boundaryConditionsNode->childCount() > 0)
    {
        QTreeWidgetItem *item = boundaryConditionsNode->child(0);
        boundaryConditionsNode->removeChild(item);
        delete item;
    }
    // materials
    while (materialsNode->childCount() > 0)
    {
        QTreeWidgetItem *item = materialsNode->child(0);
        materialsNode->removeChild(item);
        delete item;
    }

    // geometry
    while (nodesNode->childCount() > 0)
    {
        QTreeWidgetItem *item = nodesNode->child(0);
        nodesNode->removeChild(item);
        delete item;
    }
    while (edgesNode->childCount() > 0)
    {
        QTreeWidgetItem *item = edgesNode->child(0);
        edgesNode->removeChild(item);
        delete item;
    }
    while (labelsNode->childCount() > 0)
    {
        QTreeWidgetItem *item = labelsNode->child(0);
        labelsNode->removeChild(item);
        delete item;
    }

    blockSignals(false);
}

void SceneInfoView::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    doItemSelected(item, 0);

    if ((item != NULL) && (item->data(0, Qt::UserRole) != NULL))
    {
        trvWidget->setCurrentItem(item);

        mnuInfo->exec(QCursor::pos());
    }
}

void SceneInfoView::doItemSelected(QTreeWidgetItem *item, int role)
{
    actProperties->setEnabled(false);
    actDelete->setEnabled(false);    

    if (item != NULL)
    {
        m_sceneView->scene()->highlightNone();

        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>())
        {            
            objectBasic->isHighlighted = true;
            m_sceneView->doRefresh();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // edge marker
        if (SceneEdgeMarker *objectEdgeMarker = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneEdgeMarker *>())
        {
            // select all edges
            for (int i = 0; i<m_sceneView->scene()->edges.count(); i++)
            {
                if (m_sceneView->scene()->edges[i]->marker == objectEdgeMarker)
                    m_sceneView->scene()->edges[i]->isHighlighted = true;
            }
            m_sceneView->doRefresh();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // label marker
        if (SceneLabelMarker *objectLabelMarker = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneLabelMarker *>())
        {
            // select all labels
            for (int i = 0; i<m_sceneView->scene()->labels.count(); i++)
            {
                if (m_sceneView->scene()->labels[i]->marker == objectLabelMarker)
                    m_sceneView->scene()->labels[i]->isHighlighted = true;
            }
            m_sceneView->doRefresh();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
    }
}

void SceneInfoView::doItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    doProperties();
}

void SceneInfoView::doProperties() {
    if (trvWidget->currentItem() != NULL) {
        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic*>())
        {
            if (objectBasic->showDialog(m_sceneView->scene(), this) == QDialog::Accepted)
            {
                m_sceneView->doRefresh();
                doInvalidated();
            }
        }

        // edge marker
        if (SceneEdgeMarker *objectEdgeMarker = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneEdgeMarker *>())
        {
            if (objectEdgeMarker->showDialog(m_sceneView->scene(), this) == QDialog::Accepted)
            {
                m_sceneView->doRefresh();
            }
        }

        // label marker
        if (SceneLabelMarker *objectLabelMarker = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneLabelMarker *>())
        {
            if (objectLabelMarker->showDialog(m_sceneView->scene(), this) == QDialog::Accepted)
            {
                m_sceneView->doRefresh();
            }
        }
    }
}

void SceneInfoView::doDelete()
{
    if (trvWidget->currentItem() != NULL)
    {
        // scene objects
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic*>())
        {
            if (SceneNode *node = dynamic_cast<SceneNode *>(objectBasic))
            {
                m_sceneView->scene()->removeNode(node);
            }

            if (SceneEdge *edge = dynamic_cast<SceneEdge *>(objectBasic))
            {
                m_sceneView->scene()->removeEdge(edge);
            }

            if (SceneLabel *label = dynamic_cast<SceneLabel *>(objectBasic))
            {
                m_sceneView->scene()->removeLabel(label);
            }
        }
        // edge marker
        if (SceneEdgeMarker *objectEdgeMarker = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneEdgeMarker *>())
        {
            m_sceneView->scene()->removeEdgeMarker(objectEdgeMarker);
        }

        // label marker
        if (SceneLabelMarker *objectLabelMarker = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneLabelMarker *>())
        {
            m_sceneView->scene()->removeLabelMarker(objectLabelMarker);
        }

        m_sceneView->doRefresh();
    }
}
