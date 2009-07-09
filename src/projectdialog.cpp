#include "projectdialog.h"

ProjectDialog::ProjectDialog(ProjectInfo &projectInfo, bool isNewProject, QWidget *parent) : QDialog(parent)
{
    m_isNewProject = isNewProject;
    m_projectInfo = &projectInfo;

    setWindowTitle(tr("Project properties"));

    createControls();

    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

ProjectDialog::~ProjectDialog()
{
    delete txtName;
    delete cmbProblemType;
    if (m_isNewProject) delete cmbPhysicField;
    delete dtmDate;
    delete txtNumberOfRefinements;
    delete txtPolynomialOrder;
    delete txtAdaptivitySteps;
    delete txtAdaptivityTolerance;
}

int ProjectDialog::showDialog()
{
    return exec();
}

void ProjectDialog::createControls()
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
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(0);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivityTolerance = new SLineEdit("1", true, this);
    txtFrequency = new SLineEdit("0", true, this);

    QGridLayout *layoutProject = new QGridLayout();
    layoutProject->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutProject->addWidget(txtName, 0, 1);
    layoutProject->addWidget(new QLabel(tr("Date:")), 1, 0);
    layoutProject->addWidget(dtmDate, 1, 1);
    layoutProject->addWidget(new QLabel(tr("Problem type:")), 2, 0);
    layoutProject->addWidget(cmbProblemType, 2, 1);
    layoutProject->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    layoutProject->addWidget(cmbPhysicField, 3, 1);
    layoutProject->addWidget(new QLabel(tr("Frequency:")), 4, 0);
    layoutProject->addWidget(txtFrequency, 4, 1);
    layoutProject->addWidget(new QLabel(tr("Number of refinements:")), 5, 0);
    layoutProject->addWidget(txtNumberOfRefinements, 5, 1);
    layoutProject->addWidget(new QLabel(tr("Polynomial order:")), 6, 0);
    layoutProject->addWidget(txtPolynomialOrder, 6, 1);
    layoutProject->addWidget(new QLabel(tr("Adaptivity steps:")), 7, 0);
    layoutProject->addWidget(txtAdaptivitySteps, 7, 1);
    layoutProject->addWidget(new QLabel(tr("Adaptivity tolerance:")), 8, 0);
    layoutProject->addWidget(txtAdaptivityTolerance, 8, 1);

    connect(cmbPhysicField, SIGNAL(currentIndexChanged(int)), this, SLOT(doPhysicFieldChanged(int)));
    fillComboBox();

    // physicFieldString(m_projectInfo->physicField)
    
    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutProject);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ProjectDialog::fillComboBox()
{
    cmbProblemType->clear();
    cmbProblemType->addItem(problemTypeString(PROBLEMTYPE_PLANAR), PROBLEMTYPE_PLANAR);
    cmbProblemType->addItem(problemTypeString(PROBLEMTYPE_AXISYMMETRIC), PROBLEMTYPE_AXISYMMETRIC);

    cmbPhysicField->clear();
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_ELECTROSTATIC), PHYSICFIELD_ELECTROSTATIC);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_MAGNETOSTATIC), PHYSICFIELD_MAGNETOSTATIC);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_HARMONIC_MAGNETIC), PHYSICFIELD_HARMONIC_MAGNETIC);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_CURRENT), PHYSICFIELD_CURRENT);
    cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_HEAT_TRANSFER), PHYSICFIELD_HEAT_TRANSFER);
    // cmbPhysicField->addItem(physicFieldString(PHYSICFIELD_ELASTICITY), PHYSICFIELD_ELASTICITY);

    cmbPhysicField->setEnabled(m_isNewProject);
}

void ProjectDialog::load() {
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(m_projectInfo->physicField));
    txtName->setText(m_projectInfo->name);
    cmbProblemType->setCurrentIndex(cmbProblemType->findData(m_projectInfo->problemType));
    dtmDate->setDate(m_projectInfo->date);
    txtNumberOfRefinements->setValue(m_projectInfo->numberOfRefinements);
    txtPolynomialOrder->setValue(m_projectInfo->polynomialOrder);
    txtAdaptivitySteps->setValue(m_projectInfo->adaptivitySteps);
    txtAdaptivityTolerance->setValue(m_projectInfo->adaptivityTolerance);
    txtFrequency->setValue(m_projectInfo->frequency);
}

void ProjectDialog::save() {
    if (this->m_isNewProject) m_projectInfo->physicField = (PhysicField) cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toInt();
    m_projectInfo->problemType = (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt();
    m_projectInfo->name = txtName->text();
    m_projectInfo->date = dtmDate->date();
    m_projectInfo->numberOfRefinements = txtNumberOfRefinements->value();
    m_projectInfo->polynomialOrder = txtPolynomialOrder->value();
    m_projectInfo->adaptivitySteps = txtAdaptivitySteps->value();
    m_projectInfo->adaptivityTolerance = txtAdaptivityTolerance->value();
    m_projectInfo->frequency = txtFrequency->value();
}

void ProjectDialog::doAccept()
{
    save();
    accept();
}

void ProjectDialog::doReject()
{
    reject();
}

void ProjectDialog::doPhysicFieldChanged(int index)
{
    txtFrequency->setEnabled(false);

    switch ((PhysicField) cmbPhysicField->itemData(index).toInt())
    {
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        {
            txtFrequency->setEnabled(true);
        }
        break;
    }
}
