from copy import deepcopy
import random as rnd

class GeneralCrossover:
    """General class for definition of crossovers."""

    def cross(self, father, mother):
        pass

class ImplicitCrossover(GeneralCrossover):
    """Each gene is randomly chosen from father or mother."""

    def cross(self, father, mother):
        son = deepcopy(mother)
        for name in son.parameters.keys():
            son.parameters[name] = rnd.choice([mother.parameters[name],
                                               father.parameters[name]])

        return son
