import random as rnd

class OptimizationParameter:
    def __init__(self, name):
        self._name = name
        
    @property
    def name(self):
        """Name"""
        return self._name

    def continuous(self):
        pass
        
    def randomValue(self):
        """ Random value within bounds or from the list in the case of discrete """
        pass
        
    def smallPerturbation(self, original, maxRatio):
        """ chagne of original value, change only within +- maxRatio * (max val - min val). For discrete parameters this is approximate """
        pass
        
    def type(self):
        """Type of parameter (continuous / discrete)"""
        return self._type

class ContinuousParameter(OptimizationParameter):
    def __init__(self, name, min, max):
        OptimizationParameter.__init__(self, name)
        self._type = "continuous"
        self._min = min
        self._max = max
        
    def continuous(self):
        return True
        
    @property
    def min(self):
        """Minimum"""
        return self._min

    @min.setter
    def min(self, value):
        self._min = value

    @property
    def max(self):
        """Maximum"""
        return self._max

    @max.setter
    def max(self, value):
        self._max = value

    def randomValue(self):
        """ Random value within bounds """
        return self.min + rnd.random() * (self.max - self.min)
        
    def smallPerturbation(self, original, maxRatio):
        """ chagne of original value, change only within +- maxRatio * (max val - min val) . """
        wholeInterval = self.max - self.min
        maxDifference =  maxRatio * wholeInterval 
        actMin = max(self.min, original - maxDifference)
        actMax = min(self.max, original + maxDifference)
        return actMin + rnd.random() * (actMax - actMin)
        
    def intervalLength(self):
        return self.max - self.min

class DiscreteParameter(OptimizationParameter):
    def __init__(self, name, options):
        OptimizationParameter.__init__(self, name)
        self._type = "discrete"     
        self._options = options           
        
    def continuous(self):
        return False
        
    @property
    def options(self):
        """List of options"""
        return self._options

    @options.setter
    def options(self, value):
        self._options = value
                
    def randomValue(self):
        """ Random value from options """
        return self.options[rnd.randrange(len(self.options))]
        
    def smallPerturbation(self, original, maxRatio):
        """ chagne of original value, change only within +-maxRatio * range . """
        maxShift = int(len(self.options) * maxRatio)
        if maxShift == 0:
            maxShift = 1
            
        originalIndex = self.options.index(original)
        minIndex = max(0, originalIndex - maxShift)
        maxIndex = min(len(self.options) - 1, originalIndex + maxShift)
        newIndex = originalIndex
        while newIndex == originalIndex:
            newIndex = rnd.randrange(minIndex, maxIndex + 1)
        
        return self.options[newIndex]

        
class Functional:
    def __init__(self, name, direction):
        self._name = name
        self._direction = direction
        
    @property
    def name(self):
        """Name"""
        return self._name
                                                    
    @property
    def direction(self):
        """Direction"""
        return self._direction
        
    def directionSign(self):
        if self.direction == "min":
            return 1
        elif self.direction == "max":
            return -1
        else:
            assert False
            
class Functionals:
    def __init__(self, functionals):
        self.functionals = functionals

    def functional(self, index = 0):
        """ Functional, for multicriteria optimization can be called with index 0 or 1 """
        return self.functionals[index]
        
    def isMulticriterial(self):
        if len(self.functionals) == 1:
            return False
        elif len(self.functionals) == 2:
            return True
        else:
            assert False
        
    def evaluate(self, model):
        """ for one criteria returns value of functional, for multicriteria list [func1, func2] for given instance of model """
        if self.isMulticriterial():
            return [model.variables[self.functionals[0].name], model.variables[self.functionals[1].name] ]
        else:
            return model.variables[self.functionals[0].name]
        
                                                    

class OptimizationMethod:
    def __init__(self, parameters, functionals):
        self._parameters = parameters
        self._functionals = functionals
        
    @property
    def parameters(self):
        """Parameters"""
        return self._parameters
        
    @parameters.setter
    def parameters(self, value):
        self._parameters = value
        
    @property
    def directory(self):
        """Working directory"""
        return self._directory

    @directory.setter
    def directory(self, value):
        self._directory = value
                    
    @property
    def functionals(self):
        """Functionals"""
        return self._functionals      
                   
         
    def isContained(self, population, newModel):
        """ checks whether newModel is allready contained in population """
        contained = False
        for oldModel in population:
            allSame = True
            for key in oldModel.parameters.keys():
                # non equality test should be done differently
                # but even this should be better than nothing
                if oldModel.parameters[key] != newModel.parameters[key]:
                    allSame = False
                    break
            if allSame:
                contained = True
                break
                
        return contained
        
if __name__ == '__main__':
    cp = ContinuousParameter("cont1", 2, 5.2)
    dp = DiscreteParameter("disc1", [4, 6, 77, 44, 99, 11])
    print [cp.randomValue(), cp.randomValue(), cp.randomValue()]
    print [cp.smallPerturbation(2.3, 0.2),cp.smallPerturbation(2.3, 0.2),cp.smallPerturbation(2.3, 0.2)]
    print [dp.randomValue(), dp.randomValue(), dp.randomValue()]    
    print [dp.smallPerturbation(99,0.1), dp.smallPerturbation(99,0.1), dp.smallPerturbation(99,0.5)]