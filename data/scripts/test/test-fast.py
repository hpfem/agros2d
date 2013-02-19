# electrostatic
execfile("test_electrostatic_planar.py") 
execfile("test_electrostatic_axisymmetric.py")

# current field
execfile("test_current_planar.py")
execfile("test_current_axisymmetric.py")

# magnetic field
execfile("test_magnetic_steady_planar.py")
execfile("test_magnetic_steady_axisymmetric.py")
# execfile("test_magnetic_transient_planar.py")

# harmonic magnetic field
execfile("test_magnetic_harmonic_planar.py") 
execfile("test_magnetic_harmonic_axisymmetric.py")

# heat transfer
execfile("test_heat_transfer_steady_planar.py")
execfile("test_heat_transfer_steady_planar_nonlin.py")
execfile("test_heat_transfer_steady_axisymmetric.py")
execfile("test_heat_transfer_transient_axisymmetric_benchmark.py") # benchmark

# structural mechanics
execfile("test_elasticity_planar.py")
execfile("test_elasticity_axisymmetric.py")

# acoustic
execfile("test_acoustic_planar.py") 
execfile("test_acoustic_axisymmetric.py")

# electromagnetic wave - TE
# execfile("test_rf_planar.py")
# execfile("test_rf_axisymmetric.py")

# incompressible flow
execfile("test_flow_steady_planar_nonlin.py") 
execfile("test_flow_steady_axisymmetric_nonlin.py") 

# coupling
execfile("test_coupling_1_planar.py")
execfile("test_coupling_2_axisymmetric.py")
execfile("test_coupling_3_axisymmetric_nonlin.py")
execfile("test_coupling_4_transient_planar.py") 

# particle tracing
execfile("test_particle_tracing_axisymmetric.py")

# scripting
# execfile("test_scripting.py")