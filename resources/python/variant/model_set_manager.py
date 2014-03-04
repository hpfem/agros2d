#!/usr/bin/env python
from model import ModelBase
import os

class ModelSetManager:
    def __init__(self):
        self._solver = "agros2d_solver"
                                
    @property
    def solver(self):
        """Solver"""
        return self._solver

    @solver.setter
    def solver(self, values):
        self._solver = values
        
    @property
    def directory(self):
        """Working directory"""
        return self._directory

    @directory.setter
    def directory(self, value):
        self._directory = value
        
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
            name = self.directory + '/solution_' + strZeros + strIndex + '.rst'
            
        return name
            
        
    def findFiles(self):
        files = []
        for file in os.listdir(self.directory):
            if file.endswith(".rst"):
                files.append(self.directory + "/" + file)
        
        return files

    def solveProblem(self, file):
        str = "import sys; sys.path.insert(0, '{0}/..'); import problem; model = problem.Model(); model.load('{1}'); model.create(); model.solve(); model.process(); model.save('{1}');".format(os.path.dirname(os.path.abspath(file)), file)

        command = '"{0}" -l -c "{1}"'.format(self.solver, str)
        os.system(command)

    def solveAll(self, solveSolvedAgain = False):
        models = self.loadAll()
        for model in models:
            solveProblem = solveSolvedAgain or not model.solved
            if solveProblem:
                print("Solve problem: {0}".format(model.parameters)) 
                self.solveProblem(file)
            else:
                print("Problem: {0} allready solved".format(model.parameters)) 

    def loadAll(self):
        files = self.findFiles()
        models = []
        for file in files:
            model = ModelBase()
            model.load(file)
            model.fileName = file
            models.append(model)
            
        return models
        
    def saveAll(self, models):
        for model in models:
            try:
                fileName = model.filename
            except:
                fileName = self.generateFileName()
            
            model.save(fileName)
                
if __name__ == '__main__':
    solver = ModelSetManager()
    solver.solver = '/home/pkus/sources/agros2d/agros2d_solver'
    solver.directory = '/home/pkus/sources/agros2d/data/sweep/capacitor/solutions'
    solver.solveAll()
    
    models = solver.loadAll()
    for model in models:
        print model.parameters

    solver.generateFileName()
    
    solver.saveAll(models)