from variant.model import ModelBase
from variant.optimization import ContinuousParameter, DiscreteParameter

import random as rnd
from copy import deepcopy

class GeneticInfo:
    @staticmethod
    def population_from(genom):
        return int(genom.info["_population_from"])

    @staticmethod
    def set_population_from(genom, value):
        genom.info["_population_from"] = value

    @staticmethod
    def population_to(genom):
        return int(genom.info["_population_to"])

    @staticmethod
    def set_population_to(genom, value):
        genom.info["_population_to"] = value

    @staticmethod
    def priority(genom):
        return int(genom.info["_priority"])

    @staticmethod
    def set_priority(genom, value):
        genom.info["_priority"] = value

class InitialPopulationCreator:
    def __init__(self, model_class, parameters):
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

    def __init__(self, model_class, parameters):
        InitialPopulationCreator.__init__(self, model_class, parameters)

    def create(self, number):
        population = []
        for index in range(number):
            genom = self.model_class()
            for parameter in self.parameters:
                genom.parameters[parameter.name] = parameter.random_value()

            population.append(genom)

        return population


class SurvivorsSelector:
    """General class for selection of genoms that should be kept into the new population."""

    def __init__(self, parameters, functionals):
        self.parameters = parameters
        self.functionals = functionals

        self.recomended_population_size = 0

    def select(population):
        pass

class SingleCriteriaSelector(SurvivorsSelector):
    """Selector of genoms, that should be kept to next generation in the case of single criteria optimization."""

    def select(self, population):
        assert not self.functionals.multicriteria()
        signF = self.functionals.functional().direction_sign()

        print('0.5*self.recomended_population_size: {0}'.format(int(0.5 * self.recomended_population_size)))

        survivorsNum = min(len(population), int(0.5*self.recomended_population_size))

        scores = []
        for genom in population:
            scores.append(self.functionals.evaluate(genom))
        scores.sort(reverse=bool(signF == 1))

        priority_tresholds = [scores[survivorsNum-1], scores[int(survivorsNum*0.8)-1], scores[int(survivorsNum*0.5)-1]]

        survivors = []
        for genom in population:
            new_genom = deepcopy(genom)
            score = self.functionals.evaluate(genom)

            priority = 0
            for index in range(len(priority_tresholds)):
                if signF * score <= signF * priority_tresholds[index]:
                    priority = index + 1

            if priority > 0:
                GeneticInfo.set_priority(new_genom, priority)
                GeneticInfo.set_population_to(new_genom, GeneticInfo.population_to(new_genom) + 1)
                survivors.append(new_genom)

        return survivors

class MultiCriteriaSelector(SurvivorsSelector):

    def is_front(self, index, population, direction):
        [sign_f, sign_g] = direction
        member = population[index]
        [val_f, val_g] = self.functionals.evaluate(member)
        
        for i in range(len(population)):
            test_member = population[i]
            [test_f, test_g] = self.functionals.evaluate(test_member)
            
            if sign_f * test_f <  sign_f * val_f and sign_g * test_g <= sign_g * val_g:
                return False
            
            if sign_f * test_f <= sign_f * val_f and sign_g * test_g <  sign_g * val_g:
                return False
                
        return True

    def set_priority(self, member, priority):
        if (priority > member.priority):
            member.priority = priority

    def select(self, population):
        assert self.functionals.isMulticriterial()
        sign_f = self.functionals.functional(0).directionSign()
        sign_g = self.functionals.functional(1).directionSign()
        direction = [sign_f, sign_g]
        
        #create deep copy of population and assign implicit priority (with how big probability we should select this element to mutations or crossings 1..3, 3 highest)
        population_copy = []
        for member in population:
            newMember = deepcopy(member)
            newMember.priority = 1            
            population_copy.append(newMember)
        
        # what should remain to next step? Indices to the population array
        include = set()
        last_pop_size = len(population)
        
        #calculate average values of each functional for norming
        avg_f = 0
        avg_g = 0
        for member in population_copy:
            [val_f, val_g] = self.functionals.evaluate(member)
            avg_f += val_f
            avg_g += val_g
        avg_f /= last_pop_size
        avg_g /= last_pop_size
        
        # contains triples [fval, gval, index], to be sorted
        front = []
        for i in range(last_pop_size):
            if self.is_front(i, population_copy, direction):
                [val_f, val_g] = self.functionals.evaluate(population[i])
                front.append([val_f, val_g, i])
        
        #sort front according to first functional value
        front.sort()
        
        #find largest distances between neighbouring individuals on the front
        distances = []
        for i in range(len(front) - 1):
            distances.append([((front[i+1][0] - front[i][0])**2 + (front[i+1][1] - front[i][1])**2)**0.5, i])
        distances.sort()
        
        for i in range(len(front)):
            include.add(front[i][2])
            
        front_len = len(include)
        
        #on both sides of the front increase priorities
        front_ends_indices = [0, 1, 2, 3, len(front)-4, len(front)-3, len(front)-2, len(front)-1]
        front_ends_priorities = [3, 3, 2, 2, 2, 2, 3, 3]
        
        i = 0
        while (i < len(front_ends_indices)):
            if (front_ends_indices[i] not in range(front_len)):
                front_ends_indices.pop(i)
                front_ends_priorities.pop(i)
            else:
                i += 1
                
        for i in range(len(front_ends_indices)):
            front_member_idx = front[front_ends_indices[i]][2]            
            self.set_priority(population_copy[front_member_idx], front_ends_priorities[i])
                
        #high priority for several largest holes        
        if (front_len > 3):
            for num_hole in range(3):
                index_front = distances[-1-num_hole][1]
                self.set_priority(population_copy[index_front-1], 2)
                self.set_priority(population_copy[index_front], 3)
                self.set_priority(population_copy[index_front+1], 3)
                self.set_priority(population_copy[index_front+2], 2)
        
            
        ##debug print:
        #print front
        #print include
        #print distances
        #priority_print = []
        #for i in range(len(front)):
            #priority_print.append(priority[front[i][2]])
        #print priority_print
        
            
        # for each linear combination of two functionals keep several best 
        ratios = [0.01, 0.03, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.97, 0.99]
        
        # to store pairs [ratio, values], where values is list of [value, index] pair related to given ratio
        values_ratios = []
        for ratio in ratios:
            values = []
            # for each ratio go over all population and determine the distance from line given by the ratio
            for i in range(last_pop_size):
                member = population_copy[i]
                [val_f, val_g] = self.functionals.evaluate(member)

                # this is the quality with respect to the ratio (line direction)
                value = ratio * sign_f * val_f / avg_f + (1-ratio) * sign_g * val_g / avg_g
                
                # add value and index, so that the array may be sorted according to value (quality)
                values.append([value, i])
            values.sort()
            values_ratios.append(values)
        
        level = 0
        while (len(include) < 0.6 * self.recomended_population_size or len(include) - front_len < 0.25 * self.recomended_population_size):
            for ratio_idx in range(len(ratios)):
                candidate_idx = values_ratios[ratio_idx][level][1]
                
                # it will often hapen, that candidate_idx is allready there. It is ok, since include is set
                include.add(candidate_idx)
                
                # the best with higher priority    
                if (level < 3):
                    priority = 2
                else:
                    priority = 1
                    
                # set priority (if allready set, this function makes maximum)
                self.set_priority(population_copy[candidate_idx], priority)
            level += 1
            if (level > 0.9*last_pop_size):
                break
                
        survivors = []
        for i in range(last_pop_size):
            if i in include:
                survivors.append(population_copy[i])        
                
        return survivors
    


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

class GeneralCrossover:
    """General class for definition of crossovers."""

    def cross(self, father, mother):
        pass

class RandomCrossover(GeneralCrossover):
    """Each gene randomly chosen from father or mother."""

    def cross(self, father, mother):
        son = deepcopy(mother)
        for name in son.parameters.keys():
            son.parameters[name] = rnd.choice([mother.parameters[name],
                                               father.parameters[name]])

        return son

if __name__ == '__main__':
    parameters = [ContinuousParameter("A", -5, -1),
                  ContinuousParameter("B", 6, 8),
                  DiscreteParameter("C", [1,3,6,88,2])]

    population_creator = ImplicitInitialPopulationCreator(ModelBase, parameters)
    population = population_creator.create(5)

    mutation_creator = ImplicitMutation(parameters)
    for genom in population:
        mutant = mutation_creator.mutate(genom)
        print("original: {0}".format(genom.parameters))
        print("mutant: {0}".format(genom.parameters))

    crossover_creator = RandomCrossover()
    print("father: {0}".format(population[0].parameters))
    print("mother: {0}".format(population[1].parameters))
    print("son: {0}".format(crossover_creator.cross(population[0], population[1]).parameters))
    print("son: {0}".format(crossover_creator.cross(population[0], population[1]).parameters))
    print("son: {0}".format(crossover_creator.cross(population[0], population[1]).parameters))
