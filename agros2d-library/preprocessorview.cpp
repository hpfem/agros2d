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
#include "util/global.h"
#include "hermes2d/problem_config.h"

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

#include "hermes2d/problem.h"
#include "ctemplate/template.h"

PreprocessorWidget::PreprocessorWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewPreprocessor = sceneView;

    setMinimumWidth(160);
    setObjectName("PreprocessorView");

    createActions();

    // context menu
    mnuPreprocessor = new QMenu(this);

    // boundary conditions, materials and geometry information
    createControls();

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(refresh()));
    connect(Agros2D::problem(), SIGNAL(timeStepChanged()), this, SLOT(refresh()));
    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    doItemChanged(NULL, NULL);
}

PreprocessorWidget::~PreprocessorWidget()
{
    QSettings settings;
    settings.setValue("PreprocessorWidget/SplitterState", splitter->saveState());
    settings.setValue("PreprocessorWidget/SplitterGeometry", splitter->saveGeometry());
}

void PreprocessorWidget::createActions()
{
    actProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(icon("scene-delete"), tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));
}

void PreprocessorWidget::createMenu()
{
    mnuPreprocessor->clear();

    mnuPreprocessor->addAction(Agros2D::scene()->actNewNode);
    mnuPreprocessor->addAction(Agros2D::scene()->actNewEdge);
    mnuPreprocessor->addAction(Agros2D::scene()->actNewLabel);
    mnuPreprocessor->addSeparator();
    Agros2D::scene()->addBoundaryAndMaterialMenuItems(mnuPreprocessor, this);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actDelete);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actProperties);
}

void PreprocessorWidget::createControls()
{
    txtViewNodes = new QTextEdit(this);
    txtViewNodes->setReadOnly(true);
    txtViewNodes->setVisible(false);
    txtViewNodes->setText(tr("Tooltip_OperateOnNodes"));

    txtViewEdges = new QTextEdit(this);
    txtViewEdges->setReadOnly(true);
    txtViewEdges->setVisible(false);
    txtViewEdges->setText(tr("Tooltip_OperateOnEdges"));

    txtViewLabels = new QTextEdit(this);
    txtViewLabels->setReadOnly(true);
    txtViewLabels->setVisible(false);
    txtViewLabels->setText(tr("Tooltip_OperateOnLabels"));

    loadTooltip(SceneGeometryMode_OperateOnNodes);

    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(1);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setIndentation(12);

    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));

    QPushButton *btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QGridLayout *layoutTreeView = new QGridLayout();
    layoutTreeView->addWidget(trvWidget, 0, 0, 1, 4);
    layoutTreeView->addWidget(new QLabel(tr("Grid step:")), 1, 0);
    layoutTreeView->addWidget(txtGridStep, 1, 1);
    layoutTreeView->addWidget(chkSnapToGrid, 1, 2);
    layoutTreeView->addWidget(btnOK, 1, 3);

    QWidget *widgetTreeView = new QWidget();
    widgetTreeView->setLayout(layoutTreeView);

    QHBoxLayout *layoutView = new QHBoxLayout();
    layoutView->addWidget(txtViewNodes);
    layoutView->addWidget(txtViewEdges);
    layoutView->addWidget(txtViewLabels);

    QWidget *view = new QWidget();
    view->setLayout(layoutView);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(widgetTreeView);
    splitter->addWidget(view);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(splitter);

    setLayout(layoutMain);

    QSettings settings;
    splitter->restoreState(settings.value("PreprocessorWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PreprocessorWidget/SplitterGeometry").toByteArray());
}

void PreprocessorWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        doDelete();
        return;
    }

    QWidget::keyPressEvent(event);
}

void PreprocessorWidget::refresh()
{
    txtGridStep->setText(QString::number(Agros2D::problem()->setting()->value(ProblemSetting::View_GridStep).toDouble()));
    chkSnapToGrid->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_SnapToGrid).toBool());

    // script speed improvement
    if (currentPythonEngine()->isScriptRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    // markers
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
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
        foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(materialsNode);

            item->setText(0, material->name());
            item->setIcon(0, (Agros2D::scene()->labels->haveMarker(material).count() > 0) ? icon("scene-labelmarker") : icon("scene-labelmarker-notused"));
            if (Agros2D::scene()->labels->haveMarker(material).isEmpty())
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
        foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

            item->setText(0, QString("%1 (%2)").arg(boundary->name()).arg(boundaryType.name()));
            item->setIcon(0, (Agros2D::scene()->edges->haveMarker(boundary).count() > 0) ? icon("scene-edgemarker") : icon("scene-edgemarker-notused"));
            if (Agros2D::scene()->edges->haveMarker(boundary).isEmpty())
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
    foreach (SceneNode *node, Agros2D::scene()->nodes->items())
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
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
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
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
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

void PreprocessorWidget::loadTooltip(SceneGeometryMode sceneMode)
{
    txtViewNodes->setVisible(sceneMode == SceneGeometryMode_OperateOnNodes);
    txtViewEdges->setVisible(sceneMode == SceneGeometryMode_OperateOnEdges);
    txtViewLabels->setVisible(sceneMode == SceneGeometryMode_OperateOnLabels);
}

void PreprocessorWidget::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *current = trvWidget->itemAt(pos);
    doItemChanged(current, NULL);

    if (current)
        trvWidget->setCurrentItem(current);

    mnuPreprocessor->exec(QCursor::pos());
}

void PreprocessorWidget::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    createMenu();

    actProperties->setEnabled(false);
    actDelete->setEnabled(false);

    Agros2D::scene()->selectNone();
    Agros2D::scene()->highlightNone();

    if (current)
    {
        if (SceneBasic *objectBasic = current->data(0, Qt::UserRole).value<SceneBasic *>())
        {
            // geometry
            m_sceneViewPreprocessor->actSceneModePreprocessor->trigger();

            if (dynamic_cast<SceneNode *>(objectBasic))
                m_sceneViewPreprocessor->actOperateOnNodes->trigger();
            if (dynamic_cast<SceneEdge *>(objectBasic))
                m_sceneViewPreprocessor->actOperateOnEdges->trigger();
            if (dynamic_cast<SceneLabel *>(objectBasic))
                m_sceneViewPreprocessor->actOperateOnLabels->trigger();

            objectBasic->setSelected(true);

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (SceneBoundary *objectBoundary = current->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            // edge marker
            // select all edges
            m_sceneViewPreprocessor->actOperateOnEdges->trigger();

            Agros2D::scene()->edges->haveMarker(objectBoundary).setSelected();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (SceneMaterial *objectMaterial = current->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            // label marker
            // select all labels
            m_sceneViewPreprocessor->actOperateOnLabels->trigger();

            Agros2D::scene()->labels->haveMarker(objectMaterial).setSelected();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        m_sceneViewPreprocessor->refresh();
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
                m_sceneViewPreprocessor->refresh();
                refresh();
            }
            return;
        }

        // edge marker
        if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            if (objectBoundary->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewPreprocessor->refresh();
                refresh();
            }
            return;
        }

        // label marker
        if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            if (objectMaterial->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewPreprocessor->refresh();
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
                Agros2D::scene()->nodes->remove(node);
            }

            else if (SceneEdge *edge = dynamic_cast<SceneEdge *>(objectBasic))
            {
                Agros2D::scene()->edges->remove(edge);
            }

            else if (SceneLabel *label = dynamic_cast<SceneLabel *>(objectBasic))
            {
                Agros2D::scene()->labels->remove(label);
            }
        }

        // label marker
        else if (SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>())
        {
            Agros2D::scene()->removeMaterial(objectMaterial);
        }

        // edge marker
        else if (SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>())
        {
            Agros2D::scene()->removeBoundary(objectBoundary);
        }

        refresh();
        m_sceneViewPreprocessor->refresh();
    }
}

void PreprocessorWidget::doApply()
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_GridStep, txtGridStep->text().toDouble());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_SnapToGrid, chkSnapToGrid->isChecked());
}
