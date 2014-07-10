from variant import ModelBase, ModelDict
from itertools import product

class ModelGenerator:
    def __init__(self):
        self._dict = ModelDict()

        self._model_class = ModelBase
        self._parameters = dict()

    @property
    def parameters(self):
        return self._parameters

    @parameters.setter
    def parameters(self, value):
        self._parameters = value

    def save(self):
        pass

    def combination(self):
        combinations = [[{key: value} for (key, value) in zip(self.parameters, values)] 
                       for values in product(*self.parameters.values())]

        for combination in combinations:
            model = self._model_class()
            for parameter in combination:
                model.parameters.update(parameter)
            self._dict.add_model(model)

    def random_selection(self):
        pass

if __name__ == '__main__':
    mg = ModelGenerator()
    mg.parameters['a'] = [1, 2]
    mg.parameters['b'] = [1, 2]

    mg.combination()
    #mg.random_selection()
    print(mg._dict._models)