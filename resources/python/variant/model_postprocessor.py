from variant.model_dictionary import ModelDictionary

class ModelPostprocessor(object):
    """General class for models postprocessing."""

    def __init__(self, model_dictionary):
        """Initialization of model postprocessor.

        ModelGenerator(model_dictionary)

        Keyword arguments:
        model_dictionary -- ModelDictionary object with models
        """

        self._dict = model_dictionary

        # cache parameters
        self._parameters = {}
        for model in self._dict.models:
            for parameter, value in model.parameters.items():
                self._parameters.setdefault(parameter, []).append(value)

        parameters = self._dict.model_class().data.declared_parameters
        self._parameters_keys = sorted(list(parameters.keys()))
        self._parameters_scalar_keys = sorted([key for key, value in parameters.items()\
                                               if (value['type'] == int or value['type'] == float)])

        # cache variables
        self._variables = {}
        for model in self._dict.models:
            for variable, value in model.variables.items():
                self._variables.setdefault(variable, []).append(value)

        variables = self._dict.model_class().data.declared_variables
        self._variables_keys = sorted(list(variables.keys()))
        self._variables_scalar_keys = sorted([key for key, value in variables.items()\
                                              if (value['type'] == int or value['type'] == float)])

    @property
    def dictionary(self):
        """Return model dictionary."""
        return self._dict

    def parameters(self, only_numbers=False):
        """Return dictionary collected values of model parameters in lists.

        parameters(only_numbers=False)

        Keyword arguments:
        only_numbers -- return only int and float parameters (default is False)
        """
        if not only_numbers:
            return self._parameters
        else:
            parameters = {}
            for key in self.parameters_keys(only_numbers=True):
                parameters[key] = self._parameters[key]
            return parameters

    def parameter(self, name):
        return self._parameters[name]

    def parameters_keys(self, only_numbers=False):
        """Return keys of all model parameters.

        parameters_keys(only_numbers=False)

        Keyword arguments:
        only_numbers -- return only int and float parameters (default is False)
        """

        if not only_numbers:
            return self._parameters_keys
        else:
            return self._parameters_scalar_keys

    def variables(self, only_numbers=False):
        """Return dictionary collected values of variables in lists.

        variables(only_numbers=False)

        Keyword arguments:
        only_numbers -- return only int and float variables (default is False)
        """

        if not only_numbers:
            return self._variables
        else:
            variables = {}
            for key in self.variables_keys(only_numbers=True):
                variables[key] = self._variables[key]

            return variables

    def variable(self, name):
        return self._variables[name]

    def variables_keys(self, only_numbers = False):
        """Return keys of all variables.

        variables(only_numbers=False)

        Keyword arguments:
        only_numbers -- return only int and float variables (default is False)
        """

        if not only_numbers:
            return self._variables_keys
        else:
            return self._variables_scalar_keys

if __name__ == '__main__':
    from variant.test_functions import quadratic_function
    from variant.model_filter import ModelFilter

    md = ModelDictionary(quadratic_function.QuadraticFunction)
    for x in range(10):
        model = quadratic_function.QuadraticFunction()
        model.set_parameter('x', x)
        md.add_model(model)

    md.solve()

    mp = ModelPostprocessor(md)
    print(mp.parameters_keys())
    print(mp.parameters()['x'])
    print(mp.variables_keys())
    print(mp.variables()['F'])

    mf = ModelFilter()
    mf.add_parameter_range('x', 1, 9)

    mp = ModelPostprocessor(mf.filter(md))
    print(mp.parameters()['x'])
