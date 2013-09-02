import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class ParticleTracingPlanar(Agros2DTestCase):
    def setUp(self): 
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
                
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        # electrostatic
        electrostatic = agros2d.field("electrostatic")
        electrostatic.analysis_type = "steadystate"
        electrostatic.number_of_refinements = 1
        electrostatic.polynomial_order = 2
        electrostatic.adaptivity_type = "disabled"
        electrostatic.solver = "linear"
                
        # boundaries
        electrostatic.add_boundary("Voltage", "electrostatic_potential", {"electrostatic_potential" : 1e4})
        electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
                
        # materials
        electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        electrostatic.add_material("Screen", {"electrostatic_permittivity" : 2, "electrostatic_charge_density" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.075, -0.022, 0.075, 0.022, angle = 90)
        geometry.add_edge(0.075, -0.022, 0.079, -0.026)
        geometry.add_edge(0.079, -0.026, 0.079, 0.026, angle = 90)
        geometry.add_edge(0.079, 0.026, 0.075, 0.022)
        geometry.add_edge(0, 0.06, 0, -0.06, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, -0.06, 0.118, -0.06, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.118, -0.06, 0.118, 0.06, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.118, 0.06, 0, 0.06, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.062, 0.01, 0.04, 0.01, boundaries = {"electrostatic" : "Voltage"})
        geometry.add_edge(0.04, 0.017, 0.04, 0.01, boundaries = {"electrostatic" : "Voltage"})
        geometry.add_edge(0.04, 0.017, 0.062, 0.017, boundaries = {"electrostatic" : "Voltage"})
        geometry.add_edge(0.062, 0.017, 0.062, 0.01, boundaries = {"electrostatic" : "Voltage"})
        geometry.add_edge(0.05, -0.01, 0.05, -0.017, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.05, -0.017, 0.04, -0.017, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.04, -0.01, 0.04, -0.017, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.04, -0.01, 0.05, -0.01, boundaries = {"electrostatic" : "Ground"})
        
        geometry.add_label(0.0805206, 0.0216256, materials = {"electrostatic" : "Screen"})
        geometry.add_label(0.0504445, 0.0129438, materials = {"electrostatic" : "none"})
        geometry.add_label(0.0452163, -0.0148758, materials = {"electrostatic" : "none"})
        geometry.add_label(0.109507, 0.050865, materials = {"electrostatic" : "Air"})
        agros2d.view.zoom_best_fit()
        
        problem.solve()
        
    def test_values(self):        
        tracing = agros2d.particle_tracing
        tracing.drag_force_density = 1.2041
        tracing.drag_force_coefficient = 0
        tracing.drag_force_reference_area = 1e-06
        tracing.mass = 9.109e-31
        tracing.charge = -1.602e-19
        
        tracing.reflect_on_different_material = True
        tracing.reflect_on_boundary = False
        tracing.coefficient_of_restitution = 0
        
        tracing.maximum_number_of_steps = 1e3
        tracing.maximum_relative_error = 0.0001
        tracing.minimum_step = 0.0003
        
        tracing.initial_position = (0.01, 0.0)
        tracing.initial_velocity = (8e7, 0)
        
        tracing.solve()
        x, y, z = tracing.positions()
        
        self.value_test("Particle position", x[-1], 0.080043)
        self.value_test("Particle position", y[-1], 0.015374)

class ParticleTracingAxisymmetric(Agros2DTestCase):
    def setUp(self): 
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        # electrostatic
        electrostatic = agros2d.field("electrostatic")
        electrostatic.analysis_type = "steadystate"
        electrostatic.number_of_refinements = 1
        electrostatic.polynomial_order = 2
        electrostatic.adaptivity_type = "disabled"
        electrostatic.solver = "linear"
        
        # boundaries
        electrostatic.add_boundary("0 V", "electrostatic_potential", {"electrostatic_potential" : 0})
        electrostatic.add_boundary("potential", "electrostatic_potential", {"electrostatic_potential" : -2000})
        electrostatic.add_boundary("neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        # materials
        electrostatic.add_material("air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        
        # magnetic
        magnetic = agros2d.field("magnetic")
        magnetic.analysis_type = "steadystate"
        magnetic.number_of_refinements = 2
        magnetic.polynomial_order = 2
        magnetic.adaptivity_type = "disabled"
        magnetic.solver = "linear"
        
        # boundaries
        magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        # materials
        magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        magnetic.add_material("Coil 1", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "8e6" }})
        magnetic.add_material("Coil 2", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e7" }})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 0.04, 0.05, 0.04, boundaries = {"electrostatic" : "0 V", "magnetic" : "A = 0"})
        geometry.add_edge(0.05, 0.04, 0.05, -0.08, boundaries = {"electrostatic" : "neumann", "magnetic" : "A = 0"})
        geometry.add_edge(0.05, -0.08, 0, -0.08, boundaries = {"electrostatic" : "potential", "magnetic" : "A = 0"})
        geometry.add_edge(0, -0.08, 0, 0.04, boundaries = {"electrostatic" : "neumann", "magnetic" : "A = 0"})
        geometry.add_edge(0.0075, 0.00125, 0.01, 0.00125)
        geometry.add_edge(0.01, 0.00125, 0.01, -0.00125)
        geometry.add_edge(0.01, -0.00125, 0.0075, -0.00125)
        geometry.add_edge(0.0075, 0.00125, 0.0075, -0.00125)
        geometry.add_edge(0.0075, 0.00625, 0.01, 0.00625)
        geometry.add_edge(0.01, 0.00375, 0.01, 0.00625)
        geometry.add_edge(0.0075, 0.00625, 0.0075, 0.00375)
        geometry.add_edge(0.01, 0.00375, 0.0075, 0.00375)
        geometry.add_edge(0.0075, -0.01475, 0.01, -0.01475)
        geometry.add_edge(0.01, -0.01475, 0.01, -0.02125)
        geometry.add_edge(0.01, -0.02125, 0.0075, -0.02125)
        geometry.add_edge(0.0075, -0.01475, 0.0075, -0.02125)
        
        geometry.add_label(0.00870469, 0.000204637, materials = {"electrostatic" : "air", "magnetic" : "Coil 1"})
        geometry.add_label(0.027331, 0.0261643, materials = {"electrostatic" : "air", "magnetic" : "Air"})
        geometry.add_label(0.00883433, 0.00543334, materials = {"electrostatic" : "air", "magnetic" : "Coil 1"})
        geometry.add_label(0.00902444, -0.0168831, materials = {"electrostatic" : "air", "magnetic" : "Coil 2"})
        agros2d.view.zoom_best_fit()
        
        problem.solve()
        
    def test_values(self):               
        tracing = agros2d.particle_tracing
        tracing.drag_force_density = 1.2041
        tracing.drag_force_coefficient = 0
        tracing.drag_force_reference_area = 1e-06
        tracing.mass = 9.109e-31
        tracing.charge = 1.602e-19
        
        tracing.reflect_on_different_material = True
        tracing.reflect_on_boundary = False
        tracing.coefficient_of_restitution = 0
        
        tracing.maximum_number_of_steps = 1e3
        tracing.maximum_relative_error = 0.0001
        tracing.minimum_step = 0.0003
        
        tracing.initial_position = (0.003, 0.03)
        tracing.initial_velocity = (0, 0)
        
        tracing.solve()
        x, y, z = tracing.positions()
        
        self.value_test("Particle position", x[-1], 0.004637)        

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(ParticleTracingPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(ParticleTracingAxisymmetric))
    suite.run(result)            