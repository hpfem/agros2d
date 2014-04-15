import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult
                                    
class TestMagneticTransientPlanar(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.time_step_method = "fixed"
        problem.time_method_order = 2
        problem.time_total = 0.4
        problem.time_steps = 50
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # magnetic
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "transient"
        self.magnetic.transient_initial_condition = 0
        self.magnetic.number_of_refinements = 3
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"
        
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        # materials
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 57e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        self.magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e7*(exp(-10/0.7*time) - exp(-12/0.7*time))" }})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(-0.75, 0.75, -0.75, -0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.75, 0.75, 0.75, 0.75, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.75, 0.75, 0.75, -0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.75, -0.25, -0.75, -0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.25, 0, 0.2, 0.05)
        geometry.add_edge(0.1, 0.2, 0.2, 0.05)
        geometry.add_edge(0.1, 0.2, -0.2, 0.1)
        geometry.add_edge(-0.2, 0.1, -0.25, 0)
        geometry.add_edge(-0.2, 0.2, -0.05, 0.25)
        geometry.add_edge(-0.05, 0.35, -0.05, 0.25)
        geometry.add_edge(-0.05, 0.35, -0.2, 0.35)
        geometry.add_edge(-0.2, 0.35, -0.2, 0.2)
        
        geometry.add_label(0.1879, 0.520366, materials = {"magnetic" : "Air"})
        geometry.add_label(-0.15588, 0.306142, materials = {"magnetic" : "Coil"})
        geometry.add_label(-0.00331733, 0.106999, materials = {"magnetic" : "Copper"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):
        # point value
        point = self.magnetic.local_values(2.809e-02, 1.508e-01)
        self.value_test("Magnetic potential", point["Ar"], 3.856287421197996E-4)
        self.value_test("Flux density", point["Br"], 0.001116683242432341)
        self.value_test("Flux density - x", point["Brx"], 4.0104560252330734E-4)
        self.value_test("Flux density - y", point["Bry"], 0.001042182242829712)
        self.value_test("Magnetic intensity", point["Hr"], 888.628289504962)
        self.value_test("Magnetic intensity - x", point["Hrx"], 319.1419502342593)
        self.value_test("Magnetic intensity - y", point["Hry"], 829.3422777447333)
        self.value_test("Energy density", point["wm"], 0.49615819725609656)
        self.value_test("Losses density ", point["pj"], 28.304)
        self.value_test("Current density - induced transform", point["Jitr"], 40837.7342798334)
        self.value_test("Current density - total", point["Jr"], 40837.7342798334)
        self.value_test("Lorenz force - x", point["Flx"], -41.847933217335076)
        self.value_test("Lorenz force - y", point["Fly"], 16.226339194728205)
        
        # volume integral
#        volume = self.magnetic.volume_integrals([2])
#        self.value_test("Energy", volume["Wm"], 0.04391581801480497)
#        self.value_test("Losses", volume["Pj"], 0.7546173357026923)
#        self.value_test("Lorentz force integral - x", volume["Flx"], -0.06460752047773814)
#        self.value_test("Lorentz force integral - y", volume["Fly"], 0.3846384661715725)
#        self.value_test("Current - induced transform", volume["Iitr"], 812.2394578364593)
#        self.value_test("Current - total1", volume["Ir"], 812.2394578364593)
        
#        volumeSource = self.magnetic.volume_integrals([1])
#        self.value_test("Current - total2", volumeSource["Ir"], 421.23575)
#        self.value_test("Current - external", volumeSource["Ier"], 421.23575)
                
class TestMagneticTransientAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        problem.time_step_method = "fixed"
        problem.time_method_order = 2
        problem.time_total = 0.30
        problem.time_steps = 30
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "transient"
        self.magnetic.transient_initial_condition = 0
        self.magnetic.number_of_refinements = 3
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"
        
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        self.magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e7*(exp(-10/0.7*time) - exp(-12/0.7*time))" }})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, -0.5, 0.5, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.5, 0, 0, 0.5, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.5, 0, 0.15, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.15, 0, -0.15, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.15, 0, -0.5, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.05, 0.15, 0, 0.15)
        geometry.add_edge(0.05, 0.15, 0.1, -0.15)
        geometry.add_edge(0.1, -0.15, 0, -0.15)
        geometry.add_edge(0.15, -0.05, 0.25, -0.05)
        geometry.add_edge(0.25, 0.05, 0.15, 0.05)
        geometry.add_edge(0.15, 0.05, 0.15, -0.05)
        geometry.add_edge(0.25, 0.05, 0.25, -0.05)
        
        geometry.add_label(0.19253, -0.00337953, materials = {"magnetic" : "Coil"})
        geometry.add_label(0.051315, -0.0143629, materials = {"magnetic" : "Copper"})
        geometry.add_label(0.165856, 0.213151, materials = {"magnetic" : "Air"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
                        
    def test_values(self):
        # point value
        point = self.magnetic.local_values(5.340e-02, -2.316e-02)
        self.value_test("Magnetic potential", point["Ar"], 1.5449179762269398E-4)
        self.value_test("Flux density", point["Br"], 0.004639593632573921)
        self.value_test("Flux density - x", point["Brr"], 1.0775445209791154E-4)
        self.value_test("Flux density - y", point["Brz"],0.004638342128175171)
        self.value_test("Magnetic intensity", point["Hr"], 	3692.0713028091122)
        self.value_test("Magnetic intensity - x", point["Hrr"], 85.74826845770738)
        self.value_test("Magnetic intensity - y", point["Hrz"], 3691.0753872522996)
        self.value_test("Energy density", point["wm"], 8.564855460177437)
        self.value_test("Losses density ", point["pj"], 102.79740499112752)
        self.value_test("Current density - external", point["Je"], 0.0)
        self.value_test("Current density - induced transform", point["Jitr"], 76547.05797412641)
        self.value_test("Current density - total", point["Jr"], 76547.05797412641)
        
        # volume integral
        volume = self.magnetic.volume_integrals([1])
        self.value_test("Energy", volume["Wm"], 0.03185301819233872)
        self.value_test("Losses", volume["Pj"], 0.36062893759748205)
        self.value_test("Current - external", volume["Ier"], 0.0)
        self.value_test("Current - induced transform", volume["Iitr"], 965.8993026017961)
        self.value_test("Current - total", volume["Ir"], 965.8993026017961)


if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticTransientPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticTransientAxisymmetric))
    suite.run(result)
