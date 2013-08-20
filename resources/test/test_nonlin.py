import agros2d as a2d
import unittest as ut
import sys

suite = ut.TestSuite()
result = a2d.Agros2DTestResult()

# fields
sys.path.append("fields")
import flow
import heat
import magnetic

sys.path.append("coupled_problems")
# import basic_coupled_problems

# heat transfer
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatNonlinPlanar))

# magnetic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticNonlinPlanar))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticNonlinAxisymmetric))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticHarmonicNonlinPlanar))

# incompressible flow
suite.addTest(ut.TestLoader().loadTestsFromTestCase(flow.FlowPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(flow.FlowAxisymmetric))

# coupled problems
# test_nonlin.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")

# run tests
suite.run(result)
