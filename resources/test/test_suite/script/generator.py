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
                setattr(case, method, get_test(example))

def get_test(example):
    def test(self):
        agros2d.open_file(example)
        script = agros2d.get_script_from_model()
        exec script in globals(), locals()
        agros2d.problem().solve()

    return test

class Generator(Agros2DTestCase): pass
create_tests(Generator, pythonlab.datadir('/resources/examples/Examples'))

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Generator))
    suite.run(result)