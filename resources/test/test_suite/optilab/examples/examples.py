from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelDict, ModelGenerator
from variant.test_functions import booths_function

import numpy as np
import pythonlab

class TestBoothsFunction(Agros2DTestCase):
    def test_parametric_study(self):
        mg = ModelGenerator(booths_function.BoothsFunction)
        n = 50
        k = 2
        mg.add_parameter('x', np.linspace(-10, 10, n))
        mg.add_parameter('y', np.linspace(-10, 10, n))
        mg.combination()

        cwd = '{0}/models'.format(pythonlab.tempname())
        mg.save(cwd)

        md = ModelDict()
        md.directory = cwd
        md.load()
        md.solve()

        self.assertEqual(len(md.solved_models()), n**k)

    def test_optimization(self):
        pass

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestBoothsFunction))
    suite.run(result)