class Functional:
    """General class for optimization functional."""

    def __init__(self, name, direction):
        """Initialization of functional object.
        
        Functional(name, direction)
        
        Keyword arguments:
        name -- functional name
        direction -- optimization direction ("min" or "max")
        """

        self.name = name
        self._direction = str()
        self.direction = direction

    @property
    def direction(self):
        """Direction of functional ("min" or "max")."""
        return self._direction

    @direction.setter
    def direction(self, value):
        if value in ["min", "max"]:
            self._direction = value
        else:
            raise ValueError('Direction "{0}" is not defined. Allowed values are "min" and "max".'.format(value))
        
    def direction_sign(self):
        """Return sign of functional direction."""
        if self.direction == "min":
            return 1
        elif self.direction == "max":
            return -1
            
class Functionals:
    """General class collected functionals."""

    def __init__(self, functionals=None):
        """Initialization of functionals object.
        
        Functionals(functionals)
        
        Keyword arguments:
        functionals -- one Functional object or list (default is None)
        """

        self._functionals = list()
        if isinstance(functionals, list):
            for functional in functionals:
                self.add_functional(functional)

    @property
    def functionals(self):
        """List of functionals for optimization."""
        return self._functionals

    def add_functional(self, functional):
        if isinstance(functional, Functional):
            self._functionals.append(functional)
        else:
            raise TypeError('Functional must be instance of Functional class.')

    def functional(self, name=None):
        """Find and return functional by name.

        parameter(name)

        Keyword arguments:
        name -- functional name
        """

        if not name:
            name = self.functionals[0].name

        for functional in self.functionals:
            if (functional.name == name): return functional

    def multicriteria(self):
        """Return True if more functionals are defined."""
        if len(self.functionals) >= 2:
            return True
        else:
            return False

    def evaluate(self, model):
        """For one criteria returns value of functional, for multicriteria list [F1, F2, ..., Fn] for given instance of model.
        
        evaluate(model)

        Keyword arguments:
        model -- instance of the model
        """

        if self.multicriteria():
            scores = []
            for functional in self.functionals:
                scores.append(model.variables[functional.name])
            return scores
        else:
            return model.get_variable(self.functionals[0].name)