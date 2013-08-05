import agros2d as a2d
import time as timer
start = timer.time()

start_time, start_memory = a2d.memory_usage()

# fields
print("Physical fields")

# electrostatic
execfile("fields/test_electrostatic_planar.py") 
execfile("fields/test_electrostatic_axisymmetric.py")

# current field
execfile("fields/test_current_planar.py")
execfile("fields/test_current_axisymmetric.py")

# magnetic field
execfile("fields/test_magnetic_steady_planar.py")
execfile("fields/test_magnetic_steady_axisymmetric.py")

# harmonic magnetic field
execfile("fields/test_magnetic_harmonic_planar.py") 
execfile("fields/test_magnetic_harmonic_axisymmetric.py")

# heat transfer
execfile("fields/test_heat_transfer_steady_planar.py")
execfile("fields/test_heat_transfer_steady_planar_nonlin.py")
execfile("fields/test_heat_transfer_steady_axisymmetric.py")
execfile("fields/test_heat_transfer_transient_axisymmetric_benchmark.py") # benchmark

# structural mechanics
execfile("fields/test_elasticity_planar.py")
execfile("fields/test_elasticity_axisymmetric.py")

# acoustic
execfile("fields/test_acoustic_planar.py") 
execfile("fields/test_acoustic_axisymmetric.py")

# electromagnetic wave - TE
execfile("fields/test_rf_te_planar.py")
execfile("fields/test_rf_te_axisymmetric.py")

# electromagnetic wave - TM
execfile("fields/test_rf_tm_planar.py")
execfile("fields/test_rf_tm_axisymmetric.py")

# incompressible flow
execfile("fields/test_flow_steady_planar_nonlin.py") 
execfile("fields/test_flow_steady_axisymmetric_nonlin.py") 



# coupled problems
print("Coupled problems")

execfile("coupled_problems/test_cf_1_planar.py")
execfile("coupled_problems/test_cf_2_axisymmetric.py")
execfile("coupled_problems/test_cf_3_axisymmetric_nonlin.py")
execfile("coupled_problems/test_cf_4_transient_planar.py") 



# particle tracing
print("Particle tracing")
execfile("particle_tracing/test_particle_tracing_planar.py")
execfile("particle_tracing/test_particle_tracing_axisymmetric.py")



# adaptivity
print("Adaptivity")
execfile("adaptivity/test_adaptivity_acoustic.py")
execfile("adaptivity/test_adaptivity_electrostatic.py")



# scripting
print("Python script")
execfile("test_script.py")



# internal
print("Internal")
execfile("internal/test_matrix_solvers.py")



stop = timer.time()
print("Total time: " + str(stop - start)) + " s"
stop_time, stop_memory = a2d.memory_usage()
print("Used memory: " + str(stop_memory[-1] - start_memory[-1])) + " MB"