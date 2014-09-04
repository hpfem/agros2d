from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelDictionary, ModelFilter
from variant.test_functions import quadratic_function

class TestModelFilter(Agros2DTestCase):
    @classmethod
    def setUpClass(self):
        self.a = 3
        self.b = 2
        self.c = 8

        self.md = ModelDictionary(quadratic_function.QuadraticFunction)
        for x in range(10):
            model = quadratic_function.QuadraticFunction()
            model.parameters['a'] = self.a
            model.parameters['b'] = self.b
            model.parameters['c'] = self.c
            model.parameters['x'] = x
            self.md.add_model(model)

        self.md.solve()

    def test_parameter_value(self):
        mf = ModelFilter()
        mf.add_parameter_value('x', 2)
        self.assertEqual(len(mf.filter(self.md).dictionary), 1)

    def test_parameter_range(self):
        mf = ModelFilter()
        mf.add_parameter_range('x', 4, 6)
        self.assertEqual(len(mf.filter(self.md).dictionary), 3)

    def test_variable_value(self):
        mf = ModelFilter()
        mf.add_variable_value('F', self.a*4**2+self.b*4+self.c)
        self.assertEqual(len(mf.filter(self.md).dictionary), 1)

    def test_variable_range(self):
        mf = ModelFilter()
        mf.add_variable_range('F', self.a*4**2+self.b*4+self.c, self.a*6**2+self.b*6+self.c)
        self.assertEqual(len(mf.filter(self.md).dictionary), 3)

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelFilter))
    suite.run(result)