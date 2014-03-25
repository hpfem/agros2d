#!/usr/bin/env python

import subprocess
import glob
import os
import pythonlab

from model import ModelBase

class ModelSetManager(object):
    def __init__(self):
        self._solver = "agros2d_solver"
        self._directory = "solutions"
        self._suffix = ".rst"
        self._errors = list()

        self._problem = "problem"
        self._class = "Model"

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
        self._directory = directory

    @property
    def suffix(self):
        """ Solution file suffix """
        return self._suffix

    @suffix.setter
    def suffix(self, suffix):
        self._suffix = suffix

    #todo: rethink
    def generateFileName(self):
        numberLen = 5
        files = self.findFiles()
        name = ''
        index = -1
        while name == '' or name in files:
            index += 1
            strIndex = str(index)
            strZeros = '0' * (numberLen - len(strIndex))
            name = self.directory + 'solution_' + strZeros + strIndex + '.rst'

        return name

    def findFiles(self, mask = '*.rst'):
        files = list()
        for file_name in glob.glob('{0}/{1}'.format(self.directory, mask)):
          files.append(file_name)
          
        return files

    def solveProblem(self, file):
        path = os.path.dirname(os.path.abspath(file))

        code = "import sys; sys.path.insert(0, '{0}/..');".format(path)
        code += "import {0}; model = {0}.{1}();".format(self._problem, self._class)
        code += "model.load('{0}');model.create(); model.solve(); model.process(); model.save('{0}');".format(file)

        command = "{0} -l -c {1}".format(self.solver, code)
        process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
        self._errors.append(process.communicate()[1])

    def solveAll(self, solveSolvedAgain = False):
        files = self.findFiles()
        totalSolved = 0
        for file in files:
            model = ModelBase()
            model.load(file)
            solveProblem = solveSolvedAgain or not model.solved
            if solveProblem:
                self.solveProblem(file)
                totalSolved += 1

        return totalSolved

    def loadAll(self):
        files = self.findFiles()
        models = []
        for file in files:
            model = ModelBase()
            model.load(file)
            models.append(model)

        return models

    def saveAll(self, models):
        for model in models:
            try:
                fileName = model.fileName
            except:
                fileName = self.generateFileName()
            
            model.save(fileName)

    def deleteAll(self):
        files = self.findFiles()
        for file in files:
            os.remove(file)

def generateTestFiles():
    import numpy as np
    L = np.linspace(0.01, 0.04, 5)
    for i in range(len(L)):
        model = ModelBase()

        model.parameters["R1"] = 0.01
        model.parameters["R2"] = 0.03
        model.parameters["R3"] = 0.05
        model.parameters["R4"] = 0.06
        model.parameters["L"] = L[i]

        fn = pythonlab.datadir('resources/python/variant/test_set_manager/solutions/solution_{0:0{1}d}.rst'.format(i, 5))
        model.save(fn)

if __name__ == '__main__':
    solver = ModelSetManager()
    solver.solver = pythonlab.datadir('agros2d_solver')
    solver.directory = pythonlab.datadir('resources/python/variant/test_set_manager/solutions/')
    solver.deleteAll()
    generateTestFiles()
    solver.solveAll(True)
    models = solver.loadAll()
    for model in models:
        print model.parameters, model.variables, model.solved

    solver.saveAll(models)