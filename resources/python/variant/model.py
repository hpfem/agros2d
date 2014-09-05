_empty_svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import pickle
import os
import warnings

class ModelData(object):
    """General class collected all model data."""

    def __init__(self):
        self.declared_parameters = dict()
        self.parameters = dict()

        self.declared_variables = dict()
        self.variables = dict()

        self.info = {'_geometry' : _empty_svg}
        self.solved = False

class ModelBase(object):
    """General class described model."""

    def __init__(self):
        self._data = ModelData()
        self.declare()

    @property
    def data(self):
        """Return model data."""
        return self._data

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
        if not value: self._data.variables = dict()
        self._data.solved = value

    def _check_value_type(self, value, data_type):
        if (value and type(value) != data_type):
            if not (data_type == float and type(value) == int):
                raise TypeError('Data type of value do not correspond with defined data type!')

    def declare(self):
        """Method declare model input parameters and ouput variables by ModelInfo class."""
        warnings.warn("Method declare() should be overrided!", RuntimeWarning)

    def declare_parameter(self, name, data_type, default=None, description=''):
        """Declare new model input parameter.

        declare_parameter(name, type, description = '')

        Keyword arguments:
        name -- parameter name
        type -- parameter data type
        default -- parameter default value (default is None)
        description --- description of parameter (default is empty string)
        """

        self._check_value_type(default, data_type)
        self._data.declared_parameters[name] = {'type' : data_type, 'default' : default,
                                                'description' : description}

        if default: self._data.parameters[name] = default

    def set_parameter(self, name, value):
        """Set value of model input parameter.

        set_parameter(name, value)
        """

        if name not in self._data.declared_parameters.keys():
            raise KeyError('Parameter with name "{0}" is not declared!'.format(name))
        self._check_value_type(value, self._data.declared_parameters[name]['type'])

        self._data.parameters[name] = value

    def get_parameter(self, name):
        """Return value of model input parameter.
        
        get_parameter(name)
        """

        return self._data.parameters[name]

    def declare_variable(self, name, data_type, default=None, description=''):
        """Declare new model output variable.

        add_variable(name, type, description = '')

        Keyword arguments:
        name -- variable name
        type -- parameter data type
        default -- variable default value
        description --- description of variable (default is empty string)
        """

        self._check_value_type(default, data_type)
        self._data.declared_variables[name] = {'type' : data_type, 'default' : default,
                                               'description' : description}

        if default: self._data.variables[name] = default

    def set_variable(self, name, value):
        """Set value of model ouput variable.

        set_variable(name, value)
        """

        if name not in self._data.declared_variables.keys():
            raise KeyError('Variable with name "{0}" is not declared!'.format(name))
        self._check_value_type(value, self._data.declared_variables[name]['type'])

        self._data.variables[name] = value

    def get_variable(self, name):
        """Return value of model output variable.
        
        get_variable(name)
        """

        return self._data.variables[name]

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
            pickle.dump(self._data, outfile, pickle.HIGHEST_PROTOCOL)

    def serialize(self):
        """Serialize and return model data to string."""
        return pickle.dumps(self._data, pickle.HIGHEST_PROTOCOL)

    def deserialize(self, data):
        """Deserialize and return model data object from string.
        
        deserialize(data)
        """

        self._data = pickle.loads(data)

if __name__ == '__main__':
    import pythonlab

    class Model(ModelBase):
        def declare(self):
            self.declare_parameter('p', float, default = 1.0)
            self.declare_variable('v', float)

    model = Model()
    model.set_parameter('p', 2.0)
    model.set_variable('v', 2.0)

    file_name = '{0}/model.pickle'.format(pythonlab.tempname())
    model.save(file_name)
    model.load(file_name)