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

#include "util.h"
#include "scene.h"
#include "scripteditordialog.h"

static bool logFile;

static QHash<PhysicField, QString> physicFieldList;
static QHash<PhysicFieldVariable, QString> physicFieldVariableList;
static QHash<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QHash<PhysicFieldBC, QString> physicFieldBCList;
static QHash<SceneViewPostprocessorShow, QString> sceneViewPostprocessorShowList;
static QHash<AdaptivityType, QString> adaptivityTypeList;
static QHash<AnalysisType, QString> analysisTypeList;

QString analysisTypeToStringKey(AnalysisType analysisType) { return analysisTypeList[analysisType]; }
AnalysisType analysisTypeFromStringKey(const QString &analysisType) { return analysisTypeList.key(analysisType); }

QString physicFieldToStringKey(PhysicField physicField) { return physicFieldList[physicField]; }
PhysicField physicFieldFromStringKey(const QString &physicField) { return physicFieldList.key(physicField); }

QString physicFieldVariableToStringKey(PhysicFieldVariable physicFieldVariable) { return physicFieldVariableList[physicFieldVariable]; }
PhysicFieldVariable physicFieldVariableFromStringKey(const QString &physicFieldVariable) { return physicFieldVariableList.key(physicFieldVariable); }

QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QString physicFieldBCToStringKey(PhysicFieldBC physicFieldBC) { return physicFieldBCList[physicFieldBC]; }
PhysicFieldBC physicFieldBCFromStringKey(const QString &physicFieldBC) { return physicFieldBCList.key(physicFieldBC); }

QString sceneViewPostprocessorShowToStringKey(SceneViewPostprocessorShow sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList[sceneViewPostprocessorShow]; }
SceneViewPostprocessorShow sceneViewPostprocessorShowFromStringKey(const QString &sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList.key(sceneViewPostprocessorShow); }

QString adaptivityTypeToStringKey(AdaptivityType adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

void initLists()
{
    analysisTypeList.insert(ANALYSISTYPE_UNDEFINED, "");
    analysisTypeList.insert(ANALYSISTYPE_STEADYSTATE, "steadystate");
    analysisTypeList.insert(ANALYSISTYPE_TRANSIENT, "transient");
    analysisTypeList.insert(ANALYSISTYPE_HARMONIC, "harmonic");

    // PHYSICFIELD
    physicFieldList.insert(PHYSICFIELD_UNDEFINED, "");
    physicFieldList.insert(PHYSICFIELD_GENERAL, "general");
    physicFieldList.insert(PHYSICFIELD_ELECTROSTATIC, "electrostatic");
    physicFieldList.insert(PHYSICFIELD_CURRENT, "current");
    physicFieldList.insert(PHYSICFIELD_HEAT, "heat");
    physicFieldList.insert(PHYSICFIELD_ELASTICITY, "elasticity");
    physicFieldList.insert(PHYSICFIELD_MAGNETIC, "magnetic");

    // PHYSICFIELDVARIABLE
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_UNDEFINED, "");

    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_GENERAL_VARIABLE, "general_variable");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_GENERAL_GRADIENT, "general_gradient");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_GENERAL_CONSTANT, "general_constant");

    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL, "electrostatic_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD, "electrostatic_electric_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT, "electrostatic_displacement");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY, "electrostatic_energy_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY, "electrostatic_permittivity");

    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL, "magnetic_vector_potential_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG, "magnetic_vector_potential_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL, "magnetic_vector_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL, "magnetic_flux_density_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG, "magnetic_flux_density_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY, "magnetic_flux_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL, "magnetic_magnetic_field_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG, "magnetic_magnetic_field_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD, "magnetic_magnetic_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL, "magnetic_current_density_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG, "magnetic_current_density_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY, "magnetic_current_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL, "magnetic_current_density_induced_transform_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG, "magnetic_current_density_induced_transform_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM, "magnetic_current_density_induced_transform");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL, "magnetic_current_density_induced_velocity_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG, "magnetic_current_density_induced_velocity_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY, "magnetic_current_density_induced_velocity");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL, "magnetic_current_density_total_real");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG, "magnetic_current_density_total_imag");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL, "magnetic_current_density_total");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES, "magnetic_power_losses");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_LORENTZ_FORCE, "magnetic_lorentz_force");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY, "magnetic_energy_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY, "magnetic_permeability");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY, "magnetic_conductivity");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE, "magnetic_remanence");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY, "magnetic_velocity");

    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_POTENTIAL, "current_potential");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD, "current_electic_field");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY, "current_current_density");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_LOSSES, "current_power_losses");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY, "current_conductivity");

    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE, "heat_temperature");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT, "heat_temperature_gradient");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_FLUX, "heat_heat_flux");
    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY, "heat_conductivity");

    physicFieldVariableList.insert(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS, "elasticity_von_mises_stress");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_UNDEFINED, "");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_SCALAR, "scalar");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_MAGNITUDE, "magnitude");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_X, "x");
    physicFieldVariableCompList.insert(PHYSICFIELDVARIABLECOMP_Y, "y");

    // PHYSICFIELDBC
    physicFieldBCList.insert(PHYSICFIELDBC_UNDEFINED, "");
    physicFieldBCList.insert(PHYSICFIELDBC_NONE, "none");
    physicFieldBCList.insert(PHYSICFIELDBC_GENERAL_VALUE, "general_value");
    physicFieldBCList.insert(PHYSICFIELDBC_GENERAL_DERIVATIVE, "general_derivative");
    physicFieldBCList.insert(PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL, "electrostatic_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE, "electrostatic_surface_charge_density");
    physicFieldBCList.insert(PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL, "magnetic_vector_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT, "magnetic_surface_current_density");
    physicFieldBCList.insert(PHYSICFIELDBC_HEAT_TEMPERATURE, "heat_temperature");
    physicFieldBCList.insert(PHYSICFIELDBC_HEAT_HEAT_FLUX, "heat_heat_flux");
    physicFieldBCList.insert(PHYSICFIELDBC_CURRENT_POTENTIAL, "current_potential");
    physicFieldBCList.insert(PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW, "current_inward_current_flow");
    physicFieldBCList.insert(PHYSICFIELDBC_ELASTICITY_FIXED, "elasticity_fixed");
    physicFieldBCList.insert(PHYSICFIELDBC_ELASTICITY_FREE, "elasticity_free");

    // SCENEVIEW_POSTPROCESSOR_SHOW
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED, "");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_NONE, "none");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW, "scalar");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D, "scalar3d");
    sceneViewPostprocessorShowList.insert(SCENEVIEW_POSTPROCESSOR_SHOW_ORDER, "order");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(ADAPTIVITYTYPE_UNDEFINED, "");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_NONE, "disabled");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_H, "h-adaptivity");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_P, "p-adaptivity");
    adaptivityTypeList.insert(ADAPTIVITYTYPE_HP, "hp-adaptivity");    
}

QString physicFieldVariableString(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
        return QObject::tr("Variable");
    case PHYSICFIELDVARIABLE_GENERAL_GRADIENT:
        return QObject::tr("Gradient");
    case PHYSICFIELDVARIABLE_GENERAL_CONSTANT:
        return QObject::tr("Constant");

    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("Scalar potential");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        return QObject::tr("Electric field");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        return QObject::tr("Displacement");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        return QObject::tr("Energy density");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        return QObject::tr("Permittivity");

    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
        return QObject::tr("Vector potential - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:
        return QObject::tr("Vector potential - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
        return QObject::tr("Vector potential");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL:
        return QObject::tr("Flux density - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG:
        return QObject::tr("Flux density - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
        return QObject::tr("Flux density");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL:
        return QObject::tr("Magnetic field - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG:
        return QObject::tr("Magnetic field - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
        return QObject::tr("Magnetic field");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
        return QObject::tr("Current density - external - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
        return QObject::tr("Current density - external - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
        return QObject::tr("Current density - external");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        return QObject::tr("Current density - total - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        return QObject::tr("Current density - total - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
        return QObject::tr("Current density - total");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
        return QObject::tr("Current density - induced transform - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
        return QObject::tr("Current density - induced transform - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
        return QObject::tr("Current density - induced transform");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
        return QObject::tr("Current density - induced velocity - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
        return QObject::tr("Current density - induced velocity - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
        return QObject::tr("Current density - induced velocity");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES:
        return QObject::tr("Power losses");
    case PHYSICFIELDVARIABLE_MAGNETIC_LORENTZ_FORCE:
        return QObject::tr("Lorentz force");
    case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
        return QObject::tr("Energy density");
    case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
        return QObject::tr("Permeability");
    case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
        return QObject::tr("Conductivity");
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
        return QObject::tr("Remanent flux density");
    case PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY:
        return QObject::tr("Velocity");

    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        return QObject::tr("Scalar potential");
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
        return QObject::tr("Electic field");
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        return QObject::tr("Current density");
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        return QObject::tr("Power losses");
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        return QObject::tr("Conductivity");

    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        return QObject::tr("Temperature");
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        return QObject::tr("Temperature gradient");
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        return QObject::tr("Heat flux");
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        return QObject::tr("Conductivity");

    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return QObject::tr("Von Mises stress");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
        return QObject::tr("");
    case PHYSICFIELDVARIABLE_GENERAL_GRADIENT:
        return QObject::tr("");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("V");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        return QObject::tr("V/m");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        return QObject::tr("C/m2");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        return QObject::tr("J/m3");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        return QObject::tr("-");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
        return QObject::tr("Wb/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:
        return QObject::tr("Wb/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
        return QObject::tr("Wb/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES:
        return QObject::tr("W/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_LORENTZ_FORCE:
        return QObject::tr("N/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
        return QObject::tr("J/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
        return QObject::tr("-");
    case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
        return QObject::tr("S/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY:
        return QObject::tr("m/s");
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        return QObject::tr("V");
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
        return QObject::tr("V/m");
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        return QObject::tr("W/m3");
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        return QObject::tr("S/m");
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        return QObject::tr("deg.");
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        return QObject::tr("K/m");
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        return QObject::tr("W/m2");
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        return QObject::tr("W/m.K");
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return QObject::tr("Pa");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldString(PhysicField physicField)
{
    switch (physicField)
    {
    case PHYSICFIELD_GENERAL:
        return QObject::tr("General");
    case PHYSICFIELD_ELECTROSTATIC:
        return QObject::tr("Electrostatic");
    case PHYSICFIELD_MAGNETIC:
        return QObject::tr("Magnetic");
    case PHYSICFIELD_CURRENT:
        return QObject::tr("Current field");
    case PHYSICFIELD_HEAT:
        return QObject::tr("Heat transfer");
    case PHYSICFIELD_ELASTICITY:
        return QObject::tr("Elasticity");
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. physicFieldString(PhysicField physicField)" << endl;
        throw;
    }
}

QString analysisTypeString(AnalysisType analysisType)
{
    switch (analysisType)
    {
    case ANALYSISTYPE_STEADYSTATE:
        return QObject::tr("Steady state");
    case ANALYSISTYPE_TRANSIENT:
        return QObject::tr("Transient");
    case ANALYSISTYPE_HARMONIC:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

QString physicFieldBCString(PhysicFieldBC physicFieldBC)
{
    switch (physicFieldBC)
    {
    case PHYSICFIELDBC_NONE:
        return QObject::tr("none");
    case PHYSICFIELDBC_GENERAL_VALUE:
        return QObject::tr("Value");
    case PHYSICFIELDBC_GENERAL_DERIVATIVE:
        return QObject::tr("Derivative");
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("Fixed voltage");
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        return QObject::tr("Surface charge density");
    case PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL:
        return QObject::tr("Vector potential");
    case PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT:
        return QObject::tr("Surface current density");
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return QObject::tr("Temperature");
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return QObject::tr("Heat flux");
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return QObject::tr("Potential");
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return QObject::tr("Inward current flow");
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return QObject::tr("Fixed");
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return QObject::tr("Free");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldBC).toStdString() + "' is not implemented. physicFieldBCString(PhysicFieldBC physicFieldBC)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PHYSICFIELDVARIABLECOMP_SCALAR:
        return QObject::tr("Scalar");
    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
        return QObject::tr("Magnitude");
    case PHYSICFIELDVARIABLECOMP_X:
        return Util::scene()->problemInfo()->labelX();
    case PHYSICFIELDVARIABLECOMP_Y:
        return Util::scene()->problemInfo()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString problemTypeString(ProblemType problemType)
{
    return ((problemType == PROBLEMTYPE_PLANAR) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    switch (adaptivityType)
    {
    case ADAPTIVITYTYPE_NONE:
        return QObject::tr("Disabled");
    case ADAPTIVITYTYPE_H:
        return QObject::tr("h-adaptivity");
    case ADAPTIVITYTYPE_P:
        return QObject::tr("p-adaptivity");
    case ADAPTIVITYTYPE_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

bool Value::evaluate(bool quiet)
{
    ExpressionResult expressionResult = runPythonExpression(text);
    if (expressionResult.error.isEmpty())
    {
        number = expressionResult.value;
    }
    else
    {
        if (!quiet)
            QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error"), expressionResult.error);
    }

    return expressionResult.error.isEmpty();
};

void enableLogFile(bool enable)
{
    logFile = enable;
}

void setGUIStyle(const QString &styleName)
{
    QStyle *style = QStyleFactory::create(styleName);

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware())
    {
        QApplication::setPalette(QApplication::palette());
    }
}

void setLanguage(const QString &locale)
{
    // non latin-1 chars
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator *translator = new QTranslator();
    translator->load(datadir() + "/lang/" + locale + ".qm");
    QApplication::installTranslator(translator);
}

QStringList availableLanguages()
{
    QDir dir;
    dir.setPath(datadir() + "/lang");

    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    return list;
}

QIcon icon(const QString &name)
{
    QString fileName;

#ifdef Q_WS_WIN
    if (QFile::exists(":/images/" + name + "-windows.png")) return QIcon(":/images/" + name + "-windows.png");
#endif

#ifdef Q_WS_X11
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    return QIcon::fromTheme(name, QIcon(":images/" + name + ".png"));
#endif

    QDir dir;

    QString style = "";
    QStringList styles = QStyleFactory::keys();

    // kde 3
    if (getenv("KDE_FULL_SESSION") != NULL)
    {}
    // kde 4
    if (getenv("KDE_SESSION_VERSION") != NULL)
    {
        // oxygen
        fileName = "/usr/share/icons/oxygen/22x22/actions/" + name;
        if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
    // gtk+
    if (style == "")
    {
        // humanity (disabled - corrupted svg reader - Qt 4.6 has new method QIcon::fromTheme)
        // fileName = "/usr/share/icons/Humanity/actions/24/" + name;
        // if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        // if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
#endif

    if (QFile::exists(":images/" + name + ".svg")) return QIcon(":images/" + name + ".svg");
    if (QFile::exists(":images/" + name + ".png")) return QIcon(":images/" + name + ".png");

    return QIcon();
}

QString datadir()
{
    // actual data dir
    QDir dirData;
    dirData.setPath(QDir::current().absolutePath() + "/data");
    if (dirData.exists())
        return QDir::current().absolutePath();

    // linux
    dirData.setPath(QApplication::applicationDirPath() + "/../share/agros2d/data");
    if (dirData.exists())
        return QApplication::applicationDirPath() + "/../share/agros2d";

    // windows
    dirData.setPath(QApplication::applicationDirPath() + "/data");
    if (dirData.exists())
        return QApplication::applicationDirPath() + "/data";

    qCritical() << "Datadir not found.";
    exit(1);
}

QString tempProblemDir()
{
    QDir(QDir::temp().absolutePath()).mkpath("agros2d/" + QString::number(QApplication::applicationPid()));

    return QString("%1/agros2d/%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QString tempProblemFileName()
{
    return tempProblemDir() + "/temp";
}

QTime milliSecondsToTime(int ms)
{
    // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floor(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floor(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floor(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floor(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}

bool removeDirectory(const QDir &dir)
{
    bool error = false;

    if (dir.exists())
    {
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                error = removeDirectory(QDir(path));
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    error = true;
                    break;
                }
            }
        }
        if (!dir.rmdir(dir.absolutePath()))
            error = true;
    }

    return error;
}

void msleep(unsigned long msecs)
{
    QWaitCondition w;
    QMutex sleepMutex;
    sleepMutex.lock();
    w.wait(&sleepMutex, msecs);
    sleepMutex.unlock();
}

void log(const QString &message)
{
    if (logFile)
    {
        QFile file(QApplication::applicationDirPath() + "/log.txt");

        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream outFile(&file);
            outFile << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") << ": " << message << endl;

            file.close();
        }
    }
}

QString readFileContent(const QString &fileName)
{
    QString content;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContent(const QString &fileName, QString *content)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << *content;

        file.waitForBytesWritten(0);
        file.close();
    }
}

QByteArray readFileContentByteArray(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContentByteArray(const QString &fileName, QByteArray content)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(content);

        file.waitForBytesWritten(0);
        file.close();
    }
}

CheckVersion *checkVersion = NULL;
void checkForNewVersion()
{
    // download version
    QUrl url("http://hpfem.org/agros2d/download/version.xml");
    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url);

    checkVersion->run();
}

CheckVersion::CheckVersion(QUrl url) : QObject()
{
    m_url = url;

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    delete m_manager;
}

void CheckVersion::run()
{
    m_manager->get(QNetworkRequest(m_url));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    qDebug() << "downloadFinished";
    qDebug() << networkReply->errorString();
    qDebug() << networkReply->url();

    if (networkReply->errorString().isEmpty())
    {
        QDomDocument doc;
        doc.setContent(networkReply->readAll());

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problems
        QDomNode eleRoot = eleDoc.elementsByTagName("agros2d").at(0);
        QDomNode eleVersion = eleRoot.toElement().elementsByTagName("version").at(0);

        int beta = eleVersion.toElement().attribute("beta").toInt();
        int major = eleVersion.toElement().attribute("major").toInt();
        int minor = eleVersion.toElement().attribute("minor").toInt();
        int sub = eleVersion.toElement().attribute("sub").toInt();
        int git = eleVersion.toElement().attribute("git").toInt();
        int year = eleVersion.toElement().attribute("year").toInt();
        int month = eleVersion.toElement().attribute("month").toInt();
        int day = eleVersion.toElement().attribute("day").toInt();

        QDomNode eleUrl = eleRoot.toElement().elementsByTagName("url").at(0);

        QString downloadUrl = eleUrl.toElement().text();

        QMessageBox::information(QApplication::activeWindow(), "", QString::number(git));
    }
}
