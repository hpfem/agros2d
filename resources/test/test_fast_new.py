import agros2d as a2d
import unittest as ut
import sys

suite = ut.TestSuite()
result = a2d.Agros2DTestResult()

# fields
sys.path.append("fields")

import electrostatic
import current

# electrostatic
suite.addTest(ut.TestLoader().loadTestsFromTestCase(electrostatic.ElectrostaticPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(electrostatic.ElectrostaticAxisymmetric))

# current field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(current.CurrentPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(current.CurrentAxisymmetric))



# adaptivity
sys.path.append("adaptivity")

import adaptivity

suite.addTest(ut.TestLoader().loadTestsFromTestCase(adaptivity.AdaptivityElectrostatic))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(adaptivity.AdaptivityAcoustic))



# script
sys.path.append("script")

import problem
import geometry
import benchmark

suite.addTest(ut.TestLoader().loadTestsFromTestCase(problem.TestProblem))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(geometry.TestGeometry))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(geometry.TestGeometryTransformations))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(benchmark.BenchmarkGeometryTransformation))

# run tests
suite.run(result)

#from test import Test

#test_fast = Test(error_file='test_fast.err')

# magnetic field
#test_fast.add("fields/test_magnetic_steady_planar.py")
#test_fast.add("fields/test_magnetic_steady_axisymmetric.py")

# harmonic magnetic field
#test_fast.add("fields/test_magnetic_harmonic_planar.py") 
#test_fast.add("fields/test_magnetic_harmonic_axisymmetric.py")

# heat transfer
#test_fast.add("fields/test_heat_transfer_steady_planar.py")
#test_fast.add("fields/test_heat_transfer_steady_planar_nonlin.py")
#test_fast.add("fields/test_heat_transfer_steady_axisymmetric.py")
#test_fast.add("fields/test_heat_transfer_transient_axisymmetric_benchmark.py") # benchmark

# structural mechanics
#test_fast.add("fields/test_elasticity_planar.py")
#test_fast.add("fields/test_elasticity_axisymmetric.py")

# acoustic
#test_fast.add("fields/test_acoustic_planar.py") 
#test_fast.add("fields/test_acoustic_axisymmetric.py")

# electromagnetic wave - TE
#test_fast.add("fields/test_rf_te_planar.py")
#test_fast.add("fields/test_rf_te_axisymmetric.py")

# electromagnetic wave - TM
#test_fast.add("fields/test_rf_tm_planar.py")
#test_fast.add("fields/test_rf_tm_axisymmetric.py")

# incompressible flow
#test_fast.add("fields/test_flow_steady_planar_nonlin.py") 
#test_fast.add("fields/test_flow_steady_axisymmetric_nonlin.py") 

# coupled problems
#test_fast.add("coupled_problems/test_cf_1_planar.py")
#test_fast.add("coupled_problems/test_cf_2_axisymmetric.py")
#test_fast.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")
#test_fast.add("coupled_problems/test_cf_4_transient_planar.py") 

# particle tracing
#test_fast.add("particle_tracing/test_particle_tracing_planar.py")
#test_fast.add("particle_tracing/test_particle_tracing_axisymmetric.py")

# internal
#test_fast.add("internal/test_matrix_solvers.py")

#test_fast.run()