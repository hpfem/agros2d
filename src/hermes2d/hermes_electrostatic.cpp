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

#include "hermes_electrostatic.h"

#include "scene.h"
#include "gui.h"

// *************************************************************************************************************************************

SceneBoundaryElectrostaticDialog::SceneBoundaryElectrostaticDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryElectrostaticDialog::createContent()
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

void SceneBoundaryElectrostaticDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "electrostatic_potential")
    {
        txtValue->setValue(m_boundary->get_value("electrostatic_potential"));
    }
    else if (m_boundary->type == "electrostatic_surface_charge_density")
    {
        txtValue->setValue(m_boundary->get_value("electrostatic_surface_charge_density"));
    }
}

bool SceneBoundaryElectrostaticDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (txtValue->evaluate())
        if (m_boundary->type == "electrostatic_potential")
        {
            m_boundary->values["electrostatic_potential"] = txtValue->value();
        }
        else if (m_boundary->type == "electrostatic_surface_charge_density")
        {
            m_boundary->values["electrostatic_surface_charge_density"] = txtValue->value();
        }
        else
            return false;

    return true;
}

void SceneBoundaryElectrostaticDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "electrostatic_potential")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (V)").arg(QString::fromUtf8("φ")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "electrostatic_surface_charge_density")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (C/m<sup>2</sup>)").arg(QString::fromUtf8("σ")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialElectrostaticDialog::SceneMaterialElectrostaticDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialElectrostaticDialog::createContent()
{
    txtPermittivity = new ValueLineEdit(this);
    txtPermittivity->setMinimumSharp(0.0);
    txtChargeDensity = new ValueLineEdit(this);
    connect(txtPermittivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtChargeDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("ε")),
                                  tr("Permittivity")), 10, 0);
    layout->addWidget(txtPermittivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (C/m<sup>3</sup>)").arg(QString::fromUtf8("ρ")),
                                  tr("Charge density")), 11, 0);
    layout->addWidget(txtChargeDensity, 11, 2);
}

void SceneMaterialElectrostaticDialog::load()
{
    SceneMaterialDialog::load();

    txtPermittivity->setValue(m_material->get_value("electrostatic_permittivity"));
    txtChargeDensity->setValue(m_material->get_value("electrostatic_charge_density"));
}

bool SceneMaterialElectrostaticDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    if (txtPermittivity->evaluate())
        m_material->values["electrostatic_permittivity"] = txtPermittivity->value();
    else
        return false;

    if (txtChargeDensity->evaluate())
        m_material->values["electrostatic_charge_density"] = txtChargeDensity->value();
    else
        return false;

    return true;
}
