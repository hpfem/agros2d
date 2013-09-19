import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class MagneticPlanarGeneral(Agros2DTestCase):
    def setUpGeneral(self, totalCurrent):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        problem.frequency = 100
        
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
        
        if totalCurrent:
            self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 1e6*1.225e-2})
        else:
            self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6})

        self.magnetic.add_material("Fe", {"magnetic_permeability" : 500}) 
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.11, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : -30}) 
        self.magnetic.add_material("Velocity", {"magnetic_permeability" : 1, "magnetic_conductivity" : 1e7, "magnetic_velocity_x" : 2})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(-0.5, 1, 0.5, 1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.5, -1, -0.5, -1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.2, 0.6, 0, 0.6)
        geometry.add_edge(0, 0.3, 0, 0.6)
        geometry.add_edge(-0.2, 0.3, 0, 0.3)
        geometry.add_edge(-0.2, 0.6, -0.2, 0.3)
        geometry.add_edge(-0.035, 0.135, 0.035, 0.135)
        geometry.add_edge(0.035, 0.135, 0.035, -0.04)
        geometry.add_edge(0.035, -0.04, -0.035, -0.04)
        geometry.add_edge(-0.035, 0.135, -0.035, -0.04)
        geometry.add_edge(0, -0.5, 0.3, -0.5)
        geometry.add_edge(0.3, -0.5, 0.3, -0.32)
        geometry.add_edge(0.3, -0.32, 0, -0.32)
        geometry.add_edge(0, -0.32, 0, -0.5)
        geometry.add_edge(-0.5, 1, -0.5, -0.15, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.5, -0.15, -0.5, -0.2, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.5, -0.2, -0.5, -1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.5, 1, 0.5, -0.15, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.5, -0.15, 0.5, -0.2, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.5, -0.2, 0.5, -1, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.5, -0.2, -0.5, -0.2)
        geometry.add_edge(0.5, -0.15, -0.5, -0.15)
        
        # labels
        geometry.add_label(-0.0959509, 0.445344, 0.001, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.00301448, 0.0404858, 0.005, materials = {"magnetic" : "Cu"})
        geometry.add_label(-0.145434, -0.706253, 0.01, materials = {"magnetic" : "Air"})
        geometry.add_label(0.143596, -0.364811, 0, materials = {"magnetic" : "Magnet"})
        geometry.add_label(0.310203, 0.631164, 0, materials = {"magnetic" : "Air"})
        geometry.add_label(-0.295858, -0.182894, 0, materials = {"magnetic" : "Velocity"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):                  
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
        volume = self.magnetic.volume_integrals([5])
        self.value_test("Energy", volume["Wm"], 3.088946)
        self.value_test("Losses", volume["Pj"], 220.022114)
        self.value_test("Lorentz force - x", volume["Flx"], -110.011057)
        self.value_test("Lorentz force - y", volume["Fly"], -36.62167)
        self.value_test("Torque", volume["Tl"], 20.463818)
        
        # surface integral
        surface = self.magnetic.surface_integrals([2, 3, 4, 5])
        self.value_test("Maxwell force - x", surface["Ftx"], 2.531945, 0.11)
        self.value_test("Maxwell force - y", surface["Fty"], -10.176192, 0.1)            

class MagneticPlanar(MagneticPlanarGeneral):
    def setUp(self):  
        self.setUpGeneral(False)
        
class MagneticPlanarTotalCurrent(MagneticPlanarGeneral):
    def setUp(self):  
        self.setUpGeneral(True)
                        
class MagneticAxisymmetricGeneral(Agros2DTestCase):
    def setUpGeneral(self, totalCurrent):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
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
        
        if totalCurrent:
            self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 2e6*1.216e-3 }) 
        else:
            self.magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e6}) 
        self.magnetic.add_material("Fe", {"magnetic_permeability" : 300}) 
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.6, "magnetic_remanence_angle" : 90}) 
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0.01, 0.01, 0.01, 0)
        geometry.add_edge(0.01, 0, 0.04, 0,)
        geometry.add_edge(0.04, 0.1, 0, 0.1,)
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
        geometry.add_edge(0, -0.05, 0.1, 0.05, boundaries = {"magnetic" : "A = 0"}, angle=90)
        geometry.add_edge(0.1, 0.05, 0, 0.15, boundaries = {"magnetic" : "A = 0"}, angle=90)
        geometry.add_edge(0.03, 0.09, 0.03, 0.078)
        geometry.add_edge(0.03, 0.078, 0.04, 0.078)
        geometry.add_edge(0.04, 0.078, 0.04, 0.1)
        geometry.add_edge(0.04, 0.078, 0.04, 0.052)
        geometry.add_edge(0.03, 0.078, 0.03, 0.052)
        geometry.add_edge(0.03, 0.052, 0.04, 0.052)
        geometry.add_edge(0.04, 0.052, 0.04, 0)
        geometry.add_edge(0.03, 0.01, 0.03, 0.052)
        
        # labels
        geometry.add_label(0.0348743, 0.0347237, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.00512569, -0.0070852, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.021206, 0.0692964, materials = {"magnetic" : "Cu"})
        geometry.add_label(0.0141705, 0.12445, materials = {"magnetic" : "Air"})
        geometry.add_label(0.0346923, 0.0892198, materials = {"magnetic" : "Fe"})
        geometry.add_label(0.036093, 0.0654078, materials = {"magnetic" : "Magnet"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):                  
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
        
        volume = self.magnetic.volume_integrals([2])
        self.value_test("Lorentz force - r", volume["Flx"], -8.069509) 
        self.value_test("Lorentz force - z", volume["Fly"], -5.288991) 
        
        # surface integral
        surface = self.magnetic.surface_integrals([12, 13, 14, 15])
        self.value_test("Maxwell force - z", surface["Fty"], 0.368232, 0.1)

class MagneticAxisymmetric(MagneticAxisymmetricGeneral):
    def setUp(self):  
        self.setUpGeneral(False)
        
class MagneticAxisymmetricTotalCurrent(MagneticAxisymmetricGeneral):
    def setUp(self):  
        self.setUpGeneral(True)        
    
class MagneticNonlinPlanar(Agros2DTestCase):
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
        self.magnetic.solver_parameters['tolerance'] = 0.1
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

class MagneticNonlinAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"        
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # magnetic
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.number_of_refinements = 2
        self.magnetic.polynomial_order = 2
        
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['tolerance'] = 0.01
        self.magnetic.solver_parameters['damping'] = 'automatic'
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 3
        
        # boundaries
        self.magnetic.add_boundary("A=0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        # materials
        self.magnetic.add_material("air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        self.magnetic.add_material("steel", {"magnetic_permeability" : { "value" : 1000, "x" : [0,0.454,1.1733,1.4147,1.7552,1.8595,1.9037,1.9418,2,5], "y" : [9300,9264,6717.2,4710.5,1664.8,763.14,453.7,194.13,100,1] }, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        self.magnetic.add_material("coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e6" }})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 0.15, 0, -0.15, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0, -0.15, 0.015, -0.15)
        geometry.add_edge(0.015, -0.15, 0.015, 0.15)
        geometry.add_edge(0, 0.15, 0.015, 0.15)
        geometry.add_edge(0.1, 0.15, 0.1, -0.15)
        geometry.add_edge(0.1, -0.15, 0.15, -0.15)
        geometry.add_edge(0.15, -0.15, 0.15, 0.15)
        geometry.add_edge(0.15, 0.15, 0.1, 0.15)
        geometry.add_edge(0, 0.5, 0, 0.15, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0, -0.5, 0, -0.15, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0, -0.5, 0.5, 0, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.5, 0, 0, 0.5, angle = 90, boundaries = {"magnetic" : "A=0"})
        
        geometry.add_label(0.235433, 0.273031, materials = {"magnetic" : "air"})
        geometry.add_label(0.00629508, 0.0397221, materials = {"magnetic" : "steel"})
        geometry.add_label(0.125663, 0.0344, materials = {"magnetic" : "coil"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
                
    def test_values(self):            
        # point value
        point = self.magnetic.local_values(0.005985, 0.043924)
        self.value_test("Scalar potential", point["Ar"], 0.00592236689936)
        self.value_test("Flux density", point["Br"], 1.97915224026)
        self.value_test("Flux density - r", point["Brr"], 0.0101765610227)
        self.value_test("Flux density - z", point["Brz"], 1.97912607677)
        self.value_test("Magnetic intensity", point["Hr"], 15189.5109368)
        self.value_test("Magnetic intensity - r", point["Hrr"], 78.1026248556)
        self.value_test("Magnetic intensity - z", point["Hrz"], 15189.3101384)
        self.value_test("Energy density", point["wm"], 15031.1772995)
        
        point = self.magnetic.local_values(1.165e-01, 3.387e-02)
        self.value_test("Lorentz force - r - real", point["Flr"], 24645.3658523)
        self.value_test("Lorentz force - z - real", point["Flz"], -4734.00737537)
        
        # volume integral
        volume = self.magnetic.volume_integrals([0, 1, 2])
        self.value_test("Energy", volume["Wm"], 1.948e+01)
        
        volume = self.magnetic.volume_integrals([2])
        self.value_test("Lorentz force - r", volume["Flx"], 151.589357375) # TODO: Flr
        self.value_test("Lorentz force - z", volume["Fly"], -0.00108431776502) # TODO: Flz
        
        # surface integral
        surface = self.magnetic.surface_integrals([1, 2, 3])
        self.value_test("Maxwell force - z", surface["Fty"], 0.368232)
                                                                                                                                                                                                                                           
class MagneticHarmonicPlanar(Agros2DTestCase):
    def setUp(self):                                                                                                                         
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"        
        problem.frequency = 50
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        self.magnetic.solver = "linear"
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e7, "magnetic_conductivity" : 5.7e7}) 
        self.magnetic.add_material("Cond 2", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 3e7, "magnetic_conductivity" : 5.7e7}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : 20})    
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(-0.075, 0.06, 0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.075, 0.06, 0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.075, -0.06, -0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.075, -0.06, -0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.015, -0.01, -0.015, 0.01)
        geometry.add_edge(-0.015, 0.01, -0.005, 0.01)
        geometry.add_edge(-0.015, -0.01, -0.005, -0.01)
        geometry.add_edge(-0.005, -0.01, -0.005, 0.01)
        geometry.add_edge(0.005, 0.02, 0.005, 0)
        geometry.add_edge(0.005, 0, 0.015, 0)
        geometry.add_edge(0.015, 0, 0.015, 0.02)
        geometry.add_edge(0.015, 0.02, 0.005, 0.02)
        geometry.add_edge(0.01, -0.01, 0.03, -0.01)
        geometry.add_edge(0.03, -0.03, 0.01, -0.03)
        geometry.add_edge(0.01, -0.01, 0.01, -0.03)
        geometry.add_edge(0.03, -0.01, 0.03, -0.03)
        
        # labels
        geometry.add_label(0.035349, 0.036683, materials = {"magnetic" : "Air"}, area=0)
        geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
        geometry.add_label(-0.0111161, -0.00311249, materials = {"magnetic" : "Cond 2"}, area=1e-05)
        geometry.add_label(0.016868, -0.0186625, materials = {"magnetic" : "Magnet"}, area=0)
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()        
                     
    def test_values(self):                                                                                                                        
        # point value
        point = self.magnetic.local_values(0.012448, 0.016473)
        self.value_test("Magnetic potential", point["A"], 0.001087)
        self.value_test("Magnetic potential - real", point["Ar"], 3.391642e-4)
        self.value_test("Magnetic potential - imag", point["Ai"], -0.001033)
        self.value_test("Flux density", point["B"], 0.038197)
        self.value_test("Flux density - x - real", point["Brx"], -0.004274)
        self.value_test("Flux density - x - imag", point["Bix"], 0.02868)
        self.value_test("Flux density - y - real", point["Bry"], 0.003269)
        self.value_test("Flux density - y - imag", point["Biy"], -0.024707)
        self.value_test("Magnetic field", point["H"], 30351.803874)
        self.value_test("Magnetic field - x - real", point["Hrx"], -3400.886351)
        self.value_test("Magnetic field - x - imag", point["Hix"], 22823.176772)
        self.value_test("Magnetic field - y - real", point["Hry"], 2613.37651)
        self.value_test("Magnetic field - y - imag", point["Hiy"], -19543.255504)
        self.value_test("Energy density", point["wm"], 289.413568)
        self.value_test("Losses density ", point["pj"], 3.435114e5)
        self.value_test("Current density - induced transform - real", point["Jitr"], -1.849337e7)
        self.value_test("Current density - induced transform - imag", point["Jiti"], -6.073744e6)
        self.value_test("Current density - total - real", point["Jr"], 1.50663e6)
        self.value_test("Current density - total - imag", point["Ji"], -6.073744e6)
        self.value_test("Lorentz force - x", point["Flx"], -77163)
        self.value_test("Lorentz force - y", point["Fly"], -89097)
        
        # volume integral
        volume = self.magnetic.volume_integrals([1])
        self.value_test("Current - external - real", volume["Ier"], 4000.0)
        self.value_test("Current - external - imag", volume["Iei"], 0.0)
        self.value_test("Current - induced transform - real", volume["Iitr"], -4104.701323)
        self.value_test("Current - induced transform - imag", volume["Iiti"], -1381.947299)
        self.value_test("Current - real", volume["Ir"], -104.701323)
        self.value_test("Current - imag", volume["Ii"], -1381.947299)
        self.value_test("Energy", volume["Wm"], 0.042927)
        self.value_test("Losses", volume["Pj"], 90.542962)
        self.value_test("Lorentz force - x", volume["Flx"], -11.228229)
        self.value_test("Lorentz force - y", volume["Fly"], -4.995809)   
             
class MagneticHarmonicAxisymmetric(Agros2DTestCase):
    def setUp(self):                                                                                                                         
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"        
        problem.frequency = 100
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        self.magnetic.solver = "linear"
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6}) 
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 50, "magnetic_conductivity" : 5e3}) 
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, -0.17, 0.15, -0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.17, 0.15, 0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, 0.17, 0, 0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.035, -0.03, 0.055, -0.03)
        geometry.add_edge(0.055, -0.03, 0.055, 0.11)
        geometry.add_edge(0.055, 0.11, 0.035, 0.11)
        geometry.add_edge(0.035, 0.11, 0.035, -0.03)
        geometry.add_edge(0, -0.05, 0.03, -0.05)
        geometry.add_edge(0.03, -0.05, 0.03, 0.05)
        geometry.add_edge(0.03, 0.05, 0, 0.05)
        geometry.add_edge(0, 0.05, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.17, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.05, 0, -0.17, boundaries = {"magnetic" : "A = 0"})
        
        # labels
        geometry.add_label(0.109339, 0.112786, materials = {"magnetic" : "Air"}, area=0)
        geometry.add_label(0.0442134, 0.0328588, materials = {"magnetic" : "Coil"}, area=0)
        geometry.add_label(0.0116506, -0.00740064, materials = {"magnetic" : "Iron"}, area=0)
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
                     
    def test_values(self):
        # point value
        point = self.magnetic.local_values(0.027159, 0.039398)
        self.value_test("Magnetic potential", point["A"], 0.001087)
        self.value_test("Magnetic potential - real", point["Ar"], 0.001107)
        self.value_test("Magnetic potential - imag", point["Ai"], -5.24264e-6)
        self.value_test("Flux density", point["B"], 0.099325)
        self.value_test("Flux density - r - real", point["Brr"], 0.027587)
        self.value_test("Flux density - r - imag", point["Bir"], -2.430976e-4)
        self.value_test("Flux density - z - real", point["Brz"],  0.095414)
        self.value_test("Flux density - z - imag", point["Biz"], 7.424088e-4)
        self.value_test("Magnetic field", point["H"], 1580.808517)
        self.value_test("Magnetic field - r - real", point["Hrr"], 439.052884)
        self.value_test("Magnetic field - r - imag", point["Hir"], -3.869019)
        self.value_test("Magnetic field - z - real", point["Hrz"], 1518.562988)
        self.value_test("Magnetic field - z - imag", point["Hiz"], 11.815803)
        self.value_test("Energy density", point["wm"], 39.253502)
        self.value_test("Losses density ", point["pj"], 1210.138583)
        self.value_test("Current density - induced transform - real", point["Jitr"], -16.47024)
        self.value_test("Current density - induced transform - imag", point["Jiti"], -3478.665629)
        self.value_test("Current density - total - real", point["Jr"], -16.47024)
        self.value_test("Current density - total - imag", point["Ji"], -3478.665629)
        self.value_test("Lorentz force - r", point["Flr"], 0.505549)
        self.value_test("Lorentz force - z", point["Flz"], 0.650006)
        
        # volume integral
        volume = self.magnetic.volume_integrals([2])
        self.value_test("Current - induced transform - real", volume["Iitr"], -0.067164)
        self.value_test("Current - induced transform - imag", volume["Iiti"], -5.723787)
        self.value_test("Current - external - real", volume["Ier"], 0.0)
        self.value_test("Current - external - imag", volume["Iei"], 0.0)
        self.value_test("Current - real", volume["Ir"], -0.067164)
        self.value_test("Current - imag", volume["Ii"], -5.723787)
        self.value_test("Energy", volume["Wm"], 0.009187)
        self.value_test("Losses", volume["Pj"], 0.228758)
        self.value_test("Lorentz force - r", volume["Flx"], -4.018686e-4)
        self.value_test("Lorentz force - z", volume["Fly"], -1.233904e-5)    
             
class MagneticHarmonicNonlinPlanar(Agros2DTestCase):
    def setUp(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.frequency = 50
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['tolerance'] = 0.01
        self.magnetic.solver_parameters['damping_factor'] = 0.7
        self.magnetic.solver_parameters['damping'] = 'automatic'
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_ratio'] = 0.9
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 20        

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_imag" : 0, "magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Zelezo", {"magnetic_conductivity" : 5e6, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 3e6, "magnetic_permeability" : { "value" : 5000, 
            "x" : [0,0.227065,0.45413,0.681195,0.90826,1.13533,1.36239,1.58935,1.81236,2.01004,2.13316,2.19999,2.25479,2.29993,2.34251,2.37876,2.41501,2.45126,2.4875,2.52375,2.56,3,5,10,20], 
            "y" : [13001,13001,13001,12786,12168,10967,7494,1409,315,90,41,26,19,15,12,11,9,8,8,7,6,4,3,3,2] 
            }, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, 0.0035, -0.0035, 0, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        geometry.add_edge(-0.0035, 0, 0, -0.0035, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        geometry.add_edge(0, -0.0035, 0.0035, 0, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        geometry.add_edge(0.0035, 0, 0, 0.0035, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        
        # labels
        geometry.add_label(0, 0, materials = {"magnetic" : "Zelezo"}, area = 5e-07)
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):
        # point value
        point = self.magnetic.local_values(-2e-3, 4e-4)
        self.value_test("Flux density", point["B"], 1.478466609831)
        self.value_test("Permeability", point["mur"], 4381.88257, 5)
        self.value_test("Current density - total - real", point["Jr"], -141396.38032153525)
        self.value_test("Current density - total - imag", point["Ji"], -931848.5966661869)
        
        # volume integral
        volume = self.magnetic.volume_integrals()
        self.value_test("Energy", volume["Wm"], 0.012166845506925431)
        self.value_test("Current density - induced - real", volume["Iivr"], -76.31308924012728)
        self.value_test("Current density - induced - imag", volume["Iiti"], -25.458979006398277)
        self.value_test("Energy", volume["Wm"], 0.012166845506925431)
            

class MagneticHarmonicNonlinAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 50
        
        # magnetic
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 3
        self.magnetic.polynomial_order = 3
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['tolerance'] = 0.01
        self.magnetic.solver_parameters['measurement'] = "residual_norm_absolute"
        self.magnetic.solver_parameters['damping'] = "automatic"
        self.magnetic.solver_parameters['damping_factor'] = 0.8
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_ratio'] = 0.8
        self.magnetic.solver_parameters['damping_factor_decrease_ratio'] = 1.2
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 20
        self.magnetic.solver_parameters['damping_factor_increase_steps'] = 1
                
        # boundaries
        self.magnetic.add_boundary("A=0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})
                
        # materials
        self.magnetic.add_material("new material", {"magnetic_permeability" : { "value" : 9300, 
            "x" : [0,0.227065,0.45413,0.681195,0.90826,1.13533,1.36239,1.58935,1.81236,2.01004,2.13316,2.19999,2.25479,2.29993,2.34251,2.37876,2.41501,2.45126,2.4875,2.52375,2.56,3,5,10,20], 
            "y" : [13001,13001,13001,12786,12168,10967,7494,1409,315,90,41,26,19,15,12,11,9,8,8,7,6,4,3,3,2], 
            "interpolation" : "piecewise_linear", "extrapolation" : "constant", "derivative_at_endpoints" : "first" }, 
            "magnetic_conductivity" : 5e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, 
            "magnetic_current_density_external_real" : 1e6, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0.004, 0, 0.007, -0.003, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.007, -0.003, 0.01, 0, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.01, 0, 0.007, 0.003, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.007, 0.003, 0.004, 0, angle = 90, boundaries = {"magnetic" : "A=0"})
        
        geometry.add_label(0.0069576, -0.000136791, materials = {"magnetic" : "new material"})
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
                                              
    def test_values(self):
        # point value
        point = self.magnetic.local_values(0.0051, -0.0003)
#    current density 1.5e6 (calculations takes 3 minutes)
#        self.value_test("Flux density", point["B"], 0.60932, 8)
#        self.value_test("Permeability", point["mur"], 12854.05251)
#        self.value_test("Current density - total - real", point["Jr"], -1.41843e5)
#        self.value_test("Current density - total - imag", point["Ji"], -1.29996e5)
        
        self.value_test("Flux density", point["B"], 0.10239)
        self.value_test("Permeability", point["mur"], 13001)
        self.value_test("Current density - total - real", point["Jr"], -49436.7)
        self.value_test("Current density - total - imag", point["Ji"], 4043.9)
        point2 = self.magnetic.local_values(0.0043, -2e-4)
        self.value_test("Flux density", point2["B"], 1.3649)
        self.value_test("Permeability", point2["mur"], 7425.7)
        self.value_test("Current density - total - real", point2["Jr"], 4.846e5)
        self.value_test("Current density - total - imag", point2["Ji"], -3.569e5)
        
        # volume integral
        volume = self.magnetic.volume_integrals()        
#    current density 1.5e6 (calculations takes 3 minutes)
        #self.value_test("Energy", volume["Wm"], 0.012166845506925431)
#        self.value_test("Current density - induced - real", volume["Iitr"], -32.95049)
#        self.value_test("Current density - induced - imag", volume["Iiti"], -6.50704)
        self.value_test("Current density - induced - real", volume["Iitr"], -23.807)
        self.value_test("Current density - induced - imag", volume["Iiti"], -3.323)
            
                                             
class MagneticTransientPlanar(Agros2DTestCase):
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
                
class MagneticTransientAxisymmetric(Agros2DTestCase):
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

class MagneticHarmonicPlanarTotalCurrent(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.frequency = 50
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        self.magnetic.solver = "linear"        
        
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})        
        
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : { "expression" : "1e4" }, "magnetic_total_current_imag" : { "expression" : "1e4" }})
        self.magnetic.add_material("Cond 2", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 3e+07, "magnetic_current_density_external_imag" : 1e7, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : 20, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Cond3", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(-0.075, 0.06, 0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.075, 0.06, 0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.075, -0.06, -0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.075, -0.06, -0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(-0.015, -0.01, -0.015, 0.01)
        geometry.add_edge(-0.015, 0.01, -0.005, 0.01)
        geometry.add_edge(-0.015, -0.01, -0.005, -0.01)
        geometry.add_edge(-0.005, -0.01, -0.005, 0.01)
        geometry.add_edge(0.005, 0.02, 0.005, 0)
        geometry.add_edge(0.005, 0, 0.015, 0)
        geometry.add_edge(0.015, 0, 0.015, 0.02)
        geometry.add_edge(0.015, 0.02, 0.005, 0.02)
        geometry.add_edge(0.01, -0.01, 0.03, -0.01)
        geometry.add_edge(0.03, -0.03, 0.01, -0.03)
        geometry.add_edge(0.01, -0.01, 0.01, -0.03)
        geometry.add_edge(0.03, -0.01, 0.03, -0.03)
        geometry.add_edge(-0.01, 0.02, 0, 0.03, angle = 90)
        geometry.add_edge(-0.02, 0.03, -0.01, 0.02, angle = 90)
        geometry.add_edge(-0.01, 0.04, -0.02, 0.03, angle = 90)
        geometry.add_edge(0, 0.03, -0.01, 0.04, angle = 90)
        
        geometry.add_label(0.035349, 0.036683, materials = {"magnetic" : "Air"})
        geometry.add_label(0.00778124, 0.00444642, area = 1e-05, materials = {"magnetic" : "Cond 1"})
        geometry.add_label(-0.0111161, -0.00311249, area = 1e-05, materials = {"magnetic" : "Cond 2"})
        geometry.add_label(0.016868, -0.0186625, materials = {"magnetic" : "Magnet"})
        geometry.add_label(-0.00707714, 0.0289278, materials = {"magnetic" : "Cond3"})
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):        
        # in this test, total current is prescribed on several domains
        # mainly current densities are tested
        
        # point value
        point1 = self.magnetic.local_values(0.0116, 0.0132)
        point2 = self.magnetic.local_values(-0.0123, 0.00536)
        point3 = self.magnetic.local_values(-0.0042, 0.03236)
        
        # volume integral
        volume1 = self.magnetic.volume_integrals([1])
        volume2 = self.magnetic.volume_integrals([2])
        volume3 = self.magnetic.volume_integrals([4])
        
        #Area 1
        self.value_test("Magnetic potential - 1", point1["A"], 0.0062725)
        self.value_test("Magnetic potential - 1 - real", point1["Ar"], 0.00543)
        self.value_test("Magnetic potential - 1 - imag", point1["Ai"], 0.00314)
        self.value_test("Flux density - 1", point1["B"], 0.10789)
        self.value_test("Magnetic field - 1", point1["H"], 85856.68)
        self.value_test("Magnetic field - 1 - x - real", point1["Hrx"], -61409.68265)
        self.value_test("Magnetic field - 1 - x - imag", point1["Hix"], -13675.94624)
        self.value_test("Magnetic field - 1 - y - real", point1["Hry"], 57956.23371)
        self.value_test("Magnetic field - 1 - y - imag", point1["Hiy"], 7364.8575)
        self.value_test("Energy density - 1", point1["wm"], 2315.787)
        self.value_test("Losses density - 1 ", point1["pj"], 4.5933e7) # COMSOL: resistive 3.9578e7, COMSOL: from Jz: 4.5933e7
        
        #todo: ujasnit si, co ma byt u harmonickeho pole Lorentzova sila
        #to, co my pocitame se v comsolu shoduje s realnou slozkou veliciny "Lorentz force contribution, time average"
        #proc je v comsolu time average komplexni cislo???
        
        #ztraty v comsolu jsou i zaporne, co to znamena?
        
        self.value_test("Lorentz force point - 1 - x", point1["Flx"], -2.30979e6)
        self.value_test("Lorentz force point - 1 - y", point1["Fly"], -2.60749e6)
        
        self.value_test("Flux density - 1 - x - real", point1["Brx"], -0.07714)
        self.value_test("Flux density - 1 - x - imag", point1["Bix"], -0.01711)
        self.value_test("Flux density - 1 - y - real", point1["Bry"], 0.07273)
        self.value_test("Flux density - 1 - y - imag", point1["Biy"], 0.009309)
        
        self.value_test("Current density - 1 - induced transform - real", point1["Jitr"], 5.62458e7)
        self.value_test("Current density - 1 - induced transform - imag", point1["Jiti"], -9.72075e7)
        self.value_test("Current density - 1 - external - real", point1["Jer"], 1.67159e6)
        self.value_test("Current density - 1 - external - imag", point1["Jei"], 1.40589e8)
        self.value_test("Current density - 1 - total - real", point1["Jr"], 5.79174e7)
        self.value_test("Current density - 1 - total - imag", point1["Ji"], 4.3382e7)
        
        self.value_test("Current - 1 - external - real", volume1["Ier"], 334.22742)
        self.value_test("Current - 1 - external - imag", volume1["Iei"], 28117.91)
        self.value_test("Current - 1 - induced transform - real", volume1["Iitr"], 9665.77)
        self.value_test("Current - 1 - induced transform - imag", volume1["Iiti"], -18117.91)
        self.value_test("Current - 1 - real", volume1["Ir"], 1e4)
        self.value_test("Current - 1 - imag", volume1["Ii"], 1e4)
        
        self.value_test("Energy - 1", volume1["Wm"], 2.24807)
        self.value_test("Losses - 1", volume1["Pj"], 8906.2906) # COMSOL: resistive 8029.097, COMSOL: from Jz: 8906.2906
        #self.value_test("Lorentz force integral - 1 - x", volume1["Flx"], 265.14584)
        #self.value_test("Lorentz force integral - 1 - y", volume1["Fly"], 8.32792)
           
        #Area 2
        self.value_test("Flux density - 2 - x - real", point2["Brx"], 0.01789)
        self.value_test("Flux density - 2 - x - imag", point2["Bix"], 0.08029)
        self.value_test("Flux density - 2 - y - real", point2["Bry"], -0.09474)
        self.value_test("Flux density - 2 - y - imag", point2["Biy"], 0.01871)
        
        self.value_test("Current density - 2 - induced transform - real", point2["Jitr"], -2.86474e7)
        self.value_test("Current density - 2 - induced transform - imag", point2["Jiti"], -4.83838e7)
        self.value_test("Current density - 2 - total - real", point2["Jr"], 1.35258e6)
        self.value_test("Current density - 2 - total - imag", point2["Ji"], -3.83838e7)
        
        self.value_test("Current - 2 - external - real", volume2["Ier"], 6000)
        self.value_test("Current - 2 - external - imag", volume2["Iei"], 2000)
        self.value_test("Current - 2 - induced transform - real", volume2["Iitr"], -5176.3)
        self.value_test("Current - 2 - induced transform - imag", volume2["Iiti"], -9797.11)
        self.value_test("Current - 2 - real", volume2["Ir"], 823.68)
        self.value_test("Current - 2 - imag", volume2["Ii"], -7797.11)
        
        #Area 3
        self.value_test("Flux density - 3 - x - real", point3["Brx"], -0.07824)
        self.value_test("Flux density - 3 - x - imag", point3["Bix"], 0.01079)
        self.value_test("Flux density - 3 - y - real", point3["Bry"], -0.04435)
        self.value_test("Flux density - 3 - y - imag", point3["Biy"], -0.02718)
        
        self.value_test("Current density - 3 - induced transform - real", point3["Jitr"], 9.12211e6)
        self.value_test("Current density - 3 - induced transform - imag", point3["Jiti"], -3.67499e7)
        self.value_test("Current density - 3 - external - real", point3["Jer"], -6.19451e6)
        self.value_test("Current density - 3 - external - imag", point3["Jei"], 3.39449e7)
        self.value_test("Current density - 3 - total - real", point3["Jr"], 2.9276e6)
        self.value_test("Current density - 3 - total - imag", point3["Ji"], -2.80499e6)
        
        self.value_test("Current - 3 - external - real", volume3["Ier"], -1946.07)
        self.value_test("Current - 3 - external - imag", volume3["Iei"], 10664.068)
        self.value_test("Current - 3 - induced transform - real", volume3["Iitr"], 1946.07)
        self.value_test("Current - 3 - induced transform - imag", volume3["Iiti"], -10664.068)
        self.value_test("Current - 3 - real", volume3["Ir"], 0, 100)
        self.value_test("Current - 3 - imag", volume3["Ii"], 0, 100)

                                                     
class MagneticHarmonicAxisymmetricTotalCurrent(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 100
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 2
        self.magnetic.polynomial_order = 3
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"
                
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})        
        
        # materials
        self.magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 1e+06, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 50, "magnetic_conductivity" : 5000, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
        self.magnetic.add_material("Coil2", {"magnetic_permeability" : 1, "magnetic_conductivity" : 1e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 5000, "magnetic_total_current_imag" : 20000})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, -0.17, 0.15, -0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.17, 0.15, 0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, 0.17, 0, 0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.035, -0.03, 0.055, -0.03)
        geometry.add_edge(0.055, -0.03, 0.055, 0.11)
        geometry.add_edge(0.055, 0.11, 0.035, 0.11)
        geometry.add_edge(0.035, 0.11, 0.035, -0.03)
        geometry.add_edge(0, -0.05, 0.03, -0.05)
        geometry.add_edge(0.03, -0.05, 0.03, 0.05)
        geometry.add_edge(0.03, 0.05, 0, 0.05)
        geometry.add_edge(0, 0.05, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.17, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.05, 0, -0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.06, -0.05, 0.04, -0.07, angle = 90)
        geometry.add_edge(0.04, -0.07, 0.06, -0.09, angle = 90)
        geometry.add_edge(0.06, -0.09, 0.08, -0.07, angle = 90)
        geometry.add_edge(0.08, -0.07, 0.06, -0.05, angle = 90)
        
        geometry.add_label(0.109339, 0.112786, materials = {"magnetic" : "Air"})
        geometry.add_label(0.0442134, 0.0328588, materials = {"magnetic" : "Coil"})
        geometry.add_label(0.0116506, -0.00740064, materials = {"magnetic" : "Iron"})
        geometry.add_label(0.0612934, -0.075856, materials = {"magnetic" : "Coil2"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):        
        point = self.magnetic.local_values(0.05, -0.07)
        self.value_test("Magnetic potential", point["A"], 0.00727)
        self.value_test("Magnetic potential - real", point["Ar"], 0.00194)
        self.value_test("Magnetic potential - imag", point["Ai"], 0.00701)
        self.value_test("Flux density", point["B"], 0.19266)
        self.value_test("Flux density - r - real", point["Brr"], -0.01387)
        self.value_test("Flux density - r - imag", point["Bir"], -0.0242)
        self.value_test("Flux density - z - real", point["Brz"],  0.05584)
        self.value_test("Flux density - z - imag", point["Biz"],  0.18156)
        
        point2 = self.magnetic.local_values(0.02, -0.04)
        self.value_test("Magnetic field", point2["H"], 7049.46496)
        self.value_test("Magnetic field - r - real", point2["Hrr"], -587.5199)
        self.value_test("Magnetic field - r - imag", point2["Hir"], -634.21712)
        self.value_test("Magnetic field - z - real", point2["Hrz"], 2484.18713)
        self.value_test("Magnetic field - z - imag", point2["Hiz"], 6540.36386)
        
        self.value_test("Energy density", point["wm"], 7384.29938)
        self.value_test("Losses density ", point["pj"], 1.83518e8)
        self.value_test("Current density - induced transform - real", point["Jitr"], 4.40517e6)
        self.value_test("Current density - induced transform - imag", point["Jiti"], -1.21945e6)
        self.value_test("Current density - external - real", point["Jer"], 1.96967e5)
        self.value_test("Current density - external - imag", point["Jei"], 1.98167e7)
        self.value_test("Current density - total - real", point["Jr"], 4.60213e6)
        self.value_test("Current density - total - imag", point["Ji"], 1.85972e7)
        #self.value_test("Lorentz force - r", point["Flr"], 2.5714e5)
        #self.value_test("Lorentz force - z", point["Flz"], 64170.81137)
        
        # volume integral
        volume = self.magnetic.volume_integrals([3])
        self.value_test("Current - induced transform - real", volume["Iitr"], 4787.66447)
        self.value_test("Current - induced transform - imag", volume["Iiti"], -1362.82611)
        self.value_test("Current - external - real", volume["Ier"], 212.33553)
        self.value_test("Current - external - imag", volume["Iei"], 21362.82611)
        self.value_test("Current - real", volume["Ir"], 5000)
        self.value_test("Current - imag", volume["Ii"], 20000)
        self.value_test("Energy", volume["Wm"], 2.22728039) 
        self.value_test("Losses", volume["Pj"], 61996.842275)
        #self.value_test("Lorentz force - r", volume["Flx"], -4.018686e-4)
        #self.value_test("Lorentz force - z", volume["Fly"], -1.233904e-5)

if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticPlanarTotalCurrent))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticAxisymmetricTotalCurrent))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticNonlinPlanar))
    #suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticNonlinAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticHarmonicPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticHarmonicAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticHarmonicNonlinPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticHarmonicNonlinAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticHarmonicPlanarTotalCurrent))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticHarmonicAxisymmetricTotalCurrent))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticTransientPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(MagneticTransientAxisymmetric))
    suite.run(result)
