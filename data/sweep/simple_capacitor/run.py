from variant.optimization import ContinuousParameter, Functionals, Functional
from variant.genetic import GeneticOptimization
import pythonlab

parameters = [ContinuousParameter('left', 0, 0.02),
                ContinuousParameter('right', 0.03, 0.05),
                ContinuousParameter('bottom', 0, 0.02),
                ContinuousParameter('top', 0.03, 0.05),
                ContinuousParameter('eps', 5, 10)]

functionals = Functionals([Functional("C", "max")])

optimization = GeneticOptimization(parameters, functionals)
optimization.directory = pythonlab.datadir('/data/sweep/simple_capacitor/solutions/')
optimization.modelSetManager.solver = pythonlab.datadir('agros2d_solver')
optimization.populationSize = 25
optimization.run(10, False)         