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

class WeakFormHeatTransfer : public WeakFormAgros
{
public:
    WeakFormHeatTransfer() : WeakFormAgros() { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == "heat_heat_flux")
                {
                    // vector flux term


                    double flux = boundary->get_value("heat_heat_flux").number +
                            boundary->get_value("heat_heat_transfer_coefficient").number * boundary->get_value("heat_external_temperature").number;

                    if (fabs(flux) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        flux,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                    if (fabs(boundary->get_value("heat_heat_transfer_coefficient").number) > EPS_ZERO)
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(0, 0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->get_value("heat_heat_transfer_coefficient").number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterial *material = Util::scene()->labels[i]->material;

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               material->get_value("heat_conductivity").number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));

                if (fabs(material->get_value("heat_volume_heat").number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->get_value("heat_volume_heat").number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));

                // transient analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                {
                    if ((fabs(material->get_value("heat_density").number) > EPS_ZERO)
                            && (fabs(material->get_value("heat_specific_heat").number) > EPS_ZERO))
                    {
                        if (solution.size() > 0)
                        {
                            add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                                      QString::number(i).toStdString(),
                                                                                                      material->get_value("heat_density").number * material->get_value("heat_specific_heat").number / Util::scene()->problemInfo()->timeStep.number,
                                                                                                      HERMES_SYM,
                                                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));

                            add_vector_form(new CustomVectorFormTimeDep(0,
                                                                        QString::number(i).toStdString(),
                                                                        material->get_value("heat_density").number * material->get_value("heat_specific_heat").number / Util::scene()->problemInfo()->timeStep.number,
                                                                        solution[0],
                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                        }
                    }
                }
            }
        }
    }
};

// *******************************************************************************************************

Hermes::vector<SolutionArray *> ModuleHeat::solve(ProgressItemSolve *progressItemSolve)
{
    if (!solve_init_variables())
        return Hermes::vector<SolutionArray *>();

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            if (boundary->type == "heat_temperature")
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                       boundary->get_value("heat_temperature").number));
        }
    }

    WeakFormHeatTransfer wf;

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

void ModuleHeat::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = dynamic_cast<SceneMaterial *>(Util::scene()->materials[i]);
        material->get_value("heat_volume_heat").evaluate(time);
    }
}

// *************************************************************************************************************************************
// rewrite

SceneBoundary *ModuleHeat::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "heat_temperature");
}

SceneBoundary *ModuleHeat::newBoundary(PyObject *self, PyObject *args)
{
    /*
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
            return new SceneBoundaryHeat(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value)));
        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            return new SceneBoundaryHeat(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value)),
                                         Value(QString::number(h)),
                                         Value(QString::number(externaltemperature)));
    }

    return NULL;
    */
}

SceneBoundary *ModuleHeat::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        if (SceneBoundaryHeat *marker = dynamic_cast<SceneBoundaryHeat *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
            }

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
                marker->temperature = Value(QString::number(value));

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            {
                marker->heatFlux = Value(QString::number(value));
                marker->h = Value(QString::number(h));
                marker->externalTemperature = Value(QString::number(externaltemperature));
                return marker;
            }
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
    */
}

SceneMaterial *ModuleHeat::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

SceneMaterial *ModuleHeat::newMaterial(PyObject *self, PyObject *args)
{
    /*
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialHeat(name,
                                     Value(QString::number(volume_heat)),
                                     Value(QString::number(thermal_conductivity)),
                                     Value(QString::number(density)),
                                     Value(QString::number(specific_heat)));
    }

    return NULL;
    */
}

SceneMaterial *ModuleHeat::modifyMaterial(PyObject *self, PyObject *args)
{
    /*
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        if (SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(Util::scene()->getMaterial(name)))
        {
            marker->volume_heat = Value(QString::number(volume_heat));
            marker->thermal_conductivity = Value(QString::number(thermal_conductivity));
            marker->density = Value(QString::number(density));
            marker->specific_heat = Value(QString::number(specific_heat));
            return marker;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
    */
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
    txtHeatTransferCoefficient = new ValueLineEdit(this);
    txtExternalTemperature = new ValueLineEdit(this);

    connect(txtHeatFlux, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtHeatTransferCoefficient, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtExternalTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

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
    layout->addWidget(txtHeatTransferCoefficient, 13, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>ext</sub> (K)"),
                                  tr("External temperature")), 14, 0);
    layout->addWidget(txtExternalTemperature, 14, 2);

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
        txtHeatTransferCoefficient->setValue(m_boundary->get_value("heat_heat_transfer_coefficient"));
        txtExternalTemperature->setValue(m_boundary->get_value("heat_external_temperature"));
    }
}

bool SceneBoundaryHeatDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (m_boundary->type == "heat_temperature")
    {
        if (txtTemperature->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_temperature")] = txtTemperature->value();
        else
            return false;
    }
    else if (m_boundary->type == "heat_heat_flux")
    {
        if (txtHeatFlux->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_heat_flux")] = txtHeatFlux->value();
        else
            return false;
        if (txtHeatTransferCoefficient->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_heat_transfer_coefficient")] = txtHeatTransferCoefficient->value();
        else
            return false;
        if (txtExternalTemperature->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_external_temperature")] = txtExternalTemperature->value();
        else
            return false;
    }

    return true;
}

void SceneBoundaryHeatDialog::doTypeChanged(int index)
{
    txtTemperature->setEnabled(false);
    txtHeatFlux->setEnabled(false);
    txtHeatTransferCoefficient->setEnabled(false);
    txtExternalTemperature->setEnabled(false);

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
        txtHeatTransferCoefficient->setEnabled(true);
        txtExternalTemperature->setEnabled(true);
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
    txtThermalConductivity = new ValueLineEdit(this);
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
        m_material->values[m_material->get_material_type("heat_conductivity")] = txtThermalConductivity->value();
    else
        return false;

    if (txtVolumeHeat->evaluate())
        m_material->values[m_material->get_material_type("heat_volume_heat")] = txtVolumeHeat->value();
    else
        return false;

    if (txtDensity->evaluate())
        m_material->values[m_material->get_material_type("heat_density")] = txtDensity->value();
    else
        return false;

    if (txtSpecificHeat->evaluate())
        m_material->values[m_material->get_material_type("heat_specific_heat")] = txtSpecificHeat->value();
    else
        return false;

    return true;
}
