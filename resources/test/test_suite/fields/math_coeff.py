import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestMathCoeffPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.math_coeff = agros2d.field("math_coeff")
        self.math_coeff.analysis_type = "steadystate"
        self.math_coeff.matrix_solver = "mumps"
        self.math_coeff.number_of_refinements = 3
        self.math_coeff.polynomial_order = 3
        self.math_coeff.adaptivity_type = "disabled"
        self.math_coeff.solver = "linear"
        
        # boundaries
        self.math_coeff.add_boundary("Source", "math_coeff_solution", {"math_coeff_solution" : 1})
        self.math_coeff.add_boundary("Neumann", "math_coeff_flux", {"math_coeff_flux" : 10})
        
        # materials
        self.math_coeff.add_material("Material", {"math_coeff_c" : 2, "math_coeff_a" : 3, "math_coeff_beta_x" : 1, "math_coeff_beta_y" : 2, "math_coeff_f" : 5})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(-0.95, 0.6, 0.45, 0.8, boundaries = {"math_coeff" : "Neumann"})
        geometry.add_edge(0.45, 0.8, 0.95, 0.05, boundaries = {"math_coeff" : "Neumann"})
        geometry.add_edge(0.95, 0.05, 0.5, -0.95, boundaries = {"math_coeff" : "Neumann"})
        geometry.add_edge(0.5, -0.95, -1.05, -0.3, boundaries = {"math_coeff" : "Neumann"})
        geometry.add_edge(-0.95, 0.6, -1.05, -0.3, boundaries = {"math_coeff" : "Neumann"})
        geometry.add_edge(0.1, 0.2, 0.15, -0.3, boundaries = {"math_coeff" : "Source"})
        geometry.add_edge(0.4, 0.15, 0.6, -0.2, boundaries = {"math_coeff" : "Source"})
        geometry.add_edge(0.6, -0.2, 0.15, -0.3, boundaries = {"math_coeff" : "Source"})
        geometry.add_edge(0.1, 0.2, 0.4, 0.15, boundaries = {"math_coeff" : "Source"})
        geometry.add_edge(-0.55, 0.3, -0.7, 0.1, boundaries = {"math_coeff" : "Source"})
        geometry.add_edge(-0.55, 0.3, -0.4, 0.2, boundaries = {"math_coeff" : "Source"})
        geometry.add_edge(-0.7, 0.1, -0.4, 0.2, boundaries = {"math_coeff" : "Source"})
        
        # labels
        geometry.add_label(0.350372, -0.0349351, materials = {"math_coeff" : "none"})
        geometry.add_label(0.270465, 0.500445, materials = {"math_coeff" : "Material"})
        geometry.add_label(-0.540581, 0.239645, materials = {"math_coeff" : "none"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):
        # point value
        point = self.math_coeff.local_values(-6.800e-01, -2.526e-02)
        self.value_test("Solution", point["u"], 3.1225)
        self.value_test("Gradient", point["g"], 10.695858)
        self.value_test("Gradient - x", point["gx"], -4.84176)
        self.value_test("Gradient - y", point["gy"], -9.53723)
        self.value_test("Flux", point["f"], 10.695858 * 2)
        self.value_test("Flux - x", point["fx"], -4.84176 * 2)
        self.value_test("Flux - y", point["fy"], -9.53723 * 2)
        
        # volume integral
        volume = self.math_coeff.volume_integrals([1])
        self.value_test("Solution", volume["u"], 7.07878)
        
        # surface integral
        surface = self.math_coeff.surface_integrals([0, 1, 2, 3, 4])
        self.value_test("Gradient", surface["g"], 59.98499 / 2)
        self.value_test("Flux", surface["f"], 59.98499)
"""   
class TestMathCoeffAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.current = agros2d.field("current")
        self.current.analysis_type = "steadystate"
        self.current.number_of_refinements = 1
        self.current.polynomial_order = 4
        self.current.solver = "linear"
        
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        self.current.add_boundary("Ground", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("Voltage", "current_potential", {"current_potential" : 10})
        self.current.add_boundary("Inlet", "current_inward_current_flow", {"current_inward_current_flow" : -3e9})
        
        self.current.add_material("Copper", {"current_conductivity" : 5.7e7})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, 0.45, 0, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0, 0.2, 0, boundaries = {"current" : "Ground"})
        geometry.add_edge(0.2, 0, 0.2, 0.15, boundaries = {"current" : "Inlet"})
        geometry.add_edge(0.2, 0.15, 0.35, 0.45, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.35, 0.45, 0, 0.45, boundaries = {"current" : "Voltage"})
        
        # labels
        geometry.add_label(0.0933957, 0.350253, materials = {"current" : "Copper"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):
        # point value
        point = self.current.local_values(0.213175, 0.25045)
        self.value_test("Scalar potential", point["V"], 5.566438)
        self.value_test("Electric field", point["Er"], 32.059116)
        self.value_test("Electric field - r", point["Err"], -11.088553)
        self.value_test("Electric field - z", point["Erz"], -30.080408)
        self.value_test("MathCoeff density", point["Jrc"], 1.82737e9)
        self.value_test("MathCoeff density - r", point["Jrcr"], -6.320475e8)
        self.value_test("MathCoeff density - z", point["Jrcz"], -1.714583e9)
        self.value_test("Losses", point["pj"], 5.858385e10)	
        
        # volume integral
        volume = self.current.volume_integrals([0])
        self.value_test("Losses", volume["Pj"], 4.542019e9)
        
        # surface integral
        surface = self.current.surface_integrals([1])
        self.value_test("MathCoeff", surface["Ir"], -2.166256e8)        
"""
        
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMathCoeffPlanar))
    #suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMathCoeffAxisymmetric))
    suite.run(result)
