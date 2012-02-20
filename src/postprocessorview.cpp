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
#include "sceneview.h"

PostprocessorView::PostprocessorView(SceneView *sceneView, QWidget *parent) : QDockWidget(tr("View Properties"), parent)
{
    logMessage("PostprocessorView::PostprocessorView()");

    m_sceneView = sceneView;

    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorView");

    createControls();

    loadBasic();
    loadAdvanced();
}

void PostprocessorView::loadBasic()
{
    logMessage("PostprocessorView::loadBasic()");

    // show
    chkShowGeometry->setChecked(m_sceneView->sceneViewSettings().showGeometry);
    chkShowInitialMesh->setChecked(m_sceneView->sceneViewSettings().showInitialMesh);

    radPostprocessorNone->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_None);
    radPostprocessorScalarField->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView);
    radPostprocessorScalarField3D->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3D);
    radPostprocessorScalarField3DSolid->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid);
    radPostprocessorParticleTracing3D->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ParticleTracing3D);
    radPostprocessorModel->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_Model);
    radPostprocessorOrder->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_Order);
    doPostprocessorGroupClicked(butPostprocessorGroup->checkedButton());

    chkShowContours->setChecked(m_sceneView->sceneViewSettings().showContours);
    chkShowVectors->setChecked(m_sceneView->sceneViewSettings().showVectors);
    chkShowParticleTracing->setChecked(m_sceneView->sceneViewSettings().showParticleTracing);
    chkShowSolutionMesh->setChecked(m_sceneView->sceneViewSettings().showSolutionMesh);

    // scalar field
    cmbScalarFieldVariable->setCurrentIndex(cmbScalarFieldVariable->findData(m_sceneView->sceneViewSettings().scalarPhysicFieldVariable));
    doScalarFieldVariable(cmbScalarFieldVariable->currentIndex());
    cmbScalarFieldVariableComp->setCurrentIndex(cmbScalarFieldVariableComp->findData(m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp));
    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(0);
    chkScalarFieldRangeAuto->setChecked(m_sceneView->sceneViewSettings().scalarRangeAuto);
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeMin));
    txtScalarFieldRangeMax->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeMax));

    // vector field
    cmbVectorFieldVariable->setCurrentIndex(cmbVectorFieldVariable->findData(m_sceneView->sceneViewSettings().vectorPhysicFieldVariable));

    // transient view
    cmbTimeStep->setCurrentIndex(Util::scene()->sceneSolution()->timeStep());

    setControls();
}

void PostprocessorView::loadAdvanced()
{
    logMessage("PostprocessorView::loadAdvanced()");

    // workspace
    txtGridStep->setText(QString::number(Util::config()->gridStep));
    chkShowGrid->setChecked(Util::config()->showGrid);
    chkSnapToGrid->setChecked(Util::config()->snapToGrid);

    lblSceneFontExample->setFont(Util::config()->sceneFont);
    lblSceneFontExample->setText(QString("%1, %2").arg(lblSceneFontExample->font().family()).arg(lblSceneFontExample->font().pointSize()));

    chkShowAxes->setChecked(Util::config()->showAxes);
    chkShowRulers->setChecked(Util::config()->showRulers);
    chkShowLabel->setChecked(Util::config()->showLabel);

    chkZoomToMouse->setChecked(Util::config()->zoomToMouse);
    txtGeometryNodeSize->setValue(Util::config()->nodeSize);
    txtGeometryEdgeWidth->setValue(Util::config()->edgeWidth);
    txtGeometryLabelSize->setValue(Util::config()->labelSize);

    // scalar field
    chkShowScalarColorBar->setChecked(Util::config()->showScalarScale);
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
    chkShowOrderScale->setChecked(Util::config()->showOrderScale);
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

    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        lblParticleMotionEquations->setText(QString("<i>x</i>\" = <i>F</i><sub>x</sub> / <i>m</i>, &nbsp; <i>y</i>\" = <i>F</i><sub>y</sub> / <i>m</i>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>"));
    else
        lblParticleMotionEquations->setText(QString("<i>r</i>\" = <i>F</i><sub>r</sub> / <i>m</i> + <i>r</i> (<i>&phi;</i>')<sup>2</sup>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>, &nbsp; <i>&phi;</i>\" = <i>F</i><sub>&phi;</sub> / <i>m</i> - 2<i>r</i> <i>r</i>' <i>&phi;</i>' / <i>r</i>"));

    // advanced
    // scalar field
    chkScalarFieldRangeLog->setChecked(Util::config()->scalarRangeLog);
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setText(QString::number(Util::config()->scalarRangeBase));
    txtScalarDecimalPlace->setValue(Util::config()->scalarDecimalPlace);
    // 3d
    chkView3DLighting->setChecked(Util::config()->scalarView3DLighting);
    txtView3DAngle->setValue(Util::config()->scalarView3DAngle);
    chkView3DBackground->setChecked(Util::config()->scalarView3DBackground);
    txtView3DHeight->setValue(Util::config()->scalarView3DHeight);
    // deform shape
    chkDeformScalar->setChecked(Util::config()->deformScalar);
    chkDeformContour->setChecked(Util::config()->deformContour);
    chkDeformVector->setChecked(Util::config()->deformVector);
}

void PostprocessorView::saveBasic()
{
    logMessage("PostprocessorView::saveBasic()");

    // show
    m_sceneView->sceneViewSettings().showGeometry = chkShowGeometry->isChecked();
    m_sceneView->sceneViewSettings().showInitialMesh = chkShowInitialMesh->isChecked();

    if (radPostprocessorNone->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_None;
    if (radPostprocessorScalarField->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_ScalarView;
    if (radPostprocessorScalarField3D->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_ScalarView3D;
    if (radPostprocessorScalarField3DSolid->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_ScalarView3DSolid;
    if (radPostprocessorParticleTracing3D->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_ParticleTracing3D;
    if (radPostprocessorModel->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_Model;
    if (radPostprocessorOrder->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_Order;

    m_sceneView->sceneViewSettings().showContours = chkShowContours->isChecked();
    m_sceneView->sceneViewSettings().showVectors = chkShowVectors->isChecked();
    m_sceneView->sceneViewSettings().showParticleTracing = chkShowParticleTracing->isChecked();
    m_sceneView->sceneViewSettings().showSolutionMesh = chkShowSolutionMesh->isChecked();

    // scalar field
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(cmbScalarFieldVariable->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = (PhysicFieldVariableComp) cmbScalarFieldVariableComp->itemData(cmbScalarFieldVariableComp->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarRangeAuto = chkScalarFieldRangeAuto->isChecked();
    m_sceneView->sceneViewSettings().scalarRangeMin = txtScalarFieldRangeMin->text().toDouble();
    m_sceneView->sceneViewSettings().scalarRangeMax = txtScalarFieldRangeMax->text().toDouble();

    // vector field
    m_sceneView->sceneViewSettings().vectorPhysicFieldVariable = (PhysicFieldVariable) cmbVectorFieldVariable->itemData(cmbVectorFieldVariable->currentIndex()).toInt();
}

void PostprocessorView::saveAdvanced()
{
    logMessage("PostprocessorView::saveAdvanced()");

    // workspace
    Util::config()->showGrid = chkShowGrid->isChecked();
    Util::config()->gridStep = txtGridStep->text().toDouble();
    Util::config()->showRulers = chkShowRulers->isChecked();
    Util::config()->zoomToMouse = chkZoomToMouse->isChecked();
    Util::config()->snapToGrid = chkSnapToGrid->isChecked();

    Util::config()->sceneFont = lblSceneFontExample->font();

    Util::config()->showAxes = chkShowAxes->isChecked();
    Util::config()->showRulers = chkShowRulers->isChecked();
    Util::config()->showLabel = chkShowLabel->isChecked();

    Util::config()->nodeSize = txtGeometryNodeSize->value();
    Util::config()->edgeWidth = txtGeometryEdgeWidth->value();
    Util::config()->labelSize = txtGeometryLabelSize->value();

    // scalar field
    Util::config()->showScalarScale = chkShowScalarColorBar->isChecked();
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
    Util::config()->showOrderScale = chkShowOrderScale->isChecked();
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
    // 3d
    Util::config()->scalarView3DLighting = chkView3DLighting->isChecked();
    Util::config()->scalarView3DAngle = txtView3DAngle->value();
    Util::config()->scalarView3DBackground = chkView3DBackground->isChecked();
    Util::config()->scalarView3DHeight = txtView3DHeight->value();
    // deform shape
    Util::config()->deformScalar = chkDeformScalar->isChecked();
    Util::config()->deformContour = chkDeformContour->isChecked();
    Util::config()->deformVector = chkDeformVector->isChecked();

    // save
    Util::config()->save();
}

void PostprocessorView::createControls()
{
    logMessage("PostprocessorView::createControls()");

    // tab widget
    basic = controlsBasic();
    postprocessor = controlsPosprocessor();
    workspace = controlsWorkspace();

    QTabWidget *tabType = new QTabWidget(this);
    tabType->addTab(basic, icon(""), tr("Basic"));
    tabType->addTab(postprocessor, icon(""), tr("Postprocessor"));
    tabType->addTab(workspace, icon(""), tr("Workspace"));

    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnOK);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addLayout(layoutButtons);

    setControls();

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    setWidget(widget);
}

QWidget *PostprocessorView::controlsBasic()
{
    logMessage("PostprocessorView::controlsBasic()");

    double minWidth = 110;

    // layout show
    chkShowGeometry = new QCheckBox(tr("Geometry"));
    chkShowInitialMesh = new QCheckBox(tr("Initial mesh"));
    chkShowContours = new QCheckBox(tr("Contours"));
    chkShowVectors = new QCheckBox(tr("Vectors"));
    chkShowParticleTracing = new QCheckBox(tr("Particle tracing"));
    connect(chkShowVectors, SIGNAL(clicked()), this, SLOT(setControls()));
    chkShowSolutionMesh = new QCheckBox(tr("Solution mesh"));

    // postprocessor mode
    radPostprocessorNone = new QRadioButton(tr("None"), this);
    radPostprocessorOrder = new QRadioButton(tr("Polynomial order"), this);
    radPostprocessorScalarField = new QRadioButton(tr("Scalar view"), this);
    radPostprocessorScalarField3D = new QRadioButton(tr("Scalar view"), this);
    radPostprocessorScalarField3DSolid = new QRadioButton(tr("Scalar view solid"), this);
    radPostprocessorParticleTracing3D = new QRadioButton(tr("Particle tracing"), this);
    radPostprocessorModel = new QRadioButton("Model", this);

    butPostprocessorGroup = new QButtonGroup(this);
    butPostprocessorGroup->addButton(radPostprocessorNone);
    butPostprocessorGroup->addButton(radPostprocessorScalarField);
    butPostprocessorGroup->addButton(radPostprocessorOrder);
    butPostprocessorGroup->addButton(radPostprocessorScalarField3D);
    butPostprocessorGroup->addButton(radPostprocessorScalarField3DSolid);
    butPostprocessorGroup->addButton(radPostprocessorParticleTracing3D);
    butPostprocessorGroup->addButton(radPostprocessorModel);
    connect(butPostprocessorGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(doPostprocessorGroupClicked(QAbstractButton*)));

    QGridLayout *layoutShow = new QGridLayout();
    layoutShow->addWidget(chkShowGeometry, 0, 0);
    layoutShow->addWidget(chkShowInitialMesh, 1, 0);
    layoutShow->addWidget(chkShowSolutionMesh, 2, 0);
    layoutShow->addWidget(chkShowContours, 3, 0);
    layoutShow->addWidget(chkShowVectors, 4, 0);
    layoutShow->addWidget(chkShowParticleTracing, 5, 0);
    layoutShow->addWidget(new QLabel(tr("2D:")), 0, 1);
    layoutShow->addWidget(radPostprocessorNone, 0, 2);
    layoutShow->addWidget(radPostprocessorOrder, 1, 2);
    layoutShow->addWidget(radPostprocessorScalarField, 2, 2);
    layoutShow->addWidget(new QLabel(tr("3D:")), 3, 1);
    layoutShow->addWidget(radPostprocessorScalarField3D, 3, 2);
    layoutShow->addWidget(radPostprocessorScalarField3DSolid, 4, 2);
    layoutShow->addWidget(radPostprocessorParticleTracing3D, 5, 2);
    layoutShow->addWidget(radPostprocessorModel, 6, 2);

    QHBoxLayout *layoutShowSpace = new QHBoxLayout();
    layoutShowSpace->addLayout(layoutShow);
    layoutShowSpace->addStretch();

    QGroupBox *grpShow = new QGroupBox(tr("Show"));
    grpShow->setLayout(layoutShowSpace);

    // layout scalar field
    cmbScalarFieldVariable = new QComboBox();
    connect(cmbScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));

    cmbScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox(tr("Auto range"));
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

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

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->setColumnMinimumWidth(0, minWidth);
    layoutScalarField->setColumnStretch(1, 1);
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariable, 0, 1, 1, 3);

    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariableComp, 1, 1, 1, 3);

    layoutScalarField->addWidget(chkScalarFieldRangeAuto, 2, 0);
    lblScalarFieldRangeMin = new QLabel(tr("Minimum:"));
    layoutScalarField->addWidget(lblScalarFieldRangeMin, 2, 1);
    layoutScalarField->addWidget(txtScalarFieldRangeMin, 2, 2);
    layoutScalarField->addWidget(lblScalarFieldRangeMinError, 2, 3);
    lblScalarFieldRangeMax = new QLabel(tr("Maximum:"));
    layoutScalarField->addWidget(lblScalarFieldRangeMax, 3, 1);
    layoutScalarField->addWidget(txtScalarFieldRangeMax, 3, 2);
    layoutScalarField->addWidget(lblScalarFieldRangeMaxError, 3, 3);

    QGroupBox *grpScalarField = new QGroupBox(tr("Scalar field"));
    grpScalarField->setLayout(layoutScalarField);

    // vector field
    cmbVectorFieldVariable = new QComboBox();

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->setColumnMinimumWidth(0, minWidth);
    layoutVectorField->setColumnStretch(1, 1);
    layoutVectorField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutVectorField->addWidget(cmbVectorFieldVariable, 0, 1);

    QGroupBox *grpVectorField = new QGroupBox(tr("Vector field"));
    grpVectorField->setLayout(layoutVectorField);

    // transient
    cmbTimeStep = new QComboBox(this);

    QGridLayout *layoutTransient = new QGridLayout();
    layoutTransient->setColumnMinimumWidth(0, minWidth);
    layoutTransient->setColumnStretch(1, 1);
    layoutTransient->addWidget(new QLabel(tr("Time step:")), 0, 0);
    layoutTransient->addWidget(cmbTimeStep, 0, 1);

    QGroupBox *grpTransient = new QGroupBox(tr("Transient analysis"));
    grpTransient->setLayout(layoutTransient);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpShow);
    layout->addWidget(grpScalarField);
    layout->addWidget(grpVectorField);
    layout->addWidget(grpTransient);
    layout->addStretch();

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

QWidget *PostprocessorView::controlsPosprocessor()
{
    logMessage("PostprocessorView::controlsAdvanced()");

    double minWidth = 110;

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

    QGroupBox *grpScalarFieldPallete = new QGroupBox(tr("Palette"));
    grpScalarFieldPallete->setLayout(gridLayoutScalarFieldPalette);

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

    QVBoxLayout *layoutScalarField = new QVBoxLayout();
    layoutScalarField->addWidget(grpScalarFieldPallete);
    layoutScalarField->addWidget(grpScalarFieldColorbar);
    layoutScalarField->addStretch();
    layoutScalarField->addWidget(btnScalarFieldDefault, 0, Qt::AlignLeft);

    QWidget *scalarFieldWidget = new QWidget();
    scalarFieldWidget->setLayout(layoutScalarField);

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

    QPushButton *btnContoursDefault = new QPushButton(tr("Default"));
    connect(btnContoursDefault, SIGNAL(clicked()), this, SLOT(doContoursVectorsDefault()));

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
    layoutContoursVectors->addWidget(btnContoursDefault, 0, Qt::AlignLeft);

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

QWidget *PostprocessorView::controlsWorkspace()
{
    logMessage("PostprocessorView::controlsWorkspace()");

    // workspace
    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkShowGrid = new QCheckBox(tr("Show grid"));
    connect(chkShowGrid, SIGNAL(clicked()), this, SLOT(doShowGridChanged()));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));
    chkZoomToMouse = new QCheckBox(tr("Zoom to mouse pointer"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(new QLabel(tr("Grid step:")), 0, 0);
    layoutGrid->addWidget(txtGridStep, 0, 1);
    layoutGrid->addWidget(chkShowGrid, 1, 0, 1, 2);
    layoutGrid->addWidget(chkSnapToGrid, 2, 0, 1, 2);
    layoutGrid->addWidget(chkZoomToMouse, 3, 0, 1, 2);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    lblSceneFontExample = new QLabel(QString("%1, %2").arg(Util::config()->sceneFont.family()).arg(Util::config()->sceneFont.pointSize()));

    btnSceneFont = new QPushButton(tr("Set font"));
    connect(btnSceneFont, SIGNAL(clicked()), this, SLOT(doSceneFont()));

    QGridLayout *layoutFont = new QGridLayout();
    layoutFont->addWidget(lblSceneFontExample, 0, 1);
    layoutFont->addWidget(btnSceneFont, 0, 2);

    QGroupBox *grpFont = new QGroupBox(tr("Scene font"));
    grpFont->setLayout(layoutFont);

    // geometry
    txtGeometryNodeSize = new QSpinBox();
    txtGeometryNodeSize->setMinimum(1);
    txtGeometryNodeSize->setMaximum(20);
    txtGeometryEdgeWidth = new QSpinBox();
    txtGeometryEdgeWidth->setMinimum(1);
    txtGeometryEdgeWidth->setMaximum(20);
    txtGeometryLabelSize = new QSpinBox();
    txtGeometryLabelSize->setMinimum(1);
    txtGeometryLabelSize->setMaximum(20);

    QGridLayout *layoutGeometry = new QGridLayout();
    layoutGeometry->addWidget(new QLabel(tr("Node size:")), 0, 0);
    layoutGeometry->addWidget(txtGeometryNodeSize, 0, 1);
    layoutGeometry->addWidget(new QLabel(tr("Edge width:")), 1, 0);
    layoutGeometry->addWidget(txtGeometryEdgeWidth, 1, 1);
    layoutGeometry->addWidget(new QLabel(tr("Label size:")), 2, 0);
    layoutGeometry->addWidget(txtGeometryLabelSize, 2, 1);

    QGroupBox *grpGeometry = new QGroupBox(tr("Geometry"));
    grpGeometry->setLayout(layoutGeometry);

    // other
    chkShowRulers = new QCheckBox(tr("Show rulers"));
    chkShowAxes = new QCheckBox(tr("Show axes"));
    chkShowLabel = new QCheckBox(tr("Show label"));

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkShowAxes);
    layoutOther->addWidget(chkShowRulers);
    layoutOther->addWidget(chkShowLabel);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    QPushButton *btnWorkspaceDefault = new QPushButton(tr("Default"));
    connect(btnWorkspaceDefault, SIGNAL(clicked()), this, SLOT(doWorkspaceDefault()));

    QVBoxLayout *layoutWorkspace = new QVBoxLayout();
    layoutWorkspace->addWidget(grpGrid);
    layoutWorkspace->addWidget(grpFont);
    layoutWorkspace->addWidget(grpGeometry);
    layoutWorkspace->addWidget(grpOther);
    layoutWorkspace->addStretch();
    layoutWorkspace->addWidget(btnWorkspaceDefault, 0, Qt::AlignLeft);

    QWidget *workspaceWidget = new QWidget();
    workspaceWidget->setLayout(layoutWorkspace);

    // advanced
    // layout 3d
    chkView3DLighting = new QCheckBox(tr("Ligthing"), this);
    txtView3DAngle = new QDoubleSpinBox(this);
    txtView3DAngle->setDecimals(1);
    txtView3DAngle->setSingleStep(1);
    txtView3DAngle->setMinimum(30);
    txtView3DAngle->setMaximum(360);
    chkView3DBackground = new QCheckBox(tr("Gradient background"), this);
    txtView3DHeight = new QDoubleSpinBox(this);
    txtView3DHeight->setDecimals(1);
    txtView3DHeight->setSingleStep(0.1);
    txtView3DHeight->setMinimum(0.2);
    txtView3DHeight->setMaximum(10.0);

    QGridLayout *layout3D = new QGridLayout();
    layout3D->addWidget(new QLabel(tr("Angle:")), 0, 1);
    layout3D->addWidget(txtView3DAngle, 0, 2);
    layout3D->addWidget(chkView3DLighting, 0, 3);
    layout3D->addWidget(new QLabel(tr("Height:")), 1, 1);
    layout3D->addWidget(txtView3DHeight, 1, 2);
    layout3D->addWidget(chkView3DBackground, 1, 3);

    QGroupBox *grp3D = new QGroupBox(tr("3D view"));
    grp3D->setLayout(layout3D);

    // layout deform shape
    chkDeformScalar = new QCheckBox(tr("Scalar field"), this);
    chkDeformContour = new QCheckBox(tr("Contours"), this);
    chkDeformVector = new QCheckBox(tr("Vector field"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkDeformScalar, 0, 0);
    layoutDeformShape->addWidget(chkDeformContour, 0, 1);
    layoutDeformShape->addWidget(chkDeformVector, 0, 2);

    QGroupBox *grpDeformShape = new QGroupBox(tr("Deform shape"));
    grpDeformShape->setLayout(layoutDeformShape);

    QPushButton *btnAdvancedDefault = new QPushButton(tr("Default"));
    connect(btnAdvancedDefault, SIGNAL(clicked()), this, SLOT(doAdvancedDefault()));

    // layout postprocessor
    QVBoxLayout *layoutAdvanced = new QVBoxLayout();
    layoutAdvanced->addWidget(grp3D);
    layoutAdvanced->addWidget(grpDeformShape);
    layoutAdvanced->addStretch();
    layoutAdvanced->addWidget(btnAdvancedDefault, 0, Qt::AlignLeft);

    QWidget *advancedWidget = new QWidget(this);
    advancedWidget->setLayout(layoutAdvanced);

    tbxWorkspace = new QToolBox();
    tbxWorkspace->addItem(workspaceWidget, icon(""), tr("Workspace"));
    tbxWorkspace->addItem(advancedWidget, icon(""), tr("Advanced"));

    // layout workspace
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tbxWorkspace);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

void PostprocessorView::doScalarFieldVariable(int index)
{
    logMessage("PostprocessorView::doScalarFieldVariable()");

    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbScalarFieldVariableComp->itemData(cmbScalarFieldVariableComp->currentIndex()).toInt();
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(index).toInt();

    cmbScalarFieldVariableComp->clear();
    if (isPhysicFieldVariableScalar(physicFieldVariable))
    {
        cmbScalarFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbScalarFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbScalarFieldVariableComp->addItem(Util::scene()->problemInfo()->labelX(), PhysicFieldVariableComp_X);
        cmbScalarFieldVariableComp->addItem(Util::scene()->problemInfo()->labelY(), PhysicFieldVariableComp_Y);
    }

    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(cmbScalarFieldVariableComp->findData(scalarFieldVariableComp));
    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(0);
}

void PostprocessorView::doScalarFieldRangeAuto(int state)
{
    logMessage("PostprocessorView::doScalarFieldRangeAuto()");

    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());

    // show/hide range min/max
    lblScalarFieldRangeMin->setVisible(!chkScalarFieldRangeAuto->isChecked());
    lblScalarFieldRangeMax->setVisible(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMin->setVisible(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setVisible(!chkScalarFieldRangeAuto->isChecked());
    lblScalarFieldRangeMinError->setVisible(!chkScalarFieldRangeAuto->isChecked());
    lblScalarFieldRangeMaxError->setVisible(!chkScalarFieldRangeAuto->isChecked());
}

void PostprocessorView::doPaletteFilter(int state)
{
    logMessage("PostprocessorView::doPaletteFilter()");

    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}

void PostprocessorView::setControls()
{
    logMessage("PostprocessorView::setControls()");

    bool isMeshed = Util::scene()->sceneSolution()->isMeshed();
    bool isSolved = Util::scene()->sceneSolution()->isSolved();

    postprocessor->setEnabled(isSolved);

    chkShowGeometry->setEnabled(true);

    chkShowInitialMesh->setEnabled(isMeshed);
    chkShowSolutionMesh->setEnabled(isSolved && (cmbScalarFieldVariable->count() > 0));
    chkShowContours->setEnabled(isSolved);
    chkShowVectors->setEnabled(isSolved && (cmbVectorFieldVariable->count() > 0));
    if (Util::scene()->problemInfo()->hermes()->hasParticleTracing())
    {
        chkShowParticleTracing->setEnabled(isSolved && (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState));
    }
    else
    {
        chkShowParticleTracing->setEnabled(false);
        chkShowParticleTracing->setChecked(false);
    }

    radPostprocessorNone->setEnabled(isSolved);
    radPostprocessorScalarField->setEnabled(isSolved);
    radPostprocessorOrder->setEnabled(isSolved);
    radPostprocessorScalarField3D->setEnabled(isSolved);
    radPostprocessorScalarField3DSolid->setEnabled(isSolved);
    radPostprocessorModel->setEnabled(isSolved);

    radPostprocessorParticleTracing3D->setEnabled(chkShowParticleTracing->isEnabled());
    tbxPostprocessor->setItemEnabled(3, chkShowParticleTracing->isEnabled()); // FIXME - index


    cmbTimeStep->setEnabled(Util::scene()->sceneSolution()->timeStepCount() > 0);

    cmbScalarFieldVariable->setEnabled(false);
    cmbScalarFieldVariableComp->setEnabled(false);
    chkScalarFieldRangeAuto->setEnabled(false);
    cmbVectorFieldVariable->setEnabled(chkShowVectors->isChecked());

    if (isSolved && (radPostprocessorScalarField->isChecked() ||
                     radPostprocessorScalarField3D->isChecked() ||
                     radPostprocessorScalarField3DSolid->isChecked()))
    {
        cmbScalarFieldVariable->setEnabled(true);
        cmbScalarFieldVariableComp->setEnabled(true);

        chkScalarFieldRangeAuto->setEnabled(true);
        doScalarFieldRangeAuto(-1);
    }

    if (isSolved && (radPostprocessorScalarField3D->isChecked() ||
                     radPostprocessorParticleTracing3D->isChecked() ||
                     radPostprocessorScalarField3DSolid->isChecked() ||
                     radPostprocessorModel->isChecked()))
    {
        chkShowGeometry->setChecked(true);
        chkShowGeometry->setEnabled(false);
        chkShowInitialMesh->setEnabled(false);
        chkShowSolutionMesh->setEnabled(false);
        chkShowContours->setEnabled(false);
        chkShowVectors->setEnabled(false);
        chkShowParticleTracing->setEnabled(false);
    }
}

void PostprocessorView::updateControls()
{
    logMessage("PostprocessorView::updateControls()");

    fillComboBoxScalarVariable(cmbScalarFieldVariable);
    fillComboBoxVectorVariable(cmbVectorFieldVariable);
    fillComboBoxTimeStep(cmbTimeStep);

    loadBasic();
    loadAdvanced();
}

void PostprocessorView::doPostprocessorGroupClicked(QAbstractButton *button)
{
    logMessage("PostprocessorView::doPostprocessorGroupClicked()");

    setControls();
}

void PostprocessorView::doApply()
{
    logMessage("PostprocessorView::doApply()");

    saveBasic();
    saveAdvanced();

    // time step
    QApplication::processEvents();
    Util::scene()->sceneSolution()->setTimeStep(cmbTimeStep->currentIndex(), false);

    // read auto range values
    if (chkScalarFieldRangeAuto->isChecked())
    {
        txtScalarFieldRangeMin->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeMin));
        txtScalarFieldRangeMax->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeMax));
    }

    // switch to the postprocessor
    if (Util::scene()->sceneSolution()->isSolved())
        m_sceneView->actSceneModePostprocessor->trigger();

    emit apply();

    activateWindow();
}

void PostprocessorView::doWorkspaceDefault()
{
    logMessage("PostprocessorView::doWorkspaceDefault()");

    txtGridStep->setText(QString::number(GRIDSTEP));
    chkShowGrid->setChecked(SHOWGRID);
    chkSnapToGrid->setEnabled(SHOWGRID);
    chkSnapToGrid->setChecked(SNAPTOGRID);

    lblSceneFontExample->setFont(FONT);
    lblSceneFontExample->setText(QString("%1, %2").arg(lblSceneFontExample->font().family()).arg(lblSceneFontExample->font().pointSize()));

    chkShowAxes->setChecked(SHOWAXES);
    chkShowRulers->setChecked(SHOWRULERS);
    chkShowLabel->setChecked(SHOWLABEL);

    chkZoomToMouse->setChecked(ZOOMTOMOUSE);
    txtGeometryNodeSize->setValue(GEOMETRYNODESIZE);
    txtGeometryEdgeWidth->setValue(GEOMETRYEDGEWIDTH);
    txtGeometryLabelSize->setValue(GEOMETRYLABELSIZE);
}

void PostprocessorView::doScalarFieldDefault()
{
    logMessage("PostprocessorView::doScalarFieldDefault()");

    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) PALETTETYPE));
    chkPaletteFilter->setChecked(PALETTEFILTER);
    txtPaletteSteps->setValue(PALETTESTEPS);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(LINEARIZER_QUALITY));
    chkShowScalarColorBar->setChecked(SCALARCOLORBAR);
    chkScalarFieldRangeLog->setChecked(SCALARFIELDRANGELOG);
    txtScalarFieldRangeBase->setValue(SCALARFIELDRANGEBASE);
    txtScalarDecimalPlace->setValue(SCALARDECIMALPLACE);
}

void PostprocessorView::doContoursVectorsDefault()
{
    logMessage("PostprocessorView::doContoursVectorsDefault()");

    txtContoursCount->setValue(CONTOURSCOUNT);
    chkVectorProportional->setChecked(VECTORPROPORTIONAL);
    chkVectorColor->setChecked(VECTORCOLOR);
    txtVectorCount->setValue(VECTORNUMBER);
    txtVectorScale->setValue(VECTORSCALE);
}

void PostprocessorView::doOrderDefault()
{
    logMessage("PostprocessorView::doPolynomialOrderDefault()");

    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) ORDERPALETTEORDERTYPE));
    chkShowOrderScale->setChecked(ORDERSCALE);
    chkOrderLabel->setChecked(ORDERLABEL);
}

void PostprocessorView::doParticleDefault()
{
    logMessage("PostprocessorView::doParticleDefault()");

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

void PostprocessorView::doAdvancedDefault()
{
    logMessage("PostprocessorView::doAdvancedDefault()");

    chkView3DLighting->setChecked(VIEW3DLIGHTING);
    txtView3DAngle->setValue(VIEW3DANGLE);
    chkView3DBackground->setChecked(VIEW3DBACKGROUND);
    txtView3DHeight->setValue(VIEW3DHEIGHT);

    chkDeformScalar->setChecked(DEFORMSCALAR);
    chkDeformContour->setChecked(DEFORMCONTOUR);
    chkDeformVector->setChecked(DEFORMVECTOR);
}

void PostprocessorView::doScalarFieldRangeMinChanged()
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

void PostprocessorView::doScalarFieldRangeMaxChanged()
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

void PostprocessorView::doSceneFont()
{
    logMessage("PostprocessorView::doSceneFont()");

    bool ok;
    QFont sceneFont = QFontDialog::getFont(&ok, lblSceneFontExample->font(), this);
    if (ok)
    {
        lblSceneFontExample->setFont(sceneFont);
        lblSceneFontExample->setText(QString("%1, %2").arg(lblSceneFontExample->font().family()).arg(lblSceneFontExample->font().pointSize()));
    }
}

void PostprocessorView::doShowGridChanged()
{
    chkSnapToGrid->setEnabled(chkShowGrid->isChecked());
}

void PostprocessorView::doScalarFieldLog(int state)
{
    logMessage("PostprocessorView::doScalarFieldLog()");

    txtScalarFieldRangeBase->setEnabled(chkScalarFieldRangeLog->isChecked());
}
