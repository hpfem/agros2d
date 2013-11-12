import agros2d
import pythonlab
import os

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

def create_tests(case, dir):
    for (path, dirs, files) in os.walk(dir):
        for file in files:
            name, extension = os.path.splitext(file)

            method = 'test_{0}_{1}'.format(os.path.split(path)[-1].replace(" ", "_"), name.replace(" ", "_")).lower()
            example = '{0}/{1}'.format(path, file)

            if (extension == '.a2d'):
                setattr(case, method, get_a2d_test(example))
            elif (extension == '.py'):
                setattr(case, method, get_py_test(example))

def get_a2d_test(example):
    def test(self):
        agros2d.open_file(example)
        agros2d.problem().solve()
    return test
        
def get_py_test(example):
    def test(self):        
        with open(example) as f:
            exec f.read() in globals()
    return test

tests = list()
data_dirs = [pythonlab.datadir('/resources/examples/Examples'),
             pythonlab.datadir('/resources/examples/Other'),
             pythonlab.datadir('/resources/examples/PythonLab'),
             pythonlab.datadir('/resources/examples/Tutorials')]

for dir in data_dirs:
    for (path, dirs, files) in os.walk(dir):

        if not any("a2d" in file for file in files):
            continue

        name = "Examples{0}{1}".format(path.split('/')[-2].replace(" ", ""),
                path.split('/')[-1].replace(" ", ""))
        code = compile('class {0}(Agros2DTestCase): pass'.format(name), '<string>', 'exec')
        exec code
        create_tests(globals()[name], path)
        tests.append(globals()[name])

if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    for test in tests:
        suite.addTest(ut.TestLoader().loadTestsFromTestCase(test))
    suite.run(result)