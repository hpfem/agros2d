from test import Test

test_complete = Test(error_file='test_complete.err')

# fields
# acoustic
test_complete.add("fields/test_acoustic_planar.py") 
test_complete.add("fields/test_acoustic_axisymmetric.py")
test_complete.add("fields/test_acoustic_transient_planar.py") 
test_complete.add("fields/test_acoustic_transient_axisymmetric.py") 

# electrostatic
test_complete.add("fields/test_electrostatic_planar.py") 
test_complete.add("fields/test_electrostatic_axisymmetric.py")

# current field
test_complete.add("fields/test_current_planar.py")
test_complete.add("fields/test_current_axisymmetric.py")

# magnetic field
test_complete.add("fields/test_magnetic_steady_planar.py")
test_complete.add("fields/test_magnetic_steady_axisymmetric.py")
test_complete.add("fields/test_magnetic_steady_planar_nonlin.py")
test_complete.add("fields/test_magnetic_transient_planar.py")
test_complete.add("fields/test_magnetic_transient_axisymmetric.py")

# harmonic magnetic field
test_complete.add("fields/test_magnetic_harmonic_planar.py") 
test_complete.add("fields/test_magnetic_harmonic_axisymmetric.py")

# heat transfer
test_complete.add("fields/test_heat_transfer_steady_planar.py")
test_complete.add("fields/test_heat_transfer_steady_planar_nonlin.py")
test_complete.add("fields/test_heat_transfer_steady_axisymmetric.py")
test_complete.add("fields/test_heat_transfer_transient_axisymmetric_benchmark.py") # benchmark
test_complete.add("fields/test_heat_transfer_transient_axisymmetric.py") # very slow

# structural mechanics
test_complete.add("fields/test_elasticity_planar.py")
test_complete.add("fields/test_elasticity_axisymmetric.py")

# electromagnetic wave - TE
test_complete.add("fields/test_rf_te_planar.py")
test_complete.add("fields/test_rf_te_axisymmetric.py")

# electromagnetic wave - TM
test_complete.add("fields/test_rf_tm_planar.py")
test_complete.add("fields/test_rf_tm_axisymmetric.py")

# incompressible flow
test_complete.add("fields/test_flow_steady_planar_nonlin.py") 
test_complete.add("fields/test_flow_steady_axisymmetric_nonlin.py") 

# coupled problems
test_complete.add("coupled_problems/test_cf_1_planar.py")
test_complete.add("coupled_problems/test_cf_2_axisymmetric.py")
test_complete.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")
test_complete.add("coupled_problems/test_cf_4_transient_planar.py") 

# contains sets of tests obtained by varying previously present coupling tests
# various combinations of hard/weak and linear/nonlinear are tested
# in some couplings, the forms are the same for one of fields being steadystate or transient. 
# in our implementation, however, two sets of forms are present (two identical copies). 
# Those forms are also tested by changing steadystate problem artificaly to transient and 
# setting coefficients of the time derivative term to zero, thus obtaining the same results

# based on test_coupling_1_planar.py
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_heat_steady_lin_elast_lin_weak_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_heat_trans_lin_elast_lin_weak_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_weak_with_heat_steady_lin_hard_with_elast_lin_planar.py")
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_planar.py")
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_planar.py")
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_hard_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_weak_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_hard_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_weak_planar.py") 

# based on test_coupling_2_axisymmetric.py
test_complete.add("coupled_problems/test_coup2_mag_harm_lin_heat_steady_lin_weak_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_lin_heat_trans_lin_weak_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_steady_nl_hard_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_steady_nl_weak_axisym.py")
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_trans_nl_hard_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_trans_nl_weak_axisym.py") 

# based on test_coupling_3_axisymmetric_nonlin.py
test_complete.add("coupled_problems/test_coup3_curr_steady_lin_heat_steady_nl_elast_lin_axisym_weak.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_lin_heat_trans_nl_elast_lin_axisym_weak.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_axisym.py")
#test_complete.add("coupled_problems/test_coup3_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_axisym.py");
#test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_hard.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_weak.py")
#the following test works, even though it is one-step transient version of the steady state that does not
test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_hard.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_weak.py")

# based on test_coupling_4_transient_planar.py
test_complete.add("coupled_problems/test_coup4_curr_steady_lin_heat_trans_lin_planar_weak.py")
test_complete.add("coupled_problems/test_coup4_curr_steady_nl_heat_trans_nl_planar_hard.py")
test_complete.add("coupled_problems/test_coup4_curr_steady_nl_heat_trans_nl_planar_weak.py") 

# particle tracing
test_complete.add("particle_tracing/test_particle_tracing_planar.py")
test_complete.add("particle_tracing/test_particle_tracing_axisymmetric.py")

# adaptivity
test_complete.add("adaptivity/test_adaptivity_acoustic.py")
test_complete.add("adaptivity/test_adaptivity_electrostatic.py")

# scripting
test_complete.add("test_script.py")

test_complete.run()