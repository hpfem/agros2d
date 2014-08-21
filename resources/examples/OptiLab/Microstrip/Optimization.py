from variant import optimization
from variant.optimization import genetic
from problem import Model

parameters = optimization.Parameters([optimization.ContinuousParameter('W', 1e-4, 1e-3),
                                      optimization.ContinuousParameter('d', 1e-4, 1e-3)])

functionals = optimization.Functionals([optimization.Functional("F", "min")])

optimization = genetic.GeneticOptimization(parameters, functionals, Model)

#from variant import ModelDictExternal
#optimization.model_dict = ModelDictExternal()
#optimization.model_dict.solver = '{0}/agros2d_solver'.format(pythonlab.datadir())

optimization.population_size = 20
optimization.run(5, False)

star = optimization.find_best(optimization.model_dict.models())
print('Z0 = {0} Ohm (required {1} Ohm)'.format(star.variables['Z0'], star.parameters['Z0']))
print('Best variant parameters: W={0}, d={1}'.format(star.parameters['W'], star.parameters['d']))
