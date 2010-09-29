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
    analysisTypeList.insert(AnalysisType_Undefined, "");
    analysisTypeList.insert(AnalysisType_SteadyState, "steadystate");
    analysisTypeList.insert(AnalysisType_Transient, "transient");
    analysisTypeList.insert(AnalysisType_Harmonic, "harmonic");

    // PHYSICFIELD
    physicFieldList.insert(PhysicField_Undefined, "");
    physicFieldList.insert(PhysicField_General, "general");
    physicFieldList.insert(PhysicField_Electrostatic, "electrostatic");
    physicFieldList.insert(PhysicField_Current, "current");
    physicFieldList.insert(PhysicField_Heat, "heat");
    physicFieldList.insert(PhysicField_Elasticity, "elasticity");
    physicFieldList.insert(PhysicField_Magnetic, "magnetic");
    physicFieldList.insert(PhysicField_Flow, "flow");

    // PHYSICFIELDVARIABLE
    physicFieldVariableList.insert(PhysicFieldVariable_Undefined, "");

    physicFieldVariableList.insert(PhysicFieldVariable_Variable, "general_variable");
    physicFieldVariableList.insert(PhysicFieldVariable_General_Gradient, "general_gradient");
    physicFieldVariableList.insert(PhysicFieldVariable_General_Constant, "general_constant");

    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_Potential, "electrostatic_potential");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_ElectricField, "electrostatic_electric_field");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_Displacement, "electrostatic_displacement");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_EnergyDensity, "electrostatic_energy_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_Permittivity, "electrostatic_permittivity");

    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_VectorPotentialReal, "magnetic_vector_potential_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_VectorPotentialImag, "magnetic_vector_potential_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_VectorPotential, "magnetic_vector_potential");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_FluxDensityReal, "magnetic_flux_density_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_FluxDensityImag, "magnetic_flux_density_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_FluxDensity, "magnetic_flux_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_MagneticFieldReal, "magnetic_magnetic_field_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_MagneticFieldImag, "magnetic_magnetic_field_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_MagneticField, "magnetic_magnetic_field");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityReal, "magnetic_current_density_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityImag, "magnetic_current_density_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensity, "magnetic_current_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal, "magnetic_current_density_induced_transform_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag, "magnetic_current_density_induced_transform_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform, "magnetic_current_density_induced_transform");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal, "magnetic_current_density_induced_velocity_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag, "magnetic_current_density_induced_velocity_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity, "magnetic_current_density_induced_velocity");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityTotalReal, "magnetic_current_density_total_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityTotalImag, "magnetic_current_density_total_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityTotal, "magnetic_current_density_total");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_PowerLosses, "magnetic_power_losses");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_LorentzForce, "magnetic_lorentz_force");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_EnergyDensity, "magnetic_energy_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Permeability, "magnetic_permeability");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Conductivity, "magnetic_conductivity");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Remanence, "magnetic_remanence");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Velocity, "magnetic_velocity");

    physicFieldVariableList.insert(PhysicFieldVariable_Current_Potential, "current_potential");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_ElectricField, "current_electric_field");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_CurrentDensity, "current_current_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_Losses, "current_power_losses");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_Conductivity, "current_conductivity");

    physicFieldVariableList.insert(PhysicFieldVariable_Heat_Temperature, "heat_temperature");
    physicFieldVariableList.insert(PhysicFieldVariable_Heat_TemperatureGradient, "heat_temperature_gradient");
    physicFieldVariableList.insert(PhysicFieldVariable_Heat_Flux, "heat_heat_flux");
    physicFieldVariableList.insert(PhysicFieldVariable_Heat_Conductivity, "heat_conductivity");

    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_VonMisesStress, "elasticity_von_mises_stress");

    physicFieldVariableList.insert(PhysicFieldVariable_Flow_Velocity, "flow_velocity");
    physicFieldVariableList.insert(PhysicFieldVariable_Flow_VelocityX, "flow_velocity_x");
    physicFieldVariableList.insert(PhysicFieldVariable_Flow_VelocityY, "flow_velocity_y");
    physicFieldVariableList.insert(PhysicFieldVariable_Flow_Pressure, "flow_velocity");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Undefined, "");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Scalar, "scalar");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Magnitude, "magnitude");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_X, "x");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Y, "y");

    // PHYSICFIELDBC
    physicFieldBCList.insert(PhysicFieldBC_Undefined, "");
    physicFieldBCList.insert(PhysicFieldBC_None, "none");
    physicFieldBCList.insert(PhysicFieldBC_General_Value, "general_value");
    physicFieldBCList.insert(PhysicFieldBC_General_Derivative, "general_derivative");
    physicFieldBCList.insert(PhysicFieldBC_Electrostatic_Potential, "electrostatic_potential");
    physicFieldBCList.insert(PhysicFieldBC_Electrostatic_SurfaceCharge, "electrostatic_surface_charge_density");
    physicFieldBCList.insert(PhysicFieldBC_Magnetic_VectorPotential, "magnetic_vector_potential");
    physicFieldBCList.insert(PhysicFieldBC_Magnetic_SurfaceCurrent, "magnetic_surface_current_density");
    physicFieldBCList.insert(PhysicFieldBC_Heat_Temperature, "heat_temperature");
    physicFieldBCList.insert(PhysicFieldBC_Heat_Flux, "heat_heat_flux");
    physicFieldBCList.insert(PhysicFieldBC_Current_Potential, "current_potential");
    physicFieldBCList.insert(PhysicFieldBC_Current_InwardCurrentFlow, "current_inward_current_flow");
    physicFieldBCList.insert(PhysicFieldBC_Elasticity_Fixed, "elasticity_fixed");
    physicFieldBCList.insert(PhysicFieldBC_Elasticity_Free, "elasticity_free");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Velocity, "flow_velocity");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Pressure, "flow_pressure");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Outlet, "flow_outlet");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Wall, "flow_wall");

    // SCENEVIEW_POSTPROCESSOR_SHOW
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_Undefined, "");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_None, "none");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_ScalarView, "scalar");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_ScalarView3D, "scalar3d");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_Order, "order");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(AdaptivityType_Undefined, "");
    adaptivityTypeList.insert(AdaptivityType_None, "disabled");
    adaptivityTypeList.insert(AdaptivityType_H, "h-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_P, "p-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_HP, "hp-adaptivity");
}

QString physicFieldVariableString(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        return QObject::tr("Variable");
    case PhysicFieldVariable_General_Gradient:
        return QObject::tr("Gradient");
    case PhysicFieldVariable_General_Constant:
        return QObject::tr("Constant");

    case PhysicFieldVariable_Electrostatic_Potential:
        return QObject::tr("Scalar potential");
    case PhysicFieldVariable_Electrostatic_ElectricField:
        return QObject::tr("Electric field");
    case PhysicFieldVariable_Electrostatic_Displacement:
        return QObject::tr("Displacement");
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
        return QObject::tr("Energy density");
    case PhysicFieldVariable_Electrostatic_Permittivity:
        return QObject::tr("Permittivity");

    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return QObject::tr("Vector potential - real");
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return QObject::tr("Vector potential - imag");
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return QObject::tr("Vector potential");
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        return QObject::tr("Flux density - real");
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        return QObject::tr("Flux density - imag");
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return QObject::tr("Flux density");
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        return QObject::tr("Magnetic field - real");
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        return QObject::tr("Magnetic field - imag");
    case PhysicFieldVariable_Magnetic_MagneticField:
        return QObject::tr("Magnetic field");
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return QObject::tr("Current density - external - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return QObject::tr("Current density - external - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return QObject::tr("Current density - external");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return QObject::tr("Current density - total - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return QObject::tr("Current density - total - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return QObject::tr("Current density - total");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return QObject::tr("Current density - induced transform - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return QObject::tr("Current density - induced transform - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return QObject::tr("Current density - induced transform");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return QObject::tr("Current density - induced velocity - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return QObject::tr("Current density - induced velocity - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return QObject::tr("Current density - induced velocity");
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return QObject::tr("Power losses");
    case PhysicFieldVariable_Magnetic_LorentzForce:
        return QObject::tr("Lorentz force");
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return QObject::tr("Energy density");
    case PhysicFieldVariable_Magnetic_Permeability:
        return QObject::tr("Permeability");
    case PhysicFieldVariable_Magnetic_Conductivity:
        return QObject::tr("Conductivity");
    case PhysicFieldVariable_Magnetic_Remanence:
        return QObject::tr("Remanent flux density");
    case PhysicFieldVariable_Magnetic_Velocity:
        return QObject::tr("Velocity");

    case PhysicFieldVariable_Current_Potential:
        return QObject::tr("Scalar potential");
    case PhysicFieldVariable_Current_ElectricField:
        return QObject::tr("Electric field");
    case PhysicFieldVariable_Current_CurrentDensity:
        return QObject::tr("Current density");
    case PhysicFieldVariable_Current_Losses:
        return QObject::tr("Power losses");
    case PhysicFieldVariable_Current_Conductivity:
        return QObject::tr("Conductivity");

    case PhysicFieldVariable_Heat_Temperature:
        return QObject::tr("Temperature");
    case PhysicFieldVariable_Heat_TemperatureGradient:
        return QObject::tr("Temperature gradient");
    case PhysicFieldVariable_Heat_Flux:
        return QObject::tr("Heat flux");
    case PhysicFieldVariable_Heat_Conductivity:
        return QObject::tr("Conductivity");

    case PhysicFieldVariable_Elasticity_VonMisesStress:
        return QObject::tr("Von Mises stress");

    case PhysicFieldVariable_Flow_Velocity:
        return QObject::tr("Velocity");
    case PhysicFieldVariable_Flow_VelocityX:
        return QObject::tr("Velocity X");
    case PhysicFieldVariable_Flow_VelocityY:
        return QObject::tr("Velocity Y");
    case PhysicFieldVariable_Flow_Pressure:
        return QObject::tr("Pressure");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldVariableShortcutString(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        return QObject::tr("V");
    case PhysicFieldVariable_General_Gradient:
        return QObject::tr("G");
    case PhysicFieldVariable_General_Constant:
        return QObject::tr("C");

    case PhysicFieldVariable_Electrostatic_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Electrostatic_ElectricField:
        return QObject::tr("E");
    case PhysicFieldVariable_Electrostatic_Displacement:
        return QObject::tr("D");
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
        return QObject::tr("we");
    case PhysicFieldVariable_Electrostatic_Permittivity:
        return QObject::tr("epsr");

    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return QObject::tr("Are");
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return QObject::tr("Aim");
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return QObject::tr("A");
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        return QObject::tr("Bre");
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        return QObject::tr("Bim");
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return QObject::tr("B");
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        return QObject::tr("Hre");
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        return QObject::tr("Him");
    case PhysicFieldVariable_Magnetic_MagneticField:
        return QObject::tr("H");
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return QObject::tr("Jere");
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return QObject::tr("Jeim");
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return QObject::tr("Je");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return QObject::tr("Jtre");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return QObject::tr("Jtim");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return QObject::tr("Jt");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return QObject::tr("Jitre");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return QObject::tr("Jitim");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return QObject::tr("Jit");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return QObject::tr("Jivre");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return QObject::tr("Jivim");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return QObject::tr("Jiv");
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return QObject::tr("pj");
    case PhysicFieldVariable_Magnetic_LorentzForce:
        return QObject::tr("FL");
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return QObject::tr("wm");
    case PhysicFieldVariable_Magnetic_Permeability:
        return QObject::tr("mur");
    case PhysicFieldVariable_Magnetic_Conductivity:
        return QObject::tr("g");
    case PhysicFieldVariable_Magnetic_Remanence:
        return QObject::tr("Br");
    case PhysicFieldVariable_Magnetic_Velocity:
        return QObject::tr("v");

    case PhysicFieldVariable_Current_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Current_ElectricField:
        return QObject::tr("E");
    case PhysicFieldVariable_Current_CurrentDensity:
        return QObject::tr("J");
    case PhysicFieldVariable_Current_Losses:
        return QObject::tr("pj");
    case PhysicFieldVariable_Current_Conductivity:
        return QObject::tr("g");

    case PhysicFieldVariable_Heat_Temperature:
        return QObject::tr("T");
    case PhysicFieldVariable_Heat_TemperatureGradient:
        return QObject::tr("G");
    case PhysicFieldVariable_Heat_Flux:
        return QObject::tr("F");
    case PhysicFieldVariable_Heat_Conductivity:
        return QObject::tr("k");

    case PhysicFieldVariable_Elasticity_VonMisesStress:
        return QObject::tr("E");

    case PhysicFieldVariable_Flow_Velocity:
        return QObject::tr("v");
    case PhysicFieldVariable_Flow_VelocityX:
        return QObject::tr("vx");
    case PhysicFieldVariable_Flow_VelocityY:
        return QObject::tr("vy");
    case PhysicFieldVariable_Flow_Pressure:
        return QObject::tr("p");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableShortcutString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldVariableUnitsString(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        return QObject::tr("");
    case PhysicFieldVariable_General_Gradient:
        return QObject::tr("");
    case PhysicFieldVariable_Electrostatic_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Electrostatic_ElectricField:
        return QObject::tr("V/m");
    case PhysicFieldVariable_Electrostatic_Displacement:
        return QObject::tr("C/m2");
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
        return QObject::tr("J/m3");
    case PhysicFieldVariable_Electrostatic_Permittivity:
        return QObject::tr("-");
    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return QObject::tr("Wb/m");
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return QObject::tr("Wb/m");
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return QObject::tr("Wb/m");
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_MagneticField:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return QObject::tr("W/m3");
    case PhysicFieldVariable_Magnetic_LorentzForce:
        return QObject::tr("N/m3");
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return QObject::tr("J/m3");
    case PhysicFieldVariable_Magnetic_Permeability:
        return QObject::tr("-");
    case PhysicFieldVariable_Magnetic_Conductivity:
        return QObject::tr("S/m");
    case PhysicFieldVariable_Magnetic_Remanence:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_Velocity:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Current_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Current_ElectricField:
        return QObject::tr("V/m");
    case PhysicFieldVariable_Current_CurrentDensity:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Current_Losses:
        return QObject::tr("W/m3");
    case PhysicFieldVariable_Current_Conductivity:
        return QObject::tr("S/m");
    case PhysicFieldVariable_Heat_Temperature:
        return QObject::tr("deg.");
    case PhysicFieldVariable_Heat_TemperatureGradient:
        return QObject::tr("K/m");
    case PhysicFieldVariable_Heat_Flux:
        return QObject::tr("W/m2");
    case PhysicFieldVariable_Heat_Conductivity:
        return QObject::tr("W/m.K");
    case PhysicFieldVariable_Elasticity_VonMisesStress:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Flow_Velocity:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Flow_VelocityX:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Flow_VelocityY:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Flow_Pressure:
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
    case PhysicField_General:
        return QObject::tr("General");
    case PhysicField_Electrostatic:
        return QObject::tr("Electrostatic");
    case PhysicField_Magnetic:
        return QObject::tr("Magnetic");
    case PhysicField_Current:
        return QObject::tr("Current field");
    case PhysicField_Heat:
        return QObject::tr("Heat transfer");
    case PhysicField_Elasticity:
        return QObject::tr("Elasticity");
    case PhysicField_Flow:
        return QObject::tr("Incompressible flow");
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. physicFieldString(PhysicField physicField)" << endl;
        throw;
    }
}

QString analysisTypeString(AnalysisType analysisType)
{
    switch (analysisType)
    {
    case AnalysisType_SteadyState:
        return QObject::tr("Steady state");
    case AnalysisType_Transient:
        return QObject::tr("Transient");
    case AnalysisType_Harmonic:
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
    case PhysicFieldBC_None:
        return QObject::tr("none");
    case PhysicFieldBC_General_Value:
        return QObject::tr("Value");
    case PhysicFieldBC_General_Derivative:
        return QObject::tr("Derivative");
    case PhysicFieldBC_Electrostatic_Potential:
        return QObject::tr("Fixed voltage");
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
        return QObject::tr("Surface charge density");
    case PhysicFieldBC_Magnetic_VectorPotential:
        return QObject::tr("Vector potential");
    case PhysicFieldBC_Magnetic_SurfaceCurrent:
        return QObject::tr("Surface current density");
    case PhysicFieldBC_Heat_Temperature:
        return QObject::tr("Temperature");
    case PhysicFieldBC_Heat_Flux:
        return QObject::tr("Heat flux");
    case PhysicFieldBC_Current_Potential:
        return QObject::tr("Potential");
    case PhysicFieldBC_Current_InwardCurrentFlow:
        return QObject::tr("Inward current flow");
    case PhysicFieldBC_Elasticity_Fixed:
        return QObject::tr("Fixed");
    case PhysicFieldBC_Elasticity_Free:
        return QObject::tr("Free");
    case PhysicFieldBC_Flow_Outlet:
        return QObject::tr("Outlet");
    case PhysicFieldBC_Flow_Wall:
        return QObject::tr("Wall");
    case PhysicFieldBC_Flow_Velocity:
        return QObject::tr("Velocity");
    case PhysicFieldBC_Flow_Pressure:
        return QObject::tr("Pressure");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldBC).toStdString() + "' is not implemented. physicFieldBCString(PhysicFieldBC physicFieldBC)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Scalar:
        return QObject::tr("Scalar");
    case PhysicFieldVariableComp_Magnitude:
        return QObject::tr("Magnitude");
    case PhysicFieldVariableComp_X:
        return Util::scene()->problemInfo()->labelX();
    case PhysicFieldVariableComp_Y:
        return Util::scene()->problemInfo()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString problemTypeString(ProblemType problemType)
{
    return ((problemType == ProblemType_Planar) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    switch (adaptivityType)
    {
    case AdaptivityType_None:
        return QObject::tr("Disabled");
    case AdaptivityType_H:
        return QObject::tr("h-adaptivity");
    case AdaptivityType_P:
        return QObject::tr("p-adaptivity");
    case AdaptivityType_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

void fillComboBoxPhysicField(QComboBox *cmbPhysicField)
{
    cmbPhysicField->clear();
    cmbPhysicField->addItem(physicFieldString(PhysicField_General), PhysicField_General);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Electrostatic), PhysicField_Electrostatic);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Magnetic), PhysicField_Magnetic);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Current), PhysicField_Current);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Heat), PhysicField_Heat);
#ifdef BETA
    cmbPhysicField->addItem(physicFieldString(PhysicField_Elasticity), PhysicField_Elasticity);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Flow), PhysicField_Flow);
#endif

    // default physic field
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(Util::config()->defaultPhysicField));
    if (cmbPhysicField->currentIndex() == -1)
        cmbPhysicField->setCurrentIndex(0);
}

bool Value::evaluate(bool quiet)
{
    ExpressionResult expressionResult;
    expressionResult = runPythonExpression(text);
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
    // windows and local installation
    if (QFile::exists(QApplication::applicationDirPath() + "/functions.py"))
        return QApplication::applicationDirPath();

    // linux
    if (QFile::exists(QApplication::applicationDirPath() + "/../share/agros2d/functions.py"))
        return QApplication::applicationDirPath() + "/../share/agros2d";

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

QTime milisecondsToTime(int ms)
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

void logOutput(QtMsgType type, const char *msg)
{
    QString msgType = "";

    switch (type) {
    case QtDebugMsg:
        msgType = "Debug";
        break;
    case QtWarningMsg:
        msgType = "Warning";
        break;
    case QtCriticalMsg:
        msgType = "Critical";
        break;
    case QtFatalMsg:
        msgType = "Fatal";
        break;
    }

    QString str = QString("%1 %2: %3").
                  arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")).
                  arg(msgType).
                  arg(msg);

    // string
    fprintf(stderr, "%s\n", str.toStdString().c_str());

    if (Util::config()->enabledApplicationLog)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QDir("/").mkpath(location);

        QFile file(location + "/app.log");

        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream outFile(&file);
            outFile << str << endl;

            file.close();
        }
    }

    if (type == QtFatalMsg)
        abort();
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
void checkForNewVersion(bool quiet)
{
    // download version
    QUrl url("http://hpfem.org/agros2d/download/version.xml");
    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url);

    checkVersion->run(quiet);
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

void CheckVersion::run(bool quiet)
{
    m_quiet = quiet;
    m_networkReply = m_manager->get(QNetworkRequest(m_url));

    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(showProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    QString text = networkReply->readAll();

    if (!text.isEmpty())
    {
        QDomDocument doc;
        doc.setContent(text);

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problems
        QDomNode eleVersion = eleDoc.toElement().elementsByTagName("version").at(0);

        int beta = eleVersion.toElement().attribute("beta").toInt() == 1;
        int major = eleVersion.toElement().attribute("major").toInt();
        int minor = eleVersion.toElement().attribute("minor").toInt();
        int sub = eleVersion.toElement().attribute("sub").toInt();
        int git = eleVersion.toElement().attribute("git").toInt();
        int year = eleVersion.toElement().attribute("year").toInt();
        int month = eleVersion.toElement().attribute("month").toInt();
        int day = eleVersion.toElement().attribute("day").toInt();

        QDomNode eleUrl = eleDoc.toElement().elementsByTagName("url").at(0);

        if (!m_quiet && git == 0)
        {
            QMessageBox::critical(QApplication::activeWindow(), tr("New version"), tr("File is corrupted or network is disconnected."));
            return;
        }

        QString downloadUrl = eleUrl.toElement().text();
        if (git > VERSION_GIT)
        {
            QString str(tr("<b>New version available.</b><br/><br/>"
                           "Actual version: %1<br/>"
                           "New version: %2<br/><br/>"
                           "URL: <a href=\"%3\">%3</a>").
                        arg(QApplication::applicationVersion()).
                        arg(versionString(major, minor, sub, git, year, month, day, beta)).
                        arg(downloadUrl));

            QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QMessageBox::information(QApplication::activeWindow(), tr("New version"), tr("You are using actual version."));
        }
    }
}

void CheckVersion::showProgress(qint64 dl, qint64 all)
{
    // qDebug() << QString("\rDownloaded %1 bytes of %2).").arg(dl).arg(all);
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    qDebug() << "An error ocurred (code #" << error << ").";
}
