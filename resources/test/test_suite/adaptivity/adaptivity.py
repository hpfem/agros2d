import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestAdaptivityElectrostatic(Agros2DTestCase):
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
        self.electrostatic = agros2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.polynomial_order = 2
        
        self.electrostatic.adaptivity_type = "hp-adaptivity"
        self.electrostatic.adaptivity_parameters['steps'] = 10
        self.electrostatic.adaptivity_parameters['tolerance'] = 1
        self.electrostatic.adaptivity_parameters['error_calculator'] = "h1"
        self.electrostatic.solver = "linear"
        
        # boundaries
        self.electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
        self.electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        # materials
        self.electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.2, 1, 0, 0.5, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0, 0.5, 0, 0.25, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -0.25, 0, -1, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -1, 1.5, 0.5, angle = 90, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(1.5, 0.5, 0, 2, angle = 90, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, 1, 0.2, 1, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0, 2, 0, 1, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Ground"})
        
        geometry.add_label(0.879551, 0.764057, area = 0.06, materials = {"electrostatic" : "Air"})
        agros2d.view.zoom_best_fit()
        
        problem.solve()

    def test_values(self):                
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point = self.electrostatic.local_values(3.278e-2, 4.624e-1)
        self.value_test("Electrostatic potential", point["V"], 5.569e2)

class TestAdaptivityAcoustic(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 5000
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        # acoustic
        self.acoustic = agros2d.field("acoustic")
        self.acoustic.analysis_type = "harmonic"
        self.acoustic.polynomial_order = 1
        self.acoustic.adaptivity_type = "hp-adaptivity"
        self.acoustic.adaptivity_parameters['steps'] = 15
        self.acoustic.adaptivity_parameters['tolerance'] = 2
        self.acoustic.adaptivity_parameters['error_calculator'] = "h1"
        self.acoustic.solver = "linear"
        
        # boundaries
        self.acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0, "acoustic_normal_acceleration_imag" : 0})
        self.acoustic.add_boundary("Source", "acoustic_pressure", {"acoustic_pressure_real" : 1, "acoustic_pressure_imag" : 0})
        self.acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : { "expression" : "1.25*343" }})
        
        # materials
        self.acoustic.add_material("Air", {"acoustic_density" : 1.25, "acoustic_speed" : 343})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.3, 0, 0, 0.3, angle = 90, boundaries = {"acoustic" : "Matched boundary"})
        geometry.add_edge(0.1, 0, 0.3, 0, boundaries = {"acoustic" : "Wall"})
        geometry.add_edge(0.1, 0, 0.025, -0.175, boundaries = {"acoustic" : "Wall"})
        geometry.add_edge(0.025, -0.175, 0.025, -0.2, boundaries = {"acoustic" : "Wall"})
        geometry.add_edge(0.025, -0.2, 0, -0.2, boundaries = {"acoustic" : "Source"})
        geometry.add_edge(0, 0.3, 0, -0.2, boundaries = {"acoustic" : "Wall"})
        
        geometry.add_label(0.109723, 0.176647, materials = {"acoustic" : "Air"})
        agros2d.view.zoom_best_fit()
        
        problem.solve()
        
    def test_values(self):        
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point1 = self.acoustic.local_values(7.544e-3, -0.145)
        self.value_test("Acoustic pressure", point1["p"], 7.481e-1)
        point2 = self.acoustic.local_values(6.994e-2, 1.894e-2)
        self.value_test("Acoustic pressure", point2["p"], 2.889e-1)
        
class TestAdaptivityElasticityBracket(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        # elasticity
        self.elasticity = agros2d.field("elasticity")
        self.elasticity.analysis_type = "steadystate"
        self.elasticity.matrix_solver = "mumps"
        self.elasticity.number_of_refinements = 0
        self.elasticity.polynomial_order = 1
        self.elasticity.adaptivity_type = "hp-adaptivity"
        self.elasticity.adaptivity_parameters['steps'] = 3
        self.elasticity.adaptivity_parameters['tolerance'] = 0
        self.elasticity.adaptivity_parameters['threshold'] = 0.6
        self.elasticity.adaptivity_parameters['stopping_criterion'] = "singleelement"
        self.elasticity.adaptivity_parameters['error_calculator'] = "h1"
        self.elasticity.adaptivity_parameters['anisotropic_refinement'] = True
        self.elasticity.adaptivity_parameters['finer_reference_solution'] = False
        self.elasticity.adaptivity_parameters['space_refinement'] = True
        self.elasticity.adaptivity_parameters['order_increase'] = 1
        self.elasticity.solver = "linear"
                
        # boundaries
        self.elasticity.add_boundary("Wall", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
        self.elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})
        self.elasticity.add_boundary("Load", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : -200})
                
        # materials
        self.elasticity.add_material("Steel", {"elasticity_young_modulus" : 2e11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : { "expression" : "-9.81*7800" }, "elasticity_alpha" : 0, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.3, 0, 0, 0, boundaries = {"elasticity" : "Load"})
        geometry.add_edge(0, 0, 0, -0.3, boundaries = {"elasticity" : "Wall"})
        geometry.add_edge(0, -0.3, 0.03, -0.27, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.27, -0.03, 0.3, 0, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.03, -0.03, 0.03, -0.15, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.03, -0.03, 0.15, -0.03, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.27, -0.03, 0.03, -0.27, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.03, -0.15, 0.030625, -0.150375, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.030625, -0.150375, 0.031, -0.15, angle = 30, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.150375, -0.030625, 0.15, -0.03, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.15, -0.031, 0.150375, -0.030625, angle = 30, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.15, -0.031, 0.031, -0.15, angle = 45, boundaries = {"elasticity" : "Free"})
        
        geometry.add_label(0.19805, -0.0157016, materials = {"elasticity" : "Steel"})
        geometry.add_label(0.0484721, -0.0490752, materials = {"elasticity" : "none"})

        agros2d.view.zoom_best_fit()       
                                                                                                                
        problem.solve()
        
    def test_values(self):        
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point1 = self.elasticity.local_values(2.042e-1, -3e-2)
        self.value_test("Displacement", point1["d"], 1.161e-7)
        
class TestAdaptivityMagneticProfileConductor(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        problem.frequency = 50000

        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        # magnetic
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        self.magnetic.adaptivity_type = "hp-adaptivity"
        self.magnetic.adaptivity_parameters['steps'] = 5
        self.magnetic.adaptivity_parameters['tolerance'] = 0
        self.magnetic.adaptivity_parameters['threshold'] = 0.6
        self.magnetic.adaptivity_parameters['stopping_criterion'] = "singleelement"
        self.magnetic.adaptivity_parameters['error_calculator'] = "h1"
        self.magnetic.adaptivity_parameters['anisotropic_refinement'] = True
        self.magnetic.adaptivity_parameters['finer_reference_solution'] = False
        self.magnetic.adaptivity_parameters['space_refinement'] = True
        self.magnetic.adaptivity_parameters['order_increase'] = 1
        self.magnetic.solver = "linear"
                
        # boundaries
        self.magnetic.add_boundary("Neumann", "magnetic_surface_current", {"magnetic_surface_current_real" : 0, "magnetic_surface_current_imag" : 0})
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})        
        
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 57e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 0.25, "magnetic_total_current_imag" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 0.002, 0, 0.000768, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0, 0.000768, 0, 0, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0, 0, 0.000768, 0, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0.000768, 0, 0.002, 0, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0.002, 0, 0, 0.002, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.000768, 0, 0.000576, 0.000192, angle = 90)
        geometry.add_edge(0.000576, 0.000192, 0.000384, 0.000192)
        geometry.add_edge(0.000192, 0.000384, 0.000384, 0.000192, angle = 90)
        geometry.add_edge(0.000192, 0.000576, 0.000192, 0.000384)
        geometry.add_edge(0.000192, 0.000576, 0, 0.000768, angle = 90)
        
        geometry.add_label(0.000585418, 0.00126858, materials = {"magnetic" : "Air"})
        geometry.add_label(0.000109549, 8.6116e-05, materials = {"magnetic" : "Copper"})
        agros2d.view.zoom_best_fit()                                                                                                                
        problem.solve()
        
    def test_values(self):        
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point1 = self.magnetic.local_values(6.106e-04, 2.378e-04)
        self.value_test("Vector potencial", point1["A"], 2.196e-07)
        self.value_test("Flux density", point1["B"], 2.775e-04)
        
class TestAdaptivityRF_TE(Agros2DTestCase):
    # test for hp-adaptivity, used different settings from implicit (l2 error, cumulative, finer_reference=false, order_increase=2)
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        problem.frequency = 1.6e+10
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        # rf_te
        self.rf_te = agros2d.field("rf_te")
        self.rf_te.analysis_type = "harmonic"
        self.rf_te.matrix_solver = "mumps"
        self.rf_te.number_of_refinements = 0
        self.rf_te.polynomial_order = 1
        self.rf_te.adaptivity_type = "hp-adaptivity"
        self.rf_te.adaptivity_parameters['steps'] = 6
        self.rf_te.adaptivity_parameters['tolerance'] = 0
        self.rf_te.adaptivity_parameters['threshold'] = 0.7
        self.rf_te.adaptivity_parameters['stopping_criterion'] = "cumulative"
        self.rf_te.adaptivity_parameters['error_calculator'] = "l2"
        self.rf_te.adaptivity_parameters['anisotropic_refinement'] = True
        self.rf_te.adaptivity_parameters['finer_reference_solution'] = False
        self.rf_te.adaptivity_parameters['space_refinement'] = False
        self.rf_te.adaptivity_parameters['order_increase'] = 2
        self.rf_te.solver = "linear"
                
        # boundaries
        self.rf_te.add_boundary("Perfect electric conductor", "rf_te_electric_field", {"rf_te_electric_field_real" : 0, "rf_te_electric_field_imag" : 0})
        self.rf_te.add_boundary("Matched boundary", "rf_te_magnetic_field", {"rf_te_magnetic_field_real" : 0, "rf_te_magnetic_field_imag" : 0})
        self.rf_te.add_boundary("Source", "rf_te_electric_field", {"rf_te_electric_field_real" : { "expression" : "cos(y/0.01143*pi/2)" }, "rf_te_electric_field_imag" : 0})
                
        # materials
        self.rf_te.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 0, "rf_te_current_density_external_real" : 0, "rf_te_current_density_external_imag" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.17, -0.01143, 0.17, 0.01143, boundaries = {"rf_te" : "Matched boundary"})
        geometry.add_edge(0, 0.01143, 0, -0.01143, boundaries = {"rf_te" : "Source"})
        geometry.add_edge(0.076, 0.01143, 0.076, 0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, 0.0045, 0.081, 0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, 0.0045, 0.081, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, -0.0045, 0.081, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, -0.0045, 0.076, -0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, -0.0045, 0.076, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0, -0.01143, 0.076, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, -0.01143, 0.17, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.17, 0.01143, 0.081, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, 0.01143, 0, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        
        geometry.add_label(0.0367388, 0.0025708, area = 1e-05, materials = {"rf_te" : "Air"})
        agros2d.view.zoom_best_fit()
        problem.solve()
        
    def test_values(self):        
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point1 = self.rf_te.local_values(5.801e-02, 4.192e-03)
        self.value_test("Electric field", point1["E"], 1.725e-01)
        self.value_test("Flux density", point1["B"], 2.599e-09)
        
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestAdaptivityElectrostatic))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestAdaptivityAcoustic))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestAdaptivityElasticityBracket))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestAdaptivityMagneticProfileConductor))    
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestAdaptivityRF_TE))    
    suite.run(result)