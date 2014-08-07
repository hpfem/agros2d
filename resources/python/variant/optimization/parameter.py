import random as rnd

class OptimizationParameter:
    """General class for optimization parameter."""
    def __init__(self, name):
        self.name = name

    def continues(self):
        """Return True if parameter is continue."""
        pass
        
    def random_value(self):
        pass

    def perturbation(self):
        pass

class ContinuousParameter(OptimizationParameter):
    """Class for parameters with continue interval."""

    def __init__(self, name, min, max):
        """Initialization of parameter object.
        
        ContinuousParameter(name, min, max)
        
        Keyword arguments:
        name -- parameter name
        min -- minimum of parameter value interval
        max -- maximum of parameter value interval
        """

        OptimizationParameter.__init__(self, name)
        self.min = min
        self.max = max
        
    def continuous(self):
        return True

    def random_value(self):
        """Return random value within bounds."""
        return self.min + rnd.random() * (self.max - self.min)
        
    def perturbation(self, original, ratio):
        """Change original value within +-ratio*(max - min).

        perturbation(original, ratio)

        Keyword arguments:
        original -- original value
        ration -- maximum ration for parameter value change
        """

        if (self.min > original or  self.max < original):
            raise ValueError('Original must be in the range from {0} to {1}.'.format(self.min, self.max))

        difference =  ratio * self.interval()
        value_min = max(self.min, original - difference)
        value_max = min(self.max, original + difference)

        return value_min + rnd.random() * (value_max - value_min)
        
    def interval(self):
        """Return minimum and maximum value of defined interval."""
        return self.max - self.min

class DiscreteParameter(OptimizationParameter):
    def __init__(self, name, options):
        """Initialization of parameter object.
        
        DiscreteParameter(name, options)
        
        Keyword arguments:
        name -- parameter name
        options -- list of parameter values
        """

        OptimizationParameter.__init__(self, name)
        self.options = options

    def continuous(self):
        return False

    def random_value(self):
        """Return random value from options."""
        return rnd.choice(self.options)
        
    def perturbation(self, original, ratio):
        """Approximate change original value within +-ratio*(max - min).

        perturbation(self, original, ratio)

        Keyword arguments:
        original -- original value
        ration -- maximum ration for change
        """
        
        max_shift = int(len(self.options) * ratio)
        if max_shift == 0: max_shift = 1
            
        original_index = self.options.index(original)
        min_index = max(0, original_index - max_shift)
        max_index = min(len(self.options) - 1, original_index + max_shift)

        new_index = original_index
        while new_index == original_index:
            new_index = rnd.randrange(min_index, max_index + 1)

        return self.options[new_index]

class Parameters:
    """General class collected parameters."""

    def __init__(self, parameters=None):
        """Initialization of parameters object.
        
        Parameters(parameters=None)
        
        Keyword arguments:
        parameters -- list of parameters (default is None)
        """

        self._parameters = list()
        if isinstance(parameters, list):
            for parameter in parameters:
                self.add_parameter(parameter)

    @property
    def parameters(self):
        """List of optimized parameters."""
        return self._parameters

    def add_parameter(self, parameter):
        if (parameter.__class__.__base__ == OptimizationParameter):
            self._parameters.append(parameter)
        else:
            raise TypeError('Parameter must be instance or inherited object from OptimizationParameter class.')

    def parameter(self, name):
        """Find and return parameter by name.

        parameter(name)

        Keyword arguments:
        name -- parameter name
        """

        for parameter in self.parameters:
            if (parameter.name == name): return parameter

if __name__ == '__main__':
    cp = ContinuousParameter("cp", 2, 5.2)
    dp = DiscreteParameter("dp", [4, 6, 77, 44, 99, 11])

    print([cp.random_value(), cp.random_value(), cp.random_value()])
    print([cp.perturbation(2.3, 0.2), cp.perturbation(2.3, 0.2), cp.perturbation(2.3, 0.2)])

    print([dp.random_value(), dp.random_value(), dp.random_value()])
    print([dp.perturbation(99,0.1), dp.perturbation(99,0.1), dp.perturbation(99,0.5)])
