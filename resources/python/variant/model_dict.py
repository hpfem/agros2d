from glob import glob
from os.path import abspath, dirname, basename, splitext, isdir
from os import makedirs, getcwd
from subprocess import Popen, PIPE

class ModelDict(object):
    def __init__(self):
        self._models = dict()
        self._directory = getcwd()
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
        if not isdir(value):
            try:
                makedirs(value)
            except OSError as exception:
                raise

        self._directory = abspath(value)

    @property
    def output(self):
        """ Solver output """
        return self._output

    def add_model(self, model, file_name=''):
        """ Add model to dictionary """
        if not file_name:
            if hasattr(self, '_file_name_index'):
                self._file_name_index += 1
            else:
                self._file_name_index = len(self.find_files('solution_*.pickle'))

            file_name = 'solution_{0:06d}.pickle'.format(self._file_name_index)

        self._models[file_name] = model

    def find_model(self, parameters):
        for file_name, model in self._models.items():
            if (model.parameters == parameters): return model

    def find_files(self, mask):
        """ Find existing model files """
        if isdir(abspath(mask)):
            mask = '{0}/*.pickle'.format(mask)

        files = []
        for file_name in glob(mask):
            files.append(file_name)
        return sorted(files)

    def load(self, model_class, mask=''):
        """ Load models """
        if not mask:
            mask = '{0}/*.pickle'.format(self.directory)
        else:
            self.directory = abspath(dirname(mask))

        files = self.find_files(mask)
        for file_name in files:
            model = model_class()
            model.load(file_name)
            self._models[splitext(basename(file_name))[0]] = model

    def save(self):
        """ Save models """
        for file_name, model in self._models.items():
            model.save('{0}/{1}'.format(self.directory, file_name))

    def solve(self, recalculate=False):
        """ Solve models """
        for file_name, model in self._models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            model.create()
            model.solve()
            model.process()
            model.save('{0}/{1}'.format(self.directory, file_name))

    def update(self):
        """ Update models """
        for file_name in list(self._models.keys()):
            self._models[file_name].load('{0}/{1}'.format(self.directory, file_name))

    def clear(self):
        """ Clear models """
        self._models.clear()

class ModelDictExternal(ModelDict):
    def __init__(self):
        ModelDict.__init__(self)
        self.solver = "agros2d_solver"
        self.solver_parameters = ['-l', '-c']

    def solve(self, recalculate=False):
        """ Solve models """
        for file_name, model in self._models.items():
            solve_model = recalculate or not model.solved
            if not solve_model: continue

            code = "from problem import {0}; model = {0}();".format(type(model).__name__)
            code += "model.load('{0}/{1}');".format(self.directory, file_name)
            code += "model.create(); model.solve(); model.process();"
            code += "model.save('{0}/{1}')".format(self.directory, file_name)
            command = ['{0}'.format(self.solver)] + self.solver_parameters + ['{0}'.format(code)]

            process = Popen(command, stdout=PIPE)
            self._output.append(process.communicate())
            model.load('{0}/{1}'.format(self.directory, file_name))