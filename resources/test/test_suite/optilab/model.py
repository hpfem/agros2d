from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase
from problem import Model
from shutil import rmtree
from os.path import isdir

class TestModel(Agros2DTestCase):
    def setUp(self):
        if isdir('solutions'):
            rmtree('solutions')

    def test_defaults(self):
        model = ModelBase()

        model.defaults['p'] = 0
        self.assertEqual(0, model.parameters['p'])
        
        model.defaults['p'] = 1
        self.assertEqual(1, model.parameters['p'])

    def test_undefined_default(self):
        model = ModelBase()
        with self.assertRaises(KeyError):
            model.parameters['p']

    def test_save_and_load(self):
        model = Model()
        model.parameters['a'] = 5
        model.parameters['b'] = 7
        model.solve()
        model.process()
        model.save('solutions/test.pickle')

        model.clear()
        model.load('solutions/test.pickle')
        self.assertEqual(5**7, model.variables['sqr'])

    def test_solve(self):
        model = Model()
        model.parameters['a'] = 4
        model.parameters['b'] = 2

        model.solve()
        model.process()

        self.assertEqual(4**2, model.variables['sqr'])

    def test_clear(self):
        model = ModelBase()
        model.parameters['p'] = 0
        model.clear()
        with self.assertRaises(KeyError):
            model.parameters['p']

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModel))
    suite.run(result)
