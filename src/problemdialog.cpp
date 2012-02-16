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
#include "pythonlabagros.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/coupling.h"

FieldSelectDialog::FieldSelectDialog(QList<QString> fields, QWidget *parent) : QDialog(parent)
{
    logMessage("FieldSelectDialog::FieldSelectDialog()");

    setWindowTitle(tr("Select field"));
    setModal(true);

    m_selectedFieldId = "";

    lstFields = new QListWidget(this);

    std::map<std::string, std::string> modules = availableModules();
    for (std::map<std::string, std::string>::iterator it = modules.begin();
         it != modules.end(); ++it)
    {
        // add only missing fields
        if (!fields.contains(QString::fromStdString(it->first)))
        {
            QListWidgetItem *item = new QListWidgetItem(lstFields);
            item->setText(QString::fromStdString(it->second));
            item->setData(Qt::UserRole, QString::fromStdString(it->first));

            lstFields->addItem(item);
        }
    }

    connect(lstFields, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemPressed(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstFields);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstFields->count() > 0)
    {
        lstFields->setCurrentRow(0);
        doItemSelected(lstFields->currentItem());
    }
}

void FieldSelectDialog::doAccept()
{
    logMessage("FieldSelectDialog::doAccept()");

    accept();
}

void FieldSelectDialog::doReject()
{
    logMessage("FieldSelectDialog::doReject()");

    reject();
}

int FieldSelectDialog::showDialog()
{
    return exec();
}

void FieldSelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_selectedFieldId = item->data(Qt::UserRole).toString();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FieldSelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstFields->currentItem())
    {
        m_selectedFieldId = lstFields->currentItem()->data(Qt::UserRole).toString();
        accept();
    }
}

// ********************************************************************************************************

FieldWidget::FieldWidget(const ProblemInfo *problemInfo, FieldInfo *fieldInfo, QWidget *parent, ProblemDialog* problemDialog)
    : QWidget(parent), problemInfo(problemInfo), m_fieldInfo(fieldInfo), m_problemDialog(problemDialog)
{
    createContent();
    load();
}

void FieldWidget::createContent()
{
    // equations
    lblEquationPixmap = new QLabel("");
    lblEquationPixmap->setMinimumHeight(50);

    cmbAdaptivityType = new QComboBox();
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivityTolerance = new SLineEditDouble(1);

    // mesh
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(5);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);

    // weak forms
    cmbWeakForms = new QComboBox();

    // transient
    cmbAnalysisType = new QComboBox();
    txtTransientInitialCondition = new ValueLineEdit();

    // linearity
    cmbLinearityType = new QComboBox();
    txtNonlinearSteps = new QSpinBox(this);
    txtNonlinearSteps->setMinimum(1);
    txtNonlinearSteps->setMaximum(100);
    txtNonlinearTolerance = new SLineEditDouble(1);

    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));
    //connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), m_problemDialog, SLOT(doFindCouplings()));

    connect(cmbLinearityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearityTypeChanged(int)));

    // fill combobox
    fillComboBox();

    int minWidth = 130;

    // table
    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->setColumnMinimumWidth(0, minWidth);
    layoutTable->setColumnStretch(1, 1);
    layoutTable->addWidget(new QLabel(tr("Type of analysis:")), 0, 0);
    layoutTable->addWidget(cmbAnalysisType, 0, 1);
    layoutTable->addWidget(new QLabel(tr("Adaptivity:")), 1, 0);
    layoutTable->addWidget(cmbAdaptivityType, 1, 1);
    layoutTable->addWidget(new QLabel(tr("Weak forms:")), 2, 0);
    layoutTable->addWidget(cmbWeakForms, 2, 1);

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, minWidth);
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial condition:")), 0, 0);
    layoutTransientAnalysis->addWidget(txtTransientInitialCondition, 0, 1);

    QGroupBox *grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // harmonic analysis
    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->setColumnMinimumWidth(0, minWidth);
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(new QLabel(tr("Number of refinements:")), 0, 0);
    layoutMesh->addWidget(txtNumberOfRefinements, 0, 1);
    layoutMesh->addWidget(new QLabel(tr("Polynomial order:")), 1, 0);
    layoutMesh->addWidget(txtPolynomialOrder, 1, 1);

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
    layoutLinearity->addWidget(new QLabel(tr("Linearity:")), 0, 0);
    layoutLinearity->addWidget(cmbLinearityType, 0, 1);
    layoutLinearity->addWidget(new QLabel(tr("Tolerance (%):")), 1, 0);
    layoutLinearity->addWidget(txtNonlinearTolerance, 1, 1);
    layoutLinearity->addWidget(new QLabel(tr("Steps:")), 2, 0);
    layoutLinearity->addWidget(txtNonlinearSteps, 2, 1);

    QGroupBox *grpLinearity = new QGroupBox(tr("Newton solver"));
    grpLinearity->setLayout(layoutLinearity);
    // grpLinearity->setVisible(Util::config()->showExperimentalFeatures);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addLayout(layoutTable);
    layoutLeft->addWidget(grpLinearity);
    layoutLeft->addStretch();

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpMesh);
    layoutRight->addWidget(grpAdaptivity);
    layoutRight->addWidget(grpTransientAnalysis);
    layoutRight->addStretch();

    // both
    QHBoxLayout *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // equation
    QGridLayout *layoutEquation = new QGridLayout();
    layoutEquation->setColumnMinimumWidth(0, minWidth);
    layoutEquation->setColumnStretch(1, 1);
    layoutEquation->addWidget(new QLabel(tr("Equation:")), 0, 0);
    layoutEquation->addWidget(lblEquationPixmap, 0, 1, 1, 1, Qt::AlignLeft);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addLayout(layoutEquation);
    layoutProblem->addLayout(layoutPanel);

    setLayout(layoutProblem);

    setMinimumSize(sizeHint());
}

void FieldWidget::fillComboBox()
{
    logMessage("ProblemDialog::fillComboBox()");

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_None), AdaptivityType_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_H), AdaptivityType_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_P), AdaptivityType_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_HP), AdaptivityType_HP);

    cmbWeakForms->clear();
    cmbWeakForms->addItem(weakFormsTypeString(WeakFormsType_Compiled), WeakFormsType_Compiled);
    cmbWeakForms->addItem(weakFormsTypeString(WeakFormsType_Interpreted), WeakFormsType_Interpreted);

    cmbLinearityType->addItem(linearityTypeString(LinearityType_Linear), LinearityType_Linear);
    // if (hermesField->hasNonlinearity())
    {
        cmbLinearityType->addItem(linearityTypeString(LinearityType_Picard), LinearityType_Picard);
        cmbLinearityType->addItem(linearityTypeString(LinearityType_Newton), LinearityType_Newton);
    }

    std::map<std::string, std::string> analyses = availableAnalyses(m_fieldInfo->fieldId().toStdString());
    for (std::map<std::string, std::string>::iterator it = analyses.begin(); it != analyses.end(); ++it)
        cmbAnalysisType->addItem(QString::fromStdString(it->second), analysisTypeFromStringKey(QString::fromStdString(it->first)));
}

void FieldWidget::load()
{
    // analysis type
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(m_fieldInfo->analysisType()));
    if (cmbAnalysisType->currentIndex() == -1)
        cmbAnalysisType->setCurrentIndex(0);
    // adaptivity
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_fieldInfo->adaptivityType));
    txtAdaptivitySteps->setValue(m_fieldInfo->adaptivitySteps);
    txtAdaptivityTolerance->setValue(m_fieldInfo->adaptivityTolerance);
    // weakforms
    cmbWeakForms->setCurrentIndex(cmbWeakForms->findData(m_fieldInfo->weakFormsType));
    //mesh
    txtNumberOfRefinements->setValue(m_fieldInfo->numberOfRefinements);
    txtPolynomialOrder->setValue(m_fieldInfo->polynomialOrder);
    // transient
    txtTransientInitialCondition->setValue(m_fieldInfo->initialCondition);
    // linearity
    cmbLinearityType->setCurrentIndex(cmbLinearityType->findData(m_fieldInfo->linearityType));
    txtNonlinearSteps->setValue(m_fieldInfo->nonlinearSteps);
    txtNonlinearTolerance->setValue(m_fieldInfo->nonlinearTolerance);

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

bool FieldWidget::save()
{    
    m_fieldInfo->setAnalysisType((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt());

    // adaptivity
    m_fieldInfo->adaptivityType = (AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt();
    m_fieldInfo->adaptivitySteps = txtAdaptivitySteps->value();
    m_fieldInfo->adaptivityTolerance = txtAdaptivityTolerance->value();
    // weakforms
    m_fieldInfo->weakFormsType = (WeakFormsType) cmbWeakForms->itemData(cmbWeakForms->currentIndex()).toInt();
    //mesh
    m_fieldInfo->numberOfRefinements = txtNumberOfRefinements->value();
    m_fieldInfo->polynomialOrder = txtPolynomialOrder->value();
    // transient
    m_fieldInfo->initialCondition = txtTransientInitialCondition->value();
    // linearity
    m_fieldInfo->linearityType = (LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt();
    m_fieldInfo->nonlinearSteps = txtNonlinearSteps->value();
    m_fieldInfo->nonlinearTolerance = txtNonlinearTolerance->value();

    return true;
}

void FieldWidget::refresh()
{
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

FieldInfo *FieldWidget::fieldInfo()
{
    return m_fieldInfo;
}

void FieldWidget::doAnalysisTypeChanged(int index)
{
    logMessage("ProblemDialog::doAnalysisTypeChanged()");

    txtTransientInitialCondition->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);

    doShowEquation();
}

void FieldWidget::doShowEquation()
{
    readPixmap(lblEquationPixmap,
               QString(":/equations/%1/%1_%2.png")
               .arg(m_fieldInfo->fieldId())
               .arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));
}

void FieldWidget::doAdaptivityChanged(int index)
{
    logMessage("ProblemDialog::doAdaptivityChanged()");

    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
}

void FieldWidget::doLinearityTypeChanged(int index)
{
    logMessage("ProblemDialog::doLinearityTypeChanged()");

    txtNonlinearSteps->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearTolerance->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
}

// ********************************************************************************************

CouplingsWidget::CouplingsWidget(QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >* couplingInfos, QWidget *parent) :
    QWidget(parent), m_couplingInfos(couplingInfos)
{
    connect(parent, SIGNAL(fieldsChanged()), this, SLOT(doFieldsChanged()));
    layoutTable = NULL;
    createContent();
    load();
}

void CouplingsWidget::createContent()
{
    int minWidth = 130;

    if(layoutTable)
    {
        save();
        delete layoutTable;
        qDeleteAll(this->children());
    }

    layoutTable = new QGridLayout();
    layoutTable->setColumnMinimumWidth(0, minWidth);
    layoutTable->setColumnStretch(1, 1);

    m_comboBoxes.clear();
    int line = 0;
    foreach(CouplingInfo* couplingInfo, *m_couplingInfos)
    {
//        cout << "adding coupling box " << couplingInfo->coupling()->name << endl;
        layoutTable->addWidget(new QLabel(/*tr(*/QString::fromStdString(couplingInfo->coupling()->name)/*)*/), line, 0);
        m_comboBoxes[couplingInfo] = new QComboBox();
        layoutTable->addWidget(m_comboBoxes[couplingInfo], line, 1);
        line++;
    }

    fillComboBox();
    load();

    setLayout(layoutTable);
}


void CouplingsWidget::doFieldsChanged()
{
    createContent();
}

void CouplingsWidget::fillComboBox()
{
    foreach(QComboBox* comboBox, m_comboBoxes)
    {
        comboBox->addItem(couplingTypeString(CouplingType_None), CouplingType_None);
        comboBox->addItem(couplingTypeString(CouplingType_Weak), CouplingType_Weak);
        comboBox->addItem(couplingTypeString(CouplingType_Hard), CouplingType_Hard);
    }
}

void CouplingsWidget::load()
{
    foreach(CouplingInfo* couplingInfo, *m_couplingInfos)
    {
//        cout << "loading couplings widget " << couplingInfo->coupling()->name << ", value " << couplingTypeString(couplingInfo->couplingType()).toStdString() << endl;
        m_comboBoxes[couplingInfo]->setCurrentIndex(couplingInfo->couplingType());
    }
}

void CouplingsWidget::save()
{
    foreach(CouplingInfo* couplingInfo, *m_couplingInfos)
    {
        if(m_comboBoxes.contains(couplingInfo))
        {
//            cout << "saving couplings widget " << couplingInfo->coupling()->name << " value " << couplingTypeString((CouplingType)m_comboBoxes[couplingInfo]->itemData(m_comboBoxes[couplingInfo]->currentIndex()).toInt()).toStdString() << endl;
            couplingInfo->setCouplingType((CouplingType)m_comboBoxes[couplingInfo]->itemData(m_comboBoxes[couplingInfo]->currentIndex()).toInt());
        }
    }
}


// ********************************************************************************************

FieldTabWidget::FieldTabWidget(QWidget *parent,  QMap<QString, FieldInfo *> fieldInfos, QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos)
    : QTabWidget(parent), m_fieldInfos(fieldInfos), m_couplingInfos(couplingInfos)
{
    m_haveCouplingsTab = 0;

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(doRemoveFieldRequested(int)));

    foreach (FieldInfo *fieldInfo, m_fieldInfos)
        addFieldTab(fieldInfo);

    doFindCouplings();

}


void FieldTabWidget::updateCouplingTab()
{
    if(m_haveCouplingsTab)
    {
        if (m_couplingInfos.size()){
        }
        else
        {
            removeTab(count() - 1);
            m_haveCouplingsTab = 0;
        }
    }
    else
    {
        if (m_couplingInfos.size()){
            addTab(new CouplingsWidget(&m_couplingInfos, this), "Couplings");
            m_haveCouplingsTab = 1;
        }

    }
}

void FieldTabWidget::addFieldTab(FieldInfo *fieldInfo)
{
    FieldWidget* fieldWidget = new FieldWidget(((ProblemDialog*)parent())->problemInfo(), fieldInfo, this, (ProblemDialog*)parent());
    m_fieldTabs[fieldInfo] = fieldWidget;
    m_fieldInfos[fieldInfo->fieldId()] = fieldInfo;

    // add widget
    //QTabWidget::addTab(fieldWidget, QString::fromStdString(fieldInfo->module()->name));
    int fieldPosition = count() - m_haveCouplingsTab;
    insertTab(fieldPosition, fieldWidget, QString::fromStdString(fieldInfo->module()->name));
    setCurrentIndex(count() - 1 - m_haveCouplingsTab);

    doFindCouplings();
    emit fieldsChanged();

//    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(tabFields->count() > 0);


}

void FieldTabWidget::removeFieldTab(FieldInfo *fieldInfo)
{
    for(int i = 0; i < count(); i++){
        if(m_fieldTabs[fieldInfo] == widget(i)){
            QTabWidget::removeTab(i);
            return;
        }
    }

    assert(0);
}

void FieldTabWidget::doRemoveFieldRequested(int index)
{
    // remove field
    FieldWidget *wid = dynamic_cast<FieldWidget *>(widget(index));

    if (QMessageBox::question(this, tr("Remove field"), tr("Are you sure to remove field '%1'?").
                              arg(QString::fromStdString(wid->fieldInfo()->module()->name)),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        removeTab(index);

        m_fieldInfos.remove(wid->fieldInfo()->fieldId());

        // delete corresponding fileinfo (new field only)
        if (!Util::scene()->fieldInfos().keys().contains(wid->fieldInfo()->fieldId()))
            delete wid->fieldInfo();

        // enable accept button
        /// TODO
        //buttonBox->button(QDialogButtonBox::Ok)->setEnabled(tabFields->count() > 0);

        doFindCouplings();
        emit fieldsChanged();
    }
}


void FieldTabWidget::doFindCouplings()
{
    // add missing
    foreach(FieldInfo* sourceField, m_fieldInfos)
    {
        foreach(FieldInfo* targetField, m_fieldInfos)
        {
            if(sourceField == targetField)
                continue;
            QPair<FieldInfo*, FieldInfo*> fieldInfosPair(sourceField, targetField);
            if(isCouplingAvailable(sourceField, targetField)){
                if(! m_couplingInfos.contains(fieldInfosPair))
                {
                    m_couplingInfos[fieldInfosPair] = new CouplingInfo(sourceField, targetField);
                }
            }
        }
    }

    // remove extra
    foreach(CouplingInfo* couplingInfo, m_couplingInfos)
    {
        if(! (m_fieldInfos.contains(couplingInfo->sourceField()->fieldId()) &&
              m_fieldInfos.contains(couplingInfo->targetField()->fieldId()) &&
              isCouplingAvailable(couplingInfo->sourceField(), couplingInfo->targetField())))
        {
            cout << "removing info" << endl;
            m_couplingInfos.remove(QPair<FieldInfo*, FieldInfo*>(couplingInfo->sourceField(), couplingInfo->targetField()));
        }
    }

    updateCouplingTab();
}


void FieldTabWidget::save()
{
    for (int i = 0; i < count() - m_haveCouplingsTab; i++)
        static_cast<FieldWidget *>(widget(i))->save();

    // save couplings
    if(m_haveCouplingsTab)
        static_cast<CouplingsWidget *>(widget(count() - 1))->save();

    // add missing fields
    for (int i = 0; i < count() - m_haveCouplingsTab; i++)
    {
        FieldWidget *wid = static_cast<FieldWidget *>(widget(i));

        // add missing field
        if (!Util::scene()->hasField(wid->fieldInfo()->fieldId()))
            Util::scene()->addField(wid->fieldInfo());
    }

    // remove deleted fields
    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        bool exists = false;
        for (int i = 0; i < count() - m_haveCouplingsTab; i++)
        {
            FieldWidget *wid = static_cast<FieldWidget *>(widget(i));

            if (fieldInfo->fieldId() == wid->fieldInfo()->fieldId())
                exists = true;
        }

        // remove field
        if (!exists)
            Util::scene()->removeField(fieldInfo);
    }

    Util::scene()->setCouplingInfos(m_couplingInfos);

}

// ********************************************************************************************

ProblemDialog::ProblemDialog(ProblemInfo *problemInfo,
                             QMap<QString, FieldInfo *> fieldInfos,
                             QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos,
                             bool isNewProblem,
                             QWidget *parent) : QDialog(parent)
{
    logMessage("ProblemDialog::ProblemDialog()");

    m_isNewProblem = isNewProblem;
    m_problemInfo = problemInfo;
    m_fieldInfos = fieldInfos;
    m_couplingInfos = couplingInfos;

    setWindowTitle(tr("Problem properties"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
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
    QPushButton *btnAddField = new QPushButton(tr("Add field"));
    btnAddField->setDefault(false);
    btnAddField->setVisible(false);
    connect(btnAddField, SIGNAL(clicked()), this, SLOT(doAddField()));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnAddField, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    resize(sizeHint());
}

QWidget *ProblemDialog::createControlsGeneral()
{
    logMessage("ProblemDialog::createControlsGeneral()");

    // problem
    cmbCoordinateType = new QComboBox();
    txtName = new QLineEdit("");
    dtmDate = new QDateTimeEdit();
    dtmDate->setDisplayFormat("dd.MM.yyyy");
    dtmDate->setCalendarPopup(true);

    // matrix solver
    cmbMatrixSolver = new QComboBox();

    // mesh type
    cmbMeshType = new QComboBox();

    // harmonic
    txtFrequency = new SLineEditDouble();

    // transient
    txtTransientTimeStep = new ValueLineEdit();
    txtTransientTimeTotal = new ValueLineEdit();
    lblTransientSteps = new QLabel("0");

    connect(txtTransientTimeStep, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));

    // fill combobox
    fillComboBox();

    int minWidth = 130;

    // table
    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->setColumnMinimumWidth(0, minWidth);
    layoutTable->setColumnStretch(1, 1);
    layoutTable->addWidget(new QLabel(tr("Date:")), 0, 0);
    layoutTable->addWidget(dtmDate, 0, 1);
    layoutTable->addWidget(new QLabel(tr("Coordinate type:")), 1, 0);
    layoutTable->addWidget(cmbCoordinateType, 1, 1);
    layoutTable->addWidget(new QLabel(tr("Linear solver:")), 2, 0);
    layoutTable->addWidget(cmbMatrixSolver, 2, 1);
    layoutTable->addWidget(new QLabel(tr("Mesh type:")), 3, 0);
    layoutTable->addWidget(cmbMeshType, 3, 1);

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
    layoutTransientAnalysis->addWidget(new QLabel(tr("Steps:")), 2, 0);
    layoutTransientAnalysis->addWidget(lblTransientSteps, 2, 1);

    QGroupBox *grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addLayout(layoutTable);
    layoutLeft->addStretch();

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpHarmonicAnalysis);
    layoutRight->addWidget(grpTransientAnalysis);
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

    // fields
    tabFields = new FieldTabWidget(this, m_fieldInfos, m_couplingInfos);
    tabFields->setTabsClosable(true);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addLayout(layoutName);
    layoutProblem->addLayout(layoutPanel);
    layoutProblem->addWidget(tabFields);

    QWidget *widMain = new QWidget();
    widMain->setLayout(layoutProblem);

    return widMain;
}

QWidget *ProblemDialog::createControlsStartupScript()
{
    logMessage("ProblemDialog::createControlsStartupScript()");

    txtStartupScript = new ScriptEditor(currentPythonEngine(), this);

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

    cmbCoordinateType->clear();
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Planar), CoordinateType_Planar);
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Axisymmetric), CoordinateType_Axisymmetric);

    cmbMeshType->addItem(meshTypeString(MeshType_Triangle), MeshType_Triangle);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadFineDivision), MeshType_QuadFineDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadRoughDivision), MeshType_QuadRoughDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadJoin), MeshType_QuadJoin);

    cmbMatrixSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_UMFPACK), Hermes::SOLVER_UMFPACK);
}

void ProblemDialog::load()
{
    logMessage("ProblemDialog::load()");

    // main
    txtName->setText(m_problemInfo->name);
    dtmDate->setDate(m_problemInfo->date);
    cmbCoordinateType->setCurrentIndex(cmbCoordinateType->findData(m_problemInfo->coordinateType));
    if (cmbCoordinateType->currentIndex() == -1)
        cmbCoordinateType->setCurrentIndex(0);

    // mesh type
    cmbMeshType->setCurrentIndex(cmbMeshType->findData(m_problemInfo->meshType));
    // harmonic magnetic
    txtFrequency->setValue(m_problemInfo->frequency);
    // transient
    txtTransientTimeStep->setValue(m_problemInfo->timeStep);
    txtTransientTimeTotal->setValue(m_problemInfo->timeTotal);
    // matrix solver
    cmbMatrixSolver->setCurrentIndex(cmbMatrixSolver->findData(m_problemInfo->matrixSolver));
    // startup
    txtStartupScript->setPlainText(m_problemInfo->startupscript);
    // description
    txtDescription->setPlainText(m_problemInfo->description);

    doTransientChanged();
}

bool ProblemDialog::save()
{
    logMessage("ProblemDialog::save()");

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
    m_problemInfo->name = txtName->text();
    m_problemInfo->date = dtmDate->date();
    m_problemInfo->coordinateType = (CoordinateType) cmbCoordinateType->itemData(cmbCoordinateType->currentIndex()).toInt();
    m_problemInfo->meshType = (MeshType) cmbMeshType->itemData(cmbMeshType->currentIndex()).toInt();

    m_problemInfo->frequency = txtFrequency->value();

    m_problemInfo->timeStep = txtTransientTimeStep->value();
    m_problemInfo->timeTotal = txtTransientTimeTotal->value();

    m_problemInfo->description = txtDescription->toPlainText();
    m_problemInfo->startupscript = txtStartupScript->toPlainText();

    // matrix solver
    m_problemInfo->matrixSolver = (Hermes::MatrixSolverType) cmbMatrixSolver->itemData(cmbMatrixSolver->currentIndex()).toInt();

    // save fields
    tabFields->save();

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

void ProblemDialog::doOpenXML()
{
    logMessage("ProblemDialog::doOpenXML()");

    QString fileName;
    //TODO custom
    //    if (cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString() == "custom")
    //    {
    //        fileName = Util::scene()->problemInfo()->fileName.left(Util::scene()->problemInfo()->fileName.size() - 4) + ".xml";

    //        if (!QFile::exists(fileName))
    //            if (QMessageBox::question(this, tr("Custom module file"), tr("Custom module doesn't exist. Could I create it?"),
    //                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    //            {
    //                // copy custom module
    //                QFile::copy(datadir() + "/resources/custom.xml",
    //                            fileName);
    //            }
    //    }
    //    else
    //    {
    //        fileName = datadir() + "/modules/" + cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString() + ".xml";
    //    }

    //    if (QFile::exists(fileName))
    //        QDesktopServices::openUrl(QUrl(fileName));
}

void ProblemDialog::doPhysicFieldChanged(int index)
{
    logMessage("ProblemDialog::doPhysicFieldChanged()");

    // refresh modules
    for (int i = 0; i < tabFields->fieldsCount(); i++)
    {
        FieldWidget *wid = dynamic_cast<FieldWidget *>(tabFields->widget(i));
        wid->refresh();
    }
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

void ProblemDialog::doAddField()
{
    // used fields
    QList<QString> fields;
    for (int i = 0; i < tabFields->fieldsCount(); i++)
    {
        FieldWidget *wid = dynamic_cast<FieldWidget *>(tabFields->widget(i));
        fields.append(wid->fieldInfo()->fieldId());
    }

    // select field dialog
    FieldSelectDialog dialog(fields, this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        FieldInfo *fieldInfo;
        if (Util::scene()->fieldInfos().keys().contains(dialog.selectedFieldId()))
        {
            // existing field info main in collection
            fieldInfo = Util::scene()->fieldInfo(dialog.selectedFieldId());
        }
        else
        {
            // new field info
            fieldInfo = new FieldInfo(m_problemInfo, dialog.selectedFieldId());
        }

        tabFields->addFieldTab(fieldInfo);

    }

    tabFields->doFindCouplings();
}

