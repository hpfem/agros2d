import pythonlab
import agros2d
import os

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

def create_tests(case, dir):
    for (path, dirs, files) in os.walk(dir):
        for file in files:
            name, extension = os.path.splitext(file)

            method = 'test_{0}_{1}'.format(os.path.split(path)[-1].replace(" ", "_"),
                                           name.replace(" ", "_")).lower()
            file = '{0}/{1}'.format(path, file)

            if (extension == '.a2d'):
                setattr(case, method, get_test(file))

def get_test(file):
    def test(self):
        agros2d.open_file(file)
        agros2d.problem().solve()
    return test

class XSLT(Agros2DTestCase): pass
create_tests(XSLT, pythonlab.datadir('/resources/test/test_suite/internal/data_files'))

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(XSLT))
    suite.run(result)
