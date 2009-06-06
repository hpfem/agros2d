#include "gui.h"

void fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField)
{
    switch (physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL), PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY);
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY);
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_FLUX), PHYSICFIELDVARIABLE_HEAT_FLUX);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY), PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY);
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS), PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS);
        }
        break;
    default:
        cerr << "Physical field '" + physicFieldString(physicField).toStdString() + "' is not implemented. fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField)" << endl;
        throw;
        break;
    }
}

// ***********************************************************************************************************

SLineEdit::SLineEdit(QWidget *parent) : QLineEdit(parent)
{
    SLineEdit::SLineEdit("", true, parent);
}

SLineEdit::SLineEdit(const QString &contents, bool hasValidator, QWidget *parent) : QLineEdit(contents, parent)
{
    if (hasValidator)
        this->setValidator(new QDoubleValidator(this));
}

double SLineEdit::value()
{
    QScriptEngine engine;

    if (engine.evaluate(text()).isNumber())
        return engine.evaluate(text()).toNumber();
}

void SLineEdit::setValue(double value)
{
    setText(QString::number(value));
}

// ***********************************************************************************************************
