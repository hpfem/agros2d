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

#include "hermes_magnetic.h"

#include "scene.h"
#include "gui.h"

// ****************************************************************************************************************

void ModuleMagnetic::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = Util::scene()->materials[i];
        material->evaluate("magnetic_current_density_external_real", time);
    }
}

// *************************************************************************************************************************************

SceneBoundaryMagneticDialog::SceneBoundaryMagneticDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryMagneticDialog::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValueReal = new ValueLineEdit(this);
    txtValueImag = new ValueLineEdit(this);

    connect(txtValueReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtValueImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtValueReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtValueImag);

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnit, 11, 0);
    layout->addLayout(layoutCurrentDensity, 11, 2);
}

void SceneBoundaryMagneticDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "magnetic_potential")
    {
        txtValueReal->setValue(m_boundary->get_value("magnetic_potential_real"));
        txtValueImag->setValue(m_boundary->get_value("magnetic_potential_imag"));
    }
    else if (m_boundary->type == "magnetic_surface_current")
    {
        txtValueReal->setValue(m_boundary->get_value("magnetic_surface_current_real"));
        txtValueImag->setValue(m_boundary->get_value("magnetic_surface_current_imag"));
    }
}

bool SceneBoundaryMagneticDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (txtValueReal->evaluate())
        if (m_boundary->type == "magnetic_potential")
        {
            m_boundary->values["magnetic_potential_real"] = txtValueReal->value();
        }
        else if (m_boundary->type == "magnetic_surface_current")
        {
            m_boundary->values["magnetic_surface_current_real"] = txtValueReal->value();
        }
        else
            return false;

    if (txtValueImag->evaluate())
        if (m_boundary->type == "magnetic_potential")
        {
            m_boundary->values["magnetic_potential_imag"] = txtValueImag->value();
        }
        else if (m_boundary->type == "magnetic_surface_current")
        {
            m_boundary->values["magnetic_surface_current_imag"] = txtValueImag->value();
        }
        else
            return false;

    return true;
}

void SceneBoundaryMagneticDialog::doTypeChanged(int index)
{
    txtValueReal->setEnabled(false);
    txtValueImag->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "magnetic_potential")
    {
        txtValueReal->setEnabled(true);
        txtValueImag->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic);
        lblValueUnit->setText(tr("<i>A</i><sub>0</sub> (Wb/m)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "magnetic_surface_current")
    {
        txtValueReal->setEnabled(true);
        txtValueImag->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic);
        lblValueUnit->setText(tr("<i>K</i><sub>0</sub> (A/m)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialMagneticDialog::SceneMaterialMagneticDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialMagneticDialog::createContent()
{
    txtPermeability = new ValueLineEdit(this, false, true);
    txtConductivity = new ValueLineEdit(this);
    txtCurrentDensityReal = new ValueLineEdit(this, true);
    txtCurrentDensityImag = new ValueLineEdit(this, true);
    txtCurrentDensityImag->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic);
    txtRemanence = new ValueLineEdit(this);
    txtRemanenceAngle = new ValueLineEdit(this);
    txtVelocityX = new ValueLineEdit(this);
    txtVelocityX->setEnabled(Util::scene()->problemInfo()->problemType == ProblemType_Planar);
    txtVelocityY = new ValueLineEdit(this);
    txtVelocityAngular = new ValueLineEdit(this);
    txtVelocityAngular->setEnabled(Util::scene()->problemInfo()->problemType == ProblemType_Planar);

    connect(txtPermeability, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRemanence, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRemanenceAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVelocityX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVelocityY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVelocityAngular, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

    // remanence
    QGridLayout *layoutRemanence = new QGridLayout();
    layoutRemanence->addWidget(createLabel(tr("<i>B</i><sub>r</sub> (T)"),
                                           tr("Rem. flux dens.")), 0, 0);
    layoutRemanence->addWidget(txtRemanence, 0, 1);
    layoutRemanence->addWidget(createLabel(tr("<i>%1</i> (deg.)").arg(QString::fromUtf8("α")),
                                           tr("Direction of rem.")), 1, 0);
    layoutRemanence->addWidget(txtRemanenceAngle, 1, 1);

    QGroupBox *grpRemanence = new QGroupBox(tr("Permanent magnet"), this);
    grpRemanence->setLayout(layoutRemanence);

    // velocity
    QGridLayout *layoutVelocity = new QGridLayout();
    layoutVelocity->addWidget(createLabel(tr("<i>v</i><sub>%1</sub> (m/s)").arg(Util::scene()->problemInfo()->labelX().toLower()),
                                          tr("Velocity")), 0, 0);
    layoutVelocity->addWidget(txtVelocityX, 0, 1);
    layoutVelocity->addWidget(createLabel(tr("<i>v</i><sub>%1</sub> (m/s)").arg(Util::scene()->problemInfo()->labelY().toLower()),
                                          tr("Velocity")), 1, 0);
    layoutVelocity->addWidget(txtVelocityY, 1, 1);
    layoutVelocity->addWidget(createLabel(tr("<i>%1</i> (rad/s)").arg(QString::fromUtf8("ω")),
                                          tr("Velocity angular")), 2, 0);
    layoutVelocity->addWidget(txtVelocityAngular, 2, 1);

    QGroupBox *grpVelocity = new QGroupBox(tr("Velocity"), this);
    grpVelocity->setLayout(layoutVelocity);

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("μ")),
                                  tr("Permeability")), 10, 0);
    layout->addWidget(txtPermeability, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 11, 0);
    layout->addWidget(txtConductivity, 11, 2);
    layout->addWidget(createLabel(tr("<i>J</i><sub>ext</sub> (A/m<sup>2</sup>)"),
                                  tr("Current density")), 12, 0);
    layout->addLayout(layoutCurrentDensity, 12, 2);
    layout->addWidget(grpRemanence, 13, 0, 1, 3);
    layout->addWidget(grpVelocity, 14, 0, 1, 3);
}

void SceneMaterialMagneticDialog::load()
{
    SceneMaterialDialog::load();

    txtPermeability->setValue(m_material->get_value("magnetic_permeability"));
    txtConductivity->setValue(m_material->get_value("magnetic_conductivity"));
    txtCurrentDensityReal->setValue(m_material->get_value("magnetic_current_density_external_real"));
    txtCurrentDensityImag->setValue(m_material->get_value("magnetic_current_density_external_imag"));
    txtRemanence->setValue(m_material->get_value("magnetic_remanence"));
    txtRemanenceAngle->setValue(m_material->get_value("magnetic_remanence_angle"));
    txtVelocityX->setValue(m_material->get_value("magnetic_velocity_x"));
    txtVelocityY->setValue(m_material->get_value("magnetic_velocity_y"));
    txtVelocityAngular->setValue(m_material->get_value("magnetic_velocity_angular"));
}

bool SceneMaterialMagneticDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    if (txtPermeability->evaluate())
        m_material->values["magnetic_permeability"] = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        m_material->values["magnetic_conductivity"] = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        m_material->values["magnetic_current_density_external_real"] = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        m_material->values["magnetic_current_density_external_imag"] = txtCurrentDensityImag->value();
    else
        return false;

    if (txtRemanence->evaluate())
        m_material->values["magnetic_remanence"] = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        m_material->values["magnetic_remanence_angle"] = txtRemanenceAngle->value();
    else
        return false;

    if (txtVelocityX->evaluate())
        m_material->values["magnetic_velocity_x"] = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        m_material->values["magnetic_velocity_y"] = txtVelocityY->value();
    else
        return false;

    if (txtVelocityAngular->evaluate())
        m_material->values["magnetic_velocity_angular"] = txtVelocityAngular->value();
    else
        return false;

    return true;
}
