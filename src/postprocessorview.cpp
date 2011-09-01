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

PostprocessorView::PostprocessorView(SceneView *sceneView, QWidget *parent) : QDockWidget(tr("Postprocessor properties"), parent)
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
    radPostprocessorModel->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_Model);
    radPostprocessorOrder->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_Order);
    doPostprocessorGroupClicked(butPostprocessorGroup->checkedButton());

    chkShowContours->setChecked(m_sceneView->sceneViewSettings().showContours);
    chkShowVectors->setChecked(m_sceneView->sceneViewSettings().showVectors);
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

    // contours
    txtContoursCount->setValue(Util::config()->contoursCount);

    // scalar field
    chkShowScalarScale->setChecked(Util::config()->showScalarScale);
    cmbPalette->setCurrentIndex(cmbPalette->findData(Util::config()->paletteType));
    chkPaletteFilter->setChecked(Util::config()->paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(Util::config()->paletteSteps);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(Util::config()->linearizerQuality));

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
    if (radPostprocessorModel->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_Model;
    if (radPostprocessorOrder->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SceneViewPostprocessorShow_Order;

    m_sceneView->sceneViewSettings().showContours = chkShowContours->isChecked();
    m_sceneView->sceneViewSettings().showVectors = chkShowVectors->isChecked();
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

    // contours
    Util::config()->contoursCount = txtContoursCount->value();

    // scalar field
    Util::config()->showScalarScale = chkShowScalarScale->isChecked();
    Util::config()->paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    Util::config()->paletteFilter = chkPaletteFilter->isChecked();
    Util::config()->paletteSteps = txtPaletteSteps->value();
    Util::config()->linearizerQuality = cmbLinearizerQuality->itemData(cmbLinearizerQuality->currentIndex()).toDouble();

    // vector field
    Util::config()->vectorProportional = chkVectorProportional->isChecked();
    Util::config()->vectorColor = chkVectorColor->isChecked();
    Util::config()->vectorCount = txtVectorCount->value();
    Util::config()->vectorScale = txtVectorScale->value();

    // order view
    Util::config()->showOrderScale = chkShowOrderScale->isChecked();
    Util::config()->orderPaletteOrderType = (PaletteOrderType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt();
    Util::config()->orderLabel = chkOrderLabel->isChecked();

    // save
    Util::config()->save();
}

void PostprocessorView::createControls()
{
    logMessage("PostprocessorView::createControls()");

    // tab widget
    basic = controlsBasic();
    advanced = controlsAdvanced();

    QTabWidget *tabType = new QTabWidget(this);
    tabType->addTab(basic, icon(""), tr("Basic"));
    tabType->addTab(advanced, icon(""), tr("Advanced"));

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
    connect(chkShowVectors, SIGNAL(clicked()), this, SLOT(setControls()));
    chkShowSolutionMesh = new QCheckBox(tr("Solution mesh"));

    // postprocessor mode
    radPostprocessorNone = new QRadioButton(tr("None"), this);
    radPostprocessorOrder = new QRadioButton(tr("Polynomial order"), this);
    radPostprocessorScalarField = new QRadioButton(tr("Scalar view"), this);
    radPostprocessorScalarField3D = new QRadioButton(tr("Scalar view"), this);
    radPostprocessorScalarField3DSolid = new QRadioButton(tr("Scalar view solid"), this);
    radPostprocessorModel = new QRadioButton("Model", this);

    butPostprocessorGroup = new QButtonGroup(this);
    butPostprocessorGroup->addButton(radPostprocessorNone);
    butPostprocessorGroup->addButton(radPostprocessorScalarField);
    butPostprocessorGroup->addButton(radPostprocessorOrder);
    butPostprocessorGroup->addButton(radPostprocessorScalarField3D);
    butPostprocessorGroup->addButton(radPostprocessorScalarField3DSolid);
    butPostprocessorGroup->addButton(radPostprocessorModel);
    connect(butPostprocessorGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(doPostprocessorGroupClicked(QAbstractButton*)));

    QGridLayout *layoutShow = new QGridLayout();
    layoutShow->addWidget(chkShowGeometry, 0, 0);
    layoutShow->addWidget(chkShowInitialMesh, 1, 0);
    layoutShow->addWidget(chkShowSolutionMesh, 2, 0);
    layoutShow->addWidget(chkShowContours, 3, 0);
    layoutShow->addWidget(chkShowVectors, 4, 0);
    layoutShow->addWidget(new QLabel(tr("2D:")), 0, 1);
    layoutShow->addWidget(radPostprocessorNone, 0, 2);
    layoutShow->addWidget(radPostprocessorOrder, 1, 2);
    layoutShow->addWidget(radPostprocessorScalarField, 2, 2);
    layoutShow->addWidget(new QLabel(tr("3D:")), 3, 1);
    layoutShow->addWidget(radPostprocessorScalarField3D, 3, 2);
    layoutShow->addWidget(radPostprocessorScalarField3DSolid, 4, 2);
    layoutShow->addWidget(radPostprocessorModel, 5, 2);

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

QWidget *PostprocessorView::controlsAdvanced()
{
    logMessage("PostprocessorView::controlsAdvanced()");

    double minWidth = 110;

    // layout contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(1);
    txtContoursCount->setMaximum(100);

    QGridLayout *layoutContours = new QGridLayout();
    layoutContours->setColumnMinimumWidth(0, minWidth);
    layoutContours->setColumnStretch(1, 1);
    layoutContours->addWidget(new QLabel(tr("Contours count:")), 0, 0);
    layoutContours->addWidget(txtContoursCount, 0, 1);

    QGroupBox *grpContours = new QGroupBox(tr("Contours"));
    grpContours->setLayout(layoutContours);

    // layout palette
    cmbPalette = new QComboBox();
    cmbPalette->addItem(tr("Jet"), Palette_Jet);
    cmbPalette->addItem(tr("Autumn"), Palette_Autumn);
    cmbPalette->addItem(tr("Hot"), Palette_Hot);
    cmbPalette->addItem(tr("Copper"), Palette_Copper);
    cmbPalette->addItem(tr("Cool"), Palette_Cool);
    cmbPalette->addItem(tr("B/W ascending"), Palette_BWAsc);
    cmbPalette->addItem(tr("B/W descending"), Palette_BWDesc);

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

    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(5);
    txtPaletteSteps->setMaximum(100);

    chkShowScalarScale = new QCheckBox(tr("Show scale"), this);

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->setColumnMinimumWidth(0, minWidth);
    layoutScalarField->setColumnStretch(1, 1);
    layoutScalarField->addWidget(new QLabel(tr("Palette:")), 0, 0);
    layoutScalarField->addWidget(cmbPalette, 0, 1, 1, 2);
    layoutScalarField->addWidget(new QLabel(tr("Steps:")), 1, 0);
    layoutScalarField->addWidget(txtPaletteSteps, 1, 1);
    layoutScalarField->addWidget(chkPaletteFilter, 1, 2);
    layoutScalarField->addWidget(new QLabel(tr("Quality:")), 2, 0);
    layoutScalarField->addWidget(cmbLinearizerQuality, 2, 1, 1, 2);
    layoutScalarField->addWidget(chkShowScalarScale, 3, 0, 1, 2);

    QGroupBox *grpScalarView = new QGroupBox(tr("Scalar view"));
    grpScalarView->setLayout(layoutScalarField);

    // vector field
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

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->setColumnMinimumWidth(0, minWidth);
    layoutVectorField->setColumnStretch(1, 1);
    layoutVectorField->addWidget(new QLabel(tr("Vectors:")), 0, 0);
    layoutVectorField->addWidget(txtVectorCount, 0, 1);
    layoutVectorField->addWidget(chkVectorProportional, 0, 2);
    layoutVectorField->addWidget(new QLabel(tr("Scale:")), 1, 0);
    layoutVectorField->addWidget(txtVectorScale, 1, 1);
    layoutVectorField->addWidget(chkVectorColor, 1, 2);

    QGroupBox *grpVectorView = new QGroupBox(tr("Vector view"));
    grpVectorView->setLayout(layoutVectorField);

    // layout order
    cmbOrderPaletteOrder = new QComboBox();
    cmbOrderPaletteOrder->addItem(tr("Hermes"), PaletteOrder_Hermes);
    cmbOrderPaletteOrder->addItem(tr("Jet"), PaletteOrder_Jet);
    cmbOrderPaletteOrder->addItem(tr("B/W ascending"), PaletteOrder_BWAsc);
    cmbOrderPaletteOrder->addItem(tr("B/W descending"), PaletteOrder_BWDesc);

    chkShowOrderScale = new QCheckBox(tr("Show scale"), this);
    chkOrderLabel = new QCheckBox(tr("Show order labels"), this);

    QGridLayout *layoutOrder = new QGridLayout();
    layoutOrder->setColumnMinimumWidth(0, minWidth);
    layoutOrder->setColumnStretch(1, 1);
    layoutOrder->addWidget(new QLabel(tr("Palette:")), 0, 0);
    layoutOrder->addWidget(cmbOrderPaletteOrder, 0, 1);
    layoutOrder->addWidget(chkShowOrderScale, 1, 0, 1, 2);
    layoutOrder->addWidget(chkOrderLabel, 2, 0, 1, 2);

    QGroupBox *grpOrder = new QGroupBox(tr("Polynomial order"));
    grpOrder->setLayout(layoutOrder);

    // default
    QPushButton *btnDefault = new QPushButton(tr("Default"));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(doDefault()));

    // layout postprocessor
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpContours);
    layout->addWidget(grpScalarView);
    layout->addWidget(grpVectorView);
    layout->addWidget(grpOrder);
    layout->addStretch();
    layout->addWidget(btnDefault, 0, Qt::AlignLeft);

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

    // disable controls
    advanced->setEnabled(isSolved);

    chkShowGeometry->setEnabled(true);

    chkShowInitialMesh->setEnabled(isMeshed);
    chkShowSolutionMesh->setEnabled(isSolved && (cmbScalarFieldVariable->count() > 0));
    chkShowContours->setEnabled(isSolved);
    chkShowVectors->setEnabled(isSolved && (cmbVectorFieldVariable->count() > 0));

    radPostprocessorNone->setEnabled(isSolved);
    radPostprocessorScalarField->setEnabled(isSolved);
    radPostprocessorOrder->setEnabled(isSolved);
    radPostprocessorScalarField3D->setEnabled(isSolved);
    radPostprocessorScalarField3DSolid->setEnabled(isSolved);
    radPostprocessorModel->setEnabled(isSolved);

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
                     radPostprocessorScalarField3DSolid->isChecked() ||
                     radPostprocessorModel->isChecked()))
    {
        chkShowGeometry->setChecked(true);
        chkShowGeometry->setEnabled(false);
        chkShowInitialMesh->setEnabled(false);
        chkShowSolutionMesh->setEnabled(false);
        chkShowContours->setEnabled(false);
        chkShowVectors->setEnabled(false);
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

void PostprocessorView::doDefault()
{
    logMessage("PostprocessorView::doDefault()");

    // countour
    txtContoursCount->setValue(CONTOURSCOUNT);

    // scalar view
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) PALETTETYPE));
    chkPaletteFilter->setChecked(PALETTEFILTER);
    txtPaletteSteps->setValue(PALETTESTEPS);
    cmbLinearizerQuality->setCurrentIndex(cmbLinearizerQuality->findData(LINEARIZER_QUALITY));

    // vector view
    chkVectorProportional->setChecked(VECTORPROPORTIONAL);
    chkVectorColor->setChecked(VECTORCOLOR);
    txtVectorCount->setValue(VECTORNUMBER);
    txtVectorScale->setValue(VECTORSCALE);

    // order view
    chkOrderLabel->setChecked(ORDERLABEL);
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) ORDERPALETTEORDERTYPE));
}

void PostprocessorView::doScalarFieldRangeMinChanged()
{
    lblScalarFieldRangeMinError->clear();
    lblScalarFieldRangeMaxError->clear();
    btnOK->setEnabled(true);

    if (txtScalarFieldRangeMin->value() > txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("> %1").arg(txtScalarFieldRangeMax->value()));
        btnOK->setDisabled(true);
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
        btnOK->setDisabled(true);
    }
    /*
    else if (txtScalarFieldRangeMax->value() == txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("= %1").arg(txtScalarFieldRangeMin->value()));
        btnOK->setDisabled(true);
    }
    */
}
