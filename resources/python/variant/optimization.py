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
        

class OptimizationMethod:
    def __init__(self, parameters, direction):
        self.parameters = parameters
        self.direction = direction
        
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
        
    def directionToSigns(self):
        if self.direction == "min-min":
            return [1.0, 1.0]
        elif self.direction == "min-max":
            return [1.0, -1.0]
        elif self.direction == "max-min":
            return [-1.0, 1.0]
        elif self.direction == "max-max":
            return [-1.0, -1.0]
        elif self.direction == "min":
            return 1.0
        elif self.direction == "max":
            return -1.0
        else:
            assert 0    
        

        
if __name__ == '__main__':
    cp = ContinuousParameter("cont1", 2, 5.2)
    dp = DiscreteParameter("disc1", [4, 6, 77, 44, 99, 11])
    print [cp.randomValue(), cp.randomValue(), cp.randomValue()]
    print [cp.smallPerturbation(2.3, 0.2),cp.smallPerturbation(2.3, 0.2),cp.smallPerturbation(2.3, 0.2)]
    print [dp.randomValue(), dp.randomValue(), dp.randomValue()]    
    print [dp.smallPerturbation(99,0.1), dp.smallPerturbation(99,0.1), dp.smallPerturbation(99,0.5)]