import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestElectrostaticPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.electrostatic = agros2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 2
        self.electrostatic.polynomial_order = 3
        self.electrostatic.solver = "linear"
        
        self.electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.electrostatic.add_boundary("U = 0 V", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("U = 1000 V", "electrostatic_potential", {"electrostatic_potential" : 1000})
        
        self.electrostatic.add_material("Dieletric", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 3})
        self.electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
        self.electrostatic.add_material("Source", {"electrostatic_charge_density" : 4e-10, "electrostatic_permittivity" : 10})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(1, 2, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(4, 1, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(1, 2, 4, 2, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(4, 2, 4, 1, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(20, 24, 20, 1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(20, 1, 20, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(4, 1, 20, 1, boundaries = {})
        geometry.add_edge(0, 24, 0, 1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, 0, 0, 1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, 0, 20, 0, boundaries = {"electrostatic" : "U = 0 V"})
        geometry.add_edge(0, 24, 20, 24, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, 1, 1, 1, boundaries = {})
        geometry.add_edge(7, 13, 14, 13, boundaries = {})
        geometry.add_edge(14, 13, 14, 18, boundaries = {})
        geometry.add_edge(14, 18, 7, 18, boundaries = {})
        geometry.add_edge(7, 18, 7, 13, boundaries = {})
        
        # labels
        geometry.add_label(2.78257, 1.37346, materials = {"electrostatic" : "none"})
        geometry.add_label(10.3839, 15.7187, area = 0.2, materials = {"electrostatic" : "Source"})
        geometry.add_label(3.37832, 15.8626, materials = {"electrostatic" : "Air"})
        geometry.add_label(12.3992, 0.556005, materials = {"electrostatic" : "Dieletric"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):
        # point value
        local_values = self.electrostatic.local_values(13.257584, 11.117738)
        self.value_test("Scalar potential", local_values["V"], 1111.544825)
        self.value_test("Electric field", local_values["E"], 111.954358)
        self.value_test("Electric field - x", local_values["Ex"], 24.659054)
        self.value_test("Electric field - y", local_values["Ey"], -109.204896)
        self.value_test("Displacement", local_values["D"], 9.912649e-10)
        self.value_test("Displacement - x", local_values["Dx"], 2.183359e-10)
        self.value_test("Displacement - y", local_values["Dy"], -9.669207e-10)
        self.value_test("Energy density", local_values["we"], 5.548821e-8)
  
        # volume integral
        volume_integrals = self.electrostatic.volume_integrals([1])
        self.value_test("Energy", volume_integrals["We"], 1.307484e-7)
        self.value_test("Volume Maxwell force - x", volume_integrals["Ftx"], 7.592e-8, 0.1)
        self.value_test("Volume Maxwell force - y", volume_integrals["Fty"], -5.546e-7, 0.1)    
        
        # surface integral
        surface_integrals = self.electrostatic.surface_integrals([0, 1, 2, 3])
        self.value_test("Electric charge", surface_integrals["Q"], 1.048981e-7)
            
class TestElectrostaticAxisymmetric(Agros2DTestCase):
    def setUp(self):       
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.electrostatic = agros2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 5
        self.electrostatic.solver = "linear"
        
        self.electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : 10})
        self.electrostatic.add_boundary("Ground electrode", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Neumann BC", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        self.electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
        self.electrostatic.add_material("Dielectric n.1", {"electrostatic_charge_density" : 1e-5, "electrostatic_permittivity" : 10})
        self.electrostatic.add_material("Dielectric n.2", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 3})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, 0.2, 0, 0.08, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.01, 0.08, 0.01, 0, boundaries = {"electrostatic" : "Source electrode"})
        geometry.add_edge(0.01, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.03, 0, 0.03, 0.08)
        geometry.add_edge(0.03, 0.08, 0.05, 0.08)
        geometry.add_edge(0.05, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.05, 0.08, 0.05, 0, boundaries = {"electrostatic" : "Ground electrode"})
        geometry.add_edge(0.06, 0, 0.06, 0.08, boundaries = {"electrostatic" : "Ground electrode"})
        geometry.add_edge(0.05, 0.08, 0.06, 0.08, boundaries = {"electrostatic" : "Ground electrode"})
        geometry.add_edge(0.06, 0, 0.2, 0, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.01, 0.08, 0.03, 0.08)
        geometry.add_edge(0.01, 0.08, 0, 0.08, boundaries = {"electrostatic" : "Source electrode"})
        geometry.add_edge(0.2, 0, 0, 0.2, boundaries = {"electrostatic" : "Neumann BC"}, angle=90)
        
        # labels
        geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric n.1"})
        geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric n.2"})
        geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
        
        # solve
        agros2d.view.zoom_best_fit()
        problem.solve()

    def test_values(self):
        # point value
        point = self.electrostatic.local_values(0.0255872, 0.0738211)
        
        self.value_test("Scalar potential", point["V"], 25.89593)
        self.value_test("Electric field", point["E"], 151.108324)
        self.value_test("Electric field - r", point["Er"], 94.939342)
        self.value_test("Electric field - z", point["Ez"], 117.559546)
        self.value_test("Displacement", point["D"], 1.337941e-8)
        self.value_test("Displacement - r", point["Dr"], 8.406108e-9)
        self.value_test("Displacement - z", point["Dz"], 1.040894e-8)
        self.value_test("Energy density", point["we"], 1.01087e-6)
                
        # volume integral
        volume = self.electrostatic.volume_integrals([0, 1, 2])
        self.value_test("Energy", volume["We"], 1.799349e-8)
    
        # surface integral
        surface = self.electrostatic.surface_integrals([1, 12])
        self.value_test("Electric charge", surface["Q"], -1.291778e-9)

if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestElectrostaticPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestElectrostaticAxisymmetric))
    suite.run(result)
    