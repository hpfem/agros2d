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

#include "scene.h"
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

const double minWidth = 110;

PostprocessorWidget::PostprocessorWidget(SceneViewPreprocessor *sceneGeometry,
                                         SceneViewMesh *sceneMesh,
                                         SceneViewPost2D *scenePost2D,
                                         SceneViewPost3D *scenePost3D,
                                         ChartWidget *sceneChart,
                                         QWidget *parent) : QWidget(parent)
{
    m_sceneGeometry = sceneGeometry;
    m_sceneMesh = sceneMesh;
    m_scenePost2D = scenePost2D;
    m_scenePost3D = scenePost3D;
    m_sceneChart = sceneChart;

    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorView");

    createControls();

    loadBasic();
    loadAdvanced();

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(updateControls()));
    connect(currentPythonEngineAgros(), SIGNAL(executedExpression()), this, SLOT(updateControls()));
}

void PostprocessorWidget::loadBasic()
{
    // cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(Agros2D::problem()->configView()->activeField));
    if (cmbFieldInfo->currentIndex() == -1)
        cmbFieldInfo->setCurrentIndex(0);
    doFieldInfo(cmbFieldInfo->currentIndex());

    // show
    chkShowInitialMeshView->setChecked(Agros2D::problem()->configView()->showInitialMeshView);
    chkShowSolutionMeshView->setChecked(Agros2D::problem()->configView()->showSolutionMeshView);
    chkShowOrderView->setChecked(Agros2D::problem()->configView()->showOrderView);

    radPost3DNone->setChecked(Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_None);
    radPost3DScalarField3D->setChecked(Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3D);
    radPost3DScalarField3DSolid->setChecked(Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3DSolid);
    radPost3DModel->setChecked(Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_Model);
    radPost3DParticleTracing->setChecked(Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ParticleTracing);
    doPostprocessorGroupClicked(butPost3DGroup->checkedButton());

    chkShowPost2DContourView->setChecked(Agros2D::problem()->configView()->showContourView);
    chkShowPost2DVectorView->setChecked(Agros2D::problem()->configView()->showVectorView);
    chkShowPost2DScalarView->setChecked(Agros2D::problem()->configView()->showScalarView);
    chkShowPost2DParticleView->setChecked(Agros2D::problem()->configView()->showParticleView);

    // contour field
    cmbPost2DContourVariable->setCurrentIndex(cmbPost2DContourVariable->findData(Agros2D::problem()->configView()->contourVariable));
    if (cmbPost2DContourVariable->currentIndex() == -1 && cmbPost2DContourVariable->count() > 0)
    {
        // set first variable
        cmbPost2DContourVariable->setCurrentIndex(0);
    }
    Agros2D::problem()->configView()->contourVariable = cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString();

    // scalar field
    cmbPostScalarFieldVariable->setCurrentIndex(cmbPostScalarFieldVariable->findData(Agros2D::problem()->configView()->scalarVariable));
    if (cmbPostScalarFieldVariable->currentIndex() == -1 && cmbPostScalarFieldVariable->count() > 0)
    {
        // set first variable
        cmbPostScalarFieldVariable->setCurrentIndex(0);
    }
    Agros2D::problem()->configView()->scalarVariable = cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString();
    doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());
    Agros2D::problem()->configView()->scalarVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    // vector field
    cmbPost2DVectorFieldVariable->setCurrentIndex(cmbPost2DVectorFieldVariable->findData(Agros2D::problem()->configView()->vectorVariable));
    if (cmbPost2DVectorFieldVariable->currentIndex() == -1 && cmbPost2DVectorFieldVariable->count() > 0)
    {
        // set first variable
        cmbPost2DVectorFieldVariable->setCurrentIndex(0);
    }
    Agros2D::problem()->configView()->vectorVariable = cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString();

    // transient view
    // cmbTimeStep->setCurrentIndex(Agros2D::scene()->sceneSolution()->timeStep());

    refresh();
}

void PostprocessorWidget::loadAdvanced()
{
    // scalar field
    chkShowScalarColorBar->setChecked(Agros2D::problem()->configView()->showScalarColorBar);
    cmbPalette->setCurrentIndex(cmbPalette->findData(Agros2D::problem()->configView()->paletteType));
    chkPaletteFilter->setChecked(Agros2D::problem()->configView()->paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(Agros2D::problem()->configView()->paletteSteps);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(Agros2D::problem()->configView()->linearizerQuality));

    // contours
    txtContoursCount->setValue(Agros2D::problem()->configView()->contoursCount);
    txtContourWidth->setValue(Agros2D::problem()->configView()->contourWidth);

    // vector field
    chkVectorProportional->setChecked(Agros2D::problem()->configView()->vectorProportional);
    chkVectorColor->setChecked(Agros2D::problem()->configView()->vectorColor);
    txtVectorCount->setValue(Agros2D::problem()->configView()->vectorCount);
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(Agros2D::problem()->configView()->vectorScale);
    cmbVectorType->setCurrentIndex(cmbVectorType->findData(Agros2D::problem()->configView()->vectorType));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData(Agros2D::problem()->configView()->vectorCenter));

    // order view
    chkShowOrderColorbar->setChecked(Agros2D::problem()->configView()->showOrderColorBar);
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData(Agros2D::problem()->configView()->orderPaletteOrderType));
    chkOrderLabel->setChecked(Agros2D::problem()->configView()->orderLabel);

    // particle tracing
    chkParticleIncludeRelativisticCorrection->setChecked(Agros2D::problem()->configView()->particleIncludeRelativisticCorrection);
    txtParticleNumberOfParticles->setValue(Agros2D::problem()->configView()->particleNumberOfParticles);
    txtParticleStartingRadius->setValue(Agros2D::problem()->configView()->particleStartingRadius);
    txtParticleMass->setValue(Agros2D::problem()->configView()->particleMass);
    txtParticleConstant->setValue(Agros2D::problem()->configView()->particleConstant);
    txtParticlePointX->setValue(Agros2D::problem()->configView()->particleStart.x);
    txtParticlePointY->setValue(Agros2D::problem()->configView()->particleStart.y);
    txtParticleVelocityX->setValue(Agros2D::problem()->configView()->particleStartVelocity.x);
    txtParticleVelocityY->setValue(Agros2D::problem()->configView()->particleStartVelocity.y);
    chkParticleReflectOnDifferentMaterial->setChecked(Agros2D::problem()->configView()->particleReflectOnDifferentMaterial);
    chkParticleReflectOnBoundary->setChecked(Agros2D::problem()->configView()->particleReflectOnBoundary);
    txtParticleCoefficientOfRestitution->setValue(Agros2D::problem()->configView()->particleCoefficientOfRestitution);
    txtParticleCustomForceX->setValue(Agros2D::problem()->configView()->particleCustomForce.x);
    txtParticleCustomForceY->setValue(Agros2D::problem()->configView()->particleCustomForce.y);
    txtParticleCustomForceZ->setValue(Agros2D::problem()->configView()->particleCustomForce.z);
    txtParticleMaximumRelativeError->setValue(Agros2D::problem()->configView()->particleMaximumRelativeError);
    txtParticleMinimumStep->setValue(Agros2D::problem()->configView()->particleMinimumStep);
    txtParticleMaximumNumberOfSteps->setValue(Agros2D::problem()->configView()->particleMaximumNumberOfSteps);
    chkParticleColorByVelocity->setChecked(Agros2D::problem()->configView()->particleColorByVelocity);
    chkParticleShowPoints->setChecked(Agros2D::problem()->configView()->particleShowPoints);
    txtParticleDragDensity->setValue(Agros2D::problem()->configView()->particleDragDensity);
    txtParticleDragReferenceArea->setValue(Agros2D::problem()->configView()->particleDragReferenceArea);
    txtParticleDragCoefficient->setValue(Agros2D::problem()->configView()->particleDragCoefficient);

    lblParticlePointX->setText(QString("%1 (m):").arg(Agros2D::problem()->config()->labelX()));
    lblParticlePointY->setText(QString("%1 (m):").arg(Agros2D::problem()->config()->labelY()));
    lblParticleVelocityX->setText(QString("%1 (m/s):").arg(Agros2D::problem()->config()->labelX()));
    lblParticleVelocityY->setText(QString("%1 (m/s):").arg(Agros2D::problem()->config()->labelY()));
    lblParticleCustomForceX->setText(QString("%1 (N):").arg(Agros2D::problem()->config()->labelX()));
    lblParticleCustomForceY->setText(QString("%1 (N):").arg(Agros2D::problem()->config()->labelY()));
    lblParticleCustomForceZ->setText(QString("%1 (N):").arg(Agros2D::problem()->config()->labelZ()));

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
        lblParticleMotionEquations->setText(QString("<i>x</i>\" = <i>F</i><sub>x</sub> / <i>m</i>, &nbsp; <i>y</i>\" = <i>F</i><sub>y</sub> / <i>m</i>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>"));
    else
        lblParticleMotionEquations->setText(QString("<i>r</i>\" = <i>F</i><sub>r</sub> / <i>m</i> + <i>r</i> (<i>&phi;</i>')<sup>2</sup>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>, &nbsp; <i>&phi;</i>\" = <i>F</i><sub>&phi;</sub> / <i>m</i> - 2<i>r</i> <i>r</i>' <i>&phi;</i>' / <i>r</i>"));

    // advanced
    // scalar field
    chkScalarFieldRangeLog->setChecked(Agros2D::problem()->configView()->scalarRangeLog);
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setText(QString::number(Agros2D::problem()->configView()->scalarRangeBase));
    txtScalarDecimalPlace->setValue(Agros2D::problem()->configView()->scalarDecimalPlace);
    chkScalarFieldRangeAuto->setChecked(Agros2D::problem()->configView()->scalarRangeAuto);
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setValue(Agros2D::problem()->configView()->scalarRangeMin);
    txtScalarFieldRangeMax->setValue(Agros2D::problem()->configView()->scalarRangeMax);

    // solid
    lstSolidMaterials->clear();
    if (Agros2D::problem()->isSolved())
    {
        foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(Agros2D::scene()->activeViewField()).items())
        {
            QListWidgetItem *item = new QListWidgetItem(lstSolidMaterials);
            item->setText(material->name());
            item->setData(Qt::UserRole, material->variant());
            if (Agros2D::problem()->configView()->solidViewHide.contains(material->name()))
                item->setCheckState(Qt::Unchecked);
            else
                item->setCheckState(Qt::Checked);

            lstSolidMaterials->addItem(item);
        }
    }
}

void PostprocessorWidget::saveBasic()
{
    // active field
    Agros2D::problem()->configView()->activeField = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();

    Agros2D::problem()->configView()->showInitialMeshView = chkShowInitialMeshView->isChecked();
    Agros2D::problem()->configView()->showSolutionMeshView = chkShowSolutionMeshView->isChecked();
    Agros2D::problem()->configView()->showOrderView = chkShowOrderView->isChecked();

    // show
    if (radPost3DNone->isChecked()) Agros2D::problem()->configView()->showPost3D = SceneViewPost3DMode_None;
    if (radPost3DScalarField3D->isChecked()) Agros2D::problem()->configView()->showPost3D = SceneViewPost3DMode_ScalarView3D;
    if (radPost3DScalarField3DSolid->isChecked()) Agros2D::problem()->configView()->showPost3D = SceneViewPost3DMode_ScalarView3DSolid;
    if (radPost3DModel->isChecked()) Agros2D::problem()->configView()->showPost3D = SceneViewPost3DMode_Model;
    if (radPost3DParticleTracing->isChecked()) Agros2D::problem()->configView()->showPost3D = SceneViewPost3DMode_ParticleTracing;

    Agros2D::problem()->configView()->showContourView = chkShowPost2DContourView->isChecked();
    Agros2D::problem()->configView()->showScalarView = chkShowPost2DScalarView->isChecked();
    Agros2D::problem()->configView()->showVectorView = chkShowPost2DVectorView->isChecked();
    Agros2D::problem()->configView()->showParticleView = chkShowPost2DParticleView->isChecked();

    // contour field
    Agros2D::problem()->configView()->contourVariable = cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString();

    // scalar field
    Agros2D::problem()->configView()->scalarVariable = cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString();
    Agros2D::problem()->configView()->scalarVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();
    Agros2D::problem()->configView()->scalarRangeAuto = chkScalarFieldRangeAuto->isChecked();
    Agros2D::problem()->configView()->scalarRangeMin = txtScalarFieldRangeMin->value();
    Agros2D::problem()->configView()->scalarRangeMax = txtScalarFieldRangeMax->value();

    // vector field
    Agros2D::problem()->configView()->vectorVariable = cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString();
}

void PostprocessorWidget::saveAdvanced()
{
    // scalar field
    Agros2D::problem()->configView()->showScalarColorBar = chkShowScalarColorBar->isChecked();
    Agros2D::problem()->configView()->paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    Agros2D::problem()->configView()->paletteFilter = chkPaletteFilter->isChecked();
    Agros2D::problem()->configView()->paletteSteps = txtPaletteSteps->value();
    Agros2D::problem()->configView()->linearizerQuality = (PaletteQuality) cmbLinearizerQuality->itemData(cmbLinearizerQuality->currentIndex()).toInt();

    // contours
    Agros2D::problem()->configView()->contoursCount = txtContoursCount->value();
    Agros2D::problem()->configView()->contourWidth = txtContourWidth->value();

    // vector field
    Agros2D::problem()->configView()->vectorProportional = chkVectorProportional->isChecked();
    Agros2D::problem()->configView()->vectorColor = chkVectorColor->isChecked();
    Agros2D::problem()->configView()->vectorCount = txtVectorCount->value();
    Agros2D::problem()->configView()->vectorScale = txtVectorScale->value();
    Agros2D::problem()->configView()->vectorType = (VectorType) cmbVectorType->itemData(cmbVectorType->currentIndex()).toInt();
    Agros2D::problem()->configView()->vectorCenter = (VectorCenter) cmbVectorCenter->itemData(cmbVectorCenter->currentIndex()).toInt();

    // order view
    Agros2D::problem()->configView()->showOrderColorBar = chkShowOrderColorbar->isChecked();
    Agros2D::problem()->configView()->orderPaletteOrderType = (PaletteOrderType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt();
    Agros2D::problem()->configView()->orderLabel = chkOrderLabel->isChecked();

    // particle tracing
    Agros2D::problem()->configView()->particleIncludeRelativisticCorrection = chkParticleIncludeRelativisticCorrection->isChecked();
    Agros2D::problem()->configView()->particleNumberOfParticles = txtParticleNumberOfParticles->value();
    Agros2D::problem()->configView()->particleStartingRadius = txtParticleStartingRadius->value();
    Agros2D::problem()->configView()->particleMass = txtParticleMass->value();
    Agros2D::problem()->configView()->particleConstant = txtParticleConstant->value();
    Agros2D::problem()->configView()->particleStart.x = txtParticlePointX->value();
    Agros2D::problem()->configView()->particleStart.y = txtParticlePointY->value();
    Agros2D::problem()->configView()->particleStartVelocity.x = txtParticleVelocityX->value();
    Agros2D::problem()->configView()->particleStartVelocity.y = txtParticleVelocityY->value();
    Agros2D::problem()->configView()->particleReflectOnDifferentMaterial = chkParticleReflectOnDifferentMaterial->isChecked();
    Agros2D::problem()->configView()->particleReflectOnBoundary = chkParticleReflectOnBoundary->isChecked();
    Agros2D::problem()->configView()->particleCoefficientOfRestitution = txtParticleCoefficientOfRestitution->value();
    Agros2D::problem()->configView()->particleCustomForce.x = txtParticleCustomForceX->value();
    Agros2D::problem()->configView()->particleCustomForce.y = txtParticleCustomForceY->value();
    Agros2D::problem()->configView()->particleCustomForce.z = txtParticleCustomForceZ->value();
    Agros2D::problem()->configView()->particleMaximumRelativeError = txtParticleMaximumRelativeError->value();
    Agros2D::problem()->configView()->particleMinimumStep = txtParticleMinimumStep->value();
    Agros2D::problem()->configView()->particleMaximumNumberOfSteps = txtParticleMaximumNumberOfSteps->value();
    Agros2D::problem()->configView()->particleColorByVelocity = chkParticleColorByVelocity->isChecked();
    Agros2D::problem()->configView()->particleShowPoints = chkParticleShowPoints->isChecked();
    Agros2D::problem()->configView()->particleDragDensity = txtParticleDragDensity->value();
    Agros2D::problem()->configView()->particleDragCoefficient = txtParticleDragCoefficient->value();
    Agros2D::problem()->configView()->particleDragReferenceArea = txtParticleDragReferenceArea->value();

    // scalar view
    Agros2D::problem()->configView()->scalarRangeLog = chkScalarFieldRangeLog->isChecked();
    Agros2D::problem()->configView()->scalarRangeBase = txtScalarFieldRangeBase->text().toDouble();
    Agros2D::problem()->configView()->scalarDecimalPlace = txtScalarDecimalPlace->value();

    // solid
    Agros2D::problem()->configView()->solidViewHide.clear();
    for (int i = 0; i < lstSolidMaterials->count(); i++)
    {
        if (lstSolidMaterials->item(i)->checkState() == Qt::Unchecked)
        {
            SceneMaterial *material = lstSolidMaterials->item(i)->data(Qt::UserRole).value<SceneMaterial *>();
            Agros2D::problem()->configView()->solidViewHide.append(material->name());
        }
    }
}

void PostprocessorWidget::createControls()
{
    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    m_chartWidget = new ChartControlsWidget(m_scenePost2D, m_sceneChart->chart(), this);

    basic = controlsBasic();
    advanced = controlsAdvanced();

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 5, 3, 5);
    layoutMain->addWidget(basic);
    layoutMain->addWidget(advanced, 1);
    layoutMain->addWidget(btnOK, 0, Qt::AlignRight);

    refresh();

    groupPostScalarAdvanced->setVisible(false);
    groupPostContourAdvanced->setVisible(false);
    groupPostVectorAdvanced->setVisible(false);
    groupPostParticalTracingAdvanced->setVisible(false);
    groupPostSolidAdvanced->setVisible(false);
    groupPostChart->setVisible(false);

    setLayout(layoutMain);
}

QWidget *PostprocessorWidget::meshWidget()
{
    // layout mesh
    chkShowInitialMeshView = new QCheckBox(tr("Initial mesh"));
    chkShowSolutionMeshView = new QCheckBox(tr("Solution mesh"));
    chkShowOrderView = new QCheckBox(tr("Polynomial order"));
    connect(chkShowOrderView, SIGNAL(clicked()), this, SLOT(refresh()));

    QVBoxLayout *gridLayoutMesh = new QVBoxLayout();
    gridLayoutMesh->addWidget(chkShowInitialMeshView);
    gridLayoutMesh->addWidget(chkShowSolutionMeshView);
    gridLayoutMesh->addWidget(chkShowOrderView);

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
    cmbOrderPaletteOrder->addItem(tr("Copper"), PaletteOrder_Copper);
    cmbOrderPaletteOrder->addItem(tr("Hot"), PaletteOrder_Hot);
    cmbOrderPaletteOrder->addItem(tr("Cool"), PaletteOrder_Cool);
    cmbOrderPaletteOrder->addItem(tr("Bone"), PaletteOrder_Bone);
    cmbOrderPaletteOrder->addItem(tr("Pink"), PaletteOrder_Pink);
    cmbOrderPaletteOrder->addItem(tr("Spring"), PaletteOrder_Spring);
    cmbOrderPaletteOrder->addItem(tr("Summer"), PaletteOrder_Summer);
    cmbOrderPaletteOrder->addItem(tr("Autumn"), PaletteOrder_Autumn);
    cmbOrderPaletteOrder->addItem(tr("Winter"), PaletteOrder_Winter);
    cmbOrderPaletteOrder->addItem(tr("HSV"), PaletteOrder_HSV);
    cmbOrderPaletteOrder->addItem(tr("B/W ascending"), PaletteOrder_BWAsc);
    cmbOrderPaletteOrder->addItem(tr("B/W descending"), PaletteOrder_BWDesc);

    chkShowOrderColorbar = new QCheckBox(tr("Show order colorbar"), this);
    chkOrderLabel = new QCheckBox(tr("Show order labels"), this);

    QGridLayout *gridLayoutOrder = new QGridLayout();
    gridLayoutOrder->addWidget(new QLabel(tr("Order palette:")), 0, 0);
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
    chkShowPost2DParticleView = new QCheckBox(tr("Particle tracing"));
    connect(chkShowPost2DParticleView, SIGNAL(clicked()), this, SLOT(refresh()));

    QGridLayout *layoutPost2D = new QGridLayout();
    layoutPost2D->addWidget(chkShowPost2DScalarView, 0, 0);
    layoutPost2D->addWidget(chkShowPost2DContourView, 1, 0);
    layoutPost2D->addWidget(chkShowPost2DVectorView, 2, 0);
    layoutPost2D->addWidget(chkShowPost2DParticleView, 0, 1);

    QHBoxLayout *layoutShowPost2D = new QHBoxLayout();
    layoutShowPost2D->addLayout(layoutPost2D);
    layoutShowPost2D->addStretch(1);

    QGroupBox *grpShowPost2D = new QGroupBox(tr("Postprocessor 2D"));
    grpShowPost2D->setLayout(layoutShowPost2D);

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
    layoutScalarField->setColumnMinimumWidth(0, minWidth);
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
    layoutContourField->setColumnMinimumWidth(0, minWidth);
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
    layoutVectorField->setColumnMinimumWidth(0, minWidth);
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

CollapsableGroupBoxButton *PostprocessorWidget::postParticalTracingWidget()
{
    // particle tracing
    groupPostParticalTracingAdvanced = postParticalTracingAdvancedWidget();

    QVBoxLayout *layoutParticleTracing = new QVBoxLayout();
    layoutParticleTracing->addWidget(groupPostParticalTracingAdvanced);

    CollapsableGroupBoxButton *grpParticalTracing = new CollapsableGroupBoxButton(tr("Partical tracing"));
    connect(grpParticalTracing, SIGNAL(collapseEvent(bool)), this, SLOT(doParticleFieldExpandCollapse(bool)));
    grpParticalTracing->setCollapsed(true);
    grpParticalTracing->setLayout(layoutParticleTracing);

    return grpParticalTracing;
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

QWidget *PostprocessorWidget::post3DWidget()
{
    // layout post3d
    radPost3DNone = new QRadioButton(tr("None"), this);
    radPost3DScalarField3D = new QRadioButton(tr("Scalar view"), this);
    radPost3DScalarField3DSolid = new QRadioButton(tr("Scalar view solid"), this);
    radPost3DParticleTracing = new QRadioButton(tr("Particle tracing"), this);
    radPost3DModel = new QRadioButton("Model", this);

    butPost3DGroup = new QButtonGroup(this);
    butPost3DGroup->addButton(radPost3DNone);
    butPost3DGroup->addButton(radPost3DScalarField3D);
    butPost3DGroup->addButton(radPost3DScalarField3DSolid);
    butPost3DGroup->addButton(radPost3DParticleTracing);
    butPost3DGroup->addButton(radPost3DModel);
    connect(butPost3DGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(doPostprocessorGroupClicked(QAbstractButton*)));

    QGridLayout *layoutPost3D = new QGridLayout();
    layoutPost3D->addWidget(radPost3DNone, 0, 0);
    layoutPost3D->addWidget(radPost3DScalarField3D, 1, 0);
    layoutPost3D->addWidget(radPost3DScalarField3DSolid, 2, 0);
    layoutPost3D->addWidget(radPost3DParticleTracing, 1, 1);
    layoutPost3D->addWidget(radPost3DModel, 2, 1);

    QHBoxLayout *layoutShowPost3D = new QHBoxLayout();
    layoutShowPost3D->addLayout(layoutPost3D);
    layoutShowPost3D->addStretch(1);

    QGroupBox *grpShowPost3D = new QGroupBox(tr("Postprocessor 3D"));
    grpShowPost3D->setLayout(layoutShowPost3D);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(grpShowPost3D);
    layout->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorWidget::chartWidget()
{
    QVBoxLayout *layoutVariables = new QVBoxLayout();
    layoutVariables->addWidget(m_chartWidget->variablesWidget());

    QGroupBox *grpChart = new QGroupBox(tr("Chart"));
    grpChart->setLayout(layoutVariables);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(grpChart);
    layout->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorWidget::controlsBasic()
{
    cmbFieldInfo = new QComboBox();
    connect(cmbFieldInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldInfo(int)));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(doCalculationFinished()));

    QGridLayout *layoutField = new QGridLayout();
    layoutField->setColumnMinimumWidth(0, minWidth);
    layoutField->setColumnStretch(1, 1);
    layoutField->addWidget(new QLabel(tr("Physical field:")), 0, 0);
    layoutField->addWidget(cmbFieldInfo, 0, 1);

    QGroupBox *grpField = new QGroupBox(tr("Physical field"));
    grpField->setLayout(layoutField);

    // transient
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStep(int)));

    QGridLayout *layoutTransient = new QGridLayout();
    layoutTransient->setColumnMinimumWidth(0, minWidth);
    layoutTransient->setColumnStretch(1, 1);
    layoutTransient->addWidget(new QLabel(tr("Time step:")), 0, 0);
    layoutTransient->addWidget(cmbTimeStep, 0, 1);

    grpTransient = new QGroupBox(tr("Transient analysis"));
    grpTransient->setLayout(layoutTransient);

    // adaptivity
    cmbAdaptivityStep = new QComboBox(this);
    connect(cmbAdaptivityStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStep(int)));
    cmbAdaptivitySolutionType = new QComboBox(this);

    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, minWidth);
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Step:")), 0, 0);
    layoutAdaptivity->addWidget(cmbAdaptivityStep, 0, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Type:")), 1, 0);
    layoutAdaptivity->addWidget(cmbAdaptivitySolutionType, 1, 1);

    grpAdaptivity = new QGroupBox(tr("Adaptivity"));
    grpAdaptivity->setLayout(layoutAdaptivity);

    groupMesh = meshWidget();
    groupMeshOrder = meshOrderWidget();
    groupPost2d = post2DWidget();
    groupPost3d = post3DWidget();
    groupChart = chartWidget();

    widgetsLayout = new QStackedLayout();
    widgetsLayout->addWidget(groupMesh);
    widgetsLayout->addWidget(groupPost2d);
    widgetsLayout->addWidget(groupPost3d);
    widgetsLayout->addWidget(groupChart);

    QVBoxLayout *layoutBasic = new QVBoxLayout();
    layoutBasic->addWidget(grpField);
    layoutBasic->addWidget(grpTransient);
    layoutBasic->addWidget(grpAdaptivity);
    layoutBasic->addLayout(widgetsLayout);
    layoutBasic->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutBasic);

    return widget;
}

QWidget *PostprocessorWidget::controlsAdvanced()
{
    groupPostScalar = postScalarWidget();
    groupPostContour = postContourWidget();
    groupPostVector = postVectorWidget();
    groupPostParticalTracing = postParticalTracingWidget();
    groupPostSolid = postSolidWidget();
    groupPostChart = postChartWidget();

    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->addWidget(groupMeshOrder);
    layoutArea->addWidget(groupPostScalar);
    layoutArea->addWidget(groupPostContour);
    layoutArea->addWidget(groupPostVector);
    layoutArea->addWidget(groupPostParticalTracing);
    layoutArea->addWidget(groupPostSolid);
    layoutArea->addWidget(groupPostChart);
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

    // quality
    cmbLinearizerQuality = new QComboBox();
    foreach (QString key, paletteQualityStringKeys())
        cmbLinearizerQuality->addItem(paletteQualityString(paletteQualityFromStringKey(key)), paletteQualityFromStringKey(key));

    chkPaletteFilter = new QCheckBox(tr("Filter"));
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    // steps
    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(PALETTESTEPSMIN);
    txtPaletteSteps->setMaximum(PALETTESTEPSMAX);

    // log scale
    chkScalarFieldRangeLog = new QCheckBox(tr("Log. scale"));
    txtScalarFieldRangeBase = new LineEditDouble(SCALARFIELDRANGEBASE);
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    txtScalarFieldRangeMin = new LineEditDouble(0.1, true);
    connect(txtScalarFieldRangeMin, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMinChanged()));
    lblScalarFieldRangeMinError = new QLabel("");
    lblScalarFieldRangeMinError->setPalette(palette);
    lblScalarFieldRangeMinError->setVisible(false);
    txtScalarFieldRangeMax = new LineEditDouble(0.1, true);
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
    gridLayoutScalarFieldPalette->setColumnMinimumWidth(0, minWidth);
    gridLayoutScalarFieldPalette->setColumnStretch(1, 1);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutScalarFieldPalette->addWidget(cmbPalette, 0, 1, 1, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Quality:")), 1, 0);
    gridLayoutScalarFieldPalette->addWidget(cmbLinearizerQuality, 1, 1, 1, 2);
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
    gridLayoutScalarFieldColorbar->addWidget(new QLabel(tr("Decimal places:")), 0, 0);
    gridLayoutScalarFieldColorbar->addWidget(txtScalarDecimalPlace, 0, 1);
    gridLayoutScalarFieldColorbar->addWidget(chkShowScalarColorBar, 0, 2);

    QGroupBox *grpScalarFieldColorbar = new QGroupBox(tr("Colorbar"));
    grpScalarFieldColorbar->setLayout(gridLayoutScalarFieldColorbar);

    QVBoxLayout *layoutScalarFieldAdvanced = new QVBoxLayout();
    layoutScalarFieldAdvanced->setMargin(0);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldPalette);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldColorbar);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldRange);

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

    QGridLayout *gridLayoutContours = new QGridLayout();
    gridLayoutContours->setMargin(0);
    gridLayoutContours->setColumnMinimumWidth(0, minWidth);
    gridLayoutContours->setColumnStretch(1, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Number of contours:")), 0, 0);
    gridLayoutContours->addWidget(txtContoursCount, 0, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Contour width:")), 1, 0);
    gridLayoutContours->addWidget(txtContourWidth, 1, 1);

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

    QGridLayout *gridLayoutVectors = new QGridLayout();
    gridLayoutVectors->setMargin(0);
    gridLayoutVectors->setColumnMinimumWidth(0, minWidth);
    gridLayoutVectors->setColumnStretch(1, 1);
    gridLayoutVectors->addWidget(new QLabel(tr("Number of vectors:")), 0, 0);
    gridLayoutVectors->addWidget(txtVectorCount, 0, 1);
    gridLayoutVectors->addWidget(chkVectorProportional, 0, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Scale:")), 1, 0);
    gridLayoutVectors->addWidget(txtVectorScale, 1, 1);
    gridLayoutVectors->addWidget(chkVectorColor, 1, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Type:")), 2, 0);
    gridLayoutVectors->addWidget(cmbVectorType, 2, 1, 1, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Center:")), 3, 0);
    gridLayoutVectors->addWidget(cmbVectorCenter, 3, 1, 1, 2);

    QWidget *vectorWidget = new QWidget();
    vectorWidget->setLayout(gridLayoutVectors);

    return vectorWidget;
}

QWidget *PostprocessorWidget::postPostSolidAdvancedWidget()
{
    lstSolidMaterials = new QListWidget();

    QGridLayout *gridLayoutSolid = new QGridLayout();
    gridLayoutSolid->setMargin(0);
    gridLayoutSolid->setColumnMinimumWidth(0, minWidth);
    gridLayoutSolid->setColumnStretch(0, 1);
    gridLayoutSolid->addWidget(lstSolidMaterials, 0, 0);

    QWidget *solidWidget = new QWidget();
    solidWidget->setLayout(gridLayoutSolid);

    return solidWidget;
}

QWidget *PostprocessorWidget::postChartWidget()
{
    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(m_chartWidget);
    layout->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorWidget::postParticalTracingAdvancedWidget()
{
    // particle tracing
    chkParticleIncludeRelativisticCorrection = new QCheckBox(tr("Relativistic correction"));
    txtParticleNumberOfParticles = new QSpinBox(this);
    txtParticleNumberOfParticles->setMinimum(1);
    txtParticleNumberOfParticles->setMaximum(200);
    txtParticleStartingRadius = new LineEditDouble();
    txtParticleMass = new LineEditDouble();
    txtParticleConstant = new LineEditDouble();
    lblParticlePointX = new QLabel();
    lblParticlePointY = new QLabel();
    txtParticlePointX = new LineEditDouble();
    txtParticlePointY = new LineEditDouble();
    lblParticleVelocityX = new QLabel();
    lblParticleVelocityY = new QLabel();
    txtParticleVelocityX = new LineEditDouble();
    txtParticleVelocityY = new LineEditDouble();
    lblParticleCustomForceX = new QLabel();
    lblParticleCustomForceY = new QLabel();
    lblParticleCustomForceZ = new QLabel();
    txtParticleCustomForceX = new LineEditDouble();
    txtParticleCustomForceY = new LineEditDouble();
    txtParticleCustomForceZ = new LineEditDouble();
    txtParticleMaximumRelativeError = new LineEditDouble();
    txtParticleMinimumStep = new LineEditDouble();
    chkParticleReflectOnDifferentMaterial = new QCheckBox(tr("Reflection on different material"));
    chkParticleReflectOnBoundary = new QCheckBox(tr("Reflection on boundary"));
    txtParticleCoefficientOfRestitution = new LineEditDouble(0.0, true);
    txtParticleCoefficientOfRestitution->setBottom(0.0);
    txtParticleCoefficientOfRestitution->setTop(1.0);
    chkParticleColorByVelocity = new QCheckBox(tr("Line color is controlled by velocity"));
    chkParticleShowPoints = new QCheckBox(tr("Show points"));
    txtParticleMaximumNumberOfSteps = new QSpinBox();
    txtParticleMaximumNumberOfSteps->setMinimum(10);
    txtParticleMaximumNumberOfSteps->setMaximum(100000);
    txtParticleMaximumNumberOfSteps->setSingleStep(10);
    txtParticleDragDensity = new LineEditDouble();
    txtParticleDragCoefficient = new LineEditDouble();
    txtParticleDragReferenceArea = new LineEditDouble();
    lblParticleMotionEquations = new QLabel();

    // QPushButton *btnParticleDefault = new QPushButton(tr("Default"));
    // connect(btnParticleDefault, SIGNAL(clicked()), this, SLOT(doParticleDefault()));

    // Lorentz force
    QGridLayout *gridLayoutLorentzForce = new QGridLayout();
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Equation:")), 0, 0);
    gridLayoutLorentzForce->addWidget(new QLabel(QString("<i><b>F</b></i><sub>L</sub> = <i>Q</i> (<i><b>E</b></i> + <i><b>v</b></i> x <i><b>B</b></i>)")), 0, 1);
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Charge (C):")), 1, 0);
    gridLayoutLorentzForce->addWidget(txtParticleConstant, 1, 1);

    QGroupBox *grpLorentzForce = new QGroupBox(tr("Lorentz force"));
    grpLorentzForce->setLayout(gridLayoutLorentzForce);

    // drag force
    QGridLayout *gridLayoutDragForce = new QGridLayout();
    gridLayoutDragForce->setColumnMinimumWidth(0, minWidth);
    gridLayoutDragForce->setColumnStretch(1, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Equation:")), 0, 0);
    gridLayoutDragForce->addWidget(new QLabel(QString("<i><b>F</b></i><sub>D</sub> = - &frac12; <i>&rho;</i> <i>v</i><sup>2</sup> <i>C</i><sub>D</sub> <i>S</i> &sdot; <i><b>v</b></i><sub>0</sub>")), 0, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Density (kg/m<sup>3</sup>):")), 1, 0);
    gridLayoutDragForce->addWidget(txtParticleDragDensity, 1, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Reference area (m<sup>2</sup>):")), 2, 0);
    gridLayoutDragForce->addWidget(txtParticleDragReferenceArea, 2, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Coefficient (-):")), 3, 0);
    gridLayoutDragForce->addWidget(txtParticleDragCoefficient, 3, 1);

    QGroupBox *grpDragForce = new QGroupBox(tr("Drag force"));
    grpDragForce->setLayout(gridLayoutDragForce);

    // reflection
    QGridLayout *gridLayoutReflection = new QGridLayout();
    gridLayoutReflection->setColumnMinimumWidth(0, minWidth);
    gridLayoutReflection->setColumnStretch(1, 1);
    gridLayoutReflection->addWidget(chkParticleReflectOnDifferentMaterial, 0, 0, 1, 2);
    gridLayoutReflection->addWidget(chkParticleReflectOnBoundary, 1, 0, 1, 2);
    gridLayoutReflection->addWidget(new QLabel(tr("Coefficient of restitution (-):")), 2, 0);
    gridLayoutReflection->addWidget(txtParticleCoefficientOfRestitution, 2, 1);

    QGroupBox *grpReflection = new QGroupBox(tr("Reflection"));
    grpReflection->setLayout(gridLayoutReflection);

    // initial particle velocity
    QGridLayout *gridCustomForce = new QGridLayout();
    gridCustomForce->addWidget(lblParticleCustomForceX, 0, 0);
    gridCustomForce->addWidget(txtParticleCustomForceX, 0, 1);
    gridCustomForce->addWidget(lblParticleCustomForceY, 1, 0);
    gridCustomForce->addWidget(txtParticleCustomForceY, 1, 1);
    gridCustomForce->addWidget(lblParticleCustomForceZ, 2, 0);
    gridCustomForce->addWidget(txtParticleCustomForceZ, 2, 1);

    QGroupBox *grpCustomForce = new QGroupBox(tr("Custom force"));
    grpCustomForce->setLayout(gridCustomForce);

    // initial particle position
    QGridLayout *gridLayoutInitialPosition = new QGridLayout();
    gridLayoutInitialPosition->addWidget(lblParticlePointX, 0, 0);
    gridLayoutInitialPosition->addWidget(txtParticlePointX, 0, 1);
    gridLayoutInitialPosition->addWidget(lblParticlePointY, 1, 0);
    gridLayoutInitialPosition->addWidget(txtParticlePointY, 1, 1);

    QGroupBox *grpInitialPosition = new QGroupBox(tr("Initial particle position"));
    grpInitialPosition->setLayout(gridLayoutInitialPosition);

    // initial particle velocity
    QGridLayout *gridLayoutInitialVelocity = new QGridLayout();
    gridLayoutInitialVelocity->addWidget(lblParticleVelocityX, 0, 0);
    gridLayoutInitialVelocity->addWidget(txtParticleVelocityX, 0, 1);
    gridLayoutInitialVelocity->addWidget(lblParticleVelocityY, 1, 0);
    gridLayoutInitialVelocity->addWidget(txtParticleVelocityY, 1, 1);

    QGroupBox *grpInitialVelocity = new QGroupBox(tr("Initial particle velocity"));
    grpInitialVelocity->setLayout(gridLayoutInitialVelocity);

    // solver
    QGridLayout *gridLayoutSolver = new QGridLayout();
    gridLayoutSolver->addWidget(chkParticleIncludeRelativisticCorrection, 0, 0);
    gridLayoutSolver->addWidget(new QLabel(QString("<i>m</i><sub>p</sub> = m / (1 - v<sup>2</sup>/c<sup>2</sup>)<sup>1/2</sup>")), 0, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Maximum relative error (%):")), 1, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumRelativeError, 1, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Minimum step (m):")), 2, 0);
    gridLayoutSolver->addWidget(txtParticleMinimumStep, 2, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Maximum number of steps:")), 3, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumNumberOfSteps, 3, 1);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(gridLayoutSolver);

    // settings
    QGridLayout *gridLayoutSettings = new QGridLayout();
    gridLayoutSettings->addWidget(chkParticleColorByVelocity, 2, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowPoints, 3, 0, 1, 2);

    QGroupBox *grpSettings = new QGroupBox(tr("Settings"));
    grpSettings->setLayout(gridLayoutSettings);

    QGridLayout *layoutParticle = new QGridLayout();
    layoutParticle->setMargin(0);
    layoutParticle->addWidget(new QLabel(tr("Equations:")), 0, 0);
    layoutParticle->addWidget(lblParticleMotionEquations, 1, 0, 1, 2);
    layoutParticle->addWidget(new QLabel(tr("Number of particles:")), 2, 0);
    layoutParticle->addWidget(txtParticleNumberOfParticles, 2, 1);
    layoutParticle->addWidget(new QLabel(tr("Particles dispersion (m):")), 3, 0);
    layoutParticle->addWidget(txtParticleStartingRadius, 3, 1);
    layoutParticle->addWidget(new QLabel(tr("Mass (kg):")), 4, 0);
    layoutParticle->addWidget(txtParticleMass, 4, 1);
    layoutParticle->addWidget(grpInitialPosition, 5, 0, 1, 2);
    layoutParticle->addWidget(grpInitialVelocity, 6, 0, 1, 2);
    layoutParticle->addWidget(grpLorentzForce, 7, 0, 1, 2);
    layoutParticle->addWidget(grpDragForce, 8, 0, 1, 2);
    layoutParticle->addWidget(grpCustomForce, 9, 0, 1, 2);
    layoutParticle->addWidget(grpReflection, 10, 0, 1, 2);
    layoutParticle->addWidget(grpSolver, 11, 0, 1, 2);
    layoutParticle->addWidget(grpSettings, 12, 0, 1, 2);

    QWidget *particleWidget = new QWidget();
    particleWidget->setLayout(layoutParticle);

    return particleWidget;
}

void PostprocessorWidget::doCalculationFinished()
{
    QString activeFieldName = Agros2D::scene()->activeViewField()->fieldId();
    for (int index = 0; index < cmbFieldInfo->count(); index++)
    {
        if (cmbFieldInfo->itemData(index).toString() == activeFieldName)
        {
            cmbFieldInfo->setCurrentIndex(index);
            doFieldInfo(index);
            break;
        }
    }
}

void PostprocessorWidget::doFieldInfo(int index)
{
    QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
    if (Agros2D::problem()->hasField(fieldName))
    {
        FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(fieldName);
        Agros2D::scene()->setActiveViewField(fieldInfo);

        fillComboBoxScalarVariable(fieldInfo, cmbPostScalarFieldVariable);
        fillComboBoxContourVariable(fieldInfo, cmbPost2DContourVariable);
        fillComboBoxVectorVariable(fieldInfo, cmbPost2DVectorFieldVariable);
        fillComboBoxTimeStep(fieldInfo, cmbTimeStep);
        doTimeStep(0);

        doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());

        int currentStep = Agros2D::solutionStore()->nearestTimeStep(Agros2D::scene()->activeViewField(), Agros2D::scene()->activeTimeStep());
        double currentTime = Agros2D::problem()->timeStepToTotalTime(currentStep);
        int stepIndex = Agros2D::solutionStore()->timeLevelIndex(selectedField(), currentTime);
        cmbTimeStep->setCurrentIndex(stepIndex);
        cmbAdaptivityStep->setCurrentIndex(Agros2D::scene()->activeAdaptivityStep());
        // qDebug() << "timestep set to " << currentStep << ", adapt " << Agros2D::scene()->activeAdaptivityStep() << "\n";
    }
}

void PostprocessorWidget::doTimeStep(int index)
{
    fillComboBoxAdaptivityStep(selectedField(), selectedTimeStep(), cmbAdaptivityStep);
    if ((cmbAdaptivityStep->currentIndex() >= cmbAdaptivityStep->count()) || (cmbAdaptivityStep->currentIndex() < 0))
    {
        cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->count() - 1);
    }
    grpAdaptivity->setVisible(cmbAdaptivityStep->count() > 1);
    cmbAdaptivityStep->setEnabled(cmbAdaptivityStep->count() > 1);
    cmbAdaptivitySolutionType->setEnabled(cmbAdaptivityStep->count() > 1);
    doAdaptivityStep(0);
}

void PostprocessorWidget::doAdaptivityStep(int index)
{
    fillComboBoxSolutionType(selectedField(), selectedTimeStep(), selectedAdaptivityStep(), cmbAdaptivitySolutionType);
    if ((cmbAdaptivitySolutionType->currentIndex() >= cmbAdaptivitySolutionType->count()) || (cmbAdaptivitySolutionType->currentIndex() < 0))
    {
        cmbAdaptivitySolutionType->setCurrentIndex(0);
    }
}

void PostprocessorWidget::doScalarFieldVariable(int index)
{
    if (cmbFieldInfo->currentIndex() == -1)
        return;

    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    if (cmbPostScalarFieldVariable->currentIndex() != -1)
    {
        QString variableName(cmbPostScalarFieldVariable->itemData(index).toString());

        QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
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
    if (m_sceneMesh->actSceneModeMesh->isChecked())
    {
        widgetsLayout->setCurrentWidget(groupMesh);

        // mesh and order
        chkShowInitialMeshView->setEnabled(Agros2D::problem()->isMeshed());
        chkShowSolutionMeshView->setEnabled(Agros2D::problem()->isSolved());
        chkShowOrderView->setEnabled(Agros2D::problem()->isSolved());

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
        // particle tracing
        groupPostParticalTracing->setVisible(false);
        groupPostParticalTracingAdvanced->setVisible(false);
        // solid
        groupPostSolid->setVisible(false);
        groupPostSolidAdvanced->setVisible(false);
        // chart
        groupChart->setVisible(false);
        groupPostChart->setVisible(false);
    }

    if (m_scenePost2D->actSceneModePost2D->isChecked())
    {
        widgetsLayout->setCurrentWidget(groupPost2d);

        chkShowPost2DContourView->setEnabled(Agros2D::problem()->isSolved() && (cmbPost2DContourVariable->count() > 0));
        chkShowPost2DScalarView->setEnabled(Agros2D::problem()->isSolved() && (cmbPostScalarFieldVariable->count() > 0));
        chkShowPost2DVectorView->setEnabled(Agros2D::problem()->isSolved() && (cmbPost2DVectorFieldVariable->count() > 0));
        chkShowPost2DParticleView->setEnabled(Agros2D::problem()->isSolved());

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

        // partical tracing
        groupPostParticalTracing->setVisible(chkShowPost2DParticleView->isEnabled() && chkShowPost2DParticleView->isChecked());
        groupPostParticalTracingAdvanced->setVisible(chkShowPost2DParticleView->isEnabled() && chkShowPost2DParticleView->isChecked() && !groupPostParticalTracing->isCollapsed());

        // solid
        groupPostSolid->setVisible(false);
        groupPostSolidAdvanced->setVisible(false);

        // chart
        groupChart->setVisible(false);
        groupPostChart->setVisible(false);
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
        radPost3DParticleTracing->setEnabled(Agros2D::problem()->isSolved());

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

        // partical tracing
        groupPostParticalTracing->setVisible(radPost3DParticleTracing->isEnabled() && radPost3DParticleTracing->isChecked());
        groupPostParticalTracingAdvanced->setVisible(radPost3DParticleTracing->isEnabled() && radPost3DParticleTracing->isChecked() && !groupPostParticalTracing->isCollapsed());

        // solid
        groupPostSolid->setVisible((radPost3DScalarField3DSolid->isEnabled() && radPost3DScalarField3DSolid->isChecked())
                                   || (radPost3DModel->isEnabled() && radPost3DModel->isChecked()));
        groupPostSolidAdvanced->setVisible(((radPost3DScalarField3DSolid->isEnabled() && radPost3DScalarField3DSolid->isChecked())
                                            || (radPost3DModel->isEnabled() && radPost3DModel->isChecked()))
                                           && !groupPostSolid->isCollapsed());

        // chart
        groupChart->setVisible(false);
        groupPostChart->setVisible(false);
    }
    if (m_sceneChart->actSceneModeChart->isChecked())
    {
        widgetsLayout->setCurrentWidget(groupChart);

        // mesh
        groupMeshOrder->setVisible(false);
        // scalar
        groupPostScalar->setVisible(false);
        groupPostScalarAdvanced->setVisible(false);
        // contour
        groupPostContour->setVisible(false);
        groupPostContourAdvanced->setVisible(false);
        // vector
        groupPostVector->setVisible(false);
        groupPostVectorAdvanced->setVisible(false);
        // particle tracing
        groupPostParticalTracing->setVisible(false);
        groupPostParticalTracingAdvanced->setVisible(false);
        // solid
        groupPostSolid->setVisible(false);
        groupPostSolidAdvanced->setVisible(false);
        // chart
        groupChart->setVisible(true);
        groupPostChart->setVisible(true);
    }

    // scalar view
    if (groupPostScalar->isVisible())
    {
        doScalarFieldRangeAuto(-1);
    }

    grpTransient->setVisible(false);
    grpAdaptivity->setVisible(false);
    if (Agros2D::problem()->isSolved())
    {
        // transient group
        int timeSteps = Agros2D::solutionStore()->timeLevels(Agros2D::scene()->activeViewField()).count();
        grpTransient->setVisible(timeSteps > 1);
        cmbTimeStep->setEnabled(timeSteps > 1);

        // adaptivity group
        int lastStep = Agros2D::solutionStore()->lastAdaptiveStep(Agros2D::scene()->activeViewField(), SolutionMode_Normal, Agros2D::scene()->activeTimeStep());
        grpAdaptivity->setVisible(lastStep > 0);
        cmbAdaptivityStep->setEnabled(lastStep > 0);
        cmbAdaptivitySolutionType->setEnabled(lastStep > 0);
    }
}

void PostprocessorWidget::updateControls()
{
    fillComboBoxFieldInfo(cmbFieldInfo);
    doFieldInfo(cmbFieldInfo->currentIndex());

    loadBasic();
    loadAdvanced();
}

void PostprocessorWidget::doPostprocessorGroupClicked(QAbstractButton *button)
{
    refresh();
}

int PostprocessorWidget::selectedTimeStep()
{
    if (cmbTimeStep->currentIndex() == -1)
        return 0;
    else
    {
        int selectedTimeStep = cmbTimeStep->itemData(cmbTimeStep->currentIndex()).toInt();

        // todo: this is here to avoid fail after loading new model
        // todo: cmbTimeStep should probably be cleared somewhere
        // todo: or the PostprocessorWidget should be destroyed and created a new one?
        if(Agros2D::solutionStore()->timeLevels(selectedField()).size() <= selectedTimeStep)
            return 0;

        // due to timeskipping
        double realTime = Agros2D::solutionStore()->timeLevels(selectedField()).at(selectedTimeStep);
        int realTimeStep = Agros2D::problem()->timeToTimeStep(realTime);
        return realTimeStep;
    }
}

int PostprocessorWidget::selectedAdaptivityStep()
{
    if (cmbAdaptivityStep->currentIndex() == -1)
        return 0;
    else
        return cmbAdaptivityStep->itemData(cmbAdaptivityStep->currentIndex()).toInt();
}

FieldInfo* PostprocessorWidget::selectedField()
{
    return Agros2D::problem()->fieldInfo(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString());
}

void PostprocessorWidget::doApply()
{
    saveBasic();
    saveAdvanced();

    // time step
    QApplication::processEvents();

    int actualTimeStep = selectedTimeStep();
    qDebug() << "actualTimeStep : " << actualTimeStep;
    Agros2D::scene()->setActiveTimeStep(actualTimeStep);

    // todo: this should be revised
    if(this->selectedField()->adaptivityType() != AdaptivityType_None)
    {
        Agros2D::scene()->setActiveAdaptivityStep(cmbAdaptivityStep->currentIndex());
        Agros2D::scene()->setActiveSolutionType((SolutionMode)cmbAdaptivitySolutionType->currentIndex());
    }
    // read auto range values
    if (chkScalarFieldRangeAuto->isChecked())
    {
        txtScalarFieldRangeMin->setValue(Agros2D::problem()->configView()->scalarRangeMin);
        txtScalarFieldRangeMax->setValue(Agros2D::problem()->configView()->scalarRangeMax);
    }

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

void PostprocessorWidget::doParticleFieldExpandCollapse(bool collapsed)
{
    groupPostParticalTracingAdvanced->setVisible(!collapsed);
}

void PostprocessorWidget::doScalarFieldDefault()
{
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) PALETTETYPE));
    chkPaletteFilter->setChecked(PALETTEFILTER);
    txtPaletteSteps->setValue(PALETTESTEPS);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(PaletteQuality_Normal));
    chkShowScalarColorBar->setChecked(SHOWSCALARCOLORBAR);
    chkScalarFieldRangeLog->setChecked(SCALARFIELDRANGELOG);
    txtScalarFieldRangeBase->setValue(SCALARFIELDRANGEBASE);
    txtScalarDecimalPlace->setValue(SCALARDECIMALPLACE);
}

void PostprocessorWidget::doContoursVectorsDefault()
{
    txtContoursCount->setValue(CONTOURSCOUNT);
    chkVectorProportional->setChecked(VECTORPROPORTIONAL);
    chkVectorColor->setChecked(VECTORCOLOR);
    txtVectorCount->setValue(VECTORCOUNT);
    txtVectorScale->setValue(VECTORSCALE);
    cmbVectorType->setCurrentIndex(cmbVectorType->findData(VECTORTYPE));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData(VECTORCENTER));
}

void PostprocessorWidget::doOrderDefault()
{
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) ORDERPALETTEORDERTYPE));
    chkShowOrderColorbar->setChecked(SHOWORDERCOLORBAR);
    chkOrderLabel->setChecked(ORDERLABEL);
}

void PostprocessorWidget::doParticleDefault()
{
    txtParticleNumberOfParticles->setValue(PARTICLENUMBEROFPARTICLES);
    txtParticleStartingRadius->setValue(PARTICLESTARTINGRADIUS);
    chkParticleIncludeRelativisticCorrection->setChecked(PARTICLEINCLUDERELATIVISTICCORRECTION);
    txtParticleMass->setValue(PARTICLEMASS);
    txtParticleConstant->setValue(PARTICLECONSTANT);
    txtParticlePointX->setValue(PARTICLESTARTX);
    txtParticlePointY->setValue(PARTICLESTARTY);
    txtParticleVelocityX->setValue(PARTICLESTARTVELOCITYX);
    txtParticleVelocityY->setValue(PARTICLESTARTVELOCITYY);
    chkParticleReflectOnDifferentMaterial->setChecked(PARTICLEREFLECTONDIFFERENTMATERIAL);
    chkParticleReflectOnBoundary->setChecked(PARTICLEREFLECTONBOUNDARY);
    txtParticleCoefficientOfRestitution->setValue(PARTICLECOEFFICIENTOFRESTITUTION);
    txtParticleCustomForceX->setValue(PARTICLECUSTOMFORCEX);
    txtParticleCustomForceY->setValue(PARTICLECUSTOMFORCEY);
    txtParticleCustomForceZ->setValue(PARTICLECUSTOMFORCEZ);
    txtParticleMaximumRelativeError->setValue(PARTICLEMAXIMUMRELATIVEERROR);
    txtParticleMinimumStep->setValue(PARTICLEMINIMUMSTEP);
    txtParticleMaximumNumberOfSteps->setValue(PARTICLEMAXIMUMNUMBEROFSTEPS);
    chkParticleColorByVelocity->setChecked(PARTICLECOLORBYVELOCITY);
    chkParticleShowPoints->setChecked(PARTICLESHOWPOINTS);
    txtParticleDragDensity->setValue(PARTICLEDRAGDENSITY);
    txtParticleDragReferenceArea->setValue(PARTICLEDRAGREFERENCEAREA);
    txtParticleDragCoefficient->setValue(PARTICLEDRAGCOEFFICIENT);
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
