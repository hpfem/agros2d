# import libraries
import pythonlab
import agros2d
import sys
from math import *

# interactive help hack
__pydoc_help__ = help
def __a2d_help__(obj = agros2d):
    return __pydoc_help__(obj)

help = __a2d_help__

# test
def test(text, value, normal, error = 0.03):
    if ((normal == 0.0) and (abs(value) < 1e-14)):
        return True
    test = abs((value - normal)/value) < error
    if (not test):    
        print(text + ": Agros2D: " + str(value) + ", correct: " + str(normal) + ", error: " + "{:.4f}".format(abs(value - normal)/value*100) + "%)")
    return test
setattr(agros2d, "test", test)

def agros2d_material_eval(keys):
    values = []
    for i in range(len(keys)):
        values.append(agros2d_material(keys[i]))

    return values