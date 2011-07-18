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

class WeakFormElectrostatic : public WeakFormAgros
{
public:
    WeakFormElectrostatic() : WeakFormAgros() { }
};

Hermes::vector<SolutionArray *> ModuleElectrostatic::solve(ProgressItemSolve *progressItemSolve)
{
    if (!solve_init_variables())
        return Hermes::vector<SolutionArray *>();

    // boundary conditions
    /*
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            for (std::map<Hermes::Module::BoundaryTypeVariable *, Value>::iterator it = boundary->values.begin(); it != boundary->values.end(); ++it)
            {
                if (!it->second.evaluate()) return false;

                if (boundary->type == "electrostatic_potential")
                    bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                           boundary->get_value("electrostatic_potential").number));
            }
        }
    }
    */

    // boundary conditions
    Hermes::vector<EssentialBCs> bcs;
    for (int i = 0; i < Util::scene()->problemInfo()->module()->number_of_solution(); i++)
        bcs.push_back(EssentialBCs());

    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(boundary->type);

            for (std::map<int, Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->essential.begin();
                 it != boundary_type->essential.end(); ++it)
            {
                bcs[it->first - 1].add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                                  boundary->values[it->second].number));

            }
        }
    }


    WeakFormElectrostatic wf;

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

// **************************************************************************************************************************
// rewrite

SceneBoundary *ModuleElectrostatic::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "electrostatic_potential");
}

SceneBoundary *ModuleElectrostatic::newBoundary(PyObject *self, PyObject *args)
{
    /*
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryElectrostatic(name,
                                              type,
                                              Value(QString::number(value)));
    }

    return NULL;
    */
}

SceneBoundary *ModuleElectrostatic::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneBoundaryElectrostatic *marker = dynamic_cast<SceneBoundaryElectrostatic *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                marker->value = Value(QString::number(value));
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

SceneMaterial *ModuleElectrostatic::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

SceneMaterial *ModuleElectrostatic::newMaterial(PyObject *self, PyObject *args)
{
    /*
    double charge_density, permittivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &charge_density, &permittivity))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialElectrostatic(name,
                                              Value(QString::number(charge_density)),
                                              Value(QString::number(permittivity)));
    }

    return NULL;
    */
}

SceneMaterial *ModuleElectrostatic::modifyMaterial(PyObject *self, PyObject *args)
{
    /*
    double charge_density, permittivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &charge_density, &permittivity))
    {
        if (SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(Util::scene()->getMaterial(name)))
        {
            marker->charge_density = Value(QString::number(charge_density));
            marker->permittivity = Value(QString::number(permittivity));
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
            m_boundary->values[m_boundary->get_boundary_type_variable("electrostatic_potential")] = txtValue->value();
        }
        else if (m_boundary->type == "electrostatic_surface_charge_density")
        {
            m_boundary->values[m_boundary->get_boundary_type_variable("electrostatic_surface_charge_density")] = txtValue->value();
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
        m_material->values[m_material->get_material_type_variable("electrostatic_permittivity")] = txtPermittivity->value();
    else
        return false;

    if (txtChargeDensity->evaluate())
        m_material->values[m_material->get_material_type_variable("electrostatic_charge_density")] = txtChargeDensity->value();
    else
        return false;

    return true;
}
