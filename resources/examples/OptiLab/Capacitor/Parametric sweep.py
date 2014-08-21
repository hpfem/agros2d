from variant import ModelDict, ModelGenerator
from problem import Model

import pythonlab

tempdir = pythonlab.tempname()

# create generator
mg = ModelGenerator(Model)
# add parameters
mg.add_parameter_by_interval('w1', 0.02, 0.04, 0.01)
mg.add_parameter_by_interval('w2', 0.02, 0.04, 0.01)
mg.add_parameter_by_interval('l', 0.03, 0.07, 0.01)
# make combinations and save to the directory
mg.combination()
mg.save(tempdir)

# create model directory
md = ModelDict(mg.dict.models())

# solve and save problems
md.directory = tempdir
md.solve(save=True)