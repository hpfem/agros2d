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
#include "logview.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_geometry.h"
#include "scenesolution.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "problemdialog.h"
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
    mnuPreprocessor = new QMenu(this);

    // boundary conditions, materials and geometry information
    createControls();

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

void PreprocessorView::createControls()
{
    tlbFields = new QToolBar(this);
    tlbFields->setIconSize(QSize(48, 48));

    actFieldsGroup = new QActionGroup(this);
    connect(actFieldsGroup, SIGNAL(triggered(QAction *)), this, SLOT(doProblemDialog(QAction *)));

    webView = new QWebView(this);
    webView->setMinimumHeight(250);

    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(1);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setIndentation(12);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(tlbFields);
    layoutMain->addWidget(webView, 2);
    layoutMain->addWidget(trvWidget, 3);

    QWidget *main = new QWidget(this);
    main->setLayout(layoutMain);

    setWidget(main);

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
    // script speed improvement
    if (scriptIsRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    clearNodes();

    tlbFields->clear();
    actFieldsGroup->actions().clear();

    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        QAction *actField = new QAction(QString::fromStdString(fieldInfo->module()->name), this);
        actField->setIcon(icon(QString::fromStdString("fields/" + fieldInfo->module()->fieldid)));
        actField->setData(QString::fromStdString(fieldInfo->module()->fieldid));

        actFieldsGroup->addAction(actField);
        tlbFields->addAction(actField);
    }

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

    QTimer::singleShot(0, this, SLOT(showInfo()));
}

void PreprocessorView::clearNodes()
{
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

void PreprocessorView::doProblemDialog(QAction *action)
{
    FieldInfo *fieldInfo = Util::scene()->fieldInfo(action->data().toString());
    if (fieldInfo)
    {
        FieldDialog *fieldDialog = new FieldDialog(fieldInfo, this);
        if (fieldDialog->exec() == QDialog::Accepted)
            doInvalidated();

        delete fieldDialog;
    }
}

void PreprocessorView::doContextMenu(const QPoint &pos)
{
    logMessage("PreprocessorView::doContextMenu()");

    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    doItemSelected(item, 0);

    trvWidget->setCurrentItem(item);
    mnuPreprocessor->exec(QCursor::pos());
}

void PreprocessorView::doItemSelected(QTreeWidgetItem *item, int role)
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

void PreprocessorView::showInfo()
{
    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string info;
    ctemplate::TemplateDictionary problem("info");

    problem.SetValue("STYLESHEET", style);
    problem.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problem.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
    problem.SetValue("COORDINATE_TYPE", coordinateTypeString(Util::scene()->problemInfo()->coordinateType).toStdString());

    if (Util::scene()->problemInfo()->frequency > 0)
    {
        problem.SetValue("FREQUENCY_LABEL", tr("Frequency:").toStdString());
        problem.SetValue("FREQUENCY", QString::number(Util::scene()->problemInfo()->frequency).toStdString() + " Hz");
        problem.ShowSection("FREQUENCY");
    }
    if (Util::scene()->problemInfo()->timeStep.number() > 0)
    {
        problem.SetValue("TIME_STEP_LABEL", tr("Time step:").toStdString());
        problem.SetValue("TIME_STEP", QString::number(Util::scene()->problemInfo()->timeStep.number()).toStdString() + " s");
        problem.SetValue("TIME_TOTAL_LABEL", tr("Total time:").toStdString());
        problem.SetValue("TIME_TOTAL", QString::number(Util::scene()->problemInfo()->timeTotal.number()).toStdString() + " s");
        problem.ShowSection("TRANSIENT");
    }

    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        ctemplate::TemplateDictionary *field = problem.AddSectionDictionary("FIELD");

        field->SetValue("PHYSICAL_FIELDID", fieldInfo->module()->fieldid);
        field->SetValue("PHYSICAL_FIELD", fieldInfo->module()->name);

        field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
        field->SetValue("ANALYSIS_TYPE", analysisTypeString(fieldInfo->analysisType()).toStdString());

        field->SetValue("WEAK_FORMS_TYPE_LABEL", tr("Weak forms:").toStdString());
        field->SetValue("WEAK_FORMS_TYPE", weakFormsTypeString(fieldInfo->weakFormsType).toStdString());

        field->SetValue("REFINEMENS_NUMBER_LABEL", tr("Number of refinements:").toStdString());
        field->SetValue("REFINEMENS_NUMBER", QString::number(fieldInfo->numberOfRefinements).toStdString());
        field->SetValue("POLYNOMIAL_ORDER_LABEL", tr("Polynomial order:").toStdString());
        field->SetValue("POLYNOMIAL_ORDER", QString::number(fieldInfo->polynomialOrder).toStdString());

        field->SetValue("ADAPTIVITY_TYPE_LABEL", tr("Adaptivity:").toStdString());
        field->SetValue("ADAPTIVITY_TYPE", adaptivityTypeString(fieldInfo->adaptivityType).toStdString());

        if (fieldInfo->adaptivityType != AdaptivityType_None)
        {
            field->SetValue("ADAPTIVITY_STEPS_LABEL", tr("Steps:").toStdString());
            field->SetValue("ADAPTIVITY_STEPS", QString::number(fieldInfo->adaptivitySteps).toStdString());
            field->SetValue("ADAPTIVITY_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
            field->SetValue("ADAPTIVITY_TOLERANCE", QString::number(fieldInfo->adaptivityTolerance).toStdString());
            field->ShowSection("ADAPTIVITY_PARAMETERS_SECTION");
        }
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/preprocessor.tpl", ctemplate::DO_NOT_STRIP, &problem, &info);
    webView->setHtml(QString::fromStdString(info));

    setFocus();
}
