import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase

class VectorMultiplyingByScalar(ModelBase):
    def declare(self):
        self.declare_parameter('u', list)
        self.declare_parameter('c', float, default=2.0)
        self.declare_variable('v', list)

    def create(self):
        pass

    def solve(self):
        self.v = []
        for ui in self.parameters['u']:
            self.v.append(ui * self.parameters['c'])

    def process(self):
        self.variables['v'] = self.v

class Model(ModelBase):
    def declare(self):
        self.model_info.add_parameter('p', int)

class TestModel(Agros2DTestCase):
    def test_undeclared_parameter(self):
        model = ModelBase()
        with self.assertRaises(KeyError):
            model.parameters['undeclared_parameter'] = 1.0

    def test_undeclared_variable(self):
        model = ModelBase()
        with self.assertRaises(KeyError):
            model.variables['undeclared_variable'] = 1.0

    def test_parameter_with_bad_data_type(self):
        model = VectorMultiplyingByScalar()

        with self.assertRaises(TypeError):
            model.parameters['u'] = 1

        with self.assertRaises(TypeError):
            model.parameters['u'] = {'u1' : 3, 'u2' : 5}

        with self.assertRaises(TypeError):
            model.parameters['u'] = (3, 5)

        with self.assertRaises(TypeError):
            model.parameters['c'] = [3, 5]

    def test_default(self):
        model = VectorMultiplyingByScalar()
        model.parameters['u'] = [5, 3, 6]
        model.solve()
        model.process()
        self.assertEqual([10, 6, 12], model.variables['v'])

    def test_save_and_load(self):
        file_name = '{0}/model.pickle'.format('{0}'.format(pythonlab.tempname()))

        model = VectorMultiplyingByScalar()
        model.parameters['u'] = [3, 5]
        model.parameters['c'] = 3
        model.create()
        model.solve()
        model.process()
        model.save(file_name)

        model.clear()
        model.load(file_name)
        
        self.assertEqual([9, 15], model.variables['v'])

    def test_clear(self):
        model = VectorMultiplyingByScalar()
        model.parameters['u'] = [1,2,3]
        model.clear()
        with self.assertRaises(KeyError):
            model.parameters['u']

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModel))
    suite.run(result)
