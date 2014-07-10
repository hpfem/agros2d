import os
import subprocess
import re

class ModelDict(object):
    def __init__(self):
        self._models = dict()
        self._directory = os.getcwd() + '/models'
        self._output = []

    @property
    def models(self):
        """ Models """
        return list(self._models.values())

    @property
    def solved_models(self):
        """ Solved models """
        models = []
        for model in list(self._models.values()):
            if model.solved: models.append(model)
        return models

    @property
    def directory(self):
        """ Directory """
        return self._directory

    @directory.setter
    def directory(self, value):
        if not os.path.isdir(value):
            try:
                os.makedirs(value)
            except OSError as exception:
                raise

        self._directory = os.path.abspath(value)

    @property
    def output(self):
        """ Solver output """
        return self._output

    def _model_file_name(self, name):
        return '{0}/{1}.pickle'.format(self.directory, name)

    def add_model(self, model, name=''):
        """ Add model to dictionary """
        if not name:
            if hasattr(self, '_name_index'):
                self._name_index += 1
            else:
                files = self.find_files('{0}/model_.*.pickle'.format(self.directory))
                self._name_index = 0
                for file_name in files:
                    name, extension = os.path.basename(file_name).split(".")
                    index = int(name.split("_")[1])
                    if (index >= self._name_index):
                        self._name_index = index + 1

            name = 'model_{0:06d}'.format(self._name_index)
            if os.path.isfile(self._model_file_name(name)):
                raise NameError('File exist!')

        self._models[name] = model

    def find_model(self, parameters):
        for name, model in self._models.items():
            if (model.parameters == parameters): return model

    def find_files(self, mask):
        """ Find existing model files """
        files = []
        if os.path.isdir(mask):
            files = os.listdir(mask)
        else:
            for file_name in os.listdir(os.path.dirname(mask)):
                if bool(re.match(r'^{0}$'.format(os.path.basename(mask)), file_name)): files.append(file_name)

        return files

    def load(self, model_class, mask=''):
        """ Load models """
        if not mask:
            mask = self.directory

        files = self.find_files(mask)
        for file_name in files:
            model = model_class()
            model.load('{0}/{1}'.format(self.directory, file_name))
            self._models[os.path.basename(file_name)] = model

    def save(self):
        """ Save models """
        for name, model in self._models.items():
            model.save(self._model_file_name(name))

    def solve(self, mask='', recalculate=False):
        """ Solve models """
        models = {}
        if not mask:
            models = self._models
        else:
            for name, model in self._models.items():
                if bool(re.match(r'^{0}$'.format(mask), name)): models[name] = model

        for name, model in models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            model.create()
            model.solve()
            model.process()
            model.solved = True
            model.save(self._model_file_name(name))

    def update(self):
        """ Update models """
        for name in list(self._models.keys()):
            self._models[name].load(self._model_file_name(name))

    def clear(self):
        """ Clear models """
        self._models.clear()

class ModelDictExternal(ModelDict):
    def __init__(self):
        ModelDict.__init__(self)
        self.solver = "agros2d_solver"
        self.solver_parameters = ['-l', '-c']

    def solve(self, mask='', recalculate=False):
        """ Solve models """
        models = {}
        if not mask:
            models = self._models
        else:
            for name, model in self._models.items():
                if bool(re.match(r'^{0}$'.format(mask), name)): models[name] = model

        for name, model in models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            code = "from problem import {0}; model = {0}();".format(type(model).__name__)
            code += "model.load('{0}/{1}.pickle');".format(self.directory, name)
            code += "model.create(); model.solve(); model.process();"
            code += "model.save('{0}/{1}.pickle')".format(self.directory, name)
            command = ['{0}'.format(self.solver)] + self.solver_parameters + ['{0}'.format(code)]

            process = subprocess.Popen(command, stdout=subprocess.PIPE)
            self._output.append(process.communicate())
            model.load('{0}/{1}.pickle'.format(self.directory, name))