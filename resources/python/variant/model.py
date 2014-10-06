_empty_svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import pickle
import os
import warnings

class StrictDict(dict):
    """General class for model data (parameters and variables).
       Class is inherited from dict and implement data type checking."""

    def __init__(self, *args, **kwargs):
        self.update(*args, **kwargs)
        self._data_types = dict()

    @property
    def data_types(self):
        return self._data_types

    def fulfilled(self):
        for key in self._data_types.keys():
            if not key in self.keys(): return False
        return True

    def __setitem__(self, key, value):
        if not key in self._data_types:
            raise KeyError('Value with name "{0}" is not declared!'.format(key))

        data_type = self._data_types[key]
        if ((type(value) != data_type) and
            (not (data_type == float and type(value) == int))):
            raise TypeError('Value data type do not correspond with defined data type!')

        self.update({key : value})

class Parameters(StrictDict):
    def declare(self, key, data_type, default=None):
        if default:
            if ((type(default) != data_type) and
                (not (data_type == float and type(default) == int))):
                raise TypeError('Value data type do not correspond with defined data type!')
            self.update({key : default})

        self._data_types[key] = data_type

class Variables(StrictDict):
    def declare(self, key, data_type):
        self._data_types[key] = data_type

class ModelData(object):
    """General class collected all model data."""

    def __init__(self):
        self.parameters = Parameters()
        self.variables = Variables()

        self.info = {'_geometry' : _empty_svg}

class ModelBase(object):
    """General class described model."""

    def __init__(self):
        self._data = ModelData()
        self.declare()

    @property
    def data(self):
        """Return model data."""
        return self._data

    @data.setter
    def data(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def parameters(self):
        """Return model input parameters."""
        return self._data.parameters

    @parameters.setter
    def parameters(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def variables(self):
        """Return model output variables."""
        return self._data.variables

    @variables.setter
    def variables(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def info(self):
        """Optional info dictionary."""
        return self._data.info

    @info.setter
    def info(self, value):
        self._data.info = value

    @property
    def solved(self):
        """Solution state of the model (return True if all declared variables are defined)."""
        return self._data.variables.fulfilled()

    @solved.setter
    def solved(self, value):
        if not value: self._data.variables.clear()

    def declare(self):
        """Method declare model input parameters and ouput variables by ModelInfo class."""
        warnings.warn("Method declare() should be overrided!", RuntimeWarning)

    def create(self):
        """Method creates model from input parameters."""
        warnings.warn("Method create() should be overrided!", RuntimeWarning)

    def solve(self):
        """Method for solution of model."""
        warnings.warn("Method solve() should be overrided!", RuntimeWarning)

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
            pickle.dump(self._data, outfile, 0) #pickle.HIGHEST_PROTOCOL

    def serialize(self):
        """Serialize and return model data to string."""
        return pickle.dumps(self._data, 0) #pickle.HIGHEST_PROTOCOL

    def deserialize(self, data):
        """Deserialize and return model data object from string.
        
        deserialize(data)
        """

        self._data = pickle.loads(data)

if __name__ == '__main__':
    import pythonlab

    class Model(ModelBase):
        def declare(self):
            self.parameters.declare('p', float, default = 1.0)
            self.variables.declare('v', float)

    model = Model()
    model.parameters['p'] = 2.0
    model.variables['v'] = 2.0
    print(model.solved)

    file_name = '{0}/model.pickle'.format(pythonlab.tempname())
    model.save(file_name)
    model.load(file_name)