import pythonlab
import numpy as np
import pylab as pl

from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm

from variant import optimization
from variant.optimization import genetic
from variant.optimization.genetic.method import ModelGenetic

class RosenbrockFunction(ModelGenetic):
    def inform(self):
        self.model_info.add_parameter('a', float)
        self.model_info.add_parameter('b', float)
        self.model_info.add_parameter('x', float)
        self.model_info.add_parameter('y', float)

        self.model_info.add_variable('F', float)

    def create(self):
        self.defaults['a'] = 1
        self.defaults['b'] = 100

    def solve(self):
        try:
            self.F = (self.parameters['a'] - self.parameters['x'])**2 +\
                     self.parameters['b']*(self.parameters['y'] - self.parameters['x']**2)**2
            self.solved = True
        except:
            self.solved = False

    def process(self):
        self.variables['F'] = self.F

if __name__ == '__main__':
    """ optimization """
    parameters = optimization.Parameters([optimization.ContinuousParameter('x', -2.0, 2.0),
                                          optimization.ContinuousParameter('y', -2.0, 5.0)])
    
    functionals = optimization.Functionals([optimization.Functional("F", "min")])
    optimization = genetic.GeneticOptimization(parameters, functionals, RosenbrockFunction)
    
    optimization.population_size = 200
    
    optimization.selection_ratio = 8.0/10.0
    optimization.elitism_ratio = 1.0/5.0
    optimization.crossover_ratio = 1.0
    optimization.mutation_ratio = 1.0/10.0
    
    number_of_populations = 10
    optimization.run(number_of_populations, save = False)
    optimization.model_dict.save_to_zip(problem = 'Rosenbrock.py', filename = 'Rosenbrock.opt')
        
    star = optimization.find_best(optimization.model_dict.models())
    print('F = {0}; x = {1}, y = {2}'.format(star.variables['F'],
                                             star.parameters['x'], star.parameters['y']))
    print('F(a,a**2) = 0'.format((1 - 1)**2 + 100*(1 - 1**2)**2))
    
    """ results """
    """
    pl.figure()
    for population_index in range(number_of_populations):
        color = cm.jet(population_index/float(number_of_populations), 1)
    
        population = optimization.population(population_index)
        for model in population:
            pl.plot(model.parameters['x'], model.parameters['y'], color=color, marker='o')
    
        population_star = optimization.find_best(population)
        pl.plot(population_star.parameters['x'], population_star.parameters['x'], 'sk',
                mfc="None", mew=2, ms=10)
    
    pl.plot(star.parameters['a'],
            star.parameters['a']**2, 'dk', mfc="None", mew=3, ms=15)
    
    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=90, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)
    """
    """
    pl.figure()
    for population_index in range(number_of_populations):
        population = optimization.sorted_population(population_index)
        for genom in population:
            color = cm.jet(population.index(genom)/float(len(population)), 1)
            pl.plot(population_index, genom.variables['F'], 'o', color=color)
    
    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=90, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)
    """
    """
    pl.figure()
    for population_index in range(number_of_populations):
        pl.plot(population_index, optimization.find_best(optimization.population(population_index)).variables['F'], 'sk',
                mfc='None', mew=2, ms=10)
    
    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=90, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)
    """
    """
    X = np.arange(-2.0, 2.0, 0.25)
    Y = np.arange(-2.0, 5.0, 0.25)
    X, Y = np.meshgrid(X, Y)
    F = (1 - X)**2 + 100*(Y - X**2)**2
    
    pl.figure()
    axes = pl.axes(projection='3d')
    #axes.plot_surface(X, Y, F, rstride=1, cstride=1, linewidth=0, cmap=cm.jet)
    axes.plot_wireframe(X, Y, F, cmap=cm.jet)
    
    for population in range(populations):
        color = cm.hsv(population/float(populations), 1)
        for model in optimization.population(population):
            axes.scatter(model.parameters['x'], model.parameters['y'], model.variables['F'],
                         color=color, marker='o')
    
    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=100, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)
    """