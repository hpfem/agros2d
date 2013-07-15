import agros2d
from unittest import TestCase

from math import sin, cos
from time import time

class BenchmarkGeometryTransformation(TestCase):
    @classmethod
    def setUpClass(self):
        self.results = {}

    @classmethod
    def tearDownClass(self):
        for test in self.results:
            print('Test: {0}; time: {1}'.format(test, self.results[test]))

    def setUp(self):
        self.problem = agros2d.problem(clear = True)
        self.geometry = agros2d.geometry

        for i in range(100):
            self.geometry.add_node(i*sin(i), i*cos(i))

        self.geometry.select_nodes()
        self.start_time = time()

    def tearDown(self):
        self.elapsed_time = time() - self.start_time
        self.results.update({self.id().split('.')[-1] : self.elapsed_time})

    def test_move_selection(self):
        for i in range(25):
            self.geometry.move_selection(1, 0)

    def test_rotate_selection(self):
        for i in range(25):
            self.geometry.rotate_selection(0, 0, 1)

    def test_scale_selection(self):
        for i in range(25):
            self.geometry.scale_selection(0, 0, 0.5)