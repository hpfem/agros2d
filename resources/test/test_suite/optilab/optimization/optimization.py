from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase

from variant.optimization import ContinuousParameter, DiscreteParameter, Parameters
from variant.optimization import Functional, Functionals
from variant.optimization import OptimizationMethod

from test_suite.optilab.examples import booths_function
from test_suite.optilab.examples import binh_korn_function

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

class TestFunctional(Agros2DTestCase):
    def setUp(self):
        self.F = Functional('F', 'min')

    def test_direction(self):
        self.assertEqual(self.F.direction, 'min')
        self.F.direction = 'max'
        self.assertEqual(self.F.direction, 'max')

        with self.assertRaises(ValueError):
            self.F.direction = 'somewhere'

class TestFunctionals(Agros2DTestCase):
    def test_functionals(self):
        self.assertEqual(len(Functionals(Functional('F', 'min')).functionals), 1)
        self.assertEqual(len(Functionals([Functional('F', 'min'),
                                          Functional('G', 'max')]).functionals), 2)

        with self.assertRaises(TypeError):
            Functionals([Functional('F', 'min'), 'G'])

    def test_multicriteria(self):
        functionals = Functionals(Functional('F', 'min'))
        self.assertFalse(functionals.multicriteria())

        functionals = Functionals([Functional('F', 'min'), Functional('G', 'max')])
        self.assertTrue(functionals.multicriteria())

        functionals = Functionals([Functional('F', 'min'), Functional('G', 'max'),
                                   Functional('H', 'max'), Functional('I', 'max')])
        self.assertTrue(functionals.multicriteria())

    def test_evaluate_one_functional(self):
        model = booths_function.BoothsFunction()
        model.parameters['x'] = 1
        model.parameters['y'] = 3
        model.solve()
        model.process()

        self.assertEqual(Functionals(Functional('F', 'min')).evaluate(model), 0)

    def test_evaluate_two_functional(self):
        model = binh_korn_function.BinhKornFunction()
        model.parameters['x'] = 2.5
        model.parameters['y'] = 1.5
        model.solve()
        model.process()

        functionals = Functionals([Functional('F1', 'min'), Functional('F2', 'min')])
        self.assertEqual(functionals.evaluate(model), [34.0, 18.5])

class TestMethod(Agros2DTestCase):
    def test_method_init(self):
        parameters = Parameters([ContinuousParameter('x', 0, 1), DiscreteParameter('y', range(10))])
        functionals = Functionals([Functional('F', 'min'), Functional('G', 'max')])

        method = OptimizationMethod(parameters, functionals, ModelBase)
        self.assertEqual(method.parameters, parameters)
        self.assertEqual(method.functionals, functionals)

    def test_method_init_with_wrong_parameters(self):
        parameters = [DiscreteParameter('y', range(10)), 'wrong_parameter']
        functionals = Functionals([Functional('F', 'min'), Functional('G', 'max')])

        with self.assertRaises(TypeError):
            OptimizationMethod(parameters, functionals)

    def test_method_init_with_wrong_functionals(self):
        parameters = [DiscreteParameter('y', range(10)), 'wrong_parameter']
        functionals = [Functional('F', 'min'), Functional('G', 'max')]

        with self.assertRaises(TypeError):
            OptimizationMethod(parameters, functionals)

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestContinuousParameter))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestDiscreteParameter))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFunctional))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFunctionals))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMethod))
    suite.run(result)
