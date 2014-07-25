from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant.optimization import ContinuousParameter, DiscreteParameter

class TestContinuousParameter(Agros2DTestCase):
    def setUp(self):
        self.min = 1.1
        self.max = 9.9
        self.cp = ContinuousParameter("p", self.min, self.max)

    def test_random_value(self):
        self.assertGreaterEqual(self.cp.random_value(), self.min)
        self.assertLessEqual(self.cp.random_value(), self.max)

        self.assertNotEqual(self.cp.random_value(), self.cp.random_value())

    def test_perturbation(self):
        self.assertGreaterEqual(self.cp.perturbation(self.min, 0.5), self.min)
        self.assertLessEqual(self.cp.perturbation(self.max, 0.5), self.max)

        self.assertNotEqual(self.cp.perturbation((self.max-self.min)/2.0, 0.1),
                            self.cp.perturbation((self.max-self.min)/2.0, 0.1))

    def test_perturbation_with_wrong_original(self):
        with self.assertRaises(ValueError):
            self.cp.perturbation(self.min - (self.max-self.min)/2.0, 0.1)

        with self.assertRaises(ValueError):
            self.cp.perturbation(self.max + (self.max-self.min)/2.0, 0.1)

class TestDiscreteParameter(Agros2DTestCase):
    def setUp(self):
        self.options = range(10)
        self.min = min(self.options)
        self.max = max(self.options)
        self.dp = DiscreteParameter("p", self.options)

    def test_random_value(self):
        self.assertTrue(self.dp.random_value() in self.options)

    def test_perturbation(self):
        self.assertTrue(self.dp.perturbation(5, 0.1) in self.options)
        self.assertTrue(self.dp.perturbation(self.min, 0.1) in self.options)
        self.assertTrue(self.dp.perturbation(self.max, 0.1) in self.options)

    def test_perturbation_with_wrong_original(self):
        with self.assertRaises(ValueError):
            self.dp.perturbation(-1, 0.1)

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestContinuousParameter))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestDiscreteParameter))
    suite.run(result)