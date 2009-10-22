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
    txtFrequency = new SLineEdit("0", true, this);

    QGridLayout *layoutProblem = new QGridLayout();
    layoutProblem->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutProblem->addWidget(txtName, 0, 1);
    layoutProblem->addWidget(new QLabel(tr("Date:")), 1, 0);
    layoutProblem->addWidget(dtmDate, 1, 1);
    layoutProblem->addWidget(new QLabel(tr("Problem type:")), 2, 0);
    layoutProblem->addWidget(cmbProblemType, 2, 1);
    layoutProblem->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    layoutProblem->addWidget(cmbPhysicField, 3, 1);
    layoutProblem->addWidget(new QLabel(tr("Frequency:")), 4, 0);
    layoutProblem->addWidget(txtFrequency, 4, 1);
    layoutProblem->addWidget(new QLabel(tr("Number of refinements:")), 5, 0);
    layoutProblem->addWidget(txtNumberOfRefinements, 5, 1);
    layoutProblem->addWidget(new QLabel(tr("Polynomial order:")), 6, 0);
    layoutProblem->addWidget(txtPolynomialOrder, 6, 1);
    layoutProblem->addWidget(new QLabel(tr("Adaptivity:")), 7, 0);
    layoutProblem->addWidget(cmbAdaptivityType, 7, 1);
    layoutProblem->addWidget(new QLabel(tr("Adaptivity steps:")), 8, 0);
    layoutProblem->addWidget(txtAdaptivitySteps, 8, 1);
    layoutProblem->addWidget(new QLabel(tr("Adaptivity tolerance:")), 9, 0);
    layoutProblem->addWidget(txtAdaptivityTolerance, 9, 1);

    connect(cmbPhysicField, SIGNAL(currentIndexChanged(int)), this, SLOT(doPhysicFieldChanged(int)));
    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    fillComboBox();

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

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_NONE), ADAPTIVITYTYPE_NONE);
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_H), ADAPTIVITYTYPE_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_P), ADAPTIVITYTYPE_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(ADAPTIVITYTYPE_HP), ADAPTIVITYTYPE_HP);

    cmbPhysicField->setEnabled(m_isNewProblem);
}

void ProblemDialog::load() {
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(m_problemInfo->physicField()));
    txtName->setText(m_problemInfo->name);
    cmbProblemType->setCurrentIndex(cmbProblemType->findData(m_problemInfo->problemType));
    dtmDate->setDate(m_problemInfo->date);
    txtNumberOfRefinements->setValue(m_problemInfo->numberOfRefinements);
    txtPolynomialOrder->setValue(m_problemInfo->polynomialOrder);
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_problemInfo->adaptivityType));
    txtAdaptivitySteps->setValue(m_problemInfo->adaptivitySteps);
    txtAdaptivityTolerance->setValue(m_problemInfo->adaptivityTolerance);
    txtFrequency->setValue(m_problemInfo->frequency);
}

void ProblemDialog::save() {
    if (this->m_isNewProblem) m_problemInfo->hermes = hermesFieldFactory((PhysicField) cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toInt());
    m_problemInfo->problemType = (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt();
    m_problemInfo->name = txtName->text();
    m_problemInfo->date = dtmDate->date();
    m_problemInfo->numberOfRefinements = txtNumberOfRefinements->value();
    m_problemInfo->polynomialOrder = txtPolynomialOrder->value();
    m_problemInfo->adaptivityType = (AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt();
    m_problemInfo->adaptivitySteps = txtAdaptivitySteps->value();
    m_problemInfo->adaptivityTolerance = txtAdaptivityTolerance->value();
    m_problemInfo->frequency = txtFrequency->value();
}

void ProblemDialog::doAccept()
{
    save();
    accept();
}

void ProblemDialog::doReject()
{
    reject();
}

void ProblemDialog::doPhysicFieldChanged(int index)
{
    txtFrequency->setEnabled(Util::scene()->problemInfo().hermes->hasFrequency());

}

void ProblemDialog::doAdaptivityChanged(int index)
{
    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != ADAPTIVITYTYPE_NONE);
    txtAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != ADAPTIVITYTYPE_NONE);
}
