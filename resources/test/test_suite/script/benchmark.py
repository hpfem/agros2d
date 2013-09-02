import agros2d as a2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from math import sin, cos

class BenchmarkGeometryTransformation(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.geometry = a2d.geometry

        for i in range(100):
            self.geometry.add_node(i*sin(i), i*cos(i))

        self.geometry.select_nodes()

    def test_move(self):
        for i in range(25):
            self.geometry.move_selection(1, 0)

    def test_rotate(self):
        for i in range(25):
            self.geometry.rotate_selection(0, 0, 1)

    def test_scale(self):
        for i in range(25):
            self.geometry.scale_selection(0, 0, 0.5)
            
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(BenchmarkGeometryTransformation))
    suite.run(result)
