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

#include "sceneviewdialog.h"

SceneViewDialog::SceneViewDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;
       
    setWindowIcon(icon("scene-properties"));
    setWindowTitle(tr("Scene View"));

    createControls();
    
    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

SceneViewDialog::~SceneViewDialog()
{
    // show
    delete chkShowGrid;
    delete chkShowGeometry;
    delete chkShowInitialMesh;

    delete butPostprocessorGroup;
    delete radPostprocessorNone;
    delete radPostprocessorScalarField;
    delete radPostprocessorScalarField3D;
    // delete radPostprocessorScalarField3DSolid;
    delete radPostprocessorOrder;

    delete chkShowContours;
    delete chkShowVectors;
    delete chkShowSolutionMesh;

    // scalar field
    delete cmbScalarFieldVariable;
    delete cmbScalarFieldVariableComp;
    delete chkScalarFieldRangeAuto;
    delete txtScalarFieldRangeMin;
    delete txtScalarFieldRangeMax;
    delete chkScalarFieldRangeLog;
    delete txtScalarFieldRangeBase;

    // vector field
    delete cmbVectorFieldVariable;

    // transient
    delete cmbTimeStep;
}

int SceneViewDialog::showDialog()
{
    return exec();
}

void SceneViewDialog::load()
{
    // show
    chkShowGrid->setChecked(m_sceneView->sceneViewSettings().showGrid);
    chkShowGeometry->setChecked(m_sceneView->sceneViewSettings().showGeometry);
    chkShowInitialMesh->setChecked(m_sceneView->sceneViewSettings().showInitialMesh);

    radPostprocessorNone->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_NONE);
    radPostprocessorScalarField->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW);
    radPostprocessorScalarField3D->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D);
    // radPostprocessorScalarField3DSolid->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID);
    radPostprocessorOrder->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_ORDER);

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
    chkScalarFieldRangeLog->setChecked(m_sceneView->sceneViewSettings().scalarRangeLog);
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeBase));

    // vector field
    cmbVectorFieldVariable->setCurrentIndex(cmbVectorFieldVariable->findData(m_sceneView->sceneViewSettings().vectorPhysicFieldVariable));

    // transient view
    cmbTimeStep->setCurrentIndex(Util::scene()->sceneSolution()->timeStep());
}

void SceneViewDialog::save()
{
    // show
    m_sceneView->sceneViewSettings().showGrid = chkShowGrid->isChecked();
    m_sceneView->sceneViewSettings().showGeometry = chkShowGeometry->isChecked();
    m_sceneView->sceneViewSettings().showInitialMesh = chkShowInitialMesh->isChecked();

    if (radPostprocessorNone->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_NONE;
    if (radPostprocessorScalarField->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW;
    if (radPostprocessorScalarField3D->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D;
    // if (radPostprocessorScalarField3DSolid->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID;
    if (radPostprocessorOrder->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_ORDER;

    m_sceneView->sceneViewSettings().showContours = chkShowContours->isChecked();
    m_sceneView->sceneViewSettings().showVectors = chkShowVectors->isChecked();
    m_sceneView->sceneViewSettings().showSolutionMesh = chkShowSolutionMesh->isChecked();

    // scalar field
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(cmbScalarFieldVariable->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = (PhysicFieldVariableComp) cmbScalarFieldVariableComp->itemData(cmbScalarFieldVariableComp->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarRangeAuto = chkScalarFieldRangeAuto->isChecked();
    m_sceneView->sceneViewSettings().scalarRangeMin = txtScalarFieldRangeMin->text().toDouble();
    m_sceneView->sceneViewSettings().scalarRangeMax = txtScalarFieldRangeMax->text().toDouble();
    m_sceneView->sceneViewSettings().scalarRangeLog = chkScalarFieldRangeLog->isChecked();
    m_sceneView->sceneViewSettings().scalarRangeBase = txtScalarFieldRangeBase->text().toDouble();

    // vector field
    m_sceneView->sceneViewSettings().vectorPhysicFieldVariable = (PhysicFieldVariable) cmbVectorFieldVariable->itemData(cmbVectorFieldVariable->currentIndex()).toInt();

    // time step
    Util::scene()->sceneSolution()->setTimeStep(cmbTimeStep->currentIndex());
}

void SceneViewDialog::createControls()
{
    // layout show
    chkShowGrid = new QCheckBox(tr("Grid"));
    chkShowGeometry = new QCheckBox(tr("Geometry"));
    chkShowInitialMesh = new QCheckBox(tr("Initial mesh"));

    QVBoxLayout *layoutShow = new QVBoxLayout();
    layoutShow->addWidget(chkShowGrid);
    layoutShow->addWidget(chkShowGeometry);
    layoutShow->addWidget(chkShowInitialMesh);
    layoutShow->addStretch();

    QGroupBox *grpShow = new QGroupBox(tr("Show"));
    grpShow->setLayout(layoutShow);

    // postprocessor mode
    radPostprocessorNone = new QRadioButton(tr("None"), this);
    radPostprocessorScalarField = new QRadioButton(tr("Scalar view"), this);
    radPostprocessorScalarField3D = new QRadioButton(tr("Scalar view 3D"), this);
    // radPostprocessorScalarField3DSolid = new QRadioButton("Scalar view 3D solid", this);
    radPostprocessorOrder = new QRadioButton(tr("Order"), this);

    butPostprocessorGroup = new QButtonGroup(this);
    butPostprocessorGroup->addButton(radPostprocessorNone);
    butPostprocessorGroup->addButton(radPostprocessorScalarField);
    butPostprocessorGroup->addButton(radPostprocessorScalarField3D);
    // butPostprocessorGroup->addButton(radPostprocessorScalarField3DSolid);
    butPostprocessorGroup->addButton(radPostprocessorOrder);

    QVBoxLayout *layoutPostprocessorMode = new QVBoxLayout();
    layoutPostprocessorMode->addWidget(radPostprocessorNone);
    layoutPostprocessorMode->addWidget(radPostprocessorScalarField);
    layoutPostprocessorMode->addWidget(radPostprocessorScalarField3D);
    // layoutPostprocessorMode->addWidget(radPostprocessorScalarField3DSolid);
    layoutPostprocessorMode->addWidget(radPostprocessorOrder);

    // postprocessor show
    chkShowContours = new QCheckBox(tr("Contours"));
    chkShowVectors = new QCheckBox(tr("Vectors"));
    chkShowSolutionMesh = new QCheckBox(tr("Solution mesh"));

    QVBoxLayout *layoutPostprocessorShow = new QVBoxLayout();
    layoutPostprocessorShow->addWidget(chkShowContours);
    layoutPostprocessorShow->addWidget(chkShowVectors);
    layoutPostprocessorShow->addWidget(chkShowSolutionMesh);
    layoutPostprocessorShow->addStretch();

    // postprocessor
    QHBoxLayout *layoutPostprocessor = new QHBoxLayout();
    layoutPostprocessor->addLayout(layoutPostprocessorMode);
    layoutPostprocessor->addLayout(layoutPostprocessorShow);

    QGroupBox *grpPostprocessor = new QGroupBox(tr("Postprocessor"));
    grpPostprocessor->setLayout(layoutPostprocessor);

    // layout scalar field
    cmbScalarFieldVariable = new QComboBox();
    fillComboBoxScalarVariable(cmbScalarFieldVariable);
    connect(cmbScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));

    cmbScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox();
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    txtScalarFieldRangeMin = new QLineEdit("0.1");
    txtScalarFieldRangeMin->setValidator(new QDoubleValidator(txtScalarFieldRangeMin));
    txtScalarFieldRangeMax = new QLineEdit("0.1");
    txtScalarFieldRangeMax->setValidator(new QDoubleValidator(txtScalarFieldRangeMax));

    // log scale
    chkScalarFieldRangeLog = new QCheckBox("");
    txtScalarFieldRangeBase = new QLineEdit("10");
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariable, 0, 1);

    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariableComp, 1, 1);

    layoutScalarField->addWidget(new QLabel(tr("Auto range:")), 2, 0);
    layoutScalarField->addWidget(chkScalarFieldRangeAuto, 2, 1);
    layoutScalarField->addWidget(new QLabel(tr("Minimum:")), 3, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeMin, 3, 1);
    layoutScalarField->addWidget(new QLabel(tr("Maximum:")), 4, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeMax, 4, 1);

    layoutScalarField->addWidget(new QLabel(tr("Log. scale:")), 5, 0);
    layoutScalarField->addWidget(chkScalarFieldRangeLog, 5, 1);
    layoutScalarField->addWidget(new QLabel(tr("Base:")), 6, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeBase, 6, 1);

    QGroupBox *grpScalarField = new QGroupBox(tr("Scalar field"));
    grpScalarField->setLayout(layoutScalarField);

    // vector field
    cmbVectorFieldVariable = new QComboBox();
    fillComboBoxVectorVariable(cmbVectorFieldVariable);

    QHBoxLayout *layoutVectorField = new QHBoxLayout();
    layoutVectorField->addWidget(new QLabel(tr("Variable:")));
    layoutVectorField->addWidget(cmbVectorFieldVariable);
    layoutVectorField->addStretch();;

    QGroupBox *grpVectorField = new QGroupBox(tr("Vector field"));
    grpVectorField->setLayout(layoutVectorField);

    // transient
    cmbTimeStep = new QComboBox(this);
    fillComboBoxTimeStep(cmbTimeStep);

    QHBoxLayout *layoutTransient = new QHBoxLayout();
    layoutTransient->addWidget(new QLabel(tr("Time step:")));
    layoutTransient->addWidget(cmbTimeStep);
    layoutTransient->addStretch();

    QGroupBox *grpTransient = new QGroupBox(tr("Transient analysis"));
    grpTransient->setLayout(layoutTransient);

    QHBoxLayout *layoutVectorFieldTransient = new QHBoxLayout();
    layoutVectorFieldTransient->addWidget(grpVectorField);
    layoutVectorFieldTransient->addWidget(grpTransient);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QHBoxLayout *layoutShowPostprocessor = new QHBoxLayout();
    layoutShowPostprocessor->addWidget(grpShow);
    layoutShowPostprocessor->addWidget(grpPostprocessor);
    layoutShowPostprocessor->addStretch();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutShowPostprocessor);
    layout->addWidget(grpScalarField);
    layout->addLayout(layoutVectorFieldTransient);
    layout->addStretch();
    layout->addWidget(buttonBox);

    // enable controls
    chkShowInitialMesh->setEnabled(Util::scene()->sceneSolution()->isMeshed());
    chkShowSolutionMesh->setEnabled(Util::scene()->sceneSolution()->isSolved());
    chkShowContours->setEnabled(Util::scene()->sceneSolution()->isSolved() && (cmbScalarFieldVariable->count() > 0));
    chkShowVectors->setEnabled(Util::scene()->sceneSolution()->isSolved() && (cmbVectorFieldVariable->count() > 0));
    cmbScalarFieldVariable->setEnabled(Util::scene()->sceneSolution()->isSolved());
    cmbScalarFieldVariableComp->setEnabled(Util::scene()->sceneSolution()->isSolved());
    cmbVectorFieldVariable->setEnabled(Util::scene()->sceneSolution()->isSolved());
    radPostprocessorNone->setEnabled(Util::scene()->sceneSolution()->isSolved());
    radPostprocessorScalarField->setEnabled(Util::scene()->sceneSolution()->isSolved());
    radPostprocessorScalarField3D->setEnabled(Util::scene()->sceneSolution()->isSolved());
    // radPostprocessorScalarField3DSolid->setEnabled(Util::scene()->sceneSolution()->isSolved());
    radPostprocessorOrder->setEnabled(Util::scene()->sceneSolution()->isSolved());
    chkScalarFieldRangeAuto->setEnabled(Util::scene()->sceneSolution()->isSolved());
    txtScalarFieldRangeMin->setEnabled(Util::scene()->sceneSolution()->isSolved());
    txtScalarFieldRangeMax->setEnabled(Util::scene()->sceneSolution()->isSolved());
    chkScalarFieldRangeLog->setEnabled(Util::scene()->sceneSolution()->isSolved());
    cmbTimeStep->setEnabled(Util::scene()->sceneSolution()->isSolved());

    setLayout(layout);
}

void SceneViewDialog::doScalarFieldVariable(int index)
{
    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbScalarFieldVariableComp->itemData(cmbScalarFieldVariableComp->currentIndex()).toInt();
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(index).toInt();

    cmbScalarFieldVariableComp->clear();
    if (isPhysicFieldVariableScalar(physicFieldVariable))
    {
        cmbScalarFieldVariableComp->addItem(tr("Scalar"), PHYSICFIELDVARIABLECOMP_SCALAR);
    }
    else
    {
        cmbScalarFieldVariableComp->addItem(tr("Magnitude"), PHYSICFIELDVARIABLECOMP_MAGNITUDE);
        cmbScalarFieldVariableComp->addItem(Util::scene()->problemInfo()->labelX(), PHYSICFIELDVARIABLECOMP_X);
        cmbScalarFieldVariableComp->addItem(Util::scene()->problemInfo()->labelY(), PHYSICFIELDVARIABLECOMP_Y);
    }

    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(cmbScalarFieldVariableComp->findData(scalarFieldVariableComp));
    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(0);
}

void SceneViewDialog::doScalarFieldRangeAuto(int state)
{
    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());       
}

void SceneViewDialog::doScalarFieldLog(int state)
{
    txtScalarFieldRangeBase->setEnabled(chkScalarFieldRangeLog->isChecked());
}

void SceneViewDialog::doAccept()
{
    save();

    accept();
}

void SceneViewDialog::doReject()
{
    reject();
}
