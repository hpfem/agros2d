import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelDictionary, ModelDictionaryExternal
from variant.test_functions import quadratic_function

class TestModelDictionary(Agros2DTestCase):
    def setUp(self):
        self.md = ModelDictionary()
        self.file_name = '{0}.opt'.format(pythonlab.tempname())
        self.problem = pythonlab.datadir('resources/python/variant/test_functions/quadratic_function.py')

    def test_add_model(self):
        N = 100
        for i in range(N):
            model = quadratic_function.QuadraticFunction()
            model.set_parameter('x', i)
            self.md.add_model(model)

        self.assertEqual(N, len(self.md.models))

    def test_add_model_with_existing_name(self):
        model = quadratic_function.QuadraticFunction()
        self.md.add_model(model, 'model')
        with self.assertRaises(KeyError):
            self.md.add_model(model, 'model')

    def test_save_and_load(self):
        N = 100
        for i in range(N):
            model = quadratic_function.QuadraticFunction()
            model.set_parameter('x', i)
            self.md.add_model(model)

        self.md.save(self.file_name, self.problem)
        self.md.clear()
        self.md.load(self.file_name)
        self.assertEqual(N, len(self.md.models))

    def test_solve(self):
        variants = [(52, 4), (23, 12), (57, 21)]
        for a, x in variants:
            model = quadratic_function.QuadraticFunction()
            model.set_parameter('a', a)
            model.set_parameter('x', x)
            self.md.add_model(model)

        self.md.solve()
        for a, x in variants:
            model = self.md.find_model_by_parameters({'a' : a, 'b' : 1, 'c' : 1, 'x' : x})
            self.assertEqual(a*x**2 + model.get_parameter('b')*x + model.get_parameter('c'), model.get_variable('F'))

    def test_solve_and_save(self):
        N = 10
        for x in range(N):
            model = quadratic_function.QuadraticFunction()
            model.set_parameter('x', x)
            self.md.add_model(model)

        self.md.save(self.file_name, self.problem)
        self.md.solve(save=True)
        self.assertEqual(N, len(self.md.solved_models))
        self.assertEqual(N, len(self.md.models))

        self.md.clear()
        self.assertEqual(0, len(self.md.solved_models))
        self.assertEqual(0, len(self.md.models))

        self.md.load(self.file_name)
        self.assertEqual(N, len(self.md.solved_models))
        self.assertEqual(N, len(self.md.models))

class TestModelDictionaryExternal(Agros2DTestCase):
    def setUp(self):
        self.md = ModelDictionaryExternal(quadratic_function.QuadraticFunction)
        self.md.solver = pythonlab.datadir('agros2d_solver')

        self.file_name = '{0}.opt'.format(pythonlab.tempname())
        self.problem = pythonlab.datadir('resources/python/variant/test_functions/quadratic_function.py')

    def test_solve(self):
        variants = [(52, 4), (23, 12), (57, 21)]
        for a, x in variants:
            model = quadratic_function.QuadraticFunction()
            model.set_parameter('a', a)
            model.set_parameter('x', x)
            self.md.add_model(model)

        self.md.save(self.file_name, self.problem)
        self.md.solve()
        for a, x in variants:
            model = self.md.find_model_by_parameters({'a' : a, 'b' : 1, 'c' : 1, 'x' : x})
            self.assertEqual(a*x**2 + model.get_parameter('b')*x + model.get_parameter('c'), model.get_variable('F'))

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelDictionary))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelDictionaryExternal))
    suite.run(result)