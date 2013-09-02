import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class HeatPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"

        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 2
        self.heat.polynomial_order = 3
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

        # solve problem
        problem.solve()

    def test_values(self):        
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

class HeatAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"

        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 2
        self.heat.polynomial_order = 3
        self.heat.solver = "linear"

        self.heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})
        self.heat.add_boundary("Inlet", "heat_heat_flux", {"heat_heat_flux" : 500000, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})
        self.heat.add_boundary("Temperature", "heat_temperature", {"heat_temperature" : 0})
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 20, "heat_convection_external_temperature" : 20})

        self.heat.add_material("Material", {"heat_volume_heat" : 6e+06, "heat_conductivity" : 52, "heat_density" : 7800, "heat_velocity_x" : 0, "heat_velocity_y" : 0.001, "heat_specific_heat" : 300, "heat_velocity_angular" : 0})
                    
        # geometry
        geometry = agros2d.geometry

        # edges
        geometry.add_edge(0.02, 0, 0.1, 0, boundaries = {"heat" : "Temperature"})
        geometry.add_edge(0.1, 0, 0.1, 0.14, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.1, 0.14, 0.02, 0.14, boundaries = {"heat" : "Temperature"})
        geometry.add_edge(0.02, 0.14, 0.02, 0.1, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.02, 0.04, 0.02, 0, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.02, 0.04, 0.02, 0.1, boundaries = {"heat" : "Inlet"})

        # labels
        geometry.add_label(0.0460134, 0.0867717, materials = {"heat" : "Material"}, area=0.0003)

        agros2d.view.zoom_best_fit()

        # solve problem
        problem.solve()

    def test_values(self):   
        # point value
        point = self.heat.local_values(0.062926, 0.038129)
        self.value_test("Temperature", point["T"], 105.414118)
        self.value_test("Gradient", point["G"], 2890.873908)
        self.value_test("Gradient - r", point["Gr"], 370.891549)
        self.value_test("Gradient - z", point["Gz"], -2866.982692)
        self.value_test("Heat flux", point["F"], 1.503254e5)
        self.value_test("Heat flux - r", point["Fr"], 19286.360576)
        self.value_test("Heat flux - z", point["Fz"], -1.490831e5)

        # volume integral
        volume = self.heat.volume_integrals([0])
        self.value_test("Temperature", volume["T"], 0.616202)

        # surface integral
        surface = self.heat.surface_integrals([1])
        self.value_test("Heat flux", surface["f"], 199.0004)
        
class HeatNonlinPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"

        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 2
        self.heat.polynomial_order = 2

        self.heat.solver = "newton"
        self.heat.solver_parameters['tolerance'] = 0.0001
        self.heat.solver_parameters['damping'] = 'automatic'
        self.heat.solver_parameters['damping_factor'] = 0.8
        self.heat.solver_parameters['jacobian_reuse'] = True
        self.heat.solver_parameters['jacobian_reuse_ratio'] = 0.3
        self.heat.solver_parameters['jacobian_reuse_steps'] = 20

        self.heat.add_boundary("Left", "heat_temperature", {"heat_temperature" : 10})
        self.heat.add_boundary("Radiace", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 20, "heat_radiation_emissivity" : 0.9})
        self.heat.add_boundary("Neumann", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 50, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})

        self.heat.add_material("Material - nonlin", {"heat_conductivity" : { "x" : [0,100,200,290,500,1000], "y" : [210,280,380,430,310,190], "interpolation" : "cubic_spline", "extrapolation" : "constant", "derivative_at_endpoints" : "first"}, "heat_volume_heat" : 2e6})
        self.heat.add_material("Material", {"heat_conductivity" : 230, "heat_volume_heat" : 0})

        # geometry
        geometry = agros2d.geometry

        # edges
        geometry.add_edge(-0.25, 0.25, -0.1, 0.1, boundaries = {"heat" : "Convection"})
        geometry.add_edge(-0.1, 0.1, 0.05, 0.2, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.05, 0.2, 0.25, 0.25, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.25, 0.25, 0.1, 0.1, boundaries = {"heat" : "Radiace"})
        geometry.add_edge(0.1, 0.1, 0.25, -0.25, boundaries = {"heat" : "Radiace"})
        geometry.add_edge(0.25, -0.25, 0.05, -0.05, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.05, -0.05, -0.25, -0.25, boundaries = {"heat" : "Neumann"}, angle = 90)
        geometry.add_edge(-0.25, -0.05, 0.1, 0.1, boundaries = {})
        geometry.add_edge(-0.25, 0.25, -0.25, -0.05, boundaries = {"heat" : "Left"})
        geometry.add_edge(-0.25, -0.05, -0.25, -0.25, boundaries = {"heat" : "Left"})

        # labels
        geometry.add_label(-0.0150215, 0.018161, materials = {"heat" : "Material"})
        geometry.add_label(-0.183934, 0.0732177, materials = {"heat" : "Material - nonlin"})

        agros2d.view.zoom_best_fit()

        # solve problem
        problem.solve()

    def test_values(self):   
        # point value
        point = self.heat.local_values(8.620e-02, 1.620e-01)
        self.value_test("Temperature", point["T"], 357.17654)
        self.value_test("Gradient", point["G"], 444.435957)
        self.value_test("Heat flux", point["F"], 1.805517e5)

        # volume integral
        volume = self.heat.volume_integrals([1])
        self.value_test("Temperature", volume["T"], 12.221687)

        # surface integral
        surface = self.heat.surface_integrals([8])
        self.value_test("Heat flux", surface["f"], 96464.56418)
        
class HeatTransientBenchmarkAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # benchmark 
        #
        # A.D. Cameron, J. A. Casey, and G.B. Simpson: 
        # NAFEMS Benchmark Tests for Thermal Analysis (Summary), 
        # NAFEMS Ltd., Glasgow, 1986

        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"

        problem.time_step_method = "adaptive"
        problem.time_method_order = 3
        problem.time_method_tolerance = 1.0
        problem.time_steps = 20
        problem.time_total = 190

        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # fields
        # heat
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "transient"
        self.heat.transient_initial_condition = 0
        self.heat.number_of_refinements = 2
        self.heat.polynomial_order = 3
        self.heat.solver = "linear"

        self.heat.add_boundary("Symmetry", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0, "heat_radiation_emissivity" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0})
        self.heat.add_boundary("Temperature", "heat_temperature", {"heat_temperature" : 1000})

        self.heat.add_material("Material", {"heat_volume_heat" : 0, "heat_conductivity" : 52, "heat_velocity_x" : 0, "heat_density" : 7850, "heat_velocity_y" : 0, "heat_specific_heat" : 460, "heat_velocity_angular" : 0})

        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 0.4, 0, 0, boundaries = {"heat" : "Symmetry"})
        geometry.add_edge(0.3, 0.4, 0.3, 0, boundaries = {"heat" : "Temperature"})
        geometry.add_edge(0.3, 0, 0, 0, boundaries = {"heat" : "Temperature"})
        geometry.add_edge(0, 0.4, 0.3, 0.4, boundaries = {"heat" : "Temperature"})

        geometry.add_label(0.151637, 0.112281, materials = {"heat" : "Material"}, area = 0.01)

        agros2d.view.zoom_best_fit()

        # solve problem
        problem.solve()

    def test_values(self):
        # point value
        point = self.heat.local_values(0.1, 0.3)
        self.value_test("Temperature", point["T"], 186.5, 0.0004) # permissible error 0.02 %
        
class HeatTransientAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        problem.time_step_method = "fixed"
        problem.time_method_order = 2
        problem.time_method_tolerance = 1
        problem.time_total = 10000
        problem.time_steps = 20
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "transient"
        self.heat.number_of_refinements = 1
        self.heat.polynomial_order = 5
        self.heat.solver = "linear"
        self.heat.solver_parameters['tolerance'] = 0.001
        self.heat.solver_parameters['steps'] = 10
        self.heat.transient_initial_condition = 20
        
        self.heat.add_boundary("Flux", "heat_heat_flux", {"heat_heat_flux" : 0})
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 10, "heat_convection_external_temperature" : 20})
        
        self.heat.add_material("Air", {"heat_conductivity" : 0.02, "heat_volume_heat" : 0, "heat_density" : 1.2, "heat_specific_heat" : 1000}) 
        self.heat.add_material("Cu", {"heat_conductivity" : 200, "heat_volume_heat" : 26000, "heat_density" : 8700, "heat_specific_heat" : 385}) 
        self.heat.add_material("Fe", {"heat_conductivity" : 60, "heat_volume_heat" : 0, "heat_density" : 7800, "heat_specific_heat" : 460}) 
        self.heat.add_material("Brass", {"heat_conductivity" : 100, "heat_volume_heat" : 0, "heat_density" : 8400, "heat_specific_heat" : 378}) 
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, 0.18, 0.035, 0.18, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.035, 0.18, 0.035, 0.03, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.035, 0.03, 0.0135, 0.03, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.0135, 0.03, 0.0135, 0.038, 0,)
        geometry.add_edge(0.0135, 0.038, 0.019, 0.038, 0,)
        geometry.add_edge(0.027, 0.038, 0.027, 0.172, 0,)
        geometry.add_edge(0.027, 0.172, 0.008, 0.172, 0,)
        geometry.add_edge(0.008, 0.172, 0.008, 0.16, 0,)
        geometry.add_edge(0.008, 0.16, 0, 0.16, 0,)
        geometry.add_edge(0, 0.16, 0, 0.18, 0, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0, 0.13, 0.008, 0.13, 0,)
        geometry.add_edge(0.008, 0.13, 0.008, 0.0395, 0,)
        geometry.add_edge(0.008, 0, 0, 0, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0, 0, 0, 0.13, 0, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0.019, 0.038, 0.027, 0.038, 0,)
        geometry.add_edge(0.019, 0.038, 0.019, 0.0395, 0,)
        geometry.add_edge(0, 0.13, 0, 0.16, 0, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0.01, 0.041, 0.01, 0.1705, 0,)
        geometry.add_edge(0.025, 0.041, 0.01, 0.041, 0,)
        geometry.add_edge(0.01, 0.1705, 0.025, 0.1705, 0,)
        geometry.add_edge(0.025, 0.1705, 0.025, 0.041, 0,)
        geometry.add_edge(0.008, 0.0395, 0.008, 0.03, 0,)
        geometry.add_edge(0.008, 0.03, 0.008, 0, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.019, 0.0395, 0.009, 0.0395, 0,)
        geometry.add_edge(0.008, 0.03, 0.009, 0.03, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.009, 0.0395, 0.009, 0.03, 0,)
        geometry.add_edge(0.009, 0.03, 0.0135, 0.03, 0, boundaries = {"heat" : "Convection"})
        
        # labels
        geometry.add_label(0.0308709, 0.171031, materials = {"heat" : "Fe"})
        geometry.add_label(0.00316251, 0.011224, materials = {"heat" : "Fe"})
        geometry.add_label(0.0163723, 0.144289, materials = {"heat" : "Cu"})
        geometry.add_label(0.00380689, 0.151055, materials = {"heat" : "Air"})
        geometry.add_label(0.0112064, 0.0336487, materials = {"heat" : "Brass"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):
        # point value
        point = self.heat.local_values(0.00503, 0.134283)
        self.value_test("Temperature", point["T"], 44.012004)
        self.value_test("Heat flux", point["F"], 16.739787)
        self.value_test("Heat flux - r", point["Fr"], -7.697043)
        self.value_test("Heat flux - z", point["Fz"], -14.865261)
        
        # volume integral
        volume = self.heat.volume_integrals([3])
        self.value_test("Temperature", volume["T"], 0.002839)
        
        # surface integral
        surface = self.heat.surface_integrals([26])
        self.value_test("Heat flux", surface["f"], 0.032866, error = 0.05)
        
if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(HeatPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(HeatAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(HeatNonlinPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(HeatTransientAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(HeatTransientBenchmarkAxisymmetric))
    suite.run(result)
    