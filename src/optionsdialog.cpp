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

#include "optionsdialog.h"

OptionsDialog::OptionsDialog(SceneViewSettings *sceneViewSettings, QWidget *parent) : QDialog(parent)
{
    m_sceneViewSettings = sceneViewSettings;

    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));
    
    createControls();

    load();    

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void OptionsDialog::load()
{
    QSettings settings;

    // gui style
    for (int i = 0; i < cmbGUIStyle->count(); i++) {
        if (cmbGUIStyle->itemText(i) == settings.value("General/GUIStyle"))
        {
            cmbGUIStyle->setCurrentIndex(i);
            break;
        }
    }

    // language
    for (int i = 0; i < cmbLanguage->count(); i++) {
        if (cmbLanguage->itemText(i) == settings.value("General/Language")) {
            cmbLanguage->setCurrentIndex(i);
            break;
        }
    }

    // check version
    chkCheckVersion->setChecked(settings.value("General/CheckVersion", true).value<bool>());

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(settings.value("General/LineEditValueShowResult", false).value<bool>());

    // geometry
    txtGeometryAngleSegmentsCount->setValue(settings.value("Geometry/AngleSegmentsCount", 5).value<int>());
    txtGeometryNodeSize->setValue(m_sceneViewSettings->geometryNodeSize);
    txtGeometryEdgeWidth->setValue(m_sceneViewSettings->geometryEdgeWidth);
    txtGeometryLabelSize->setValue(m_sceneViewSettings->geometryLabelSize);

    // delete files
    chkDeleteTriangleMeshFiles->setChecked(settings.value("Solver/DeleteTriangleMeshFiles", true).value<bool>());
    chkDeleteHermes2DMeshFile->setChecked(settings.value("Solver/DeleteHermes2DMeshFile", true).value<bool>());

    // save problem with solution
#ifdef BETA
    chkSaveWithSolution->setChecked(settings.value("Solver/SaveProblemWithSolution", false).value<bool>());
#endif

    // colors
    colorBackground->setColor(m_sceneViewSettings->colorBackground);
    colorGrid->setColor(m_sceneViewSettings->colorGrid);
    colorCross->setColor(m_sceneViewSettings->colorCross);
    colorNodes->setColor(m_sceneViewSettings->colorNodes);
    colorEdges->setColor(m_sceneViewSettings->colorEdges);
    colorLabels->setColor(m_sceneViewSettings->colorLabels);
    colorContours->setColor(m_sceneViewSettings->colorContours);
    colorVectors->setColor(m_sceneViewSettings->colorVectors);
    colorInitialMesh->setColor(m_sceneViewSettings->colorInitialMesh);
    colorSolutionMesh->setColor(m_sceneViewSettings->colorSolutionMesh);
    colorHighlighted->setColor(m_sceneViewSettings->colorHighlighted);
    colorSelected->setColor(m_sceneViewSettings->colorSelected);

    // grid
    txtGridStep->setText(QString::number(m_sceneViewSettings->gridStep));
    chkSnapToGrid->setChecked(m_sceneViewSettings->snapToGrid);
    chkRulers->setChecked(m_sceneViewSettings->showRulers);

    // contours
    txtContoursCount->setValue(m_sceneViewSettings->contoursCount);

    // scalar field
    cmbPalette->setCurrentIndex(cmbPalette->findData(m_sceneViewSettings->paletteType));
    chkPaletteFilter->setChecked(m_sceneViewSettings->paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(m_sceneViewSettings->paletteSteps);    
    chkScalarFieldRangeLog->setChecked(m_sceneViewSettings->scalarRangeLog);
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setText(QString::number(m_sceneViewSettings->scalarRangeBase));

    // vector field
    chkVectorProportional->setChecked(m_sceneViewSettings->vectorProportional);
    chkVectorColor->setChecked(m_sceneViewSettings->vectorColor);
    txtVectorCount->setValue(m_sceneViewSettings->vectorCount);
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(m_sceneViewSettings->vectorScale);

    // 3d
    chkView3DLighting->setChecked(m_sceneViewSettings->scalarView3DLighting);

    // adaptivity
    chkIsoOnly->setChecked(settings.value("Adaptivity/IsoOnly", ADAPTIVITY_ISOONLY).value<bool>());
    txtConvExp->setValue(settings.value("Adaptivity/ConvExp", ADAPTIVITY_CONVEXP).value<double>());
    txtThreshold->setValue(settings.value("Adaptivity/Threshold", ADAPTIVITY_THRESHOLD).value<double>());
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(settings.value("Adaptivity/Strategy", ADAPTIVITY_STRATEGY).value<int>()));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(settings.value("Adaptivity/MeshRegularity", ADAPTIVITY_MESHREGULARITY).value<int>()));

    // command argument
    txtArgumentTriangle->setText(settings.value("Commands/Triangle", COMMANDS_TRIANGLE).toString());
    txtArgumentFFmpeg->setText(settings.value("Commands/FFmpeg", COMMANDS_FFMPEG).toString());
}

void OptionsDialog::save()
{
    QSettings settings;
    // gui style
    settings.setValue("General/GUIStyle", cmbGUIStyle->currentText());
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (settings.value("General/Language", QLocale::system().name()) != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                                 tr("Language change"),
                                 tr("Interface language has been changed. You must restart the application."));
    settings.setValue("General/Language", cmbLanguage->currentText());

    // check version
    settings.setValue("General/CheckVersion", chkCheckVersion->isChecked());

    // show result in line edit value widget
    settings.setValue("General/LineEditValueShowResult", chkLineEditValueShowResult->isChecked());

    // geometry
    settings.setValue("Geometry/AngleSegmentsCount", txtGeometryAngleSegmentsCount->value());
    m_sceneViewSettings->geometryNodeSize = txtGeometryNodeSize->value();
    m_sceneViewSettings->geometryEdgeWidth = txtGeometryEdgeWidth->value();
    m_sceneViewSettings->geometryLabelSize = txtGeometryLabelSize->value();

    // delete files
    settings.setValue("Solver/DeleteTriangleMeshFiles", chkDeleteTriangleMeshFiles->isChecked());
    settings.setValue("Solver/DeleteHermes2DMeshFile", chkDeleteHermes2DMeshFile->isChecked());

    // save problem with solution
#ifdef BETA
    settings.setValue("Solver/SaveProblemWithSolution", chkSaveWithSolution->isChecked());
#endif

    // color
    m_sceneViewSettings->colorBackground = colorBackground->color();
    m_sceneViewSettings->colorGrid = colorGrid->color();
    m_sceneViewSettings->colorCross = colorCross->color();
    m_sceneViewSettings->colorNodes = colorNodes->color();
    m_sceneViewSettings->colorEdges = colorEdges->color();
    m_sceneViewSettings->colorLabels = colorLabels->color();
    m_sceneViewSettings->colorContours = colorContours->color();
    m_sceneViewSettings->colorVectors = colorVectors->color();
    m_sceneViewSettings->colorInitialMesh = colorInitialMesh->color();
    m_sceneViewSettings->colorSolutionMesh = colorSolutionMesh->color();
    m_sceneViewSettings->colorHighlighted = colorHighlighted->color();
    m_sceneViewSettings->colorSelected = colorSelected->color();

    // grid
    m_sceneViewSettings->gridStep = txtGridStep->text().toDouble();
    m_sceneViewSettings->showRulers = chkRulers->isChecked();
    m_sceneViewSettings->snapToGrid = chkSnapToGrid->isChecked();

    // contours
    m_sceneViewSettings->contoursCount = txtContoursCount->value();

    // scalar field
    m_sceneViewSettings->paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    m_sceneViewSettings->paletteFilter = chkPaletteFilter->isChecked();
    m_sceneViewSettings->paletteSteps = txtPaletteSteps->value();
    m_sceneViewSettings->scalarRangeLog = chkScalarFieldRangeLog->isChecked();
    m_sceneViewSettings->scalarRangeBase = txtScalarFieldRangeBase->text().toDouble();

    // vector field
    m_sceneViewSettings->vectorProportional = chkVectorProportional->isChecked();
    m_sceneViewSettings->vectorColor = chkVectorColor->isChecked();
    m_sceneViewSettings->vectorCount = txtVectorCount->value();
    m_sceneViewSettings->vectorScale = txtVectorScale->value();

    // 3d
    m_sceneViewSettings->scalarView3DLighting = chkView3DLighting->isChecked();

    // save
    m_sceneViewSettings->save();

    // adaptivity
    settings.setValue("Adaptivity/IsoOnly", chkIsoOnly->isChecked());
    settings.setValue("Adaptivity/ConvExp", txtConvExp->value());
    settings.setValue("Adaptivity/Threshold", txtThreshold->value());
    settings.setValue("Adaptivity/Strategy", cmbStrategy->itemData(cmbStrategy->currentIndex()).toInt());
    settings.setValue("Adaptivity/MeshRegularity", cmbMeshRegularity->itemData(cmbMeshRegularity->currentIndex()).toInt());

    // command argument
    settings.setValue("Commands/Triangle", txtArgumentTriangle->text());
    settings.setValue("Commands/FFmpeg", txtArgumentFFmpeg->text());
}

void OptionsDialog::createControls()
{    
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panView = createViewWidget();
    panColors = createColorsWidget();
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

    QListWidgetItem *itemColors = new QListWidgetItem(icon("options-colors"), tr("Colors"), lstView);
    itemColors->setTextAlignment(Qt::AlignHCenter);
    itemColors->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemAdvanced    = new QListWidgetItem(icon("options-advanced"), tr("Advanced"), lstView);
    itemAdvanced->setTextAlignment(Qt::AlignHCenter);
    itemAdvanced->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    pages->addWidget(panMain);
    pages->addWidget(panView);
    pages->addWidget(panColors);
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

QWidget *OptionsDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    // general
    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutGeneral->addWidget(cmbGUIStyle, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutGeneral->addWidget(cmbLanguage, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // solver layout
    chkDeleteTriangleMeshFiles = new QCheckBox(tr("Delete files with initial mesh (Triangle)"));
    chkDeleteHermes2DMeshFile = new QCheckBox(tr("Delete files with solution mesh (Hermes2D)"));
#ifdef BETA
    chkSaveWithSolution = new QCheckBox(tr("Save problem with solution"));
#endif

    QVBoxLayout *layoutSolver = new QVBoxLayout();
    layoutSolver->addWidget(chkDeleteTriangleMeshFiles);
    layoutSolver->addWidget(chkDeleteHermes2DMeshFile);
#ifdef BETA
    layoutSolver->addWidget(chkSaveWithSolution);
#endif

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    // other layout
    cmdClearCommandHistory = new QPushButton(mainWidget);
    cmdClearCommandHistory->setText(tr("Clear command history"));
    connect(cmdClearCommandHistory, SIGNAL(clicked()), this, SLOT(doClearCommandHistory()));

    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));
    chkCheckVersion = new QCheckBox(tr("Check new version during startup."));

    QHBoxLayout *layoutClearCommandHistory = new QHBoxLayout();
    layoutClearCommandHistory->addWidget(cmdClearCommandHistory);    
    layoutClearCommandHistory->addStretch();

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkCheckVersion);
    layoutOther->addLayout(layoutClearCommandHistory);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpSolver);
    layout->addWidget(grpOther);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *OptionsDialog::createViewWidget()
{
    QWidget *viewWidget = new QWidget(this);

    // geometry
    txtGeometryAngleSegmentsCount = new QSpinBox(this);
    txtGeometryAngleSegmentsCount->setMinimum(1);
    txtGeometryAngleSegmentsCount->setMaximum(100);
    txtGeometryNodeSize = new SLineEditDouble();
    txtGeometryEdgeWidth = new SLineEditDouble();
    txtGeometryLabelSize = new SLineEditDouble();

    QGridLayout *layoutGeometry = new QGridLayout();
    layoutGeometry->addWidget(new QLabel(tr("Angle segments count:")), 0, 0);
    layoutGeometry->addWidget(txtGeometryAngleSegmentsCount, 0, 1);
    layoutGeometry->addWidget(new QLabel(tr("Node size:")), 1, 0);
    layoutGeometry->addWidget(txtGeometryNodeSize, 1, 1);
    layoutGeometry->addWidget(new QLabel(tr("Edge width:")), 2, 0);
    layoutGeometry->addWidget(txtGeometryEdgeWidth, 2, 1);
    layoutGeometry->addWidget(new QLabel(tr("Label size:")), 3, 0);
    layoutGeometry->addWidget(txtGeometryLabelSize, 3, 1);

    QGroupBox *grpGeometry = new QGroupBox(tr("Geometry"));
    grpGeometry->setLayout(layoutGeometry);

    // layout grid
    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkRulers = new QCheckBox(tr("Show rulers"));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(new QLabel(tr("Grid step:")), 0, 0);
    layoutGrid->addWidget(txtGridStep, 0, 1);
    layoutGrid->addWidget(chkSnapToGrid, 2, 0);
    layoutGrid->addWidget(chkRulers, 1, 0);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

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

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Palette:")), 0, 0);
    layoutScalarField->addWidget(cmbPalette, 0, 1, 1, 2);

    layoutScalarField->addWidget(new QLabel(tr("Steps:")), 1, 0);
    layoutScalarField->addWidget(txtPaletteSteps, 1, 1);
    layoutScalarField->addWidget(chkPaletteFilter, 1, 2);
    layoutScalarField->addWidget(new QLabel(tr("Base:")), 2, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeBase, 2, 1);
    layoutScalarField->addWidget(chkScalarFieldRangeLog, 2, 2);

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
    txtVectorCount->setMaximum(100);
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

    QHBoxLayout *layout3D = new QHBoxLayout();
    layout3D->addWidget(chkView3DLighting);

    QGroupBox *grp3D = new QGroupBox(tr("3D"));
    grp3D->setLayout(layout3D);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeometry);
    layout->addWidget(grpGrid);
    layout->addWidget(grpContours);
    layout->addWidget(grpScalarView);
    layout->addWidget(grpVectorView);
    layout->addWidget(grp3D);
    layout->addStretch();

    viewWidget->setLayout(layout);

    return viewWidget;
}

QWidget *OptionsDialog::createColorsWidget()
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

    QGroupBox *grpColor = new QGroupBox(tr("Colors"));
    grpColor->setLayout(layoutColors);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpColor);
    layout->addStretch();

    colorsWidget->setLayout(layout);

    return colorsWidget;
}

QWidget *OptionsDialog::createAdvancedWidget()
{
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

    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->addWidget(chkIsoOnly, 0, 0);
    layoutAdaptivity->addWidget(lblIsoOnly, 1, 0, 1, 2);
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

void OptionsDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    pages->setCurrentIndex(lstView->row(current));
}

void OptionsDialog::doAccept()
{
    save();

    accept();
}

void OptionsDialog::doReject()
{
    reject();
}

void OptionsDialog::doPaletteFilter(int state)
{
    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}

void OptionsDialog::doClearCommandHistory()
{
    QSettings settings;
    settings.setValue("CommandDialog/RecentCommands", QStringList());
    QStringListModel *model = dynamic_cast<QStringListModel *>(Util::completer()->model());
    model->setStringList(QStringList());

    QMessageBox::information(QApplication::activeWindow(), tr("Information"), tr("Command history was cleared succesfully."));
}

void OptionsDialog::doAdvancedDefault()
{
    // adaptivity
    chkIsoOnly->setChecked(ADAPTIVITY_ISOONLY);
    txtConvExp->setValue(ADAPTIVITY_CONVEXP);
    txtThreshold->setValue(ADAPTIVITY_THRESHOLD);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(ADAPTIVITY_STRATEGY));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(ADAPTIVITY_MESHREGULARITY));

    // command argument
    txtArgumentTriangle->setText(COMMANDS_TRIANGLE);
    txtArgumentFFmpeg->setText(COMMANDS_FFMPEG);
}

void OptionsDialog::doScalarFieldLog(int state)
{
    txtScalarFieldRangeBase->setEnabled(chkScalarFieldRangeLog->isChecked());
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

