# import libraries
import pythonlab
import sys
from math import *

# add actual directory to the path
sys.path.insert(0, ".")

# user functions
def sgn(number):
	return (number >= 0) and 1 or -1

# from rope.base.project import Project
# pythonlab_rope_project = Project(".", ropefolder=None)

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

# redirect stdout and stderr
class CatchOutErr:
    def write(self, str):
        pythonlab.__stdout__(str)

sys.stdout = CatchOutErr()
sys.stderr = CatchOutErr()
