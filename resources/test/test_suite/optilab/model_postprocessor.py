from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase, ModelDictionary, ModelPostprocessor
from time import time

class WallisFormula(ModelBase):
    def declare(self):
        self.declare_parameter('n', int)
        self.declare_variable('pi', float)
        self.declare_variable('te', list)

    def create(self):
        pass

    def solve(self):
        self.elapsed_time = []
        self.f = 1.0

        start_time = time()
        for i in range(1, self.get_parameter('n') + 1):
            self.f *= 4*i**2/(4*i**2 -1)
            self.elapsed_time.append(time()-start_time)

    def process(self):
        self.set_variable('pi', 2 * self.f)
        self.set_variable('te', self.elapsed_time)

class TestModelPostprocessor(Agros2DTestCase):
    @classmethod
    def setUpClass(self):
        self.md = ModelDictionary(WallisFormula)

        self.variants = [10, 50, 100, 500, 1000, 5000, 10000]
        for n in self.variants:
            model = WallisFormula()
            model.set_parameter('n', n)
            self.md.add_model(model)

        self.md.solve()
        """
        for model in self.md.models:
            print('n={0:1.1e}; pi={1}'.format(model.get_parameter('n'),
                                              model.get_variable('pi')))
        """

    def test_parameters(self):
        mp = ModelPostprocessor(self.md)

        number_of_parameters = len(self.md.models[-1].data.declared_parameters)
        number_of_scalar_parameters = 0
        for parameter in self.md.models[-1].data.declared_parameters.values():
            if (parameter['type'] == int or parameter['type'] == float):
                number_of_scalar_parameters += 1

        self.assertEqual(len(mp.parameters()), number_of_parameters)
        self.assertEqual(len(mp.parameters(only_scalars=True)), number_of_scalar_parameters)
        self.assertEqual(len(mp.parameters_keys()), number_of_parameters)
        self.assertEqual(len(mp.parameters_keys(only_scalars=True)), number_of_scalar_parameters)

        for parameter, values in mp.parameters().items():
            self.assertEqual(len(values), len(self.variants))

    def test_variables(self):
        mp = ModelPostprocessor(self.md)

        number_of_variables = len(self.md.models[-1].data.declared_variables)
        number_of_scalar_variables = 0
        for variable in self.md.models[-1].data.declared_variables.values():
            if (variable['type'] == int or variable['type'] == float):
                number_of_scalar_variables += 1

        self.assertEqual(len(mp.variables()), number_of_variables)
        self.assertEqual(len(mp.variables(only_scalars=True)), number_of_scalar_variables)
        self.assertEqual(len(mp.variables_keys()), number_of_variables)
        self.assertEqual(len(mp.variables_keys(only_scalars=True)), number_of_scalar_variables)

        for parameter, values in mp.parameters().items():
            self.assertEqual(len(values), len(self.variants))

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelPostprocessor))
    suite.run(result)