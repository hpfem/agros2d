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

#include "physicalfield.h"

#include "util/global.h"
#include "gui/common.h"

#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/field.h"

PhysicalFieldWidget::PhysicalFieldWidget(QWidget *parent) : QWidget(parent)
{
    cmbFieldInfo = new QComboBox();
    connect(cmbFieldInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldInfo(int)));

    QGridLayout *layoutField = new QGridLayout();
    layoutField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutField->setColumnStretch(1, 1);
    layoutField->addWidget(new QLabel(tr("Physical field:")), 0, 0);
    layoutField->addWidget(cmbFieldInfo, 0, 1);

    QGroupBox *grpField = new QGroupBox(tr("Postprocessor settings"), this);
    grpField->setLayout(layoutField);

    // transient
    lblTimeStep = new QLabel(tr("Time step:"));
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStep(int)));

    QGridLayout *layoutTransient = new QGridLayout();
    layoutTransient->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransient->setColumnStretch(1, 1);
    layoutTransient->addWidget(lblTimeStep, 0, 0);
    layoutTransient->addWidget(cmbTimeStep, 0, 1);

    grpTime = new QGroupBox(tr("Transient analysis"), this);
    grpTime->setVisible(false);
    grpTime->setLayout(layoutTransient);

    // adaptivity
    lblAdaptivityStep = new QLabel(tr("Adaptivity step:"));
    cmbAdaptivityStep = new QComboBox(this);
    connect(cmbAdaptivityStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStep(int)));
    lblAdaptivitySolutionType = new QLabel(tr("Adaptivity type:"));
    cmbAdaptivitySolutionType = new QComboBox(this);

    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(lblAdaptivityStep, 0, 0);
    layoutAdaptivity->addWidget(cmbAdaptivityStep, 0, 1);
    layoutAdaptivity->addWidget(lblAdaptivitySolutionType, 1, 0);
    layoutAdaptivity->addWidget(cmbAdaptivitySolutionType, 1, 1);

    grpAdaptivity = new QGroupBox(tr("Space adaptivity"), this);
    grpAdaptivity->setVisible(false);
    grpAdaptivity->setLayout(layoutAdaptivity);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(grpField);
    layoutMain->addWidget(grpTime);
    layoutMain->addWidget(grpAdaptivity);

    setLayout(layoutMain);

    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(updateControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(updateControls()));
}

PhysicalFieldWidget::~PhysicalFieldWidget()
{

}

FieldInfo* PhysicalFieldWidget::selectedField()
{
    if (Agros2D::problem()->hasField(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString()))
        return Agros2D::problem()->fieldInfo(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString());
    else
        return NULL;
}

void PhysicalFieldWidget::selectField(const FieldInfo* fieldInfo)
{
    if (cmbFieldInfo->findData(fieldInfo->fieldId()) != -1)
    {
        cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldInfo->fieldId()));

        if (Agros2D::problem()->isSolved())
        {
            fillComboBoxTimeStep(fieldInfo, cmbTimeStep);
            doTimeStep(-1);
        }

        // if (m_currentFieldName != fieldInfo->fieldId())
        //     emit fieldChanged();

        // set current field name
        // m_currentFieldName = fieldInfo->fieldId();
    }
}

int PhysicalFieldWidget::selectedTimeStep()
{
    if (cmbTimeStep->currentIndex() == -1)
        return 0;
    else
    {
        int selectedTimeStep = cmbTimeStep->itemData(cmbTimeStep->currentIndex()).toInt();

        // todo: this is here to avoid fail after loading new model
        // todo: cmbTimeStep should probably be cleared somewhere
        // todo: or the PostprocessorWidget should be destroyed and created a new one?
        if(Agros2D::solutionStore()->timeLevels(selectedField()).size() <= selectedTimeStep)
            return 0;

        // due to timeskipping
        double realTime = Agros2D::solutionStore()->timeLevels(selectedField()).at(selectedTimeStep);
        int realTimeStep = Agros2D::problem()->timeToTimeStep(realTime);
        return realTimeStep;
    }
}

void PhysicalFieldWidget::selectTimeStep(int timeStep)
{
    if (cmbTimeStep->findData(timeStep) != -1)
    {
        cmbTimeStep->setCurrentIndex(cmbTimeStep->findData(timeStep));
        doAdaptivityStep(-1);
    }
}

int PhysicalFieldWidget::selectedAdaptivityStep()
{
    if (cmbAdaptivityStep->currentIndex() == -1)
        return 0;
    else
        return cmbAdaptivityStep->itemData(cmbAdaptivityStep->currentIndex()).toInt();
}

void PhysicalFieldWidget::selectAdaptivityStep(int adaptivityStep)
{
    if (cmbAdaptivityStep->findData(adaptivityStep) != -1)
    {
        cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->findData(adaptivityStep));
    }
}

SolutionMode PhysicalFieldWidget::selectedAdaptivitySolutionType()
{
    if (cmbAdaptivityStep->currentIndex() == -1)
        return SolutionMode_Undefined;
    else
        return (SolutionMode) cmbAdaptivitySolutionType->itemData(cmbAdaptivitySolutionType->currentIndex()).toInt();
}

void PhysicalFieldWidget::selectedAdaptivitySolutionType(SolutionMode solutionMode)
{
    cmbAdaptivitySolutionType->setCurrentIndex(cmbAdaptivitySolutionType->findData(solutionMode));
}

void PhysicalFieldWidget::updateControls()
{
    if (Agros2D::problem()->isMeshed())
    {
        fillComboBoxFieldInfo(cmbFieldInfo);
        doFieldInfo(cmbFieldInfo->currentIndex());
    }
    else
    {
        cmbFieldInfo->clear();
        doFieldInfo(cmbFieldInfo->currentIndex());
    }

    grpTime->setVisible(cmbTimeStep->count() > 1);
    grpAdaptivity->setVisible(cmbAdaptivityStep->count() > 1);
}

void PhysicalFieldWidget::doFieldInfo(int index)
{
    QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
    if (Agros2D::problem()->hasField(fieldName))
    {
        FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(fieldName);
        if (Agros2D::problem()->isSolved())
        {
            fillComboBoxTimeStep(fieldInfo, cmbTimeStep);            
        }
        else
        {
            cmbTimeStep->clear();
        }
        doTimeStep(-1);

        if ((m_currentFieldName != fieldName) || (m_currentAnalysisType != fieldInfo->analysisType()))
            emit fieldChanged();

        // set current field name
        m_currentFieldName = fieldName;
        m_currentAnalysisType = fieldInfo->analysisType();
    }
    else
    {
        cmbTimeStep->clear();
        doTimeStep(-1);
    }
}

void PhysicalFieldWidget::doTimeStep(int index)
{
    if (Agros2D::problem()->isSolved())
    {
        fillComboBoxAdaptivityStep(selectedField(), selectedTimeStep(), cmbAdaptivityStep);
        if ((cmbAdaptivityStep->currentIndex() >= cmbAdaptivityStep->count()) || (cmbAdaptivityStep->currentIndex() < 0))
        {
            cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->count() - 1);
        }
    }
    else
    {
        cmbAdaptivityStep->clear();        
    }

    doAdaptivityStep(-1);
}

void PhysicalFieldWidget::doAdaptivityStep(int index)
{
    fillComboBoxSolutionType(selectedField(), selectedTimeStep(), selectedAdaptivityStep(), cmbAdaptivitySolutionType);
    if ((cmbAdaptivitySolutionType->currentIndex() >= cmbAdaptivitySolutionType->count()) || (cmbAdaptivitySolutionType->currentIndex() < 0))
    {
        cmbAdaptivitySolutionType->setCurrentIndex(0);
    }
}
