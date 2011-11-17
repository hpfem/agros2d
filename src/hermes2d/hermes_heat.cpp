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

#include "hermes_heat.h"

#include "scene.h"
#include "gui.h"

// *******************************************************************************************************

void ModuleHeat::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = dynamic_cast<SceneMaterial *>(Util::scene()->materials[i]);
        material->evaluate("heat_volume_heat", time);
    }
}

// *************************************************************************************************************************************

SceneBoundaryHeatDialog::SceneBoundaryHeatDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryHeatDialog::createContent()
{
    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtHeatFlux = new ValueLineEdit(this);
    txtTemperature = new ValueLineEdit(this);
    txtConvectionHeatTransferCoefficient = new ValueLineEdit(this);
    txtConvectionExternalTemperature = new ValueLineEdit(this);
    txtRadiationEmissivity = new ValueLineEdit(this);
    txtRadiationAmbientTemperature = new ValueLineEdit(this);

    connect(txtHeatFlux, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConvectionHeatTransferCoefficient, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConvectionExternalTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRadiationEmissivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRadiationAmbientTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());
    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>0</sub> (K)"),
                                  tr("Temperature")), 11, 0);
    layout->addWidget(txtTemperature, 11, 2);
    layout->addWidget(createLabel(tr("<i>f</i><sub>0</sub> (W/m<sup>2</sup>)"),
                                  tr("Heat flux")), 12, 0);
    layout->addWidget(txtHeatFlux, 12, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (W/m<sup>2</sup>·K)").arg(QString::fromUtf8("α")),
                                  tr("Heat transfer coef.")), 13, 0);
    layout->addWidget(txtConvectionHeatTransferCoefficient, 13, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>ext</sub> (K)"),
                                  tr("External temperature")), 14, 0);
    layout->addWidget(txtConvectionExternalTemperature, 14, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (-)").arg(QString::fromUtf8("ε")),
                                  tr("Emissivity")), 15, 0);
    layout->addWidget(txtRadiationEmissivity, 15, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>amb</sub> (K)"),
                                  tr("Ambient temperature")), 16, 0);
    layout->addWidget(txtRadiationAmbientTemperature, 16, 2);

}

void SceneBoundaryHeatDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "heat_temperature")
    {
        txtTemperature->setValue(m_boundary->get_value("heat_temperature"));
    }
    else if (m_boundary->type == "heat_heat_flux")
    {
        txtHeatFlux->setValue(m_boundary->get_value("heat_heat_flux"));
        txtConvectionHeatTransferCoefficient->setValue(m_boundary->get_value("heat_convection_heat_transfer_coefficient"));
        txtConvectionExternalTemperature->setValue(m_boundary->get_value("heat_convection_external_temperature"));
        txtRadiationEmissivity->setValue(m_boundary->get_value("heat_radiation_emissivity"));
        txtRadiationAmbientTemperature->setValue(m_boundary->get_value("heat_radiation_ambient_temperature"));
    }
}

bool SceneBoundaryHeatDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (m_boundary->type == "heat_temperature")
    {
        if (txtTemperature->evaluate())
            m_boundary->values["heat_temperature"] = txtTemperature->value();
        else
            return false;
    }
    else if (m_boundary->type == "heat_heat_flux")
    {
        if (txtHeatFlux->evaluate())
            m_boundary->values["heat_heat_flux"] = txtHeatFlux->value();
        else
            return false;
        if (txtConvectionHeatTransferCoefficient->evaluate())
            m_boundary->values["heat_convection_heat_transfer_coefficient"] = txtConvectionHeatTransferCoefficient->value();
        else
            return false;
        if (txtConvectionExternalTemperature->evaluate())
            m_boundary->values["heat_convection_external_temperature"] = txtConvectionExternalTemperature->value();
        else
            return false;
        if (txtRadiationEmissivity->evaluate())
            m_boundary->values["heat_radiation_emissivity"] = txtRadiationEmissivity->value();
        else
            return false;
        if (txtRadiationAmbientTemperature->evaluate())
            m_boundary->values["heat_radiation_ambient_temperature"] = txtRadiationAmbientTemperature->value();
        else
            return false;
    }

    return true;
}

void SceneBoundaryHeatDialog::doTypeChanged(int index)
{
    txtTemperature->setEnabled(false);
    txtHeatFlux->setEnabled(false);
    txtConvectionHeatTransferCoefficient->setEnabled(false);
    txtConvectionExternalTemperature->setEnabled(false);
    txtRadiationEmissivity->setEnabled(false);
    txtRadiationAmbientTemperature->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "heat_temperature")
    {
        txtTemperature->setEnabled(true);
    }
    else if (cmbType->itemData(index) == "heat_heat_flux")
    {
        txtHeatFlux->setEnabled(true);
        txtConvectionHeatTransferCoefficient->setEnabled(true);
        txtConvectionExternalTemperature->setEnabled(true);
        txtRadiationEmissivity->setEnabled(true);
        txtRadiationAmbientTemperature->setEnabled(true);
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialHeatDialog::SceneMaterialHeatDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialHeatDialog::createContent()
{
    txtThermalConductivity = new ValueLineEdit(this, false, true);
    txtThermalConductivity->setMinimumSharp(0.0);
    txtVolumeHeat = new ValueLineEdit(this, true);
    txtDensity = new ValueLineEdit(this);
    txtDensity->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);
    txtSpecificHeat = new ValueLineEdit(this);
    txtSpecificHeat->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);

    connect(txtThermalConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtSpecificHeat, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (W/m·K)").arg(QString::fromUtf8("λ")),
                                  tr("Thermal conductivity")), 10, 0);
    layout->addWidget(txtThermalConductivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>Q</i> (W/m<sup>3</sup>)"),
                                  tr("Volume heat")), 11, 0);
    layout->addWidget(txtVolumeHeat, 11, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (kg/m<sup>3</sup>)").arg(QString::fromUtf8("ρ")),
                                  tr("Mass density")), 12, 0);
    layout->addWidget(txtDensity, 12, 2);
    layout->addWidget(createLabel(tr("<i>c</i><sub>p</sub> (J/kg·K)"),
                                  tr("Specific heat")), 13, 0);
    layout->addWidget(txtSpecificHeat, 13, 2);
}

void SceneMaterialHeatDialog::load()
{
    SceneMaterialDialog::load();

    txtThermalConductivity->setValue(m_material->get_value("heat_conductivity"));
    txtVolumeHeat->setValue(m_material->get_value("heat_volume_heat"));
    txtDensity->setValue(m_material->get_value("heat_density"));
    txtSpecificHeat->setValue(m_material->get_value("heat_specific_heat"));
}

bool SceneMaterialHeatDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;;

    if (txtThermalConductivity->evaluate())
        m_material->values["heat_conductivity"] = txtThermalConductivity->value();
    else
        return false;

    if (txtVolumeHeat->evaluate())
        m_material->values["heat_volume_heat"] = txtVolumeHeat->value();
    else
        return false;

    if (txtDensity->evaluate())
        m_material->values["heat_density"] = txtDensity->value();
    else
        return false;

    if (txtSpecificHeat->evaluate())
        m_material->values["heat_specific_heat"] = txtSpecificHeat->value();
    else
        return false;

    return true;
}
