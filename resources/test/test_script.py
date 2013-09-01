import test_suite

tests = [
test_suite.script.problem.TestProblem,
test_suite.script.geometry.TestGeometry,
test_suite.script.geometry.TestGeometryTransformations,
test_suite.script.benchmark.BenchmarkGeometryTransformation
]

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
