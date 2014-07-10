import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase, ModelDict, ModelDictExternal
from problem import Model

class TestModelDict(Agros2DTestCase):
    def setUp(self):
        self.md = ModelDict()
        self.md.directory = '{0}/models'.format(pythonlab.tempname())

    def test_add_model(self):
        model = ModelBase()
        self.md.add_model(model)
        self.assertTrue(len(self.md.models))

    def test_save_and_load(self):
        model = ModelBase()

        N = 10
        for i in range(N):
            self.md.add_model(model)
        self.md.save()

        self.md.clear()
        self.md.load(ModelBase)
        self.assertEqual(N, len(self.md.models))

    def test_solve(self):
        variants = [(1, 2), (2, 3), (3, 4)]
        for a, b in variants:
            model = Model()
            model.parameters['a'] = a
            model.parameters['b'] = b
            self.md.add_model(model)

        self.md.solve()
        for a, b in variants:
            model = self.md.find_model({'a' : a, 'b' : b})
            self.assertEqual(a**b, model.variables['sqr'])

    def test_solve_models_by_mask(self):
        variants = [(1, 2), (2, 3), (3, 4)]
        for a, b in variants:
            model = Model()
            model.parameters['a'] = a
            model.parameters['b'] = b
            self.md.add_model(model)

        self.md.solve(mask='model_00000[0, 2]')
        self.assertEqual(len(self.md.models), 3)
        self.assertEqual(len(self.md.solved_models), 2)

class TestModelDictExternal(Agros2DTestCase):
    def setUp(self):
        self.md = ModelDictExternal()
        self.md.directory = '{0}/solutions'.format(pythonlab.tempname())
        self.md.solver = pythonlab.datadir('agros2d_solver')

    def test_external_solver(self):
        model = Model()
        model.parameters['a'] = 4
        model.parameters['b'] = 2

        self.md.add_model(model)
        self.md.save()

        self.md.solve()
        self.assertEqual(4**2, self.md.models[0].variables['sqr'])

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelDict))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelDictExternal))
    suite.run(result)
