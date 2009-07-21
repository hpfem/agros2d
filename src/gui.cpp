#include "gui.h"

void fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField)
{
    // store variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();

    // clear combo
    cmbFieldVariable->clear();

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
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_POWER_LOSSES), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_POWER_LOSSES);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_ENERGY_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_PERMEABILITY), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_PERMEABILITY);
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
    case PHYSICFIELD_CURRENT:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_POTENTIAL), PHYSICFIELDVARIABLE_CURRENT_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD), PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY), PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_LOSSES), PHYSICFIELDVARIABLE_CURRENT_LOSSES);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY), PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY);
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS), PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS);
        }
        break;
    default:
        std::cerr << "Physical field '" + physicFieldStringKey(physicField).toStdString() + "' is not implemented. fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField)" << endl;
        throw;
        break;
    }

     cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
     if (cmbFieldVariable->currentIndex() == -1)
         cmbFieldVariable->setCurrentIndex(0);
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

    QScriptValue scriptValue = engine.evaluate(text());
    if (scriptValue.isNumber())
        return scriptValue.toNumber();
}

void SLineEdit::setValue(double value)
{
    setText(QString::number(value));
}

// ***********************************************************************************************************

SLineEditValue::SLineEditValue(QWidget *parent) : QLineEdit(parent)
{
    setToolTip(tr("This textedit allows using variables."));
    setText("0");
}

Value SLineEditValue::value()
{
    return Value(text());
}

bool SLineEditValue::evaluate()
{
    Value val = value();
    if (val.evaluate(Util::scene()->problemInfo().scriptStartup))
    {
        m_number = val.number;
        return true;
    }
    else
    {
        setFocus();
        return false;
    }
}

double SLineEditValue::number()
{
    if (evaluate())
        return m_number;
}
