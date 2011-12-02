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
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview.h"
#include "scenesolution.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scripteditordialog.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "ctemplate/template.h"

SceneInfoView::SceneInfoView(SceneView *sceneView, QWidget *parent): QDockWidget(tr("Problem"), parent)
{
    logMessage("SceneInfoView::SceneInfoView()");

    this->m_sceneView = sceneView;

    setMinimumWidth(160);
    setObjectName("SceneInfoView");

    createActions();
    createMenu();

    // problem information
    webView = new QWebView(this);

    // boundary conditions, materials and geometry information
    createTreeView();

    splitter = new QSplitter(Qt::Vertical, parent);
    splitter->addWidget(webView);
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
    if (!Util::scene()->fieldInfo("TODO"))
        return;

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string info;
    ctemplate::TemplateDictionary problem("info");

    problem.SetValue("STYLESHEET", style);
    problem.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problem.SetValue("NAME_LABEL", tr("Name:").toStdString());
    problem.SetValue("NAME", Util::scene()->problemInfo()->name.toStdString());

    problem.SetValue("PHYSICAL_FIELD_LABEL", tr("Field:").toStdString());
    problem.SetValue("PHYSICAL_FIELD", Util::scene()->fieldInfo("TODO")->module()->name);

    problem.SetValue("PROBLEM_TYPE_LABEL", tr("Type:").toStdString());
    problem.SetValue("PROBLEM_TYPE", coordinateTypeString(Util::scene()->problemInfo()->coordinateType).toStdString());

    problem.SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
    problem.SetValue("ANALYSIS_TYPE", analysisTypeString(Util::scene()->fieldInfo("TODO")->analysisType).toStdString());
    if (Util::scene()->fieldInfo("TODO")->analysisType == AnalysisType_Harmonic)
    {
        problem.SetValue("FREQUENCY_LABEL", tr("Frequency:").toStdString());
        problem.SetValue("FREQUENCY", QString::number(Util::scene()->problemInfo()->frequency).toStdString() + " Hz");
        problem.ShowSection("ANALYSIS_PARAMETERS_SECTION");
    }
    if (Util::scene()->fieldInfo("TODO")->analysisType == AnalysisType_Transient)
    {
        problem.SetValue("TIME_STEP_LABEL", tr("Time step:").toStdString());
        problem.SetValue("TIME_STEP", QString::number(Util::scene()->problemInfo()->timeStep.number()).toStdString() + " s");
        problem.SetValue("TIME_TOTAL_LABEL", tr("Total time:").toStdString());
        problem.SetValue("TIME_TOTAL", QString::number(Util::scene()->problemInfo()->timeTotal.number()).toStdString() + " s");
        problem.SetValue("INITIAL_CONDITION_LABEL", tr("Total time:").toStdString());
        problem.SetValue("INITIAL_CONDITION", QString::number(Util::scene()->fieldInfo("TODO")->initialCondition.number()).toStdString());
        problem.ShowSection("ANALYSIS_PARAMETERS_SECTION");
    }

    problem.SetValue("ADAPTIVITY_TYPE_LABEL", tr("Adaptivity:").toStdString());
    problem.SetValue("ADAPTIVITY_TYPE", adaptivityTypeString(Util::scene()->fieldInfo("TODO")->adaptivityType).toStdString());
    if (Util::scene()->fieldInfo("TODO")->adaptivityType != AdaptivityType_None)
    {
        problem.SetValue("ADAPTIVITY_STEPS_LABEL", tr("Steps:").toStdString());
        problem.SetValue("ADAPTIVITY_STEPS", QString::number(Util::scene()->fieldInfo("TODO")->adaptivitySteps).toStdString());
        problem.SetValue("ADAPTIVITY_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
        problem.SetValue("ADAPTIVITY_TOLERANCE", QString::number(Util::scene()->fieldInfo("TODO")->adaptivityTolerance).toStdString());
        problem.ShowSection("ADAPTIVITY_PARAMETERS_SECTION");
    }

    problem.SetValue("WEAK_FORMS_TYPE_LABEL", tr("Weak forms:").toStdString());
    problem.SetValue("WEAK_FORMS_TYPE", weakFormsTypeString(Util::scene()->fieldInfo("TODO")->weakFormsType).toStdString());

    problem.SetValue("MESH_TYPE_LABEL", tr("Mesh type:").toStdString());
    problem.SetValue("MESH_TYPE", meshTypeString(Util::scene()->fieldInfo("TODO")->meshType).toStdString());
    problem.SetValue("REFINEMENS_NUMBER_LABEL", tr("Number of refinements:").toStdString());
    problem.SetValue("REFINEMENS_NUMBER", QString::number(Util::scene()->fieldInfo("TODO")->numberOfRefinements).toStdString());
    problem.SetValue("POLYNOMIAL_ORDER_LABEL", tr("Polynomial order:").toStdString());
    problem.SetValue("POLYNOMIAL_ORDER", QString::number(Util::scene()->fieldInfo("TODO")->polynomialOrder).toStdString());

    problem.SetValue("SOLUTION_TYPE_LABEL", tr("Solution type:").toStdString());
    problem.SetValue("SOLUTION_TYPE", linearityTypeString(Util::scene()->fieldInfo("TODO")->linearityType).toStdString());
    if (Util::scene()->fieldInfo("TODO")->linearityType != LinearityType_Linear)
    {
        problem.SetValue("NONLINEAR_STEPS_LABEL", tr("Steps:").toStdString());
        problem.SetValue("NONLINEAR_STEPS", QString::number(Util::scene()->fieldInfo("TODO")->nonlinearSteps).toStdString());
        problem.SetValue("NONLINEAR_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
        problem.SetValue("NONLINEAR_TOLERANCE", QString::number(Util::scene()->fieldInfo("TODO")->nonlinearTolerance).toStdString());
        problem.ShowSection("SOLVER_PARAMETERS_SECTION");
    }

    if (Util::scene()->sceneSolution()->isMeshed())
    {
        problem.SetValue("SOLUTION_INFORMATION_LABEL", tr("Mesh and solution informations").toStdString());

        problem.SetValue("INITIAL_MESH_LABEL", tr("Initial mesh").toStdString());
        problem.SetValue("INITIAL_MESH_NODES_LABEL", tr("Nodes:").toStdString());
        problem.SetValue("INITIAL_MESH_NODES", QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_nodes()).toStdString());
        problem.SetValue("INITIAL_MESH_ELEMENTS_LABEL", tr("Elements:").toStdString());
        problem.SetValue("INITIAL_MESH_ELEMENTS", QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_active_elements()).toStdString());

        if (Util::scene()->sceneSolution()->isSolved())
        {
            if (Util::scene()->sceneSolution()->space() && (Util::scene()->sceneSolution()->space()->get_num_dofs() > 0))
            {
                QTime time = milisecondsToTime(Util::scene()->sceneSolution()->timeElapsed());
                problem.SetValue("ELAPSED_TIME_LABEL", tr("Elapsed time:").toStdString());
                problem.SetValue("ELAPSED_TIME", time.toString("mm:ss.zzz").toStdString());

                problem.SetValue("DOFS_LABEL", tr("DOFs:").toStdString());
                problem.SetValue("DOFS", QString::number(Util::scene()->sceneSolution()->space()->get_num_dofs()).toStdString());
            }

            if (Util::scene()->fieldInfo("TODO")->adaptivityType != AdaptivityType_None)
            {
                problem.SetValue("ADAPTIVITY_LABEL", tr("Adaptivity").toStdString());
                problem.SetValue("ADAPTIVITY_ERROR_LABEL", tr("Error:").toStdString());
                problem.SetValue("ADAPTIVITY_ERROR", QString::number(Util::scene()->sceneSolution()->adaptiveError(), 'f', 3).toStdString());

                problem.SetValue("SOLUTION_MESH_LABEL", tr("Solution mesh").toStdString());
                problem.SetValue("SOLUTION_MESH_NODES_LABEL", tr("Nodes:").toStdString());
                problem.SetValue("SOLUTION_MESH_NODES", QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_nodes()).toStdString());
                problem.SetValue("SOLUTION_MESH_ELEMENTS_LABEL", tr("Elements:").toStdString());
                problem.SetValue("SOLUTION_MESH_ELEMENTS", QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_active_elements()).toStdString());

                problem.ShowSection("ADAPTIVITY_SECTION");
            }
            problem.ShowSection("SOLUTION_PARAMETERS_SECTION");
        }
        problem.ShowSection("SOLUTION_SECTION");
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/problem.tpl", ctemplate::DO_NOT_STRIP, &problem, &info);
    webView->setHtml(QString::fromStdString(info));
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

    // markers
    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        // boundary conditions
        QTreeWidgetItem *fieldBoundaryConditionsNode = new QTreeWidgetItem(boundaryConditionsNode);
        fieldBoundaryConditionsNode->setText(0, QString::fromStdString(fieldInfo->module()->name));
        fieldBoundaryConditionsNode->setExpanded(true);

        QList<QTreeWidgetItem *> listMarkes;
        foreach (SceneBoundary *boundary, fieldInfo->module()->boundaries().items())
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
        foreach (SceneMaterial *material, fieldInfo->module()->materials().items())
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

            Util::scene()->edges->haveMarker(objectBoundary).setSelected();

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

            Util::scene()->labels->haveMarker(objectMaterial).setSelected();

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

        m_sceneView->refresh();
    }
}
