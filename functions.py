# import libraries
from agros2d import *
from agros2file import *
from math import *
import sys

# add actual directory to the path
sys.path.append(".")

# consts
MU0 = 4*pi*1e-7
EPS0 = 8.854e-12

# user functions
def addsemicircle(x0, y0, radius, marker = "none"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, 90, marker)
	addedge((x0)+radius, y0, x0, (y0)+radius, 90, marker)
	addedge(x0, (y0)+radius, x0, (y0)-radius, 0, marker)
	return

def addcircle(x0, y0, radius, marker = "none"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, 90, marker)
	addedge((x0)+radius, y0, x0, (y0)+radius, 90, marker)
	addedge(x0, (y0)+radius, (x0)-radius, y0, 90, marker)
	addedge((x0)-radius, y0, x0, (y0)-radius, 90, marker)
	return

def addrect(x0, y0, width, height, marker = "none"):
	addedge(x0, y0, (x0)+width, y0, 0, marker)
	addedge(x0+width, y0, (x0)+width, (y0)+height, 0, marker)
	addedge(x0+width, (y0)+height, x0, (y0)+height, 0, marker)
	addedge(x0, (y0)+height, x0, y0, 0, marker)
	return

def agroslistvariables():
	import types
	agrosType = [types.IntType, types.StringType, types.TupleType, types.FloatType, types.BooleanType, types.ListType, types.LongType, types.UnicodeType]

	return filter(lambda x: type(x) in [types.IntType, types.DictType], globals().values())

def test(text, value, normal, error = 0.03):
	if ((normal == 0.0) and abs(value < 1e-14)):
		return True
	test = abs((value - normal)/value) < error
	if (not test):	
		print(text + ": (" + str(value) + " != " + str(normal) + ")")
	return test

# redirect script output
class StdoutCatcher:
	def write(self, str):
		capturestdout(str)

sys.stdout = StdoutCatcher()
