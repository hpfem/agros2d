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

#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ConfigDialog::ConfigDialog()");

    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void ConfigDialog::load()
{
    logMessage("ConfigDialog::load()");

    // gui style
    cmbGUIStyle->setCurrentIndex(cmbGUIStyle->findText(Util::config()->guiStyle));
    if (cmbGUIStyle->currentIndex() == -1 && cmbGUIStyle->count() > 0) cmbGUIStyle->setCurrentIndex(0);

    // language
    cmbLanguage->setCurrentIndex(cmbLanguage->findText(Util::config()->language));
    if (cmbLanguage->currentIndex() == -1 && cmbLanguage->count() > 0) cmbLanguage->setCurrentIndex(0);

    // default physic field
    cmbDefaultPhysicField->setCurrentIndex(cmbDefaultPhysicField->findData(Util::config()->defaultPhysicField));

    // check version
    chkCheckVersion->setChecked(Util::config()->checkVersion);

    // show convergence chart
    chkShowConvergenceChart->setChecked(Util::config()->showConvergenceChart);

    // logs
    chkEnabledApplicationLog->setChecked(Util::config()->enabledApplicationLog);
    chkEnabledProgressLog->setChecked(Util::config()->enabledProgressLog);

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(Util::config()->lineEditValueShowResult);

    // geometry
    txtMeshAngleSegmentsCount->setValue(Util::config()->angleSegmentsCount);
    txtGeometryNodeSize->setValue(Util::config()->nodeSize);
    txtGeometryEdgeWidth->setValue(Util::config()->edgeWidth);
    txtGeometryLabelSize->setValue(Util::config()->labelSize);
    chkMeshCurvilinearElements->setChecked(Util::config()->curvilinearElements);
    chkZoomToMouse->setChecked(Util::config()->zoomToMouse);

    // delete files
    chkDeleteTriangleMeshFiles->setChecked(Util::config()->deleteTriangleMeshFiles);
    chkDeleteHermes2DMeshFile->setChecked(Util::config()->deleteHermes2DMeshFile);

    // save problem with solution
#ifdef BETA
    chkSaveWithSolution->setChecked(Util::config()->saveProblemWithSolution);
#endif

    // colors
    colorBackground->setColor(Util::config()->colorBackground);
    colorGrid->setColor(Util::config()->colorGrid);
    colorCross->setColor(Util::config()->colorCross);
    colorNodes->setColor(Util::config()->colorNodes);
    colorEdges->setColor(Util::config()->colorEdges);
    colorLabels->setColor(Util::config()->colorLabels);
    colorContours->setColor(Util::config()->colorContours);
    colorVectors->setColor(Util::config()->colorVectors);
    colorInitialMesh->setColor(Util::config()->colorInitialMesh);
    colorSolutionMesh->setColor(Util::config()->colorSolutionMesh);
    colorHighlighted->setColor(Util::config()->colorHighlighted);
    colorSelected->setColor(Util::config()->colorSelected);

    // grid
    txtGridStep->setText(QString::number(Util::config()->gridStep));
    chkShowGrid->setChecked(Util::config()->showGrid);
    chkSnapToGrid->setChecked(Util::config()->snapToGrid);
    chkRulers->setChecked(Util::config()->showRulers);

    // axes
    chkShowAxes->setChecked(Util::config()->showAxes);

    // contours
    txtContoursCount->setValue(Util::config()->contoursCount);

    // scalar field
    cmbPalette->setCurrentIndex(cmbPalette->findData(Util::config()->paletteType));
    chkPaletteFilter->setChecked(Util::config()->paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(Util::config()->paletteSteps);
    chkScalarFieldRangeLog->setChecked(Util::config()->scalarRangeLog);
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setText(QString::number(Util::config()->scalarRangeBase));
    txtScalarDecimalPlace->setValue(Util::config()->scalarDecimalPlace);

    // vector field
    chkVectorProportional->setChecked(Util::config()->vectorProportional);
    chkVectorColor->setChecked(Util::config()->vectorColor);
    txtVectorCount->setValue(Util::config()->vectorCount);
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(Util::config()->vectorScale);

    // order view
    chkOrderLabel->setChecked(Util::config()->orderLabel);

    // 3d
    chkView3DLighting->setChecked(Util::config()->scalarView3DLighting);
    txtView3DAngle->setValue(Util::config()->scalarView3DAngle);
    chkView3DBackground->setChecked(Util::config()->scalarView3DBackground);

    // deform shape
    chkDeformScalar->setChecked(Util::config()->deformScalar);
    chkDeformContour->setChecked(Util::config()->deformContour);
    chkDeformVector->setChecked(Util::config()->deformVector);

    // adaptivity
    chkIsoOnly->setChecked(Util::config()->isoOnly);
    txtConvExp->setValue(Util::config()->convExp);
    txtThreshold->setValue(Util::config()->threshold);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(Util::config()->strategy));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(Util::config()->meshRegularity));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(Util::config()->projNormType));

    // command argument
    txtArgumentTriangle->setText(Util::config()->commandTriangle);
    txtArgumentFFmpeg->setText(Util::config()->commandFFmpeg);

    // global script
    txtGlobalScript->setPlainText(Util::config()->globalScript);
}

void ConfigDialog::save()
{
    logMessage("ConfigDialog::save()");

    // gui style
    Util::config()->guiStyle = cmbGUIStyle->currentText();
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (Util::config()->language != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                                 tr("Language change"),
                                 tr("Interface language has been changed. You must restart the application."));
    Util::config()->language = cmbLanguage->currentText();

    // default physic field
    Util::config()->defaultPhysicField = (PhysicField) cmbDefaultPhysicField->itemData(cmbDefaultPhysicField->currentIndex()).toInt();

    // check version
    Util::config()->checkVersion = chkCheckVersion->isChecked();

    // show convergence chart
    Util::config()->showConvergenceChart = chkShowConvergenceChart->isChecked();

    // logs
    Util::config()->enabledApplicationLog = chkEnabledApplicationLog->isChecked();
    Util::config()->enabledProgressLog = chkEnabledProgressLog->isChecked();

    // show result in line edit value widget
    Util::config()->lineEditValueShowResult = chkLineEditValueShowResult->isChecked();

    // geometry
    Util::config()->nodeSize = txtGeometryNodeSize->value();
    Util::config()->edgeWidth = txtGeometryEdgeWidth->value();
    Util::config()->labelSize = txtGeometryLabelSize->value();
    Util::config()->zoomToMouse = chkZoomToMouse->isChecked();

    // mesh
    Util::config()->angleSegmentsCount = txtMeshAngleSegmentsCount->value();
    Util::config()->curvilinearElements = chkMeshCurvilinearElements->isChecked();

    // delete files
    Util::config()->deleteTriangleMeshFiles = chkDeleteTriangleMeshFiles->isChecked();
    Util::config()->deleteHermes2DMeshFile = chkDeleteHermes2DMeshFile->isChecked();

    // save problem with solution
#ifdef BETA
    Util::config()->saveProblemWithSolution = chkSaveWithSolution->isChecked();
#endif

    // color
    Util::config()->colorBackground = colorBackground->color();
    Util::config()->colorGrid = colorGrid->color();
    Util::config()->colorCross = colorCross->color();
    Util::config()->colorNodes = colorNodes->color();
    Util::config()->colorEdges = colorEdges->color();
    Util::config()->colorLabels = colorLabels->color();
    Util::config()->colorContours = colorContours->color();
    Util::config()->colorVectors = colorVectors->color();
    Util::config()->colorInitialMesh = colorInitialMesh->color();
    Util::config()->colorSolutionMesh = colorSolutionMesh->color();
    Util::config()->colorHighlighted = colorHighlighted->color();
    Util::config()->colorSelected = colorSelected->color();

    // grid
    Util::config()->showGrid = chkShowGrid->isChecked();
    Util::config()->gridStep = txtGridStep->text().toDouble();
    Util::config()->showRulers = chkRulers->isChecked();
    Util::config()->snapToGrid = chkSnapToGrid->isChecked();

    // axes
    Util::config()->showAxes = chkShowAxes->isChecked();

    // contours
    Util::config()->contoursCount = txtContoursCount->value();

    // scalar field
    Util::config()->paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    Util::config()->paletteFilter = chkPaletteFilter->isChecked();
    Util::config()->paletteSteps = txtPaletteSteps->value();
    Util::config()->scalarRangeLog = chkScalarFieldRangeLog->isChecked();
    Util::config()->scalarRangeBase = txtScalarFieldRangeBase->text().toDouble();
    Util::config()->scalarDecimalPlace = txtScalarDecimalPlace->value();

    // vector field
    Util::config()->vectorProportional = chkVectorProportional->isChecked();
    Util::config()->vectorColor = chkVectorColor->isChecked();
    Util::config()->vectorCount = txtVectorCount->value();
    Util::config()->vectorScale = txtVectorScale->value();

    // order view
    Util::config()->orderLabel = chkOrderLabel->isChecked();

    // 3d
    Util::config()->scalarView3DLighting = chkView3DLighting->isChecked();
    Util::config()->scalarView3DAngle = txtView3DAngle->value();
    Util::config()->scalarView3DBackground = chkView3DBackground->isChecked();

    // deform shape
    Util::config()->deformScalar = chkDeformScalar->isChecked();
    Util::config()->deformContour = chkDeformContour->isChecked();
    Util::config()->deformVector = chkDeformVector->isChecked();

    // adaptivity
    Util::config()->isoOnly = chkIsoOnly->isChecked();
    Util::config()->convExp = txtConvExp->value();
    Util::config()->threshold = txtThreshold->value();
    Util::config()->strategy = cmbStrategy->itemData(cmbStrategy->currentIndex()).toInt();
    Util::config()->meshRegularity = cmbMeshRegularity->itemData(cmbMeshRegularity->currentIndex()).toInt();
    Util::config()->projNormType = (ProjNormType) cmbProjNormType->itemData(cmbProjNormType->currentIndex()).toInt();

    // command argument
    Util::config()->commandTriangle = txtArgumentTriangle->text();
    Util::config()->commandFFmpeg = txtArgumentFFmpeg->text();

    // global script
    Util::config()->globalScript = txtGlobalScript->toPlainText();

    // save
    Util::config()->save();
}

void ConfigDialog::createControls()
{
    logMessage("ConfigDialog::createControls()");

    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panView = createViewWidget();
    panSolver = createSolverWidget();
    panColors = createColorsWidget();
    panGlobalScriptWidget = createGlobalScriptWidget();
    panAdvanced = createAdvancedWidget();

    // List View
    lstView->setCurrentRow(0);
    lstView->setViewMode(QListView::IconMode);
    lstView->setMovement(QListView::Static);
    lstView->setIconSize(QSize(64, 64));
    lstView->setMinimumWidth(100);
    lstView->setMaximumWidth(100);
    lstView->setSpacing(12);
    connect(lstView, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
               this, SLOT(doCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));

    // listView items
    QListWidgetItem *itemMain = new QListWidgetItem(icon("options-main"), tr("Main"), lstView);
    itemMain->setTextAlignment(Qt::AlignHCenter);
    itemMain->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemView = new QListWidgetItem(icon("options-view"), tr("View"), lstView);
    itemView->setTextAlignment(Qt::AlignHCenter);
    itemView->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemSolver = new QListWidgetItem(icon("options-solver"), tr("Solver"), lstView);
    itemSolver->setTextAlignment(Qt::AlignHCenter);
    itemSolver->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemColors = new QListWidgetItem(icon("options-colors"), tr("Colors"), lstView);
    itemColors->setTextAlignment(Qt::AlignHCenter);
    itemColors->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemGlobalScript = new QListWidgetItem(icon("options-python"), tr("Python"), lstView);
    itemGlobalScript->setTextAlignment(Qt::AlignHCenter);
    itemGlobalScript->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemAdvanced = new QListWidgetItem(icon("options-advanced"), tr("Advanced"), lstView);
    itemAdvanced->setTextAlignment(Qt::AlignHCenter);
    itemAdvanced->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    pages->addWidget(panMain);
    pages->addWidget(panView);
    pages->addWidget(panSolver);
    pages->addWidget(panColors);
    pages->addWidget(panGlobalScriptWidget);
    pages->addWidget(panAdvanced);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(lstView);
    layoutHorizontal->addWidget(pages);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ConfigDialog::createMainWidget()
{
    logMessage("ConfigDialog::createMainWidget()");

    QWidget *mainWidget = new QWidget(this);

    // general
    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    cmbDefaultPhysicField = new QComboBox();
    fillComboBoxPhysicField(cmbDefaultPhysicField);

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutGeneral->addWidget(cmbGUIStyle, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutGeneral->addWidget(cmbLanguage, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Default physic field:")), 2, 0);
    layoutGeneral->addWidget(cmbDefaultPhysicField, 2, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // other layout
    cmdClearCommandHistory = new QPushButton(mainWidget);
    cmdClearCommandHistory->setText(tr("Clear command history"));
    connect(cmdClearCommandHistory, SIGNAL(clicked()), this, SLOT(doClearCommandHistory()));

    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));
    chkCheckVersion = new QCheckBox(tr("Check new version during startup."));
    chkEnabledApplicationLog = new QCheckBox(tr("Enabled application log"));
    chkEnabledProgressLog = new QCheckBox(tr("Enabled progress log"));

    QHBoxLayout *layoutClearCommandHistory = new QHBoxLayout();
    layoutClearCommandHistory->addWidget(cmdClearCommandHistory);
    layoutClearCommandHistory->addStretch();

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkCheckVersion);
    layoutOther->addWidget(chkEnabledApplicationLog);
    layoutOther->addWidget(chkEnabledProgressLog);
    layoutOther->addLayout(layoutClearCommandHistory);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpOther);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ConfigDialog::createViewWidget()
{
    logMessage("ConfigDialog::createViewWidget()");

    // geometry
    txtGeometryNodeSize = new SLineEditDouble();
    // txtGeometryNodeSize->setMaximumWidth(60);
    txtGeometryEdgeWidth = new SLineEditDouble();
    // txtGeometryEdgeWidth->setMaximumWidth(60);
    txtGeometryLabelSize = new SLineEditDouble();
    // txtGeometryLabelSize->setMaximumWidth(60);

    QGridLayout *layoutGeometry = new QGridLayout();
    layoutGeometry->addWidget(new QLabel(tr("Node size:")), 0, 0);
    layoutGeometry->addWidget(txtGeometryNodeSize, 0, 1);
    layoutGeometry->addWidget(new QLabel(tr("Edge width:")), 1, 0);
    layoutGeometry->addWidget(txtGeometryEdgeWidth, 1, 1);
    layoutGeometry->addWidget(new QLabel(tr("Label size:")), 2, 0);
    layoutGeometry->addWidget(txtGeometryLabelSize, 2, 1);

    QGroupBox *grpGeometry = new QGroupBox(tr("Geometry"));
    grpGeometry->setLayout(layoutGeometry);

    // layout grid
    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkShowGrid = new QCheckBox(tr("Show grid"));
    chkRulers = new QCheckBox(tr("Show rulers"));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));
    chkShowAxes = new QCheckBox(tr("Show axes"));
    chkZoomToMouse = new QCheckBox(tr("Zoom to mouse pointer"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(new QLabel(tr("Grid step:")), 0, 0);
    layoutGrid->addWidget(txtGridStep, 0, 1);
    layoutGrid->addWidget(chkShowGrid, 1, 0, 1, 2);
    layoutGrid->addWidget(chkSnapToGrid, 2, 0, 1, 2);
    layoutGrid->addWidget(chkRulers, 3, 0, 1, 2);
    layoutGrid->addWidget(chkShowAxes, 4, 0, 1, 2);
    layoutGrid->addWidget(chkZoomToMouse, 5, 0, 1, 2);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    // layout general
    QVBoxLayout *layoutGeneral = new QVBoxLayout();
    layoutGeneral->addWidget(grpGeometry);
    layoutGeneral->addWidget(grpGrid);
    layoutGeneral->addStretch();

    QWidget *widgetGeneral = new QWidget(this);
    widgetGeneral->setLayout(layoutGeneral);

    // layout palette
    cmbPalette = new QComboBox();
    cmbPalette->addItem(tr("Jet"), Palette_Jet);
    cmbPalette->addItem(tr("Autumn"), Palette_Autumn);
    cmbPalette->addItem(tr("Hot"), Palette_Hot);
    cmbPalette->addItem(tr("Copper"), Palette_Copper);
    cmbPalette->addItem(tr("Cool"), Palette_Cool);
    cmbPalette->addItem(tr("B/W ascending"), Palette_BWAsc);
    cmbPalette->addItem(tr("B/W descending"), Palette_BWDesc);

    chkPaletteFilter = new QCheckBox(tr("Filter"));
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(5);
    txtPaletteSteps->setMaximum(100);

    // log scale
    chkScalarFieldRangeLog = new QCheckBox(tr("Log. scale"));
    txtScalarFieldRangeBase = new QLineEdit("10");
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    txtScalarDecimalPlace = new QSpinBox(this);
    txtScalarDecimalPlace->setMinimum(1);
    txtScalarDecimalPlace->setMaximum(10);

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Palette:")), 0, 0);
    layoutScalarField->addWidget(cmbPalette, 0, 1, 1, 1);

    layoutScalarField->addWidget(new QLabel(tr("Steps:")), 1, 0);
    layoutScalarField->addWidget(txtPaletteSteps, 1, 1);
    layoutScalarField->addWidget(chkPaletteFilter, 1, 2);
    layoutScalarField->addWidget(new QLabel(tr("Base:")), 2, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeBase, 2, 1);
    layoutScalarField->addWidget(chkScalarFieldRangeLog, 2, 2);

    layoutScalarField->addWidget(new QLabel(tr("Number of decimal places:")), 3, 0);
    layoutScalarField->addWidget(txtScalarDecimalPlace, 3, 1);

    QGroupBox *grpScalarView = new QGroupBox(tr("Scalar view"));
    grpScalarView->setLayout(layoutScalarField);

    // layout contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(1);
    txtContoursCount->setMaximum(100);

    QGridLayout *layoutContours = new QGridLayout();
    layoutContours->addWidget(new QLabel(tr("Contours count:")), 0, 0);
    layoutContours->addWidget(txtContoursCount, 0, 1);

    QGroupBox *grpContours = new QGroupBox(tr("Contours"));
    grpContours->setLayout(layoutContours);

    // vector field
    chkVectorProportional = new QCheckBox(tr("Proportional"), this);
    chkVectorColor = new QCheckBox(tr("Color (b/w)"), this);
    txtVectorCount = new QSpinBox(this);
    txtVectorCount->setMinimum(1);
    txtVectorCount->setMaximum(200);
    txtVectorScale = new SLineEditDouble(0);

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->addWidget(new QLabel(tr("Vectors:")), 0, 0);
    layoutVectorField->addWidget(txtVectorCount, 0, 1);
    layoutVectorField->addWidget(chkVectorProportional, 0, 2);
    layoutVectorField->addWidget(new QLabel(tr("Scale:")), 1, 0);
    layoutVectorField->addWidget(txtVectorScale, 1, 1);
    layoutVectorField->addWidget(chkVectorColor, 1, 2);

    QGroupBox *grpVectorView = new QGroupBox(tr("Vector view"));
    grpVectorView->setLayout(layoutVectorField);

    // layout 3d
    chkView3DLighting = new QCheckBox(tr("Ligthing"), this);
    txtView3DAngle = new SLineEditDouble(0, this);
    chkView3DBackground = new QCheckBox(tr("Gradient background"), this);

    QGridLayout *layout3D = new QGridLayout();
    layout3D->addWidget(new QLabel(tr("Angle:")), 0, 1);
    layout3D->addWidget(txtView3DAngle, 0, 2);
    layout3D->addWidget(chkView3DLighting, 0, 3);
    layout3D->addWidget(chkView3DBackground, 1, 3);

    QGroupBox *grp3D = new QGroupBox(tr("3D"));
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

    // layout order
    chkOrderLabel = new QCheckBox(tr("Show order label"), this);

    QHBoxLayout *layoutOrder = new QHBoxLayout();
    layoutOrder->addWidget(chkOrderLabel);

    QGroupBox *grpOrder = new QGroupBox(tr("Polynomial order"));
    grpOrder->setLayout(layoutOrder);

    // layout postprocessor
    QVBoxLayout *layoutPostprocessor = new QVBoxLayout();
    layoutPostprocessor->addWidget(grpContours);
    layoutPostprocessor->addWidget(grpScalarView);
    layoutPostprocessor->addWidget(grpVectorView);
    layoutPostprocessor->addWidget(grpOrder);
    layoutPostprocessor->addWidget(grp3D);
    layoutPostprocessor->addWidget(grpDeformShape);
    layoutPostprocessor->addStretch();

    QWidget *widgetPostprocessor = new QWidget(this);
    widgetPostprocessor->setLayout(layoutPostprocessor);

    QTabWidget *tabType = new QTabWidget(this);
    tabType->addTab(widgetGeneral, icon(""), tr("General"));
    tabType->addTab(widgetPostprocessor, icon(""), tr("Postprocessor"));

    return tabType;
}

QWidget *ConfigDialog::createSolverWidget()
{
    logMessage("ConfigDialog::createSolverWidget()");

    QWidget *solverWidget = new QWidget(this);

    // solver
    chkDeleteTriangleMeshFiles = new QCheckBox(tr("Delete files with initial mesh (Triangle)"));
    chkDeleteHermes2DMeshFile = new QCheckBox(tr("Delete files with solution mesh (Hermes2D)"));
#ifdef BETA
    chkSaveWithSolution = new QCheckBox(tr("Save problem with solution"));
#endif
    chkShowConvergenceChart = new QCheckBox(tr("Show convergence chart after solving"));

    QVBoxLayout *layoutSolver = new QVBoxLayout();
    layoutSolver->addWidget(chkDeleteTriangleMeshFiles);
    layoutSolver->addWidget(chkDeleteHermes2DMeshFile);
#ifdef BETA
    layoutSolver->addWidget(chkSaveWithSolution);
#endif
    layoutSolver->addWidget(chkShowConvergenceChart);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    // mesh
    txtMeshAngleSegmentsCount = new QSpinBox(this);
    txtMeshAngleSegmentsCount->setMinimum(2);
    txtMeshAngleSegmentsCount->setMaximum(20);
    chkMeshCurvilinearElements = new QCheckBox(tr("Curvilinear elements"));

    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->addWidget(new QLabel(tr("Angle segments count:")), 0, 0);
    layoutMesh->addWidget(txtMeshAngleSegmentsCount, 0, 1);
    layoutMesh->addWidget(chkMeshCurvilinearElements, 1, 0, 1, 2);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh"));
    grpMesh->setLayout(layoutMesh);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpSolver);
    layout->addWidget(grpMesh);
    layout->addStretch();

    solverWidget->setLayout(layout);

    return solverWidget;
}

QWidget *ConfigDialog::createColorsWidget()
{
    logMessage("ConfigDialog::createColorsWidget()");

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

QWidget *ConfigDialog::createAdvancedWidget()
{
    logMessage("ConfigDialog::createAdvancedWidget()");

    QWidget *viewWidget = new QWidget(this);

    // adaptivity
    chkIsoOnly = new QCheckBox(tr("Isotropic refinement"));
    lblIsoOnly = new QLabel(tr("<table>"
                               "<tr><td><b>true</b><td><td>isotropic refinement</td></tr>"
                               "<tr><td><b>false</b><td><td>anisotropic refinement</td></tr>"
                               "</table>"));
    txtConvExp = new SLineEditDouble();
    lblConvExp = new QLabel(tr("<b></b>default value is 1.0, this parameter influences<br/>the selection of cancidates in hp-adaptivity"));
    txtThreshold = new SLineEditDouble();
    lblThreshold = new QLabel(tr("<b></b>quantitative parameter of the adapt(...) function<br/>with different meanings for various adaptive strategies"));
    cmbStrategy = new QComboBox();
    cmbStrategy->addItem(tr("0"), 0);
    cmbStrategy->addItem(tr("1"), 1);
    cmbStrategy->addItem(tr("2"), 2);
    lblStrategy = new QLabel(tr("<table>"
                                 "<tr><td><b>0</b><td><td>refine elements until sqrt(<b>threshold</b>)<br/>times total error is processed.<br/>If more elements have similar errors,<br/>refine all to keep the mesh symmetric</td></tr>"
                                 "<tr><td><b>1</b><td><td>refine all elements<br/>whose error is larger than <b>threshold</b><br/>times maximum element error</td></tr>"
                                 "<tr><td><b>2</b><td><td>refine all elements<br/>whose error is larger than <b>threshold</b></td></tr>"
                                 "</table>"));
    cmbMeshRegularity = new QComboBox();
    cmbMeshRegularity->addItem(tr("arbitrary level hang. nodes"), -1);
    cmbMeshRegularity->addItem(tr("at most one-level hang. nodes"), 1);
    cmbMeshRegularity->addItem(tr("at most two-level hang. nodes"), 2);
    cmbMeshRegularity->addItem(tr("at most three-level hang. nodes"), 3);
    cmbMeshRegularity->addItem(tr("at most four-level hang. nodes"), 4);
    cmbMeshRegularity->addItem(tr("at most five-level hang. nodes"), 5);

    cmbProjNormType = new QComboBox();
    cmbProjNormType->addItem(errorNormString(HERMES_H1_NORM), HERMES_H1_NORM);
    cmbProjNormType->addItem(errorNormString(HERMES_L2_NORM), HERMES_L2_NORM);
    cmbProjNormType->addItem(errorNormString(HERMES_H1_SEMINORM), HERMES_H1_SEMINORM);

    QGridLayout *layoutAdaptivity = new QGridLayout();
    // layoutAdaptivity->addWidget(chkIsoOnly, 0, 0);
    // layoutAdaptivity->addWidget(lblIsoOnly, 1, 0, 1, 2);
    layoutAdaptivity->addWidget(new QLabel(tr("Conv. exp.:")), 2, 0);
    layoutAdaptivity->addWidget(txtConvExp, 2, 1);
    layoutAdaptivity->addWidget(lblConvExp, 3, 0, 1, 2);
    layoutAdaptivity->addWidget(new QLabel(tr("Strategy:")), 4, 0);
    layoutAdaptivity->addWidget(cmbStrategy, 4, 1);
    layoutAdaptivity->addWidget(lblStrategy, 5, 0, 1, 2);
    layoutAdaptivity->addWidget(new QLabel(tr("Threshold:")), 6, 0);
    layoutAdaptivity->addWidget(txtThreshold, 6, 1);
    layoutAdaptivity->addWidget(lblThreshold, 7, 0, 1, 2);
    layoutAdaptivity->addWidget(new QLabel(tr("Mesh regularity:")), 8, 0);
    layoutAdaptivity->addWidget(cmbMeshRegularity, 8, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Norm:")), 9, 0);
    layoutAdaptivity->addWidget(cmbProjNormType, 9, 1);

    QGroupBox *grpAdaptivity = new QGroupBox(tr("Adaptivity"));
    grpAdaptivity->setLayout(layoutAdaptivity);

    // commands
    txtArgumentTriangle = new QLineEdit("");
    txtArgumentFFmpeg = new QLineEdit("");

    QGridLayout *layoutArgument = new QGridLayout();
    layoutArgument->addWidget(new QLabel(tr("Triangle")), 0, 0);
    layoutArgument->addWidget(txtArgumentTriangle, 1, 0);
    layoutArgument->addWidget(new QLabel(tr("FFmpeg")), 2, 0);
    layoutArgument->addWidget(txtArgumentFFmpeg, 3, 0);

    QGroupBox *grpArgument = new QGroupBox(tr("Commands"));
    grpArgument->setLayout(layoutArgument);

    // default
    QPushButton *btnDefault = new QPushButton(tr("Default"));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(doAdvancedDefault()));

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpAdaptivity);
    layout->addWidget(grpArgument);
    layout->addStretch();
    layout->addWidget(btnDefault, 0, Qt::AlignLeft);

    viewWidget->setLayout(layout);

    return viewWidget;
}

QWidget *ConfigDialog::createGlobalScriptWidget()
{
    logMessage("ConfigDialog::createGlobalScriptWidget()");

    QWidget *viewWidget = new QWidget(this);

    txtGlobalScript = new ScriptEditor(this);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtGlobalScript);

    viewWidget->setLayout(layout);

    return viewWidget;
}

void ConfigDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    logMessage("ConfigDialog::doCurrentItemChanged()");

    pages->setCurrentIndex(lstView->row(current));
}

void ConfigDialog::doAccept()
{
    logMessage("ConfigDialog::doAccept()");

    save();
    accept();
}

void ConfigDialog::doReject()
{
    logMessage("ConfigDialog::ConfigDialog()");

    reject();
}

void ConfigDialog::doPaletteFilter(int state)
{
    logMessage("ConfigDialog::doPaletteFilter()");

    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}

void ConfigDialog::doClearCommandHistory()
{
    logMessage("ConfigDialog::doClearCommandHistory()");

    QSettings settings;
    settings.setValue("CommandDialog/RecentCommands", QStringList());
    QStringListModel *model = dynamic_cast<QStringListModel *>(Util::completer()->model());
    model->setStringList(QStringList());

    QMessageBox::information(QApplication::activeWindow(), tr("Information"), tr("Command history was cleared succesfully."));
}

void ConfigDialog::doAdvancedDefault()
{
    logMessage("ConfigDialog::doAdvancedDefault()");

    // adaptivity
    chkIsoOnly->setChecked(ADAPTIVITY_ISOONLY);
    txtConvExp->setValue(ADAPTIVITY_CONVEXP);
    txtThreshold->setValue(ADAPTIVITY_THRESHOLD);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(ADAPTIVITY_STRATEGY));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(ADAPTIVITY_MESHREGULARITY));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(ADAPTIVITY_PROJNORMTYPE));

    // command argument
    txtArgumentTriangle->setText(COMMANDS_TRIANGLE);
    txtArgumentFFmpeg->setText(COMMANDS_FFMPEG);
}

void ConfigDialog::doColorsDefault()
{
    logMessage("ConfigDialog::doColorsDefault()");

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

void ConfigDialog::doScalarFieldLog(int state)
{
    logMessage("ConfigDialog::doScalarFieldLog()");

    txtScalarFieldRangeBase->setEnabled(chkScalarFieldRangeLog->isChecked());
}

// *******************************************************************************************************

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    logMessage("ColorButton::ColorButton()");

    setAutoFillBackground(false);
    setCursor(Qt::PointingHandCursor);
    connect(this, SIGNAL(clicked()), this, SLOT(doClicked()));
}

ColorButton::~ColorButton()
{
    logMessage("ColorButton::~ColorButton()");
}

void ColorButton::setColor(const QColor &color)
{
    logMessage("ColorButton::setColor()");

    m_color = color;
    repaint();
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    logMessage("ColorButton::paintEvent()");

    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(m_color);
    painter.setBrush(m_color);
    painter.drawRect(rect());
}

void ColorButton::doClicked()
{
    logMessage("ColorButton::doClicked()");

    QColor color = QColorDialog::getColor(m_color);

    if (color.isValid())
    {
        setColor(color);
    }
}
