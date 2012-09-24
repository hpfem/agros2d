# contains sets of tests obtained by varying previously present coupling tests
# various combinations of hard/weak and linear/nonlinear are tested
# in some couplings, the forms are the same for one of fields being steadystate or transient. 
# in our implementation, however, two sets of forms are present (two identical copies). 
# Those forms are also tested by changing steadystate problem artificaly to transient and 
# setting coefficients of the time derivative term to zero, thus obtaining the same results


# based on test_coupling_1_planar.py
execfile("test_coup1_curr_steady_lin_heat_steady_lin_elast_lin_weak_planar.py") 
execfile("test_coup1_curr_steady_lin_heat_trans_lin_elast_lin_weak_planar.py") 
execfile("test_coup1_curr_steady_lin_weak_with_heat_steady_lin_hard_with_elast_lin_planar.py")
execfile("test_coup1_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_planar.py")
execfile("test_coup1_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_planar.py")
execfile("test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_hard_planar.py") 
execfile("test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_weak_planar.py") 
execfile("test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_hard_planar.py") 
execfile("test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_weak_planar.py") 

# based on test_coupling_2_axisymmetric.py
execfile("test_coup2_mag_harm_lin_heat_steady_lin_weak_axisym.py") 
execfile("test_coup2_mag_harm_lin_heat_trans_lin_weak_axisym.py") 
execfile("test_coup2_mag_harm_nl_heat_steady_nl_hard_axisym.py") 
execfile("test_coup2_mag_harm_nl_heat_steady_nl_weak_axisym.py") 
execfile("test_coup2_mag_harm_nl_heat_trans_nl_hard_axisym.py") 
execfile("test_coup2_mag_harm_nl_heat_trans_nl_weak_axisym.py") 

# based on test_coupling_3_axisymmetric_nonlin.py
execfile("test_coup3_curr_steady_lin_heat_steady_nl_elast_lin_axisym_weak.py")
execfile("test_coup3_curr_steady_lin_heat_trans_nl_elast_lin_axisym_weak.py")
execfile("test_coup3_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_axisym.py")
#execfile("test_coup3_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_axisym.py");
#execfile("test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_hard.py")
execfile("test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_weak.py")
#the following test works, even though it is one-step transient version of the steady state that does not
execfile("test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_hard.py")
execfile("test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_weak.py")

# based on test_coupling_4_transient_planar.py
execfile("test_coup4_curr_steady_lin_heat_trans_lin_planar_weak.py")
execfile("test_coup4_curr_steady_nl_heat_trans_nl_planar_hard.py")
execfile("test_coup4_curr_steady_nl_heat_trans_nl_planar_weak.py") 
