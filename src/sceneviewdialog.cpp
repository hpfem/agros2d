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

#include "gui.h"

#include "scene.h"
#include "sceneview.h"

SceneViewDialog::SceneViewDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    logMessage("SceneViewDialog::SceneViewDialog()");

    m_sceneView = sceneView;

    setWindowIcon(icon("scene-properties"));
    setWindowTitle(tr("Postprocessor properties"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

int SceneViewDialog::showDialog()
{
    logMessage("SceneViewDialog::showDialog()");

    return exec();
}

void SceneViewDialog::load()
{
    logMessage("SceneViewDialog::load()");

    // show
    chkShowGeometry->setChecked(m_sceneView->sceneViewSettings().showGeometry);
    chkShowInitialMesh->setChecked(m_sceneView->sceneViewSettings().showInitialMesh);

    radPostprocessorNone->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_None);
    radPostprocessorScalarField->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView);
    radPostprocessorScalarField3D->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3D);
    radPostprocessorScalarField3DSolid->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid);
    radPostprocessorModel->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_Model);
    radPostprocessorOrder->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_Order);
    buttonClicked(butPostprocessorGroup->checkedButton());

    chkShowContours->setChecked(m_sceneView->sceneViewSettings().showContours);
    chkShowVectors->setChecked(m_sceneView->sceneViewSettings().showVectors);
    chkShowSolutionMesh->setChecked(m_sceneView->sceneViewSettings().showSolutionMesh);
    connect(chkShowVectors, SIGNAL(clicked()), this, SLOT(setControls()));

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

void SceneViewDialog::save()
{
    logMessage("SceneViewDialog::save()");

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

    // time step
    QApplication::processEvents();
    Util::scene()->sceneSolution()->setTimeStep(cmbTimeStep->currentIndex());
}

void SceneViewDialog::createControls()
{
    logMessage("SceneViewDialog::createControls()");

    // layout show
    chkShowGeometry = new QCheckBox(tr("Geometry"));
    chkShowInitialMesh = new QCheckBox(tr("Initial mesh"));

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
    connect(butPostprocessorGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));

    // postprocessor 2d
    QVBoxLayout *layoutPostprocessorMode2D = new QVBoxLayout();
    layoutPostprocessorMode2D->addWidget(new QLabel(tr("2D mode")));
    layoutPostprocessorMode2D->addWidget(radPostprocessorNone);
    layoutPostprocessorMode2D->addWidget(radPostprocessorOrder);
    layoutPostprocessorMode2D->addWidget(radPostprocessorScalarField);
    layoutPostprocessorMode2D->addStretch();

    QVBoxLayout *layoutPostprocessorMode3D = new QVBoxLayout();
    layoutPostprocessorMode3D->addWidget(new QLabel(tr("3D mode")));
    layoutPostprocessorMode3D->addWidget(radPostprocessorScalarField3D);
    layoutPostprocessorMode3D->addWidget(radPostprocessorScalarField3DSolid);
    layoutPostprocessorMode3D->addWidget(radPostprocessorModel);
    layoutPostprocessorMode3D->addStretch();

    // postprocessor show
    chkShowContours = new QCheckBox(tr("Contours"));
    chkShowVectors = new QCheckBox(tr("Vectors"));
    chkShowSolutionMesh = new QCheckBox(tr("Solution mesh"));

    QVBoxLayout *layoutShow = new QVBoxLayout();
    layoutShow->addWidget(chkShowGeometry);
    layoutShow->addWidget(chkShowInitialMesh);
    layoutShow->addWidget(chkShowSolutionMesh);
    layoutShow->addWidget(chkShowContours);
    layoutShow->addWidget(chkShowVectors);
    layoutShow->addStretch();

    QGroupBox *grpShow = new QGroupBox(tr("Show"));
    grpShow->setLayout(layoutShow);

    // postprocessor
    QHBoxLayout *layoutPostprocessor = new QHBoxLayout();
    layoutPostprocessor->addLayout(layoutPostprocessorMode2D);
    layoutPostprocessor->addLayout(layoutPostprocessorMode3D);

    QGroupBox *grpPostprocessor = new QGroupBox(tr("Postprocessor"));
    grpPostprocessor->setLayout(layoutPostprocessor);

    // layout scalar field
    cmbScalarFieldVariable = new QComboBox();
    fillComboBoxScalarVariable(cmbScalarFieldVariable);
    connect(cmbScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));

    cmbScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox();
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    txtScalarFieldRangeMin = new SLineEditDouble(0.1, true);
    connect(txtScalarFieldRangeMin, SIGNAL(textEdited(const QString &)), this, SLOT(doScalarFieldRangeMinChanged()));
    lblScalarFieldRangeMinError = new QLabel("");
    lblScalarFieldRangeMinError->setPalette(palette);
    txtScalarFieldRangeMax = new SLineEditDouble(0.1, true);
    connect(txtScalarFieldRangeMax, SIGNAL(textEdited(const QString &)), this, SLOT(doScalarFieldRangeMaxChanged()));
    lblScalarFieldRangeMaxError = new QLabel("");
    lblScalarFieldRangeMaxError->setPalette(palette);

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariable, 0, 1);

    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariableComp, 1, 1);

    layoutScalarField->addWidget(new QLabel(tr("Auto range:")), 2, 0);
    layoutScalarField->addWidget(chkScalarFieldRangeAuto, 2, 1);
    layoutScalarField->addWidget(new QLabel(tr("Minimum:")), 3, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeMin, 3, 1);
    layoutScalarField->addWidget(lblScalarFieldRangeMinError, 3, 2);
    layoutScalarField->addWidget(new QLabel(tr("Maximum:")), 4, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeMax, 4, 1);
    layoutScalarField->addWidget(lblScalarFieldRangeMaxError, 4, 2);

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
//    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
//    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    btnOK = new QPushButton(tr("Ok"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doAccept()));

    btnCancel = new QPushButton(tr("Cancel"));
    connect(btnCancel, SIGNAL(clicked()), SLOT(doReject()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnOK);
    layoutButtons->addWidget(btnCancel);

    QHBoxLayout *layoutShowPostprocessor = new QHBoxLayout();
    layoutShowPostprocessor->addWidget(grpShow);
    layoutShowPostprocessor->addWidget(grpPostprocessor);
    layoutShowPostprocessor->addStretch();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutShowPostprocessor);
    layout->addWidget(grpScalarField);
    layout->addLayout(layoutVectorFieldTransient);
    layout->addStretch();
    layout->addLayout(layoutButtons);

    setControls();

    setLayout(layout);
}

void SceneViewDialog::doScalarFieldVariable(int index)
{
    logMessage("SceneViewDialog::doScalarFieldVariable()");

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

void SceneViewDialog::doScalarFieldRangeAuto(int state)
{
    logMessage("SceneViewDialog::doScalarFieldRangeAuto()");

    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());
}

void SceneViewDialog::setControls()
{
    logMessage("SceneViewDialog::setControls()");

    // disable controls
    chkShowGeometry->setEnabled(true);

    chkShowInitialMesh->setEnabled(false);
    chkShowSolutionMesh->setEnabled(false);
    chkShowContours->setEnabled(false);
    chkShowVectors->setEnabled(false);

    radPostprocessorNone->setEnabled(false);
    radPostprocessorScalarField->setEnabled(false);
    radPostprocessorOrder->setEnabled(false);
    radPostprocessorScalarField3D->setEnabled(false);
    radPostprocessorScalarField3DSolid->setEnabled(false);
    radPostprocessorModel->setEnabled(false);

    cmbScalarFieldVariable->setEnabled(false);
    cmbScalarFieldVariableComp->setEnabled(false);
    cmbVectorFieldVariable->setEnabled(false);

    chkScalarFieldRangeAuto->setEnabled(false);
    txtScalarFieldRangeMin->setEnabled(false);
    txtScalarFieldRangeMax->setEnabled(false);
    cmbTimeStep->setEnabled(false);

    if (Util::scene()->sceneSolution()->isMeshed() && !Util::scene()->sceneSolution()->isSolved())
    {
        chkShowInitialMesh->setEnabled(true);
    }

    if (Util::scene()->sceneSolution()->isSolved())
    {
        radPostprocessorNone->setEnabled(true);
        radPostprocessorScalarField->setEnabled(true);
        radPostprocessorOrder->setEnabled(true);
        radPostprocessorScalarField3D->setEnabled(true);
        radPostprocessorScalarField3DSolid->setEnabled(true);
        radPostprocessorModel->setEnabled(true);

        cmbTimeStep->setEnabled(Util::scene()->sceneSolution()->timeStepCount() > 0);

        if (radPostprocessorScalarField->isChecked() ||
            radPostprocessorScalarField3D->isChecked() ||
            radPostprocessorScalarField3DSolid->isChecked())
        {
            cmbScalarFieldVariable->setEnabled(true);
            cmbScalarFieldVariableComp->setEnabled(true);

            chkScalarFieldRangeAuto->setEnabled(true);
            doScalarFieldRangeAuto(-1);
        }

        if (radPostprocessorNone->isChecked() ||
            radPostprocessorOrder->isChecked() ||
            radPostprocessorScalarField->isChecked())
        {
            chkShowGeometry->setEnabled(true);
            chkShowInitialMesh->setEnabled(true);
            chkShowSolutionMesh->setEnabled(true);
            chkShowContours->setEnabled(cmbScalarFieldVariable->count() > 0);
            chkShowVectors->setEnabled(cmbVectorFieldVariable->count() > 0);

            cmbVectorFieldVariable->setEnabled(chkShowVectors->isChecked());
        }
        else
        {
            chkShowGeometry->setEnabled(false);
        }
    }
}

void SceneViewDialog::buttonClicked(QAbstractButton *button)
{
    logMessage("SceneViewDialog::buttonClicked()");

    setControls();
}

void SceneViewDialog::doAccept()
{
    logMessage("SceneViewDialog::doAccept()");

    save();

    accept();
}

void SceneViewDialog::doReject()
{
    logMessage("SceneViewDialog::doReject()");

    reject();
}

void SceneViewDialog::doScalarFieldRangeMinChanged()
{
    lblScalarFieldRangeMinError->clear();
    lblScalarFieldRangeMaxError->clear();
    btnOK->setEnabled(true);

    if (txtScalarFieldRangeMin->value() > txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("> %1").arg(txtScalarFieldRangeMax->value()));
        btnOK->setDisabled(true);
    }
    else if (txtScalarFieldRangeMin->value() == txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("= %1").arg(txtScalarFieldRangeMax->value()));
        btnOK->setDisabled(true);
    }
}

void SceneViewDialog::doScalarFieldRangeMaxChanged()
{
    lblScalarFieldRangeMaxError->clear();
    lblScalarFieldRangeMinError->clear();
    btnOK->setEnabled(true);

    if (txtScalarFieldRangeMax->value() < txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("< %1").arg(txtScalarFieldRangeMin->value()));
        btnOK->setDisabled(true);
    }
    else if (txtScalarFieldRangeMax->value() == txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("= %1").arg(txtScalarFieldRangeMin->value()));
        btnOK->setDisabled(true);
    }
}
