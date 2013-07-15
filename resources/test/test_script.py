from unittest import TestLoader, TestResult

import sys
sys.path.append("script")

import test_script_problem

import test_script_geometry
import benchmark_script_geometry

result = TestResult()

""" problem """
TestLoader().loadTestsFromTestCase(test_script_problem.TestProblem).run(result)
print('Basic problem test: {0}'.format(result.wasSuccessful()))

""" geometry """
TestLoader().loadTestsFromTestCase(test_script_geometry.TestGeometry).run(result)
print('Basic geometry test: {0}'.format(result.wasSuccessful()))

TestLoader().loadTestsFromTestCase(test_script_geometry.TestGeometryTransformations).run(result)
print('Geometry transformations test: {0}'.format(result.wasSuccessful()))

TestLoader().loadTestsFromTestCase(benchmark_script_geometry.BenchmarkGeometryTransformation).run(result)
print('Geometry benchmark: {0}'.format(result.wasSuccessful()))

if (result.wasSuccessful() != True):
    print(result.errors)
