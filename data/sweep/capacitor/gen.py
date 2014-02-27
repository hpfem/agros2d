import os

import problem
import numpy as np

L = np.linspace(0.01, 0.04, 5)
for i in range(len(L)):
    model = problem.Model()
    
    model.parameters["R1"] = 0.01
    model.parameters["R2"] = 0.03
    model.parameters["R3"] = 0.05
    model.parameters["R4"] = 0.06
    model.parameters["L"] = L[i]
    
    fn = "solutions/solution_{0:0{1}d}.rst".format(i, 5)
    model.save(fn)    