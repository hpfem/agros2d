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

class WeakFormMagnetic : public WeakFormAgros
{
public:
    WeakFormMagnetic(int neq) : WeakFormAgros(neq) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == "magnetic_surface_current")
                {
                    if (fabs(boundary->get_value("magnetic_surface_current_real").number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->get_value("magnetic_surface_current_real").number,
                                                                                                        HERMES_PLANAR));
                    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                        if (fabs(boundary->get_value("magnetic_surface_current_imag").number) > EPS_ZERO)
                            add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                            QString::number(i + 1).toStdString(),
                                                                                                            boundary->get_value("magnetic_surface_current_imag").number,
                                                                                                            HERMES_PLANAR));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterial *material = Util::scene()->labels[i]->material;

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                // steady state and transient analysis
                add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(0, 0,
                                                                                                         QString::number(i).toStdString(),
                                                                                                         1.0 / (material->get_value("magnetic_permeability").number * MU0),
                                                                                                         HERMES_NONSYM,
                                                                                                         convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                         (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));

                // velocity
                if ((fabs(material->get_value("magnetic_conductivity").number) > EPS_ZERO) &&
                        ((fabs(material->get_value("magnetic_velocity_x").number) > EPS_ZERO) ||
                         (fabs(material->get_value("magnetic_velocity_y").number) > EPS_ZERO) ||
                         (fabs(material->get_value("magnetic_velocity_angular").number) > EPS_ZERO)))
                    add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostaticsVelocity(0, 0,
                                                                                                                     QString::number(i).toStdString(),
                                                                                                                     material->get_value("magnetic_conductivity").number,
                                                                                                                     material->get_value("magnetic_velocity_x").number,
                                                                                                                     material->get_value("magnetic_velocity_y").number,
                                                                                                                     material->get_value("magnetic_velocity_angular").number));

                // external current density
                if (fabs(material->get_value("magnetic_current_density_external_real").number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->get_value("magnetic_current_density_external_real").number,
                                                                                                   HERMES_PLANAR));

                // remanence
                if (fabs(material->get_value("magnetic_remanence").number) > EPS_ZERO)
                    add_vector_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostaticsRemanence(0,
                                                                                                                      QString::number(i).toStdString(),
                                                                                                                      material->get_value("magnetic_permeability").number * MU0,
                                                                                                                      material->get_value("magnetic_remanence").number,
                                                                                                                      material->get_value("magnetic_remanence_angle").number,
                                                                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));

                // harmonic analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(1, 1,
                                                                                                             QString::number(i).toStdString(),
                                                                                                             1.0 / (material->get_value("magnetic_permeability").number * MU0),
                                                                                                             HERMES_NONSYM,
                                                                                                             convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                             (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 5)));

                    if (fabs(material->get_value("magnetic_conductivity").number) > EPS_ZERO)
                    {
                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  - 2 * M_PI * Util::scene()->problemInfo()->frequency * material->get_value("magnetic_conductivity").number,
                                                                                                  HERMES_NONSYM,
                                                                                                  HERMES_PLANAR));

                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 0,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  2 * M_PI * Util::scene()->problemInfo()->frequency * material->get_value("magnetic_conductivity").number,
                                                                                                  HERMES_NONSYM,
                                                                                                  HERMES_PLANAR));
                    }

                    // external current density
                    if (fabs(material->get_value("magnetic_current_density_external_imag").number) > EPS_ZERO)
                        add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                       QString::number(i).toStdString(),
                                                                                                       material->get_value("magnetic_current_density_external_imag").number,
                                                                                                       HERMES_PLANAR));
                }

                // transient analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                {
                    if (fabs(material->get_value("magnetic_conductivity").number) > EPS_ZERO)
                    {
                        if (solution.size() > 0)
                        {
                            add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                                      QString::number(i).toStdString(),
                                                                                                      material->get_value("magnetic_conductivity").number / Util::scene()->problemInfo()->timeStep.number,
                                                                                                      HERMES_SYM,
                                                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));

                            add_vector_form(new CustomVectorFormTimeDep(0,
                                                                        QString::number(i).toStdString(),
                                                                        material->get_value("magnetic_conductivity").number / Util::scene()->problemInfo()->timeStep.number,
                                                                        solution[0],
                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                        }
                    }
                }
            }
        }
    }
};

// ****************************************************************************************************************

int ModuleMagnetic::number_of_solution() const
{
    return (get_analysis_type() == AnalysisType_Harmonic) ? 2 : 1;
}

Hermes::vector<SolutionArray *> ModuleMagnetic::solve(ProgressItemSolve *progressItemSolve)
{
    if (!solve_init_variables())
        return Hermes::vector<SolutionArray *>();

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            if (boundary->type == "magnetic_potential")
            {
                bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                       boundary->get_value("magnetic_potential_real").number));
                bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                       boundary->get_value("magnetic_potential_imag").number));
            }
        }
    }
    Hermes::vector<EssentialBCs> bcs(bc1, bc2);

    WeakFormMagnetic wf(number_of_solution());

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

void ModuleMagnetic::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = Util::scene()->materials[i];

        material->get_value("magnetic_current_density_external_real").evaluate(time);
        material->get_value("magnetic_current_density_external_imag").evaluate(time);
    }
}

// **************************************************************************************************************************
// rewrite

SceneBoundary *ModuleMagnetic::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "magnetic_surface_current");
}

SceneBoundary *ModuleMagnetic::newBoundary(PyObject *self, PyObject *args)
{
    /*
    double value_real;
    double value_imag = 0.0;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|d", &name, &type, &value_real, &value_imag))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryMagnetic(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value_real)),
                                         Value(QString::number(value_imag)));
    }

    return NULL;
    */
}

SceneBoundary *ModuleMagnetic::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
    double value_real, value_imag = 0.0;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|d", &name, &type, &value_real, &value_imag))
    {
        if (SceneBoundaryMagnetic *marker = dynamic_cast<SceneBoundaryMagnetic *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                marker->value_real = Value(QString::number(value_real));
                marker->value_imag = Value(QString::number(value_imag));
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
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

SceneMaterial *ModuleMagnetic::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

SceneMaterial *ModuleMagnetic::newMaterial(PyObject *self, PyObject *args)
{
    /*
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialMagnetic(name,
                                         Value(QString::number(current_density_real)),
                                         Value(QString::number(current_density_imag)),
                                         Value(QString::number(permeability)),
                                         Value(QString::number(conductivity)),
                                         Value(QString::number(remanence)),
                                         Value(QString::number(remanence_angle)),
                                         Value(QString::number(velocity_x)),
                                         Value(QString::number(velocity_y)),
                                         Value(QString::number(velocity_angular)));
    }

    return NULL;
    */
}

SceneMaterial *ModuleMagnetic::modifyMaterial(PyObject *self, PyObject *args)
{
    /*
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        if (SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->getMaterial(name)))
        {
            marker->current_density_real = Value(QString::number(current_density_real));
            marker->current_density_imag = Value(QString::number(current_density_imag));
            marker->permeability = Value(QString::number(permeability));
            marker->conductivity = Value(QString::number(conductivity));
            marker->remanence = Value(QString::number(remanence));
            marker->remanence_angle = Value(QString::number(remanence_angle));
            marker->velocity_x = Value(QString::number(velocity_x));
            marker->velocity_y = Value(QString::number(velocity_y));
            marker->velocity_angular = Value(QString::number(velocity_angular));
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
            m_boundary->values[m_boundary->get_boundary_type_variable("magnetic_potential_real")] = txtValueReal->value();
        }
        else if (m_boundary->type == "magnetic_surface_current")
        {
            m_boundary->values[m_boundary->get_boundary_type_variable("magnetic_surface_current_real")] = txtValueReal->value();
        }
        else
            return false;

    if (txtValueImag->evaluate())
        if (m_boundary->type == "magnetic_potential")
        {
            m_boundary->values[m_boundary->get_boundary_type_variable("magnetic_potential_imag")] = txtValueImag->value();
        }
        else if (m_boundary->type == "magnetic_surface_current")
        {
            m_boundary->values[m_boundary->get_boundary_type_variable("magnetic_surface_current_imag")] = txtValueImag->value();
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
    txtPermeability = new ValueLineEdit(this);
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
        m_material->values[m_material->get_material_type("magnetic_permeability")] = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        m_material->values[m_material->get_material_type("magnetic_conductivity")] = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        m_material->values[m_material->get_material_type("magnetic_current_density_external_real")] = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        m_material->values[m_material->get_material_type("magnetic_current_density_external_imag")] = txtCurrentDensityImag->value();
    else
        return false;

    if (txtRemanence->evaluate())
        m_material->values[m_material->get_material_type("magnetic_remanence")] = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        m_material->values[m_material->get_material_type("magnetic_remanence_angle")] = txtRemanenceAngle->value();
    else
        return false;

    if (txtVelocityX->evaluate())
        m_material->values[m_material->get_material_type("magnetic_velocity_x")] = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        m_material->values[m_material->get_material_type("magnetic_velocity_y")] = txtVelocityY->value();
    else
        return false;

    if (txtVelocityAngular->evaluate())
        m_material->values[m_material->get_material_type("magnetic_velocity_angular")] = txtVelocityAngular->value();
    else
        return false;

    return true;
}
