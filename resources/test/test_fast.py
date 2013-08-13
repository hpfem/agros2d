from test import Test

test_fast = Test(error_file='test_fast.err')

# electrostatic
test_fast.add("fields/test_electrostatic_planar.py")
test_fast.add("fields/test_electrostatic_axisymmetric.py")

# current field
test_fast.add("fields/test_current_planar.py")
test_fast.add("fields/test_current_axisymmetric.py")

# magnetic field
test_fast.add("fields/test_magnetic_steady_planar.py")
test_fast.add("fields/test_magnetic_steady_axisymmetric.py")

# harmonic magnetic field
test_fast.add("fields/test_magnetic_harmonic_planar.py") 
test_fast.add("fields/test_magnetic_harmonic_axisymmetric.py")

# heat transfer
test_fast.add("fields/test_heat_transfer_steady_planar.py")
test_fast.add("fields/test_heat_transfer_steady_planar_nonlin.py")
test_fast.add("fields/test_heat_transfer_steady_axisymmetric.py")
test_fast.add("fields/test_heat_transfer_transient_axisymmetric_benchmark.py") # benchmark

# structural mechanics
test_fast.add("fields/test_elasticity_planar.py")
test_fast.add("fields/test_elasticity_axisymmetric.py")

# acoustic
test_fast.add("fields/test_acoustic_planar.py") 
test_fast.add("fields/test_acoustic_axisymmetric.py")

# electromagnetic wave - TE
test_fast.add("fields/test_rf_te_planar.py")
test_fast.add("fields/test_rf_te_axisymmetric.py")

# electromagnetic wave - TM
test_fast.add("fields/test_rf_tm_planar.py")
test_fast.add("fields/test_rf_tm_axisymmetric.py")

# incompressible flow
test_fast.add("fields/test_flow_steady_planar_nonlin.py") 
test_fast.add("fields/test_flow_steady_axisymmetric_nonlin.py") 

# coupled problems
test_fast.add("coupled_problems/test_cf_1_planar.py")
test_fast.add("coupled_problems/test_cf_2_axisymmetric.py")
test_fast.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")
test_fast.add("coupled_problems/test_cf_4_transient_planar.py") 

# particle tracing
test_fast.add("particle_tracing/test_particle_tracing_planar.py")
test_fast.add("particle_tracing/test_particle_tracing_axisymmetric.py")

# adaptivity
test_fast.add("adaptivity/test_adaptivity_acoustic.py")
test_fast.add("adaptivity/test_adaptivity_electrostatic.py")

# scripting
test_fast.add("test_script.py")

# internal
test_fast.add("internal/test_matrix_solvers.py")

test_fast.run()