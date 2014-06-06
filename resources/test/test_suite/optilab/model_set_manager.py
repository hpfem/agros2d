import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelSetManager, ModelBase

class TestModelSetManager(Agros2DTestCase):
    def setUp(self):
        self.solver = ModelSetManager()
        self.solver.solver = pythonlab.datadir('agros2d_solver')
        self.solver.directory = pythonlab.datadir('resources/test/test_suite/optilab/model_set_manager/solutions')
        self.solver.delete_all()

        L = [0.01, 0.02, 0.03, 0.04, 0.05]
        for i in range(len(L)):
            model = ModelBase()

            model.parameters["R1"] = 0.01
            model.parameters["R2"] = 0.03
            model.parameters["R3"] = 0.05
            model.parameters["R4"] = 0.06
            model.parameters["L"] = L[i]

            model.save('{0}/solutions_{1}.pickle'.format(self.solver.directory, i))

    def test_values(self):
        self.solver.solve_all()

        models = self.solver.load_all()
        for model in models:
            self.assertEqual(model.solved, 1)

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestModelSetManager))
    suite.run(result)