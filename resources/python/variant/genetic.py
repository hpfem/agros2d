from optimization import OptimizationMethod, ContinuousParameter
from model_set_manager import ModelSetManager
from genetic_elements import ImplicitInitialPopulationCreator, SingleCriteriaSelector, ImplicitMutation,\
        RandomCrossover
import random as rnd


class GeneticOptimization(OptimizationMethod):
    def __init__(self, parameters, direction):
        OptimizationMethod.__init__(self, parameters, direction)
        self.modelSetManager = ModelSetManager()
        self.initialPopulationCreator = ImplicitInitialPopulationCreator(self.parameters)
        self.selector = SingleCriteriaSelector(self.parameters, self.directionToSigns())
        self.crossover = RandomCrossover()
        self.selector.direction = direction
        self.mutation = ImplicitMutation(self.parameters)

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
    
    def findBest(self, population):
        signF = self.directionToSigns()    
        optimum = signF * 1e50
        for member in population:
            if signF * member.functional < signF * optimum:
                optimum = member.functional
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
            popul = solution.getPopulationTo()
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
                if solution.getPopulationTo() == lastPopulationIdx:
                    self.LastPopulation.append(solution)
        
        return lastPopulationIdx
        
    def oneStep(self):
        print "starting step ", self.populationIdx
        models = self.modelSetManager.loadAll()        
        lastPopulation = []
        for model in models:
            assert model.getPopulationTo() < self.populationIdx
            if model.getPopulationTo() == self.populationIdx - 1:
                lastPopulation.append(model)
                print "pop before select: ", model.getPopulationFrom(), ", ", model.getPopulationTo(), ", ", model.functional
 
        self.selector.recomendedPopulationSize = self.populationSize
        population = self.selector.select(lastPopulation)
        
        for model in population:
            print "pop after select: ", model.getPopulationFrom(), ", ", model.getPopulationTo(), ", ", model.functional


        print "best member of the population: ", self.findBest(population)
                        
        # Mutations
        numMutations = (self.populationSize - len(population)) / 2
        mutations = []
        for i in range(numMutations):
            originalIdx = rnd.randrange(len(population))
            mutation = self.mutation.mutate(population[originalIdx])
            mutation.setPopulationFrom(self.populationIdx)
            mutation.setPopulationTo(self.populationIdx)
            mutations.append(mutation)
            
        # Crossovers
        numCrossovers = self.populationSize - len(population) - len(mutations)
        crossovers = []
        for i in range(numCrossovers):
            fatherIdx = rnd.randrange(len(population))
            motherIdx = rnd.randrange(len(population))
            crossover = self.crossover.cross(population[fatherIdx], population[motherIdx])
            crossover.setPopulationFrom(self.populationIdx)
            crossover.setPopulationTo(self.populationIdx)
            crossovers.append(crossover)
            
        population.extend(mutations)
        population.extend(crossovers)
            
                
        for model in population:
            print "pop after mutations: ", model.getPopulationFrom(), ", ", model.getPopulationTo()#, ", ", model.functional        

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
    print "fdfdf"
    parameters = [ContinuousParameter('a', 0,10),
                  ContinuousParameter('b', 0,10),
                  ContinuousParameter('c', 0,10),
                  ContinuousParameter('d', 0,10),
                  ContinuousParameter('e', 0,10)]
    
    optimization = GeneticOptimization(parameters, "max")
    optimization.directory = '/home/pkus/sources/agros2d/resources/python/variant/test_genetic/solutions/'
    optimization.modelSetManager.solver = '/home/pkus/sources/agros2d/agros2d_solver'
    optimization.populationSize = 25
    optimization.run(20, False)                