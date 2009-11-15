from math import *
from agros2d import *
from agros2file import *

import sys

class StdoutCatcher:
    def write(self, data):
        file = open(agrosstdout, "a")
        file.write(data)
        file.close()
 
        sys.__stdout__.write(data)
        sys.__stdout__.flush()

sys.stdout = StdoutCatcher()
