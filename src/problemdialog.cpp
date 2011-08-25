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

#include "problemdialog.h"

#include "gui.h"
#include "scene.h"
#include "scenesolution.h"
#include "scripteditordialog.h"
#include "hermes2d/module.h"

ProblemDialog::ProblemDialog(ProblemInfo *problemInfo, bool isNewProblem, QWidget *parent) : QDialog(parent)
{
    logMessage("ProblemDialog::ProblemDialog()");

    m_isNewProblem = isNewProblem;
    m_problemInfo = problemInfo;

    setWindowTitle(tr("Problem properties"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

ProblemDialog::~ProblemDialog()
{
    logMessage("ProblemDialog::~ProblemDialog()");

    delete txtName;
    delete cmbProblemType;
    if (m_isNewProblem) delete cmbPhysicField;
    delete dtmDate;
    delete txtNumberOfRefinements;
    delete txtPolynomialOrder;
    delete txtAdaptivitySteps;
    delete txtAdaptivityTolerance;
    delete cmbAdaptivityType;

    // harmonic
    delete txtFrequency;

    // transient
    delete txtTransientTimeStep;
    delete txtTransientTimeTotal;
    delete txtTransientInitialCondition;
    delete lblTransientSteps;

    delete txtStartupScript;
    delete txtDescription;
}

int ProblemDialog::showDialog()
{
    logMessage("ProblemDialog::showDialog()");

    return exec();
}

void ProblemDialog::createControls()
{
    logMessage("ProblemDialog::createControls()");

    // tab
    QTabWidget *tabType = new QTabWidget();
    tabType->addTab(createControlsGeneral(), icon(""), tr("General"));
    tabType->addTab(createControlsStartupScript(), icon(""), tr("Startup script"));
    tabType->addTab(createControlsDescription(), icon(""), tr("Description"));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ProblemDialog::createControlsGeneral()
{
    logMessage("ProblemDialog::createControlsGeneral()");

    // equations
    lblEquationPixmap = new QLabel("");
    lblEquationPixmap->setMinimumHeight(50);

    // problem
    cmbProblemType = new QComboBox();
    cmbPhysicField = new QComboBox();
    txtName = new QLineEdit("");
    dtmDate = new QDateTimeEdit();
    dtmDate->setDisplayFormat("dd.MM.yyyy");
    dtmDate->setCalendarPopup(true);
    cmbAdaptivityType = new QComboBox();
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivityTolerance = new SLineEditDouble(1);
    cmbMatrixSolver = new QComboBox();

    // mesh
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(5);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);
    cmbMeshType = new QComboBox();

    // harmonic
    txtFrequency = new SLineEditDouble();

    // transient
    cmbAnalysisType = new QComboBox();
    txtTransientTimeStep = new ValueLineEdit();
    txtTransientTimeTotal = new ValueLineEdit();
    txtTransientInitialCondition = new ValueLineEdit();
    lblTransientSteps = new QLabel("0");

    // weak forms
    cmbWeakForms = new QComboBox();

    // linearity
    txtNonlinearSteps = new QSpinBox(this);
    txtNonlinearSteps->setMinimum(1);
    txtNonlinearSteps->setMaximum(100);
    txtNonlinearTolerance = new SLineEditDouble(1);

    connect(txtTransientTimeStep, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));

    connect(cmbPhysicField, SIGNAL(currentIndexChanged(int)), this, SLOT(doPhysicFieldChanged(int)));
    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));

    // fill combobox
    fillComboBox();

    int minWidth = 130;

    // table
    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->setColumnMinimumWidth(0, minWidth);
    layoutTable->setColumnStretch(1, 1);
    layoutTable->addWidget(new QLabel(tr("Date:")), 2, 0);
    layoutTable->addWidget(dtmDate, 2, 1);
    layoutTable->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    layoutTable->addWidget(cmbPhysicField, 3, 1);
    layoutTable->addWidget(new QLabel(tr("Problem type:")), 4, 0);
    layoutTable->addWidget(cmbProblemType, 4, 1);
    layoutTable->addWidget(new QLabel(tr("Type of analysis:")), 5, 0);
    layoutTable->addWidget(cmbAnalysisType, 5, 1);
    layoutTable->addWidget(new QLabel(tr("Adaptivity:")), 6, 0);
    layoutTable->addWidget(cmbAdaptivityType, 6, 1);
    layoutTable->addWidget(new QLabel(tr("Linear solver:")), 8, 0);
    layoutTable->addWidget(cmbMatrixSolver, 8, 1);
    layoutTable->addWidget(new QLabel(tr("Weak forms:")), 9, 0);
    layoutTable->addWidget(cmbWeakForms, 9, 1);

    // harmonic analysis
    QGridLayout *layoutHarmonicAnalysis = new QGridLayout();
    layoutHarmonicAnalysis->setColumnMinimumWidth(0, minWidth);
    layoutHarmonicAnalysis->addWidget(new QLabel(tr("Frequency (Hz):")), 0, 0);
    layoutHarmonicAnalysis->addWidget(txtFrequency, 0, 1);

    QGroupBox *grpHarmonicAnalysis = new QGroupBox(tr("Harmonic analysis"));
    grpHarmonicAnalysis->setLayout(layoutHarmonicAnalysis);

    // harmonic analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, minWidth);
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Time step (s):")), 0, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeStep, 0, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Total time (s):")), 1, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeTotal, 1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial condition:")), 2, 0);
    layoutTransientAnalysis->addWidget(txtTransientInitialCondition, 2, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Steps:")), 3, 0);
    layoutTransientAnalysis->addWidget(lblTransientSteps, 3, 1);

    QGroupBox *grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // harmonic analysis
    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->setColumnMinimumWidth(0, minWidth);
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(new QLabel(tr("Mesh type:")), 0, 0);
    layoutMesh->addWidget(cmbMeshType, 0, 1);
    layoutMesh->addWidget(new QLabel(tr("Number of refinements:")), 1, 0);
    layoutMesh->addWidget(txtNumberOfRefinements, 1, 1);
    layoutMesh->addWidget(new QLabel(tr("Polynomial order:")), 2, 0);
    layoutMesh->addWidget(txtPolynomialOrder, 2, 1);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh parameters"));
    grpMesh->setLayout(layoutMesh);

    // adaptivity
    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, minWidth);
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Adaptivity steps:")), 0, 0);
    layoutAdaptivity->addWidget(txtAdaptivitySteps, 0, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Adaptivity tolerance (%):")), 1, 0);
    layoutAdaptivity->addWidget(txtAdaptivityTolerance, 1, 1);

    QGroupBox *grpAdaptivity = new QGroupBox(tr("Adaptivity"));
    grpAdaptivity->setLayout(layoutAdaptivity);

    // linearity
    QGridLayout *layoutLinearity = new QGridLayout();
    layoutLinearity->setColumnMinimumWidth(0, minWidth);
    layoutLinearity->setColumnStretch(1, 1);
    layoutLinearity->addWidget(new QLabel(tr("Tolerance (%):")), 0, 0);
    layoutLinearity->addWidget(txtNonlinearTolerance, 0, 1);
    layoutLinearity->addWidget(new QLabel(tr("Steps:")), 1, 0);
    layoutLinearity->addWidget(txtNonlinearSteps, 1, 1);

    QGroupBox *grpLinearity = new QGroupBox(tr("Newton solver"));
    grpLinearity->setLayout(layoutLinearity);
    // grpLinearity->setVisible(Util::config()->showExperimentalFeatures);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addLayout(layoutTable);
    // layoutLeft->addWidget(grpAdaptivity);
    layoutLeft->addStretch();
    layoutLeft->addWidget(grpLinearity);
    layoutLeft->addWidget(grpMesh);

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpHarmonicAnalysis);
    layoutRight->addWidget(grpTransientAnalysis);
    // layoutRight->addWidget(grpMesh);
    layoutRight->addWidget(grpAdaptivity);
    // layoutRight->addWidget(grpLinearity);
    layoutRight->addStretch();

    // both
    QHBoxLayout *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // name
    QGridLayout *layoutName = new QGridLayout();
    layoutName->setColumnMinimumWidth(0, minWidth);
    layoutName->setColumnStretch(1, 1);
    layoutName->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutName->addWidget(txtName, 0, 1);

    // equation
    QGridLayout *layoutEquation = new QGridLayout();
    layoutEquation->setColumnMinimumWidth(0, minWidth);
    layoutEquation->setColumnStretch(1, 1);
    layoutEquation->addWidget(new QLabel(tr("Equation:")), 0, 0);
    layoutEquation->addWidget(lblEquationPixmap, 0, 1, 1, 1, Qt::AlignLeft);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addLayout(layoutName);
    layoutProblem->addLayout(layoutEquation);
    layoutProblem->addLayout(layoutPanel);

    QWidget *widMain = new QWidget();
    widMain->setLayout(layoutProblem);

    return widMain;
}

QWidget *ProblemDialog::createControlsStartupScript()
{
    logMessage("ProblemDialog::createControlsStartupScript()");

    txtStartupScript = new ScriptEditor(this);

    QVBoxLayout *layoutStartup = new QVBoxLayout();
    layoutStartup->addWidget(txtStartupScript);

    QWidget *widStartup = new QWidget();
    widStartup->setLayout(layoutStartup);

    return widStartup;
}

QWidget *ProblemDialog::createControlsDescription()
{
    logMessage("ProblemDialog::createControlsDescription()");

    txtDescription = new QTextEdit(this);
    txtDescription->setAcceptRichText(false);

    QVBoxLayout *layoutDescription = new QVBoxLayout();
    layoutDescription->addWidget(txtDescription);

    QWidget *widDescription = new QWidget();
    widDescription->setLayout(layoutDescription);

    return widDescription;
}

void ProblemDialog::fillComboBox()
{
    logMessage("ProblemDialog::fillComboBox()");

    cmbProblemType->clear();
    cmbProblemType->addItem(problemTypeString(ProblemType_Planar), ProblemType_Planar);
    cmbProblemType->addItem(problemTypeString(ProblemType_Axisymmetric), ProblemType_Axisymmetric);

    fillComboBoxPhysicField(cmbPhysicField);
    //cmbPhysicField->setEnabled(m_isNewProblem);

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_None), AdaptivityType_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_H), AdaptivityType_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_P), AdaptivityType_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_HP), AdaptivityType_HP);

    cmbMeshType->addItem(meshTypeString(MeshType_Triangle), MeshType_Triangle);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadFineDivision), MeshType_QuadFineDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadRoughDivision), MeshType_QuadRoughDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadJoin), MeshType_QuadJoin);

    cmbMatrixSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_UMFPACK), Hermes::SOLVER_UMFPACK);
#ifdef WITH_MUMPS
    cmbMatrixSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_MUMPS), Hermes::SOLVER_MUMPS);
#endif
#ifdef WITH_SUPERLU
    cmbMatrixSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_SUPERLU), Hermes::SOLVER_SUPERLU);
#endif

    cmbWeakForms->clear();
    cmbWeakForms->addItem(weakFormsTypeString(WeakFormsType_Compiled), WeakFormsType_Compiled);
    cmbWeakForms->addItem(weakFormsTypeString(WeakFormsType_Interpreted), WeakFormsType_Interpreted);
}

void ProblemDialog::load()
{
    logMessage("ProblemDialog::load()");

    // main
    if (!m_isNewProblem) cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(QString::fromStdString(m_problemInfo->module()->id)));
    txtName->setText(m_problemInfo->name);
    cmbProblemType->setCurrentIndex(cmbProblemType->findData(m_problemInfo->problemType));
    dtmDate->setDate(m_problemInfo->date);
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_problemInfo->adaptivityType));
    txtAdaptivitySteps->setValue(m_problemInfo->adaptivitySteps);
    txtAdaptivityTolerance->setValue(m_problemInfo->adaptivityTolerance);
    //mesh
    txtNumberOfRefinements->setValue(m_problemInfo->numberOfRefinements);
    txtPolynomialOrder->setValue(m_problemInfo->polynomialOrder);
    cmbMeshType->setCurrentIndex(cmbMeshType->findData(m_problemInfo->meshType));
    // harmonic magnetic
    txtFrequency->setValue(m_problemInfo->frequency);
    // transient
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(m_problemInfo->analysisType));
    txtTransientTimeStep->setValue(m_problemInfo->timeStep);
    txtTransientTimeTotal->setValue(m_problemInfo->timeTotal);
    txtTransientInitialCondition->setValue(m_problemInfo->initialCondition);

    // linearity
    txtNonlinearSteps->setValue(m_problemInfo->nonlinearSteps);
    txtNonlinearTolerance->setValue(m_problemInfo->nonlinearTolerance);

    // matrix solver
    cmbMatrixSolver->setCurrentIndex(cmbMatrixSolver->findData(m_problemInfo->matrixSolver));

    // weakforms
    cmbWeakForms->setCurrentIndex(cmbWeakForms->findData(m_problemInfo->weakFormsType));

    // startup
    txtStartupScript->setPlainText(m_problemInfo->scriptStartup);

    // description
    txtDescription->setPlainText(m_problemInfo->description);

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
    doTransientChanged();
}

    bool ProblemDialog::save()
{
    logMessage("ProblemDialog::save()");

    // physical field type
    if (Util::scene()->problemInfo()->module()->id !=
            cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString())
    {
        if (!this->m_isNewProblem)
        {
            if (Util::scene()->boundaries.count() != 1 || Util::scene()->materials.count() != 1)
            {
                if (QMessageBox::question(this, tr("Change physical field type"), tr("Are you sure change physical field type?"), tr("&Yes"), tr("&No")) == 1)
                    return false;
            }
        }

        if (Util::scene()->sceneSolution()->isSolved())
            Util::scene()->doClearSolution();

        m_problemInfo->setModule(moduleFactory(cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString(),
                                               (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt(),
                                               (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt()));

        for (int i = 1; i < Util::scene()->boundaries.count(); i++)
        {
            Util::scene()->replaceBoundary(Util::scene()->boundaries[1]);
        }

        for (int i = 1; i < Util::scene()->materials.count(); i++)
        {
            Util::scene()->replaceMaterial(Util::scene()->materials[1]);
        }
    }
    else
    {
        m_problemInfo->setModule(moduleFactory(cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString(),
                                               (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt(),
                                               (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt()));
    }

    // check values
    if (cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt() == AnalysisType_Harmonic)
    {
        if (txtFrequency->value() < 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Frequency cannot be negative."));
            return false;
        }
    }
    if (cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt() == AnalysisType_Transient)
    {
        txtTransientTimeStep->evaluate(false);
        if (txtTransientTimeStep->number() <= 0.0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Time step must be positive."));
            return false;
        }
        txtTransientTimeTotal->evaluate(false);
        if (txtTransientTimeTotal->number() <= 0.0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Total time must be positive."));
            return false;
        }
        txtTransientTimeStep->evaluate(false);
        if (txtTransientTimeStep->number() > txtTransientTimeTotal->number())
        {
            QMessageBox::critical(this, tr("Error"), tr("Time step is greater then total time."));
            return false;
        }
    }

    // run and check startup script
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        ScriptResult scriptResult = runPythonScript(txtStartupScript->toPlainText());
        if (scriptResult.isError)
        {
            QMessageBox::critical(QApplication::activeWindow(), QObject::tr("Error"), scriptResult.text);
            return false;
        }
    }

    // save properties
    m_problemInfo->problemType = (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt();
    m_problemInfo->name = txtName->text();
    m_problemInfo->date = dtmDate->date();
    m_problemInfo->numberOfRefinements = txtNumberOfRefinements->value();
    m_problemInfo->polynomialOrder = txtPolynomialOrder->value();
    m_problemInfo->meshType = (MeshType) cmbMeshType->itemData(cmbMeshType->currentIndex()).toInt();
    m_problemInfo->adaptivityType = (AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt();
    m_problemInfo->adaptivitySteps = txtAdaptivitySteps->value();
    m_problemInfo->adaptivityTolerance = txtAdaptivityTolerance->value();

    m_problemInfo->frequency = txtFrequency->value();

    m_problemInfo->analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();
    m_problemInfo->timeStep = txtTransientTimeStep->value();
    m_problemInfo->timeTotal = txtTransientTimeTotal->value();
    m_problemInfo->initialCondition = txtTransientInitialCondition->value();

    m_problemInfo->description = txtDescription->toPlainText();
    m_problemInfo->scriptStartup = txtStartupScript->toPlainText();

    m_problemInfo->nonlinearSteps = txtNonlinearSteps->value();
    m_problemInfo->nonlinearTolerance = txtNonlinearTolerance->value();

    // matrix solver
    m_problemInfo->matrixSolver = (Hermes::MatrixSolverType) cmbMatrixSolver->itemData(cmbMatrixSolver->currentIndex()).toInt();

    m_problemInfo->weakFormsType = (WeakFormsType) cmbWeakForms->itemData(cmbWeakForms->currentIndex()).toInt();

    return true;
}

void ProblemDialog::doAccept()
{
    logMessage("ProblemDialog::doAccept()");

    if (save()) accept();
}

void ProblemDialog::doReject()
{
    logMessage("ProblemDialog::doReject()");

    reject();
}

void ProblemDialog::doPhysicFieldChanged(int index)
{
    logMessage("ProblemDialog::doPhysicFieldChanged()");

    Hermes::Module::ModuleAgros *module = moduleFactory(cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString(),
                                                        ProblemType_Planar, AnalysisType_SteadyState);

    // analysis type
    // store analysis type
    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();

    cmbAnalysisType->clear();
    if (module->steady_state_solutions) cmbAnalysisType->addItem(analysisTypeString(AnalysisType_SteadyState), AnalysisType_SteadyState);
    if (module->harmonic_solutions) cmbAnalysisType->addItem(analysisTypeString(AnalysisType_Harmonic), AnalysisType_Harmonic);
    if (module->transient_solutions) cmbAnalysisType->addItem(analysisTypeString(AnalysisType_Transient), AnalysisType_Transient);

    // restore analysis type
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(analysisType));
    if (cmbAnalysisType->currentIndex() == -1) cmbAnalysisType->setCurrentIndex(0);
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());

    delete module;

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

void ProblemDialog::doAdaptivityChanged(int index)
{
    logMessage("ProblemDialog::doAdaptivityChanged()");

    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
}

void ProblemDialog::doAnalysisTypeChanged(int index)
{
    logMessage("ProblemDialog::doAnalysisTypeChanged()");

    txtTransientTimeStep->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);
    txtTransientTimeTotal->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);
    txtTransientInitialCondition->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);

    txtFrequency->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Harmonic);

    doShowEquation();
}

void ProblemDialog::doTransientChanged()
{
    logMessage("ProblemDialog::doTransientChanged()");

    if (txtTransientTimeStep->evaluate(true) &&
            txtTransientTimeTotal->evaluate(true))
    {
        lblTransientSteps->setText(QString("%1").arg(floor(txtTransientTimeTotal->number()/txtTransientTimeStep->number())));
    }
}

void ProblemDialog::doShowEquation()
{
    readPixmap(lblEquationPixmap,
               QString(":/images/equations/%1/%1_%2.png")
               .arg(cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString())
               .arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));
}
