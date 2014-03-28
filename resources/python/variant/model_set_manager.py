#!/usr/bin/env python

from model import ModelBase
import subprocess, glob, os

class ModelSetManager(object):
    def __init__(self):
        self._solver = "agros2d_solver"
        self._directory = "solutions"

        self.problem_module = "problem"
        self.model_class = "Model"

        self._output = []

    @property
    def solver(self):
        """ Solver """
        return self._solver

    @solver.setter
    def solver(self, solver):
        self._solver = solver

    @property
    def directory(self):
        """ Working directory """
        return self._directory

    @directory.setter
    def directory(self, directory):
        if not os.path.isdir(directory):
          os.makedirs(directory)
        self._directory = directory

    @property
    def output(self):
        """ Solver output """
        return self._output

    #todo: rethink
    def generate_file_name(self):
        numberLen = 5
        files = self.find_files()
        name = ''
        index = -1
        while name == '' or name in files:
            index += 1
            strIndex = str(index)
            strZeros = '0' * (numberLen - len(strIndex))
            name = self.directory + 'solution_' + strZeros + strIndex + '.rst'

        return name

    def find_files(self, mask = '*.rst'):
        files = []
        for file_name in glob.glob('{0}/{1}'.format(self.directory, mask)):
          files.append(file_name)
        return files

    def solve_problem(self, file):
        path = os.path.dirname(os.path.abspath(file))

        code = "import sys; sys.path.insert(0, '{0}/..');".format(path)
        code += "import {0}; model = {0}.{1}();".format(self.problem_module, self.model_class)
        code += "model.load('{0}');model.create(); model.solve(); model.process(); model.save('{0}');".format(file)
        
        command = ['{0}'.format(self.solver), '-l', '-c', '{0}'.format(code)]
        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        self._output.append(process.communicate())

    def solve_all(self, solveSolvedAgain = False):
        files = self.find_files()
        totalSolved = 0
        for file in files:
            model = ModelBase()
            model.load(file)
            solveProblem = solveSolvedAgain or not model.solved
            if solveProblem:
                self.solve_problem(file)
                totalSolved += 1

        return totalSolved

    def load_all(self, mask = '*.rst'):
        files = self.find_files(mask)
        models = []
        for file in files:
            model = ModelBase()
            model.load(file)
            models.append(model)

        return models

    def save_all(self, models):
        for model in models:
            try:
                fileName = model.fileName
            except:
                fileName = self.generate_file_name()
            model.save(fileName)

    def delete_all(self):
        files = self.find_files()
        for file in files:
            os.remove(file)
