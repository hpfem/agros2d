#!/usr/bin/env python

import os

class Solver:
    def __init__(self):
        self._solver = "agros2d_solver"
                                
    @property
    def solver(self):
        """Solver"""
        return self._solver

    @solver.setter
    def solver(self, values):
        self._solver = values
        
    def find_problems(self, dir):
        files = []
        for file in os.listdir(dir):
            if file.endswith(".rst"):
                files.append(dir + "/" + file)
        
        return files

    def solve_problem(self, file):
        str = "import sys; sys.path.insert(0, '{0}/..'); import problem; model = problem.Model(); model.load('{1}'); model.create(); model.solve(); model.process(); model.save('{1}');".format(os.path.dirname(os.path.abspath(file)), file)

        command = '"{0}" -l -c "{1}"'.format(self.solver, str)
        os.system(command)

    def solve_problems(self, dir):
        files = self.find_problems(dir)
        for file in files:
            print("Solve problem: {0}".format(file)) 
            self.solve_problem(file)

if __name__ == '__main__':
    solver = Solver()
    solver.solver = '/home/karban/Projects/agros2d/agros2d_solver'
    solver.solve_problems("/home/karban/Projects/agros2d/data/sweep/capacitor/solutions")
