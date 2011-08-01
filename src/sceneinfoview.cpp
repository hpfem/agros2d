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

    // problem information
    txtView = new QTextEdit(this);
    txtView->setReadOnly(true);
    txtView->setMinimumSize(160, 160);

    // boundary conditions, materials and geometry information
    createTreeView();

    splitter = new QSplitter(Qt::Vertical, parent);
    splitter->addWidget(txtView);
    splitter->addWidget(trvWidget);
    setWidget(splitter);

    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(Util::scene()->sceneSolution(), SIGNAL(solved()), this, SLOT(doInvalidated()));
    connect(Util::scene()->sceneSolution(), SIGNAL(timeStepChanged(bool)), this, SLOT(doInvalidated()));

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    QSettings settings;
    splitter->restoreState(settings.value("SceneInfoView/SplitterState", splitter->saveState()).toByteArray());
    splitter->restoreGeometry(settings.value("SceneInfoView/SplitterGeometry", splitter->saveGeometry()).toByteArray());

    doItemSelected(NULL, Qt::UserRole);
}

SceneInfoView::~SceneInfoView()
{
    logMessage("SceneInfoView::~SceneInfoView()");

    QSettings settings;
    settings.setValue("SceneInfoView/SplitterState", splitter->saveState());
    settings.setValue("SceneInfoView/SplitterGeometry", splitter->saveGeometry());
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

void SceneInfoView::showInfo()
{
    // template
    QString content = readFileContent(datadir() + "/resources/report/dock.html");

    QString html;
    html += "<h4>"+ tr("Basic informations") + "</h4>";
    html += "<table width=\"100%\">";
    html += "<tr><td width=\"50%\"><b>" + tr("Name: ") + "</b></td><td>" + Util::scene()->problemInfo()->name + "</td></tr>";
    html += "<tr><td><b>" + tr("Field") + "</b></td><td>" + QString::fromStdString(Util::scene()->problemInfo()->module()->name) + "</td></tr>";
    html += "<tr><td><b>" + tr("Type") + "</b></td><td>" + problemTypeString(Util::scene()->problemInfo()->problemType) + "</td></tr>";
    html += "<tr><td><b>" + tr("Analysis") + "</b></td><td>" + analysisTypeString(Util::scene()->problemInfo()->analysisType) + "</td></tr>";
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        html += "<tr><td>" + tr("Frequency") + "</td><td>" + QString::number(Util::scene()->problemInfo()->frequency) + " Hz" + "</td></tr>";
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        //html += "<tr><td>" + tr("Time step") + "</td><td>" + QString::number(Util::scene()->problemInfo()->timeStep) + " s" + "</td></tr>";
        //html += "<tr><td>" + tr("Total time") + "</td><td>" + QString::number(Util::scene()->problemInfo()->timeTotal) + " s" + "</td></tr>";
        //html += "<tr><td>" + tr("Initial condition") + "</td><td>" + QString::number(Util::scene()->problemInfo()->initialCondition) + "</td></tr>";
    }

    html += "<tr><td><b>" + tr("Adaptivity") + "</b></td><td>" + adaptivityTypeString(Util::scene()->problemInfo()->adaptivityType) + "</td></tr>";
    if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
    {
        html += "<tr><td>" + tr("Adaptivity steps") + "</td><td>" + QString::number(Util::scene()->problemInfo()->adaptivitySteps) + "</td></tr>";
        html += "<tr><td>" + tr("Adaptivity tolerance") + "</td><td>" + QString::number(Util::scene()->problemInfo()->adaptivityTolerance) + " %" + "</td></tr>";
        html += "<tr><td>" + tr("Maximum number of DOFs") + "</td><td>" + QString::number(Util::scene()->problemInfo()->adaptivityMaxDOFs) + "</td></tr>";
    }

    html += "<tr><td><b>" + tr("Linearity") + "</b></td><td>" + linearityTypeString(Util::scene()->problemInfo()->linearityType) + "</td></tr>";
    if (Util::scene()->problemInfo()->linearityType != LinearityType_Linear)
    {
        html += "<tr><td>" + tr("Nonlin. tolerance") + "</td><td>" + QString::number(Util::scene()->problemInfo()->linearityNonlinearTolerance) + " %" + "</td></tr>";
        html += "<tr><td>" + tr("Nonlin. steps") + "</td><td>" + QString::number(Util::scene()->problemInfo()->linearityNonlinearSteps) + "</td></tr>";
    }

    html += "<tr><td><b>" + tr("Linear Solver") + "</b></td><td>" + matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver) + "</td></tr>";
    html += "<tr><td><b>" + tr("Mesh type") + "</b></td><td>" + meshTypeString(Util::scene()->problemInfo()->meshType) + "</td></tr>";
    html += "<tr><td><b>" + tr("Number of refinements") + "</b></td><td>" + QString::number(Util::scene()->problemInfo()->numberOfRefinements) + "</td></tr>";
    html += "<tr><td><b>" + tr("Polynomial order") + "</b></td><td>" + QString::number(Util::scene()->problemInfo()->polynomialOrder) + "</td></tr>";
    html += "</table>";

    if (Util::scene()->sceneSolution()->isMeshed())
    {
        html += "<h4>"+ tr("Mesh and solution") + "</h4>";
        html += "<table width=\"100%\">";
        html += "<tr><td width=\"50%\"><b>" + tr("Initial mesh") + "</b></td><td>" + tr("nodes") + "</td><td>" + QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_nodes()) + "</td></tr>";
        html += "<tr><td></td><td>" + tr("elements") + "</td><td>" + QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_active_elements()) + "</td></tr>";
        if (Util::scene()->sceneSolution()->isSolved())
        {
            if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
            {
                html += "<tr><td><b>" + tr("Adaptivity") + "</b></td><td>" + tr("error") + "</td><td>" + QString::number(Util::scene()->sceneSolution()->adaptiveError(), 'f', 3) + " %" + "</td></tr>";
                html += "<tr><td></td><td>"  + tr("steps") + "</td><td>" + QString::number(Util::scene()->sceneSolution()->adaptiveSteps()) + "</td></tr>";
                html += "<tr><td><b>" + tr("Solution mesh") + "</b></td><td>" + tr("nodes") + "</td><td>" + QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_nodes()) + "</td></tr>";
                html += "<tr><td></td><td>"+ tr("elements") + "</td><td>" + QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_active_elements())  + "</td></tr>";
            }

            if (Util::scene()->sceneSolution()->sln()->get_num_dofs() > 0)
            {
                html += "<tr><td><b>" + tr("DOFs") + "</b></td><td>" + QString::number(Util::scene()->sceneSolution()->sln()->get_num_dofs()) + "</td></tr>";
            }

            QTime time = milisecondsToTime(Util::scene()->sceneSolution()->timeElapsed());
            html += "<tr><td><b>" + tr("Elapsed time") + "</b></td><td>" + time.toString("mm:ss.zzz") + "</td></tr>";
        }
        html += "</table>";
    }

    content.replace("[Body]", html);
    txtView->setText(content);
}

void SceneInfoView::doInvalidated()
{
    logMessage("SceneInfoView::doInvalidated()");

    // script speed improvement
    if (scriptIsRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    showInfo();

    clearNodes();

    // boundary conditions
    QList<QTreeWidgetItem *> listMarkes;
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

        item->setText(0, QString::fromStdString(Util::scene()->boundaries[i]->name));
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

        item->setText(0, QString::fromStdString(Util::scene()->materials[i]->name));
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
                              sqrt(Hermes::sqr(Util::scene()->edges[i]->nodeEnd->point.x - Util::scene()->edges[i]->nodeStart->point.x) + Hermes::sqr(Util::scene()->edges[i]->nodeEnd->point.y - Util::scene()->edges[i]->nodeStart->point.y)) :
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
