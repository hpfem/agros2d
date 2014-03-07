import problem
from variant import model
import numpy as np


models = model.ModelDict()
models.parameters["AR1"] = "m"
models.parameters["AR2"] = "m"
models.parameters["AR3"] = "m"
models.parameters["AR4"] = "m"
models.parameters["AR5"] = "m"

i = 0
R = np.linspace(0.006, 0.01, 2)
for i1 in range(len(R)):
    for i2 in range(len(R)):
        for i3 in range(len(R)):
            for i4 in range(len(R)):
                for i5 in range(len(R)):
                    model = problem.Model()

                    model.parameters["AR1"] = R[i1]
                    model.parameters["AR2"] = R[i2]
                    model.parameters["AR3"] = R[i3]
                    model.parameters["AR4"] = R[i4]
                    model.parameters["AR5"] = R[i5]
                    
                    fn = "solutions/solution_{0:0{1}d}.rst".format(i, 5)
                    model.save(fn)                  
                    
                    i = i+1