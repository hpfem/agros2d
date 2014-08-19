from variant import ModelBase, ModelDictExternal
from variant.optimization import *

from variant.optimization.genetic.info import GeneticInfo
from variant.optimization.genetic.initial_population import ImplicitInitialPopulationCreator
from variant.optimization.genetic.selector import SingleCriteriaSelector, MultiCriteriaSelector
from variant.optimization.genetic.mutation import ImplicitMutation
from variant.optimization.genetic.crossover import ImplicitCrossover

import random as rnd
import collections

class GeneticOptimization(OptimizationMethod):
    """Genetic optimization method class."""

    def __init__(self, parameters, functionals, model_class=ModelBase):
        """Initialization of method object.
        
        GeneticOptimization(parameters, functionals, model_class=ModelBase)
        
        Keyword arguments:
        parameters -- optimization parameters
        functionals -- functionals
        model_class -- model class inherited from ModelBase class (default is ModelBase)
        """

        OptimizationMethod.__init__(self, parameters, functionals, model_class)

        self.current_population_index = 0
        self.cache = collections.OrderedDict()
        self.selection_ratio = 4.0/5.0
        self.elitism_ratio = 1.0/100.0
        self.crossover_ratio = 1.0
        self.mutation_ratio = 1.0/10.0

        self.initial_population_creator = ImplicitInitialPopulationCreator(self.parameters, self.model_class)

        if self.functionals.multicriteria():
            self.selector = MultiCriteriaSelector(self.functionals, self.model_class)
        else:
            self.selector = SingleCriteriaSelector(self.functionals, self.model_class)

        self.mutation_creator = ImplicitMutation(self.parameters)
        self.crossover_creator = ImplicitCrossover()

    @property
    def population_size(self):
        """Number of genoms in population."""
        return self._population_size

    @population_size.setter
    def population_size(self, value):
        self._population_size = value
        self.selector.recomended_population_size = value

    def duplicity(self, genom, population):
        duplicity = False
        for other in population:
            sameness = True
            for key in genom.parameters.keys():
                if (genom.parameters[key] != other.parameters[key]):
                    sameness = False
                    break

            if sameness:
                duplicity = True
                break

        return duplicity

    def find_best(self, population):
        """Return best genom from population.
        
        find_best(population)
        
        Keyword arguments:
        population -- list of considered models
        """

        if not self.functionals.multicriteria():
            direction = self.functionals.functional().direction_sign()
            best_score = direction * 1e99
            best_genom = None

            evaluate = self.functionals.evaluate
            for genom in population:
                score = evaluate(genom)
                if direction * score < direction * best_score:
                    best_score = score
                    best_genom = genom

            return best_genom

        else:
            #TODO: Multicriteria.
            pass

    def random_member(self, population):
        """Return random genom of the population. Takes into account its priority.

        find_best(population)

        Keyword arguments:
        population -- list of considered models
        """

        priority = GeneticInfo.priority
        indices = []
        for index in range(len(population)):
            indices += [index] * priority(population[index])

        index = indices[rnd.choice(indices)]
        return population[index], index

    def population(self, index = -1):
        """Find and return population (list of models) by index.
        
        population(index=-1)
        
        Keyword arguments:
        index -- population index (default is last population)
        """

        if (index == -1):
            return next(reversed(self.cache))
        else:
            return self.cache[index]
        """
        population = []
        population_to = GeneticInfo.population_to
        for model in self.model_dict.models():
            if population_to(model) == index:
                population.append(model)

        return population
        """

    def _functional_as_key(self, genom):
        return self.functionals.evaluate(genom)

    def sorted_population(self, index=-1):
        """Find and return sorted population (list of models) by index.
        
        population(index=-1)
        
        Keyword arguments:
        index -- population index (default is last population)
        """
        direction = self.functionals.functional().direction_sign()
        population = self.population(index)
        return sorted(population, key=self._functional_as_key, reverse=bool(direction != 1))

    def selection(self, population, selections, elitists):
        direction = self.functionals.functional().direction_sign()
        selected = self.selector.select(population, selections)

        selected = sorted(selected, key=self._functional_as_key, reverse=bool(direction != 1))        
        return selected, selected[0:elitists]

    def crossover(self, population, number):
        """Return crossbreeds (list of crossovered models).
        
        crossover(population, number)
        
        Keyword arguments:
        population -- list of considered models
        number -- number of crossbreeds
        """

        crossbreeds = []
        attempts = 0
        while len(crossbreeds) < number:
            mother, mother_index = self.random_member(population)
            father, father_index = self.random_member(population)

            while mother_index == father_index:
                father, father_index = self.random_member(population)

            son = self.crossover_creator.cross(mother, father)
            # TODO: Duplicity!
            """
            if self.duplicity(son, crossbreeds):
                continue
            """

            son.solved = False
            crossbreeds.append(son)

            attempts += 1
            if (attempts > 5 * self._population_size):
                print("Unable to create enough new crossovers. Population may have degenerated.")
                break

        return crossbreeds

    def mutation(self, population, number):
        """Return population with mutated genoms.
        
        mutation(population, number)
        
        Keyword arguments:
        population -- list of considered models
        number -- number of mutants
        """

        mutants = []
        while len(mutants) < number:
            original, index = self.random_member(population)
            population.pop(index)

            mutant = self.mutation_creator.mutate(original)
            mutant.solved = False
            mutants.append(mutant)

        return population + mutants

    def create_population(self):
        """Create new population and placed it to model dictionary."""

        if (self.current_population_index != 0):
            last_population = self.population()

            # selection
            selections = min(int(self.selection_ratio * self._population_size),
                             len(last_population))
            elitists = max(int(self.elitism_ratio * self._population_size), 1)
            selected, elite = self.selection(last_population, selections, elitists)

            # crossover
            crossbreeds = max(int(self.crossover_ratio * self._population_size),
                              int(self._population_size/2))
            crossed = self.crossover(selected + elite, crossbreeds)

            # mutation
            mutants = int(self.mutation_ratio * self._population_size)
            population = self.mutation(crossed, mutants)

            population += elite
        else:
            population = self.initial_population_creator.create(self._population_size)

        set_population_from = GeneticInfo.set_population_from
        set_population_to = GeneticInfo.set_population_to

        for genom in population:
            set_population_from(genom, self.current_population_index)
            set_population_to(genom, self.current_population_index)

            self.model_dict.add_model(genom)

        self.cache = population

    def run(self, populations, save=True):
        """Run optimization.

        run(populations, resume=True)

        Keyword arguments:
        populations -- number of computed populations
        resume -- continue optimization from last population (default is True)
        """

        #TODO: Resume in optimization!
        for index in range(self.current_population_index, populations):
            self.current_population_index = index
            self.create_population()

            if save or self.model_dict.__class__.__base__ == ModelDictExternal:
                self.model_dict.solve()
            else:
                self.model_dict.solve(save=False)


if __name__ == '__main__':
    from test_suite.optilab.examples import holder_table_function
    parameters = Parameters([ContinuousParameter('x', -10, 10), ContinuousParameter('y', -10, 10)])
    functionals = Functionals([Functional("F", "min")])

    optimization = GeneticOptimization(parameters, functionals,
                                       holder_table_function.HolderTableFunction)

    optimization.population_size = 300
    optimization.run(50, False)
    star = optimization.find_best(optimization.model_dict.models()) 
    print('Minimum F={0} was found with parameters: {1}'.format(star.variables['F'], star.parameters))
    print('Genuine minimum is F=-19.2085')
