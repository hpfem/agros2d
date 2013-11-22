import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestCoupledProblemsManyDomains(Agros2DTestCase):
    def setUp(self):         
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        # current
        self.current = agros2d.field("current")
        self.current.analysis_type = "steadystate"
        self.current.matrix_solver = "mumps"
        self.current.number_of_refinements = 1
        self.current.polynomial_order = 2
        self.current.adaptivity_type = "disabled"
        self.current.solver = "linear"        
        
        # boundaries
        self.current.add_boundary("Source", "current_potential", {"current_potential" : 10})
        self.current.add_boundary("Ground", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})        
        
        # materials
        self.current.add_material("Cond 1", {"current_conductivity" : 3.3e+07})
        self.current.add_material("Cond 2", {"current_conductivity" : 5.7e+07})
        
        # elasticity
        self.elasticity = agros2d.field("elasticity")
        self.elasticity.analysis_type = "steadystate"
        self.elasticity.matrix_solver = "mumps"
        self.elasticity.number_of_refinements = 0
        self.elasticity.polynomial_order = 3
        self.elasticity.adaptivity_type = "disabled"
        self.elasticity.solver = "linear"        
        
        # boundaries
        self.elasticity.add_boundary("fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
        self.elasticity.add_boundary("fixed-free", "elasticity_fixed_free", {"elasticity_displacement_x" : 0, "elasticity_force_y" : 0})
                
        # materials
        self.elasticity.add_material("structural", {"elasticity_young_modulus" : 1e+11, "elasticity_poisson_ratio" : 0.3, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_alpha" : 1e-05, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 0})
        
        # electrostatic
        self.electrostatic = agros2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.matrix_solver = "mumps"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 2
        self.electrostatic.adaptivity_type = "disabled"
        self.electrostatic.solver = "linear"
                
        # boundaries
        self.electrostatic.add_boundary("electrode1", "electrostatic_potential", {"electrostatic_potential" : 1000})
        self.electrostatic.add_boundary("electrode2", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("neuman", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
                
        # materials
        self.electrostatic.add_material("electrostatic", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        
        # heat
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.matrix_solver = "mumps"
        self.heat.number_of_refinements = 1
        self.heat.polynomial_order = 2
        self.heat.adaptivity_type = "disabled"
        self.heat.solver = "linear"
                
        # boundaries
        self.heat.add_boundary("zero flux", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 5, "heat_convection_external_temperature" : 20, "heat_radiation_emissivity" : 0, "heat_radiation_ambient_temperature" : 20})
        self.heat.add_boundary("t=0", "heat_temperature", {"heat_temperature" : 0})        
        
        # materials
        self.heat.add_material("heat", {"heat_conductivity" : 385, "heat_volume_heat" : 0, "heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_density" : 0, "heat_specific_heat" : 0})
        self.heat.add_material("heat2", {"heat_conductivity" : 38500, "heat_volume_heat" : 0, "heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_density" : 0, "heat_specific_heat" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 0.1, 0, 0, boundaries = {"current" : "Source"})
        geometry.add_edge(0.15, 0, 0.15, 0.1, boundaries = {"current" : "Ground"})
        geometry.add_edge(0, 0, 0.05, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.05, 0, 0.15, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.05, 0.1, 0.05, 0)
        geometry.add_edge(0.05, 0.1, 0, 0.1, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.05, 0.1, 0.15, 0.1, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.085, 0.065, 0.085, 0.06, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.085, 0.06, 0.09, 0.06, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.09, 0.06, 0.09, 0.065, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.09, 0.065, 0.085, 0.065, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.065, 0.03, 0.065, 0.01, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.065, 0.01, 0.09, 0.01, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.07, 0.025, 0.075, 0.025, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.075, 0.025, 0.075, 0.015, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.075, 0.015, 0.07, 0.015, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.07, 0.015, 0.07, 0.025, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.08, 0.025, 0.08, 0.015, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.08, 0.015, 0.085, 0.015, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.06, 0.07, 0.06, 0.06, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.065, 0.06, 0.06, 0.06, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.065, 0.06, 0.065, 0.07, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.065, 0.07, 0.06, 0.07, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.115, 0.02, 0.12, 0.02, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.12, 0.03, 0.115, 0.03, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.12, 0.04, 0.145, 0.04, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.145, 0.04, 0.145, 0.02, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.145, 0.02, 0.12, 0.02, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.125, 0.035, 0.125, 0.025, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.13, 0.025, 0.13, 0.035, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.135, 0.035, 0.135, 0.025, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.135, 0.025, 0.14, 0.025, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.14, 0.025, 0.14, 0.035, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.14, 0.035, 0.135, 0.035, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.1, 0.09, 0.1, 0.055, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.1, 0.055, 0.14, 0.055, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.14, 0.055, 0.14, 0.09, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.14, 0.09, 0.1, 0.09, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.105, 0.085, 0.105, 0.06, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.105, 0.06, 0.135, 0.06, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.135, 0.06, 0.135, 0.085, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.135, 0.085, 0.105, 0.085, boundaries = {"heat" : "t=0"})
        geometry.add_edge(0.125, 0.07, 0.125, 0.065, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.13, 0.065, 0.125, 0.065, boundaries = {"elasticity" : "fixed", "heat" : "t=0"})
        geometry.add_edge(0.13, 0.065, 0.13, 0.07, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.13, 0.07, 0.125, 0.07, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.115, 0.08, 0.115, 0.075, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.11, 0.075, 0.115, 0.075, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.11, 0.075, 0.11, 0.08, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.11, 0.08, 0.115, 0.08, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.115, 0.07, 0.115, 0.065, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.115, 0.065, 0.12, 0.065, boundaries = {"elasticity" : "fixed", "heat" : "t=0"})
        geometry.add_edge(0.12, 0.065, 0.12, 0.07, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.12, 0.07, 0.115, 0.07, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.125, 0.08, 0.125, 0.075, boundaries = {"current" : "Neumann", "elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.125, 0.075, 0.13, 0.075, boundaries = {"current" : "Neumann", "elasticity" : "fixed", "heat" : "zero flux"})
        geometry.add_edge(0.13, 0.08, 0.13, 0.075, boundaries = {"current" : "Neumann", "elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.13, 0.08, 0.125, 0.08, boundaries = {"current" : "Neumann", "elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.015, 0.085, 0.015, 0.07, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.015, 0.085, 0.04, 0.085, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.04, 0.085, 0.04, 0.07, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.04, 0.07, 0.015, 0.07, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.02, 0.075, 0.025, 0.075, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.025, 0.08, 0.02, 0.08, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.025, 0.075, 0.025, 0.08, boundaries = {"current" : "Ground"})
        geometry.add_edge(0.02, 0.08, 0.02, 0.075, boundaries = {"current" : "Source"})
        geometry.add_edge(0.03, 0.075, 0.03, 0.08, boundaries = {"current" : "Source", "heat" : "zero flux"})
        geometry.add_edge(0.035, 0.08, 0.035, 0.075, boundaries = {"current" : "Ground", "heat" : "zero flux"})
        geometry.add_edge(0.035, 0.075, 0.03, 0.075, boundaries = {"current" : "Neumann", "heat" : "t=0"})
        geometry.add_edge(0.03, 0.08, 0.035, 0.08, boundaries = {"current" : "Neumann", "heat" : "zero flux"})
        geometry.add_edge(0.065, 0.03, 0.0825, 0.03, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.0825, 0.03, 0.09, 0.03, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.08, 0.025, 0.0825, 0.025, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.0825, 0.025, 0.085, 0.025, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.085, 0.025, 0.085, 0.0225, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.085, 0.0225, 0.085, 0.015, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.09, 0.0225, 0.09, 0.03, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.09, 0.0225, 0.09, 0.01, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.115, 0.03, 0.115, 0.0225, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.115, 0.0225, 0.115, 0.02, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.12, 0.03, 0.12, 0.0225, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.12, 0.0225, 0.12, 0.02, boundaries = {"elasticity" : "fixed-free", "heat" : "zero flux"})
        geometry.add_edge(0.125, 0.025, 0.1275, 0.025, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.1275, 0.025, 0.13, 0.025, boundaries = {"elasticity" : "fixed"})
        geometry.add_edge(0.125, 0.035, 0.1275, 0.035, boundaries = {"elasticity" : "fixed-free", "electrostatic" : "electrode2"})
        geometry.add_edge(0.1275, 0.035, 0.13, 0.035, boundaries = {"elasticity" : "fixed-free"})
        geometry.add_edge(0.12, 0.04, 0.12, 0.035, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.12, 0.035, 0.12, 0.03, boundaries = {"heat" : "zero flux"})
        geometry.add_edge(0.0825, 0.045, 0.0825, 0.03, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.0825, 0.03, 0.0825, 0.025, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.0825, 0.025, 0.0825, 0.0225, boundaries = {"electrostatic" : "electrode1"})
        geometry.add_edge(0.0825, 0.0225, 0.085, 0.0225, boundaries = {"electrostatic" : "electrode1"})
        geometry.add_edge(0.085, 0.0225, 0.09, 0.0225, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.09, 0.0225, 0.115, 0.0225, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.115, 0.0225, 0.12, 0.0225, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.12, 0.0225, 0.1275, 0.0225, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.1275, 0.0225, 0.1275, 0.025, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.1275, 0.025, 0.1275, 0.035, boundaries = {"electrostatic" : "neuman"})
        geometry.add_edge(0.125, 0.035, 0.12, 0.035, boundaries = {"electrostatic" : "electrode2"})
        geometry.add_edge(0.12, 0.035, 0.0825, 0.045, boundaries = {"electrostatic" : "neuman"})
        
        geometry.add_label(0.0266815, 0.0278347, materials = {"current" : "Cond 1", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.104249, 0.0211345, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.0887109, 0.0616798, materials = {"current" : "none", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.0758851, 0.0176937, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.0727184, 0.0222517, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.0832365, 0.0204127, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.0621138, 0.0653831, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.117585, 0.0258373, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "electrostatic", "heat" : "none"})
        geometry.add_label(0.12689, 0.0303108, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "electrostatic", "heat" : "heat"})
        geometry.add_label(0.132079, 0.0217217, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.136911, 0.0290582, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.10226, 0.0648003, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.111193, 0.0684784, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.127832, 0.0677778, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.111368, 0.0772358, materials = {"current" : "none", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.117608, 0.0666663, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.126893, 0.0779052, materials = {"current" : "none", "elasticity" : "structural", "electrostatic" : "none", "heat" : "heat"})
        geometry.add_label(0.0195236, 0.0823198, materials = {"current" : "none", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.0225718, 0.0771613, materials = {"current" : "Cond 1", "elasticity" : "none", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.0333576, 0.0783337, materials = {"current" : "Cond 1", "elasticity" : "none", "electrostatic" : "none", "heat" : "heat2"})
        geometry.add_label(0.101318, 0.0322917, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "electrostatic", "heat" : "none"})
        geometry.add_label(0.0872661, 0.0274572, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "electrostatic", "heat" : "heat"})
        geometry.add_label(0.084043, 0.0239763, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "electrostatic", "heat" : "heat"})
        geometry.add_label(0.118077, 0.0212412, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "none"})
        geometry.add_label(0.122094, 0.0238927, materials = {"current" : "Cond 2", "elasticity" : "none", "electrostatic" : "electrostatic", "heat" : "heat"})
        geometry.add_label(0.128557, 0.0273469, materials = {"current" : "Cond 2", "elasticity" : "structural", "electrostatic" : "none", "heat" : "heat"})
        
        agros2d.view.zoom_best_fit()

        # solve problem
        problem.solve()

    def test_values(self):         
        # point value
        point1_elast = self.elasticity.local_values(7.161e-02, 1.999e-02)
        self.value_test("Elasticity displ", point1_elast["d"], 4.629e-3)
        point2_elast = self.elasticity.local_values(6.181e-02, 6.472e-02)
        self.value_test("Elasticity displ", point2_elast["d"], 0)
        point3_elast = self.elasticity.local_values(1.173e-01, 6.803e-02)
        self.value_test("Elasticity displ", point3_elast["d"], 0)
        point4_elast = self.elasticity.local_values(1.374e-01, 3.193e-02)
        self.value_test("Elasticity displ", point4_elast["d"], 6.584e-03)
        
        point1_heat = self.heat.local_values(1.329e-01, 3.714e-02)
        self.value_test("Temperature", point1_heat["T"], 7.624e+04)
        point2_heat = self.heat.local_values(3.142e-02, 7.729e-02)
        self.value_test("Temperature", point2_heat["T"], 3.025e+04)
        
        point1_electrostatic = self.electrostatic.local_values(1.169e-01, 2.757e-02)
        self.value_test("Electric field", point1_electrostatic["E"], 1.785e+04)        
        
        
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestCoupledProblemsManyDomains))
    suite.run(result)