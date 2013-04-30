# import libraries
import pythonlab
import agros2d
import sys
from math import *

def test(text, value, normal, error = 0.03):
    if ((normal == 0.0) and abs(value < 1e-14)):
        return True
    test = abs((value - normal)/value) < error
    if (not test):    
        print(text + ": Agros2D: " + str(value) + ", correct: " + str(normal) + ")")
    return test
setattr(agros2d, "test", test)
