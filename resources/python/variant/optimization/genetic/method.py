from variant import ModelBase
from variant.optimization import *

from variant.optimization.genetic.info import GeneticInfo
from variant.optimization.genetic.initial_population import ImplicitInitialPopulationCreator
from variant.optimization.genetic.selector import SingleCriteriaSelector, MultiCriteriaSelector
from variant.optimization.genetic.mutation import ImplicitMutation
from variant.optimization.genetic.crossover import ImplicitCrossover

import random as rnd

class GeneticOptimization(OptimizationMethod):
    def __init__(self, parameters, functionals, model_class = ModelBase):
        OptimizationMethod.__init__(self, parameters, functionals, model_class)
        self.current_population_index = 0

        self.initial_population_creator = ImplicitInitialPopulationCreator(self.model_class, self.parameters)

        if self.functionals.multicriteria():
            self.selector = MultiCriteriaSelector(self.functionals)
        else:
            self.selector = SingleCriteriaSelector(self.functionals)

        self.mutation_creator = ImplicitMutation(self.parameters)
        self.crossover_creator = ImplicitCrossover()

    @property
    def population_size(self):
        """Return number of genoms in population."""
        return self._population_size

    @population_size.setter
    def population_size(self, value):
        self._population_size = value
        self.selector.recomended_population_size = value

    def find_best(self, population):
        direction = self.functionals.functional().direction_sign()
        optimum = direction * 1e99

        for genom in population:
            score = self.functionals.evaluate(genom)
            if direction * score < direction * optimum:
                optimum = score
                optimal_parameters = genom.parameters

        return optimum, optimal_parameters

    def random_member(self, population):
        """Return random genom of the population. Takes into account its priority."""
        genoms = []
        for genom in population:
            genoms += [genom] * GeneticInfo.priority(genom)

        return rnd.choice(genoms)

    def population(self, index):
        """Find and return population (list of models) by index.
        
        population(index)
        
        Keyword arguments:
        index -- population index
        """
        population = []
        for model in self.model_dict.models:
            if GeneticInfo.population_to(model) == index:
                population.append(model)

        return population

    def selection(self):
        """ Return list of selected genoms from previous population."""

        population = self.population(self.current_population_index - 1)
        selected = self.selector.select(population)
        #print('Number of selected genoms: {0}/{1} (previous/selected)'.format(len(population), len(selected)))

        return selected

    def crossover(self, population, number):
        """Return crossbreeds (list of crossovered models).
        
        crossover(population)
        
        Keyword arguments:
        population -- population for crossover
        """

        crossbreeds = []
        attempts = 0
        while len(crossbreeds) < number:
            father = self.random_member(population)
            mother = self.random_member(population)

            while (population.index(mother) == population.index(father)):
                mother = self.random_member(population)

            son = self.crossover_creator.cross(mother, father)
            son.solved = False
            crossbreeds.append(son)

            attempts += 1
            if (attempts > 5 * self.population_size):
                print("Unable to create enough new crossovers. Population may have degenerated.")
                break

        return crossbreeds

    def mutation(self, population, number):
        """Return mutants (list of mutated models).
        
        mutation(population)
        
        Keyword arguments:
        population -- population for mutation
        """

        mutants = []
        while len(mutants) < number:
            original = self.random_member(population)
            mutant = self.mutation_creator.mutate(original)
            mutant.solved = False
            mutants.append(mutant)

        return mutants

    def create_population(self):
        """Create new population and store in ModelDict."""

        if (self.current_population_index != 0):
            population = self.selection()
            population += self.crossover(population, max(self.population_size - len(population) - len(population),
                                                         self.population_size/4))
            population += self.mutation(population, max((self.population_size - len(population)) / 2,
                                                        self.population_size / 5))
        else:
            population = self.initial_population_creator.create(self.population_size)

        for genom in population:
            GeneticInfo.set_population_from(genom, self.current_population_index)
            GeneticInfo.set_population_to(genom, self.current_population_index)

            self.model_dict.add_model(genom)

    def run(self, populations, resume=True):
        """Run optimization.

        run(populations, resume=True)

        Keyword arguments:
        populations -- number of computed populations
        resume -- continue optimization from last population (default is True)
        """

        if resume:
            # TODO: Set self.current_population_index!
            pass

        for index in range(self.current_population_index, populations):
            self.current_population_index = index
            self.create_population()
            self.model_dict.solve(save=False)

            #self.model_dict.save()
            #print(self.find_best(self.model_dict.solved_models))
