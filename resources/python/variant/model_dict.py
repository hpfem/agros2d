import os
import subprocess
import re

class ModelDict:
    def __init__(self, models=None, directory=None):
        """ModelDict class create colection of models.
        
        ModelDict(models=None, directory=None)
        
        Keyword arguments:
        models -- list or dictionary (style {name : model}) contain models (default is None)
        directory -- current working directory (default is None)
        """

        self._dict = dict()

        if (models and isinstance(models, dict)):
            for name, model in models.items():
                self.add_model(model, name)

        if (models and isinstance(models, list)):
            for model in models:
                self.add_model(model)
        
        if not directory:
            self.directory = '{0}/models/'.format(os.getcwd())
        else:
            self.directory = directory

    @property
    def dict(self):
        """Return models dictionary."""
        return self._dict

    @property
    def models(self):
        """Return list of models in dictionary."""
        return list(self._dict.values())

    @property
    def solved_models(self):
        """Return list of solved models in dictionary."""
        models = []
        for model in list(self._dict.values()):
            if model.solved: models.append(model)
        return models

    @property
    def directory(self):
        """Current working directory."""
        return self._directory

    @directory.setter
    def directory(self, value):
        if not os.path.isdir(value):
            try:
                os.makedirs(value)
            except OSError:
                raise IOError('Directory "{0}" can not be created.'.format(value))

        self._directory = os.path.abspath(value)

    def _model_file_name(self, name):
        return '{0}/{1}.pickle'.format(self._directory, name)

    def _find_last_model_index(self):
        files = self.find_files('{0}/model_.*.pickle'.format(self._directory))
        for file_name in files:
            name, extension = os.path.basename(file_name).split(".")
            index = int(name.split("_")[1])
            if (index >= self._name_index):
                self._model_index = index

    def add_model(self, model, name=None, resume=False):
        """Add new model to dictionary.

        add_model(model, name=None, resume=False)

        Keyword arguments:
        model -- model object inherited from ModelBase class
        name -- name of model used as dictionary key and file name (default means automatic model name)
        resume -- resume in model index counting for model name (default is False)
        """

        if (resume and not name):
            self._model_index = self._find_last_model_index() + 1

        if not name:
            if hasattr(self, '_model_index'):
                self._model_index += 1
            else:
                self._model_index = 0
            name = 'model_{0:06d}'.format(self._model_index)

        self._dict[name] = model

    def delete_model(self, name):
        """Delete existing model in dictionary.

        delete_model(name)

        Keyword arguments:
        name -- name of model used as dictionary key
        """
        if name in self._dict.keys():
            del self._dict[name]

    def find_files(self, mask):
        """Find and return list of model files in directory.

        find_files(mask)

        Keyword arguments:
        mask -- regular expression for directory or files
        """

        files = []
        if os.path.isdir(mask):
            files = os.listdir(mask)
        else:
            files_list = os.listdir(os.path.dirname(mask))
            expression  = r'^{0}$'.format(os.path.basename(mask))
            for file_name in files_list:
                if bool(re.match(expression, file_name)): files.append(file_name)

        return files

    def load(self, model_class, mask=''):
        """Load models from directory to dictionary.

        load(model_class, mask='')

        Keyword arguments:
        model_class -- class inherited from ModelBase class
        mask -- regular expression for directory or files
        """

        if not mask:
            mask = self._directory
        elif not os.path.dirname(mask):
            mask = '{0}/{1}'.format(self._directory, mask)
        elif (os.path.abspath(os.path.dirname(mask)) != self._directory and
              len(self.models)):
            raise RuntimeError('Mask do not match with current working directory.')

        files = self.find_files(mask)
        for file_name in files:
            model = model_class()
            model.load('{0}/{1}'.format(self._directory, file_name))
            name, extension = os.path.basename(file_name).split(".")
            self._dict[name] = model

    def save(self):
        """Save models from dictionary to current working directory."""
        for name, model in self._dict.items():
            model.save(self._model_file_name(name))

    def solve(self, mask='', recalculate=False, save=True):
        """Solve model in directory.

        solve(mask='', recalculate=False)

        Keyword arguments:
        mask -- regular expression for model keys (in default solve all models)
        recalculate -- recalculate solved models (default is False)
        """

        models = {}
        if not mask:
            models = self._dict
        else:
            expression  = r'^{0}$'.format(mask)
            for name, model in self._dict.items():
                if bool(re.match(expression, name)): models[name] = model

        for name, model in models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            model.create()
            model.solve()
            model.process()

            if save: model.save(self._model_file_name(name))

    def find_model_by_parameters(self, parameters):
        """Find and return model in dictionary by parameters.

        find_model_by_parameters(parameters)

        Keyword arguments:
        parameters -- list of model parameters
        """

        for name, model in self._dict.items():
            if (model.parameters == parameters): return model

    def update(self):
        """Update dictionary from models in current working dictionary."""
        for name in list(self._dict.keys()):
            self._dict[name].load(self._model_file_name(name))

    def clear(self):
        """Clear dictionary."""
        self._dict.clear()

class ModelDictExternal(ModelDict):
    def __init__(self):
        ModelDict.__init__(self)
        self.solver = "agros2d_solver"
        self.solver_parameters = ['-l', '-c']
        self._output = []

    @property
    def output(self):
        """Output of external solver."""
        return self._output

    def solve(self, mask='', recalculate=False):
        """Solve model in directory.

        solve(mask='', recalculate=False)

        Keyword arguments:
        mask -- regular expression for model keys (in default solve all models)
        recalculate -- recalculate solved models (default is False)
        """

        models = {}
        if not mask:
            models = self._dict
        else:
            expression  = r'^{0}$'.format(mask)
            for name, model in self._dict.items():
                if bool(re.match(expression, name)): models[name] = model

        for name, model in models.items():
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

if __name__ == '__main__':
    from test_suite.optilab.examples import quadratic_function
    md = ModelDict()
    for x in range(10000):
        model = quadratic_function.QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    md.save()
    md.clear()
    md.load(quadratic_function.QuadraticFunction)
    md.solve(save=True)
