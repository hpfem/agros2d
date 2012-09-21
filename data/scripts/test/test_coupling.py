# contains sets of tests obtained by varying previously present coupling tests
# various combinations of hard/weak and linear/nonlinear are tested
# in some couplings, the forms are the same for one of fields being steadystate or transient. 
# in our implementation, however, two sets of forms are present (two identical copies). 
# Those forms are also tested by changing steadystate problem artificaly to transient and 
# setting coefficients of the time derivative term to zero, thus obtaining the same results


# based on test_coupling_1_planar.py
execfile("test_coup_curr_steady_lin_heat_steady_lin_elast_lin_weak_planar.py") 
execfile("test_coup_curr_steady_lin_heat_trans_lin_elast_lin_weak_planar.py") 
execfile("test_coup_curr_steady_nl_heat_steady_nl_elast_nl_hard_planar.py") 
#execfile("test_coup_curr_steady_nl_heat_trans_nl_elast_nl_hard_planar.py") 
execfile("test_coup_curr_steady_nl_heat_steady_nl_elast_nl_weak_planar.py") 
#execfile("test_coup_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_planar.py")
execfile("test_coup_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_planar.py")
execfile("test_coup_curr_steady_nl_heat_trans_nl_elast_nl_weak_planar.py") 

# based on test_coupling_2_axisymmetric.py
execfile("test_coup_mag_harm_lin_heat_steady_lin_weak_axisym.py") 
execfile("test_coup_mag_harm_nl_heat_steady_nl_weak_axisym.py") 
#execfile("test_coup_mag_harm_nl_heat_steady_nl_hard_axisym.py") 
execfile("test_coup_mag_harm_lin_heat_trans_lin_weak_axisym.py") 
execfile("test_coup_mag_harm_nl_heat_trans_nl_weak_axisym.py") 
#execfile("test_coup_mag_harm_nl_heat_trans_nl_hard_axisym.py") 
