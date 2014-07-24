from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase, ModelDict, ModelGenerator

import numpy as np
import pythonlab

class BoothsFunction(ModelBase):
    """ f(x,y) = (x + 2y - 7)**2 + (2x + y - 5)**2 """
    def solve(self):
        try:
            self.F = (self.parameters['x'] + 2 * self.parameters['y'] - 7)**2 + \
                     (2 * self.parameters['x'] + self.parameters['y'] - 5)**2

            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F

class TestBoothsFunction(Agros2DTestCase):
    def setUp(self):
        self.model = BoothsFunction

    def test_parameteric_study(self):
        mg = ModelGenerator()
        n = 50
        k = 2
        mg.add_parameter('x', np.linspace(-10, 10, n))
        mg.add_parameter('y', np.linspace(-10, 10, n))
        mg.combination()

        cwd = '{0}/models'.format(pythonlab.tempname())
        mg.save(cwd)

        md = ModelDict()
        md.directory = cwd
        md.load(BoothsFunction)
        md.solve()

        self.assertEqual(len(md.solved_models), n**k)

    def test_optimization(self):
        pass

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestBoothsFunction))
    suite.run(result)