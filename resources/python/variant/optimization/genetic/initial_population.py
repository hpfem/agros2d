from variant.optimization import *

class InitialPopulationCreator:
    """General class for initial population creator."""

    def __init__(self, parameters, model_class):
        """Initialization of population creator.
        
        InitialPopulationCreator(parameters, model_class)
        
        Keyword arguments:
        parameters -- optimization parameters
        model_class -- model class inherited from ModelBase class (default is ModelBase)
        """

        self.parameters = parameters
        self.model_class = model_class

    def create(self, number):
        """Return new population as list of genoms (models).
        
        create(number)
        
        Keyword arguments:
        number -- number of genoms in population
        """
        pass

class ImplicitInitialPopulationCreator(InitialPopulationCreator):
    """Implicitly used initial population creator. All optimizated parameters are taken randomly."""

    def __init__(self, parameters, model_class):
        InitialPopulationCreator.__init__(self, parameters, model_class)

    def create(self, number):
        population = []
        for index in range(number):
            genom = self.model_class()
            for parameter in self.parameters.parameters:
                genom.parameters[parameter.name] = parameter.random_value()

            population.append(genom)

        return population
