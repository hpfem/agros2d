from variant import ModelDict, ModelGenerator
from problem import Model

mg = ModelGenerator(Model)
mg.add_parameter_by_interval('W', 1e-4, 5e-4, 1e-4)
mg.add_parameter_by_interval('d', 1e-4, 5e-4, 1e-4)
mg.combination()

md = ModelDict(mg.dict.models)
md.solve()
