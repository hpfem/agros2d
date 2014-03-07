from optimization import OptimizationMethod, ContinuousParameter, Functionals, Functional
from model_set_manager import ModelSetManager
from genetic_elements import ImplicitInitialPopulationCreator, SingleCriteriaSelector, ImplicitMutation,\
        RandomCrossover, GeneticInfo
import random as rnd

import pythonlab


class GeneticOptimization(OptimizationMethod):
    def __init__(self, parameters, functionals):
        OptimizationMethod.__init__(self, parameters, functionals)
        self.modelSetManager = ModelSetManager()
        self.initialPopulationCreator = ImplicitInitialPopulationCreator(self.parameters)
        self.selector = SingleCriteriaSelector(self.parameters, self.functionals)
        self.crossover = RandomCrossover()
        self.mutation = ImplicitMutation(self.parameters)

    @property
    def populationSize(self):
        """Parameters"""
        return self._populationSize

    @populationSize.setter
    def populationSize(self, value):
        self._populationSize = value

        # why does not work from here? Has to be set again in oneStep()
        self.selector.recomendedPopulationSize = value

    def findBest(self, population):
        signF = self.functionals.functional().directionSign()
        optimum = signF * 1e50
        for member in population:
            if signF * self.functionals.evaluate(member) < signF * optimum:
                optimum = self.functionals.evaluate(member)
                optimalParameters = member.parameters

        return [optimum, optimalParameters]


    def initialStep(self, resume):
        print "initial step"
        # if not resume previous optimization, delete all solution files in the directory
        if not resume:
            self.modelSetManager.deleteAll()

        # read all the solution files in the directory
        solutions = self.modelSetManager.loadAll()

        # find what is the latest present population
        lastPopulationIdx = -1
        solutionsWithPopulNum = []
        for solution in solutions:
            popul = GeneticInfo.populationTo(solution)
            if popul >= 0:
                solutionsWithPopulNum.append(solution)
            else:
                print "Solution does not contain population_to key"

            lastPopulationIdx = max(lastPopulationIdx, popul)

        if lastPopulationIdx == -1:
            # no previous population found, create initial one
            print "no previous population found, create initial one"
            self.lastPopulation = self.initialPopulationCreator.create(self.populationSize)
            self.modelSetManager.saveAll(self.lastPopulation)
            lastPopulationIdx = 0
        else:
            self.LastPopulation = []
            for solution in solutionsWithPopulNum:
                if GeneticInfo.populationTo(solution) == lastPopulationIdx:
                    self.LastPopulation.append(solution)

        return lastPopulationIdx

    def oneStep(self):
        print "starting step ", self.populationIdx
        models = self.modelSetManager.loadAll()
        lastPopulation = []
        for model in models:
            assert GeneticInfo.populationTo(model) < self.populationIdx
            if GeneticInfo.populationTo(model) == self.populationIdx - 1:
                lastPopulation.append(model)
                print "pop before select: ", GeneticInfo.populationFrom(model), ", ", GeneticInfo.populationTo(model), ", ", self.functionals.evaluate(model)

        self.selector.recomendedPopulationSize = self.populationSize
        population = self.selector.select(lastPopulation)

        for model in population:
            print "pop after select: ", GeneticInfo.populationFrom(model), ", ", GeneticInfo.populationTo(model), ", ", self.functionals.evaluate(model)


        print "best member of the population: ", self.findBest(population)

        # Mutations
        numMutations = (self.populationSize - len(population)) / 2
        mutations = []
        for i in range(numMutations):
            originalIdx = rnd.randrange(len(population))
            mutation = self.mutation.mutate(population[originalIdx])
            GeneticInfo.setPopulationFrom(mutation, self.populationIdx)
            GeneticInfo.setPopulationTo(mutation, self.populationIdx)
            mutations.append(mutation)

        # Crossovers
        numCrossovers = self.populationSize - len(population) - len(mutations)
        crossovers = []
        for i in range(numCrossovers):
            fatherIdx = rnd.randrange(len(population))
            motherIdx = rnd.randrange(len(population))
            crossover = self.crossover.cross(population[fatherIdx], population[motherIdx])
            GeneticInfo.setPopulationFrom(crossover, self.populationIdx)
            GeneticInfo.setPopulationTo(crossover, self.populationIdx)
            crossovers.append(crossover)

        population.extend(mutations)
        population.extend(crossovers)


        for model in population:
            print "pop after mutations: ", GeneticInfo.populationFrom(model), ", ", GeneticInfo.populationTo(model)#, ", ", model.functional

        self.modelSetManager.saveAll(population)

    def run(self, maxIters, resume = True):
        self.modelSetManager.directory = self.directory

        lastPopulationIdx = self.initialStep(resume)
        self.modelSetManager.solveAll()

        for self.populationIdx in range(lastPopulationIdx + 1, maxIters):
            self.oneStep()
            solved = self.modelSetManager.solveAll()
            print "solved {0} from population of {1}".format(solved, self.populationSize)

if __name__ == '__main__':
    parameters = [ContinuousParameter('a', 0,10),
                  ContinuousParameter('b', 0,10),
                  ContinuousParameter('c', 0,10),
                  ContinuousParameter('d', 0,10),
                  ContinuousParameter('e', 0,10)]

    functionals = Functionals([Functional("Func1", "max")])

    optimization = GeneticOptimization(parameters, functionals)
    optimization.directory = pythonlab.datadir('/resources/python/variant/test_genetic/solutions/')
    optimization.modelSetManager.solver = pythonlab.datadir('agros2d_solver')
    optimization.populationSize = 25
    optimization.run(25, True)                

