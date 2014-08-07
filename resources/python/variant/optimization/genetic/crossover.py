from copy import deepcopy
import random as rnd

class GeneralCrossover:
    """General class for crossover creator."""

    def cross(self, mother, father):
        """Return crossed genom from parents.
        
        cross(mother, father)
        
        Keyword arguments:
        mother -- mother genom
        father -- father genom
        """

        pass

class ImplicitCrossover(GeneralCrossover):
    """Crossover generator, that cross randomly chosed genes."""

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
