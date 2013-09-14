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

            if(extension == '.a2d'):
                setattr(case, method, get_a2d_test(example))
            elif(extension == '.py'):
                setattr(case, method, get_py_test(example))

def get_a2d_test(example):
    def test(self):
        agros2d.open_file(example)
        agros2d.problem().solve()
    return test
        
def get_py_test(example):
    def test(self):
        execfile(example)
    return test

class Examples(Agros2DTestCase): pass
create_tests(Examples, pythonlab.datadir('/resources/examples/Examples'))

class Other(Agros2DTestCase): pass
create_tests(Other, pythonlab.datadir('/resources/examples/Other'))

class PythonLab(Agros2DTestCase): pass
create_tests(PythonLab, pythonlab.datadir('/resources/examples/PythonLab'))

class Tutorials(Agros2DTestCase): pass
create_tests(Tutorials, pythonlab.datadir('/resources/examples/Tutorials'))

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Examples))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Other))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(PythonLab))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Tutorials))
    suite.run(result)