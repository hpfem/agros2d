class Functional:
    def __init__(self, name, direction):
        self.name = name

        self._direction = str()
        self.direction = direction

    @property
    def direction(self):
        """Direction of functional ("min", "max")."""
        return self._direction

    @direction.setter
    def direction(self, value):
        if value in ["min", "max"]:
            self._direction = value
        else:
            raise ValueError('Direction "{0}" is not defined. Allowed directions are "min" and "max".'.format(value))
        
    def direction_sign(self):
        """Return sign of functional direction."""
        if self.direction == "min":
            return 1
        elif self.direction == "max":
            return -1
            
class Functionals:
    def __init__(self, functionals):
        self._functionals = list()
        self.functionals = functionals

    @property
    def functionals(self):
        """Functionals for optimization."""
        return self._functionals

    def _add_functional(self, functional):
        if isinstance(functional, Functional):
            self._functionals.append(functional)
        else:
            raise TypeError('Functional must be instance of variant.optimization.Functional class.')

    @functionals.setter
    def functionals(self, value):
        if isinstance(value, list):
            for item in value:
                self._add_functional(item)
        else:
            self._add_functional(value)

    def functional(self, index = 0):
        """Return functional for multicriteria optimization can be called with index 0 or 1."""
        return self.functionals[index]

    def multicriteria(self):
        """Functional, for multicriteria optimization can be called with index 0 or 1."""
        if len(self.functionals) >= 2:
            return True
        else:
            return False

    def evaluate(self, model):
        """For one criteria returns value of functional, for multicriteria list [F1, F2] for given instance of model.
        
        evaluate(model)

        Keyword arguments:
        model -- instance of the model
        """
        if self.multicriteria():
            return [model.variables[self.functional().name],
                    model.variables[self.functional(index = 1).name]]
        else:
            return model.variables[self.functional().name]