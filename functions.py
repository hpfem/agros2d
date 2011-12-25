# import libraries
from python_engine import *
from agros2d import *
from agros2file import *
from math import *
import sys

# add actual directory to the path
sys.path.append(".")

# user functions
def sgn(number):
	return (number >= 0) and 1 or -1 

def addsemicircle(x0, y0, radius, marker = "none", label = "null"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, 90, marker)
	addedge((x0)+radius, y0, x0, (y0)+radius, 90, marker)
	addedge(x0, (y0)+radius, x0, (y0)-radius, 0, marker)
	if (label != "null"):
		addlabel((x0)+(radius/2.0), y0, 0, 0, label)
	return

def addcircle(x0, y0, radius, marker = "none", label = "null"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, 90, marker)
	addedge((x0)+radius, y0, x0, (y0)+radius, 90, marker)
	addedge(x0, (y0)+radius, (x0)-radius, y0, 90, marker)
	addedge((x0)-radius, y0, x0, (y0)-radius, 90, marker)
	if (label != "null"):
		addlabel(x0, y0, 0, 0, label)
	return

def addrect(x0, y0, width, height, marker = "none", label = "null"):
	addedge(x0, y0, (x0)+width, y0, 0, marker)
	addedge(x0+width, y0, (x0)+width, (y0)+height, 0, marker)
	addedge(x0+width, (y0)+height, x0, (y0)+height, 0, marker)
	addedge(x0, (y0)+height, x0, y0, 0, marker)
	if (label != "null"):
		addlabel((x0)+(width/2.0), (y0)+(height/2.0), 0, 0, label)
	return

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
