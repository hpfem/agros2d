from copy import deepcopy
import random as rnd

class GeneralCrossover:
    """General class for definition of crossovers."""

    def cross(self, father, mother):
        pass

class ImplicitCrossover(GeneralCrossover):
    """Each gene is randomly chosen from father or mother."""

    def cross(self, mother, father):
        son = deepcopy(mother)

        mother_gens = {}
        father_gens = {}
        mother_gens.update(mother.parameters)

        while len(mother_gens) >= len(father_gens):
            key = rnd.choice(list(mother_gens.keys()))
            son.parameters[key] = father.parameters[key]
            father_gens[key] = father.parameters[key]
            del(mother_gens[key])

        return son
