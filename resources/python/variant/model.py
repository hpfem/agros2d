_empty_svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import pickle
import os

class Parameters(dict):
    """General class collected model parameters."""

    def __init__(self, defaults):
        dict.__init__(self)
        self._defaults = defaults

    def __getitem__(self, key):
        if key in dict.keys(self):
            return dict.__getitem__(self, key)
        elif key in self._defaults.keys():
            return self._defaults[key]
        else:
            raise KeyError(key)

class ModelData:
    """General class collected all model data."""

    def __init__(self):
        self.defaults = dict()
        self.parameters = Parameters(self.defaults)
        self.variables = dict()
        self.info = {'_geometry' : _empty_svg}
        self.solved = False

class ModelInfo:
    """General class stored informations about model parameters and variables."""
    def __init__(self):
        self._parameters = dict()
        self._variables = dict()

    def add_parameter(self, name, type, description = ''):
        self._parameters[name] = [type, description]

    def parameters(self):
        return self._parameters

    def add_variable(self, name, type, description = ''):
        self._variables[name] = [type, description]

    def variables(self):
        return self._variables

class ModelBase:
    """General model class."""

    def __init__(self):
        self.model_info = ModelInfo()
        self.declare()

        self._data = ModelData()

    @property
    def data(self):
        """Return model data object."""
        return self._data

    @property
    def parameters(self):
        """Model parameters dictionary."""
        return self._data.parameters

    @parameters.setter
    def parameters(self, value):
        self._data.parameters = value

    @property
    def defaults(self):
        """Default parameters dictionary used if key can not be find in model parameters."""
        return self._data.defaults

    @defaults.setter
    def defaults(self, value):
        self._data.defaults = value

    @property
    def variables(self):
        """Output variables dictionary"""
        return self._data.variables

    @variables.setter
    def variables(self, value):
        self._data.variables = value

    @property
    def info(self):
        """Optional info dictionary"""
        return self._data.info

    @info.setter
    def info(self, value):
        self._data.info = value

    @property
    def solved(self):
        """Solution state of the model."""
        return self._data.solved

    @solved.setter
    def solved(self, value):
        self._data.solved = value
        if not value: self._data.variables = {}

    def declare(self):
        raise RuntimeError('Method declare() must be overrided!')

    def create(self):
        """Method creates model from defined parameters."""
        pass

    def solve(self):
        """Method solved model."""
        pass

    def process(self):
        """Method calculate output variables."""
        pass    
        
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
        
        protocol = pickle.HIGHEST_PROTOCOL
        # protocol = 0 # human-readable

        directory = os.path.dirname(file_name)
        if not os.path.isdir(directory):
            os.makedirs(directory)

        with open(file_name, 'wb') as outfile:
            pickle.dump(self._data, outfile, protocol)

    def serialize(self):
        return pickle.dumps(self._data, pickle.HIGHEST_PROTOCOL)

    def deserialize(self, dta):
        self._data = pickle.loads(dta)

    def clear(self):
        """Clear model data."""
        self._data = ModelData()