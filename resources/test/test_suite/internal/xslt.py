import pythonlab
import agros2d
import os

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

def create_tests(case, dir):
    for file in os.listdir(dir):
        name, extension = os.path.splitext(file)

        method = 'test_{0}_{1}'.format(os.path.split(path)[-1].replace(" ", "_"),
                                       name.replace(" ", "_")).lower().replace("~1", "")
        file = '{0}/{1}'.format(path, file)

        if (extension == '.a2d'):
            setattr(case, method, get_test(file))

def get_test(file):
    def test(self):
        agros2d.open_file(file)
        agros2d.problem().solve()
    return test

tests = list()
data_dirs = [pythonlab.datadir('/resources/test/test_suite/internal/data_files/0/'),
             pythonlab.datadir('/resources/test/test_suite/internal/data_files/21/'),
             pythonlab.datadir('/resources/test/test_suite/internal/data_files/30/')]

for dir in data_dirs:
    for (path, dirs, files) in os.walk(dir):
        if (dirs):
            continue

        rest, field = os.path.split(path)
        rest, version = os.path.split(rest)
        name = "TestXSLT{0}{1}".format(version, field.title()).replace("~1", "")

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