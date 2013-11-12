import agros2d as a2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestProblem(Agros2DTestCase):
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

class TestProblemTime(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.problem.time_step_method = "fixed"
        self.problem.time_total = 1e2
        self.problem.time_steps = 10

        heat = a2d.field('heat')
        heat.analysis_type = 'transient'
        heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 10,
                                                           "heat_convection_external_temperature" : 293})
        heat.add_material("Copper", {"heat_conductivity" : 200, "heat_volume_heat" : 1e3,
                                     "heat_density" : 8700, "heat_specific_heat" : 385})

        a2d.geometry.add_rect(0, 0, 1, 1, boundaries = {'heat' : 'Convection'}, materials = {'heat' : 'Copper'})

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

    """ time_steps_length """
    def test_time_steps_length(self):
        self.problem.solve()
        self.assertEqual(self.problem.time_steps_length(), [10]*10)

    """ time_steps_total """
    def test_time_steps_total(self):
        self.problem.solve()
        self.assertEqual(self.problem.time_steps_total(), range(0, 101, 10))

    """ elapsed_time """
    def test_elapsed_time(self):
        self.problem.solve()
        self.assertNotEqual(self.problem.elapsed_time(), 0.0)

    def test_elapsed_time_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.problem.elapsed_time()

class TestProblemSolution(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        current = a2d.field('current')
        current.add_boundary("Source", "current_potential", {"current_potential" : 10})
        current.add_boundary("Ground", "current_potential", {"current_potential" : 0})
        current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        current.add_material("Copper", {"current_conductivity" : 33e6})

        heat = a2d.field('heat')
        heat.analysis_type = 'transient'
        heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 10,
                                                           "heat_convection_external_temperature" : 293})
        heat.add_material("Copper", {"heat_conductivity" : 200, "heat_density" : 8700, "heat_specific_heat" : 385})

        a2d.geometry.add_edge(0, 0, 1, 0, boundaries = {'current' : 'Neumann', 'heat' : 'Convection'})
        a2d.geometry.add_edge(1, 0, 1, 1, boundaries = {'current' : 'Ground', 'heat' : 'Convection'})
        a2d.geometry.add_edge(1, 1, 0, 1, boundaries = {'current' : 'Neumann', 'heat' : 'Convection'})
        a2d.geometry.add_edge(0, 1, 0, 0, boundaries = {'current' : 'Source', 'heat' : 'Convection'})
        a2d.geometry.add_label(0.5, 0.5, materials = {'current' : 'Copper', 'heat' : 'Copper'})

    """ coupling_type """
    def test_coupling_type(self):
        for type in ['none', 'weak']:
            self.problem.set_coupling_type('current', 'heat', type)
            self.assertEqual(self.problem.get_coupling_type('current', 'heat'), type)

    """ clear_solution """
    def test_clear_solution(self):
        self.problem.solve()
        self.problem.clear_solution()
        with self.assertRaises(RuntimeError):
            self.problem.elapsed_time()

    """ clear """
    def test_clear(self):
        self.problem.clear()
        self.assertEqual(a2d.geometry.nodes_count(), 0)

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestProblem))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestProblemTime))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestProblemSolution))
    suite.run(result)