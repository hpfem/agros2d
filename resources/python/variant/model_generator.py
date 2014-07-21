from variant import ModelBase, ModelDict
from itertools import product
from random import choice

class ModelGenerator:
    def __init__(self):
        self._dict = ModelDict()

        self._model_class = ModelBase
        self._parameters = dict()

    @property
    def parameters(self):
        """Parameters of the model."""
        return list(self._parameters)

    def add_parameter(self, name, values):
        """Add new model parameter.

        add_parameter(name, values)

        Keyword arguments:
        name -- parameter name
        values -- list of parameter values
        """
        self._parameters[name] = values

    def remove_parameter(self, name):
        """Remove model parameter.

        remove_parameter(name)

        Keyword arguments:
        name -- parameter name
        """
        del self._parameters[name]

    def save(self, directory):
        """Save models to directory.

        save(directory)

        Keyword arguments:
        directory -- directory path
        """
        self._dict.directory = directory
        self._dict.save()

    def combination(self):
        """Generate models by combination of parameters values."""
        combinations = [[{key: value} for (key, value) in zip(self._parameters, values)] 
                       for values in product(*self._parameters.values())]

        for combination in combinations:
            model = self._model_class()
            for parameter in combination:
                model.parameters.update(parameter)

            self._dict.add_model(model)

    def random_selection(self, count):
        """Generate models by random selection of parameters values."""
        for index in range(count):
            model = self._model_class()
            for key, value in self._parameters.items():
                model.parameters[key] = choice(value)

            self._dict.add_model(model)

if __name__ == '__main__':
    mg = ModelGenerator()
    mg.add_parameter('a', [1, 2, 3, 4, 5])
    mg.add_parameter('b', [0.01, 0.02])
    mg.add_parameter('c', [100, 200, 300])

    mg.combination()
    #mg.random_selection(5)
    mg.save('models')