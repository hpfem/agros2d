_empty_svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import pickle
import os

class BBB:
    def __init__(self):
        self.a = 1
        
class Parameters(dict):
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
    def __init__(self):
        self.defaults = dict()
        self.parameters = Parameters(self.defaults)
        self.variables = dict()
        self.info = {'_geometry' : _empty_svg}
        self.solved = False

class ModelBase:
    def __init__(self):
        self._data = ModelData()

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
        """Solution state of model."""
        return self._data.solved

    @solved.setter
    def solved(self, value):
        self._data.solved = value
        if not value: self._data.variables.clear()

    def create(self):
        """Method creates model from parameters."""
        pass

    def solve(self):
        """Method solved model."""
        pass

    def process(self):
        """Method calculate output variables."""
        pass    
        
    def load(self, file_name):
        """Unpickle model and save binary file (marshalling of model object).

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

    def clear(self):
        """Clear model data"""
        self._data = ModelData()
