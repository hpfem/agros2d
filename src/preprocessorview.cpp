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

#include "util/constants.h"

#include "scene.h"
#include "logview.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "problemdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "ctemplate/template.h"

PreprocessorWidget::PreprocessorWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewGeometry = sceneView;

    setMinimumWidth(160);
    setObjectName("PreprocessorView");

    createActions();

    // context menu
    mnuPreprocessor = new QMenu(this);

    // boundary conditions, materials and geometry information
    createControls();

    connect(Util::scene(), SIGNAL(cleared()), this, SLOT(refresh()));

    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(Util::problem(), SIGNAL(solved()), this, SLOT(refresh()));
    connect(Util::problem(), SIGNAL(timeStepChanged()), this, SLOT(refresh()));
    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    doItemSelected(NULL, Qt::UserRole);
}

PreprocessorWidget::~PreprocessorWidget()
{
}

void PreprocessorWidget::createActions()
{
    actProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    actProperties->setStatusTip(tr("Properties"));
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(icon("scene-delete"), tr("&Delete"), this);
    actDelete->setStatusTip(tr("Delete item"));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));
}

void PreprocessorWidget::createMenu()
{
    mnuPreprocessor->clear();

    mnuPreprocessor->addAction(Util::scene()->actNewNode);
    mnuPreprocessor->addAction(Util::scene()->actNewEdge);
    mnuPreprocessor->addAction(Util::scene()->actNewLabel);
    mnuPreprocessor->addSeparator();
    Util::scene()->addBoundaryAndMaterialMenuItems(mnuPreprocessor, this);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actDelete);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actProperties);
}

void PreprocessorWidget::createControls()
{
    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(1);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setIndentation(12);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(trvWidget);

    setLayout(layoutMain);
}

void PreprocessorWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        doDelete();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void PreprocessorWidget::refresh()
{
    // script speed improvement
    if (currentPythonEngine()->isRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    // markers
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        // field
        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        fieldNode->setExpanded(true);

        // materials
        QTreeWidgetItem *materialsNode = new QTreeWidgetItem(fieldNode);
        materialsNode->setIcon(0, icon("scenelabelmarker"));
        materialsNode->setText(0, tr("Materials"));
        // materialsNode->setForeground(0, QBrush(Qt::darkBlue));
        materialsNode->setFont(0, fnt);
        materialsNode->setExpanded(true);

        QList<QTreeWidgetItem *> listMaterials;
        foreach (SceneMaterial *material, Util::scene()->materials->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(materialsNode);

            item->setText(0, material->name());
            item->setIcon(0, (Util::scene()->labels->haveMarker(material).count() > 0) ? icon("scene-labelmarker") : icon("scene-labelmarker-notused"));
            if (Util::scene()->labels->haveMarker(material).count() == 0)
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, material->variant());

            listMaterials.append(item);
        }
        materialsNode->addChildren(listMaterials);

        // boundary conditions
        QTreeWidgetItem *boundaryConditionsNode = new QTreeWidgetItem(fieldNode);
        boundaryConditionsNode->setIcon(0, icon("sceneedgemarker"));
        boundaryConditionsNode->setText(0, tr("Boundary conditions"));
        // boundaryConditionsNode->setForeground(0, QBrush(Qt::darkBlue));
        boundaryConditionsNode->setFont(0, fnt);
        boundaryConditionsNode->setExpanded(true);

        QList<QTreeWidgetItem *> listMarkes;
        foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

            Module::BoundaryType *boundary_type = fieldInfo->module()->boundaryType(boundary->type());

            item->setText(0, QString("%1 (%2)").arg(boundary->name()).arg(boundary_type->name()));
            item->setIcon(0, (Util::scene()->edges->haveMarker(boundary).count() > 0) ? icon("scene-edgemarker") : icon("scene-edgemarker-notused"));
            if (Util::scene()->edges->haveMarker(boundary).count() == 0)
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, boundary->variant());

            listMarkes.append(item);
        }
        boundaryConditionsNode->addChildren(listMarkes);
    }

    // geometry
    QTreeWidgetItem *geometryNode = new QTreeWidgetItem(trvWidget);
    // geometryNode->setIcon(0, icon("geometry"));
    geometryNode->setText(0, tr("Geometry"));
    geometryNode->setFont(0, fnt);
    geometryNode->setExpanded(false);

    // nodes
    QTreeWidgetItem *nodesNode = new QTreeWidgetItem(geometryNode);
    nodesNode->setText(0, tr("Nodes"));
    nodesNode->setIcon(0, icon("scenenode"));
    // nodesNode->setForeground(0, QBrush(Qt::darkBlue));
    nodesNode->setFont(0, fnt);

    QList<QTreeWidgetItem *> listNodes;
    int inode = 0;
    foreach (SceneNode *node, Util::scene()->nodes->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - [%2; %3]").
                      arg(inode).
                      arg(node->point().x, 0, 'e', 2).
                      arg(node->point().y, 0, 'e', 2));
        item->setIcon(0, icon("scene-node"));
        item->setData(0, Qt::UserRole, node->variant());

        listNodes.append(item);

        inode++;
    }
    nodesNode->addChildren(listNodes);

    // edges
    QTreeWidgetItem *edgesNode = new QTreeWidgetItem(geometryNode);
    edgesNode->setText(0, tr("Edges"));
    edgesNode->setIcon(0, icon("sceneedge"));
    // edgesNode->setForeground(0, QBrush(Qt::darkBlue));
    edgesNode->setFont(0, fnt);

    QList<QTreeWidgetItem *> listEdges;
    int iedge = 0;
    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - %2 m").
                      arg(iedge).
                      arg((edge->angle() < EPS_ZERO) ?
                              sqrt(Hermes::sqr(edge->nodeEnd()->point().x - edge->nodeStart()->point().x) + Hermes::sqr(edge->nodeEnd()->point().y - edge->nodeStart()->point().y)) :
                              edge->radius() * edge->angle() / 180.0 * M_PI, 0, 'e', 2));
        item->setIcon(0, icon("scene-edge"));
        item->setData(0, Qt::UserRole, edge->variant());

        listEdges.append(item);

        iedge++;
    }
    edgesNode->addChildren(listEdges);

    // labels
    QTreeWidgetItem *labelsNode = new QTreeWidgetItem(geometryNode);
    labelsNode->setText(0, tr("Labels"));
    labelsNode->setIcon(0, icon("scenelabel"));
    // labelsNode->setForeground(0, QBrush(Qt::darkBlue));
    labelsNode->setFont(0, fnt);

    QList<QTreeWidgetItem *> listLabels;
    int ilabel = 0;
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setText(0, QString("%1 - [%2; %3]").
                      arg(ilabel).
                      arg(label->point().x, 0, 'e', 2).
                      arg(label->point().y, 0, 'e', 2));
        item->setIcon(0, icon("scene-label"));
        item->setData(0, Qt::UserRole, label->variant());

        listLabels.append(item);

        ilabel++;
    }
    labelsNode->addChildren(listLabels);

    setUpdatesEnabled(true);
    blockSignals(false);
}

void PreprocessorWidget::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    doItemSelected(item, 0);

    trvWidget->setCurrentItem(item);
    mnuPreprocessor->exec(QCursor::pos());
}

void PreprocessorWidget::doItemSelected(QTreeWidgetItem *item, int role)
{
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
            m_sceneViewGeometry->actSceneModePreprocessor->trigger();

            if (dynamic_cast<SceneNode *>(objectBasic))
                m_sceneViewGeometry->actOperateOnNodes->trigger();
            if (dynamic_cast<SceneEdge *>(objectBasic))
                m_sceneViewGeometry->actOperateOnEdges->trigger();
            if (dynamic_cast<SceneLabel *>(objectBasic))
                m_sceneViewGeometry->actOperateOnLabels->trigger();

            objectBasic->setSelected(true);
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

void PreprocessorWidget::doItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    doProperties();
}

void PreprocessorWidget::doProperties()
{
    if (trvWidget->currentItem())
    {
        // geometry
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>())
        {
            if (objectBasic->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewGeometry->refresh();
                refresh();
            }
            return;
        }

        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            if (objectBoundary->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewGeometry->refresh();
                refresh();
            }
            return;
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            if (objectMaterial->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewGeometry->refresh();
                refresh();
            }
            return;
        }
    }
}

void PreprocessorWidget::doDelete()
{
    if (trvWidget->currentItem())
    {
        // scene objects
        if (SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic*>())
        {
            if (SceneNode *node = dynamic_cast<SceneNode *>(objectBasic))
            {
                Util::scene()->nodes->remove(node);
            }

            else if (SceneEdge *edge = dynamic_cast<SceneEdge *>(objectBasic))
            {
                Util::scene()->edges->remove(edge);
            }

            else if (SceneLabel *label = dynamic_cast<SceneLabel *>(objectBasic))
            {
                Util::scene()->labels->remove(label);
            }
        }

        // label marker
        else if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            Util::scene()->removeMaterial(objectMaterial);
        }

        // edge marker
        else if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            Util::scene()->removeBoundary(objectBoundary);
        }

        m_sceneViewGeometry->refresh();
    }
}
