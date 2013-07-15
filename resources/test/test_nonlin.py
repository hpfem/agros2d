import pythonlab
import time as timer
start = timer.time()

# nonlinear problems
print("Nonlinear problems")

execfile("fields/test_heat_transfer_steady_planar_nonlin.py")
execfile("fields/test_flow_steady_planar_nonlin.py") 
execfile("fields/test_flow_steady_axisymmetric_nonlin.py") 
execfile("fields/test_magnetic_steady_planar_nonlin.py")
execfile("coupled_problems/test_cf_3_axisymmetric_nonlin.py")
#execfile("fields/test_magnetic_harmonic_planar_nonlin.py")

stop = timer.time()
print("")
print("Total time: " + str(stop - start))