import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase

class Model(ModelBase):
    """ y = a*x**2 """
    def create(self):
        self.defaults['a'] = 1

    def solve(self):
        try:
            self.y = self.parameters['a'] * self.parameters['x']**2
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['y'] = self.y

class TestModel(Agros2DTestCase):
    def test_defaults(self):
        model = ModelBase()
        model.defaults['b'] = 0
        self.assertEqual(0, model.parameters['b'])

        model = Model()
        model.parameters['x'] = 4
        model.create()
        model.solve()
        model.process()
        self.assertEqual(4**2, model.variables['y'])

    def test_undefined_default(self):
        model = ModelBase()
        with self.assertRaises(KeyError):
            model.parameters['b']

    def test_save_and_load(self):
        file_name = '{0}/model.pickle'.format('{0}'.format(pythonlab.tempname()))

        model = Model()
        model.parameters['x'] = 7
        model.create()
        model.solve()
        model.process()
        model.save(file_name)

        model.clear()
        model.load(file_name)
        self.assertEqual(7**2, model.variables['y'])

    def test_solved(self):
        model = ModelBase()
        model.parameters['nonexisting_parameter'] = 123
        model.create()
        model.solve()
        model.process()

        self.assertFalse(model.solved)

    def test_clear(self):
        model = ModelBase()
        model.parameters['b'] = 0
        model.clear()
        with self.assertRaises(KeyError):
            model.parameters['b']

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModel))
    suite.run(result)