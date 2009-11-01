// electrostatic
include("test_electrostatic_planar.qs"); 
include("test_electrostatic_axisymmetric.qs");

// magnetostatic
include("test_magnetostatic_planar.qs");
include("test_magnetostatic_axisymmetric.qs");

// harmonic magnetic
include("test_harmonicmagnetic_planar.qs"); 
include("test_harmonicmagnetic_axisymmetric.qs"); 

// current field
//// include("test_current_planar.qs"); // ERROR: current
include("test_current_axisymmetric.qs");

// heat transfer
include("test_heat_transfer_planar.qs");
include("test_heat_transfer_axisymmetric.qs");