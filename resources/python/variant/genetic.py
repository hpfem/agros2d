from optimization import OptimizationMethod, ContinuousParameter
from model_set_manager import ModelSetManager
from genetic_elements import ImplicitInitialPopulationCreator, SingleCriteriaSelector, ImplicitMutation,\
        RandomCrossover
import random as rnd


class GeneticOptimization(OptimizationMethod):
    def __init__(self, parameters):
        OptimizationMethod.__init__(self, parameters)
        self.modelSetManager = ModelSetManager()
        self.initialPopulationCreator = ImplicitInitialPopulationCreator(self.parameters)
        self.selector = SingleCriteriaSelector(self.parameters)
        self.crossover = RandomCrossover()

    @property
    def populationSize(self):
        """Parameters"""
        return self._populationSize
        
    @populationSize.setter
    def parameters(self, value):
        self._populationSize = value
                
    def run(self, maxIters):
        self.modelSetManager.directory = self.directory
        
        population = self.initialPopulationCreator.create(self.populationSize)
        self.modelSetManager.saveAll(population)
        

if __name__ == '__main__':
    parameters = [ContinuousParameter('a', 0,10),
                  ContinuousParameter('b', 0,10),
                  ContinuousParameter('c', 0,10),
                  ContinuousParameter('d', 0,10),
                  ContinuousParameter('e', 0,10)]
    
    optimization = GeneticOptimization(parameters)
    optimization.directory = 'test/'
    optimization.populationSize = 10
    optimization.run(20)                