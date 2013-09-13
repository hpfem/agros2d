import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class AdaptivityElectrostatic(Agros2DTestCase):
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

class AdaptivityAcoustic(Agros2DTestCase):
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
        
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(AdaptivityElectrostatic))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(AdaptivityAcoustic))
    suite.run(result)