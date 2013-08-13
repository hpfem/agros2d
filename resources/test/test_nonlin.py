from test import Test

test_nonlin = Test(error_file='test_nonlin.err')

# nonlinear problems
test_nonlin.add("fields/test_heat_transfer_steady_planar_nonlin.py")
test_nonlin.add("fields/test_flow_steady_planar_nonlin.py") 
test_nonlin.add("fields/test_flow_steady_axisymmetric_nonlin.py") 
test_nonlin.add("fields/test_magnetic_steady_planar_nonlin.py")
test_nonlin.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")
#test_nonlin.add("fields/test_magnetic_harmonic_planar_nonlin.py")

test_nonlin.run()