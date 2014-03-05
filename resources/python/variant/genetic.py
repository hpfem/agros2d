from optimization import OptimizationMethod, ContinuousParameter
from model_set_manager import ModelSetManager
from genetic_elements import ImplicitInitialPopulationCreator, SingleCriteriaSelector, ImplicitMutation,\
        RandomCrossover
import random as rnd


class GeneticOptimization(OptimizationMethod):
    def __init__(self, parameters, direction):
        OptimizationMethod.__init__(self, parameters)
        self.modelSetManager = ModelSetManager()
        self.initialPopulationCreator = ImplicitInitialPopulationCreator(self.parameters)
        self.selector = SingleCriteriaSelector(self.parameters)
        self.crossover = RandomCrossover()
        self.direction = direction
        self.selector.direction = direction

    @property
    def populationSize(self):
        """Parameters"""
        return self._populationSize
        
    @populationSize.setter
    def parameters(self, value):
        print "triyng to set ", value
        self._populationSize = value
        
        # why does not work from here? Has to be set again in oneStep()
        self.selector.recomendedPopulationSize = value
                
    
    def initialStep(self, resume):
        # if not resume previous optimization, delete all solution files in the directory
        if not resume:
            self.modelSetManager.deleteAll()
            
        # read all the solution files in the directory
        solutions = self.modelSetManager.loadAll()
        
        # find what is the latest present population
        lastPopulationIdx = -1
        solutionsWithPopulNum = []
        for solution in solutions:
            popul = solution.populationTo
            if popul >= 0:
                solutionsWithPopulNum.append(solution)
            else:
                print "Solution does not contain population_to key"
            
            lastPopulationIdx = max(lastPopulationIdx, popul)
            
        if lastPopulationIdx == -1:                    
            # no previous population found, create initial one
            self.lastPopulation = self.initialPopulationCreator.create(self.populationSize)
            self.modelSetManager.saveAll(self.lastPopulation)
            lastPopulationIdx = 0
        else:
            self.LastPopulation = []
            for solution in solutionsWithPopulNum:
                if solution.populationTo == lastPopulationIdx:
                    self.LastPopulation.append(solution)
        
        return lastPopulationIdx
        
    def oneStep(self):
        solved = self.modelSetManager.solveAll()
        models = self.modelSetManager.loadAll()        
        lastPopulation = []
        for model in models:
            print model.populationTo, ", ", self.populationIdx
            assert model.populationTo < self.populationIdx
            if model.populationTo == self.populationIdx - 1:
                lastPopulation.append(model)
              
        self.selector.recomendedPopulationSize = self.populationSize
        population = self.selector.select(lastPopulation)
        
        print "solved {0}, transfered to new population {1}".format(solved, len(population))
            
        self.modelSetManager.saveAll(population)
                
    def run(self, maxIters, resume = True):
        self.modelSetManager.directory = self.directory

        lastPopulationIdx = self.initialStep(resume)        
                
        for self.populationIdx in range(lastPopulationIdx + 1, maxIters):
            self.oneStep()

if __name__ == '__main__':
    parameters = [ContinuousParameter('a', 0,10),
                  ContinuousParameter('b', 0,10),
                  ContinuousParameter('c', 0,10),
                  ContinuousParameter('d', 0,10),
                  ContinuousParameter('e', 0,10)]
    
    optimization = GeneticOptimization(parameters, "min")
    optimization.directory = '/home/pkus/sources/agros2d/resources/python/variant/test_genetic/solutions/'
    optimization.modelSetManager.solver = '/home/pkus/sources/agros2d/agros2d_solver'
    optimization.populationSize = 8
    optimization.run(3, False)                