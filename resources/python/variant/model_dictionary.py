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

        ModelDictionary(models=None, directory=None)

        Keyword arguments:
        model_class -- general class for managed models
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

    def models(self):
        """Return list of models."""
        return list(self._dict.values())

    def solved_models(self):
        """Return list of solved models."""
        models = []
        for model in list(self._dict.values()):
            if model.solved: models.append(model)
        return models

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
        """Clear dictionaty and load models from OptiLab data file (*.opt).

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

        solve(recalculate=False, save=False, file_name=None)

        Keyword arguments:
        recalculate -- recalculate solved models (default is False)
        save -- save solved models to data file (default is False)
        """

        for name, model in self._dict.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            model.create()
            model.solve()
            model.process()

            if save:
                # TODO: Extremely slow!
                if not self._file_name:
                    raise RuntimeError('Data file does not exist! Dictionary must be saved before solution.')

                with zipfile.ZipFile(self._file_name, 'a', zipfile.ZIP_DEFLATED) as zip_file:
                    zip_file.writestr('{0}.pickle'.format(name), model.serialize())
                    zip_file.close()

    def find_model_by_parameters(self, parameters):
        """Find and return model in dictionary by parameters.

        find_model_by_parameters(parameters)

        Keyword arguments:
        parameters -- list of model parameters
        """

        for name, model in self._dict.items():
            if (model.parameters == parameters): return model

    def clear(self):
        """Clear dictionary."""
        self._dict.clear()

class ModelDictionaryExternal(ModelDictionary):
    """Class inherited from ModelDictionary allows use external solver for models (default solver is agros2d_solver)."""

    def __init__(self, model_class=ModelBase, models=None):
        """Initialization of model dictionary.

        ModelDictionaryExternal(models=None, directory=None)

        Keyword arguments:
        model_class -- general class for managed models
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

        """
        for name, model in self._dict.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            code = "import sys; sys.path.insert(0, '{0}');".format(os.path.dirname(self._directory))
            code += "from problem import {0}; model = {0}();".format(type(model).__name__)
            code += "model.load('{0}/{1}.pickle');".format(self._directory, name)
            code += "model.create(); model.solve(); model.process();"
            code += "model.save('{0}/{1}.pickle')".format(self._directory, name)
            command = ['{0}'.format(self.solver)] + self.solver_parameters + ['{0}'.format(code)]

            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            self._output.append(process.communicate())
            model.load('{0}/{1}.pickle'.format(self._directory, name))
        """

if __name__ == '__main__':
    from variant.test_functions import quadratic_function
    import pythonlab

    md = ModelDictionary(quadratic_function.QuadraticFunction)
    for x in range(500):
        model = quadratic_function.QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    file_name = '{0}.opt'.format(pythonlab.tempname())
    md.save(file_name, 'test_functions/quadratic_function.py')

    md.solve()
    results = []
    for model in md.models():
        results.append(model.variables['F'])

    md.clear()
    md.load(file_name)
    print('{0}/{1}'.format(len(md.models()), len(md.solved_models())))
    md.solve(save=True)
    print('{0}/{1}'.format(len(md.models()), len(md.solved_models())))

    md.clear()
    print('{0}/{1}'.format(len(md.models()), len(md.solved_models())))
    md.load(file_name)
    print('{0}/{1}'.format(len(md.models()), len(md.solved_models())))