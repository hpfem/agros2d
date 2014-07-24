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
