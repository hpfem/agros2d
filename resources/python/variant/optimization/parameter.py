import random as rnd

class OptimizationParameter:
    def __init__(self, name):
        self.name = name

    def continues(self):
        pass
        
    def random_value(self):
        pass

    def perturbation(self):
        pass

class ContinuousParameter(OptimizationParameter):
    def __init__(self, name, min, max):
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

        perturbation(self, original, ratio)

        Keyword arguments:
        original -- original value
        ration -- maximum ration for change
        """

        if (self.min > original or  self.max < original):
            raise ValueError('Original must be in the range from {0} to {1}.'.format(self.min, self.max))

        diff =  ratio * self.interval()
        act_min = max(self.min, original - diff)
        act_max = min(self.max, original + diff)

        return act_min + rnd.random() * (act_max - act_min)
        
    def interval(self):
        return self.max - self.min

class DiscreteParameter(OptimizationParameter):
    def __init__(self, name, options):
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

if __name__ == '__main__':
    cp = ContinuousParameter("cp", 2, 5.2)
    dp = DiscreteParameter("dp", [4, 6, 77, 44, 99, 11])

    print([cp.random_value(), cp.random_value(), cp.random_value()])
    print([cp.perturbation(2.3, 0.2), cp.perturbation(2.3, 0.2), cp.perturbation(2.3, 0.2)])

    print([dp.random_value(), dp.random_value(), dp.random_value()])
    print([dp.perturbation(99,0.1), dp.perturbation(99,0.1), dp.perturbation(99,0.5)])