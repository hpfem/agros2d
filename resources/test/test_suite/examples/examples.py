import agros2d
import pythonlab
import os

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class ExamplesTest(Agros2DTestCase):
    def getExamples(self, dir):
        self.a2d_examples = list()
        self.py_examples = list()

        for (path, dirs, files) in os.walk(dir):
            for file in files:
                name, extension = os.path.splitext(file)
                if(extension == '.a2d'):
                    self.a2d_examples.append('{0}/{1}'.format(path, file))
                elif(extension == '.py'):
                    self.py_examples.append('{0}/{1}'.format(path, file))

    def test(self):
        for example in self.a2d_examples:
            agros2d.open_file(example)
            agros2d.problem().solve()

        for example in self.py_examples:
            execfile(example)

class Examples(ExamplesTest):
    def setUp(self):
        self.getExamples(pythonlab.datadir('/resources/examples/Examples'))

class Other(ExamplesTest):
    def setUp(self):
        self.getExamples(pythonlab.datadir('/resources/examples/Other'))

class PythonLab(ExamplesTest):
    def setUp(self):
        self.getExamples(pythonlab.datadir('/resources/examples/PythonLab'))

class Tutorials(ExamplesTest):
    def setUp(self):
        self.getExamples(pythonlab.datadir('/resources/examples/Tutorials'))

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Examples))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Other))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(PythonLab))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(Tutorials))
    suite.run(result)