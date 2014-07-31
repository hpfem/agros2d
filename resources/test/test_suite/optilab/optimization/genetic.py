import pythonlab
import random as rnd

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from variant import ModelBase

from variant.optimization import *
from variant.optimization.genetic import *

def test_population(genoms):
    parameters = Parameters([ContinuousParameter("a", -5, -1),
                             ContinuousParameter("b", 6, 8),
                             ContinuousParameter("c", 0, 99),
                             ContinuousParameter("d", 0, 1),
                             DiscreteParameter("e", [1, 3, 6, 88, 2]),
                             DiscreteParameter("f", [-41, 8, 1, 2, 3]),
                             DiscreteParameter("g", [21, 75, 25, 88, 11]),
                             DiscreteParameter("h", [85, 12, -147])])

    population_creator = ImplicitInitialPopulationCreator(ModelBase, parameters)
    return population_creator.create(genoms), parameters

class TestCrossover(Agros2DTestCase):
    def setUp(self):
        self.population, self.parameters = test_population(10)
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

class TestMutation(Agros2DTestCase):
    def setUp(self):
        self.population, self.parameters = test_population(10)
        self.mutation_creator = ImplicitMutation(self.parameters)

    def test_mutation(self):
        original = rnd.choice(self.population)
        mutant = self.mutation_creator.mutate(original)

        mutations = 0
        for key in original.parameters.keys():
            if (original.parameters[key] != mutant.parameters[key]): mutations += 1

        self.assertTrue(mutations != 0)


if __name__ == '__main__':
    import unittest as ut
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestCrossover))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMutation))
    suite.run(result)
