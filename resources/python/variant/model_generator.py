from variant.model_dictionary import ModelDictionary
from itertools import product

import random as rnd

class ModelGenerator(object):
    """General class for generation of model sets (combination, random selection, etc.)."""

    def __init__(self, model_class):
        """Initialization of model generator.
        
        ModelGenerator(model_class)
        
        model_class -- general class for models
        """

        self._dict = ModelDictionary(model_class)
        self._parameters = dict()

    @property
    def parameters(self):
        """Return parameters of the model."""
        return self._parameters

    @property
    def dictionary(self):
        """Return models dictionary."""
        return self._dict

    def models(self):
        """Return list of models in dictionary."""
        return self._dict.models()

    def add_parameter(self, name, values):
        """Add new model parameter defined by list of values.

        add_parameter(name, values)

        Keyword arguments:
        name -- parameter name
        values -- list of parameter values
        """

        self._parameters[name] = values

    def add_parameter_by_interval(self, name, start, stop, step):
        """Add new model parameter by interval and step value.

        add_parameter(name, start, stop, step)

        Keyword arguments:
        name -- parameter name
        start -- start of interval
        stop -- end of interval
        step -- spacing between values
        """

        number = int(round((stop - start)/float(step)))
        if (number > 1):
            self._parameters[name] = [start + step*i for i in range(number + 1)]
        else:
            raise RuntimeError('Interval is not defined correctly.')

    def remove_parameter(self, name):
        """Remove model parameter.

        remove_parameter(name)

        Keyword arguments:
        name -- parameter name
        """

        del self._parameters[name]

    def combination(self):
        """Generate models by combination of all parameters values."""
        combinations = [[{key: value} for (key, value) in zip(self._parameters, values)] 
                       for values in product(*self._parameters.values())]

        for combination in combinations:
            model = self._dict.model_class()
            for parameter in combination:
                model.parameters.update(parameter)

            self._dict.add_model(model)

    def random_selection(self, count):
        """Generate models by random selection of parameters values."""
        for index in range(count):
            model = self._dict.model_class()
            for key, value in self._parameters.items():
                model.parameters[key] = rnd.choice(value)

            self._dict.add_model(model)

if __name__ == '__main__':
    from variant.test_functions import quadratic_function

    mg = ModelGenerator(quadratic_function.QuadraticFunction)
    mg.add_parameter('a', range(5))
    mg.add_parameter('b', range(10))
    mg.add_parameter('c', range(50))
    mg.add_parameter('x', range(4))

    mg.combination()
    #mg.random_selection(10000)