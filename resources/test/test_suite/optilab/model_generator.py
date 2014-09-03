from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase, ModelGenerator

class Model(ModelBase):
    def declare(self):
        self.n = 5
        self.k = 2

        for i in range(self.n):
            self.declare_parameter('p{0}'.format(i), int)

class TestModelGenerator(Agros2DTestCase):
    def setUp(self):
        self.mg = ModelGenerator(Model)

    def test_add_and_remove_parameter(self):
        self.mg.add_parameter('p', range(10))
        self.assertGreater(len(self.mg.parameters), 0)

        self.mg.remove_parameter('p')
        self.assertEqual(len(self.mg.parameters), 0)

    def test_add_parameter_by_interval(self):
        for start, stop, step in [(0,10,1), (20,24,2), (-4, 4, 4),
                                  (0.002, 0.07, 0.0001), (1e-5, 2e-5, 1e-6)]:

          self.mg.add_parameter_by_interval('p', start, stop, step)
          self.assertEqual(len(self.mg.parameters['p']), int((stop - start)/step + 1))

    def test_combination(self):
        n = 5
        k = 2

        for i in range(n):
            self.mg.add_parameter('p{0}'.format(i), range(k))

        self.mg.combination()
        self.assertTrue(len(self.mg.models()), n**k)

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelGenerator))
    suite.run(result)
