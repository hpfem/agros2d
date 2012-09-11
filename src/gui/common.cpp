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

#include "common.h"

#include "scene.h"
#include "util.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/solver.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

void readPixmap(QLabel *lblEquation, const QString &name)
{
    QPixmap pixmap;
    pixmap.load(name);
    lblEquation->setPixmap(pixmap);
    lblEquation->setMaximumSize(pixmap.size());
    lblEquation->setMinimumSize(pixmap.size());
}

QLabel *createLabel(const QString &label, const QString &toolTip)
{
    QLabel *lblEquation = new QLabel(label + ":");
    lblEquation->setToolTip(toolTip);
    lblEquation->setMinimumWidth(100);
    return lblEquation;
}

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, QString("%1 ").arg(unit));
    item->setTextAlignment(2, Qt::AlignLeft);
}

void fillComboBoxFieldInfo(QComboBox *cmbFieldInfo)
{
    // store variable
    QString fieldId = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();

    // clear combo
    cmbFieldInfo->blockSignals(true);
    cmbFieldInfo->clear();
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        cmbFieldInfo->addItem(fieldInfo->name(), fieldInfo->fieldId());

    cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldId));
    if (cmbFieldInfo->currentIndex() == -1)
        cmbFieldInfo->setCurrentIndex(0);
    cmbFieldInfo->blockSignals(false);
}

void fillComboBoxScalarVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    fieldInfo->module()->fillComboBoxScalarVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxContourVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    fieldInfo->module()->fillComboBoxContourVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxVectorVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    fieldInfo->module()->fillComboBoxVectorVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxTimeStep(FieldInfo* fieldInfo, QComboBox *cmbTimeStep)
{
    if (!Util::problem()->isSolved())
        return;

    cmbTimeStep->blockSignals(true);

    // store variable
    int timeStep = cmbTimeStep->currentIndex();
    double timeValue;
    if (timeStep == -1){
        timeStep = Util::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
        timeValue = Util::problem()->timeStepToTime(timeStep);
    }
    else
    {
        timeValue = cmbTimeStep->currentText().toDouble();
    }

    // clear combo
    cmbTimeStep->clear();

    QList<double> timeLevels = Util::solutionStore()->timeLevels(fieldInfo);
    int i = 0;
    timeStep = 0;
    foreach (double time, timeLevels)
    {
        cmbTimeStep->addItem(QString("Step: %1, time: %2 s").arg(i - 1).arg(QString::number(time, 'e', 2)),
                             i++);
        if (time < timeValue)
            timeStep = i;
    }

    cmbTimeStep->setCurrentIndex(timeStep);
    if (cmbTimeStep->currentIndex() == -1)
        cmbTimeStep->setCurrentIndex(0);

    cmbTimeStep->blockSignals(false);
}

void fillComboBoxAdaptivityStep(FieldInfo* fieldInfo, int timeStep, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    cmbFieldVariable->blockSignals(true);

    int lastAdaptiveStep = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal, timeStep);

    // store variable
    int adaptivityStep = cmbFieldVariable->currentIndex();
    if (adaptivityStep == -1)
        adaptivityStep = lastAdaptiveStep;

    // clear combo
    cmbFieldVariable->clear();

    for (int step = 0; step <= lastAdaptiveStep; step++)
    {
        cmbFieldVariable->addItem(QString::number(step + 1), step);
    }

    cmbFieldVariable->setCurrentIndex(adaptivityStep);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxSolutionType(QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    cmbFieldVariable->blockSignals(true);

    // store variable
    SolutionMode solutionType = (SolutionMode) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    //    if (adaptivityStep == -1)
    //        adaptivityStep = lastAdaptiveStep;

    // clear combo
    cmbFieldVariable->clear();

    int timeStep = Util::scene()->activeTimeStep();
    int adaptivityStep = Util::scene()->activeAdaptivityStep();

    //TODO je potreba udelat dynamicky, jak se vybira casovy a adaptivni krok ve formulari
    if (Util::solutionStore()->contains(FieldSolutionID(Util::scene()->activeViewField(), timeStep, adaptivityStep, SolutionMode_Normal)))
    {
        cmbFieldVariable->addItem(solutionTypeString(SolutionMode_Normal), SolutionMode_Normal);
    }
    if (Util::solutionStore()->contains(FieldSolutionID(Util::scene()->activeViewField(), timeStep, adaptivityStep, SolutionMode_Reference)))
    {
        cmbFieldVariable->addItem(solutionTypeString(SolutionMode_Reference), SolutionMode_Reference);
    }

    //    for(int step = 0; step <= lastAdaptiveStep; step++)
    //    {
    //        cmbFieldVariable->addItem(QString::number(step), step);
    //    }

    cmbFieldVariable->setCurrentIndex(solutionType);
    cmbFieldVariable->blockSignals(false);
}
