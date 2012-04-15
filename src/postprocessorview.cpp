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

#include "gui.h"

#include "scene.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

const double minWidth = 110;

PostprocessorWidget::PostprocessorWidget(SceneViewPreprocessor *sceneGeometry,
                                     SceneViewMesh *sceneMesh,
                                     SceneViewPost2D *scenePost2D,
                                     SceneViewPost3D *scenePost3D,
                                     QWidget *parent) : QWidget(parent)
{
    m_sceneGeometry = sceneGeometry;
    m_sceneMesh = sceneMesh;
    m_scenePost2D = scenePost2D;
    m_scenePost3D = scenePost3D;

    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorView");

    createControls();

    loadBasic();
    loadAdvanced();

    connect(this, SIGNAL(apply()), m_scenePost2D, SLOT(timeStepChanged()));
    connect(this, SIGNAL(apply()), m_scenePost3D, SLOT(timeStepChanged()));
}

void PostprocessorWidget::loadBasic()
{
    cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(Util::config()->activeField));
    if (cmbFieldInfo->currentIndex() == -1)
        cmbFieldInfo->setCurrentIndex(0);
    doFieldInfo(cmbFieldInfo->currentIndex());

    // show
    chkShowInitialMeshView->setChecked(Util::config()->showInitialMeshView);
    chkShowSolutionMeshView->setChecked(Util::config()->showSolutionMeshView);
    chkShowOrderView->setChecked(Util::config()->showOrderView);

    radPost3DNone->setChecked(Util::config()->showPost3D == SceneViewPost3DShow_None);
    radPost3DScalarField3D->setChecked(Util::config()->showPost3D == SceneViewPost3DShow_ScalarView3D);
    radPost3DScalarField3DSolid->setChecked(Util::config()->showPost3D == SceneViewPost3DShow_ScalarView3DSolid);
    radPost3DModel->setChecked(Util::config()->showPost3D == SceneViewPost3DShow_Model);
    doPostprocessorGroupClicked(butPost3DGroup->checkedButton());

    chkShowPost2DContourView->setChecked(Util::config()->showContourView);
    chkShowPost2DVectorView->setChecked(Util::config()->showVectorView);
    chkShowPost2DScalarView->setChecked(Util::config()->showScalarView);

    // contour field
    cmbPost2DContourVariable->setCurrentIndex(cmbPost2DContourVariable->findData(Util::config()->contourVariable));
    if (cmbPost2DContourVariable->currentIndex() == -1 && cmbPost2DContourVariable->count() > 0)
    {
        // set first variable
        cmbPost2DContourVariable->setCurrentIndex(0);
    }
    Util::config()->contourVariable = cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString();

    // scalar field
    cmbPostScalarFieldVariable->setCurrentIndex(cmbPostScalarFieldVariable->findData(Util::config()->scalarVariable));
    if (cmbPostScalarFieldVariable->currentIndex() == -1 && cmbPostScalarFieldVariable->count() > 0)
    {
        // set first variable
        cmbPostScalarFieldVariable->setCurrentIndex(0);
    }
    Util::config()->scalarVariable = cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString();
    doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());
    Util::config()->scalarVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    // vector field
    cmbPost2DVectorFieldVariable->setCurrentIndex(cmbPost2DVectorFieldVariable->findData(Util::config()->vectorVariable));
    if (cmbPost2DVectorFieldVariable->currentIndex() == -1 && cmbPost2DVectorFieldVariable->count() > 0)
    {
        // set first variable
        cmbPost2DVectorFieldVariable->setCurrentIndex(0);
    }
    Util::config()->vectorVariable = cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString();

    // transient view
    // cmbTimeStep->setCurrentIndex(Util::scene()->sceneSolution()->timeStep());

    setControls();
}

void PostprocessorWidget::loadAdvanced()
{
    // scalar field
    chkShowScalarColorBar->setChecked(Util::config()->showScalarColorBar);
    cmbPalette->setCurrentIndex(cmbPalette->findData(Util::config()->paletteType));
    chkPaletteFilter->setChecked(Util::config()->paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(Util::config()->paletteSteps);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(Util::config()->linearizerQuality));

    // contours
    txtContoursCount->setValue(Util::config()->contoursCount);

    // vector field
    chkVectorProportional->setChecked(Util::config()->vectorProportional);
    chkVectorColor->setChecked(Util::config()->vectorColor);
    txtVectorCount->setValue(Util::config()->vectorCount);
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(Util::config()->vectorScale);

    // order view
    chkShowOrderScale->setChecked(Util::config()->showOrderColorBar);
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData(Util::config()->orderPaletteOrderType));
    chkOrderLabel->setChecked(Util::config()->orderLabel);

    // particle tracing
    chkParticleIncludeGravitation->setChecked(Util::config()->particleIncludeGravitation);
    txtParticleNumberOfParticles->setValue(Util::config()->particleNumberOfParticles);
    txtParticleStartingRadius->setValue(Util::config()->particleStartingRadius);
    txtParticleMass->setValue(Util::config()->particleMass);
    txtParticleConstant->setValue(Util::config()->particleConstant);
    txtParticlePointX->setValue(Util::config()->particleStart.x);
    txtParticlePointY->setValue(Util::config()->particleStart.y);
    txtParticleVelocityX->setValue(Util::config()->particleStartVelocity.x);
    txtParticleVelocityY->setValue(Util::config()->particleStartVelocity.y);
    chkParticleTerminateOnDifferentMaterial->setChecked(Util::config()->particleTerminateOnDifferentMaterial);
    txtParticleMaximumRelativeError->setValue(Util::config()->particleMaximumRelativeError);
    txtParticleMaximumSteps->setValue(Util::config()->particleMaximumSteps);
    chkParticleColorByVelocity->setChecked(Util::config()->particleColorByVelocity);
    chkParticleShowPoints->setChecked(Util::config()->particleShowPoints);
    txtParticleDragDensity->setValue(Util::config()->particleDragDensity);
    txtParticleDragReferenceArea->setValue(Util::config()->particleDragReferenceArea);
    txtParticleDragCoefficient->setValue(Util::config()->particleDragCoefficient);

    lblParticlePointX->setText(QString("%1 (m):").arg(Util::scene()->problemInfo()->labelX()));
    lblParticlePointY->setText(QString("%1 (m):").arg(Util::scene()->problemInfo()->labelY()));
    lblParticleVelocityX->setText(QString("%1 (m):").arg(Util::scene()->problemInfo()->labelX()));
    lblParticleVelocityY->setText(QString("%1 (m):").arg(Util::scene()->problemInfo()->labelY()));

    if (Util::scene()->problemInfo()->coordinateType == CoordinateType_Planar)
        lblParticleMotionEquations->setText(QString("<i>x</i>\" = <i>F</i><sub>x</sub> / <i>m</i>, &nbsp; <i>y</i>\" = <i>F</i><sub>y</sub> / <i>m</i>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>"));
    else
        lblParticleMotionEquations->setText(QString("<i>r</i>\" = <i>F</i><sub>r</sub> / <i>m</i> + <i>r</i> (<i>&phi;</i>')<sup>2</sup>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>, &nbsp; <i>&phi;</i>\" = <i>F</i><sub>&phi;</sub> / <i>m</i> - 2<i>r</i> <i>r</i>' <i>&phi;</i>' / <i>r</i>"));

    // advanced
    // scalar field
    chkScalarFieldRangeLog->setChecked(Util::config()->scalarRangeLog);
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setText(QString::number(Util::config()->scalarRangeBase));
    txtScalarDecimalPlace->setValue(Util::config()->scalarDecimalPlace);
    chkScalarFieldRangeAuto->setChecked(Util::config()->scalarRangeAuto);
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setValue(Util::config()->scalarRangeMin);
    txtScalarFieldRangeMax->setValue(Util::config()->scalarRangeMax);
}

void PostprocessorWidget::saveBasic()
{
    // active field
    Util::config()->activeField = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();

    Util::config()->showInitialMeshView = chkShowInitialMeshView->isChecked();
    Util::config()->showSolutionMeshView = chkShowSolutionMeshView->isChecked();
    Util::config()->showOrderView = chkShowOrderView->isChecked();

    // show
    if (radPost3DNone->isChecked()) Util::config()->showPost3D = SceneViewPost3DShow_None;
    if (radPost3DScalarField3D->isChecked()) Util::config()->showPost3D = SceneViewPost3DShow_ScalarView3D;
    if (radPost3DScalarField3DSolid->isChecked()) Util::config()->showPost3D = SceneViewPost3DShow_ScalarView3DSolid;
    if (radPost3DModel->isChecked()) Util::config()->showPost3D = SceneViewPost3DShow_Model;

    Util::config()->showContourView = chkShowPost2DContourView->isChecked();
    Util::config()->showScalarView = chkShowPost2DScalarView->isChecked();
    Util::config()->showVectorView = chkShowPost2DVectorView->isChecked();

    // contour field
    Util::config()->contourVariable = cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString();

    // scalar field
    Util::config()->scalarVariable = cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString();
    Util::config()->scalarVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();
    Util::config()->scalarRangeAuto = chkScalarFieldRangeAuto->isChecked();
    Util::config()->scalarRangeMin = txtScalarFieldRangeMin->value();
    Util::config()->scalarRangeMax = txtScalarFieldRangeMax->value();

    Hermes::Module::LocalVariable *physicFieldVariable = Util::scene()->activeViewField()->module()->get_variable(Util::config()->scalarVariable.toStdString());
    if (physicFieldVariable && physicFieldVariable->id == "custom")
        physicFieldVariable->expression.scalar = txtPostScalarFieldExpression->text().toStdString();

    // vector field
    Util::config()->vectorVariable = cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString();
}

void PostprocessorWidget::saveAdvanced()
{
    // scalar field
    Util::config()->showScalarColorBar = chkShowScalarColorBar->isChecked();
    Util::config()->paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    Util::config()->paletteFilter = chkPaletteFilter->isChecked();
    Util::config()->paletteSteps = txtPaletteSteps->value();
    Util::config()->linearizerQuality = cmbLinearizerQuality->itemData(cmbLinearizerQuality->currentIndex()).toDouble();

    // contours
    Util::config()->contoursCount = txtContoursCount->value();

    // vector field
    Util::config()->vectorProportional = chkVectorProportional->isChecked();
    Util::config()->vectorColor = chkVectorColor->isChecked();
    Util::config()->vectorCount = txtVectorCount->value();
    Util::config()->vectorScale = txtVectorScale->value();

    // order view
    Util::config()->showOrderColorBar = chkShowOrderScale->isChecked();
    Util::config()->orderPaletteOrderType = (PaletteOrderType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt();
    Util::config()->orderLabel = chkOrderLabel->isChecked();

    // particle tracing
    Util::config()->particleIncludeGravitation = chkParticleIncludeGravitation->isChecked();
    Util::config()->particleNumberOfParticles = txtParticleNumberOfParticles->value();
    Util::config()->particleStartingRadius = txtParticleStartingRadius->value();
    Util::config()->particleMass = txtParticleMass->value();
    Util::config()->particleConstant = txtParticleConstant->value();
    Util::config()->particleStart.x = txtParticlePointX->value();
    Util::config()->particleStart.y = txtParticlePointY->value();
    Util::config()->particleStartVelocity.x = txtParticleVelocityX->value();
    Util::config()->particleStartVelocity.y = txtParticleVelocityY->value();
    Util::config()->particleTerminateOnDifferentMaterial = chkParticleTerminateOnDifferentMaterial->isChecked();
    Util::config()->particleMaximumRelativeError = txtParticleMaximumRelativeError->value();
    Util::config()->particleMaximumSteps = txtParticleMaximumSteps->value();
    Util::config()->particleColorByVelocity = chkParticleColorByVelocity->isChecked();
    Util::config()->particleShowPoints = chkParticleShowPoints->isChecked();
    Util::config()->particleDragDensity = txtParticleDragDensity->value();
    Util::config()->particleDragCoefficient = txtParticleDragCoefficient->value();
    Util::config()->particleDragReferenceArea = txtParticleDragReferenceArea->value();

    // advanced
    // scalar view
    Util::config()->scalarRangeLog = chkScalarFieldRangeLog->isChecked();
    Util::config()->scalarRangeBase = txtScalarFieldRangeBase->text().toDouble();
    Util::config()->scalarDecimalPlace = txtScalarDecimalPlace->value();

    // save
    Util::config()->save();
}

void PostprocessorWidget::createControls()
{
    // tab widget
    basic = controlsBasic();
    postprocessor = controlsPostprocessor();

    QTabWidget *tabType = new QTabWidget(this);
    tabType->addTab(basic, icon(""), tr("Basic"));
    tabType->addTab(postprocessor, icon(""), tr("Postprocessor"));

    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnOK);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 5, 3, 5);
    layoutMain->addWidget(tabType);
    layoutMain->addLayout(layoutButtons);

    setControls();

    setLayout(layoutMain);
}

QWidget *PostprocessorWidget::meshWidget()
{
    // layout mesh
    chkShowInitialMeshView = new QCheckBox(tr("Initial mesh"));
    chkShowSolutionMeshView = new QCheckBox(tr("Solution mesh"));
    chkShowOrderView = new QCheckBox(tr("Polynomial order"));

    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->addWidget(chkShowInitialMeshView, 0, 0);
    layoutMesh->addWidget(chkShowSolutionMeshView, 1, 0);
    layoutMesh->addWidget(chkShowOrderView, 0, 1);

    QHBoxLayout *layoutShowMesh = new QHBoxLayout();
    layoutShowMesh->addLayout(layoutMesh);
    layoutShowMesh->addStretch();

    QGroupBox *grpShowMesh = new QGroupBox(tr("Mesh and polynomial order"));
    grpShowMesh->setLayout(layoutShowMesh);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(grpShowMesh);
    layout->addStretch();

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorWidget::post2DWidget()
{
    // layout post2d
    chkShowPost2DContourView = new QCheckBox(tr("Contours"));
    connect(chkShowPost2DContourView, SIGNAL(clicked()), this, SLOT(setControls()));
    chkShowPost2DVectorView = new QCheckBox(tr("Vectors"));
    connect(chkShowPost2DVectorView, SIGNAL(clicked()), this, SLOT(setControls()));
    chkShowPost2DScalarView = new QCheckBox(tr("Scalar view"));
    connect(chkShowPost2DScalarView, SIGNAL(clicked()), this, SLOT(setControls()));
    chkShowPost2DParticleView = new QCheckBox(tr("Particle tracing"));

    QGridLayout *layoutPost2D = new QGridLayout();
    layoutPost2D->addWidget(chkShowPost2DScalarView, 0, 0);
    layoutPost2D->addWidget(chkShowPost2DParticleView, 1, 0);
    layoutPost2D->addWidget(chkShowPost2DContourView, 0, 1);
    layoutPost2D->addWidget(chkShowPost2DVectorView, 1, 1);

    QHBoxLayout *layoutShowPost2D = new QHBoxLayout();
    layoutShowPost2D->addLayout(layoutPost2D);
    layoutShowPost2D->addStretch();

    QGroupBox *grpShowPost2D = new QGroupBox(tr("Postprocessor 2D"));
    grpShowPost2D->setLayout(layoutShowPost2D);

    // layout scalar field
    cmbPostScalarFieldVariable = new QComboBox();
    connect(cmbPostScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));
    cmbPostScalarFieldVariableComp = new QComboBox();
    connect(cmbPostScalarFieldVariableComp, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariableComp(int)));
    txtPostScalarFieldExpression = new QLineEdit();

    chkScalarFieldRangeAuto = new QCheckBox(tr("Auto range"));
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->setColumnMinimumWidth(0, minWidth);
    layoutScalarField->setColumnStretch(1, 1);
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariable, 0, 1, 1, 3);
    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariableComp, 1, 1, 1, 3);
    layoutScalarField->addWidget(new QLabel(tr("Expression:")), 2, 0);
    layoutScalarField->addWidget(txtPostScalarFieldExpression, 2, 1, 1, 3);

    QGroupBox *grpScalarField = new QGroupBox(tr("Scalar field"));
    grpScalarField->setLayout(layoutScalarField);

    // contour field
    cmbPost2DContourVariable = new QComboBox();

    QGridLayout *layoutContourField = new QGridLayout();
    layoutContourField->setColumnMinimumWidth(0, minWidth);
    layoutContourField->setColumnStretch(1, 1);
    layoutContourField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutContourField->addWidget(cmbPost2DContourVariable, 0, 1);

    QGroupBox *grpContourField = new QGroupBox(tr("Contour field"));
    grpContourField->setLayout(layoutContourField);

    // vector field
    cmbPost2DVectorFieldVariable = new QComboBox();

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->setColumnMinimumWidth(0, minWidth);
    layoutVectorField->setColumnStretch(1, 1);
    layoutVectorField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutVectorField->addWidget(cmbPost2DVectorFieldVariable, 0, 1);

    QGroupBox *grpVectorField = new QGroupBox(tr("Vector field"));
    grpVectorField->setLayout(layoutVectorField);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(grpShowPost2D);
    layout->addWidget(grpScalarField);
    layout->addWidget(grpContourField);
    layout->addWidget(grpVectorField);
    layout->addStretch();

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
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
    layoutShowPost3D->addStretch();

    QGroupBox *grpShowPost3D = new QGroupBox(tr("Postprocessor 3D"));
    grpShowPost3D->setLayout(layoutShowPost3D);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(grpShowPost3D);
    layout->addStretch();

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorWidget::controlsBasic()
{
    cmbFieldInfo = new QComboBox();
    connect(cmbFieldInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldInfo(int)));

    QGridLayout *layoutField = new QGridLayout();
    layoutField->setColumnMinimumWidth(0, minWidth);
    layoutField->setColumnStretch(1, 1);
    layoutField->addWidget(new QLabel(tr("Physical field:")), 0, 0);
    layoutField->addWidget(cmbFieldInfo, 0, 1);

    QGroupBox *grpField = new QGroupBox(tr("Physical field"));
    grpField->setLayout(layoutField);

    // transient
    cmbTimeStep = new QComboBox(this);

    QGridLayout *layoutTransient = new QGridLayout();
    layoutTransient->setColumnMinimumWidth(0, minWidth);
    layoutTransient->setColumnStretch(1, 1);
    layoutTransient->addWidget(new QLabel(tr("Time step:")), 0, 0);
    layoutTransient->addWidget(cmbTimeStep, 0, 1);

    grpTransient = new QGroupBox(tr("Transient analysis"));
    grpTransient->setLayout(layoutTransient);

    // adaptivity
    cmbAdaptivityStep = new QComboBox(this);
    cmbAdaptivitySolutionType = new QComboBox(this);

    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, minWidth);
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Adaptivity step:")), 0, 0);
    layoutAdaptivity->addWidget(cmbAdaptivityStep, 0, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Solution type:")), 1, 0);
    layoutAdaptivity->addWidget(cmbAdaptivitySolutionType, 1, 1);

    grpAdaptivity = new QGroupBox(tr("Adaptivity"));
    grpAdaptivity->setLayout(layoutAdaptivity);

    mesh = meshWidget();
    post2d = post2DWidget();
    post3d = post3DWidget();

    widgetsLayout = new QStackedLayout();
    widgetsLayout->addWidget(mesh);
    widgetsLayout->addWidget(post2d);
    widgetsLayout->addWidget(post3d);

    QVBoxLayout *layoutBasic = new QVBoxLayout();
    layoutBasic->addWidget(grpField);
    layoutBasic->addWidget(grpTransient);
    layoutBasic->addWidget(grpAdaptivity);
    layoutBasic->addLayout(widgetsLayout);
    layoutBasic->addStretch();

    QWidget *basicWidget = new QWidget(this);
    basicWidget->setLayout(layoutBasic);

    // layout postprocessor
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(basicWidget);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorWidget::controlsPostprocessor()
{
    // scalar field
    // palette
    cmbPalette = new QComboBox();
    cmbPalette->addItem(tr("Jet"), Palette_Jet);
    cmbPalette->addItem(tr("Hot"), Palette_Hot);
    cmbPalette->addItem(tr("Copper"), Palette_Copper);
    cmbPalette->addItem(tr("Cool"), Palette_Cool);
    cmbPalette->addItem(tr("Bone"), Palette_Bone);
    cmbPalette->addItem(tr("Pink"), Palette_Pink);
    cmbPalette->addItem(tr("Spring"), Palette_Spring);
    cmbPalette->addItem(tr("Summer"), Palette_Summer);
    cmbPalette->addItem(tr("Autumn"), Palette_Autumn);
    cmbPalette->addItem(tr("Winter"), Palette_Winter);
    cmbPalette->addItem(tr("HSV"), Palette_HSV);
    cmbPalette->addItem(tr("B/W ascending"), Palette_BWAsc);
    cmbPalette->addItem(tr("B/W descending"), Palette_BWDesc);

    // quality
    cmbLinearizerQuality = new QComboBox();
    cmbLinearizerQuality->addItem(tr("Extremely coarse"), 0.01);
    cmbLinearizerQuality->addItem(tr("Extra coarse"), 0.007);
    cmbLinearizerQuality->addItem(tr("Coarser"), 0.003);
    cmbLinearizerQuality->addItem(tr("Coarse"), 0.001);
    cmbLinearizerQuality->addItem(tr("Normal"), LINEARIZER_QUALITY);
    cmbLinearizerQuality->addItem(tr("Fine"), 0.0001);
    cmbLinearizerQuality->addItem(tr("Finer"), 0.0006);
    cmbLinearizerQuality->addItem(tr("Extra fine"), 0.00001);

    chkPaletteFilter = new QCheckBox(tr("Filter"));
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    // steps
    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(5);
    txtPaletteSteps->setMaximum(100);

    // log scale
    chkScalarFieldRangeLog = new QCheckBox(tr("Log. scale"));
    txtScalarFieldRangeBase = new SLineEditDouble(SCALARFIELDRANGEBASE);
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    txtScalarFieldRangeMin = new SLineEditDouble(0.1, true);
    connect(txtScalarFieldRangeMin, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMinChanged()));
    lblScalarFieldRangeMinError = new QLabel("");
    lblScalarFieldRangeMinError->setPalette(palette);
    txtScalarFieldRangeMax = new SLineEditDouble(0.1, true);
    connect(txtScalarFieldRangeMax, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMaxChanged()));
    lblScalarFieldRangeMaxError = new QLabel("");
    lblScalarFieldRangeMaxError->setPalette(palette);

    QGridLayout *layoutScalarFieldRange = new QGridLayout();
    layoutScalarFieldRange->addWidget(chkScalarFieldRangeAuto, 3, 0);
    lblScalarFieldRangeMin = new QLabel(tr("Minimum:"));
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMin, 3, 1);
    layoutScalarFieldRange->addWidget(txtScalarFieldRangeMin, 3, 2);
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMinError, 3, 3);
    lblScalarFieldRangeMax = new QLabel(tr("Maximum:"));
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMax, 4, 1);
    layoutScalarFieldRange->addWidget(txtScalarFieldRangeMax, 4, 2);
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMaxError, 4, 3);

    QGroupBox *grpScalarFieldRange = new QGroupBox(tr("Range"));
    grpScalarFieldRange->setLayout(layoutScalarFieldRange);

    QGridLayout *gridLayoutScalarFieldPalette = new QGridLayout();
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
    txtScalarDecimalPlace->setMinimum(1);
    txtScalarDecimalPlace->setMaximum(10);

    // color bar
    chkShowScalarColorBar = new QCheckBox(tr("Show colorbar"), this);

    QGridLayout *gridLayoutScalarFieldColorbar = new QGridLayout();
    gridLayoutScalarFieldColorbar->addWidget(new QLabel(tr("Decimal places:")), 0, 0);
    gridLayoutScalarFieldColorbar->addWidget(txtScalarDecimalPlace, 0, 1);
    gridLayoutScalarFieldColorbar->addWidget(chkShowScalarColorBar, 1, 0, 1, 2);

    QGroupBox *grpScalarFieldColorbar = new QGroupBox(tr("Colorbar"));
    grpScalarFieldColorbar->setLayout(gridLayoutScalarFieldColorbar);

    QPushButton *btnScalarFieldDefault = new QPushButton(tr("Default"));
    connect(btnScalarFieldDefault, SIGNAL(clicked()), this, SLOT(doScalarFieldDefault()));

    QVBoxLayout *layoutScalarFieldAdvanced = new QVBoxLayout();
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldPalette);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldColorbar);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldRange);
    layoutScalarFieldAdvanced->addStretch();
    layoutScalarFieldAdvanced->addWidget(btnScalarFieldDefault, 0, Qt::AlignLeft);

    QWidget *scalarFieldWidget = new QWidget();
    scalarFieldWidget->setLayout(layoutScalarFieldAdvanced);

    // contours and vectors
    // contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(1);
    txtContoursCount->setMaximum(100);

    // vectors
    chkVectorProportional = new QCheckBox(tr("Proportional"), this);
    chkVectorColor = new QCheckBox(tr("Color (b/w)"), this);
    txtVectorCount = new QSpinBox(this);
    txtVectorCount->setMinimum(1);
    txtVectorCount->setMaximum(500);
    txtVectorScale = new QDoubleSpinBox(this);
    txtVectorScale->setDecimals(2);
    txtVectorScale->setSingleStep(0.1);
    txtVectorScale->setMinimum(0);
    txtVectorScale->setMaximum(20);

    // QPushButton *btnContoursDefault = new QPushButton(tr("Default"));
    // connect(btnContoursDefault, SIGNAL(clicked()), this, SLOT(doContoursVectorsDefault()));

    QGridLayout *gridLayoutContours = new QGridLayout();
    gridLayoutContours->setColumnMinimumWidth(0, minWidth);
    gridLayoutContours->setColumnStretch(1, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Contours count:")), 0, 0);
    gridLayoutContours->addWidget(txtContoursCount, 0, 1);

    QGroupBox *grpContours = new QGroupBox(tr("Contours"));
    grpContours->setLayout(gridLayoutContours);

    QGridLayout *gridLayoutVectors = new QGridLayout();
    gridLayoutVectors->addWidget(new QLabel(tr("Vectors:")), 0, 0);
    gridLayoutVectors->addWidget(txtVectorCount, 0, 1);
    gridLayoutVectors->addWidget(chkVectorProportional, 0, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Scale:")), 1, 0);
    gridLayoutVectors->addWidget(txtVectorScale, 1, 1);
    gridLayoutVectors->addWidget(chkVectorColor, 1, 2);

    QGroupBox *grpVectors = new QGroupBox(tr("Vectors"));
    grpVectors->setLayout(gridLayoutVectors);

    QVBoxLayout *layoutContoursVectors = new QVBoxLayout();
    layoutContoursVectors->addWidget(grpContours);
    layoutContoursVectors->addWidget(grpVectors);
    layoutContoursVectors->addStretch();
    // layoutContoursVectors->addWidget(btnContoursDefault, 0, Qt::AlignLeft);

    QWidget *contoursVectorsWidget = new QWidget();
    contoursVectorsWidget->setLayout(layoutContoursVectors);

    // polynomial order
    // palette
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

    // scale
    chkShowOrderScale = new QCheckBox(tr("Show scale"), this);
    chkOrderLabel = new QCheckBox(tr("Show order labels"), this);

    QPushButton *btnOrderDefault = new QPushButton(tr("Default"));
    connect(btnOrderDefault, SIGNAL(clicked()), this, SLOT(doOrderDefault()));

    QGridLayout *gridLayoutOrder = new QGridLayout();
    gridLayoutOrder->setColumnMinimumWidth(0, minWidth);
    gridLayoutOrder->setColumnStretch(1, 1);
    gridLayoutOrder->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutOrder->addWidget(cmbOrderPaletteOrder, 0, 1);
    gridLayoutOrder->addWidget(chkShowOrderScale, 1, 0, 1, 2);
    gridLayoutOrder->addWidget(chkOrderLabel, 2, 0, 1, 2);

    QVBoxLayout *layoutOrder = new QVBoxLayout();
    layoutOrder->addLayout(gridLayoutOrder);
    layoutOrder->addStretch();
    layoutOrder->addWidget(btnOrderDefault, 0, Qt::AlignLeft);

    QWidget *orderWidget = new QWidget();
    orderWidget->setLayout(layoutOrder);

    // particle tracing
    chkParticleIncludeGravitation = new QCheckBox(tr("Include gravitation"));
    txtParticleNumberOfParticles = new QSpinBox(this);
    txtParticleNumberOfParticles->setMinimum(1);
    txtParticleNumberOfParticles->setMaximum(200);
    txtParticleStartingRadius = new SLineEditDouble();
    txtParticleMass = new SLineEditDouble();
    txtParticleConstant = new SLineEditDouble();
    txtParticlePointX = new SLineEditDouble();
    txtParticlePointY = new SLineEditDouble();
    txtParticleVelocityX = new SLineEditDouble();
    txtParticleVelocityY = new SLineEditDouble();
    txtParticleMaximumRelativeError = new SLineEditDouble();
    chkParticleTerminateOnDifferentMaterial = new QCheckBox(tr("Terminate on different material"));
    lblParticlePointX = new QLabel();
    lblParticlePointY = new QLabel();
    lblParticleVelocityX = new QLabel();
    lblParticleVelocityY = new QLabel();
    chkParticleColorByVelocity = new QCheckBox(tr("Line color is controlled by velocity"));
    chkParticleShowPoints = new QCheckBox(tr("Show points"));
    txtParticleMaximumSteps = new QSpinBox();
    txtParticleMaximumSteps->setMinimum(100);
    txtParticleMaximumSteps->setMaximum(100000);
    txtParticleMaximumSteps->setSingleStep(100);
    txtParticleDragDensity = new SLineEditDouble();
    txtParticleDragCoefficient = new SLineEditDouble();
    txtParticleDragReferenceArea = new SLineEditDouble();
    lblParticleMotionEquations = new QLabel();

    QPushButton *btnParticleDefault = new QPushButton(tr("Default"));
    connect(btnParticleDefault, SIGNAL(clicked()), this, SLOT(doParticleDefault()));

    // Lorentz force
    QGridLayout *gridLayoutLorentzForce = new QGridLayout();
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Equation:")), 0, 0);
    gridLayoutLorentzForce->addWidget(new QLabel(QString("<i><b>F</b></i><sub>L</sub> = <i>Q</i> (<i><b>E</b></i> + <i><b>v</b></i> x <i><b>B</b></i>)")), 0, 1);
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Charge (C):")), 1, 0);
    gridLayoutLorentzForce->addWidget(txtParticleConstant, 1, 1);

    QGroupBox *grpLorentzForce = new QGroupBox(tr("Lorentz Force"));
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

    // advanced
    QGridLayout *gridLayoutAdvanced = new QGridLayout();
    gridLayoutAdvanced->addWidget(chkParticleIncludeGravitation, 0, 0);
    gridLayoutAdvanced->addWidget(new QLabel(QString("<i><b>F</b></i><sub>G</sub> = (0, m g<sub>0</sub>, 0))")), 0, 1);
    gridLayoutAdvanced->addWidget(chkParticleTerminateOnDifferentMaterial, 1, 0, 1, 2);
    gridLayoutAdvanced->addWidget(chkParticleColorByVelocity, 2, 0, 1, 2);
    gridLayoutAdvanced->addWidget(chkParticleShowPoints, 3, 0, 1, 2);
    gridLayoutAdvanced->addWidget(new QLabel(tr("Maximum relative error (%):")), 4, 0);
    gridLayoutAdvanced->addWidget(txtParticleMaximumRelativeError, 4, 1);
    gridLayoutAdvanced->addWidget(new QLabel(tr("Maximum steps:")), 5, 0);
    gridLayoutAdvanced->addWidget(txtParticleMaximumSteps, 5, 1);

    QGroupBox *grpAdvanced = new QGroupBox(tr("Advanced"));
    grpAdvanced->setLayout(gridLayoutAdvanced);

    QGridLayout *gridLayoutParticle = new QGridLayout();
    gridLayoutParticle->addWidget(new QLabel(tr("Equations:")), 0, 0);
    gridLayoutParticle->addWidget(lblParticleMotionEquations, 1, 0, 1, 2);
    gridLayoutParticle->addWidget(new QLabel(tr("Number of particles:")), 2, 0);
    gridLayoutParticle->addWidget(txtParticleNumberOfParticles, 2, 1);
    gridLayoutParticle->addWidget(new QLabel(tr("Particles dispersion (m):")), 3, 0);
    gridLayoutParticle->addWidget(txtParticleStartingRadius, 3, 1);
    gridLayoutParticle->addWidget(new QLabel(tr("Mass (kg):")), 4, 0);
    gridLayoutParticle->addWidget(txtParticleMass, 4, 1);
    gridLayoutParticle->addWidget(grpInitialPosition, 5, 0, 1, 2);
    gridLayoutParticle->addWidget(grpInitialVelocity, 6, 0, 1, 2);
    gridLayoutParticle->addWidget(grpLorentzForce, 7, 0, 1, 2);
    gridLayoutParticle->addWidget(grpDragForce, 8, 0, 1, 2);
    gridLayoutParticle->addWidget(grpAdvanced, 9, 0, 1, 2);

    QVBoxLayout *layoutParticle = new QVBoxLayout();
    layoutParticle->addLayout(gridLayoutParticle);
    layoutParticle->addStretch();
    layoutParticle->addWidget(btnParticleDefault, 0, Qt::AlignLeft);

    QWidget *particleWidget = new QWidget();
    particleWidget->setLayout(layoutParticle);

    tbxPostprocessor = new QToolBox();
    tbxPostprocessor->addItem(scalarFieldWidget, icon(""), tr("Scalar view"));
    tbxPostprocessor->addItem(contoursVectorsWidget, icon(""), tr("Contours and vectors"));
    tbxPostprocessor->addItem(orderWidget, icon(""), tr("Polynomial order"));
    tbxPostprocessor->addItem(particleWidget, icon(""), tr("Particle tracing"));

    // layout postprocessor
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tbxPostprocessor);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

void PostprocessorWidget::doFieldInfo(int index)
{
    QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
    if (Util::scene()->hasField(fieldName))
    {
        FieldInfo *fieldInfo = Util::scene()->fieldInfo(fieldName);
        Util::scene()->setActiveViewField(fieldInfo);

        fillComboBoxScalarVariable(fieldInfo, cmbPostScalarFieldVariable);
        fillComboBoxContourVariable(fieldInfo, cmbPost2DContourVariable);
        fillComboBoxVectorVariable(fieldInfo, cmbPost2DVectorFieldVariable);
    }
}

void PostprocessorWidget::doScalarFieldVariable(int index)
{
    if (cmbFieldInfo->currentIndex() == -1)
        return;

    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    Hermes::Module::LocalVariable *physicFieldVariable = NULL;

    if (cmbPostScalarFieldVariable->currentIndex() != -1)
    {
        QString variableName(cmbPostScalarFieldVariable->itemData(index).toString());

        QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
        physicFieldVariable = Util::scene()->fieldInfo(fieldName)->module()->get_variable(variableName.toStdString());
    }

    // component
    cmbPostScalarFieldVariableComp->clear();
    if (physicFieldVariable)
    {        
        if (physicFieldVariable->is_scalar)
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
        }
        else
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
            cmbPostScalarFieldVariableComp->addItem(Util::scene()->problemInfo()->labelX(), PhysicFieldVariableComp_X);
            cmbPostScalarFieldVariableComp->addItem(Util::scene()->problemInfo()->labelY(), PhysicFieldVariableComp_Y);
        }
    }

    if (cmbPostScalarFieldVariableComp->currentIndex() == -1)
        cmbPostScalarFieldVariableComp->setCurrentIndex(cmbPostScalarFieldVariableComp->findData(scalarFieldVariableComp));
    if (cmbPostScalarFieldVariableComp->currentIndex() == -1)
        cmbPostScalarFieldVariableComp->setCurrentIndex(0);

    doScalarFieldVariableComp(cmbPostScalarFieldVariableComp->currentIndex());
}

void PostprocessorWidget::doScalarFieldVariableComp(int index)
{
    if (cmbPostScalarFieldVariable->currentIndex() == -1)
        return;

    txtPostScalarFieldExpression->setText("");

    Hermes::Module::LocalVariable *physicFieldVariable = NULL;

    // TODO: proc je tu index a cmb..->currentIndex?
    if (cmbPostScalarFieldVariable->currentIndex() != -1 && index != -1)
    {
        QString variableName(cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());

        // TODO: not good - relies on variable names begining with module name
        std::string fieldName(variableName.split("_")[0].toStdString());

        physicFieldVariable = Util::scene()->fieldInfo(fieldName)->module()->get_variable(variableName.toStdString());
    }

    if (physicFieldVariable)
    {
        txtPostScalarFieldExpression->setEnabled(physicFieldVariable->id == "custom");

        // expression
        switch ((PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt())
        {
        case PhysicFieldVariableComp_Scalar:
            txtPostScalarFieldExpression->setText(QString::fromStdString(physicFieldVariable->expression.scalar));
            break;
        case PhysicFieldVariableComp_Magnitude:
            txtPostScalarFieldExpression->setText(QString("sqrt((%1)^2 + (%2)^2)").
                                                    arg(QString::fromStdString(physicFieldVariable->expression.comp_x)).
                                                    arg(QString::fromStdString(physicFieldVariable->expression.comp_y)));
            break;
        case PhysicFieldVariableComp_X:
            txtPostScalarFieldExpression->setText(QString::fromStdString(physicFieldVariable->expression.comp_x));
            break;
        case PhysicFieldVariableComp_Y:
            txtPostScalarFieldExpression->setText(QString::fromStdString(physicFieldVariable->expression.comp_y));
            break;
        }
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

void PostprocessorWidget::setControls()
{
    bool isMeshed = Util::problem()->isMeshed();
    bool isSolved = Util::problem()->isSolved();

    if (m_sceneMesh->actSceneModeMesh->isChecked())
    {
        widgetsLayout->setCurrentWidget(mesh);
        postprocessor->setEnabled(false);

        // mesh and order
        chkShowInitialMeshView->setEnabled(isMeshed);
        chkShowSolutionMeshView->setEnabled(isSolved);
        chkShowOrderView->setEnabled(isSolved);
    }

    if (m_scenePost2D->actSceneModePost2D->isChecked())
    {
        widgetsLayout->setCurrentWidget(post2d);
        postprocessor->setEnabled(true);

        chkShowPost2DContourView->setEnabled(isSolved && (cmbPost2DContourVariable->count() > 0));
        chkShowPost2DScalarView->setEnabled(isSolved && (cmbPostScalarFieldVariable->count() > 0));
        chkShowPost2DVectorView->setEnabled(isSolved && (cmbPost2DVectorFieldVariable->count() > 0));
        // if (Util::scene()->problemInfo()->hermes()->hasParticleTracing())
        // {
        // chkShowParticleTracing->setEnabled(isSolved && (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState));
        chkShowPost2DParticleView->setEnabled(isSolved);
        // }
        // else
        // {
        //     chkShowParticleTracing->setEnabled(false);
        //     chkShowParticleTracing->setChecked(false);
        // }

        // contour
        cmbPost2DContourVariable->setEnabled(chkShowPost2DContourView->isEnabled() && chkShowPost2DContourView->isChecked());

        // scalar view
        cmbPostScalarFieldVariable->setEnabled(chkShowPost2DScalarView->isEnabled() && chkShowPost2DScalarView->isChecked());
        cmbPostScalarFieldVariableComp->setEnabled(chkShowPost2DScalarView->isEnabled() && chkShowPost2DScalarView->isChecked());
        txtPostScalarFieldExpression->setEnabled(false);
        if (chkShowPost2DScalarView->isEnabled() && chkShowPost2DScalarView->isChecked())
        {
            doScalarFieldRangeAuto(-1);
            doScalarFieldVariableComp(cmbPostScalarFieldVariableComp->currentIndex());
        }

        cmbPost2DVectorFieldVariable->setEnabled(chkShowPost2DVectorView->isEnabled() && chkShowPost2DVectorView->isChecked());
    }

    if (m_scenePost3D->actSceneModePost3D->isChecked())
    {
        widgetsLayout->setCurrentWidget(post3d);
        postprocessor->setEnabled(true);

        // scalar view 3d
        radPost3DNone->setEnabled(isSolved);
        radPost3DScalarField3D->setEnabled(isSolved);
        radPost3DScalarField3DSolid->setEnabled(isSolved);
        radPost3DModel->setEnabled(isSolved);
        radPost3DParticleTracing->setEnabled(chkShowPost2DParticleView->isEnabled());
    }

    grpTransient->setVisible(false);
    grpAdaptivity->setVisible(false);
    if (isSolved)
    {
        // transient group
        int timeSteps = Util::solutionStore()->timeLevels(Util::scene()->activeViewField()).count();
        grpTransient->setVisible(timeSteps > 1);
        cmbTimeStep->setEnabled(timeSteps > 1);

        // adaptivity group
        int lastStep = Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionType_Normal, Util::scene()->activeTimeStep());
        grpAdaptivity->setVisible(lastStep > 0);
        cmbAdaptivityStep->setEnabled(lastStep > 0);
        cmbAdaptivitySolutionType->setEnabled(lastStep > 0);
    }
}

void PostprocessorWidget::updateControls()
{
    fillComboBoxFieldInfo(cmbFieldInfo);
    doFieldInfo(cmbFieldInfo->currentIndex());
    fillComboBoxTimeStep(cmbTimeStep);
    fillComboBoxAdaptivityStep(cmbAdaptivityStep);
    fillComboBoxSolutionType(cmbAdaptivitySolutionType);

    loadBasic();
    loadAdvanced();
}

void PostprocessorWidget::doPostprocessorGroupClicked(QAbstractButton *button)
{
    setControls();
}

void PostprocessorWidget::doApply()
{
    saveBasic();
    saveAdvanced();

    // time step
    QApplication::processEvents();

    Util::scene()->setActiveTimeStep(cmbTimeStep->currentIndex());
    Util::scene()->setActiveAdaptivityStep(cmbAdaptivityStep->currentIndex());
    Util::scene()->setActiveSolutionType((SolutionType)cmbAdaptivitySolutionType->currentIndex());

    // read auto range values
    if (chkScalarFieldRangeAuto->isChecked())
    {
        txtScalarFieldRangeMin->setValue(Util::config()->scalarRangeMin);
        txtScalarFieldRangeMax->setValue(Util::config()->scalarRangeMax);
    }

    emit apply();

    activateWindow();
}

void PostprocessorWidget::doScalarFieldDefault()
{
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) PALETTETYPE));
    chkPaletteFilter->setChecked(PALETTEFILTER);
    txtPaletteSteps->setValue(PALETTESTEPS);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(LINEARIZER_QUALITY));
    chkShowScalarColorBar->setChecked(SHOWSCALARCOLORBAR);
    chkScalarFieldRangeLog->setChecked(SCALARFIELDRANGELOG);
    txtScalarFieldRangeBase->setValue(SCALARFIELDRANGEBASE);
    txtScalarDecimalPlace->setValue(SCALARDECIMALPLACE);
}

void PostprocessorWidget::doContoursDefault()
{
    txtContoursCount->setValue(CONTOURSCOUNT);
}

void PostprocessorWidget::doVectorFieldDefault()
{
    chkVectorProportional->setChecked(VECTORPROPORTIONAL);
    chkVectorColor->setChecked(VECTORCOLOR);
    txtVectorCount->setValue(VECTORNUMBER);
    txtVectorScale->setValue(VECTORSCALE);
}

void PostprocessorWidget::doOrderDefault()
{
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) ORDERPALETTEORDERTYPE));
    chkShowOrderScale->setChecked(SHOWORDERCOLORBAR);
    chkOrderLabel->setChecked(ORDERLABEL);
}

void PostprocessorWidget::doParticleDefault()
{
    txtParticleNumberOfParticles->setValue(PARTICLENUMBEROFPARTICLES);
    txtParticleStartingRadius->setValue(PARTICLESTARTINGRADIUS);
    chkParticleIncludeGravitation->setChecked(PARTICLEINCLUDEGRAVITATION);
    txtParticleMass->setValue(PARTICLEMASS);
    txtParticleConstant->setValue(PARTICLECONSTANT);
    txtParticlePointX->setValue(PARTICLESTARTX);
    txtParticlePointY->setValue(PARTICLESTARTY);
    txtParticleVelocityX->setValue(PARTICLESTARTVELOCITYX);
    txtParticleVelocityY->setValue(PARTICLESTARTVELOCITYY);
    chkParticleTerminateOnDifferentMaterial->setChecked(PARTICLETERMINATEONDIFFERENTMATERIAL);
    txtParticleMaximumRelativeError->setValue(PARTICLEMAXIMUMRELATIVEERROR);
    txtParticleMaximumSteps->setValue(PARTICLEMAXIMUMSTEPS);
    chkParticleColorByVelocity->setChecked(PARTICLECOLORBYVELOCITY);
    chkParticleShowPoints->setChecked(PARTICLESHOWPOINTS);
    txtParticleDragDensity->setValue(PARTICLEDRAGDENSITY);
    txtParticleDragReferenceArea->setValue(PARTICLEDRAGREFERENCEAREA);
    txtParticleDragCoefficient->setValue(PARTICLEDRAGCOEFFICIENT);
}

void PostprocessorWidget::doScalarFieldRangeMinChanged()
{
    lblScalarFieldRangeMinError->clear();
    lblScalarFieldRangeMaxError->clear();
    btnOK->setEnabled(true);

    if (txtScalarFieldRangeMin->value() > txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("> %1").arg(txtScalarFieldRangeMax->value()));
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
