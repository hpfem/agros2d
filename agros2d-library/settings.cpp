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
#include "util/global.h"

#include "gui/common.h"
#include "gui/lineeditdouble.h"

#include "hermes2d/problem_config.h"

#include "scene.h"
#include "pythonlab/pythonengine_agros.h"

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
    actSettings->setShortcut(tr("Ctrl+8"));
    actSettings->setCheckable(true);
}

void SettingsWidget::load()
{
    // workspace
    chkShowGrid->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowGrid).toBool());

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_RulersFontFamily).toString()));
    txtRulersFontSizes->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_RulersFontPointSize).toInt());
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_PostFontFamily).toString()));
    txtPostFontSizes->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_PostFontPointSize).toInt());

    chkShowAxes->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool());
    chkShowRulers->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool());

    chkZoomToMouse->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ZoomToMouse).toBool());
    txtGeometryNodeSize->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_NodeSize).toInt());
    txtGeometryEdgeWidth->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_EdgeWidth).toInt());
    txtGeometryLabelSize->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_LabelSize).toInt());

    // script
    txtStartupScript->setPlainText(Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString());

    // 3d
    chkView3DLighting->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DLighting).toBool());
    txtView3DAngle->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DAngle).toDouble());
    chkView3DBackground->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DBackground).toBool());
    txtView3DHeight->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DHeight).toDouble());
    chkView3DBoundingBox->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DBoundingBox).toBool());
    chkView3DSolidGeometry->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DSolidGeometry).toBool());

    // deform shape
    chkDeformContour->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_DeformContour).toBool());
    chkDeformScalar->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_DeformScalar).toBool());
    chkDeformVector->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_DeformVector).toBool());

    // colors
    colorBackground->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundRed).toInt(),
                                     Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundGreen).toInt(),
                                     Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundBlue).toInt()));
    colorGrid->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorGridRed).toInt(),
                               Agros2D::problem()->setting()->value(ProblemSetting::View_ColorGridGreen).toInt(),
                               Agros2D::problem()->setting()->value(ProblemSetting::View_ColorGridBlue).toInt()));
    colorCross->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorCrossRed).toInt(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ColorCrossGreen).toInt(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ColorCrossBlue).toInt()));
    colorNodes->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorNodesRed).toInt(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ColorNodesGreen).toInt(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ColorNodesBlue).toInt()));
    colorEdges->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesRed).toInt(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesGreen).toInt(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesBlue).toInt()));
    colorLabels->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorLabelsRed).toInt(),
                                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorLabelsGreen).toInt(),
                                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorLabelsBlue).toInt()));
    colorContours->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursRed).toInt(),
                                   Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursGreen).toInt(),
                                   Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursBlue).toInt()));
    colorVectors->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorVectorsRed).toInt(),
                                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorVectorsGreen).toInt(),
                                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorVectorsBlue).toInt()));
    colorInitialMesh->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorInitialMeshRed).toInt(),
                                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorInitialMeshGreen).toInt(),
                                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorInitialMeshBlue).toInt()));
    colorSolutionMesh->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSolutionMeshRed).toInt(),
                                       Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSolutionMeshGreen).toInt(),
                                       Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSolutionMeshBlue).toInt()));
    colorHighlighted->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorHighlightedRed).toInt(),
                                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorHighlightedGreen).toInt(),
                                      Agros2D::problem()->setting()->value(ProblemSetting::View_ColorHighlightedBlue).toInt()));
    colorSelected->setColor(QColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedRed).toInt(),
                                   Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedGreen).toInt(),
                                   Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSelectedBlue).toInt()));

    // mesh and solver
    txtMeshAngleSegmentsCount->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_MeshAngleSegmentsCount).toInt());
    chkMeshCurvilinearElements->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_MeshCurvilinearElements).toBool());
}

void SettingsWidget::save()
{
    // run and check startup script
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        currentPythonEngineAgros()->blockSignals(true);
        bool successfulRun = currentPythonEngineAgros()->runExpression(txtStartupScript->toPlainText());
        currentPythonEngineAgros()->blockSignals(false);
        if (!successfulRun)
            return;
    }

    // workspace
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowGrid, chkShowGrid->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowRulers, chkShowRulers->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ZoomToMouse, chkZoomToMouse->isChecked());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_RulersFontFamily, cmbRulersFont->itemData(cmbRulersFont->currentIndex()).toString());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_RulersFontPointSize, txtRulersFontSizes->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PostFontFamily, cmbPostFont->itemData(cmbPostFont->currentIndex()).toString());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PostFontPointSize, txtPostFontSizes->value());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowAxes, chkShowAxes->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowRulers, chkShowRulers->isChecked());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_NodeSize, txtGeometryNodeSize->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_EdgeWidth, txtGeometryEdgeWidth->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_LabelSize, txtGeometryLabelSize->value());

    // script
    Agros2D::problem()->setting()->setValue(ProblemSetting::Problem_StartupScript, txtStartupScript->toPlainText());

    // 3d
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DLighting, chkView3DLighting->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DAngle, txtView3DAngle->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DBackground, chkView3DBackground->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DHeight, txtView3DHeight->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DBoundingBox, chkView3DBoundingBox->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarView3DSolidGeometry, chkView3DSolidGeometry->isChecked());

    // deform shape
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_DeformScalar, chkDeformScalar->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_DeformContour, chkDeformContour->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_DeformVector, chkDeformVector->isChecked());

    // color
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorBackgroundRed, colorBackground->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorBackgroundGreen, colorBackground->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorBackgroundBlue, colorBackground->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorGridRed, colorGrid->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorGridGreen, colorGrid->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorGridBlue, colorGrid->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorCrossRed, colorCross->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorCrossGreen, colorCross->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorCrossBlue, colorCross->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorNodesRed, colorNodes->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorNodesGreen, colorNodes->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorNodesBlue, colorNodes->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorEdgesRed, colorEdges->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorEdgesGreen, colorEdges->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorEdgesBlue, colorEdges->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorLabelsRed, colorLabels->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorLabelsGreen, colorLabels->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorLabelsBlue, colorLabels->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorContoursRed, colorContours->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorContoursGreen, colorContours->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorContoursBlue, colorContours->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorVectorsRed, colorVectors->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorVectorsGreen, colorVectors->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorVectorsBlue, colorVectors->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorInitialMeshRed, colorInitialMesh->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorInitialMeshGreen, colorInitialMesh->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorInitialMeshBlue, colorInitialMesh->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorSolutionMeshRed, colorSolutionMesh->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorSolutionMeshGreen, colorSolutionMesh->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorSolutionMeshBlue, colorSolutionMesh->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorHighlightedRed, colorHighlighted->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorHighlightedGreen, colorHighlighted->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorHighlightedBlue, colorHighlighted->color().blue());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorSelectedRed, colorSelected->color().red());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorSelectedGreen, colorSelected->color().green());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ColorSelectedBlue, colorSelected->color().blue());

    // mesh and solver
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_MeshAngleSegmentsCount, txtMeshAngleSegmentsCount->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_MeshCurvilinearElements, chkMeshCurvilinearElements->isChecked());
}

void SettingsWidget::createControls()
{
    QWidget *workspace = controlsWorkspace();
    QWidget *colors = controlsColors();
    QWidget *meshAndSolver = controlsMeshAndSolver();
    QWidget *startupScript = controlsStartupScript();

    // tab widget
    QToolBox *tbxWorkspace = new QToolBox();
    tbxWorkspace->addItem(workspace, icon(""), tr("Workspace"));
    tbxWorkspace->addItem(colors, icon(""), tr("Colors"));
    tbxWorkspace->addItem(startupScript, icon(""), tr("Startup script"));
    tbxWorkspace->addItem(meshAndSolver, icon(""), tr("Mesh and solver"));

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
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(widget);
    layoutMain->addLayout(layoutButtons);

    setControls();

    setLayout(layoutMain);
}

QWidget *SettingsWidget::controlsStartupScript()
{
    txtStartupScript = new ScriptEditor(currentPythonEngine(), this);
    connect(txtStartupScript, SIGNAL(textChanged()), this, SLOT(doStartupScriptChanged()));
    lblStartupScriptError = new QLabel();

    QPalette palette = lblStartupScriptError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblStartupScriptError->setPalette(palette);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtStartupScript);
    layout->addWidget(lblStartupScriptError);

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *SettingsWidget::controlsWorkspace()
{
    // workspace
    chkShowGrid = new QCheckBox(tr("Show grid"));
    chkZoomToMouse = new QCheckBox(tr("Zoom to mouse"));
    chkShowRulers = new QCheckBox(tr("Show rulers"));
    chkShowAxes = new QCheckBox(tr("Show axes"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(chkShowGrid, 1, 0);
    layoutGrid->addWidget(chkShowAxes, 2, 0);
    layoutGrid->addWidget(chkShowRulers, 3, 0);
    layoutGrid->addWidget(chkZoomToMouse, 2, 1);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    cmbRulersFont = new QComboBox();
    fillComboBoxFonts(cmbRulersFont);
    txtRulersFontSizes = new QSpinBox();
    txtRulersFontSizes->setMinimum(6);
    txtRulersFontSizes->setMaximum(40);

    cmbPostFont = new QComboBox();
    fillComboBoxFonts(cmbPostFont);
    txtPostFontSizes = new QSpinBox();
    txtPostFontSizes->setMinimum(6);
    txtPostFontSizes->setMaximum(40);

    QGridLayout *layoutFont = new QGridLayout();
    layoutFont->setColumnStretch(1, 1);
    layoutFont->addWidget(new QLabel(tr("Rulers:")), 0, 0);
    layoutFont->addWidget(cmbRulersFont, 0, 1);
    layoutFont->addWidget(txtRulersFontSizes, 0, 2);
    layoutFont->addWidget(new QLabel(tr("Postprocessor:")), 2, 0);
    layoutFont->addWidget(cmbPostFont, 2, 1);
    layoutFont->addWidget(txtPostFontSizes, 2, 2);

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
    layoutGeometry->setColumnStretch(1, 1);
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
    chkView3DBackground = new QCheckBox(tr("Gradient back."), this);
    txtView3DHeight = new QDoubleSpinBox(this);
    txtView3DHeight->setDecimals(1);
    txtView3DHeight->setSingleStep(0.1);
    txtView3DHeight->setMinimum(0.2);
    txtView3DHeight->setMaximum(10.0);
    chkView3DBoundingBox = new QCheckBox(tr("Bounding box"), this);
    chkView3DSolidGeometry = new QCheckBox(tr("Show edges"), this);

    QGridLayout *layout3D = new QGridLayout();
    layout3D->setColumnStretch(1, 1);
    layout3D->addWidget(new QLabel(tr("Angle:")), 0, 0);
    layout3D->addWidget(txtView3DAngle, 0, 1);
    layout3D->addWidget(chkView3DLighting, 0, 2);
    layout3D->addWidget(new QLabel(tr("Height:")), 1, 0);
    layout3D->addWidget(txtView3DHeight, 1, 1);
    layout3D->addWidget(chkView3DBackground, 1, 2);
    layout3D->addWidget(chkView3DBoundingBox, 2, 2);
    layout3D->addWidget(chkView3DSolidGeometry, 3, 2);

    QGroupBox *grp3D = new QGroupBox(tr("3D view"));
    grp3D->setLayout(layout3D);

    // layout deform shape
    chkDeformScalar = new QCheckBox(tr("Scalar field"), this);
    chkDeformContour = new QCheckBox(tr("Contours"), this);
    chkDeformVector = new QCheckBox(tr("Vectors"), this);

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
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(chkMeshCurvilinearElements, 0, 0, 1, 2);
    layoutMesh->addWidget(new QLabel(tr("Angle seg. count:")), 1, 0);
    layoutMesh->addWidget(txtMeshAngleSegmentsCount, 1, 1);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh"));
    grpMesh->setLayout(layoutMesh);

    QPushButton *btnMeshAndSolverDefault = new QPushButton(tr("Default"));
    connect(btnMeshAndSolverDefault, SIGNAL(clicked()), this, SLOT(doMeshAndSolverDefault()));

    // adaptivity
    QFont fnt = font();
    fnt.setPointSize(fnt.pointSize() - 1);

    // layout mesh and solver
    QVBoxLayout *layoutMeshAndSolver = new QVBoxLayout();
    layoutMeshAndSolver->addWidget(grpMesh);
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
    layoutColors->setColumnStretch(1, 1);
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
    chkShowGrid->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowGrid).toBool());

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_RulersFontFamily).toString()));
    txtRulersFontSizes->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_RulersFontPointSize).toInt());
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PostFontFamily).toString()));
    txtPostFontSizes->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PostFontPointSize).toInt());

    chkShowAxes->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowAxes).toBool());
    chkShowRulers->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowRulers).toBool());

    chkZoomToMouse->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ZoomToMouse).toBool());
    txtGeometryNodeSize->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_NodeSize).toInt());
    txtGeometryEdgeWidth->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_EdgeWidth).toInt());
    txtGeometryLabelSize->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_LabelSize).toInt());

    chkView3DLighting->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarView3DLighting).toBool());
    txtView3DAngle->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarView3DAngle).toDouble());
    chkView3DBackground->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarView3DBackground).toBool());
    txtView3DHeight->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ScalarView3DHeight).toDouble());

    chkDeformScalar->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_DeformScalar).toBool());
    chkDeformContour->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_DeformContour).toBool());
    chkDeformVector->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_DeformVector).toBool());
}

void SettingsWidget::doMeshAndSolverDefault()
{
    txtMeshAngleSegmentsCount->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_MeshAngleSegmentsCount).toInt());
    chkMeshCurvilinearElements->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_MeshCurvilinearElements).toBool());
}

void SettingsWidget::doColorsDefault()
{
    colorBackground->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorBackgroundRed).toInt(),
                                     Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorBackgroundGreen).toInt(),
                                     Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorBackgroundBlue).toInt()));
    colorGrid->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorGridRed).toInt(),
                               Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorGridGreen).toInt(),
                               Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorGridBlue).toInt()));
    colorCross->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorCrossRed).toInt(),
                                Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorCrossGreen).toInt(),
                                Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorCrossBlue).toInt()));
    colorNodes->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorNodesRed).toInt(),
                                Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorNodesGreen).toInt(),
                                Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorNodesBlue).toInt()));
    colorEdges->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorEdgesRed).toInt(),
                                Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorEdgesGreen).toInt(),
                                Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorEdgesBlue).toInt()));
    colorLabels->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorLabelsRed).toInt(),
                                 Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorLabelsGreen).toInt(),
                                 Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorLabelsBlue).toInt()));
    colorContours->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorContoursRed).toInt(),
                                   Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorContoursGreen).toInt(),
                                   Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorContoursBlue).toInt()));
    colorVectors->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorVectorsRed).toInt(),
                                  Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorVectorsGreen).toInt(),
                                  Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorVectorsBlue).toInt()));
    colorInitialMesh->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorInitialMeshRed).toInt(),
                                      Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorInitialMeshGreen).toInt(),
                                      Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorInitialMeshBlue).toInt()));
    colorSolutionMesh->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorSolutionMeshRed).toInt(),
                                       Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorSolutionMeshGreen).toInt(),
                                       Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorSolutionMeshBlue).toInt()));
    colorHighlighted->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorHighlightedRed).toInt(),
                                      Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorHighlightedGreen).toInt(),
                                      Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorHighlightedBlue).toInt()));
    colorSelected->setColor(QColor(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorSelectedRed).toInt(),
                                   Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorSelectedGreen).toInt(),
                                   Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ColorSelectedBlue).toInt()));
}

void SettingsWidget::doStartupScriptChanged()
{
    lblStartupScriptError->clear();

    // run and check startup script
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        currentPythonEngineAgros()->blockSignals(true);
        bool successfulRun = currentPythonEngineAgros()->runScript(txtStartupScript->toPlainText());
        currentPythonEngineAgros()->blockSignals(false);

        if (!successfulRun)
        {
            ErrorResult result = currentPythonEngineAgros()->parseError();
            lblStartupScriptError->setText(QObject::tr("Error: %1").arg(result.error()));
        }
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
