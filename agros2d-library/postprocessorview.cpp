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

#include "postprocessorview.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "pythonlab/pythonengine_agros.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "util/constants.h"

PostprocessorWidget::PostprocessorWidget(PostHermes *postHermes,
                                         SceneViewPreprocessor *sceneGeometry,
                                         SceneViewMesh *sceneMesh,
                                         SceneViewPost2D *scenePost2D,
                                         SceneViewPost3D *scenePost3D,
                                         QWidget *parent) : QWidget(parent)
{
    m_postHermes = postHermes;
    m_sceneGeometry = sceneGeometry;
    m_sceneMesh = sceneMesh;
    m_scenePost2D = scenePost2D;
    m_scenePost3D = scenePost3D;

    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorView");

    createControls();

    loadBasic();
    loadAdvanced();
}

void PostprocessorWidget::loadBasic()
{
    // show
    chkShowInitialMeshView->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowInitialMeshView).toBool());
    chkShowSolutionMeshView->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowSolutionMeshView).toBool());
    chkShowOrderView->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderView).toBool());
    txtOrderComponent->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_OrderComponent).toInt());

    radPost3DNone->setChecked((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt() == SceneViewPost3DMode_None);
    radPost3DScalarField3D->setChecked((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt() == SceneViewPost3DMode_ScalarView3D);
    radPost3DScalarField3DSolid->setChecked((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt() == SceneViewPost3DMode_ScalarView3DSolid);
    radPost3DModel->setChecked((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt() == SceneViewPost3DMode_Model);
    doPostprocessorGroupClicked(butPost3DGroup->checkedButton());

    chkShowPost2DContourView->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool());
    chkShowPost2DVectorView->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool());
    chkShowPost2DScalarView->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool());

    // contour field
    cmbPost2DContourVariable->setCurrentIndex(cmbPost2DContourVariable->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString()));
    if (cmbPost2DContourVariable->count() > 0 && cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()) != QVariant::Invalid)
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContourVariable,
                                                cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString());
    }
    if (cmbPost2DContourVariable->currentIndex() == -1 && cmbPost2DContourVariable->count() > 0)
    {
        // set first variable
        cmbPost2DContourVariable->setCurrentIndex(0);
    }

    // scalar field
    cmbPostScalarFieldVariable->setCurrentIndex(cmbPostScalarFieldVariable->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString()));
    if (cmbPostScalarFieldVariable->count() > 0 && cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariable, cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());
        doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());

        cmbPostScalarFieldVariableComp->setCurrentIndex(cmbPostScalarFieldVariableComp->findData((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt()));
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt());
    }
    if (cmbPostScalarFieldVariable->currentIndex() == -1 && cmbPostScalarFieldVariable->count() > 0)
    {
        // set first variable
        cmbPostScalarFieldVariable->setCurrentIndex(0);
    }

    // vector field
    if (cmbPost2DVectorFieldVariable->count() > 0 && cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        cmbPost2DVectorFieldVariable->setCurrentIndex(cmbPost2DVectorFieldVariable->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString()));
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorVariable, cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString());
    }
    if (cmbPost2DVectorFieldVariable->currentIndex() == -1 && cmbPost2DVectorFieldVariable->count() > 0)
    {
        // set first variable
        cmbPost2DVectorFieldVariable->setCurrentIndex(0);
    }
}

void PostprocessorWidget::loadAdvanced()
{
    // scalar field
    chkShowScalarColorBar->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool());
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt()));
    chkPaletteFilter->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteFilter).toBool());
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt());
    chkScalarDeform->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_DeformScalar).toBool());

    // contours
    txtContoursCount->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursCount).toInt());
    txtContourWidth->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursWidth).toDouble());
    chkContourDeform->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_DeformContour).toBool());

    // vector field
    chkVectorProportional->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorProportional).toBool());
    chkVectorColor->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorColor).toBool());
    txtVectorCount->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCount).toInt());
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble());
    cmbVectorType->setCurrentIndex(cmbVectorType->findData((VectorType) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorType).toInt()));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData((VectorCenter) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCenter).toInt()));
    chkVectorDeform->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_DeformVector).toBool());

    // order view
    chkShowOrderColorbar->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderColorBar).toBool());
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) Agros2D::problem()->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt()));
    chkOrderLabel->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderLabel).toBool());

    // advanced
    // scalar field
    chkScalarFieldRangeLog->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool());
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toDouble());
    txtScalarDecimalPlace->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt());
    chkScalarFieldRangeAuto->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool());
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble());
    txtScalarFieldRangeMax->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble());

    // solid
    lstSolidMaterials->clear();
    if (Agros2D::problem()->isSolved() && m_scenePost2D->postHermes()->activeViewField())
    {
        foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(m_scenePost2D->postHermes()->activeViewField()).items())
        {
            QListWidgetItem *item = new QListWidgetItem(lstSolidMaterials);
            item->setText(material->name());
            item->setData(Qt::UserRole, material->variant());
            if (Agros2D::problem()->setting()->value(ProblemSetting::View_SolidViewHide).toStringList().contains(material->name()))
                item->setCheckState(Qt::Unchecked);
            else
                item->setCheckState(Qt::Checked);

            lstSolidMaterials->addItem(item);
        }
    }

    chkView3DLighting->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool());
    txtView3DAngle->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DAngle).toDouble());
    chkView3DBackground->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DBackground).toBool());
    txtView3DHeight->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble());
    chkView3DBoundingBox->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DBoundingBox).toBool());
    chkView3DSolidGeometry->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DSolidGeometry).toBool());
}

void PostprocessorWidget::saveBasic()
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowInitialMeshView, chkShowInitialMeshView->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowSolutionMeshView, chkShowSolutionMeshView->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowOrderView, chkShowOrderView->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_OrderComponent, txtOrderComponent->value());

    // show
    if (radPost3DNone->isChecked()) Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, SceneViewPost3DMode_None);
    if (radPost3DScalarField3D->isChecked()) Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, SceneViewPost3DMode_ScalarView3D);
    if (radPost3DScalarField3DSolid->isChecked()) Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, SceneViewPost3DMode_ScalarView3DSolid);
    if (radPost3DModel->isChecked()) Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, SceneViewPost3DMode_Model);

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowContourView, chkShowPost2DContourView->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowScalarView, chkShowPost2DScalarView->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowVectorView, chkShowPost2DVectorView->isChecked());

    // contour field
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContourVariable, cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString());

    // scalar field
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariable, cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeAuto, chkScalarFieldRangeAuto->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeMin, txtScalarFieldRangeMin->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeMax, txtScalarFieldRangeMax->value());

    // vector field
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorVariable, cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString());
}

void PostprocessorWidget::saveAdvanced()
{
    // scalar field
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowScalarColorBar, chkShowScalarColorBar->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteType, (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteFilter, chkPaletteFilter->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PaletteSteps, txtPaletteSteps->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_DeformScalar, chkScalarDeform->isChecked());

    // contours
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContoursCount, txtContoursCount->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContoursWidth, txtContourWidth->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_DeformContour, chkContourDeform->isChecked());

    // vector field
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorProportional, chkVectorProportional->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorColor, chkVectorColor->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorCount, txtVectorCount->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorScale, txtVectorScale->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorType, (VectorType) cmbVectorType->itemData(cmbVectorType->currentIndex()).toInt());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorCenter, (VectorCenter) cmbVectorCenter->itemData(cmbVectorCenter->currentIndex()).toInt());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_DeformVector, chkVectorDeform->isChecked());

    // order view
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowOrderColorBar, chkShowOrderColorbar->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_OrderPaletteOrderType, (PaletteOrderType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowOrderLabel, chkOrderLabel->isChecked());

    // scalar view
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeLog, chkScalarFieldRangeLog->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeBase, txtScalarFieldRangeBase->text().toDouble());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarDecimalPlace, txtScalarDecimalPlace->value());

    // solid
    QStringList hideList;
    for (int i = 0; i < lstSolidMaterials->count(); i++)
    {
        if (lstSolidMaterials->item(i)->checkState() == Qt::Unchecked)
        {
            SceneMaterial *material = lstSolidMaterials->item(i)->data(Qt::UserRole).value<SceneMaterial *>();
            hideList.append(material->name());
        }
    }
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_SolidViewHide, hideList);

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DLighting, chkView3DLighting->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DAngle, txtView3DAngle->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DBackground, chkView3DBackground->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DHeight, txtView3DHeight->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DBoundingBox, chkView3DBoundingBox->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DSolidGeometry, chkView3DSolidGeometry->isChecked());
}

void PostprocessorWidget::createControls()
{
    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    basic = controlsBasic();
    advanced = controlsAdvanced();

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(basic);
    layoutMain->addWidget(advanced, 1);
    layoutMain->addWidget(btnOK, 0, Qt::AlignRight);

    refresh();

    groupPostScalarAdvanced->setVisible(false);
    groupPostContourAdvanced->setVisible(false);
    groupPostVectorAdvanced->setVisible(false);
    groupPostSolidAdvanced->setVisible(false);
    groupPost3DAdvanced->setVisible(false);

    setLayout(layoutMain);
}

QWidget *PostprocessorWidget::meshWidget()
{
    // layout mesh
    chkShowInitialMeshView = new QCheckBox(tr("Initial mesh"));
    chkShowSolutionMeshView = new QCheckBox(tr("Solution mesh"));
    chkShowOrderView = new QCheckBox(tr("Polynomial order"));
    connect(chkShowOrderView, SIGNAL(clicked()), this, SLOT(refresh()));

    txtOrderComponent = new QSpinBox(this);
    txtOrderComponent->setMinimum(1);

    QGridLayout *gridLayoutMesh = new QGridLayout();
    gridLayoutMesh->addWidget(chkShowInitialMeshView, 0, 0, 1, 2);
    gridLayoutMesh->addWidget(chkShowSolutionMeshView, 1, 0, 1, 2);
    gridLayoutMesh->addWidget(chkShowOrderView, 2, 0, 1, 2);
    gridLayoutMesh->addWidget(new QLabel(tr("Component:")), 3, 0);
    gridLayoutMesh->addWidget(txtOrderComponent, 3, 1);

    QGroupBox *grpShowMesh = new QGroupBox(tr("Mesh"));
    grpShowMesh->setLayout(gridLayoutMesh);

    return grpShowMesh;
}

QWidget *PostprocessorWidget::meshOrderWidget()
{
    // layout order
    cmbOrderPaletteOrder = new QComboBox();
    cmbOrderPaletteOrder->addItem(tr("Hermes"), PaletteOrder_Hermes);
    cmbOrderPaletteOrder->addItem(tr("Jet"), PaletteOrder_Jet);
    cmbOrderPaletteOrder->addItem(tr("Paruly"), PaletteOrder_Paruly);
    cmbOrderPaletteOrder->addItem(tr("Viridis"), PaletteOrder_Viridis);
    cmbOrderPaletteOrder->addItem(tr("B/W ascending"), PaletteOrder_BWAsc);
    cmbOrderPaletteOrder->addItem(tr("B/W descending"), PaletteOrder_BWDesc);

    chkShowOrderColorbar = new QCheckBox(tr("Show colorbar"), this);
    chkOrderLabel = new QCheckBox(tr("Show labels"), this);

    QGridLayout *gridLayoutOrder = new QGridLayout();
    gridLayoutOrder->setColumnStretch(1, 1);
    gridLayoutOrder->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutOrder->addWidget(cmbOrderPaletteOrder, 0, 1);
    gridLayoutOrder->addWidget(chkShowOrderColorbar, 1, 0, 1, 2);
    gridLayoutOrder->addWidget(chkOrderLabel, 2, 0, 1, 2);

    QGroupBox *grpShowOrder = new QGroupBox(tr("Polynomial order"));
    grpShowOrder->setLayout(gridLayoutOrder);

    return grpShowOrder;
}

QWidget *PostprocessorWidget::post2DWidget()
{
    // layout post2d
    chkShowPost2DContourView = new QCheckBox(tr("Contours"));
    connect(chkShowPost2DContourView, SIGNAL(clicked()), this, SLOT(refresh()));
    chkShowPost2DVectorView = new QCheckBox(tr("Vectors"));
    connect(chkShowPost2DVectorView, SIGNAL(clicked()), this, SLOT(refresh()));
    chkShowPost2DScalarView = new QCheckBox(tr("Scalar view"));
    connect(chkShowPost2DScalarView, SIGNAL(clicked()), this, SLOT(refresh()));

    QGridLayout *layoutPost2D = new QGridLayout();
    layoutPost2D->addWidget(chkShowPost2DScalarView, 0, 0);
    layoutPost2D->addWidget(chkShowPost2DContourView, 1, 0);
    layoutPost2D->addWidget(chkShowPost2DVectorView, 2, 0);
    layoutPost2D->setRowStretch(50, 1);

    QGroupBox *grpShowPost2D = new QGroupBox(tr("Postprocessor 2D"));
    grpShowPost2D->setLayout(layoutPost2D);

    return grpShowPost2D;
}

CollapsableGroupBoxButton *PostprocessorWidget::postScalarWidget()
{
    // layout scalar field
    cmbPostScalarFieldVariable = new QComboBox();
    connect(cmbPostScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));
    cmbPostScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox(tr("Auto range"));
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    groupPostScalarAdvanced = postScalarAdvancedWidget();

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutScalarField->setColumnStretch(1, 1);
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariable, 0, 1);
    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariableComp, 1, 1);
    layoutScalarField->addWidget(groupPostScalarAdvanced, 2, 0, 1, 2);

    CollapsableGroupBoxButton *grpScalarField = new CollapsableGroupBoxButton(tr("Scalar field"));
    connect(grpScalarField, SIGNAL(collapseEvent(bool)), this, SLOT(doScalarFieldExpandCollapse(bool)));
    grpScalarField->setCollapsed(true);
    grpScalarField->setLayout(layoutScalarField);

    return grpScalarField;
}

CollapsableGroupBoxButton *PostprocessorWidget::postContourWidget()
{
    // contour field
    cmbPost2DContourVariable = new QComboBox();

    groupPostContourAdvanced = postContourAdvancedWidget();

    QGridLayout *layoutContourField = new QGridLayout();
    layoutContourField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutContourField->setColumnStretch(1, 1);
    layoutContourField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutContourField->addWidget(cmbPost2DContourVariable, 0, 1);
    layoutContourField->addWidget(groupPostContourAdvanced, 1, 0, 1, 2);

    CollapsableGroupBoxButton *grpContourField = new CollapsableGroupBoxButton(tr("Contour field"));
    connect(grpContourField, SIGNAL(collapseEvent(bool)), this, SLOT(doContourFieldExpandCollapse(bool)));
    grpContourField->setCollapsed(true);
    grpContourField->setLayout(layoutContourField);

    return grpContourField;
}

CollapsableGroupBoxButton *PostprocessorWidget::postVectorWidget()
{
    // vector field
    cmbPost2DVectorFieldVariable = new QComboBox();

    groupPostVectorAdvanced = postVectorAdvancedWidget();

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutVectorField->setColumnStretch(1, 1);
    layoutVectorField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutVectorField->addWidget(cmbPost2DVectorFieldVariable, 0, 1);
    layoutVectorField->addWidget(groupPostVectorAdvanced, 1, 0, 1, 2);

    CollapsableGroupBoxButton *grpVectorField = new CollapsableGroupBoxButton(tr("Vector field"));
    connect(grpVectorField, SIGNAL(collapseEvent(bool)), this, SLOT(doVectorFieldExpandCollapse(bool)));
    grpVectorField->setCollapsed(true);
    grpVectorField->setLayout(layoutVectorField);

    return grpVectorField;
}

CollapsableGroupBoxButton *PostprocessorWidget::postSolidWidget()
{
    // solid view
    groupPostSolidAdvanced = postPostSolidAdvancedWidget();

    QVBoxLayout *layoutSolid = new QVBoxLayout();
    layoutSolid->addWidget(groupPostSolidAdvanced);

    CollapsableGroupBoxButton *grpSolidView = new CollapsableGroupBoxButton(tr("Solid view"));
    connect(grpSolidView, SIGNAL(collapseEvent(bool)), this, SLOT(doSolidExpandCollapse(bool)));
    grpSolidView->setCollapsed(true);
    grpSolidView->setLayout(layoutSolid);

    return grpSolidView;
}

CollapsableGroupBoxButton *PostprocessorWidget::postPost3DWidget()
{
    // solid view
    groupPost3DAdvanced = postPost3DAdvancedWidget();

    QVBoxLayout *layoutSolid = new QVBoxLayout();
    layoutSolid->addWidget(groupPost3DAdvanced);

    CollapsableGroupBoxButton *grp3DView = new CollapsableGroupBoxButton(tr("Settings"));
    connect(grp3DView, SIGNAL(collapseEvent(bool)), this, SLOT(doPost3DExpandCollapse(bool)));
    grp3DView->setCollapsed(true);
    grp3DView->setLayout(layoutSolid);

    return grp3DView;
}

QWidget *PostprocessorWidget::post3DWidget()
{
    // layout post3d
    radPost3DNone = new QRadioButton(tr("None"), this);
    radPost3DScalarField3D = new QRadioButton(tr("Scalar view"), this);
    radPost3DScalarField3DSolid = new QRadioButton(tr("Scalar view solid"), this);
    radPost3DModel = new QRadioButton("Model", this);

    butPost3DGroup = new QButtonGroup(this);
    butPost3DGroup->addButton(radPost3DNone);
    butPost3DGroup->addButton(radPost3DScalarField3D);
    butPost3DGroup->addButton(radPost3DScalarField3DSolid);
    butPost3DGroup->addButton(radPost3DModel);
    connect(butPost3DGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(doPostprocessorGroupClicked(QAbstractButton*)));

    QGridLayout *layoutPost3D = new QGridLayout();
    layoutPost3D->addWidget(radPost3DNone, 0, 0);
    layoutPost3D->addWidget(radPost3DScalarField3D, 1, 0);
    layoutPost3D->addWidget(radPost3DScalarField3DSolid, 2, 0);
    layoutPost3D->addWidget(radPost3DModel, 3, 0);
    layoutPost3D->setRowStretch(50, 1);

    QGroupBox *grpShowPost3D = new QGroupBox(tr("Postprocessor 3D"));
    grpShowPost3D->setLayout(layoutPost3D);

    return grpShowPost3D;
}

QWidget *PostprocessorWidget::controlsBasic()
{
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(doCalculationFinished()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(doCalculationFinished()));
    connect(currentPythonEngine(), SIGNAL(executedScript()), this, SLOT(doCalculationFinished()));

    fieldWidget = new PhysicalFieldWidget(this);
    connect(fieldWidget, SIGNAL(fieldChanged()), this, SLOT(doField()));

    groupMesh = meshWidget();
    groupMeshOrder = meshOrderWidget();
    groupPost2d = post2DWidget();
    groupPost3d = post3DWidget();

    widgetsLayout = new QStackedLayout();

    QVBoxLayout *layoutBasic = new QVBoxLayout();
    layoutBasic->setContentsMargins(2, 2, 2, 3);
    layoutBasic->addWidget(fieldWidget);
    layoutBasic->addLayout(widgetsLayout);
    layoutBasic->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutBasic);

    widgetsLayout->addWidget(groupMesh);
    widgetsLayout->addWidget(groupPost2d);
    widgetsLayout->addWidget(groupPost3d);

    return widget;
}

QWidget *PostprocessorWidget::controlsAdvanced()
{
    groupPostScalar = postScalarWidget();
    groupPostContour = postContourWidget();
    groupPostVector = postVectorWidget();
    groupPostSolid = postSolidWidget();
    groupPost3D = postPost3DWidget();

    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->addWidget(groupMeshOrder);
    layoutArea->addWidget(groupPostScalar);
    layoutArea->addWidget(groupPostContour);
    layoutArea->addWidget(groupPostVector);
    layoutArea->addWidget(groupPostSolid);
    layoutArea->addWidget(groupPost3D);
    layoutArea->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutArea);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    return widgetArea;
}

QWidget *PostprocessorWidget::postScalarAdvancedWidget()
{
    // scalar field
    // palette
    cmbPalette = new QComboBox();
    foreach (QString key, paletteTypeStringKeys())
        cmbPalette->addItem(paletteTypeString(paletteTypeFromStringKey(key)), paletteTypeFromStringKey(key));

    chkPaletteFilter = new QCheckBox(tr("Filter"));
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    // steps
    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(PALETTESTEPSMIN);
    txtPaletteSteps->setMaximum(PALETTESTEPSMAX);

    // log scale
    chkScalarFieldRangeLog = new QCheckBox(tr("Log. scale"));
    txtScalarFieldRangeBase = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarRangeBase).toDouble());
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    txtScalarFieldRangeMin = new LineEditDouble(0.1);
    connect(txtScalarFieldRangeMin, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMinChanged()));
    lblScalarFieldRangeMinError = new QLabel("");
    lblScalarFieldRangeMinError->setPalette(palette);
    lblScalarFieldRangeMinError->setVisible(false);
    txtScalarFieldRangeMax = new LineEditDouble(0.1);
    connect(txtScalarFieldRangeMax, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMaxChanged()));
    lblScalarFieldRangeMaxError = new QLabel("");
    lblScalarFieldRangeMaxError->setPalette(palette);
    lblScalarFieldRangeMaxError->setVisible(false);

    QGridLayout *layoutScalarFieldRange = new QGridLayout();
    lblScalarFieldRangeMin = new QLabel(tr("Minimum:"));
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMin, 0, 0);
    layoutScalarFieldRange->addWidget(txtScalarFieldRangeMin, 0, 1);
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMinError, 0, 2);
    layoutScalarFieldRange->addWidget(chkScalarFieldRangeAuto, 0, 3);
    lblScalarFieldRangeMax = new QLabel(tr("Maximum:"));
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMax, 1, 0);
    layoutScalarFieldRange->addWidget(txtScalarFieldRangeMax, 1, 1);
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMaxError, 1, 2);

    QGroupBox *grpScalarFieldRange = new QGroupBox(tr("Range"));
    grpScalarFieldRange->setLayout(layoutScalarFieldRange);

    QGridLayout *gridLayoutScalarFieldPalette = new QGridLayout();
    gridLayoutScalarFieldPalette->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutScalarFieldPalette->setColumnStretch(1, 1);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutScalarFieldPalette->addWidget(cmbPalette, 0, 1, 1, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Steps:")), 2, 0);
    gridLayoutScalarFieldPalette->addWidget(txtPaletteSteps, 2, 1);
    gridLayoutScalarFieldPalette->addWidget(chkPaletteFilter, 2, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Base:")), 3, 0);
    gridLayoutScalarFieldPalette->addWidget(txtScalarFieldRangeBase, 3, 1);
    gridLayoutScalarFieldPalette->addWidget(chkScalarFieldRangeLog, 3, 2);

    QGroupBox *grpScalarFieldPalette = new QGroupBox(tr("Palette"));
    grpScalarFieldPalette->setLayout(gridLayoutScalarFieldPalette);

    // decimal places
    txtScalarDecimalPlace = new QSpinBox(this);
    txtScalarDecimalPlace->setMinimum(SCALARDECIMALPLACEMIN);
    txtScalarDecimalPlace->setMaximum(SCALARDECIMALPLACEMAX);

    // color bar
    chkShowScalarColorBar = new QCheckBox(tr("Show colorbar"), this);

    QGridLayout *gridLayoutScalarFieldColorbar = new QGridLayout();
    gridLayoutScalarFieldColorbar->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutScalarFieldColorbar->setColumnStretch(1, 1);
    gridLayoutScalarFieldColorbar->addWidget(new QLabel(tr("Decimal places:")), 0, 0);
    gridLayoutScalarFieldColorbar->addWidget(txtScalarDecimalPlace, 0, 1);
    gridLayoutScalarFieldColorbar->addWidget(chkShowScalarColorBar, 0, 2);

    QGroupBox *grpScalarFieldColorbar = new QGroupBox(tr("Colorbar"));
    grpScalarFieldColorbar->setLayout(gridLayoutScalarFieldColorbar);

    chkScalarDeform = new QCheckBox(tr("Deform shape"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkScalarDeform, 0, 0);

    QGroupBox *grpScalarDeformShape = new QGroupBox(tr("Displacement"));
    grpScalarDeformShape->setLayout(layoutDeformShape);

    QVBoxLayout *layoutScalarFieldAdvanced = new QVBoxLayout();
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldPalette);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldColorbar);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldRange);
    layoutScalarFieldAdvanced->addWidget(grpScalarDeformShape);

    QWidget *scalarWidget = new QWidget();
    scalarWidget->setLayout(layoutScalarFieldAdvanced);

    return scalarWidget;
}

QWidget *PostprocessorWidget::postContourAdvancedWidget()
{
    // contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(CONTOURSCOUNTMIN);
    txtContoursCount->setMaximum(CONTOURSCOUNTMAX);
    txtContourWidth = new QDoubleSpinBox(this);
    txtContourWidth->setMinimum(CONTOURSWIDTHMIN);
    txtContourWidth->setMaximum(CONTOURSWIDTHMAX);
    txtContourWidth->setSingleStep(0.1);

    chkContourDeform = new QCheckBox(tr("Deform shape"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkContourDeform, 0, 0);

    QGroupBox *grpContourDeformShape = new QGroupBox(tr("Displacement"));
    grpContourDeformShape->setLayout(layoutDeformShape);

    QGridLayout *gridLayoutContours = new QGridLayout();
    gridLayoutContours->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutContours->setColumnStretch(1, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Number of contours:")), 0, 0);
    gridLayoutContours->addWidget(txtContoursCount, 0, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Contour width:")), 1, 0);
    gridLayoutContours->addWidget(txtContourWidth, 1, 1);
    gridLayoutContours->addWidget(grpContourDeformShape, 2, 0, 1, 2);

    QWidget *contourWidget = new QWidget();
    contourWidget->setLayout(gridLayoutContours);

    return contourWidget;
}

QWidget *PostprocessorWidget::postVectorAdvancedWidget()
{
    // vectors
    chkVectorProportional = new QCheckBox(tr("Proportional"), this);
    chkVectorColor = new QCheckBox(tr("Color (b/w)"), this);
    txtVectorCount = new QSpinBox(this);
    txtVectorCount->setMinimum(VECTORSCOUNTMIN);
    txtVectorCount->setMaximum(VECTORSCOUNTMAX);
    txtVectorScale = new QDoubleSpinBox(this);
    txtVectorScale->setDecimals(2);
    txtVectorScale->setSingleStep(0.1);
    txtVectorScale->setMinimum(VECTORSSCALEMIN);
    txtVectorScale->setMaximum(VECTORSSCALEMAX);
    cmbVectorType = new QComboBox();
    foreach (QString key, vectorTypeStringKeys())
        cmbVectorType->addItem(vectorTypeString(vectorTypeFromStringKey(key)), vectorTypeFromStringKey(key));
    cmbVectorCenter = new QComboBox();
    foreach (QString key, vectorCenterStringKeys())
        cmbVectorCenter->addItem(vectorCenterString(vectorCenterFromStringKey(key)), vectorCenterFromStringKey(key));

    chkVectorDeform = new QCheckBox(tr("Deform shape"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkVectorDeform, 0, 0);

    QGroupBox *grpVectorDeformShape = new QGroupBox(tr("Displacement"));
    grpVectorDeformShape->setLayout(layoutDeformShape);

    QGridLayout *gridLayoutVectors = new QGridLayout();
    gridLayoutVectors->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutVectors->setColumnStretch(1, 1);
    gridLayoutVectors->addWidget(new QLabel(tr("Number of vec.:")), 0, 0);
    gridLayoutVectors->addWidget(txtVectorCount, 0, 1);
    gridLayoutVectors->addWidget(chkVectorProportional, 0, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Scale:")), 1, 0);
    gridLayoutVectors->addWidget(txtVectorScale, 1, 1);
    gridLayoutVectors->addWidget(chkVectorColor, 1, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Type:")), 2, 0);
    gridLayoutVectors->addWidget(cmbVectorType, 2, 1, 1, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Center:")), 3, 0);
    gridLayoutVectors->addWidget(cmbVectorCenter, 3, 1, 1, 2);
    gridLayoutVectors->addWidget(grpVectorDeformShape, 4, 0, 1, 2);

    QWidget *vectorWidget = new QWidget();
    vectorWidget->setLayout(gridLayoutVectors);

    return vectorWidget;
}

QWidget *PostprocessorWidget::postPostSolidAdvancedWidget()
{
    lstSolidMaterials = new QListWidget();

    QGridLayout *gridLayoutSolid = new QGridLayout();
    gridLayoutSolid->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutSolid->setColumnStretch(0, 1);
    gridLayoutSolid->addWidget(lstSolidMaterials, 0, 0);

    QWidget *solidWidget = new QWidget();
    solidWidget->setLayout(gridLayoutSolid);

    return solidWidget;
}

QWidget *PostprocessorWidget::postPost3DAdvancedWidget()
{
    // layout 3d
    chkView3DLighting = new QCheckBox(tr("Ligthing"), this);
    txtView3DAngle = new QDoubleSpinBox(this);
    txtView3DAngle->setDecimals(1);
    txtView3DAngle->setSingleStep(1);
    txtView3DAngle->setMinimum(30);
    txtView3DAngle->setMaximum(360);
    chkView3DBackground = new QCheckBox(tr("Gradient back."), this);
    txtView3DHeight = new QDoubleSpinBox(this);
    txtView3DHeight->setDecimals(1);
    txtView3DHeight->setSingleStep(0.1);
    txtView3DHeight->setMinimum(0.2);
    txtView3DHeight->setMaximum(10.0);
    chkView3DBoundingBox = new QCheckBox(tr("Bounding box"), this);
    chkView3DSolidGeometry = new QCheckBox(tr("Show edges"), this);

    QGridLayout *layout3D = new QGridLayout();
    layout3D->setColumnMinimumWidth(0, columnMinimumWidth());
    layout3D->addWidget(new QLabel(tr("Angle:")), 0, 0);
    layout3D->addWidget(txtView3DAngle, 0, 1);
    layout3D->addWidget(new QLabel(tr("Height:")), 1, 0);
    layout3D->addWidget(txtView3DHeight, 1, 1);
    layout3D->addWidget(chkView3DLighting, 2, 0, 1, 2);
    layout3D->addWidget(chkView3DBackground, 3, 0, 1, 2);
    layout3D->addWidget(chkView3DBoundingBox, 4, 0, 1, 2);
    layout3D->addWidget(chkView3DSolidGeometry, 5, 0, 1, 2);

    QWidget *viewWidget = new QWidget();
    viewWidget->setLayout(layout3D);

    return viewWidget;
}

void PostprocessorWidget::doField()
{
    fillComboBoxScalarVariable(fieldWidget->selectedField(), cmbPostScalarFieldVariable);
    fillComboBoxContourVariable(fieldWidget->selectedField(), cmbPost2DContourVariable);
    fillComboBoxVectorVariable(fieldWidget->selectedField(), cmbPost2DVectorFieldVariable);
    doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());

    txtOrderComponent->setMaximum(fieldWidget->selectedField()->numberOfSolutions());
}

void PostprocessorWidget::doCalculationFinished()
{
    if (currentPythonEngine()->isScriptRunning())
        return;

    if (Agros2D::problem()->isSolved())
    {
        fieldWidget->selectField(m_postHermes->activeViewField());
        fieldWidget->selectTimeStep(m_postHermes->activeTimeStep());
        fieldWidget->selectAdaptivityStep(m_postHermes->activeAdaptivityStep());
        fieldWidget->selectedAdaptivitySolutionType(m_postHermes->activeAdaptivitySolutionType());
    }

    if ((Agros2D::problem()->isMeshed() && !Agros2D::problem()->isSolving()) || Agros2D::problem()->isSolved())
        emit apply();
}

void PostprocessorWidget::doScalarFieldVariable(int index)
{
    if (!fieldWidget->selectedField())
        return;

    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    if (cmbPostScalarFieldVariable->currentIndex() != -1)
    {
        QString variableName(cmbPostScalarFieldVariable->itemData(index).toString());

        QString fieldName = fieldWidget->selectedField()->fieldId();
        Module::LocalVariable physicFieldVariable = Agros2D::problem()->fieldInfo(fieldName)->localVariable(variableName);

        // component
        cmbPostScalarFieldVariableComp->clear();
        if (physicFieldVariable.isScalar())
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
        }
        else
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
            cmbPostScalarFieldVariableComp->addItem(Agros2D::problem()->config()->labelX(), PhysicFieldVariableComp_X);
            cmbPostScalarFieldVariableComp->addItem(Agros2D::problem()->config()->labelY(), PhysicFieldVariableComp_Y);
        }

        cmbPostScalarFieldVariableComp->setCurrentIndex(cmbPostScalarFieldVariableComp->findData(scalarFieldVariableComp));
        if (cmbPostScalarFieldVariableComp->currentIndex() == -1)
            cmbPostScalarFieldVariableComp->setCurrentIndex(0);
    }
}

void PostprocessorWidget::doScalarFieldRangeAuto(int state)
{
    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());
}

void PostprocessorWidget::doPaletteFilter(int state)
{
    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}

void PostprocessorWidget::refresh()
{
    fieldWidget->updateControls();

    if (m_sceneMesh->actSceneModeMesh->isChecked())
    {
        widgetsLayout->setCurrentWidget(groupMesh);

        // mesh and order
        chkShowInitialMeshView->setEnabled(Agros2D::problem()->isMeshed());
        chkShowSolutionMeshView->setEnabled(Agros2D::problem()->isSolved());
        chkShowOrderView->setEnabled(Agros2D::problem()->isSolved());
        txtOrderComponent->setEnabled(Agros2D::problem()->isSolved() && (chkShowOrderView->isChecked() || chkShowSolutionMeshView->isChecked()));

        // mesh
        groupMeshOrder->setVisible(Agros2D::problem()->isSolved() && chkShowOrderView->isChecked());

        // scalar
        groupPostScalar->setVisible(false);
        groupPostScalarAdvanced->setVisible(false);
        // contour
        groupPostContour->setVisible(false);
        groupPostContourAdvanced->setVisible(false);
        // vector
        groupPostVector->setVisible(false);
        groupPostVectorAdvanced->setVisible(false);
        // solid
        groupPostSolid->setVisible(false);
        groupPostSolidAdvanced->setVisible(false);
        // 3d
        groupPost3D->setVisible(false);
        groupPost3DAdvanced->setVisible(false);
    }

    if (m_scenePost2D->actSceneModePost2D->isChecked())
    {
        widgetsLayout->setCurrentWidget(groupPost2d);

        chkShowPost2DContourView->setEnabled(Agros2D::problem()->isSolved() && (cmbPost2DContourVariable->count() > 0));
        chkShowPost2DScalarView->setEnabled(Agros2D::problem()->isSolved() && (cmbPostScalarFieldVariable->count() > 0));
        chkShowPost2DVectorView->setEnabled(Agros2D::problem()->isSolved() && (cmbPost2DVectorFieldVariable->count() > 0));

        // mesh
        groupMeshOrder->setVisible(false);

        // contour
        groupPostContour->setVisible(chkShowPost2DContourView->isEnabled() && chkShowPost2DContourView->isChecked());
        groupPostContourAdvanced->setVisible(chkShowPost2DContourView->isEnabled() && chkShowPost2DContourView->isChecked() && !groupPostContour->isCollapsed());

        // scalar view
        groupPostScalar->setVisible(chkShowPost2DScalarView->isEnabled() && chkShowPost2DScalarView->isChecked());
        groupPostScalarAdvanced->setVisible(chkShowPost2DScalarView->isEnabled() && chkShowPost2DScalarView->isChecked() && !groupPostScalar->isCollapsed());

        // vector view
        groupPostVector->setVisible(chkShowPost2DVectorView->isEnabled() && chkShowPost2DVectorView->isChecked());
        groupPostVectorAdvanced->setVisible(chkShowPost2DVectorView->isEnabled() && chkShowPost2DVectorView->isChecked() && !groupPostVector->isCollapsed());

        // solid
        groupPostSolid->setVisible(false);
        groupPostSolidAdvanced->setVisible(false);

        // 3d
        groupPost3D->setVisible(false);
        groupPost3DAdvanced->setVisible(false);
    }

    if (m_scenePost3D->actSceneModePost3D->isChecked())
    {
        widgetsLayout->setCurrentWidget(groupPost3d);

        // mesh
        groupMeshOrder->setVisible(false);

        // scalar view 3d
        radPost3DNone->setEnabled(Agros2D::problem()->isSolved());
        radPost3DScalarField3D->setEnabled(Agros2D::problem()->isSolved());
        radPost3DScalarField3DSolid->setEnabled(Agros2D::problem()->isSolved());
        radPost3DModel->setEnabled(Agros2D::problem()->isSolved());

        // scalar
        groupPostScalar->setVisible((radPost3DScalarField3D->isEnabled() && radPost3DScalarField3D->isChecked())
                                    || (radPost3DScalarField3DSolid->isEnabled() && radPost3DScalarField3DSolid->isChecked()));
        groupPostScalarAdvanced->setVisible(((radPost3DScalarField3D->isEnabled() && radPost3DScalarField3D->isChecked())
                                             || (radPost3DScalarField3DSolid->isEnabled() && radPost3DScalarField3DSolid->isChecked()))
                                            && !groupPostScalar->isCollapsed());

        // contour
        groupPostContour->setVisible(false);
        groupPostContourAdvanced->setVisible(false);

        // vector
        groupPostVector->setVisible(false);
        groupPostVectorAdvanced->setVisible(false);

        // solid
        groupPostSolid->setVisible((radPost3DScalarField3DSolid->isEnabled() && radPost3DScalarField3DSolid->isChecked())
                                   || (radPost3DModel->isEnabled() && radPost3DModel->isChecked()));
        groupPostSolidAdvanced->setVisible(((radPost3DScalarField3DSolid->isEnabled() && radPost3DScalarField3DSolid->isChecked())
                                            || (radPost3DModel->isEnabled() && radPost3DModel->isChecked()))
                                           && !groupPostSolid->isCollapsed());

        // 3d
        groupPost3D->setVisible(true);
        groupPost3DAdvanced->setVisible(!groupPost3D->isCollapsed());
    }

    // scalar view
    if (groupPostScalar->isVisible())
    {
        doScalarFieldRangeAuto(-1);
    }
}

void PostprocessorWidget::updateControls()
{
    if (Agros2D::problem()->isMeshed())
    {
        fieldWidget->updateControls();

        if (Agros2D::problem()->isSolved())
        {
            loadBasic();
            loadAdvanced();
        }
    }

    refresh();
}

void PostprocessorWidget::doPostprocessorGroupClicked(QAbstractButton *button)
{
    refresh();
}

void PostprocessorWidget::doApply()
{
    m_postHermes->setActiveViewField(fieldWidget->selectedField());
    m_postHermes->setActiveTimeStep(fieldWidget->selectedTimeStep());
    m_postHermes->setActiveAdaptivityStep(fieldWidget->selectedAdaptivityStep());
    m_postHermes->setActiveAdaptivitySolutionType(fieldWidget->selectedAdaptivitySolutionType());

    // qDebug() << "doApply: " << fieldWidget->selectedField()->fieldId() << fieldWidget->selectedTimeStep() << fieldWidget->selectedAdaptivityStep() << fieldWidget->selectedAdaptivitySolutionType();

    saveBasic();
    saveAdvanced();

    // time step
    // QApplication::processEvents();

    // read auto range values
    if (chkScalarFieldRangeAuto->isChecked() && m_postHermes->linScalarView())
    {
        txtScalarFieldRangeMin->setValue(m_postHermes->linScalarView()->get_min_value());
        txtScalarFieldRangeMax->setValue(m_postHermes->linScalarView()->get_max_value());
    }
    else
    {
        txtScalarFieldRangeMin->setValue(0.0);
        txtScalarFieldRangeMax->setValue(0.0);
    }

    refresh();

    // refresh
    emit apply();

    activateWindow();
}

void PostprocessorWidget::doScalarFieldExpandCollapse(bool collapsed)
{
    groupPostScalarAdvanced->setVisible(!collapsed);
}

void PostprocessorWidget::doContourFieldExpandCollapse(bool collapsed)
{
    groupPostContourAdvanced->setVisible(!collapsed);
}

void PostprocessorWidget::doVectorFieldExpandCollapse(bool collapsed)
{
    groupPostVectorAdvanced->setVisible(!collapsed);
}

void PostprocessorWidget::doSolidExpandCollapse(bool collapsed)
{
    groupPostSolidAdvanced->setVisible(!collapsed);
}

void PostprocessorWidget::doPost3DExpandCollapse(bool collapsed)
{
    groupPost3DAdvanced->setVisible(!collapsed);
}

void PostprocessorWidget::doScalarFieldDefault()
{
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PaletteType).toInt()));
    chkPaletteFilter->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PaletteFilter).toBool());
    txtPaletteSteps->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PaletteSteps).toInt());
    chkShowScalarColorBar->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowScalarColorBar).toBool());
    chkScalarFieldRangeLog->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarRangeLog).toBool());
    txtScalarFieldRangeBase->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarRangeBase).toInt());
    txtScalarDecimalPlace->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarDecimalPlace).toInt());
    chkScalarDeform->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_DeformScalar).toBool());
}

void PostprocessorWidget::doContoursVectorsDefault()
{
    txtContoursCount->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ContoursCount).toInt());
    chkContourDeform->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_DeformContour).toBool());

    chkVectorProportional->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_VectorProportional).toBool());
    chkVectorColor->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_VectorColor).toBool());
    txtVectorCount->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_VectorCount).toInt());
    txtVectorScale->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_VectorScale).toDouble());
    cmbVectorType->setCurrentIndex(cmbVectorType->findData((VectorType) Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_VectorType).toInt()));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData((VectorCenter) Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_VectorCenter).toInt()));
    chkVectorDeform->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_DeformVector).toBool());
}

void PostprocessorWidget::doOrderDefault()
{
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_OrderPaletteOrderType).toInt()));
    chkShowOrderColorbar->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowOrderColorBar).toBool());
    chkOrderLabel->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowOrderLabel).toBool());
}

void PostprocessorWidget::doScalarFieldRangeMinChanged()
{
    lblScalarFieldRangeMinError->clear();
    lblScalarFieldRangeMinError->setVisible(false);
    lblScalarFieldRangeMaxError->clear();
    lblScalarFieldRangeMaxError->setVisible(false);
    btnOK->setEnabled(true);

    if (txtScalarFieldRangeMin->value() > txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("> %1").arg(txtScalarFieldRangeMax->value()));
        lblScalarFieldRangeMinError->setVisible(true);
        //btnOK->setDisabled(true);
    }
    /*
    else if (txtScalarFieldRangeMin->value() == txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("= %1").arg(txtScalarFieldRangeMax->value()));
        btnOK->setDisabled(true);
    }
    */
}

void PostprocessorWidget::doScalarFieldRangeMaxChanged()
{
    lblScalarFieldRangeMaxError->clear();
    lblScalarFieldRangeMinError->clear();
    btnOK->setEnabled(true);

    if (txtScalarFieldRangeMax->value() < txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("< %1").arg(txtScalarFieldRangeMin->value()));
        //btnOK->setDisabled(true);
    }
    /*
    else if (txtScalarFieldRangeMax->value() == txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("= %1").arg(txtScalarFieldRangeMin->value()));
        btnOK->setDisabled(true);
    }
    */
}

void PostprocessorWidget::doScalarFieldLog(int state)
{
    txtScalarFieldRangeBase->setEnabled(chkScalarFieldRangeLog->isChecked());
}
