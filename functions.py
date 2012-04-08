# import libraries
import pythonlab
import agros2d
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
		print(text + ": Agros2D: " + str(value) + ", correct: " + str(normal) + ")")
	return test

from rope.base.project import Project as PythonLabRopeProject
pythonlab_rope_project = PythonLabRopeProject(".", ropefolder=None)

# get completion list
def python_engine_get_completion_string(code, offset):
    from rope.contrib import codeassist

    proposals = codeassist.code_assist(pythonlab_rope_project, code, offset, maxfixes=20)
    # proposals = codeassist.sorted_proposals(proposals)
    proposals_string = []
    for p in proposals:
        proposals_string.append(p.__str__())
    
    return proposals_string
    # return [proposal.name for proposal in proposals]

def python_engine_get_completion_string_dot(code):
    try:      
        return dir(eval(code))
    except:
        return []

def python_engine_get_completion_file(filename, offset):
    from rope.contrib import codeassist

    f = open(filename, 'r')
    code = ''.join(f.readlines())

    proposals_string = []
    try:
        proposals = codeassist.code_assist(pythonlab_rope_project, code, offset, maxfixes=20) 
        # proposals = codeassist.sorted_proposals(proposals)        
        for p in proposals:
            proposals_string.append(p.__str__())
        
        return proposals_string
        # return [proposal.name for proposal in proposals]
    except:
        return []

def python_engine_pyflakes_check(filename):
    f = open(filename, 'r')
    code = ''.join(f.readlines())

    # first, compile into an AST and handle syntax errors.
    try:
        import _ast
        tree = compile(code, "", "exec", _ast.PyCF_ONLY_AST)
    except SyntaxError, value:
        msg = value.args[0]

        (lineno, offset, text) = value.lineno, value.offset, value.text

        line = text.splitlines()[-1]

        if offset is not None:
            offset = offset - (len(text) - len(line))

            return ['%s:%d: %s' % ("", lineno, msg)]

        return 1
    else:
        # okay, it's syntactically valid. Now check it.
        import pyflakes.checker as checker

        w = checker.Checker(tree, "")
        w.messages.sort(lambda a, b: cmp(a.lineno, b.lineno))       
        return [warning.__str__() for warning in w.messages]

# redirect std output
class StdoutCatcher:
    def write(self, str):
        pythonlab.__stdout__(str)

sys.stdout = StdoutCatcher()
