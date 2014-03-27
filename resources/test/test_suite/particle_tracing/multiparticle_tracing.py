import agros2d as a2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from pairparticle_tracing import save_data

class TestMultiParticleTracingPlanar(Agros2DTestCase):
    @classmethod
    def setUpClass(cls):
        problem = a2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"

        electrostatic = a2d.field("electrostatic")
        electrostatic.analysis_type = "steadystate"
        electrostatic.matrix_solver = "mumps"
        electrostatic.number_of_refinements = 1
        electrostatic.polynomial_order = 2
        electrostatic.adaptivity_type = "disabled"
        electrostatic.solver = "linear"

        electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 0})
        electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})

        geometry = a2d.geometry

        d = 1
        h = 1

        geometry.add_edge(-d/2.0, 0, d/2.0, 0, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(d/2.0, 0, d/2.0, h, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(d/2.0, h, -d/2.0, h, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(-d/2.0, h, -d/2.0, 0, boundaries = {"electrostatic" : "Source"})

        geometry.add_label(0, h/2.0, materials = {"electrostatic" : "Air"})
        a2d.view.zoom_best_fit()

        problem.solve()

    def setUp(self):
        self.tracing = a2d.particle_tracing
        self.tracing.number_of_particles = 2

        x0 = 0.001
        y0 = 0.75
        v0 = 0.25

        dx = 0.003
        dy = 0.025
        
        self.initial_positions = [[x0, y0, 0], [x0-dx, y0-dy, 0]]
        self.initial_velocities = [[0, -v0, 0], [0, 0, 0]]
        self.particle_charges = [-5e-10, 1e-10]

        self.tracing.mass = 3.5e-5
        self.tracing.custom_force = [0, - self.tracing.mass * 9.823, 0]
        self.tracing.charge = 0
        self.tracing.electrostatic_interaction = False
        self.tracing.magnetic_interaction = False

        self.tracing.drag_force_density = 1.2041
        self.tracing.drag_force_reference_area = pi*2.84e-3/2.0**2
        self.tracing.drag_force_coefficient = 0 #0.47

        self.tracing.butcher_table_type = 'fehlberg' #fehlberg heun-euler
        self.tracing.maximum_relative_error = 1e-3
        self.tracing.minimum_step = 1
        self.tracing.maximum_number_of_steps = 1e5
        self.tracing.include_relativistic_correction = False

    def test_free_fall_with_electric_interaction(self):
        self.tracing.electrostatic_interaction = True
        self.tracing.solve(self.initial_positions, self.initial_velocities, self.particle_charges)
        x, y, z = self.tracing.positions()

        data = [{'x' : x[0], 'y' : y[0], 'z' : z[0]}, {'x' : x[1], 'y' : y[1], 'z' : z[1]}]
        save_data(data, 'free_fall-a2d')

if __name__ == '__main__':
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMultiParticleTracingPlanar))
    suite.run(result)