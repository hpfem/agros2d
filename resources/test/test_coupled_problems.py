from test import Test

test_coupled = Test(error_file='test_coupled_problems.err')

# coupled problems
test_coupled.add("coupled_problems/test_coupled_cf_1_planar.py")
test_coupled.add("coupled_problems/test_coupled_cf_2_axisymmetric.py")
test_coupled.add("coupled_problems/test_coupled_cf_3_axisymmetric_nonlin.py")
test_coupled.add("coupled_problems/test_coupled_cf_4_transient_planar.py") 

# contains sets of test_coupleds obtained by varying previously present coupling test_coupleds
# various combinations of hard/weak and linear/nonlinear are test_coupleded
# in some couplings, the forms are the same for one of fields being steadystate or transient. 
# in our implementation, however, two sets of forms are present (two identical copies). 
# Those forms are also test_coupleded by changing steadystate problem artificaly to transient and 
# setting coefficients of the time derivative term to zero, thus obtaining the same results

# based on test_coupled_coupling_1_planar.py
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_lin_heat_steady_lin_elast_lin_weak_planar.py") 
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_lin_heat_trans_lin_elast_lin_weak_planar.py") 
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_lin_weak_with_heat_steady_lin_hard_with_elast_lin_planar.py")
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_planar.py")
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_planar.py")
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_nl_heat_steady_nl_elast_nl_hard_planar.py") 
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_nl_heat_steady_nl_elast_nl_weak_planar.py") 
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_nl_heat_trans_nl_elast_nl_hard_planar.py") 
test_coupled.add("coupled_problems/test_coupled_coup1_curr_steady_nl_heat_trans_nl_elast_nl_weak_planar.py") 

# based on test_coupled_coupling_2_axisymmetric.py
test_coupled.add("coupled_problems/test_coupled_coup2_mag_harm_lin_heat_steady_lin_weak_axisym.py") 
test_coupled.add("coupled_problems/test_coupled_coup2_mag_harm_lin_heat_trans_lin_weak_axisym.py") 
test_coupled.add("coupled_problems/test_coupled_coup2_mag_harm_nl_heat_steady_nl_hard_axisym.py") 
test_coupled.add("coupled_problems/test_coupled_coup2_mag_harm_nl_heat_steady_nl_weak_axisym.py") 
test_coupled.add("coupled_problems/test_coupled_coup2_mag_harm_nl_heat_trans_nl_hard_axisym.py") 
test_coupled.add("coupled_problems/test_coupled_coup2_mag_harm_nl_heat_trans_nl_weak_axisym.py") 

# based on test_coupled_coupling_3_axisymmetric_nonlin.py
test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_lin_heat_steady_nl_elast_lin_axisym_weak.py")
test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_lin_heat_trans_nl_elast_lin_axisym_weak.py")
test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_axisym.py")
#test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_axisym.py");
#test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_hard.py")
test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_weak.py")
#the following test_coupled works, even though it is one-step transient version of the steady state that does not
test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_hard.py")
test_coupled.add("coupled_problems/test_coupled_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_weak.py")

# based on test_coupled_coupling_4_transient_planar.py
test_coupled.add("coupled_problems/test_coupled_coup4_curr_steady_lin_heat_trans_lin_planar_weak.py")
test_coupled.add("coupled_problems/test_coupled_coup4_curr_steady_nl_heat_trans_nl_planar_hard.py")
test_coupled.add("coupled_problems/test_coupled_coup4_curr_steady_nl_heat_trans_nl_planar_weak.py") 

test_coupled.run()