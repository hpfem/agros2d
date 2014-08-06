from variant import optimization
from variant.optimization import genetic
from problem import Model

parameters = optimization.Parameters([optimization.ContinuousParameter('W', 1e-4, 1e-3),
                                      optimization.ContinuousParameter('d', 1e-4, 1e-3)])

functionals = optimization.Functionals([optimization.Functional("F", "min")])

optimization = genetic.GeneticOptimization(parameters, functionals, Model)

optimization.population_size = 20
optimization.run(5, False)

optimum = optimization.find_best(optimization.model_dict.models)
print(optimum.variables['Z0'])