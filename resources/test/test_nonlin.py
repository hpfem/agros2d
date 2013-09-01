import test_suite

tests = [
# heat transfer
test_suite.fields.heat.HeatNonlinPlanar,
# magnetic field
test_suite.fields.magnetic.MagneticNonlinPlanar,
#test_suite.fields.magnetic.MagneticNonlinAxisymmetric,
#test_suite.fields.magnetic.MagneticHarmonicNonlinPlanar,
# incompressible flow
test_suite.fields.flow.FlowPlanar,
test_suite.fields.flow.FlowAxisymmetric
]

# coupled problems
# test_nonlin.add("coupled_problems/test_cf_3_axisymmetric_nonlin.py")


if __name__ == '__main__':        
    import unittest as ut
    import agros2d
    
    suite = ut.TestSuite()
    
    for test in tests:
        suite.addTest(ut.TestLoader().loadTestsFromTestCase(test))
    
    result = agros2d.Agros2DTestResult()
    suite.run(result)

    if (not result.wasSuccessful()):
        raise Exception('Failure', result.failures)
