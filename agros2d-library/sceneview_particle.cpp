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

#include "sceneview_particle.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"
#include "util/loops.h"

#include "gui/lineeditdouble.h"
#include "gui/common.h"

#include "particle/particle_tracing.h"

#include "scene.h"
#include "hermes2d/problem.h"
#include "logview.h"

#include "pythonlab/pythonengine_agros.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem_config.h"

ParticleTracingWidget::ParticleTracingWidget(SceneViewParticleTracing *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewParticleTracing = sceneView;

    setMinimumWidth(160);
    setObjectName("ParticleTracingView");

    createControls();

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(refresh()));

    updateControls();

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(updateControls()));
    connect(currentPythonEngineAgros(), SIGNAL(executedExpression()), this, SLOT(updateControls()));
}

ParticleTracingWidget::~ParticleTracingWidget()
{
}

void ParticleTracingWidget::createControls()
{
    // particle tracing
    cmbParticleButcherTableType = new QComboBox(this);
    foreach (QString key, butcherTableTypeStringKeys())
        cmbParticleButcherTableType->addItem(butcherTableTypeString(butcherTableTypeFromStringKey(key)), butcherTableTypeFromStringKey(key));

    chkParticleIncludeRelativisticCorrection = new QCheckBox(tr("Relativistic correction"));
    txtParticleNumberOfParticles = new QSpinBox(this);
    txtParticleNumberOfParticles->setMinimum(1);
    txtParticleNumberOfParticles->setMaximum(200);
    txtParticleStartingRadius = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartingRadius).toDouble(), true);
    txtParticleStartingRadius->setBottom(0.0);
    txtParticleMass = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMass).toDouble(), true);
    txtParticleMass->setBottom(0.0);
    txtParticleConstant = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleConstant).toDouble(), true);
    lblParticlePointX = new QLabel();
    lblParticlePointY = new QLabel();
    txtParticlePointX = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartX).toDouble(), true);
    txtParticlePointY = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartY).toDouble(), true);
    lblParticleVelocityX = new QLabel();
    lblParticleVelocityY = new QLabel();
    txtParticleVelocityX = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartVelocityX).toDouble(), true);
    txtParticleVelocityY = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartVelocityY).toDouble(), true);
    lblParticleCustomForceX = new QLabel();
    lblParticleCustomForceY = new QLabel();
    lblParticleCustomForceZ = new QLabel();
    txtParticleCustomForceX = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCustomForceX).toDouble(), true);
    txtParticleCustomForceY = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCustomForceY).toDouble(), true);
    txtParticleCustomForceZ = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCustomForceZ).toDouble(), true);
    txtParticleMaximumRelativeError = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMaximumRelativeError).toDouble(), true);
    txtParticleMaximumRelativeError->setBottom(0.0);
    txtParticleMinimumStep = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMinimumStep).toDouble(), true);
    txtParticleMinimumStep->setBottom(0.0);
    chkParticleReflectOnDifferentMaterial = new QCheckBox(tr("Reflection on different material"));
    chkParticleReflectOnBoundary = new QCheckBox(tr("Reflection on boundary"));
    txtParticleCoefficientOfRestitution = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble(), true);
    txtParticleCoefficientOfRestitution->setBottom(0.0);
    txtParticleCoefficientOfRestitution->setTop(1.0);
    chkParticleColorByVelocity = new QCheckBox(tr("Line color is controlled by velocity"));
    chkParticleShowPoints = new QCheckBox(tr("Show points"));
    chkParticleShowBlendedFaces = new QCheckBox(tr("Show blended faces"));
    txtParticleNumShowParticleAxi = new QSpinBox();
    txtParticleNumShowParticleAxi->setMinimum(1);
    txtParticleNumShowParticleAxi->setMaximum(500);
    txtParticleMaximumNumberOfSteps = new QSpinBox();
    txtParticleMaximumNumberOfSteps->setMinimum(10);
    txtParticleMaximumNumberOfSteps->setMaximum(100000);
    txtParticleMaximumNumberOfSteps->setSingleStep(10);
    txtParticleDragDensity = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleDragDensity).toDouble(), true);
    txtParticleDragDensity->setBottom(0.0);
    txtParticleDragCoefficient = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleDragCoefficient).toDouble(), true);
    txtParticleDragCoefficient->setBottom(0.0);
    txtParticleDragReferenceArea = new LineEditDouble(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleDragReferenceArea).toDouble(), true);
    txtParticleDragReferenceArea->setBottom(0.0);
    lblParticleMotionEquations = new QLabel();

    // initial particle position
    QGridLayout *gridLayoutGeneral = new QGridLayout();
    gridLayoutGeneral->addWidget(new QLabel(tr("Equations:")), 0, 0);
    gridLayoutGeneral->addWidget(lblParticleMotionEquations, 1, 0, 1, 2);
    gridLayoutGeneral->addWidget(new QLabel(tr("Number of particles:")), 2, 0);
    gridLayoutGeneral->addWidget(txtParticleNumberOfParticles, 2, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Particles dispersion (m):")), 3, 0);
    gridLayoutGeneral->addWidget(txtParticleStartingRadius, 3, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Mass (kg):")), 4, 0);
    gridLayoutGeneral->addWidget(txtParticleMass, 4, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(gridLayoutGeneral);

    // QPushButton *btnParticleDefault = new QPushButton(tr("Default"));
    // connect(btnParticleDefault, SIGNAL(clicked()), this, SLOT(doParticleDefault()));

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

    // reflection
    QGridLayout *gridLayoutReflection = new QGridLayout();
    gridLayoutReflection->setContentsMargins(5, 5, 0, 0);
    gridLayoutReflection->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutReflection->setColumnStretch(1, 1);
    gridLayoutReflection->addWidget(chkParticleReflectOnDifferentMaterial, 0, 0, 1, 2);
    gridLayoutReflection->addWidget(chkParticleReflectOnBoundary, 1, 0, 1, 2);
    gridLayoutReflection->addWidget(new QLabel(tr("Coefficient of restitution (-):")), 2, 0);
    gridLayoutReflection->addWidget(txtParticleCoefficientOfRestitution, 2, 1);
    gridLayoutReflection->addWidget(new QLabel(""), 10, 0);
    gridLayoutReflection->setRowStretch(10, 1);

    QGroupBox *grpReflection = new QGroupBox(tr("Reflection"));
    grpReflection->setLayout(gridLayoutReflection);

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
    gridLayoutDragForce->setColumnMinimumWidth(0, columnMinimumWidth());
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

    // custom force
    QGridLayout *gridCustomForce = new QGridLayout();
    gridCustomForce->addWidget(lblParticleCustomForceX, 0, 0);
    gridCustomForce->addWidget(txtParticleCustomForceX, 0, 1);
    gridCustomForce->addWidget(lblParticleCustomForceY, 1, 0);
    gridCustomForce->addWidget(txtParticleCustomForceY, 1, 1);
    gridCustomForce->addWidget(lblParticleCustomForceZ, 2, 0);
    gridCustomForce->addWidget(txtParticleCustomForceZ, 2, 1);

    QGroupBox *grpCustomForce = new QGroupBox(tr("Custom force"));
    grpCustomForce->setLayout(gridCustomForce);

    // forces
    QVBoxLayout *layoutForces = new QVBoxLayout();
    layoutForces->setContentsMargins(5, 5, 0, 0);
    layoutForces->addWidget(grpLorentzForce);
    layoutForces->addWidget(grpDragForce);
    layoutForces->addWidget(grpCustomForce);
    layoutForces->addStretch(1);

    QWidget *widgetForces = new QWidget(this);
    widgetForces->setLayout(layoutForces);

    // solver
    QGridLayout *gridLayoutSolver = new QGridLayout();
    gridLayoutSolver->addWidget(new QLabel(tr("Butcher tableau:")), 0, 0);
    gridLayoutSolver->addWidget(cmbParticleButcherTableType, 0, 1);
    gridLayoutSolver->addWidget(chkParticleIncludeRelativisticCorrection, 1, 0);
    gridLayoutSolver->addWidget(new QLabel(QString("<i>m</i><sub>p</sub> = m / (1 - v<sup>2</sup>/c<sup>2</sup>)<sup>1/2</sup>")), 1, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Maximum relative error (%):")), 2, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumRelativeError, 2, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Minimum step (m):")), 3, 0);
    gridLayoutSolver->addWidget(txtParticleMinimumStep, 3, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Maximum number of steps:")), 4, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumNumberOfSteps, 4, 1);
    gridLayoutSolver->addWidget(new QLabel(""), 10, 0);
    gridLayoutSolver->setRowStretch(10, 1);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(gridLayoutSolver);

    // settings
    QGridLayout *gridLayoutSettings = new QGridLayout();
    gridLayoutSettings->setColumnStretch(1, 1);
    gridLayoutSettings->setContentsMargins(5, 5, 0, 0);
    gridLayoutSettings->addWidget(chkParticleColorByVelocity, 2, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowPoints, 3, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowBlendedFaces, 4, 0, 1, 2);
    gridLayoutSettings->addWidget(new QLabel(tr("Show particle multiple times:")), 5, 0);
    gridLayoutSettings->addWidget(txtParticleNumShowParticleAxi, 5, 1);
    //gridLayoutSettings->addWidget(new QLabel(""), 10, 0);
    gridLayoutSettings->setRowStretch(10, 1);

    QGroupBox *grpSettings = new QGroupBox(tr("Settings"));
    grpSettings->setLayout(gridLayoutSettings);

    // tab widget
    QToolBox *tbxWorkspace = new QToolBox();
    tbxWorkspace->addItem(widgetForces, icon(""), tr("Forces"));
    tbxWorkspace->addItem(grpReflection, icon(""), tr("Reflection"));
    tbxWorkspace->addItem(grpSolver, icon(""), tr("Solver"));
    tbxWorkspace->addItem(grpSettings, icon(""), tr("Settings"));

    QVBoxLayout *layoutParticle = new QVBoxLayout();
    // layoutParticle->setContentsMargins(0, 0, 0, 0);
    layoutParticle->addWidget(grpGeneral);
    layoutParticle->addWidget(grpInitialPosition);
    layoutParticle->addWidget(grpInitialVelocity);
    layoutParticle->addWidget(tbxWorkspace);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutParticle);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setContentsMargins(0, 0, 0, 0);
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    // dialog buttons
    QPushButton *btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(widgetArea, 1);
    layoutMain->addWidget(btnOK, 0, Qt::AlignRight);

    setLayout(layoutMain);
}

void ParticleTracingWidget::updateControls()
{
    // particle tracing
    cmbParticleButcherTableType->setCurrentIndex(cmbParticleButcherTableType->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt()));
    chkParticleIncludeRelativisticCorrection->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool());
    txtParticleNumberOfParticles->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt());
    txtParticleStartingRadius->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble());
    txtParticleMass->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMass).toDouble());
    txtParticleConstant->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleConstant).toDouble());
    txtParticlePointX->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble());
    txtParticlePointY->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble());
    txtParticleVelocityX->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble());
    txtParticleVelocityY->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble());
    chkParticleReflectOnDifferentMaterial->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool());
    chkParticleReflectOnBoundary->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnBoundary).toBool());
    txtParticleCoefficientOfRestitution->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble());
    txtParticleCustomForceX->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble());
    txtParticleCustomForceY->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble());
    txtParticleCustomForceZ->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble());
    txtParticleMaximumRelativeError->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble());
    txtParticleMinimumStep->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toDouble());
    txtParticleMaximumNumberOfSteps->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt());
    chkParticleColorByVelocity->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleColorByVelocity).toBool());
    chkParticleShowPoints->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowPoints).toBool());
    chkParticleShowBlendedFaces->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowBlendedFaces).toBool());
    txtParticleNumShowParticleAxi->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumShowParticlesAxi).toInt());
    txtParticleNumShowParticleAxi->setEnabled(Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric);
    txtParticleDragDensity->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble());
    txtParticleDragReferenceArea->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble());
    txtParticleDragCoefficient->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble());

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
        lblParticleMotionEquations->setText(QString("<i>r</i>\" = <i>F</i><sub>r</sub> / <i>m</i> + <i>r</i> (<i>&phi;</i>')<sup>2</sup>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i><br /><i>&phi;</i>\" = <i>F</i><sub>&phi;</sub> / <i>m</i> - 2<i>r</i> <i>r</i>' <i>&phi;</i>' / <i>r</i>"));
}

void ParticleTracingWidget::doParticleDefault()
{
    cmbParticleButcherTableType->setCurrentIndex(cmbParticleButcherTableType->findData(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleButcherTableType).toInt()));
    txtParticleNumberOfParticles->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleNumberOfParticles).toInt());
    txtParticleStartingRadius->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartingRadius).toDouble());
    chkParticleIncludeRelativisticCorrection->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool());
    txtParticleMass->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMass).toDouble());
    txtParticleConstant->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleConstant).toDouble());
    txtParticlePointX->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartX).toDouble());
    txtParticlePointY->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartY).toDouble());
    txtParticleVelocityX->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartVelocityX).toDouble());
    txtParticleVelocityY->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleStartVelocityY).toDouble());
    chkParticleReflectOnDifferentMaterial->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool());
    chkParticleReflectOnBoundary->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleReflectOnBoundary).toBool());
    txtParticleCoefficientOfRestitution->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble());
    txtParticleCustomForceX->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCustomForceX).toDouble());
    txtParticleCustomForceY->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCustomForceY).toDouble());
    txtParticleCustomForceZ->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleCustomForceZ).toDouble());
    txtParticleMaximumRelativeError->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMaximumRelativeError).toDouble());
    txtParticleMinimumStep->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMinimumStep).toDouble());
    txtParticleMaximumNumberOfSteps->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt());
    chkParticleColorByVelocity->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleColorByVelocity).toBool());
    chkParticleShowPoints->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleShowPoints).toBool());
    chkParticleShowBlendedFaces->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleShowBlendedFaces).toBool());
    txtParticleNumShowParticleAxi->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleNumShowParticlesAxi).toInt());
    txtParticleDragDensity->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleDragDensity).toDouble());
    txtParticleDragReferenceArea->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleDragReferenceArea).toDouble());
    txtParticleDragCoefficient->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ParticleDragCoefficient).toDouble());
}

void ParticleTracingWidget::refresh()
{

}

void ParticleTracingWidget::doApply()
{
    // particle tracing
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleButcherTableType, (Hermes::ButcherTableType) cmbParticleButcherTableType->itemData(cmbParticleButcherTableType->currentIndex()).toInt());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleIncludeRelativisticCorrection, chkParticleIncludeRelativisticCorrection->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleNumberOfParticles, txtParticleNumberOfParticles->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartingRadius, txtParticleStartingRadius->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMass, txtParticleMass->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleConstant, txtParticleConstant->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartX, txtParticlePointX->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartY, txtParticlePointY->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityX, txtParticleVelocityX->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityY, txtParticleVelocityY->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleReflectOnDifferentMaterial, chkParticleReflectOnDifferentMaterial->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleReflectOnBoundary, chkParticleReflectOnBoundary->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCoefficientOfRestitution, txtParticleCoefficientOfRestitution->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceX, txtParticleCustomForceX->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceY, txtParticleCustomForceY->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceZ, txtParticleCustomForceZ->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMaximumRelativeError, txtParticleMaximumRelativeError->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMinimumStep, txtParticleMinimumStep->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMaximumNumberOfSteps, txtParticleMaximumNumberOfSteps->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleColorByVelocity, chkParticleColorByVelocity->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleShowPoints, chkParticleShowPoints->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleShowBlendedFaces, chkParticleShowBlendedFaces->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleNumShowParticlesAxi, txtParticleNumShowParticleAxi->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragDensity, txtParticleDragDensity->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragCoefficient, txtParticleDragCoefficient->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragReferenceArea, txtParticleDragReferenceArea->value());

    m_sceneViewParticleTracing->processParticleTracing();
}

// *************************************************************************************************

SceneViewParticleTracing::SceneViewParticleTracing(PostHermes *postHermes, QWidget *parent)
    : SceneViewCommon3D(postHermes, parent),
      m_listParticleTracing(-1)
{
    createActionsParticleTracing();

    connect(Agros2D::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postHermes, SIGNAL(processed()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));
}

SceneViewParticleTracing::~SceneViewParticleTracing()
{
}

void SceneViewParticleTracing::createActionsParticleTracing()
{
    actSceneModeParticleTracing = new QAction(iconView(), tr("Particle\nTracing"), this);
    actSceneModeParticleTracing->setShortcut(tr("Ctrl+7"));
    actSceneModeParticleTracing->setCheckable(true);
}

void SceneViewParticleTracing::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon3D::mousePressEvent(event);
}

void SceneViewParticleTracing::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundRed).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundGreen).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundBlue).toInt() / 255.0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // gradient background
    paintBackground();

    if (Agros2D::problem()->isSolved())
    {
        // todo: what is better?
        //paintGeometrySurface(Agros2D::problem()->configView()->particleShowBlendedFaces);
        if(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowBlendedFaces).toBool())
            paintGeometrySurface(true);

        paintGeometryOutline();
        paintParticleTracing();

        // bars
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleColorByVelocity).toInt())
            paintParticleTracingColorBar(m_velocityMin, m_velocityMax);
    }

    emit labelCenter(tr("Particle tracing"));

    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool()) paintAxes();
}

void SceneViewParticleTracing::resizeGL(int w, int h)
{
    SceneViewCommon::resizeGL(w, h);
}


void SceneViewParticleTracing::paintGeometryOutline()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!particleTracingIsPrepared()) return;

    loadProjection3d(true, false);

    RectPoint rect = Agros2D::scene()->boundingBox();
    double max = qMax(rect.width(), rect.height());
    double depth = max / Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble();

    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPopMatrix();

    // geometry
    glDisable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    glColor3d(0.0, 0.0, 0.0);
    glLineWidth(1.3);

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
    {
        // depth
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            glBegin(GL_LINES);
            if (edge->isStraight())
            {
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);

                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
            }
            glEnd();
        }

        // length
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            glBegin(GL_LINES);
            if (edge->isStraight())
            {
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);

                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                          center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                int segments = edge->angle() / 5.0;
                if (segments < 2) segments = 2;

                double theta = edge->angle() / double(segments);

                for (int i = 0; i < segments; i++)
                {
                    double arc1 = (startAngle + i*theta)/180.0*M_PI;
                    double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                    double x1 = radius * cos(arc1);
                    double y1 = radius * sin(arc1);
                    double x2 = radius * cos(arc2);
                    double y2 = radius * sin(arc2);

                    glVertex3d(center.x + x1, center.y + y1, depth/2.0);
                    glVertex3d(center.x + x2, center.y + y2, depth/2.0);

                    glVertex3d(center.x + x1, center.y + y1, -depth/2.0);
                    glVertex3d(center.x + x2, center.y + y2, -depth/2.0);
                }
            }
            glEnd();
        }
    }
    else
    {
        // top
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            for (int j = 0; j <= 360; j = j + 90)
            {
                if (edge->isStraight())
                {
                    glBegin(GL_LINES);
                    glVertex3d(edge->nodeStart()->point().x * cos(j/180.0*M_PI),
                               edge->nodeStart()->point().y,
                               edge->nodeStart()->point().x * sin(j/180.0*M_PI));
                    glVertex3d(edge->nodeEnd()->point().x * cos(j/180.0*M_PI),
                               edge->nodeEnd()->point().y,
                               edge->nodeEnd()->point().x * sin(j/180.0*M_PI));
                    glEnd();
                }
                else
                {
                    Point center = edge->center();
                    double radius = edge->radius();
                    double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                    double theta = edge->angle() / double(edge->angle()/2 - 1);

                    glBegin(GL_LINE_STRIP);
                    for (int i = 0; i < edge->angle()/2; i++)
                    {
                        double arc = (startAngle + i*theta)/180.0*M_PI;

                        double x = radius * cos(arc);
                        double y = radius * sin(arc);

                        glVertex3d((center.x + x) * cos(j/180.0*M_PI),
                                   center.y + y,
                                   (center.x + x) * sin(j/180.0*M_PI));
                    }
                    glEnd();
                }
            }
        }

        // side
        foreach (SceneNode *node, Agros2D::scene()->nodes->items())
        {
            int count = 29.0;
            double step = 360.0/count;

            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < count; j++)
            {
                glVertex3d(node->point().x * cos((j+0)*step/180.0*M_PI),
                           node->point().y,
                           node->point().x * sin((j+0)*step/180.0*M_PI));
                glVertex3d(node->point().x * cos((j+1)*step/180.0*M_PI),
                           node->point().y,
                           node->point().x * sin((j+1)*step/180.0*M_PI));
            }
            glEnd();
        }
    }

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_DEPTH_TEST);
}

void SceneViewParticleTracing::paintGeometrySurface(bool blend)
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!particleTracingIsPrepared()) return;

    loadProjection3d(true, false);

    RectPoint rect = Agros2D::scene()->boundingBox();
    double max = qMax(rect.width(), rect.height());
    double depth = max / Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble();

    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);

    if (blend)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4d(0.2, 0.4, 0.1, 0.3);
    }
    else
    {
        glColor3d(0.2, 0.4, 0.1);
    }

    // surfaces
    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
    {
        glBegin(GL_TRIANGLES);
        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(Agros2D::scene()->loopsInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();
            if (i.key()->isHole())
                continue;

            foreach (LoopsInfo::Triangle triangle, i.value())
            {
                // z = - depth / 2.0
                glVertex3d(triangle.a.x, triangle.a.y, -depth/2.0);
                glVertex3d(triangle.b.x, triangle.b.y, -depth/2.0);
                glVertex3d(triangle.c.x, triangle.c.y, -depth/2.0);

                // z = + depth / 2.0
                glVertex3d(triangle.a.x, triangle.a.y, depth/2.0);
                glVertex3d(triangle.c.x, triangle.c.y, depth/2.0);
                glVertex3d(triangle.b.x, triangle.b.y, depth/2.0);
            }
        }
        glEnd();

        // length
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            glBegin(GL_TRIANGLE_STRIP);
            if (edge->isStraight())
            {
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);

                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                          center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                int segments = edge->angle() / 5.0;
                if (segments < 2) segments = 2;

                double theta = edge->angle() / double(segments);

                for (int i = 0; i < segments + 1; i++)
                {
                    double arc = (startAngle + i*theta)/180.0*M_PI;

                    double x = radius * cos(arc);
                    double y = radius * sin(arc);

                    glVertex3d(center.x + x, center.y + y, -depth/2.0);
                    glVertex3d(center.x + x, center.y + y, depth/2.0);
                }
            }
            glEnd();
        }
    }
    else
    {
        glBegin(GL_TRIANGLES);
        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(Agros2D::scene()->loopsInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();
            if (i.key()->isHole())
                continue;

            foreach (LoopsInfo::Triangle triangle, i.value())
            {
                for (int j = 0; j <= 360; j = j + 90)
                {
                    glVertex3d(triangle.a.x * cos(j/180.0*M_PI), triangle.a.y, triangle.a.x * sin(j/180.0*M_PI));
                    glVertex3d(triangle.b.x * cos(j/180.0*M_PI), triangle.b.y, triangle.b.x * sin(j/180.0*M_PI));
                    glVertex3d(triangle.c.x * cos(j/180.0*M_PI), triangle.c.y, triangle.c.x * sin(j/180.0*M_PI));
                }
            }
        }
        glEnd();

        // length
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            int count = 29.0;
            double step = 360.0/count;

            glBegin(GL_TRIANGLE_STRIP);
            if (edge->isStraight())
            {
                for (int j = 0; j < count + 1; j++)
                {
                    glVertex3d(edge->nodeStart()->point().x * cos((j+0)*step/180.0*M_PI), edge->nodeStart()->point().y, edge->nodeStart()->point().x * sin((j+0)*step/180.0*M_PI));
                    glVertex3d(edge->nodeEnd()->point().x * cos((j+0)*step/180.0*M_PI), edge->nodeEnd()->point().y, edge->nodeEnd()->point().x * sin((j+0)*step/180.0*M_PI));
                }
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                          center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                int segments = edge->angle() / 5.0;
                if (segments < 2) segments = 2;

                double theta = edge->angle() / double(segments);

                for (int i = 0; i < segments; i++)
                {
                    double arc1 = (startAngle + i*theta)/180.0*M_PI;
                    double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                    double x1 = radius * cos(arc1);
                    double y1 = radius * sin(arc1);
                    double x2 = radius * cos(arc2);
                    double y2 = radius * sin(arc2);

                    for (int j = 0; j < count + 1; j++)
                    {
                        glVertex3d((center.x + x1) * cos((j+0)*step/180.0*M_PI), (center.y + y1), (center.x + x1) * sin((j+0)*step/180.0*M_PI));
                        glVertex3d((center.x + x2) * cos((j+0)*step/180.0*M_PI), (center.y + y2), (center.x + x2) * sin((j+0)*step/180.0*M_PI));
                    }
                }
            }
            glEnd();
        }
    }

    if (blend)
    {
        glDisable(GL_BLEND);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneViewParticleTracing::paintParticleTracing()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!particleTracingIsPrepared()) return;

    loadProjection3d(true, false);

    if (m_listParticleTracing == -1)
    {
        m_listParticleTracing = glGenLists(1);
        glNewList(m_listParticleTracing, GL_COMPILE);

        RectPoint rect = Agros2D::scene()->boundingBox();
        double max = qMax(rect.width(), rect.height());
        double depth = max / Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble();

        glPushMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPopMatrix();

        double velocityMin = m_velocityMin;
        double velocityMax = m_velocityMax;

        double positionMin = m_positionMin;
        double positionMax = m_positionMax;

        if ((positionMax - positionMin) < EPS_ZERO)
        {
            positionMin = -1.0;
            positionMax = +1.0;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);

        // particle visualization
        for (int k = 0; k < Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt(); k++)
        {
            // starting point
            /*
            glPointSize(Agros2D::problem()->configView()->value(ProblemConfigView::View_NodeSize).toInt() * 1.2);
            glColor3d(0.0, 0.0, 0.0);
            glBegin(GL_POINTS);
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                glVertex3d(m_positionsList[k][0].x, m_positionsList[k][0].y, -depth/2.0 + (m_positionsList[k][0].z - positionMin) * depth/(positionMax - positionMin));
            else
                glVertex3d(m_positionsList[k][0].x * cos(m_positionsList[k][0].z), m_positionsList[k][0].y, m_positionsList[k][0].x * sin(m_positionsList[k][0].z));
            glEnd();
            */

            // lines
            glLineWidth(3.0);

            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
            {
                glColor3d(rand() / double(RAND_MAX),
                          rand() / double(RAND_MAX),
                          rand() / double(RAND_MAX));

                glBegin(GL_LINES);
                for (int i = 0; i < m_positionsList[k].length() - 1; i++)
                {
                    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleColorByVelocity).toBool())
                        glColor3d(1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                  1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                  1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin));

                    glVertex3d(m_positionsList[k][i].x,
                               m_positionsList[k][i].y,
                               0.0);
                    glVertex3d(m_positionsList[k][i+1].x,
                            m_positionsList[k][i+1].y,
                            0.0);
                }
                glEnd();

                // points
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowPoints).toBool())
                {
                    glPointSize(Agros2D::problem()->setting()->value(ProblemSetting::View_NodeSize).toInt() * 3.0/5.0);

                    glBegin(GL_POINTS);
                    for (int i = 0; i < m_positionsList[k].length(); i++)
                    {
                        glVertex3d(m_positionsList[k][i].x,
                                   m_positionsList[k][i].y,
                                   0.0);
                    }
                    glEnd();
                }
            }
            else
            {
                double stepAngle = 360.0 / Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumShowParticlesAxi).toInt();

                for (int l = 0; l < Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumShowParticlesAxi).toInt(); l++)
                {
                    glColor3d(rand() / double(RAND_MAX),
                              rand() / double(RAND_MAX),
                              rand() / double(RAND_MAX));

                    glBegin(GL_LINES);
                    for (int i = 0; i < m_positionsList[k].length() - 1; i++)
                    {
                        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleColorByVelocity).toBool())
                            glColor3d(1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                      1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                      1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin));

                        glVertex3d(m_positionsList[k][i].x * cos(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI),
                                   m_positionsList[k][i].y,
                                   m_positionsList[k][i].x * sin(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI));
                        glVertex3d(m_positionsList[k][i+1].x * cos(m_positionsList[k][i+1].z + l * stepAngle/180.0 * M_PI),
                                m_positionsList[k][i+1].y,
                                m_positionsList[k][i+1].x * sin(m_positionsList[k][i+1].z + l * stepAngle/180.0 * M_PI));

                    }
                    glEnd();

                    // points
                    glBegin(GL_POINTS);
                    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowPoints).toBool())
                    {
                        glPointSize(Agros2D::problem()->setting()->value(ProblemSetting::View_NodeSize).toInt() * 3.0/5.0);
                        for (int i = 0; i < m_positionsList[k].length(); i++)
                        {
                            glVertex3d(m_positionsList[k][i].x * cos(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI),
                                       m_positionsList[k][i].y,
                                       m_positionsList[k][i].x * sin(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI));
                        }
                    }
                    glEnd();
                }
            }
        }

        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_DEPTH_TEST);

        glEndList();

        glCallList(m_listParticleTracing);
    }
    else
    {
        glCallList(m_listParticleTracing);
    }
}

void SceneViewParticleTracing::paintParticleTracingColorBar(double min, double max)
{
    if (!Agros2D::problem()->isSolved()) return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) * (QString::number(-1.0, '+e', Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt()).length() + 1);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // contextHeight() - 20.0
    Point scaleBorder = Point(10.0, (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glBegin(GL_QUADS);
    glColor3d(0.0, 0.0, 0.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glColor3d(0.8, 0.8, 0.8);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    // ticks
    glColor3d(0.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = min + (double) (i-1) / (numTicks-1) * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) : 0.0),
                    scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                    QString::number(value, '+e', Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt()));
    }

    // variable
    QString str = QString("%1 (m/s)").arg(tr("Vel."));

    printPostAt(scaleLeft + scaleSize.x / 2.0 - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0)  * str.count() / 2.0,
                scaleBorder.y + scaleSize.y - 20.0,
                str);
}

void SceneViewParticleTracing::clearGLLists()
{
    if (m_listParticleTracing != -1) glDeleteLists(m_listParticleTracing, 1);

    m_listParticleTracing = -1;
}

void SceneViewParticleTracing::refresh()
{
    clearGLLists();

    setControls();

    SceneViewCommon::refresh();
}

void SceneViewParticleTracing::setControls()
{
    actSceneModeParticleTracing->setEnabled(Agros2D::problem()->isSolved());
    actSetProjectionXY->setEnabled(Agros2D::problem()->isSolved());
    actSetProjectionXZ->setEnabled(Agros2D::problem()->isSolved());
    actSetProjectionYZ->setEnabled(Agros2D::problem()->isSolved());
}

void SceneViewParticleTracing::clear()
{
    clearParticleLists();

    SceneViewCommon3D::clear();
}

void SceneViewParticleTracing::clearParticleLists()
{
    // clear lists    
    foreach (QList<Point3> list, m_positionsList)
        list.clear();
    m_positionsList.clear();

    foreach (QList<Point3> list, m_velocitiesList)
        list.clear();
    m_velocitiesList.clear();

    foreach (QList<double> list, m_timesList)
        list.clear();
    m_timesList.clear();

    m_velocityMin = 0.0;
    m_velocityMax = 0.0;
}

void SceneViewParticleTracing::processParticleTracing()
{
    QTime cpuTime;
    cpuTime.start();

    clearParticleLists();

    if (Agros2D::problem()->isSolved())
    {
        Agros2D::log()->printMessage(tr("Post View"), tr("Particle view"));

        m_velocityMin =  numeric_limits<double>::max();
        m_velocityMax = -numeric_limits<double>::max();

        for (int k = 0; k < Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt(); k++)
        {
            // position and velocity cache
            ParticleTracing particleTracing;
            try
            {
                // initial position
                Point3 initialPosition;
                initialPosition.x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble();
                initialPosition.y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble();
                initialPosition.z = 0.0;

                // initial velocity
                Point3 initialVelocity;
                initialVelocity.x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble();
                initialVelocity.y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble();
                initialVelocity.z = 0.0;

                // random point
                if (k > 0)
                {
                    Point3 dp(rand() * (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble()) / RAND_MAX,
                              rand() * (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble()) / RAND_MAX,
                              (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? 0.0 : rand() * 2.0*M_PI / RAND_MAX);

                    initialPosition = Point3(-Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble() / 2,
                                             -Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble() / 2,
                                             (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? 0.0 : -1.0*M_PI) + initialPosition + dp;
                }

                particleTracing.computeTrajectoryParticle(initialPosition, initialVelocity);
            }
            catch (AgrosException& e)
            {
                Agros2D::log()->printWarning(tr("Particle tracing"), tr("Particle tracing failed (%1)").append(e.what()));
                m_velocityMin = 0.0;
                m_velocityMax = 0.0;

                return;
            }
            catch (...)
            {
                Agros2D::log()->printWarning(tr("Particle tracing"), tr("Catched unknown exception in particle tracing"));
                m_velocityMin = 0.0;
                m_velocityMax = 0.0;

                return;
            }

            m_positionsList.append(particleTracing.positions());
            m_velocitiesList.append(particleTracing.velocities());
            m_timesList.append(particleTracing.times());

            // velocity min and max value
            if (particleTracing.velocityMin() < m_velocityMin) m_velocityMin = particleTracing.velocityMin();
            if (particleTracing.velocityMax() > m_velocityMax) m_velocityMax = particleTracing.velocityMax();

            Agros2D::log()->printMessage(tr("Particle Tracing"), tr("Particle %1: %2 steps, final time %3 s").
                                         arg(k + 1).
                                         arg(particleTracing.times().count()).
                                         arg(particleTracing.times().last()));
        }
    }
    Agros2D::log()->printDebug(tr("Particle Tracing"), tr("Total cpu time %1 ms").arg(cpuTime.elapsed()));

    refresh();
}
