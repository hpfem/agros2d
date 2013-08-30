import agros2d as a2d
import unittest as ut
import sys

suite = ut.TestSuite()
result = a2d.Agros2DTestResult()

sys.path.append("script")

import problem
import field
import geometry
import benchmark

suite.addTest(ut.TestLoader().loadTestsFromTestCase(problem.TestProblem))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(problem.TestField))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(geometry.TestGeometry))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(geometry.TestGeometryTransformations))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(benchmark.BenchmarkGeometryTransformation))

# run tests
suite.run(result)
