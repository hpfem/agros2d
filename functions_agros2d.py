# import libraries
import pythonlab
import agros2d
import sys
from math import *

def addsemicircle(x0, y0, radius, boundary = "none", material = "null"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, boundary=boundary, angle=90)
	addedge((x0)+radius, y0, x0, (y0)+radius, boundary=boundary, angle=90)
	addedge(x0, (y0)+radius, x0, (y0)-radius, boundary=boundary, angle=0)
	if (material != "null"):
		addlabel((x0)+(radius/2.0), y0, 0, 0, material=material)
	return
setattr(agros2d, "addsemicircle", addsemicircle)

def addcircle(x0, y0, radius, boundary = "none", material = "null"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, boundary=boundary, angle=90)
	addedge((x0)+radius, y0, x0, (y0)+radius, boundary=boundary, angle=90)
	addedge(x0, (y0)+radius, (x0)-radius, y0, boundary=boundary, angle=90)
	addedge((x0)-radius, y0, x0, (y0)-radius, boundary=boundary, angle=90)
	if (material != "null"):
		addlabel(x0, y0, 0, 0, material=material)
	return
setattr(agros2d, "addcircle", addcircle)

def addrect(x0, y0, width, height, boundary = "none", material = "null"):
	addedge(x0, y0, (x0)+width, y0, boundary=boundary)
	addedge(x0+width, y0, (x0)+width, (y0)+height, boundary=boundary)
	addedge(x0+width, (y0)+height, x0, (y0)+height, boundary=boundary)
	addedge(x0, (y0)+height, x0, y0, boundary=boundary)
	if (material != "null"):
		addlabel((x0)+(width/2.0), (y0)+(height/2.0), material=material)
	return
setattr(agros2d, "addrect", addrect)

def test(text, value, normal, error = 0.03):
	if ((normal == 0.0) and abs(value < 1e-14)):
		return True
	test = abs((value - normal)/value) < error
	if (not test):	
		print(text + ": Agros2D: " + str(value) + ", correct: " + str(normal) + ")")
	return test
setattr(agros2d, "test", test)
