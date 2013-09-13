import agros2d
import os

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class Examples(Agros2DTestCase):
    def getExamples(self, dir, ext):
        self.examples = list()
        os.chdir(os.getcwd())
        for (path, dirs, files) in os.walk(dir):
            for file in files:
                name, extension = os.path.splitext(file)
                if(extension == ext):
                    self.examples.append('{0}/{1}'.format(path, file))

    def test(self):
        for example in self.examples:
            agros2d.open_file(example)
            agros2d.problem().solve()

class A2DExamples(Examples):
    def setUp(self):
        self.getExamples('../../../examples/Examples', '.a2d')

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(A2DExamples))
    suite.run(result)