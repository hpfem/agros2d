import agros2d as a2d

class TestProblem(a2d.Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)

    """ coordinate_type """
    def test_coordinate_type(self):
        for type in ['planar', 'axisymmetric']:
            self.problem.coordinate_type = type
            self.assertEqual(self.problem.coordinate_type, type)

    def test_set_wrong_coordinate_type(self):
        with self.assertRaises(ValueError):
            self.problem.coordinate_type = 'wrong_coordinate_type'

    """ mesh_type """
    def test_mesh_type(self):
        for type in ['triangle', 'triangle_quad_fine_division',
                     'triangle_quad_rough_division', 'triangle_quad_join',
                     'gmsh_triangle', 'gmsh_quad', 'gmsh_quad_delaunay']:
            self.problem.mesh_type = type
            self.assertEqual(self.problem.mesh_type, type)

    def test_set_wrong_mesh_type(self):
        with self.assertRaises(ValueError):
            self.problem.mesh_type = 'wrong_mesh_type'

    """ frequency """
    def test_frequency(self):
        self.problem.frequency = 50
        self.assertEqual(self.problem.frequency, 50)

    def test_set_wrong_frequency(self):
        with self.assertRaises(IndexError):
            self.problem.frequency = -100

    """ time_step_method """
    def test_time_step_method(self):
        for method in ['fixed', 'adaptive', 'adaptive_numsteps']:
            self.problem.time_step_method = method
            self.assertEqual(self.problem.time_step_method, method)

    def test_set_wrong_time_step_method(self):
        with self.assertRaises(ValueError):
            self.problem.time_step_method = 'wrong_method'

    """ time_method_order """
    def test_time_method_order(self):
        self.problem.time_method_order = 2
        self.assertEqual(self.problem.time_method_order, 2)

    def test_set_wrong_time_order(self):
        with self.assertRaises(IndexError):
            self.problem.time_method_order = 0

        with self.assertRaises(IndexError):
            self.problem.time_method_order = 4

    """ time_method_tolerance """
    def test_time_method_tolerance(self):
        self.problem.time_method_tolerance = 1e-3
        self.assertEqual(self.problem.time_method_tolerance, 1e-3)

    def test_set_wrong_time_tolerance(self):
        with self.assertRaises(IndexError):
            self.problem.time_method_order = -1e-3

    """ time_total """
    def test_time_total(self):
        self.problem.time_total = 300
        self.assertEqual(self.problem.time_total, 300)

    def test_set_wrong_time_total(self):
        with self.assertRaises(IndexError):
            self.problem.time_total = -300

    """ time_steps """
    def test_time_steps(self):
        self.problem.time_steps = 1e2
        self.assertEqual(self.problem.time_steps, 1e2)

    def test_set_wrong_time_steps(self):
        with self.assertRaises(IndexError):
            self.problem.time_steps = 0
            
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = a2d.Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestProblem))
    suite.run(result)