import time as timer

start = timer.time()

execfile("test_heat_transfer_steady_planar_nonlin.py")
execfile("test_flow_steady_planar_nonlin.py") 
execfile("test_flow_steady_axisymmetric_nonlin.py") 
execfile("test_magnetic_steady_planar_nonlin.py")
execfile("test_coupling_3_axisymmetric_nonlin.py")
#execfile("test_magnetic_harmonic_planar_nonlin.py")

stop = timer.time()
print("\nTotal time: " + str(stop - start))