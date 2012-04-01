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

#include "preprocessorview.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_geometry.h"
#include "scenesolution.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "ctemplate/template.h"

PreprocessorView::PreprocessorView(SceneViewGeometry *sceneView, QWidget *parent): QDockWidget(tr("Preprocessor"), parent)
{
    logMessage("PreprocessorView::PreprocessorView()");

    this->m_sceneViewGeometry = sceneView;

    setMinimumWidth(160);
    setObjectName("PreprocessorView");

    createActions();

    // context menu
    mnuInfo = new QMenu(this);

    // boundary conditions, materials and geometry information
    createTreeView();

    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(doInvalidated()));

    /// TODO
    connect(Util::problem(), SIGNAL(solved()), this, SLOT(doInvalidated()));
    connect(Util::problem(), SIGNAL(timeStepChanged()), this, SLOT(doInvalidated()));

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    doItemSelected(NULL, Qt::UserRole);
}

PreprocessorView::~PreprocessorView()
{
}

void PreprocessorView::createActions()
{
    logMessage("PreprocessorView::createActions()");

    actProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    actProperties->setStatusTip(tr("Properties"));
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(icon("scene-delete"), tr("&Delete"), this);
    actDelete->setStatusTip(tr("Delete item"));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));
}

void PreprocessorView::createMenu()
{
    logMessage("PreprocessorView::createMenu()");

    mnuInfo->clear();

    mnuInfo->addAction(Util::scene()->actNewNode);
    mnuInfo->addAction(Util::scene()->actNewEdge);
    mnuInfo->addAction(Util::scene()->actNewLabel);
    mnuInfo->addSeparator();
    Util::scene()->addBoundaryAndMaterialMenuItems(mnuInfo, this);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actDelete);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actProperties);
}

void PreprocessorView::createTreeView()
{
    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(1);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setIndentation(12);

    setWidget(trvWidget);

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

void PreprocessorView::keyPressEvent(QKeyEvent *event)
{
    logMessage("PreprocessorView::keyPressEvent()");

    switch (event->key()) {
    case Qt::Key_Delete:
        doDelete();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void PreprocessorView::doInvalidated()
{
    logMessage("PreprocessorView::doInvalidated()");

    // script speed improvement
    if (scriptIsRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    clearNodes();

    // markers
    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        // boundary conditions
        QTreeWidgetItem *fieldBoundaryConditionsNode = new QTreeWidgetItem(boundaryConditionsNode);
        fieldBoundaryConditionsNode->setText(0, QString::fromStdString(fieldInfo->module()->name));
        fieldBoundaryConditionsNode->setExpanded(true);

        QList<QTreeWidgetItem *> listMarkes;
        foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(fieldBoundaryConditionsNode);

            item->setText(0, QString::fromStdString(boundary->getName()));
            item->setIcon(0, icon("scene-edgemarker"));
            item->setData(0, Qt::UserRole, boundary->variant());

            listMarkes.append(item);
        }
        boundaryConditionsNode->addChildren(listMarkes);

        // materials
        QTreeWidgetItem *fieldMaterialsNode = new QTreeWidgetItem(materialsNode);
        fieldMaterialsNode->setText(0, QString::fromStdString(fieldInfo->module()->name));
        fieldMaterialsNode->setExpanded(true);

        QList<QTreeWidgetItem *> listMaterials;
        foreach (SceneMaterial *material, Util::scene()->materials->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(fieldMaterialsNode);

            item->setText(0, QString::fromStdString(material->getName()));
            item->setIcon(0, icon("scene-labelmarker"));
            item->setData(0, Qt::UserRole, material->variant());

            listMaterials.append(item);
        }
        materialsNode->addChildren(listMaterials);
    }

    // geometry
    // nodes
    QList<QTreeWidgetItem *> listNodes;
    for (int i = 0; i<Util::scene()->nodes->length(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - [%2; %3]").
                      arg(i).
                      arg(Util::scene()->nodes->at(i)->point.x, 0, 'e', 2).
                      arg(Util::scene()->nodes->at(i)->point.y, 0, 'e', 2));
        item->setIcon(0, icon("scene-node"));
        item->setData(0, Qt::UserRole, Util::scene()->nodes->at(i)->variant());

        listNodes.append(item);
    }
    nodesNode->addChildren(listNodes);

    // edges
    QList<QTreeWidgetItem *> listEdges;
    for (int i = 0; i<Util::scene()->edges->length(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - %2 m").
                      arg(i).
                      arg((Util::scene()->edges->at(i)->angle < EPS_ZERO) ?
                              sqrt(Hermes::sqr(Util::scene()->edges->at(i)->nodeEnd->point.x - Util::scene()->edges->at(i)->nodeStart->point.x) + Hermes::sqr(Util::scene()->edges->at(i)->nodeEnd->point.y - Util::scene()->edges->at(i)->nodeStart->point.y)) :
                              Util::scene()->edges->at(i)->radius() * Util::scene()->edges->at(i)->angle / 180.0 * M_PI, 0, 'e', 2));
        item->setIcon(0, icon("scene-edge"));
        item->setData(0, Qt::UserRole, Util::scene()->edges->at(i)->variant());

        listEdges.append(item);
    }
    edgesNode->addChildren(listEdges);

    // labels
    QList<QTreeWidgetItem *> listLabels;
    for (int i = 0; i<Util::scene()->labels->length(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - [%2; %3]").
                      arg(i).
                      arg(Util::scene()->labels->at(i)->point.x, 0, 'e', 2).
                      arg(Util::scene()->labels->at(i)->point.y, 0, 'e', 2));
        item->setIcon(0, icon("scene-label"));
        item->setData(0, Qt::UserRole, Util::scene()->labels->at(i)->variant());

        listLabels.append(item);
    }
    labelsNode->addChildren(listLabels);

    setUpdatesEnabled(true);
    blockSignals(false);
}

void PreprocessorView::clearNodes()
{
    logMessage("PreprocessorView::clearNodes()");

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

void PreprocessorView::doContextMenu(const QPoint &pos)
{
    logMessage("PreprocessorView::doContextMenu()");

    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    doItemSelected(item, 0);

    trvWidget->setCurrentItem(item);
    mnuInfo->exec(QCursor::pos());
}

void PreprocessorView::doItemSelected(QTreeWidgetItem *item, int role)
{
    logMessage("PreprocessorView::doItemSelected()");

    createMenu();

    actProperties->setEnabled(false);
    actDelete->setEnabled(false);

    if (item != NULL)
    {
        Util::scene()->selectNone();
        Util::scene()->highlightNone();

        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>())
        {
            m_sceneViewGeometry->actSceneModeGeometry->trigger();

            if (dynamic_cast<SceneNode *>(objectBasic))
                m_sceneViewGeometry->actOperateOnNodes->trigger();
            if (dynamic_cast<SceneEdge *>(objectBasic))
                m_sceneViewGeometry->actOperateOnEdges->trigger();
            if (dynamic_cast<SceneLabel *>(objectBasic))
                m_sceneViewGeometry->actOperateOnLabels->trigger();

            objectBasic->isSelected = true;
            m_sceneViewGeometry->refresh();
            m_sceneViewGeometry->setFocus();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            // select all edges
            m_sceneViewGeometry->actOperateOnEdges->trigger();

            Util::scene()->edges->haveMarker(objectBoundary).setSelected();

            m_sceneViewGeometry->refresh();
            m_sceneViewGeometry->setFocus();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            // select all labels
            m_sceneViewGeometry->actOperateOnLabels->trigger();

            Util::scene()->labels->haveMarker(objectMaterial).setSelected();

            m_sceneViewGeometry->refresh();
            m_sceneViewGeometry->setFocus();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
    }
}

void PreprocessorView::doItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    logMessage("PreprocessorView::doItemDoubleClicked()");

    doProperties();
}

void PreprocessorView::doProperties()
{
    logMessage("PreprocessorView::doProperties()");

    if (trvWidget->currentItem())
    {
        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>())
        {
            if (objectBasic->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewGeometry->refresh();
                doInvalidated();
            }
            return;
        }

        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            if (objectBoundary->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewGeometry->refresh();
                doInvalidated();                
            }
            return;
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            if (objectMaterial->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewGeometry->refresh();
                doInvalidated();
            }
            return;
        }
    }
}

void PreprocessorView::doDelete()
{
    logMessage("PreprocessorView::doDelete()");

    if (trvWidget->currentItem() != NULL)
    {
        // scene objects
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic*>())
        {
            if (SceneNode *node = dynamic_cast<SceneNode *>(objectBasic))
            {
                Util::scene()->nodes->remove(node);
            }

            if (SceneEdge *edge = dynamic_cast<SceneEdge *>(objectBasic))
            {
                Util::scene()->edges->remove(edge);
            }

            if (SceneLabel *label = dynamic_cast<SceneLabel *>(objectBasic))
            {
                Util::scene()->labels->remove(label);
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

        m_sceneViewGeometry->refresh();
    }
}
