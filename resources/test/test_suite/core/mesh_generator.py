import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestMeshGenerator(Agros2DTestCase):
    def setUp(self):       
        # model
        self.problem = agros2d.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"

        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 2
        self.heat.polynomial_order = 2
        self.heat.solver = "linear"

        self.heat.add_boundary("T inner", "heat_temperature", {"heat_temperature" : -15})
        self.heat.add_boundary("T outer", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 20, "heat_convection_external_temperature" : 20})
        self.heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})

        self.heat.add_material("Material 1", {"heat_volume_heat" : 0, "heat_conductivity" : 2, "heat_density" : 7800, "heat_velocity_x" : 0.00002, "heat_velocity_y" : -0.00003, "heat_specific_heat" : 300, "heat_velocity_angular" : 0})
        self.heat.add_material("Material 2", {"heat_volume_heat" : 70000, "heat_conductivity" : 10, "heat_density" : 0, "heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_specific_heat" : 0, "heat_velocity_angular" : 0})

        # geometry
        geometry = agros2d.geometry

        # edges
        geometry.add_edge(0.1, 0.15, 0, 0.15, boundaries = {"heat" : "T outer"})
        geometry.add_edge(0, 0.15, 0, 0.1, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0, 0.1, 0.05, 0.1, boundaries = {"heat" : "T inner"})
        geometry.add_edge(0.05, 0.1, 0.05, 0, boundaries = {"heat" : "T inner"})
        geometry.add_edge(0.05, 0, 0.1, 0, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.05, 0.1, 0.1, 0.1)
        geometry.add_edge(0.1, 0.15, 0.1, 0.1, boundaries = {"heat" : "T outer"})
        geometry.add_edge(0.1, 0.1, 0.1, 0, boundaries = {"heat" : "T outer"})

        # labels
        geometry.add_label(0.0553981, 0.124595, materials = {"heat" : "Material 1"}, area=0.0003)
        geometry.add_label(0.070091, 0.068229, materials = {"heat" : "Material 2"}, area=0.0003)

        agros2d.view.zoom_best_fit()

    def test_triangle(self):
        self.problem.mesh_type = "triangle"
        self.solution_test_values()

    def test_triangle_quad_rough_division(self):
        self.problem.mesh_type = "triangle_quad_rough_division"
        self.solution_test_values()

    def test_triangle_quad_fine_division(self):
        self.problem.mesh_type = "triangle_quad_fine_division"
        self.solution_test_values()

    def test_triangle_quad_join(self):
        self.problem.mesh_type = "triangle_quad_join"
        self.solution_test_values()

    def test_gmsh_triangle(self):
        self.problem.mesh_type = "gmsh_triangle"
        self.solution_test_values()

    def test_gmsh_quad(self):
        self.problem.mesh_type = "gmsh_quad"
        self.solution_test_values()

    def test_gmsh_quad_delaunay(self):
        self.problem.mesh_type = "gmsh_quad_delaunay"
        self.solution_test_values()
        
    def solution_test_values(self):
       self.problem.solve()
        
       # point value
       point = self.heat.local_values(0.079734, 0.120078)
       self.value_test("Temperature", point["T"], 2.76619)
       self.value_test("Gradient", point["G"], 299.50258)
       self.value_test("Gradient - x", point["Gx"], -132.7564285)
       self.value_test("Gradient - y", point["Gy"], -268.47258)
       self.value_test("Heat flux", point["F"], 599.00516)
       self.value_test("Heat flux - x", point["Fx"], -265.512857)
       self.value_test("Heat flux - y", point["Fy"], -536.94516)

       # volume integral
       volume = self.heat.volume_integrals([0])
       self.value_test("Temperature", volume["T"], 0.00335)

       # surface integral
       surface = self.heat.surface_integrals([0, 6, 7])
       self.value_test("Heat flux", surface["f"], -85.821798)
    
if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMeshGenerator))
    suite.run(result)
    