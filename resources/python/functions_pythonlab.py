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
def python_engine_get_completion_script(script, row, column, filename = None):
	import jedi
	
	jedi.settings.additional_dynamic_modules = [pythonlab]
	
	s = jedi.Script(script, row, column, filename)
	completions = s.completions()
	
	comps = []
	for completion in completions:
		comps.append(completion.name)
		
	return comps

def python_engine_get_completion_interpreter(script):
	import jedi
	
	s = jedi.Interpreter(script, [globals()])
	completions = s.completions()
	
	comps = []
	for completion in completions:
		comps.append(completion.name)
		
	return comps
		
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

# chart
def chart(x, y, xlabel = "", ylabel = ""):
    import pylab as pl

    fig = pl.figure()
    pl.plot(x, y)
    pl.grid(True)
    pl.xlabel(xlabel)
    pl.ylabel(ylabel)
    fn_chart = pythonlab.tempname("png")
    pl.savefig(fn_chart, dpi=60)
    pl.clf()
    pl.close(fig)
    
    # show in console
    pythonlab.image(fn_chart)

setattr(pythonlab, "chart", chart)

# redirect stdout and stderr
class CatchOutErr:
    def write(self, str):
        pythonlab.__stdout__(str)

    def writeln(self, str):
        pythonlab.__stdout__(str + "\n")

    def flush(self):
        pass

sys.stdout = CatchOutErr()
sys.stderr = CatchOutErr()
