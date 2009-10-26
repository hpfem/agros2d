#include "problemdialog.h"

ProblemDialog::ProblemDialog(ProblemInfo &problemInfo, bool isNewProblem, QWidget *parent) : QDialog(parent)
{
    m_isNewProblem = isNewProblem;
    m_problemInfo = &problemInfo;

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
    delete txtDescription;
    // harmonic magnetic
    delete txtFrequency;
    // transient
    delete txtTransientTimeStep;
    delete txtTransientTimeTotal;
    delete txtTransientInitialCondition;
    delete lblTransientSteps;
}

int ProblemDialog::showDialog()
{
    return exec();
}

void ProblemDialog::createControls()
{
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
    txtAdaptivityTolerance = new SLineEdit("1", true, this);
    // txtDescription = new QTextEdit(this);
    // txtDescription->setLineWidth(2);
    // harmonic magnetic
    txtFrequency = new SLineEdit("0", true, this);
    // transient
    cmbAnalysisType = new QComboBox();
    txtTransientTimeStep = new SLineEdit("0", true, false, this);
    txtTransientTimeTotal = new SLineEdit("0", true, false, this);
    txtTransientInitialCondition = new SLineEdit("0", true, false, this);
    lblTransientSteps = new QLabel("0");

    connect(txtTransientTimeStep, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));

    connect(cmbPhysicField, SIGNAL(currentIndexChanged(int)), this, SLOT(doPhysicFieldChanged(int)));
    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));
    fillComboBox();

    QGridLayout *layoutProblem = new QGridLayout();
    // top
    layoutProblem->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutProblem->addWidget(txtName, 1, 1, 1, 3);
    // left
    layoutProblem->addWidget(new QLabel(tr("Date:")), 2, 0);
    layoutProblem->addWidget(dtmDate, 2, 1);
    layoutProblem->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    layoutProblem->addWidget(cmbPhysicField, 3, 1);
    layoutProblem->addWidget(new QLabel(tr("Problem type:")), 4, 0);
    layoutProblem->addWidget(cmbProblemType, 4, 1);
    layoutProblem->addWidget(new QLabel(tr("Number of refinements:")), 5, 0);
    layoutProblem->addWidget(txtNumberOfRefinements, 5, 1);
    layoutProblem->addWidget(new QLabel(tr("Polynomial order:")), 6, 0);
    layoutProblem->addWidget(txtPolynomialOrder, 6, 1);
    layoutProblem->addWidget(new QLabel(tr("Adaptivity:")), 7, 0);
    layoutProblem->addWidget(cmbAdaptivityType, 7, 1);
    layoutProblem->addWidget(new QLabel(tr("Adaptivity steps:")), 8, 0);
    layoutProblem->addWidget(txtAdaptivitySteps, 8, 1);
    layoutProblem->addWidget(new QLabel(tr("Adaptivity tolerance (%):")), 9, 0);
    layoutProblem->addWidget(txtAdaptivityTolerance, 9, 1);
    // right
    layoutProblem->addWidget(new QLabel(tr("Frequency (Hz):")), 2, 2);
    layoutProblem->addWidget(txtFrequency, 2, 3);
    layoutProblem->addWidget(new QLabel(tr("Type of analysis:")), 3, 2);
    layoutProblem->addWidget(cmbAnalysisType, 3, 3);
    layoutProblem->addWidget(new QLabel(tr("Time step (s):")), 4, 2);
    layoutProblem->addWidget(txtTransientTimeStep, 4, 3);
    layoutProblem->addWidget(new QLabel(tr("Total time (s):")), 5, 2);
    layoutProblem->addWidget(txtTransientTimeTotal, 5, 3);
    layoutProblem->addWidget(new QLabel(tr("Initial condition:")), 6, 2);
    layoutProblem->addWidget(txtTransientInitialCondition, 6, 3);
    layoutProblem->addWidget(new QLabel(tr("Steps:")), 7, 2);
    layoutProblem->addWidget(lblTransientSteps, 7, 3);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutProblem);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ProblemDialog::fillComboBox()
{
    cmbProblemType->clear();
    cmbProblemType->addItem(problemTypeString(PROBLEMTYPE_PLANAR), PROBLEMTYPE_PLANAR);
    cmbProblemType->addItem(problemTypeString(PROBLEMTYPE_AXISYMMETRIC), PROBLEMTYPE_AXISYMMETRIC);

    cmbPhysicField->clear();
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_GENERAL), PHYSICFIELD_GENERAL);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_ELECTROSTATIC), PHYSICFIELD_ELECTROSTATIC);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_MAGNETOSTATIC), PHYSICFIELD_MAGNETOSTATIC);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_HARMONICMAGNETIC), PHYSICFIELD_HARMONICMAGNETIC);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_CURRENT), PHYSICFIELD_CURRENT);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_HEAT), PHYSICFIELD_HEAT);
    // cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_ELASTICITY), PHYSICFIELD_ELASTICITY);
    cmbPhysicField->setEnabled(m_isNewProblem);

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_NONE), ADAPTIVITYTYPE_NONE);
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_H), ADAPTIVITYTYPE_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_P), ADAPTIVITYTYPE_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_HP), ADAPTIVITYTYPE_HP);

    cmbAnalysisType->clear();
    cmbAnalysisType->addItem(analysisTypeString(ANALYSISTYPE_STEADYSTATE), ANALYSISTYPE_STEADYSTATE);
    cmbAnalysisType->addItem(analysisTypeString(ANALYSISTYPE_TRANSIENT), ANALYSISTYPE_TRANSIENT);
    cmbAnalysisType->setCurrentIndex(0);
}

void ProblemDialog::load()
{
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

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
    doTransientChanged();
}

bool ProblemDialog::save()
{
    if (this->m_isNewProblem) m_problemInfo->hermes = hermesFieldFactory((PhysicField) cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toInt());

    // check values
    if (m_problemInfo->hermes->hasFrequency())
    {
        if (txtFrequency->value() < 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Frequency cannot be negative."));
            return false;
        }
    }

    if (m_problemInfo->hermes->hasTransient() && cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toBool())
    {
        if (txtTransientTimeStep->value() <= 0.0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Time step must be positive."));
            return false;
        }
        if (txtTransientTimeTotal->value()<= 0.0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Total time must be positive."));
            return false;
        }
        if (txtTransientTimeStep->value() > txtTransientTimeTotal->value())
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

    txtFrequency->setEnabled(hermesField->hasFrequency());
    cmbAnalysisType->setEnabled(hermesField->hasTransient());
    cmbAnalysisType->setCurrentIndex(0);
    txtTransientTimeStep->setEnabled(Util::scene()->problemInfo().hermes->hasTransient());
    txtTransientTimeTotal->setEnabled(Util::scene()->problemInfo().hermes->hasTransient());
    txtTransientInitialCondition->setEnabled(Util::scene()->problemInfo().hermes->hasTransient());

    delete hermesField;

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

void ProblemDialog::doAdaptivityChanged(int index)
{
    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != ADAPTIVITYTYPE_NONE);
    txtAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != ADAPTIVITYTYPE_NONE);
}

void ProblemDialog::doAnalysisTypeChanged(int index)
{
    txtTransientTimeStep->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == ANALYSISTYPE_TRANSIENT);
    txtTransientTimeTotal->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == ANALYSISTYPE_TRANSIENT);
    txtTransientInitialCondition->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == ANALYSISTYPE_TRANSIENT);
}

void ProblemDialog::doTransientChanged()
{
    lblTransientSteps->setText(QString("%1").arg(floor(txtTransientTimeTotal->value()/txtTransientTimeStep->value())));
}
