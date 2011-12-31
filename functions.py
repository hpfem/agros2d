# import libraries
import pythonlab
from agros2d import *
from agros2file import *
from math import *
import sys

# add actual directory to the path
sys.path.append(".")

# user functions
def sgn(number):
	return (number >= 0) and 1 or -1 

def addsemicircle(x0, y0, radius, boundary = "none", material = "null"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, boundary=boundary, angle=90)
	addedge((x0)+radius, y0, x0, (y0)+radius, boundary=boundary, angle=90)
	addedge(x0, (y0)+radius, x0, (y0)-radius, boundary=boundary, angle=0)
	if (material != "null"):
		addlabel((x0)+(radius/2.0), y0, 0, 0, material=material)
	return

def addcircle(x0, y0, radius, boundary = "none", material = "null"):
	addedge(x0, (y0)-radius, (x0)+radius, y0, boundary=boundary, angle=90)
	addedge((x0)+radius, y0, x0, (y0)+radius, boundary=boundary, angle=90)
	addedge(x0, (y0)+radius, (x0)-radius, y0, boundary=boundary, angle=90)
	addedge((x0)-radius, y0, x0, (y0)-radius, boundary=boundary, angle=90)
	if (material != "null"):
		addlabel(x0, y0, 0, 0, material=material)
	return

def addrect(x0, y0, width, height, boundary = "none", material = "null"):
	addedge(x0, y0, (x0)+width, y0, boundary=boundary)
	addedge(x0+width, y0, (x0)+width, (y0)+height, boundary=boundary)
	addedge(x0+width, (y0)+height, x0, (y0)+height, boundary=boundary)
	addedge(x0, (y0)+height, x0, y0, boundary=boundary)
	if (material != "null"):
		addlabel((x0)+(width/2.0), (y0)+(height/2.0), material=material)
	return

def test(text, value, normal, error = 0.03):
	if ((normal == 0.0) and abs(value < 1e-14)):
		return True
	test = abs((value - normal)/value) < error
	if (not test):	
		print(text + ": (" + str(value) + " != " + str(normal) + ")")
	return test

# get completion list
def python_engine_get_completion_string(code, offset):
	try:
		from rope.base.project import Project
		from rope.contrib import codeassist
	
		project = Project(".", ropefolder=None)	

		proposals = codeassist.code_assist(project, code, offset)
		proposals = codeassist.sorted_proposals(proposals)
		proposals_string = []
		for p in proposals:
			proposals_string.append(p.__str__())
		
		return proposals_string
		# return [proposal.name for proposal in proposals]
	except:
		return []

def python_engine_get_completion_file(filename, offset):
	try:
		from rope.base.project import Project
		from rope.contrib import codeassist
	
		project = Project(".", ropefolder=None)	
	
		f = open(filename, 'r')
		code = ''.join(f.readlines())
	
		proposals = codeassist.code_assist(project, code, offset, maxfixes=10) 
		proposals = codeassist.sorted_proposals(proposals)
		proposals_string = []
		for p in proposals:
			proposals_string.append(p.__str__())
		
		return proposals_string
		# return [proposal.name for proposal in proposals]
	except:
		return []

# redirect std output
class StdoutCatcher:
    def write(self, str):
        pythonlab.stdout(str)

sys.stdout = StdoutCatcher()
