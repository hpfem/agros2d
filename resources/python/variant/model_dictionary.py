from variant.model import ModelBase
from collections import OrderedDict

import os
import subprocess
import zipfile
import json

MAXIMUM_NUMBER_OF_MODELS = 9

class ModelDictionary(object):
    """General class for management of models."""

    def __init__(self, model_class=ModelBase, models=None):
        """Initialization of model dictionary.

        ModelDictionary(model_class=ModelBase, models=None)

        Keyword arguments:
        model_class -- general class for managed models (default is ModelBase)
        models -- list or dictionary in style {name : model} (default is None)
        """

        self._dict = OrderedDict()
        self.model_class = model_class

        if (models and isinstance(models, dict)):
            for name, model in models.items():
                self.add_model(model, name)

        if (models and isinstance(models, list)):
            for model in models:
                self.add_model(model)

        self._file_name = None

    @property
    def dictionary(self):
        """Return dictionary of models."""
        return self._dict

    @property
    def models(self):
        """Return list of models."""
        return list(self._dict.values())

    @property
    def solved_models(self):
        """Return list of solved models."""
        models = []
        for model in list(self._dict.values()):
            if model.solved: models.append(model)

        return models

    @property
    def names(self):
        """Return list of models name (model key in dictionary)."""
        return list(self._dict.keys())

    def add_model(self, model, name=None):
        """Add new model to dictionary.

        add_model(model, name=None)

        Keyword arguments:
        model -- model object inherited from ModelBase class
        name -- name of model used as dictionary key and file name (default is None)
        """

        if name in self._dict.keys():
            raise KeyError('Model with key "{0}" already exist.'.format(name))

        if not name:
            if hasattr(self, '_model_index'): self._model_index += 1
            else: self._model_index = 0

            name = 'model_{0:0{1}d}'.format(self._model_index, MAXIMUM_NUMBER_OF_MODELS)

        self._dict[name] = model

    def delete_model(self, name):
        """Delete existing model in dictionary.

        delete_model(name)

        Keyword arguments:
        name -- name of model used as dictionary key
        """

        if name in self._dict.keys():
            del self._dict[name]

    def load(self, file_name):
        """Clear dictionary and load models from OptiLab data file (*.opt).

        load(file_name)

        Keyword arguments:
        file_name -- data file name
        """

        with zipfile.ZipFile(file_name, 'r') as zip_file:
            description = json.loads(zip_file.read('problem.desc').decode())
            problem = zip_file.read('problem.py').decode()

            exec(problem)
            self.model_class = eval(description["model"])

            self.clear()
            for file in sorted(zip_file.namelist()):
                if not file.endswith('pickle'): continue

                model = self.model_class()
                model.deserialize(zip_file.read(file))
                self._dict[os.path.splitext(file)[0]] = model

            zip_file.close()

        self._file_name = file_name

    def save(self, file_name, problem):
        """Save all models to OptiLab data file (*.opt).

        load(file_name, problem)

        Keyword arguments:
        file_name -- data file name
        problem -- Python script file name with model class definition
        """

        with zipfile.ZipFile(file_name, 'w', zipfile.ZIP_DEFLATED) as zip_file:
            for name, model in self._dict.items():
                zip_file.writestr('{0}.pickle'.format(name), model.serialize())

            description = {'model' : self.model_class.__name__}
            zip_file.writestr('problem.desc', json.dumps(description))
            zip_file.write(problem, arcname='problem.py')
            zip_file.close()

        self._file_name = file_name

    def solve(self, recalculate=False, save=False):
        """Solve model in directory.

        solve(recalculate=False, save=False)

        Keyword arguments:
        recalculate -- recalculate solved models (default is False)
        save -- save solved models to data file (default is False)
        """

        if save and not self._file_name:
            raise RuntimeError('Data file does not exist! Dictionary must be saved before solution.')

        if save:
            zip_file = zipfile.ZipFile(self._file_name, 'a', zipfile.ZIP_DEFLATED)

        for name, model in self._dict.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            model.create()
            model.solve()
            model.process()

            if save:
                zip_file.writestr('{0}.pickle'.format(name), model.serialize())

        if save: zip_file.close()

    def find_model_by_parameters(self, parameters):
        """Find and return model in dictionary by parameters.

        find_model_by_parameters(parameters)

        Keyword arguments:
        parameters -- list of model parameters
        """

        for name, model in self._dict.items():
            if (model.data.parameters == parameters): return model

    def clear(self):
        """Clear dictionary."""
        self._dict.clear()

class ModelDictionaryExternal(ModelDictionary):
    """Class inherited from ModelDictionary allows use external solver for models (default solver is agros2d_solver)."""

    def __init__(self, model_class=ModelBase, models=None):
        """Initialization of model dictionary.

        ModelDictionaryExternal(model_class=ModelBase, models=None)

        Keyword arguments:
        model_class -- general class for managed models (default is ModelBase)
        models -- list or dictionary in style {name : model} (default is None)
        """

        ModelDictionary.__init__(self, model_class, models)

        self.solver = "agros2d_solver"
        self.solver_parameters = ['-l', '-c']
        self._output = []

    @property
    def output(self):
        """Return output from external solver."""
        return self._output

    def solve(self, recalculate=False):
        """Solve model in directory.

        solve(recalculate=False, save=False, file_name=None)

        Keyword arguments:
        recalculate -- recalculate solved models (default is False)
        save -- save solved models to data file (default is False)
        """

        if not self._file_name:
            raise RuntimeError('Data file does not exist! Dictionary must be saved before solution.')

        code = 'from variant import ModelDictionary; md = ModelDictionary();'
        code += 'md.load("{0}");'.format(self._file_name)
        code += 'md.solve(recalculate={0}, save=True)'.format(recalculate)
        command = ['{0}'.format(self.solver)] + self.solver_parameters + ['{0}'.format(code)]

        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        self._output.append(process.communicate())
        self.load('{0}'.format(self._file_name))

if __name__ == '__main__':
    from variant.test_functions import quadratic_function
    import pythonlab

    md = ModelDictionary(quadratic_function.QuadraticFunction)
    md.solver = pythonlab.datadir('agros2d_solver')
    for x in range(100):
        model = quadratic_function.QuadraticFunction()
        model.set_parameter('x', x)
        md.add_model(model)

    file_name = '{0}.opt'.format(pythonlab.tempname())
    md.save(file_name, 'test_functions/quadratic_function.py')

    md.solve()
    results = []
    for model in md.models:
        results.append(model.get_variable('F'))

    md.clear()
    md.load(file_name)
    print('{0}/{1}'.format(len(md.models), len(md.solved_models)))

    md.solve(save=True)
    print('{0}/{1}'.format(len(md.models), len(md.solved_models)))

    md.clear()
    print('{0}/{1}'.format(len(md.models), len(md.solved_models)))
    md.load(file_name)
    print('{0}/{1}'.format(len(md.models), len(md.solved_models)))