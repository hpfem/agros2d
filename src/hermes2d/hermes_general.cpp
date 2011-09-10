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

#include "hermes_general.h"

#include "scene.h"
#include "gui.h"

// **************************************************************************************************************************
// rewrite

SceneBoundary *ModuleGeneral::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "electrostatic_potential");
}

// *************************************************************************************************************************************

SceneBoundaryGeneralDialog::SceneBoundaryGeneralDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

    load();
    setSize();
}

void SceneBoundaryGeneralDialog::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValue = new ValueLineEdit(this);
    connect(txtValue, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnit, 11, 0);
    layout->addWidget(txtValue, 11, 2);
}

void SceneBoundaryGeneralDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "general_essential")
    {
        txtValue->setValue(m_boundary->get_value("general_essential"));
    }
    else if (m_boundary->type == "general_neumann")
    {
        txtValue->setValue(m_boundary->get_value("general_neumann"));
    }
}

bool SceneBoundaryGeneralDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (txtValue->evaluate())
        if (m_boundary->type == "general_essential")
        {
            m_boundary->values["general_essential"] = txtValue->value();
        }
        else if (m_boundary->type == "general_neumann")
        {
            m_boundary->values["general_neumann"] = txtValue->value();
        }
        else
            return false;

    return true;
}

void SceneBoundaryGeneralDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "general_essential")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (-)").arg(QString::fromUtf8("u")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "general_neumann")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (-)").arg(QString::fromUtf8("n")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialGeneralDialog::SceneMaterialGeneralDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    // tab order
    setTabOrder(txtName, txtConstant1);
    setTabOrder(txtConstant1, txtRightSide);

    load();
    setSize();
}

void SceneMaterialGeneralDialog::createContent()
{
    txtConstant1 = new ValueLineEdit(this);
    txtConstant1->setMinimumSharp(0.0);
    txtConstant2 = new ValueLineEdit(this);
    txtRightSide = new ValueLineEdit(this);

    connect(txtConstant1, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRightSide, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>c</i> (-)"),
                                  tr("Constant c")), 10, 0);
    layout->addWidget(txtConstant1, 10, 2);
    layout->addWidget(createLabel(tr("<i>a</i> (-)"),
                                  tr("Constant a")), 11, 0);
    layout->addWidget(txtConstant2, 11, 2);
    layout->addWidget(createLabel(tr("<i>r</i> (-)"),
                                  tr("Rightside")), 15, 0);
    layout->addWidget(txtRightSide, 15, 2);
}

void SceneMaterialGeneralDialog::load()
{
    SceneMaterialDialog::load();

    txtConstant1->setValue(m_material->get_value("general_constant_1"));
    txtConstant2->setValue(m_material->get_value("general_constant_2"));
    txtRightSide->setValue(m_material->get_value("general_rightside"));
}

bool SceneMaterialGeneralDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    if (txtConstant1->evaluate())
        m_material->values["general_constant_1"] = txtConstant1->value();
    else
        return false;

    if (txtConstant2->evaluate())
        m_material->values["general_constant_2"] = txtConstant2->value();
    else
        return false;

    if (txtRightSide->evaluate())
        m_material->values["general_rightside"] = txtRightSide->value();
    else
        return false;

    return true;
}
