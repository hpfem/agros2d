import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult
                                                                                                                                                                                                                                           
class TestMagneticHarmonicPlanar(Agros2DTestCase):
    def setUp(self):                                                                                                                         
        # model
        self.problem = agros2d.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"        
        self.problem.frequency = 50
        
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
        self.magnetic.add_material("Cond 2", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 3e7, "magnetic_conductivity" : 5.7e7}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : 20})    
        
        # geometry
        self.geometry = agros2d.geometry
        
        # edges
        self.geometry.add_edge(-0.075, 0.06, 0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.075, 0.06, 0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.075, -0.06, -0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.075, -0.06, -0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.015, -0.01, -0.015, 0.01)
        self.geometry.add_edge(-0.015, 0.01, -0.005, 0.01)
        self.geometry.add_edge(-0.015, -0.01, -0.005, -0.01)
        self.geometry.add_edge(-0.005, -0.01, -0.005, 0.01)
        self.geometry.add_edge(0.005, 0.02, 0.005, 0)
        self.geometry.add_edge(0.005, 0, 0.015, 0)
        self.geometry.add_edge(0.015, 0, 0.015, 0.02)
        self.geometry.add_edge(0.015, 0.02, 0.005, 0.02)
        self.geometry.add_edge(0.01, -0.01, 0.03, -0.01)
        self.geometry.add_edge(0.03, -0.03, 0.01, -0.03)
        self.geometry.add_edge(0.01, -0.01, 0.01, -0.03)
        self.geometry.add_edge(0.03, -0.01, 0.03, -0.03)
        
        # labels
        self.geometry.add_label(0.035349, 0.036683, materials = {"magnetic" : "Air"}, area=0)
        self.geometry.add_label(-0.0111161, -0.00311249, materials = {"magnetic" : "Cond 2"}, area=1e-05)
        self.geometry.add_label(0.016868, -0.0186625, materials = {"magnetic" : "Magnet"}, area=0)
        
        agros2d.view.zoom_best_fit()
            
                     
    # in this test, both Cond1 and Cond2 have conductivity 5.7e7
    # results from comsol test test_magnetic_harmonic_planar
    def general_nonzero_cond_test_values(self):                                                                                                                        
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
        volume = self.magnetic.volume_integrals([3])
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

    # in this test, Cond1 has conductivity 0
    # results from comsol test test_magnetic_harmonic_planar_zero_cond
    def general_zero_cond_test_values(self):                                                                                                                        
        point = self.magnetic.local_values(-0.0078561, 0.00453680)
        self.value_test("Current density - total - real", point["Jr"], -2.261e6)
        self.value_test("Current density - total - imag", point["Ji"], -2.02266e7)
        self.value_test("Current density - external - real", point["Jer"], 3e7)
        self.value_test("Current density - external - imag", point["Jei"], 0)
        
        point2 = self.magnetic.local_values(0.0073347240686417, 0.0036161467432976)
        self.value_test("Current density - total - real", point2["Jr"], 2e7)
        self.value_test("Current density - total - imag", point2["Ji"], 0)
        self.value_test("Current density - external - real", point2["Jer"], 2e7)
        self.value_test("Current density - external - imag", point2["Jei"], 0)
        
        volume = self.magnetic.volume_integrals([3])
        self.value_test("Current - real", volume["Ir"], 4000)
        self.value_test("Current - imag", volume["Ii"], 0)
        self.value_test("Current - external - real", volume["Ier"], 4000)
        self.value_test("Current - external - imag", volume["Iei"], 0)

        volume2 = self.magnetic.volume_integrals([1])
        self.value_test("Current - real", volume2["Ir"], -55.75)
        self.value_test("Current - imag", volume2["Ii"], -3601)
        self.value_test("Current - external - real", volume2["Ier"], 6000)
        self.value_test("Current - external - imag", volume2["Iei"], 0)

        
    # nonzero conductivity
    def test_values_nonzero_cond(self):
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e7, "magnetic_conductivity" : 5.7e7}) 
        self.geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
        self.problem.solve()
        self.general_nonzero_cond_test_values()
                          
    # zero conductivity and external current density given                                                        
    def test_values_zero_cond_J_ext_given(self):
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e7, "magnetic_conductivity" : 0}) 
        self.geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
        self.problem.solve()
        self.general_zero_cond_test_values()

    # zero conductivity and total current given 
    def test_values_zero_cond_I_given(self):
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_total_current_real" : 2e7 * 2e-4, "magnetic_total_current_prescribed" : 1, "magnetic_conductivity" : 0}) 
        self.geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
        self.problem.solve()
        self.general_zero_cond_test_values()

                                                                              
class TestMagneticHarmonicAxisymmetric(Agros2DTestCase):
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
             
class TestMagneticHarmonicNonlinPlanar(Agros2DTestCase):
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
        self.magnetic.solver_parameters['residual'] = 0.01
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
            
class TestMagneticHarmonicNonlinAxisymmetric(Agros2DTestCase):
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
        self.magnetic.solver_parameters['residual'] = 0.01
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
                      
class TestMagneticHarmonicPlanarTotalCurrent(Agros2DTestCase):
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
                                                     
class TestMagneticHarmonicAxisymmetricTotalCurrent(Agros2DTestCase):
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
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticHarmonicPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticHarmonicAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticHarmonicNonlinPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticHarmonicNonlinAxisymmetric))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticHarmonicPlanarTotalCurrent))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMagneticHarmonicAxisymmetricTotalCurrent))
    suite.run(result)
