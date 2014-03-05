from model import ModelBase
from optimization import directionToSigns, ContinuousParameter, DiscreteParameter
import random as rnd
from copy import deepcopy

class InitialPopulationCreator:
    """
        General class for initial population creation
    """

    def __init__(self, bounds):
        self.bounds = bounds
        
    def create(self, number):
        pass
        
    def markInitialMember(self, member):
        member.setPopulationFrom(0)
        member.setPopulationTo(0)   
    
class ImplicitInitialPopulationCreator(InitialPopulationCreator):
    """
        Implicitly used initial population creator - all parameters taken randomly
    """

    def __init__(self, bounds):
        InitialPopulationCreator.__init__(self, bounds)
        
    def create(self, number):
        population = []
        for i in range(number):
            member = ModelBase()
            parameters = dict()
            for bound in self.bounds:
                parameters[bound.name] = bound.randomValue()        
            
            member.parameters = parameters
            self.markInitialMember(member)
            population.append(member)
            
        return population                    
        

class SurvivorsSelector:
    """
        General class for selection of genoms that should be kept into the new population
    """

    def __init__(self, bounds):
        self.bounds = bounds
        
    @property
    def recomendedPopulationSize(self):
        """Recomended population size. Actual population size may differ, but should be close to this"""
        return self._recomendedPopulationSize

    @recomendedPopulationSize.setter
    def recomendedPopulationSize(self, value):
        print "setting ", value
        self._recomendedPopulationSize = value

    @property
    def direction(self):
        """min for minimization, max for maximization"""
        return self._direction

    @direction.setter
    def direction(self, value):
        self.direction = value        
        
    def select(population):
        pass
        
class SingleCriteriaSelector(SurvivorsSelector):
    """
        Selector of genoms, that should be kept to next generation in the case of single criteria optimization
    """
    
    def select(self, population):
        signF = directionToSigns(self.direction)    
    
        print "len poulation ", len(population), ", 0.3*rec ", int(0.35*self.recomendedPopulationSize)
        survivorsNum = min(len(population), int(0.35*self.recomendedPopulationSize))
        scores = []
        for member in population:
            scores.append(member.functional)
        
        if signF == 1:
            scores.sort()        
        else:
            scores.sort(reverse = True)
             
                
        #print "Scores ", len(scores), ", ", survivorsNum-1, ", ", int(survivorsNum*0.8)-1, ", ", int(survivorsNum*0.5)-1
        priorityTresholds = [scores[survivorsNum-1],
                             scores[int(survivorsNum*0.8)-1],
                             scores[int(survivorsNum*0.5)-1]]

        #print "Tresholds: ", priorityTresholds       
        
        survivors = []
        
        for member in population:
            newMember = deepcopy(member)
            score = member.functional
            priority = 0
            for prior in range(3):
                if signF * score < signF * priorityTresholds[prior]:
                    priority = prior + 1
            
            if priority > 0:                    
                newMember.priority = priority
                newMember.setPopulationTo(newMember.getPopulationTo() + 1)
                survivors.append(newMember)
        
        return survivors

class MutationCreator:
    """
        General class for creation of mutations
    """
    
    def __init__(self, bounds):
        self.bounds = bounds
        
    def mutate(self, original):
        pass
        
class GeneralMutation(MutationCreator):
    """
        General mutation, user can select how many parameters is changed and whether they are large or small changes
    """
    
    def __init__(self, bounds, numMutatedEntries, mutationStrength):
        """ if mutationStrength == 1, use any random value from bounds. If <1, change in parameter at most mutationStrength * (bound_max - bound_min) """
        MutationCreator.__init__(self, bounds)
        self.numMutatedEntries = numMutatedEntries
        self.mutationStrength = mutationStrength

    def mutate(self, original):
        mutant = ModelBase()
        mutant.parameters = deepcopy(original.parameters)
        genomSize = len(self.bounds)
        
        # random indices of genes to be changed
        varsToChange = []
        for i in range(self.numMutatedEntries):
            varToChange = rnd.randrange(genomSize)
            while varToChange in varsToChange:
                varToChange = rnd.randrange(genomSize)
                
            varsToChange.append(varToChange)
        
        for idx in varsToChange:
            parameter = self.bounds[idx]
            originalValue = original.parameters[parameter.name]
            if self.mutationStrength == 1:
                newValue = parameter.randomValue()
            elif (self.mutationStrength < 1) and (self.mutationStrength > 0):
                newValue = parameter.smallPerturbation(originalValue, self.mutationStrength)
            else:
                assert 0
               
            mutant.parameters[parameter.name] = newValue 
            
        return mutant

class ImplicitMutation(MutationCreator):
    """
        Implicit mutation, randomly select one of standard types of mutations
    """
    
    def __init__(self, bounds):
        MutationCreator.__init__(self, bounds)
        
        # different types of mutation (large, small, 1 parameter, 2 parameters ....)
        self.mutationCreators = [GeneralMutation(self.bounds, 1, 1.0),
                                 GeneralMutation(self.bounds, 2, 1.0),
                                 GeneralMutation(self.bounds, 1, 0.25),
                                 GeneralMutation(self.bounds, 1, 0.1),
                                 GeneralMutation(self.bounds, 2, 0.1)]
                            
        
    def mutate(self, original):
        # select randomly one of them                            
        idx = rnd.randrange(len(self.mutationCreators))                            
                            
        return self.mutationCreators[idx].mutate(original)
       
            
class GeneralCrossover:
    """
        General class for definition of crossovers
    """
     
    def __init__(self):
        pass
    
    def cross(self, father, mother):
        pass
        

class RandomCrossover(GeneralCrossover):
    """
        Each gene randomly chosen from father or mother
    """
    
    def __init__(self):
        pass
        
    def cross(self, father, mother):
        son = ModelBase()
        for i in range(len(father.parameters)):
            paramName = father.parameters.keys()[i]
            if rnd.randrange(2) == 0:
                son.parameters[paramName] = father.parameters[paramName]
            else:
                son.parameters[paramName] = mother.parameters[paramName]
                
        return son                  
     
if __name__ == '__main__':
    # test initial population creation
    parameterCont1 = ContinuousParameter("A", -5, -1)
    parameterCont2 = ContinuousParameter("B", 6, 8)
    parameterDisc1 = DiscreteParameter("C", [1,3,6,88,2])
    
    bounds = [parameterCont1, parameterCont2, parameterDisc1]
    
    populationCreator = ImplicitInitialPopulationCreator(bounds)
    population = populationCreator.create(8)
    
    mutationCreator = ImplicitMutation(bounds)
    
    for member in population:
        mutant = mutationCreator.mutate(member)
        print "original: ", member.parameters
        print "mutation: ", mutant.parameters
            
    crossover = RandomCrossover()
            
    print "father: ", population[0].parameters    
    print "mother: ", population[1].parameters
    print "son1: ", crossover.cross(population[0], population[1]).parameters
    print "son2: ", crossover.cross(population[0], population[1]).parameters 
    print "son3: ", crossover.cross(population[0], population[1]).parameters 