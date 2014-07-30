import pythonlab
import random as rnd

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase

from variant.optimization import *
from variant.optimization.genetic import *


class TestCrossover(Agros2DTestCase):
    def setUp(self):
        parameters = [ContinuousParameter("a", -5, -1),
                      ContinuousParameter("b", 6, 8),
                      ContinuousParameter("c", 0, 99),
                      ContinuousParameter("d", 0, 1),
                      DiscreteParameter("e", [1, 3, 6, 88, 2]),
                      DiscreteParameter("f", [-41, 8, 1, 2, 3]),
                      DiscreteParameter("g", [21, 75, 25, 88, 11]),
                      DiscreteParameter("h", [85, 12, -147, 21, 1])]

        population_creator = ImplicitInitialPopulationCreator(ModelBase, parameters)
        self.population = population_creator.create(5)
        self.crossover_creator = ImplicitCrossover()

    def test_crossover(self):
        mother = rnd.choice(self.population)
        father = rnd.choice(self.population)
        son = self.crossover_creator.cross(mother, father)

        self.assertTrue(son.parameters != mother.parameters)
        self.assertTrue(son.parameters != father.parameters)

        for key, value in son.parameters.items():
            self.assertTrue((value == mother.parameters[key]) or (value == father.parameters[key]))

        mother_gens_count = 0
        father_gens_count = 0
        for key, value in son.parameters.items():
            if (mother.parameters[key] == value):
                mother_gens_count += 1
            else:
                father_gens_count += 1

        self.assertTrue((mother_gens_count != 0) or (father_gens_count != 0))

if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestCrossover))
    suite.run(result)
