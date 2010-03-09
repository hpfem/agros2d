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

ProblemDialog::ProblemDialog(ProblemInfo *problemInfo, bool isNewProblem, QWidget *parent) : QDialog(parent)
{
    m_isNewProblem = isNewProblem;
    m_problemInfo = problemInfo;

    setWindowTitle(tr("Problem properties"));

    createControls();

    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

ProblemDialog::~ProblemDialog()
{
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
    return exec();
}

void ProblemDialog::createControls()
{
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
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(5);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);
    cmbAdaptivityType = new QComboBox();
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivityTolerance = new SLineEditDouble(1);

    // harmonic
    txtFrequency = new SLineEditDouble();

    // transient
    cmbAnalysisType = new QComboBox();
    txtTransientTimeStep = new SLineEditValue();
    txtTransientTimeTotal = new SLineEditValue();
    txtTransientInitialCondition = new SLineEditValue();
    lblTransientSteps = new QLabel("0");

    connect(txtTransientTimeStep, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));

    connect(cmbPhysicField, SIGNAL(currentIndexChanged(int)), this, SLOT(doPhysicFieldChanged(int)));
    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));
    fillComboBox();

    QGridLayout *layoutProblemTable = new QGridLayout();
    // top
    layoutProblemTable->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutProblemTable->addWidget(txtName, 1, 1, 1, 3);
    // left
    layoutProblemTable->addWidget(new QLabel(tr("Date:")), 2, 0);
    layoutProblemTable->addWidget(dtmDate, 2, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    layoutProblemTable->addWidget(cmbPhysicField, 3, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Problem type:")), 4, 0);
    layoutProblemTable->addWidget(cmbProblemType, 4, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Number of refinements:")), 5, 0);
    layoutProblemTable->addWidget(txtNumberOfRefinements, 5, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Polynomial order:")), 6, 0);
    layoutProblemTable->addWidget(txtPolynomialOrder, 6, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Adaptivity:")), 7, 0);
    layoutProblemTable->addWidget(cmbAdaptivityType, 7, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Adaptivity steps:")), 8, 0);
    layoutProblemTable->addWidget(txtAdaptivitySteps, 8, 1);
    layoutProblemTable->addWidget(new QLabel(tr("Adaptivity tolerance (%):")), 9, 0);
    layoutProblemTable->addWidget(txtAdaptivityTolerance, 9, 1);
    // right
    layoutProblemTable->addWidget(new QLabel(tr("Type of analysis:")), 2, 2);
    layoutProblemTable->addWidget(cmbAnalysisType, 2, 3);
    layoutProblemTable->addWidget(new QLabel(tr("Frequency (Hz):")), 3, 2);
    layoutProblemTable->addWidget(txtFrequency, 3, 3);
    layoutProblemTable->addWidget(new QLabel(tr("Time step (s):")), 4, 2);
    layoutProblemTable->addWidget(txtTransientTimeStep, 4, 3);
    layoutProblemTable->addWidget(new QLabel(tr("Total time (s):")), 5, 2);
    layoutProblemTable->addWidget(txtTransientTimeTotal, 5, 3);
    layoutProblemTable->addWidget(new QLabel(tr("Initial condition:")), 6, 2);
    layoutProblemTable->addWidget(txtTransientInitialCondition, 6, 3);
    layoutProblemTable->addWidget(new QLabel(tr("Steps:")), 7, 2);
    layoutProblemTable->addWidget(lblTransientSteps, 7, 3);

    // equation
    QHBoxLayout *layoutEquation = new QHBoxLayout();
    layoutEquation->addWidget(new QLabel(tr("Equation:")));
    layoutEquation->addWidget(lblEquationPixmap, 1);

    // QWidget *widgetEquation = new QWidget(this);
    // widgetEquation->setLayout(widgetEquation);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addLayout(layoutProblemTable);
    // layoutProblem->addWidget();
    layoutProblem->addLayout(layoutEquation);

    QWidget *widMain = new QWidget();
    widMain->setLayout(layoutProblem);

    return widMain;
}

QWidget *ProblemDialog::createControlsStartupScript()
{
    txtStartupScript = new ScriptEditor(this);

    QVBoxLayout *layoutStartup = new QVBoxLayout();
    layoutStartup->addWidget(txtStartupScript);

    QWidget *widStartup = new QWidget();
    widStartup->setLayout(layoutStartup);

    return widStartup;
}

QWidget *ProblemDialog::createControlsDescription()
{
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
    cmbProblemType->clear();
    cmbProblemType->addItem(problemTypeString(ProblemType_Planar), ProblemType_Planar);
    cmbProblemType->addItem(problemTypeString(ProblemType_Axisymmetric), ProblemType_Axisymmetric);

    cmbPhysicField->clear();
    cmbPhysicField->addItem(physicFieldString(PhysicField_General), PhysicField_General);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Electrostatic), PhysicField_Electrostatic);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Magnetic), PhysicField_Magnetic);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Current), PhysicField_Current);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Heat), PhysicField_Heat);
    // cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_ELASTICITY), PHYSICFIELD_ELASTICITY);
    cmbPhysicField->setEnabled(m_isNewProblem);

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_None), AdaptivityType_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_H), AdaptivityType_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_P), AdaptivityType_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_HP), AdaptivityType_HP);
}

void ProblemDialog::load()
{
    // main
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(m_problemInfo->physicField()));
    txtName->setText(m_problemInfo->name);
    cmbProblemType->setCurrentIndex(cmbProblemType->findData(m_problemInfo->problemType));
    dtmDate->setDate(m_problemInfo->date);
    txtNumberOfRefinements->setValue(m_problemInfo->numberOfRefinements);
    txtPolynomialOrder->setValue(m_problemInfo->polynomialOrder);
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_problemInfo->adaptivityType));
    txtAdaptivitySteps->setValue(m_problemInfo->adaptivitySteps);
    txtAdaptivityTolerance->setValue(m_problemInfo->adaptivityTolerance);
    // harmonic magnetic
    txtFrequency->setValue(m_problemInfo->frequency);
    // transient
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(m_problemInfo->analysisType));
    txtTransientTimeStep->setValue(m_problemInfo->timeStep);
    txtTransientTimeTotal->setValue(m_problemInfo->timeTotal);
    txtTransientInitialCondition->setValue(m_problemInfo->initialCondition);

    // startup
    txtStartupScript->setPlainText(m_problemInfo->scriptStartup);

    // description
    txtDescription->setPlainText(m_problemInfo->description);

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
    doTransientChanged();
}

bool ProblemDialog::save()
{
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        ScriptResult scriptResult = runPythonScript(txtStartupScript->toPlainText());
        if (scriptResult.isError)
        {
            QMessageBox::critical(QApplication::activeWindow(), QObject::tr("Error"), scriptResult.text);
            return false;
        }
    }

    if (this->m_isNewProblem) m_problemInfo->setHermes(hermesFieldFactory((PhysicField) cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toInt()));

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

    Util::scene()->sceneSolution()->clear();

    m_problemInfo->problemType = (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt();
    m_problemInfo->name = txtName->text();
    m_problemInfo->date = dtmDate->date();
    m_problemInfo->numberOfRefinements = txtNumberOfRefinements->value();
    m_problemInfo->polynomialOrder = txtPolynomialOrder->value();
    m_problemInfo->adaptivityType = (AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt();
    m_problemInfo->adaptivitySteps = txtAdaptivitySteps->value();
    m_problemInfo->adaptivityTolerance = txtAdaptivityTolerance->value();

    // harmonic magnetic
    m_problemInfo->frequency = txtFrequency->value();

    // transient
    m_problemInfo->analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();
    m_problemInfo->timeStep = txtTransientTimeStep->value();
    m_problemInfo->timeTotal = txtTransientTimeTotal->value();
    m_problemInfo->initialCondition = txtTransientInitialCondition->value();

    // description
    m_problemInfo->description = txtDescription->toPlainText();

    // startup script
    m_problemInfo->scriptStartup = txtStartupScript->toPlainText();

    return true;
}

void ProblemDialog::doAccept()
{
    if (save()) accept();
}

void ProblemDialog::doReject()
{
    reject();
}

void ProblemDialog::doPhysicFieldChanged(int index)
{
    HermesField *hermesField = hermesFieldFactory((PhysicField) cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toInt());

    // analysis type
    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();
    cmbAnalysisType->clear();
    cmbAnalysisType->addItem(analysisTypeString(AnalysisType_SteadyState), AnalysisType_SteadyState);
    if (hermesField->hasHarmonic()) cmbAnalysisType->addItem(analysisTypeString(AnalysisType_Harmonic), AnalysisType_Harmonic);
    if (hermesField->hasTransient()) cmbAnalysisType->addItem(analysisTypeString(AnalysisType_Transient), AnalysisType_Transient);
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(analysisType));
    if (cmbAnalysisType->currentIndex() == -1) cmbAnalysisType->setCurrentIndex(0);
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());

    delete hermesField;

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

void ProblemDialog::doAdaptivityChanged(int index)
{
    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
}

void ProblemDialog::doAnalysisTypeChanged(int index)
{
    txtTransientTimeStep->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);
    txtTransientTimeTotal->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);
    txtTransientInitialCondition->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);

    txtFrequency->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Harmonic);

    doShowEquation();
}

void ProblemDialog::doTransientChanged()
{
    if (txtTransientTimeStep->evaluate(true) &&
        txtTransientTimeTotal->evaluate(true))
    {
        lblTransientSteps->setText(QString("%1").arg(floor(txtTransientTimeTotal->number()/txtTransientTimeStep->number())));
    }
}

void ProblemDialog::doShowEquation()
{    
    QPixmap pixmap;
    pixmap.load(QString(":/images/equations/%1_%2.png")
                .arg(physicFieldToStringKey((PhysicField) cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toInt()))
                .arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));
    lblEquationPixmap->setPixmap(pixmap);
}
