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

#include "settings.h"

#include "util/constants.h"
#include "util/glfont.h"
#include "util/global.h"

#include "gui/common.h"
#include "gui/lineeditdouble.h"

#include "hermes2d/problem_config.h"

#include "scene.h"
#include "pythonlab/pythonengine_agros.h"

const double minWidth = 110;

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    setWindowIcon(icon("options"));
    setObjectName("Settings");

    createActions();
    createControls();

    load();

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(updateControls()));
    connect(currentPythonEngineAgros(), SIGNAL(executedExpression()), this, SLOT(updateControls()));
}

void SettingsWidget::createActions()
{
    actSettings = new QAction(icon("options"), tr("Settings"), this);
    actSettings->setShortcut(tr("Ctrl+7"));
    actSettings->setCheckable(true);
}

void SettingsWidget::load()
{
    // workspace
    txtGridStep->setText(QString::number(Agros2D::problem()->configView()->gridStep));
    chkShowGrid->setChecked(Agros2D::problem()->configView()->showGrid);
    chkSnapToGrid->setChecked(Agros2D::problem()->configView()->snapToGrid);

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(Agros2D::problem()->configView()->rulersFont));
    if (cmbRulersFont->currentIndex() == -1)
        cmbRulersFont->setCurrentIndex(0);
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(Agros2D::problem()->configView()->postFont));
    if (cmbPostFont->currentIndex() == -1)
        cmbPostFont->setCurrentIndex(0);

    chkShowAxes->setChecked(Agros2D::problem()->configView()->showAxes);
    chkShowRulers->setChecked(Agros2D::problem()->configView()->showRulers);

    chkZoomToMouse->setChecked(Agros2D::problem()->configView()->zoomToMouse);
    txtGeometryNodeSize->setValue(Agros2D::problem()->configView()->nodeSize);
    txtGeometryEdgeWidth->setValue(Agros2D::problem()->configView()->edgeWidth);
    txtGeometryLabelSize->setValue(Agros2D::problem()->configView()->labelSize);

    // script and description
    txtDescription->setPlainText(Agros2D::problem()->configView()->description);
    txtStartupScript->setPlainText(Agros2D::problem()->configView()->startupScript);

    // 3d
    chkView3DLighting->setChecked(Agros2D::problem()->configView()->scalarView3DLighting);
    txtView3DAngle->setValue(Agros2D::problem()->configView()->scalarView3DAngle);
    chkView3DBackground->setChecked(Agros2D::problem()->configView()->scalarView3DBackground);
    txtView3DHeight->setValue(Agros2D::problem()->configView()->scalarView3DHeight);
    chkView3DBoundingBox->setChecked(Agros2D::problem()->configView()->scalarView3DBoundingBox);
    chkView3DSolidGeometry->setChecked(Agros2D::problem()->configView()->scalarView3DSolidGeometry);

    // deform shape
    chkDeformScalar->setChecked(Agros2D::problem()->configView()->deformScalar);
    chkDeformContour->setChecked(Agros2D::problem()->configView()->deformContour);
    chkDeformVector->setChecked(Agros2D::problem()->configView()->deformVector);

    // colors
    colorBackground->setColor(Agros2D::problem()->configView()->colorBackground);
    colorGrid->setColor(Agros2D::problem()->configView()->colorGrid);
    colorCross->setColor(Agros2D::problem()->configView()->colorCross);
    colorNodes->setColor(Agros2D::problem()->configView()->colorNodes);
    colorEdges->setColor(Agros2D::problem()->configView()->colorEdges);
    colorLabels->setColor(Agros2D::problem()->configView()->colorLabels);
    colorContours->setColor(Agros2D::problem()->configView()->colorContours);
    colorVectors->setColor(Agros2D::problem()->configView()->colorVectors);
    colorInitialMesh->setColor(Agros2D::problem()->configView()->colorInitialMesh);
    colorSolutionMesh->setColor(Agros2D::problem()->configView()->colorSolutionMesh);
    colorHighlighted->setColor(Agros2D::problem()->configView()->colorHighlighted);
    colorSelected->setColor(Agros2D::problem()->configView()->colorSelected);

    // mesh and solver
    txtMeshAngleSegmentsCount->setValue(Agros2D::problem()->configView()->angleSegmentsCount);
    chkMeshCurvilinearElements->setChecked(Agros2D::problem()->configView()->curvilinearElements);

    // adaptivity
    txtMaxDOFs->setValue(Agros2D::problem()->configView()->maxDofs);
    txtConvExp->setValue(Agros2D::problem()->configView()->convExp);
    txtThreshold->setValue(Agros2D::problem()->configView()->threshold);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(Agros2D::problem()->configView()->strategy));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(Agros2D::problem()->configView()->meshRegularity));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(Agros2D::problem()->configView()->projNormType));
    chkUseAnIso->setChecked(Agros2D::problem()->configView()->useAniso);
    chkFinerReference->setChecked(Agros2D::problem()->configView()->finerReference);

    // command argument
    txtArgumentTriangle->setText(Agros2D::problem()->configView()->commandTriangle);
    txtArgumentGmsh->setText(Agros2D::problem()->configView()->commandGmsh);
}

void SettingsWidget::save()
{
    // run and check startup script
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        currentPythonEngineAgros()->blockSignals(true);
        ScriptResult scriptResult = currentPythonEngineAgros()->runScript(txtStartupScript->toPlainText());
        currentPythonEngineAgros()->blockSignals(false);
        if (scriptResult.isError)
            return;
    }

    // workspace
    Agros2D::problem()->configView()->showGrid = chkShowGrid->isChecked();
    Agros2D::problem()->configView()->gridStep = txtGridStep->text().toDouble();
    Agros2D::problem()->configView()->showRulers = chkShowRulers->isChecked();
    Agros2D::problem()->configView()->zoomToMouse = chkZoomToMouse->isChecked();
    Agros2D::problem()->configView()->snapToGrid = chkSnapToGrid->isChecked();

    Agros2D::problem()->configView()->rulersFont = cmbRulersFont->itemData(cmbRulersFont->currentIndex()).toString();
    Agros2D::problem()->configView()->postFont = cmbPostFont->itemData(cmbPostFont->currentIndex()).toString();

    Agros2D::problem()->configView()->showAxes = chkShowAxes->isChecked();
    Agros2D::problem()->configView()->showRulers = chkShowRulers->isChecked();

    Agros2D::problem()->configView()->nodeSize = txtGeometryNodeSize->value();
    Agros2D::problem()->configView()->edgeWidth = txtGeometryEdgeWidth->value();
    Agros2D::problem()->configView()->labelSize = txtGeometryLabelSize->value();

    // script and description
    Agros2D::problem()->configView()->description = txtDescription->toPlainText();
    Agros2D::problem()->configView()->startupScript = txtStartupScript->toPlainText();

    // 3d
    Agros2D::problem()->configView()->scalarView3DLighting = chkView3DLighting->isChecked();
    Agros2D::problem()->configView()->scalarView3DAngle = txtView3DAngle->value();
    Agros2D::problem()->configView()->scalarView3DBackground = chkView3DBackground->isChecked();
    Agros2D::problem()->configView()->scalarView3DHeight = txtView3DHeight->value();
    Agros2D::problem()->configView()->scalarView3DBoundingBox = chkView3DBoundingBox->isChecked();
    Agros2D::problem()->configView()->scalarView3DSolidGeometry = chkView3DSolidGeometry->isChecked();

    // deform shape
    Agros2D::problem()->configView()->deformScalar = chkDeformScalar->isChecked();
    Agros2D::problem()->configView()->deformContour = chkDeformContour->isChecked();
    Agros2D::problem()->configView()->deformVector = chkDeformVector->isChecked();

    // color
    Agros2D::problem()->configView()->colorBackground = colorBackground->color();
    Agros2D::problem()->configView()->colorGrid = colorGrid->color();
    Agros2D::problem()->configView()->colorCross = colorCross->color();
    Agros2D::problem()->configView()->colorNodes = colorNodes->color();
    Agros2D::problem()->configView()->colorEdges = colorEdges->color();
    Agros2D::problem()->configView()->colorLabels = colorLabels->color();
    Agros2D::problem()->configView()->colorContours = colorContours->color();
    Agros2D::problem()->configView()->colorVectors = colorVectors->color();
    Agros2D::problem()->configView()->colorInitialMesh = colorInitialMesh->color();
    Agros2D::problem()->configView()->colorSolutionMesh = colorSolutionMesh->color();
    Agros2D::problem()->configView()->colorHighlighted = colorHighlighted->color();
    Agros2D::problem()->configView()->colorSelected = colorSelected->color();

    // mesh and solver
    Agros2D::problem()->configView()->angleSegmentsCount = txtMeshAngleSegmentsCount->value();
    Agros2D::problem()->configView()->curvilinearElements = chkMeshCurvilinearElements->isChecked();

    // adaptivity
    Agros2D::problem()->configView()->maxDofs = txtMaxDOFs->value();
    Agros2D::problem()->configView()->convExp = txtConvExp->value();
    Agros2D::problem()->configView()->threshold = txtThreshold->value();
    Agros2D::problem()->configView()->strategy = cmbStrategy->itemData(cmbStrategy->currentIndex()).toInt();
    Agros2D::problem()->configView()->meshRegularity = cmbMeshRegularity->itemData(cmbMeshRegularity->currentIndex()).toInt();
    Agros2D::problem()->configView()->projNormType = (Hermes::Hermes2D::ProjNormType) cmbProjNormType->itemData(cmbProjNormType->currentIndex()).toInt();
    Agros2D::problem()->configView()->useAniso = chkUseAnIso->isChecked();
    Agros2D::problem()->configView()->finerReference = chkFinerReference->isChecked();

    // command argument
    Agros2D::problem()->configView()->commandTriangle = txtArgumentTriangle->text();
    Agros2D::problem()->configView()->commandGmsh = txtArgumentGmsh->text();
}

void SettingsWidget::createControls()
{
    QWidget *workspace = controlsWorkspace();
    QWidget *colors = controlsColors();
    QWidget *meshAndSolver = controlsMeshAndSolver();
    QWidget *descriptionAndScript = controlsControlsScriptAndDescription();

    // tab widget
    QToolBox *tbxWorkspace = new QToolBox();
    tbxWorkspace->addItem(workspace, icon(""), tr("Workspace"));
    tbxWorkspace->addItem(descriptionAndScript, icon(""), tr("Script and description"));
    tbxWorkspace->addItem(colors, icon(""), tr("Colors"));
    tbxWorkspace->addItem(meshAndSolver, icon(""), tr("Mesh and Solver"));

    // layout workspace
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tbxWorkspace);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnOK);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 5, 3, 5);
    layoutMain->addWidget(widget);
    layoutMain->addLayout(layoutButtons);

    setControls();

    setLayout(layoutMain);
}

QWidget *SettingsWidget::controlsControlsScriptAndDescription()
{
    // startup script
    txtStartupScript = new ScriptEditor(currentPythonEngine(), this);   
    connect(txtStartupScript, SIGNAL(textChanged()), this, SLOT(doStartupScriptChanged()));
    lblStartupScriptError = new QLabel();

    QPalette palette = lblStartupScriptError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblStartupScriptError->setPalette(palette);

    QVBoxLayout *layoutStartup = new QVBoxLayout();
    layoutStartup->addWidget(txtStartupScript);
    layoutStartup->addWidget(lblStartupScriptError);

    QGroupBox *grpStartup = new QGroupBox(tr("Startup script"));
    grpStartup->setLayout(layoutStartup);

    // description
    txtDescription = new QTextEdit(this);
    txtDescription->setAcceptRichText(false);

    QVBoxLayout *layoutDescription = new QVBoxLayout();
    layoutDescription->addWidget(txtDescription);

    QGroupBox *grpDescription = new QGroupBox(tr("Description"));
    grpDescription->setLayout(layoutDescription);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpStartup, 2);
    layout->addWidget(grpDescription, 1);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *SettingsWidget::controlsWorkspace()
{
    // workspace
    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkShowGrid = new QCheckBox(tr("Show grid"));
    connect(chkShowGrid, SIGNAL(clicked()), this, SLOT(doShowGridChanged()));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));
    chkZoomToMouse = new QCheckBox(tr("Zoom to mouse pointer"));   
    chkShowRulers = new QCheckBox(tr("Show rulers"));
    chkShowAxes = new QCheckBox(tr("Show axes"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(new QLabel(tr("Grid step:")), 0, 0);
    layoutGrid->addWidget(txtGridStep, 0, 1);
    layoutGrid->addWidget(chkShowGrid, 1, 0);
    layoutGrid->addWidget(chkShowAxes, 2, 0);
    layoutGrid->addWidget(chkShowRulers, 3, 0);
    layoutGrid->addWidget(chkSnapToGrid, 1, 1);
    layoutGrid->addWidget(chkZoomToMouse, 2, 1);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    cmbRulersFont = new QComboBox();
    fillFontsComboBox(cmbRulersFont);
    cmbPostFont = new QComboBox();
    fillFontsComboBox(cmbPostFont);

    QGridLayout *layoutFont = new QGridLayout();
    layoutFont->addWidget(new QLabel(tr("Rulers:")), 0, 0);
    layoutFont->addWidget(cmbRulersFont, 0, 1);
    layoutFont->addWidget(new QLabel(tr("Postprocessor:")), 1, 0);
    layoutFont->addWidget(cmbPostFont, 1, 1);

    QGroupBox *grpFont = new QGroupBox(tr("Fonts"));
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
    chkView3DBoundingBox = new QCheckBox(tr("Bounding box"), this);
    chkView3DSolidGeometry = new QCheckBox(tr("Show geometry lines"), this);

    QGridLayout *layout3D = new QGridLayout();
    layout3D->addWidget(new QLabel(tr("Angle:")), 0, 1);
    layout3D->addWidget(txtView3DAngle, 0, 2);
    layout3D->addWidget(chkView3DLighting, 0, 3);
    layout3D->addWidget(new QLabel(tr("Height:")), 1, 1);
    layout3D->addWidget(txtView3DHeight, 1, 2);
    layout3D->addWidget(chkView3DBackground, 1, 3);
    layout3D->addWidget(chkView3DBoundingBox, 2, 3);
    layout3D->addWidget(chkView3DSolidGeometry, 3, 3);

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

    QPushButton *btnWorkspaceDefault = new QPushButton(tr("Default"));
    connect(btnWorkspaceDefault, SIGNAL(clicked()), this, SLOT(doWorkspaceDefault()));

    QVBoxLayout *layoutWorkspace = new QVBoxLayout();
    layoutWorkspace->addWidget(grpGrid);
    layoutWorkspace->addWidget(grpFont);
    layoutWorkspace->addWidget(grpGeometry);
    layoutWorkspace->addWidget(grp3D);
    layoutWorkspace->addWidget(grpDeformShape);
    layoutWorkspace->addStretch();
    layoutWorkspace->addWidget(btnWorkspaceDefault, 0, Qt::AlignLeft);

    QWidget *workspaceWidget = new QWidget();
    workspaceWidget->setLayout(layoutWorkspace);

    return workspaceWidget;
}

QWidget *SettingsWidget::controlsMeshAndSolver()
{
    // QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    // grpSolver->setLayout(layoutSolver);

    txtMeshAngleSegmentsCount = new QSpinBox(this);
    txtMeshAngleSegmentsCount->setMinimum(2);
    txtMeshAngleSegmentsCount->setMaximum(20);
    chkMeshCurvilinearElements = new QCheckBox(tr("Curvilinear elements"));

    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->addWidget(chkMeshCurvilinearElements, 0, 0, 1, 2);
    layoutMesh->addWidget(new QLabel(tr("Angle segments count:")), 1, 0);
    layoutMesh->addWidget(txtMeshAngleSegmentsCount, 1, 1);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh"));
    grpMesh->setLayout(layoutMesh);

    QPushButton *btnMeshAndSolverDefault = new QPushButton(tr("Default"));
    connect(btnMeshAndSolverDefault, SIGNAL(clicked()), this, SLOT(doMeshAndSolverDefault()));

    // adaptivity
    QFont fnt = font();
    fnt.setPointSize(fnt.pointSize() - 1);

    lblMaxDofs = new QLabel(tr("Maximum number of DOFs:"));
    txtMaxDOFs = new QSpinBox(this);
    txtMaxDOFs->setMinimum(1e2);
    txtMaxDOFs->setMaximum(1e9);
    txtMaxDOFs->setSingleStep(1e2);

    txtConvExp = new LineEditDouble();
    lblConvExp = new QLabel(tr("<b></b>default value is 1.0, this parameter influences the selection<br/>of candidates in hp-adaptivity"));
    lblConvExp->setFont(fnt);

    txtThreshold = new LineEditDouble();
    lblThreshold = new QLabel(tr("<b></b>quantitative parameter of the adapt(...) function<br/>with different meanings for various adaptive strategies"));
    lblThreshold->setFont(fnt);

    cmbStrategy = new QComboBox();
    cmbStrategy->addItem(tr("0"), 0);
    cmbStrategy->addItem(tr("1"), 1);
    cmbStrategy->addItem(tr("2"), 2);
    connect(cmbStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(doStrategyChanged(int)));
    lblStrategy = new QLabel("");
    lblStrategy->setFont(fnt);
    doStrategyChanged(0);

    cmbMeshRegularity = new QComboBox();
    cmbMeshRegularity->addItem(tr("arbitrary level"), -1);
    cmbMeshRegularity->addItem(tr("at most one-level"), 1);
    cmbMeshRegularity->addItem(tr("at most two-level"), 2);
    cmbMeshRegularity->addItem(tr("at most three-level"), 3);
    cmbMeshRegularity->addItem(tr("at most four-level"), 4);
    cmbMeshRegularity->addItem(tr("at most five-level"), 5);

    cmbProjNormType = new QComboBox();
    cmbProjNormType->addItem(errorNormString(Hermes::Hermes2D::HERMES_H1_NORM), Hermes::Hermes2D::HERMES_H1_NORM);
    cmbProjNormType->addItem(errorNormString(Hermes::Hermes2D::HERMES_L2_NORM), Hermes::Hermes2D::HERMES_L2_NORM);
    cmbProjNormType->addItem(errorNormString(Hermes::Hermes2D::HERMES_H1_SEMINORM), Hermes::Hermes2D::HERMES_H1_SEMINORM);

    chkUseAnIso = new QCheckBox(tr("Use anisotropic refinements"));
    chkFinerReference = new QCheckBox(tr("Use hp reference solution for h and p adaptivity"));

    QGridLayout *layoutAdaptivitySettings = new QGridLayout();
    layoutAdaptivitySettings->setColumnStretch(1, 1);
    layoutAdaptivitySettings->addWidget(lblMaxDofs, 0, 0);
    layoutAdaptivitySettings->addWidget(txtMaxDOFs, 0, 1, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Conv. exp.:")), 2, 0);
    layoutAdaptivitySettings->addWidget(txtConvExp, 2, 1);
    layoutAdaptivitySettings->addWidget(lblConvExp, 3, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Strategy:")), 4, 0);
    layoutAdaptivitySettings->addWidget(cmbStrategy, 4, 1);
    layoutAdaptivitySettings->addWidget(lblStrategy, 5, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Threshold:")), 6, 0);
    layoutAdaptivitySettings->addWidget(txtThreshold, 6, 1);
    layoutAdaptivitySettings->addWidget(lblThreshold, 7, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Hanging nodes:")), 8, 0);
    layoutAdaptivitySettings->addWidget(cmbMeshRegularity, 8, 1);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Norm:")), 9, 0);
    layoutAdaptivitySettings->addWidget(cmbProjNormType, 9, 1);
    layoutAdaptivitySettings->addWidget(chkUseAnIso, 10, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(chkFinerReference, 11, 0, 1, 2);

    QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
    layoutAdaptivity->addLayout(layoutAdaptivitySettings);

    QGroupBox *grpAdaptivity = new QGroupBox("Adaptivity");
    grpAdaptivity->setLayout(layoutAdaptivity);

    // commands
    txtArgumentTriangle = new QLineEdit("");
    txtArgumentGmsh = new QLineEdit("");

    // default
    QGridLayout *layoutCommands = new QGridLayout();
    layoutCommands->addWidget(new QLabel(tr("Triangle")), 0, 0);
    layoutCommands->addWidget(txtArgumentTriangle, 1, 0);
    layoutCommands->addWidget(new QLabel(tr("GMSH")), 2, 0);
    layoutCommands->addWidget(txtArgumentGmsh, 3, 0);

    QGroupBox *grpCommands = new QGroupBox("Commands");
    grpCommands->setLayout(layoutCommands);

    // layout mesh and solver
    QVBoxLayout *layoutMeshAndSolver = new QVBoxLayout();
    layoutMeshAndSolver->addWidget(grpMesh);
    layoutMeshAndSolver->addWidget(grpAdaptivity);
    layoutMeshAndSolver->addWidget(grpCommands);
    layoutMeshAndSolver->addStretch();
    layoutMeshAndSolver->addWidget(btnMeshAndSolverDefault, 0, Qt::AlignLeft);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutMeshAndSolver);

    return widget;
}

QWidget *SettingsWidget::controlsColors()
{
    QWidget *colorsWidget = new QWidget(this);

    // colors
    colorBackground = new ColorButton(this);
    colorGrid = new ColorButton(this);
    colorCross = new ColorButton(this);

    colorNodes = new ColorButton(this);
    colorEdges = new ColorButton(this);
    colorLabels = new ColorButton(this);
    colorContours = new ColorButton(this);
    colorVectors = new ColorButton(this);
    colorInitialMesh = new ColorButton(this);
    colorSolutionMesh = new ColorButton(this);

    colorHighlighted = new ColorButton(this);
    colorSelected = new ColorButton(this);

    QGridLayout *layoutColors = new QGridLayout();
    layoutColors->addWidget(new QLabel(tr("Background:")), 0, 0);
    layoutColors->addWidget(new QLabel(tr("Grid:")), 1, 0);
    layoutColors->addWidget(new QLabel(tr("Cross:")), 2, 0);
    layoutColors->addWidget(new QLabel(tr("Nodes:")), 3, 0);
    layoutColors->addWidget(new QLabel(tr("Edges:")), 4, 0);
    layoutColors->addWidget(new QLabel(tr("Labels:")), 5, 0);
    layoutColors->addWidget(new QLabel(tr("Contours:")), 6, 0);
    layoutColors->addWidget(new QLabel(tr("Vectors:")), 7, 0);
    layoutColors->addWidget(new QLabel(tr("Initial mesh:")), 8, 0);
    layoutColors->addWidget(new QLabel(tr("Solution mesh:")), 9, 0);
    layoutColors->addWidget(new QLabel(tr("Highlighted elements:")), 10, 0);
    layoutColors->addWidget(new QLabel(tr("Selected elements:")), 11, 0);

    layoutColors->addWidget(colorBackground, 0, 1);
    layoutColors->addWidget(colorGrid, 1, 1);
    layoutColors->addWidget(colorCross, 2, 1);
    layoutColors->addWidget(colorNodes, 3, 1);
    layoutColors->addWidget(colorEdges, 4, 1);
    layoutColors->addWidget(colorLabels, 5, 1);
    layoutColors->addWidget(colorContours, 6, 1);
    layoutColors->addWidget(colorVectors, 7, 1);
    layoutColors->addWidget(colorInitialMesh, 8, 1);
    layoutColors->addWidget(colorSolutionMesh, 9, 1);
    layoutColors->addWidget(colorHighlighted, 10, 1);
    layoutColors->addWidget(colorSelected, 11, 1);

    // default
    QPushButton *btnDefault = new QPushButton(tr("Default"));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(doColorsDefault()));

    QGroupBox *grpColor = new QGroupBox(tr("Colors"));
    grpColor->setLayout(layoutColors);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpColor);
    layout->addStretch();
    layout->addWidget(btnDefault, 0, Qt::AlignLeft);

    colorsWidget->setLayout(layout);

    return colorsWidget;
}

void SettingsWidget::doStrategyChanged(int index)
{


    switch (index)
    {
    case 0:
        lblStrategy->setText(tr("refine elements until sqrt(<b>threshold</b>) times total error<br/>is processed. If more elements have similar errors, refine all<br/>to keep the mesh symmetric"));
        break;
    case 1:
        lblStrategy->setText(tr("refine all elements whose error is larger<br/>than <b>threshold</b> times maximum element error"));
        break;
    case 2:
        lblStrategy->setText(tr("refine all elements whose error is larger<br/>than <b>threshold</b>"));
        break;
    default:
        break;
    }
}

void SettingsWidget::setControls()
{

}

void SettingsWidget::updateControls()
{
    load();
}

void SettingsWidget::doApply()
{
    save();

    // time step
    QApplication::processEvents();

    emit apply();

    activateWindow();
}

void SettingsWidget::doWorkspaceDefault()
{
    txtGridStep->setText(QString::number(GRIDSTEP));
    chkShowGrid->setChecked(SHOWGRID);
    chkSnapToGrid->setChecked(SNAPTOGRID);

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(RULERSFONT));
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(POSTFONT));

    chkShowAxes->setChecked(SHOWAXES);
    chkShowRulers->setChecked(SHOWRULERS);

    chkZoomToMouse->setChecked(ZOOMTOMOUSE);
    txtGeometryNodeSize->setValue(GEOMETRYNODESIZE);
    txtGeometryEdgeWidth->setValue(GEOMETRYEDGEWIDTH);
    txtGeometryLabelSize->setValue(GEOMETRYLABELSIZE);

    chkView3DLighting->setChecked(VIEW3DLIGHTING);
    txtView3DAngle->setValue(VIEW3DANGLE);
    chkView3DBackground->setChecked(VIEW3DBACKGROUND);
    txtView3DHeight->setValue(VIEW3DHEIGHT);

    chkDeformScalar->setChecked(DEFORMSCALAR);
    chkDeformContour->setChecked(DEFORMCONTOUR);
    chkDeformVector->setChecked(DEFORMVECTOR);
}

void SettingsWidget::doMeshAndSolverDefault()
{
    txtMeshAngleSegmentsCount->setValue(MESHANGLESEGMENTSCOUNT);
    chkMeshCurvilinearElements->setChecked(MESHCURVILINEARELEMENTS);

    txtMaxDOFs->setValue(MAX_DOFS);
    txtConvExp->setValue(ADAPTIVITY_CONVEXP);
    txtThreshold->setValue(ADAPTIVITY_THRESHOLD);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(ADAPTIVITY_STRATEGY));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(ADAPTIVITY_MESHREGULARITY));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(ADAPTIVITY_PROJNORMTYPE));
    chkUseAnIso->setChecked(ADAPTIVITY_ANISO);
    chkFinerReference->setChecked(ADAPTIVITY_FINER_REFERENCE_H_AND_P);

    txtArgumentTriangle->setText(COMMANDS_TRIANGLE);
    txtArgumentGmsh->setText(COMMANDS_GMSH);
}

void SettingsWidget::doColorsDefault()
{
    colorBackground->setColor(COLORBACKGROUND);
    colorGrid->setColor(COLORGRID);
    colorCross->setColor(COLORCROSS);
    colorNodes->setColor(COLORNODES);
    colorEdges->setColor(COLOREDGES);
    colorLabels->setColor(COLORLABELS);
    colorContours->setColor(COLORCONTOURS);
    colorVectors->setColor(COLORVECTORS);
    colorInitialMesh->setColor(COLORINITIALMESH);
    colorSolutionMesh->setColor(COLORSOLUTIONMESH);
    colorHighlighted->setColor(COLORHIGHLIGHTED);
    colorSelected->setColor(COLORSELECTED);
}

void SettingsWidget::doShowGridChanged()
{
    chkSnapToGrid->setEnabled(chkShowGrid->isChecked());
}

void SettingsWidget::doStartupScriptChanged()
{
    lblStartupScriptError->clear();

    // run and check startup script
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        currentPythonEngineAgros()->blockSignals(true);
        ScriptResult scriptResult = currentPythonEngineAgros()->runScript(txtStartupScript->toPlainText());
        currentPythonEngineAgros()->blockSignals(false);
        if (scriptResult.isError)
            lblStartupScriptError->setText(QObject::tr("Error: %1").arg(scriptResult.text));
    }
}

// *******************************************************************************************************

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    setAutoFillBackground(false);
    setCursor(Qt::PointingHandCursor);
    connect(this, SIGNAL(clicked()), this, SLOT(doClicked()));
}

ColorButton::~ColorButton()
{
}

void ColorButton::setColor(const QColor &color)
{
    m_color = color;
    repaint();
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(m_color);
    painter.setBrush(m_color);
    painter.drawRect(rect());
}

void ColorButton::doClicked()
{
    QColor color = QColorDialog::getColor(m_color);

    if (color.isValid())
    {
        setColor(color);
    }
}
