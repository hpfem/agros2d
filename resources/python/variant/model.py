_empty_svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import pickle
import os
import warnings

class ModelValues(dict):
    """General class for model parameters and variables."""

    def __init__(self, declarations):
        self._declarations = declarations

        defaults = dict()
        for key, value in self._declarations.items():
            if value['default']: defaults[key] = value['default']
        self.update(defaults)

    def __setitem__(self, key, value):
        if key not in self._declarations.keys():
            raise KeyError('Value with key "{0}" is not declared!'.format(key))

        if (not (self._declarations[key]['type'] == float and type(value) == int)):
            if (type(value) != self._declarations[key]['type']):
                raise TypeError('Value must be type {0}!'.format(self._declarations[key]['type']))

        dict.__setitem__(self, key, value)

class ModelData(object):
    """General class collected all model data."""

    def __init__(self, parameters, variables):
        self.parameters = ModelValues(parameters)
        self.variables = ModelValues(variables)

        self.info = {'_geometry' : _empty_svg}
        self.solved = False

class ModelBase(object):
    """General model class."""

    def __init__(self):
        self._declared_parameters = dict()
        self._declared_variables = dict()

        self.declare()
        self._data = ModelData(self._declared_parameters,
                               self._declared_variables)

    @property
    def data(self):
        """Return model data."""
        return self._data

    @property
    def parameters(self):
        """Model input parameters dictionary."""
        return self._data.parameters

    @parameters.setter
    def parameters(self, value):
        self._data.parameters = value

    @property
    def variables(self):
        """Output output variables dictionary."""
        return self._data.variables

    @variables.setter
    def variables(self, value):
        self._data.variables = value

    @property
    def info(self):
        """Optional info dictionary."""
        return self._data.info

    @info.setter
    def info(self, value):
        self._data.info = value

    @property
    def solved(self):
        """Solution state of the model (return True if model is solved)."""
        return self._data.solved

    @solved.setter
    def solved(self, value):
        if not value: self._data.variables = {}
        self._data.solved = value

    def declare(self):
        """Method declare model input parameters and ouput variables by ModelInfo class."""
        warnings.warn("Method declare() should be overrided!", RuntimeWarning)

    def declare_parameter(self, name, data_type, default=None, description=''):
        """Declare new model input parameter.

        add_parameter(name, type, description = '')

        Keyword arguments:
        name -- parameter name
        type -- parameter data type
        default -- parameter default value (default is None)
        description --- description of parameter (default is empty string)
        """

        if (default and type(default) != data_type):
            if not (data_type == float and type(default) == int):
                raise TypeError('Data type of default value do not correspond with defined data type!')

        self._declared_parameters[name] = {'type' : data_type, 'default' : default,
                                           'description' : description}

    def declare_variable(self, name, data_type, default=None, description=''):
        """Declare new model output variable.

        add_variable(name, type, description = '')

        Keyword arguments:
        name -- variable name
        type -- parameter data type
        default -- variable default value
        description --- description of variable (default is empty string)
        """

        if (default and type(default) != data_type):
            if not (data_type == float and type(default) == int):
                raise TypeError('Data type of default value do not correspond with defined data type!')

        self._declared_variables[name] = {'type' : data_type, 'default' : default,
                                          'description' : description}

    def create(self):
        """Method creates model from input parameters."""
        warnings.warn("Method create() should be overrided!", RuntimeWarning)

    def solve(self):
        """Method for solution of model."""
        warnings.warn("Method solve() should be overrided!", RuntimeWarning)

    def process(self):
        """Method calculate output variables."""
        warnings.warn("Method process() should be overrided!", RuntimeWarning)
        
    def load(self, file_name):
        """Unpickle model and load binary file (marshalling of model object).

        load(file_name)

        Keyword arguments:
        file_name -- file name of binary file for read
        """

        with open(file_name, 'rb') as infile:
            self._data = pickle.load(infile)

    def save(self, file_name):
        """Pickle model and save binary file (serialization of model object).

        save(file_name)

        Keyword arguments:
        file_name -- file name of binary file for write
        """

        directory = os.path.dirname(file_name)
        if not os.path.isdir(directory):
            os.makedirs(directory)

        with open(file_name, 'wb') as outfile:
            # TODO: pickle.HIGHEST_PROTOCOL
            pickle.dump(self._data, outfile, 0)

    def serialize(self):
        # TODO: pickle.HIGHEST_PROTOCOL
        return pickle.dumps(self._data, 0)

    def deserialize(self, data):
        self._data = pickle.loads(data)

    def clear(self):
        """Clear model data."""
        self._data = ModelData(self._declared_parameters,
                               self._declared_variables)

if __name__ == '__main__':
    import pythonlab

    class Model(ModelBase):
        def declare(self):
            self.declare_parameter('p', float, default = 1.0)
            self.declare_variable('v', float)

    model = Model()
    model.parameters['p'] = 2.0
    model.variables['v'] = 2.0
    
    file_name = '{0}/model.pickle'.format(pythonlab.tempname())
    model.save(file_name)
    model.load(file_name)