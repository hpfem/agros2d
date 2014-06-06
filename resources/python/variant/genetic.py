from variant.optimization import OptimizationMethod, ContinuousParameter, Functionals, Functional
from variant.model_set_manager import ModelSetManager
from variant.genetic_elements import ImplicitInitialPopulationCreator, SingleCriteriaSelector, MultiCriteriaSelector, ImplicitMutation,\
        RandomCrossover, GeneticInfo
import random as rnd

import pythonlab


class GeneticOptimization(OptimizationMethod):
    def __init__(self, parameters, functionals):
        OptimizationMethod.__init__(self, parameters, functionals)
        self.modelSetManager = ModelSetManager()
        self.initialPopulationCreator = ImplicitInitialPopulationCreator(self.parameters)
        if functionals.isMulticriterial():
            self.selector = MultiCriteriaSelector(self.parameters, self.functionals)
        else:
            self.selector = SingleCriteriaSelector(self.parameters, self.functionals)
        self.crossover = RandomCrossover()
        self.mutation = ImplicitMutation(self.parameters)

    @property
    def population_size(self):
        """Parameters"""
        return self._population_size

    @population_size.setter
    def population_size(self, value):
        self._population_size = value

        # why does not work from here? Has to be set again in oneStep()
        self.selector.recomended_population_size = value

    def findBest(self, population):
        signF = self.functionals.functional().directionSign()
        optimum = signF * 1e50
        for member in population:
            if signF * self.functionals.evaluate(member) < signF * optimum:
                optimum = self.functionals.evaluate(member)
                optimalParameters = member.parameters

        return [optimum, optimalParameters]

    
    # random number of the population
    # takes into account its priority
    def randomMemberIdx(self, population):
        indices = []
        for index in range(len(population)):
            for j in range(population[index].priority):
                indices.append(index);
                
        return indices[rnd.randrange(len(indices))]


    def initialStep(self, resume):
        print("initial step")
        # if not resume previous optimization, delete all solution files in the directory
        if not resume:
            self.modelSetManager.delete_all()

        # read all the solution files in the directory
        solutions = self.modelSetManager.load_all()

        # find what is the latest present population
        lastPopulationIdx = -1
        solutionsWithPopulNum = []
        for solution in solutions:
            popul = GeneticInfo.population_to(solution)
            if popul >= 0:
                solutionsWithPopulNum.append(solution)
            else:
                print("Solution does not contain population_to key")

            lastPopulationIdx = max(lastPopulationIdx, popul)

        if lastPopulationIdx == -1:
            # no previous population found, create initial one
            print("no previous population found, create initial one")
            self.lastPopulation = self.initialPopulationCreator.create(self.population_size)
            self.modelSetManager.save_all(self.lastPopulation)
            lastPopulationIdx = 0
        else:
            self.LastPopulation = []
            for solution in solutionsWithPopulNum:
                if GeneticInfo.population_to(solution) == lastPopulationIdx:
                    self.LastPopulation.append(solution)

        return lastPopulationIdx

    def oneStep(self):
        print("starting step ", self.populationIdx)
        models = self.modelSetManager.load_all()
        lastPopulation = []
        for model in models:
            assert GeneticInfo.population_to(model) < self.populationIdx
            if GeneticInfo.population_to(model) == self.populationIdx - 1:
                lastPopulation.append(model)
                print("pop before select: ", GeneticInfo.population_from(model), ", ", GeneticInfo.population_to(model), ", ", self.functionals.evaluate(model))

        self.selector.recomended_population_size = self.population_size
        population = self.selector.select(lastPopulation)

        for model in population:
            GeneticInfo.set_population_to(model, self.populationIdx)
            print("pop after select: ", GeneticInfo.population_from(model), ", ", GeneticInfo.population_to(model), ", ", self.functionals.evaluate(model), " prior: ", model.priority)


        #print "best member of the population: ", self.findBest(population)

        # Mutations
        numMutations = (self.population_size - len(population)) / 2
        # at least 1/5 of recomended population size
        numMutations = max(numMutations, self.population_size / 5)
        mutations = []
        while len(mutations) < numMutations:
            originalIdx = self.randomMemberIdx(population)
            mutation = self.mutation.mutate(population[originalIdx])
            GeneticInfo.set_population_from(mutation, self.populationIdx)
            GeneticInfo.set_population_to(mutation, self.populationIdx)
            if (not self.isContained(population, mutation)) and (not self.isContained(mutations, mutation)):                
                mutations.append(mutation)
            
        # Crossovers
        numCrossovers = self.population_size - len(population) - len(mutations)
        numCrossovers = max(numCrossovers, self.population_size / 4)
        crossovers = []
        attempts = 0
        while len(crossovers) < numCrossovers:
            fatherIdx = self.randomMemberIdx(population)
            motherIdx = self.randomMemberIdx(population)
            while (motherIdx == fatherIdx):
                motherIdx = self.randomMemberIdx(population)
            
            #print "fat and mat ", [fatherIdx, motherIdx]
            crossover = self.crossover.cross(population[fatherIdx], population[motherIdx])
            GeneticInfo.set_population_from(crossover, self.populationIdx)
            GeneticInfo.set_population_to(crossover, self.populationIdx)
            if (not self.isContained(population, crossover)) and (not self.isContained(mutations, crossover)) and (not self.isContained(crossovers, crossover)):                
                crossovers.append(crossover)
            attempts += 1
            if (attempts > 5 * self.population_size):
                print("Unable to create enough new crossovers. Population may have degenerated.")
                break
        
            
        print("in step {0} survived {1}, added {2} mutations and {3} crossovers".format(self.populationIdx, len(population), len(mutations), len(crossovers)))
                
        population.extend(mutations)
        population.extend(crossovers)

        for model in population:
            print("pop after mutations: ", GeneticInfo.population_from(model), ", ", GeneticInfo.population_to(model)) #, ", ", model.functional)

        self.modelSetManager.save_all(population)

    def run(self, maxIters, resume = True):
        self.modelSetManager.directory = self.directory

        lastPopulationIdx = self.initialStep(resume)
        self.modelSetManager.solve_all()

        for self.populationIdx in range(lastPopulationIdx + 1, maxIters):
            self.oneStep()
            solved = self.modelSetManager.solve_all()
            print("solved {0} ".format(solved))


if __name__ == '__main__':
    parameters = [ContinuousParameter('a', 0, 10),
                    ContinuousParameter('b', 0, 10),
                    ContinuousParameter('c', 0, 10),
                    ContinuousParameter('d', 0, 10),
                    ContinuousParameter('e', 0, 10)]

    functionals = Functionals([Functional("Func1", "max")])

    self_optimization = GeneticOptimization(parameters, functionals)
    self_optimization.directory = pythonlab.datadir('/resources/test/test_suite/optilab/genetic/solutions/')
    self_optimization.modelSetManager.solver = pythonlab.datadir('agros2d_solver')
    self_optimization.population_size = 15
    self_optimization.run(15, False)
