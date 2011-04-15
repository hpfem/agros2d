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

#include "sceneinfoview.h"

#include "scene.h"
#include "sceneview.h"
#include "scripteditordialog.h"

SceneInfoView::SceneInfoView(SceneView *sceneView, QWidget *parent): QDockWidget(tr("Problem"), parent)
{
    logMessage("SceneInfoView::SceneInfoView()");

    this->m_sceneView = sceneView;

    setMinimumWidth(160);
    setObjectName("SceneInfoView");

    createActions();

    createMenu();
    createTreeView();

    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(Util::scene()->sceneSolution(), SIGNAL(solved()), this, SLOT(doInvalidated()));
    connect(Util::scene()->sceneSolution(), SIGNAL(timeStepChanged(bool)), this, SLOT(doInvalidated()));

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    setWidget(trvWidget);

    doItemSelected(NULL, Qt::UserRole);
}

SceneInfoView::~SceneInfoView()
{
    logMessage("SceneInfoView::~SceneInfoView()");

}

void SceneInfoView::createActions()
{
    logMessage("SceneInfoView::createActions()");

    actProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    actProperties->setStatusTip(tr("Properties"));
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(icon("scene-delete"), tr("&Delete"), this);
    actDelete->setStatusTip(tr("Delete item"));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));
}

void SceneInfoView::createMenu()
{
    logMessage("SceneInfoView::createMenu()");

    mnuInfo = new QMenu(this);

    mnuInfo->addAction(Util::scene()->actNewNode);
    mnuInfo->addAction(Util::scene()->actNewEdge);
    mnuInfo->addAction(Util::scene()->actNewLabel);
    mnuInfo->addSeparator();
    mnuInfo->addAction(Util::scene()->actNewBoundary);
    mnuInfo->addAction(Util::scene()->actNewMaterial);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actDelete);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actProperties);
}

void SceneInfoView::createTreeView()
{
    logMessage("SceneInfoView::createTreeView()");

    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(1);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setIndentation(12);

    // problem
    problemNode = new QTreeWidgetItem(trvWidget);
    // problemNode->setIcon(0, icon("problem"));
    problemNode->setText(0, tr("Problem"));
    problemNode->setExpanded(true);
    // general
    problemInfoGeneralNode = new QTreeWidgetItem(problemNode);
    problemInfoGeneralNode->setText(0, tr("General"));
    problemInfoGeneralNode->setExpanded(true);

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

    // functions
    #ifdef BETA
    functionsNode = new QTreeWidgetItem(trvWidget);
    functionsNode->setText(0, tr("Functions"));
    functionsNode->setIcon(0, icon("scenefunction"));
    functionsNode->setExpanded(true);
    #endif

    // geometry
    geometryNode = new QTreeWidgetItem(trvWidget);
    // geometryNode->setIcon(0, icon("geometry"));
    geometryNode->setText(0, tr("Geometry"));
    geometryNode->setExpanded(false);

    // nodes
    nodesNode = new QTreeWidgetItem(geometryNode);
    nodesNode->setText(0, tr("Nodes"));
    nodesNode->setIcon(0, icon("scenenode"));

    // edges
    edgesNode = new QTreeWidgetItem(geometryNode);
    edgesNode->setText(0, tr("Edges"));
    edgesNode->setIcon(0, icon("sceneedge"));

    // labels
    labelsNode = new QTreeWidgetItem(geometryNode);
    labelsNode->setText(0, tr("Labels"));
    labelsNode->setIcon(0, icon("scenelabel"));
}

void SceneInfoView::keyPressEvent(QKeyEvent *event)
{
    logMessage("SceneInfoView::keyPressEvent()");

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
    logMessage("SceneInfoView::doInvalidated()");

    // script speed improvement
    if (scriptIsRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    clearNodes();

    // problem name
    QTreeWidgetItem *itemProblemName = new QTreeWidgetItem(problemInfoGeneralNode);
    itemProblemName->setText(0, tr("Name: ") + Util::scene()->problemInfo()->name);
    // problem type
    QTreeWidgetItem *itemProblemType = new QTreeWidgetItem(problemInfoGeneralNode);
    itemProblemType->setText(0, tr("Type: ") + problemTypeString(Util::scene()->problemInfo()->problemType));
    // physic field
    QTreeWidgetItem *itemPhysicField = new QTreeWidgetItem(problemInfoGeneralNode);
    itemPhysicField->setText(0, tr("Field: ") + physicFieldString(Util::scene()->problemInfo()->physicField()));
    // analysis
    QTreeWidgetItem *itemAnalysisType = new QTreeWidgetItem(problemInfoGeneralNode);
    itemAnalysisType->setText(0, tr("Analysis: ") + analysisTypeString(Util::scene()->problemInfo()->analysisType));
    // solver
    QTreeWidgetItem *itemSolverMatrixSolver = new QTreeWidgetItem(problemInfoGeneralNode);
    itemSolverMatrixSolver->setText(0, tr("Solver: ") + matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver));

    // solver
    if (Util::scene()->sceneSolution()->isMeshed())
    {
        QTreeWidgetItem *itemInitialMeshNode = new QTreeWidgetItem(problemInfoSolverNode);
        itemInitialMeshNode->setText(0, tr("Initial mesh"));
        itemInitialMeshNode->setExpanded(true);

        QTreeWidgetItem *itemSolverInitialVertices = new QTreeWidgetItem(itemInitialMeshNode);
        itemSolverInitialVertices->setText(0, tr("Nodes: ") + QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_nodes()));

        QTreeWidgetItem *itemSolverInitialElements = new QTreeWidgetItem(itemInitialMeshNode);
        itemSolverInitialElements->setText(0, tr("Elements: ") + QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_active_elements()));

        if (Util::scene()->sceneSolution()->isSolved())
        {
            QTreeWidgetItem *itemSolvedMeshNode = new QTreeWidgetItem(problemInfoSolverNode);
            itemSolvedMeshNode->setText(0, tr("Solved mesh"));
            itemSolvedMeshNode->setExpanded(true);

            QTreeWidgetItem *itemSolverSolvedVertices = new QTreeWidgetItem(itemSolvedMeshNode);
            itemSolverSolvedVertices->setText(0, tr("Nodes: ") + QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_nodes()));

            QTreeWidgetItem *itemSolverSolvedElements = new QTreeWidgetItem(itemSolvedMeshNode);
            itemSolverSolvedElements->setText(0, tr("Elements: ") + QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_active_elements()));

            if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
            {
                QTreeWidgetItem *adaptivityNode = new QTreeWidgetItem(problemInfoSolverNode);
                adaptivityNode->setText(0, tr("Adaptivity"));
                adaptivityNode->setExpanded(true);

                QTreeWidgetItem *itemSolverAdaptiveError = new QTreeWidgetItem(adaptivityNode);
                itemSolverAdaptiveError->setText(0, tr("Error: ") + QString::number(Util::scene()->sceneSolution()->adaptiveError(), 'f', 3) + " %");

                QTreeWidgetItem *itemSolverAdaptiveSteps = new QTreeWidgetItem(adaptivityNode);
                itemSolverAdaptiveSteps->setText(0, tr("Steps: ") + QString::number(Util::scene()->sceneSolution()->adaptiveSteps()));
            }

            if (Util::scene()->sceneSolution()->sln()->get_num_dofs() > 0)
            {
                QTreeWidgetItem *itemSolverDOFs = new QTreeWidgetItem(problemInfoSolverNode);
                itemSolverDOFs->setText(0, tr("DOFs: ") + QString::number(Util::scene()->sceneSolution()->sln()->get_num_dofs()));
            }
                        
            QTime time = milisecondsToTime(Util::scene()->sceneSolution()->timeElapsed());
            QTreeWidgetItem *itemSolverTimeElapsed = new QTreeWidgetItem(problemInfoSolverNode);
            itemSolverTimeElapsed->setText(0, tr("Time elapsed: ") + time.toString("mm:ss.zzz"));
        }
    }

    // boundary conditions
    QList<QTreeWidgetItem *> listMarkes;
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

        item->setText(0, Util::scene()->boundaries[i]->name);
        item->setIcon(0, icon("scene-edgemarker"));
        item->setData(0, Qt::UserRole, Util::scene()->boundaries[i]->variant());

        listMarkes.append(item);
    }
    boundaryConditionsNode->addChildren(listMarkes);

    // materials
    QList<QTreeWidgetItem *> listMaterials;
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, Util::scene()->materials[i]->name);
        item->setIcon(0, icon("scene-labelmarker"));
        item->setData(0, Qt::UserRole, Util::scene()->materials[i]->variant());

        listMaterials.append(item);
    }
    materialsNode->addChildren(listMaterials);

    // geometry
    // nodes
    QList<QTreeWidgetItem *> listNodes;
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - [%2; %3]").
                      arg(i).
                      arg(Util::scene()->nodes[i]->point.x, 0, 'e', 2).
                      arg(Util::scene()->nodes[i]->point.y, 0, 'e', 2));
        item->setIcon(0, icon("scene-node"));
        item->setData(0, Qt::UserRole, Util::scene()->nodes[i]->variant());

        listNodes.append(item);
    }
    nodesNode->addChildren(listNodes);

    // edges
    QList<QTreeWidgetItem *> listEdges;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - %2 m").
                      arg(i).
                      arg((Util::scene()->edges[i]->angle < EPS_ZERO) ?
                          sqrt(sqr(Util::scene()->edges[i]->nodeEnd->point.x - Util::scene()->edges[i]->nodeStart->point.x) + sqr(Util::scene()->edges[i]->nodeEnd->point.y - Util::scene()->edges[i]->nodeStart->point.y)) :
                          Util::scene()->edges[i]->radius() * Util::scene()->edges[i]->angle / 180.0 * M_PI, 0, 'e', 2));
        item->setIcon(0, icon("scene-edge"));
        item->setData(0, Qt::UserRole, Util::scene()->edges[i]->variant());

        listEdges.append(item);
    }
    edgesNode->addChildren(listEdges);

    // labels
    QList<QTreeWidgetItem *> listLabels;
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - [%2; %3]").
                      arg(i).
                      arg(Util::scene()->labels[i]->point.x, 0, 'e', 2).
                      arg(Util::scene()->labels[i]->point.y, 0, 'e', 2));
        item->setIcon(0, icon("scene-label"));
        item->setData(0, Qt::UserRole, Util::scene()->labels[i]->variant());

        listLabels.append(item);
    }
    labelsNode->addChildren(listLabels);

    setUpdatesEnabled(true);
    blockSignals(false);
}

void SceneInfoView::clearNodes()
{
    logMessage("SceneInfoView::clearNodes()");

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

    #ifdef BETA
    // functions
    while (functionsNode->childCount() > 0)
    {
        QTreeWidgetItem *item = functionsNode->child(0);
        functionsNode->removeChild(item);
        delete item;
    }
    #endif

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
    logMessage("SceneInfoView::doContextMenu()");

    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    doItemSelected(item, 0);

    trvWidget->setCurrentItem(item);
    mnuInfo->exec(QCursor::pos());
}

void SceneInfoView::doItemSelected(QTreeWidgetItem *item, int role)
{
    logMessage("SceneInfoView::doItemSelected()");

    actProperties->setEnabled(false);
    actDelete->setEnabled(false);

    if (item != NULL)
    {
        Util::scene()->selectNone();
        Util::scene()->highlightNone();

        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>())
        {
            if (dynamic_cast<SceneNode *>(objectBasic))
                m_sceneView->actSceneModeNode->trigger();
            if (dynamic_cast<SceneEdge *>(objectBasic))
                m_sceneView->actSceneModeEdge->trigger();
            if (dynamic_cast<SceneLabel *>(objectBasic))
                m_sceneView->actSceneModeLabel->trigger();

            objectBasic->isSelected = true;
            m_sceneView->refresh();
            m_sceneView->setFocus();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            // select all edges
            m_sceneView->actSceneModeEdge->trigger();
            for (int i = 0; i<Util::scene()->edges.count(); i++)
            {
                if (Util::scene()->edges[i]->boundary == objectBoundary)
                    Util::scene()->edges[i]->isSelected = true;
            }            
            m_sceneView->refresh();
            m_sceneView->setFocus();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            // select all labels
            m_sceneView->actSceneModeLabel->trigger();
            for (int i = 0; i<Util::scene()->labels.count(); i++)
            {
                if (Util::scene()->labels[i]->material == objectMaterial)
                    Util::scene()->labels[i]->isSelected = true;
            }            
            m_sceneView->refresh();
            m_sceneView->setFocus();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
    }
}

void SceneInfoView::doItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    logMessage("SceneInfoView::doItemDoubleClicked()");

    doProperties();
}

void SceneInfoView::doProperties()
{
    logMessage("SceneInfoView::doProperties()");

    if (trvWidget->currentItem() != NULL) {
        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>())
        {
            if (objectBasic->showDialog(this) == QDialog::Accepted)
            {
                m_sceneView->refresh();
                doInvalidated();
            }
        }

        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            if (objectBoundary->showDialog(this) == QDialog::Accepted)
            {
                m_sceneView->refresh();
                doInvalidated();
            }
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            if (objectMaterial->showDialog(this) == QDialog::Accepted)
            {
                m_sceneView->refresh();
                doInvalidated();
            }
        }
    }
}

void SceneInfoView::doDelete()
{
    logMessage("SceneInfoView::doDelete()");

    if (trvWidget->currentItem() != NULL)
    {
        // scene objects
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic*>())
        {
            if (SceneNode *node = dynamic_cast<SceneNode *>(objectBasic))
            {
                Util::scene()->removeNode(node);
            }

            if (SceneEdge *edge = dynamic_cast<SceneEdge *>(objectBasic))
            {
                Util::scene()->removeEdge(edge);
            }

            if (SceneLabel *label = dynamic_cast<SceneLabel *>(objectBasic))
            {
                Util::scene()->removeLabel(label);
            }
        }
        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            Util::scene()->removeBoundary(objectBoundary);
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            Util::scene()->removeMaterial(objectMaterial);
        }

        m_sceneView->refresh();
    }
}
