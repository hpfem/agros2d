import agros2d as a2d
import unittest as ut

import test_suite

suite = ut.TestSuite()

# script
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.problem.TestProblem))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.geometry.TestGeometry))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.geometry.TestGeometryTransformations))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.benchmark.BenchmarkGeometryTransformation))


# run tests
result = a2d.Agros2DTestResult()
suite.run(result)

if (not result.wasSuccessful()):
   raise Exception('Failure', result.failures)
