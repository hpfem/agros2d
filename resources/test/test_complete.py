import time as timer
start = timer.time()

# fields
print("Physical fields")

# acoustic
execfile("fields/test_acoustic_planar.py") 
execfile("fields/test_acoustic_axisymmetric.py")
execfile("fields/test_acoustic_transient_planar.py") 
execfile("fields/test_acoustic_transient_axisymmetric.py") 

# electrostatic
execfile("fields/test_electrostatic_planar.py") 
execfile("fields/test_electrostatic_axisymmetric.py")

# current field
execfile("fields/test_current_planar.py")
execfile("fields/test_current_axisymmetric.py")

# magnetic field
execfile("fields/test_magnetic_steady_planar.py")
execfile("fields/test_magnetic_steady_axisymmetric.py")
execfile("fields/test_magnetic_steady_planar_nonlin.py")
execfile("fields/test_magnetic_transient_planar.py")
execfile("fields/test_magnetic_transient_axisymmetric.py")

# harmonic magnetic field
execfile("fields/test_magnetic_harmonic_planar.py") 
execfile("fields/test_magnetic_harmonic_axisymmetric.py")

# heat transfer
execfile("fields/test_heat_transfer_steady_planar.py")
execfile("fields/test_heat_transfer_steady_planar_nonlin.py")
execfile("fields/test_heat_transfer_steady_axisymmetric.py")
execfile("fields/test_heat_transfer_transient_axisymmetric_benchmark.py") # benchmark
execfile("fields/test_heat_transfer_transient_axisymmetric.py") # very slow

# structural mechanics
execfile("fields/test_elasticity_planar.py")
execfile("fields/test_elasticity_axisymmetric.py")

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

# contains sets of tests obtained by varying previously present coupling tests
# various combinations of hard/weak and linear/nonlinear are tested
# in some couplings, the forms are the same for one of fields being steadystate or transient. 
# in our implementation, however, two sets of forms are present (two identical copies). 
# Those forms are also tested by changing steadystate problem artificaly to transient and 
# setting coefficients of the time derivative term to zero, thus obtaining the same results

# based on test_coupling_1_planar.py
execfile("coupled_problems/test_coup1_curr_steady_lin_heat_steady_lin_elast_lin_weak_planar.py") 
execfile("coupled_problems/test_coup1_curr_steady_lin_heat_trans_lin_elast_lin_weak_planar.py") 
execfile("coupled_problems/test_coup1_curr_steady_lin_weak_with_heat_steady_lin_hard_with_elast_lin_planar.py")
execfile("coupled_problems/test_coup1_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_planar.py")
execfile("coupled_problems/test_coup1_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_planar.py")
execfile("coupled_problems/test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_hard_planar.py") 
execfile("coupled_problems/test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_weak_planar.py") 
execfile("coupled_problems/test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_hard_planar.py") 
execfile("coupled_problems/test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_weak_planar.py") 

# based on test_coupling_2_axisymmetric.py
execfile("coupled_problems/test_coup2_mag_harm_lin_heat_steady_lin_weak_axisym.py") 
execfile("coupled_problems/test_coup2_mag_harm_lin_heat_trans_lin_weak_axisym.py") 
execfile("coupled_problems/test_coup2_mag_harm_nl_heat_steady_nl_hard_axisym.py") 
execfile("coupled_problems/test_coup2_mag_harm_nl_heat_steady_nl_weak_axisym.py")
execfile("coupled_problems/test_coup2_mag_harm_nl_heat_trans_nl_hard_axisym.py") 
execfile("coupled_problems/test_coup2_mag_harm_nl_heat_trans_nl_weak_axisym.py") 

# based on test_coupling_3_axisymmetric_nonlin.py
execfile("coupled_problems/test_coup3_curr_steady_lin_heat_steady_nl_elast_lin_axisym_weak.py")
execfile("coupled_problems/test_coup3_curr_steady_lin_heat_trans_nl_elast_lin_axisym_weak.py")
execfile("coupled_problems/test_coup3_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_axisym.py")
#execfile("coupled_problems/test_coup3_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_axisym.py");
#execfile("coupled_problems/test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_hard.py")
execfile("coupled_problems/test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_weak.py")
#the following test works, even though it is one-step transient version of the steady state that does not
execfile("coupled_problems/test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_hard.py")
execfile("coupled_problems/test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_weak.py")

# based on test_coupling_4_transient_planar.py
execfile("coupled_problems/test_coup4_curr_steady_lin_heat_trans_lin_planar_weak.py")
execfile("coupled_problems/test_coup4_curr_steady_nl_heat_trans_nl_planar_hard.py")
execfile("coupled_problems/test_coup4_curr_steady_nl_heat_trans_nl_planar_weak.py") 



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



stop = timer.time()
print("\nTotal time: " + str(stop - start))