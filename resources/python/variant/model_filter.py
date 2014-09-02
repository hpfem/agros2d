import collections

from variant.model_dict import ModelDict

class ModelFilter():
    def __init__(self):
        self._parameters = dict()
        self._variables = dict()

    def add_parameter_value(self, name, value):
        self._parameters[name] = value

    def add_parameter_range(self, name, start, stop):
        self._parameters[name] = [start, stop]

    def add_variable_value(self, name, value):
        self._variables[name] = value

    def add_variable_range(self, name, start, stop):
        self._variables[name] = [start, stop]

    def filter(self, model_dict):
        md = ModelDict()
        for name, model in model_dict.dict.items():
            if self._test(model): md.add_model(model, name)

        return md

    def _test(self, model):
        for parameter, value in self._parameters.items():
            if not isinstance(value, collections.Iterable):
                if (model.parameters[parameter] != value):
                    return False
            else:
                if (model.parameters[parameter] < value[0] or
                    model.parameters[parameter] > value[1]):
                    return False

        for variable, value in self._variables.items():
            if not isinstance(value, collections.Iterable):
                if model.variables[variable] != value:
                    return False
            else:
                if (model.variables[variable] < value[0] or
                    model.variables[variable] > value[1]):
                    return False

        return True

if __name__ == '__main__':
    from variant.test_functions import quadratic_function

    md = ModelDict(quadratic_function.QuadraticFunction)
    for x in range(10):
        model = quadratic_function.QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    md.solve()

    mf = ModelFilter()
    #mf.add_parameter_value('x', 2)
    #mf.add_variable_value('F', 4)
    #mf.add_parameter_range('x', 3, 5)
    mf.add_variable_range('F', 1, 16)
    print(mf.filter(md).dict)