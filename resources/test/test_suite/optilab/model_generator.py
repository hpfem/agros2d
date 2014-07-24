from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelGenerator
from math import factorial

class TestModelGenerator(Agros2DTestCase):
    def setUp(self):
        self.mg = ModelGenerator()

    def test_add_and_remove_parameter(self):
        self.mg.add_parameter('p', range(10))
        self.assertGreater(len(self.mg.parameters), 0)

        self.mg.remove_parameter('p')
        self.assertEqual(len(self.mg.parameters), 0)

    def test_combination(self):
        n = 5
        k = 2

        for i in range(n):
            self.mg.add_parameter('p{0}'.format(i), range(k))

        self.mg.combination()
        self.assertTrue(factorial(n)/(factorial(k)*factorial(n-k)), len(self.mg.models))

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelGenerator))
    suite.run(result)