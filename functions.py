from math import *
from agros2d import *
from agros2file import *

import sys

class StdoutCatcher:
	def write(self, str):
		capturestdout(str)

sys.stdout = StdoutCatcher()
