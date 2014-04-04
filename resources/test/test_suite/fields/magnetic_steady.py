import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestMagneticPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        self.problem = agros2d.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"
        self.problem.frequency = 100
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 5
        self.magnetic.solver = "linear"
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Fe", {"magnetic_permeability" : 500}) 
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.11, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : -30}) 
        self.magnetic.add_material("Velocity", {"magnetic_permeability" : 1, "magnetic_conductivity" : 1e7, "magnetic_velocity_x" : 2})
        
        # geometry
        self.geometry = agros2d.geometry
        
        # edges
        self.geometry.add_edge(-0.5, 1, 0.5, 1, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.5, -1, -0.5, -1, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.2, 0.6, 0, 0.6)
        self.geometry.add_edge(0, 0.3, 0, 0.6)
        self.geometry.add_edge(-0.2, 0.3, 0, 0.3)
        self.geometry.add_edge(-0.2, 0.6, -0.2, 0.3)
        self.geometry.add_edge(-0.035, 0.135, 0.035, 0.135)
        self.geometry.add_edge(0.035, 0.135, 0.035, -0.04)
        self.geometry.add_edge(0.035, -0.04, -0.035, -0.04)
        self.geometry.add_edge(-0.035, 0.135, -0.035, -0.04)
        self.geometry.add_edge(0, -0.5, 0.3, -0.5)
        self.geometry.add_edge(0.3, -0.5, 0.3, -0.32)
        self.geometry.add_edge(0.3, -0.32, 0, -0.32)
        self.geometry.add_edge(0, -0.32, 0, -0.5)
        self.geometry.add_edge(-0.5, 1, -0.5, -0.15, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.5, -0.15, -0.5, -0.2, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.5, -0.2, -0.5, -1, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.5, 1, 0.5, -0.15, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.5, -0.15, 0.5, -0.2, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.5, -0.2, 0.5, -1, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.5, -0.2, -0.5, -0.2)
        self.geometry.add_edge(0.5, -0.15, -0.5, -0.15)
        
        # labels
        self.geometry.add_label(-0.0959509, 0.445344, area = 0.0002, materials = {"magnetic" : "Fe"})
        self.geometry.add_label(-0.145434, -0.706253, area = 0.01, materials = {"magnetic" : "Air"})
        self.geometry.add_label(0.143596, -0.364811, materials = {"magnetic" : "Magnet"})
        self.geometry.add_label(0.310203, 0.631164, materials = {"magnetic" : "Air"})
        self.geometry.add_label(-0.295858, -0.182894, materials = {"magnetic" : "Velocity"})
        
        agros2d.view.zoom_best_fit()
        

    def general_test_values(self):                  
        # point value
        point = self.magnetic.local_values(0.018895, -0.173495)
        self.value_test("Magnetic potential", point["Ar"], 0.002978)
        self.value_test("Flux density", point["Br"], 0.013047)
        self.value_test("Flux density - x", point["Brx"], 0.008413)
        self.value_test("Flux density - y", point["Bry"], -0.009972)
        self.value_test("Magnetic intensity", point["Hr"], 10382.294319)
        self.value_test("Magnetic intensity - x", point["Hrx"], 6695.236126)
        self.value_test("Magnetic intensity - y", point["Hry"], -7935.102302)
        self.value_test("Energy density", point["wm"], 67.727733)
        self.value_test("Losses density ", point["pj"], 3977.255622)
        self.value_test("Current density - induced velocity", point["Jivr"], -1.987738e5)
        self.value_test("Current density - total", point["Jr"], -1.987738e5)
        self.value_test("Lorentz force - x", point["Flx"], -1975.551246)
        self.value_test("Lorentz force - y", point["Fly"], -1671.99571)
        
        # volume integral
        volume = self.magnetic.volume_integrals([4])
        self.value_test("Energy", volume["Wm"], 3.088946)
        self.value_test("Losses", volume["Pj"], 220.022114)
        self.value_test("Lorentz force - x", volume["Flx"], -110.011057)
        self.value_test("Lorentz force - y", volume["Fly"], -36.62167)
        self.value_test("Torque", volume["Tl"], 20.463818)
        volume = self.magnetic.volume_integrals([0])
        self.value_test("Volume Maxwell force - x", volume["Ftx"], 2.66, 0.15)
        self.value_test("Volume Maxwell force - y", volume["Fty"], -11.87, 0.15)            
        
        # surface integral
        surface = self.magnetic.surface_integrals([2, 3, 4, 5])
        self.value_test("Surface Maxwell force - x", surface["Ftx"], 2.66, 0.15)
        self.value_test("Surface Maxwell force - y", surface["Fty"], -11.87, 0.15)        
        
    def test_values_total_current(self):
        self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 1e6*1.225e-2})
        self.geometry.add_label(0.00301448, 0.0404858, area = 0.005, materials = {"magnetic" : "Cu"})
        self.problem.solve()
        self.general_test_values()

    def test_values_current_density(self):
        self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6})
        self.geometry.add_label(0.00301448, 0.0404858, area = 0.005, materials = {"magnetic" : "Cu"})
        self.problem.solve()
        self.general_test_values()
            
class TestMagneticAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # model
        self.problem = agros2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 7
        self.magnetic.solver = "linear"
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Fe", {"magnetic_permeability" : 300}) 
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.6, "magnetic_remanence_angle" : 90}) 
        
        # geometry
        self.geometry = agros2d.geometry
        
        # edges
        self.geometry.add_edge(0.01, 0.01, 0.01, 0)
        self.geometry.add_edge(0.01, 0, 0.04, 0,)
        self.geometry.add_edge(0.04, 0.1, 0, 0.1,)
        self.geometry.add_edge(0, 0.1, 0, 0.08, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, 0.08, 0.01, 0.08)
        self.geometry.add_edge(0.01, 0.09, 0.03, 0.09)
        self.geometry.add_edge(0.01, 0.09, 0.01, 0.08)
        self.geometry.add_edge(0.01, 0.01, 0.03, 0.01)
        self.geometry.add_edge(0.012, 0.088, 0.012, 0.012)
        self.geometry.add_edge(0.012, 0.012, 0.028, 0.012)
        self.geometry.add_edge(0.028, 0.012, 0.028, 0.088)
        self.geometry.add_edge(0.028, 0.088, 0.012, 0.088)
        self.geometry.add_edge(0, 0.05, 0.009, 0.05)
        self.geometry.add_edge(0.009, 0.05, 0.009, -0.02)
        self.geometry.add_edge(0, 0.05, 0, -0.02, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, -0.02, 0.009, -0.02)
        self.geometry.add_edge(0, 0.15, 0, 0.1, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, 0.08, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, -0.02, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, -0.05, 0.1, 0.05, boundaries = {"magnetic" : "A = 0"}, angle=90)
        self.geometry.add_edge(0.1, 0.05, 0, 0.15, boundaries = {"magnetic" : "A = 0"}, angle=90)
        self.geometry.add_edge(0.03, 0.09, 0.03, 0.078)
        self.geometry.add_edge(0.03, 0.078, 0.04, 0.078)
        self.geometry.add_edge(0.04, 0.078, 0.04, 0.1)
        self.geometry.add_edge(0.04, 0.078, 0.04, 0.052)
        self.geometry.add_edge(0.03, 0.078, 0.03, 0.052)
        self.geometry.add_edge(0.03, 0.052, 0.04, 0.052)
        self.geometry.add_edge(0.04, 0.052, 0.04, 0)
        self.geometry.add_edge(0.03, 0.01, 0.03, 0.052)
        
        # labels
        self.geometry.add_label(0.0348743, 0.0347237, materials = {"magnetic" : "Fe"})
        self.geometry.add_label(0.00512569, -0.0070852, area = 0.8e-06, materials = {"magnetic" : "Fe"})
        self.geometry.add_label(0.0141705, 0.12445, materials = {"magnetic" : "Air"})
        self.geometry.add_label(0.0346923, 0.0892198, materials = {"magnetic" : "Fe"})
        self.geometry.add_label(0.036093, 0.0654078, materials = {"magnetic" : "Magnet"})
        
        agros2d.view.zoom_best_fit()
        
    def general_test_values(self):                  
        # point value
        point = self.magnetic.local_values(0.005985, 0.043924)
        self.value_test("Scalar potential", point["Ar"], 5.438198e-4)
        self.value_test("Flux density", point["Br"], 0.195525)
        self.value_test("Flux density - r", point["Brr"], 0.059085)
        self.value_test("Flux density - z", point["Brz"], 0.186384)
        self.value_test("Magnetic intensity", point["Hr"], 518.646027)
        self.value_test("Magnetic intensity - r", point["Hrr"], 156.728443)
        self.value_test("Magnetic intensity - z", point["Hrz"], 494.39852)
        self.value_test("Energy density", point["wm"], 50.704118)
        
        point = self.magnetic.local_values(0.0199415, 0.0609942)
        self.value_test("Lorentz force - r - real", point["Flr"], -1.237274e5)
        self.value_test("Lorentz force - z - real", point["Flz"], -41951.726164)
        
        # volume integral
        volume = self.magnetic.volume_integrals([1])
        self.value_test("Energy", volume["Wm"], 0.002273)
        
        volume = self.magnetic.volume_integrals([5])
        self.value_test("Integral Lorentz force - r", volume["Flx"], -8.069509) 
        self.value_test("Integral Lorentz force - z", volume["Fly"], -5.288991) 
        volume = self.magnetic.volume_integrals([1])        
        self.value_test("Volume Maxwell force - z", volume["Fty"], 0.429770, 0.1)
        
        # surface integral
        surface = self.magnetic.surface_integrals([12, 13, 14, 15])
        self.value_test("Surface Maxwell force - z", surface["Fty"], 0.429770, 0.1)
        
    def test_values_total_current(self):
        self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 2e6*1.216e-3 }) 
        self.geometry.add_label(0.021206, 0.0692964, materials = {"magnetic" : "Cu"})        
        self.problem.solve()
        self.general_test_values()

    def test_values_current_density(self):
        self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e6}) 
        self.geometry.add_label(0.021206, 0.0692964, materials = {"magnetic" : "Cu"})        
        self.problem.solve()
        self.general_test_values()
    
class TestMagneticNonlinPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['residual'] = 0.1
        self.magnetic.solver_parameters['damping_factor'] = 0.6
        self.magnetic.solver_parameters['damping'] = 'automatic'
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_ratio'] = 0.9
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 20
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Civka 1", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : 5e6, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        self.magnetic.add_material("Civka 2", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : -5e6, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        self.magnetic.add_material("Iron", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : { "value" : 995, "x" : [0,0.2,0.5,0.8,1.15,1.3,1.45,1.6,1.69,2,2.2,2.5,3,5,10,20], "y" : [995,995,991,933,771,651,473,311,245,40,30,25,20,8,5,2] }, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        self.magnetic.add_material("Air", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(-0.05, -0.01, 0.05, -0.01, boundaries = {})
        geometry.add_edge(0.05, -0.01, 0.05, -0.005, boundaries = {})
        geometry.add_edge(0.05, -0.005, -0.05, -0.005, boundaries = {})
        geometry.add_edge(-0.05, -0.005, -0.05, -0.01, boundaries = {})
        geometry.add_edge(-0.05, 0.06, -0.05, 0.04, boundaries = {})
        geometry.add_edge(-0.03, 0, -0.05, 0, boundaries = {})
        geometry.add_edge(-0.05, 0, -0.05, 0.04, boundaries = {})
        geometry.add_edge(-0.05, 0.06, 0.05, 0.06, boundaries = {})
        geometry.add_edge(0.05, 0.06, 0.05, 0.04, boundaries = {})
        geometry.add_edge(0.05, 0, 0.05, 0.04, boundaries = {})
        geometry.add_edge(0.03, 0, 0.05, 0, boundaries = {})
        geometry.add_edge(-0.028, 0.04, 0.028, 0.04, boundaries = {})
        geometry.add_edge(-0.15, -0.1, 0.15, -0.1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.1, 0.15, 0.15, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, 0.15, -0.15, 0.15, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.15, 0.15, -0.15, -0.1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.028, 0.074, 0.028, 0.074, boundaries = {})
        geometry.add_edge(0.028, 0.074, 0.028, 0.062, boundaries = {})
        geometry.add_edge(-0.028, 0.074, -0.028, 0.062, boundaries = {})
        geometry.add_edge(-0.028, 0.062, 0.028, 0.062, boundaries = {})
        geometry.add_edge(-0.028, 0.038, -0.028, 0.026, boundaries = {})
        geometry.add_edge(-0.028, 0.038, 0.028, 0.038, boundaries = {})
        geometry.add_edge(0.028, 0.038, 0.028, 0.026, boundaries = {})
        geometry.add_edge(0.028, 0.026, -0.028, 0.026, boundaries = {})
        geometry.add_edge(0.03, 0.038, 0.028, 0.04, boundaries = {}, angle = 90)
        geometry.add_edge(0.03, 0, 0.03, 0.038, boundaries = {})
        geometry.add_edge(-0.03, 0.038, -0.03, 0, boundaries = {})
        geometry.add_edge(-0.028, 0.04, -0.03, 0.038, boundaries = {}, angle = 90)
        
        # labels
        geometry.add_label(0.076268, 0.119048, materials = {"magnetic" : "Air"})
        geometry.add_label(-0.0126334, 0.0554136, materials = {"magnetic" : "Iron"}, area = 2e-05)
        geometry.add_label(0.0173943, -0.00630466, materials = {"magnetic" : "Iron"})
        geometry.add_label(0.00305095, 0.0688963, materials = {"magnetic" : "Civka 2"})
        geometry.add_label(-0.00501291, 0.0328188, materials = {"magnetic" : "Civka 1"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()        
        
    def test_values(self):            
        # point value
        point = self.magnetic.local_values(1.814e-02, -7.690e-03)
        self.value_test("Flux density", point["Br"], 1.819661)
        self.value_test("Permeability", point["mur"], 139.364773, 0.04)
        
        # volume integral
        volume = self.magnetic.volume_integrals([2])
        self.value_test("Energy", volume["Wm"], 3.264012)

class TestMagneticNonlinAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # magnetic
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 7
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['residual'] = 1e-06
        self.magnetic.solver_parameters['relative_change_of_solutions'] = 0.01
        self.magnetic.solver_parameters['damping'] = "automatic"
        self.magnetic.solver_parameters['damping_factor'] = 0.8
        self.magnetic.solver_parameters['damping_factor_increase_steps'] = 1
        self.magnetic.solver_parameters['damping_factor_decrease_ratio'] = 1.2
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_ratio'] = 0.8
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 20
        
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        # materials
        self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 2e+06, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0})
        self.magnetic.add_material("Fe", {"magnetic_permeability" : { "value" : 300, "x" : [0,0.170911,0.177249,0.20487,0.212115,0.253777,0.275513,0.282755,0.328941,0.405909,0.424924,0.771689,0.858155,1.2092,1.33438,1.3589,1.51368,1.60909,1.64417,1.66552,1.69157,1.73858,1.79572,1.82437,1.93984,1.9929,2.00005,2.01811,2.07161,2.08962,2.20335,2.30633,2.48983], "y" : [729.678,982.941,1008.92,1119.05,1146.74,1289.7,1357.53,1378.93,1508.03,1696.32,1739.63,2274.94,2330.44,2002.5,1706.76,1633.54,997.751,638.141,524.418,464.22,399.375,297.585,211.579,176.496,87.0871,62.8709,59.9063,53.3691,37.7024,33.2283,16.7621,10.3287,6.16704], "interpolation" : "piecewise_linear", "extrapolation" : "constant", "derivative_at_endpoints" : "first" }, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0})
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0.6, "magnetic_remanence_angle" : 90, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.01, 0.01, 0.01, 0)
        geometry.add_edge(0.01, 0, 0.04, 0)
        geometry.add_edge(0.04, 0.1, 0, 0.1)
        geometry.add_edge(0, 0.1, 0, 0.08, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.08, 0.01, 0.08)
        geometry.add_edge(0.01, 0.09, 0.03, 0.09)
        geometry.add_edge(0.01, 0.09, 0.01, 0.08)
        geometry.add_edge(0.01, 0.01, 0.03, 0.01)
        geometry.add_edge(0.012, 0.088, 0.012, 0.012)
        geometry.add_edge(0.012, 0.012, 0.028, 0.012)
        geometry.add_edge(0.028, 0.012, 0.028, 0.088)
        geometry.add_edge(0.028, 0.088, 0.012, 0.088)
        geometry.add_edge(0, 0.05, 0.009, 0.05)
        geometry.add_edge(0.009, 0.05, 0.009, -0.02)
        geometry.add_edge(0, 0.05, 0, -0.02, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.02, 0.009, -0.02)
        geometry.add_edge(0, 0.15, 0, 0.1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.08, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.02, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.05, 0.1, 0.05, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.1, 0.05, 0, 0.15, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.03, 0.09, 0.03, 0.078)
        geometry.add_edge(0.03, 0.078, 0.04, 0.078)
        geometry.add_edge(0.04, 0.078, 0.04, 0.1)
        geometry.add_edge(0.04, 0.078, 0.04, 0.052)
        geometry.add_edge(0.03, 0.078, 0.03, 0.052)
        geometry.add_edge(0.03, 0.052, 0.04, 0.052)
        geometry.add_edge(0.04, 0.052, 0.04, 0)
        geometry.add_edge(0.03, 0.01, 0.03, 0.052)
        
        geometry.add_label(0.0348743, 0.0347237, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.00512569, -0.0070852, area = 8e-07, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.021206, 0.0692964, materials = {"magnetic" : "Cu"})
        geometry.add_label(0.0141705, 0.12445, materials = {"magnetic" : "Air"})
        geometry.add_label(0.0346923, 0.0892198, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.036093, 0.0654078, materials = {"magnetic" : "Magnet"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):                  
        # point value
        point = self.magnetic.local_values(0.0043, 0.032)
        self.value_test("Scalar potential", point["Ar"], 8.060968068023004E-4)
        self.value_test("Flux density", point["Br"], 0.3823335718298335)
        self.value_test("Flux density - r", point["Brr"], 0.029885255691612152)
        self.value_test("Flux density - z", point["Brz"], 0.3811637855667586)
        self.value_test("Magnetic intensity", point["Hr"], 185.67238098160567)
        self.value_test("Magnetic intensity - r", point["Hrr"], 14.513155488294023)
        self.value_test("Magnetic intensity - z", point["Hrz"], 185.1042985102139)
        self.value_test("Energy density", point["wm"], 35.4943923762853)
        
        point = self.magnetic.local_values(0.0199415, 0.0609942)
        self.value_test("Lorentz force - r - real", point["Flr"], -123060.01919)
        self.value_test("Lorentz force - z - real", point["Flz"], -42838.956)
        
        # volume integral
        volume = self.magnetic.volume_integrals([1])
        self.value_test("Energy", volume["Wm"], 4.625935002717195E-4)
        
        volume = self.magnetic.volume_integrals([2])
        self.value_test("Integral Lorentz force - r", volume["Flx"], -8.101651950576503) 
        self.value_test("Integral Lorentz force - z", volume["Fly"], -5.501189057498289) 

        volume = self.magnetic.volume_integrals([1])        
        self.value_test("Volume Maxwell force - z", volume["Fty"], 0.4922392956664127, 0.1)
        
        # surface integral
        surface = self.magnetic.surface_integrals([12, 13, 14, 15])
        self.value_test("Surface Maxwell force - z", surface["Fty"], 0.4922392956664127, 0.1)
                                                                                                                                                                                                                                           
if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticNonlinPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticNonlinAxisymmetric)) 
    suite.run(result)
