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

#include "hermes_current.h"

#include "scene.h"
#include "gui.h"

// *******************************************************************************************************
// rewrite

SceneBoundary *ModuleCurrent::newBoundary()
{
    return new SceneBoundary(tr("new boundary condition").toStdString(), "current_potential");
}

SceneBoundary *ModuleCurrent::newBoundary(PyObject *self, PyObject *args)
{
    /*
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryCurrent(name,
                                        physicFieldBCFromStringKey(type),
                                        Value(QString::number(value)));
    }

    return NULL;
    */
}

SceneBoundary *ModuleCurrent::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneBoundaryCurrent *marker = dynamic_cast<SceneBoundaryCurrent *>(Util::scene()->getBoundary(name)))
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
    */
}

SceneMaterial *ModuleCurrent::newMaterial(PyObject *self, PyObject *args)
{
    /*
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialCurrent(name,
                                        Value(QString::number(conductivity)));
    }

    return NULL;
    */
}

SceneMaterial *ModuleCurrent::modifyMaterial(PyObject *self, PyObject *args)
{
    /*
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        if (SceneMaterialCurrent *marker = dynamic_cast<SceneMaterialCurrent *>(Util::scene()->getMaterial(name)))
        {
            marker->conductivity = Value(QString::number(conductivity));
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

SceneBoundaryCurrentDialog::SceneBoundaryCurrentDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryCurrentDialog::createContent()
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

void SceneBoundaryCurrentDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "current_potential")
    {
        txtValue->setValue(m_boundary->get_value("current_potential"));
    }
    else if (m_boundary->type == "current_inward_current_flow")
    {
        txtValue->setValue(m_boundary->get_value("current_inward_current_flow"));
    }
}

bool SceneBoundaryCurrentDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (txtValue->evaluate())
        if (m_boundary->type == "current_potential")
        {
            m_boundary->values["current_potential"] = txtValue->value();
        }
        else if (m_boundary->type == "current_inward_current_flow")
        {
            m_boundary->values["current_inward_current_flow"] = txtValue->value();
        }
        else
            return false;

    return true;
}

void SceneBoundaryCurrentDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "current_potential")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (V)").arg(QString::fromUtf8("φ")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "current_inward_current_flow")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>J</i><sub>0</sub> (A/m<sup>2</sup>)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialCurrentDialog::SceneMaterialCurrentDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialCurrentDialog::createContent()
{
    txtConductivity = new ValueLineEdit(this);
    txtConductivity->setMinimumSharp(0.0);
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 10, 0);
    layout->addWidget(txtConductivity, 10, 2);
}

void SceneMaterialCurrentDialog::load()
{
    SceneMaterialDialog::load();

    txtConductivity->setValue(m_material->get_value("current_conductivity"));
}

bool SceneMaterialCurrentDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;;

    if (txtConductivity->evaluate())
        m_material->values["current_conductivity"] = txtConductivity->value();
    else
        return false;

    return true;
}
