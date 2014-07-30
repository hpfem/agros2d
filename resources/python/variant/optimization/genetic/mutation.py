from copy import deepcopy
import random as rnd

class MutationCreator:
    """General class for creation of mutations."""

    def __init__(self, parameters):
        self.parameters = parameters

    def mutate(self, original):
        pass

class GeneralMutation(MutationCreator):
    """General mutation, user can select how many parameters is changed and whether they are large or small changes."""

    def __init__(self, parameters, number, strength):
        """ If mutationStrength == 1, use any random value from bounds.
            If mutationStrength < 1, change in parameter at most mutationStrength * (bound_max - bound_min)."""
        MutationCreator.__init__(self, parameters)
        self.number = number
        self.strength = strength

    def mutate(self, original):
        mutant = deepcopy(original)
        assert (len(mutant.parameters) == len(self.parameters))

        parameters_names = list(mutant.parameters.keys())
        mutated_parameters = []
        for mutation in range(self.number):
            name = rnd.choice(parameters_names)
            while name in mutated_parameters:
                name = rnd.choice(parameters_names)

            index = 0
            for parameter in self.parameters:
                if (parameter.name == name):
                    index = self.parameters.index(parameter)

            if (self.strength == 1):
                mutant.parameters[name] = self.parameters[index].random_value()
            elif (self.strength < 1) and (self.strength > 0):
                mutant.parameters[name] = self.parameters[index].perturbation(mutant.parameters[name], self.strength)
            else:
                pass

            mutated_parameters.append(name)

        return mutant

class ImplicitMutation(MutationCreator):
    """Implicit mutation, randomly select one of standard types of mutations."""

    def __init__(self, parameters):
        MutationCreator.__init__(self, parameters)
        self.mutation_creators = [GeneralMutation(self.parameters, 1, 1.0),
                                  GeneralMutation(self.parameters, 2, 1.0),
                                  GeneralMutation(self.parameters, 1, 0.25),
                                  GeneralMutation(self.parameters, 1, 0.1),
                                  GeneralMutation(self.parameters, 2, 0.1)]


    def mutate(self, original):
        creator = rnd.choice(self.mutation_creators)
        return creator.mutate(original)
