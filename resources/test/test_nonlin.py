import agros2d as a2d
import unittest as ut

import test_suite

suite = ut.TestSuite()

# heat transfer
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.heat.HeatNonlinPlanar))

# magnetic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticNonlinPlanar))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticNonlinAxisymmetric))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticHarmonicNonlinPlanar))

# incompressible flow
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.flow.FlowPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.flow.FlowAxisymmetric))

# coupled problems
# test_nonlin.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")

# run tests
result = a2d.Agros2DTestResult()
suite.run(result)

if (not result.wasSuccessful()):
   raise Exception('Failure', result.failures)
