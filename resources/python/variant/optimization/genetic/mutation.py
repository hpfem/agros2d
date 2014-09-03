from copy import deepcopy
import random as rnd

class MutationCreator:
    """General class for mutation creator."""

    def __init__(self, parameters):
        """Initialization of mutation creator.
        
        MutationCreator(parameters)
        
        Keyword arguments:
        parameters -- optimized parameters
        """

        self.parameters = parameters

    def mutate(self, original):
        """Return mutant of original genom.

        mutate(original)

        Keyword arguments:
        original -- original genom for mutation
        """

        pass

class GeneralMutation(MutationCreator):
    """General mutation creator. User can select how many parameters is changed and whether they are large or small changes."""

    def __init__(self, parameters, strength):
        """Initialization of mutation creator.

        GeneralMutation(parameters, strength)

        Keyword arguments:
        parameters -- optimized parameters
        strength -- strength of mutation (strength == 1: use any random value from bounds, strength < 1: use parameter perturbation)
        """

        MutationCreator.__init__(self, parameters)
        self.strength = strength

    def mutate(self, original):
        mutant = deepcopy(original)
        #assert (len(mutant.parameters) == len(self.parameters.parameters))

        parameter_name = rnd.choice(list(mutant.parameters.keys()))
        changed_parameter = self.parameters.parameter(parameter_name)
        if (self.strength == 1):
            mutant.parameters[parameter_name] = changed_parameter.random_value()
        elif (self.strength < 1) and (self.strength > 0):
            mutant.parameters[parameter_name] = changed_parameter.perturbation(mutant.parameters[parameter_name], self.strength)
        else:
            pass

        return mutant

class ImplicitMutation(MutationCreator):
    """Implicit mutation, randomly select one of standard types of mutations."""

    def __init__(self, parameters):
        MutationCreator.__init__(self, parameters)
        self.strengths = [1.0, 0.25, 0.125, 0.1, 0.05, 0.025, 0.0125]

    def mutate(self, original):
        creator = GeneralMutation(self.parameters, rnd.choice(self.strengths))
        return creator.mutate(original)
