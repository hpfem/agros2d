import test_current_axisymmetric

test_current_axisymmetric.current_axisymmetric()
"""
# electrostatic
execfile("test_electrostatic_planar.py"); 
execfile("test_electrostatic_axisymmetric.py");

# magnetostatic
execfile("test_magnetostatic_planar.py");
execfile("test_magnetostatic_axisymmetric.py");

# harmonic magnetic
execfile("test_harmonicmagnetic_planar.py"); 
execfile("test_harmonicmagnetic_axisymmetric.py"); 

# current field
# execfile("test_current_planar.py"); # ERROR: current
execfile("test_current_axisymmetric.py");

# heat transfer
execfile("test_heat_transfer_planar.py");
execfile("test_heat_transfer_axisymmetric.py");
"""