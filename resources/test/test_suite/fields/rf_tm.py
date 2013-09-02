import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult
        
class RFTMHarmonicPlanar(Agros2DTestCase):
    def setUp(self): 
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.frequency = 1e10
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # rf_tm
        self.rf_tm = agros2d.field("rf_tm")
        self.rf_tm.analysis_type = "harmonic"
        self.rf_tm.number_of_refinements = 4
        self.rf_tm.polynomial_order = 3
        self.rf_tm.adaptivity_type = "disabled"
        self.rf_tm.solver = "linear"
        
        # boundaries
        self.rf_tm.add_boundary("Perfect electric conductor", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})
        self.rf_tm.add_boundary("Matched boundary", "rf_tm_impedance", {"rf_tm_impedance" : 377})
        self.rf_tm.add_boundary("Surface current", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 1, "rf_tm_magnetic_field_imag" : 1})
        
        # materials
        self.rf_tm.add_material("Air", {"rf_tm_permittivity" : 1, "rf_tm_permeability" : 1, "rf_tm_conductivity" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(-0.01, 0.02286, -0.01, 0, boundaries = {"rf_tm" : "Surface current"})
        geometry.add_edge(0.06907, 0.02286, 0.076, 0.01593, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.076, 0.01593, 0.081, 0.01593, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.081, 0.01593, 0.081, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.086, 0.00693, 0.086, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.086, 0.00693, 0.081, 0.00693, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.081, 0.00693, 0.05907, 0, angle = 20, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(-0.01, 0, 0.05907, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.17, 0.02286, 0.081, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.06907, 0.02286, -0.01, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.19286, 0, 0.17, 0.02286, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.19286, 0, 0.19286, -0.04, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.175, -0.04, 0.175, -0.015, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.175, -0.015, 0.16, 0, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
        geometry.add_edge(0.175, -0.04, 0.19286, -0.04, boundaries = {"rf_tm" : "Matched boundary"})
        geometry.add_edge(0.086, 0, 0.16, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
        
        geometry.add_label(0.0359418, 0.0109393, materials = {"rf_tm" : "Air"})
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self): 
        # point value
        point = self.rf_tm.local_values(0.019107, 0.016725)

        self.value_test("Magnetic field", point["H"], 3.23)
        self.value_test("Magnetic field - real", point["Hr"], 2.28)
        self.value_test("Magnetic field - imag", point["Hi"], 2.29)
        
        self.value_test("Magnetic flux density", point["B"], 4.061379e-6)
        self.value_test("Magnetic flux density - real", point["Br"], 2.868038e-6)
        self.value_test("Magnetic flux density - imag", point["Bi"], 2.875614e-6)
        
        self.value_test("Electric field", point["E"], 853.416645103)
        self.value_test("Electric field - x - real", point["Erx"], -503.757373)
        self.value_test("Electric field - x - imag", point["Eix"],  502.430138)
        self.value_test("Electric field - y - real", point["Ery"], -341.713906)
        self.value_test("Electric field - y - imag", point["Eiy"], 341.713906)
        
        self.value_test("Electric displacement", point["D"], 7.615992e-9)
        self.value_test("Electric displacement - x - real", point["Drx"],-4.460362e-9)
        self.value_test("Electric displacement - x - imag", point["Dix"],4.448611e-9)
        self.value_test("Electric displacement - y - real", point["Dry"], 3.025599e-9)
        self.value_test("Electric displacement - y - imag", point["Diy"], -3.027227e-9)

        self.value_test("Poynting vector - x", point["Nx"], 0.820297)
        point = self.rf_tm.local_values(1.841e-01, -3.055e-02)
        self.value_test("Poynting vector - y", point["Ny"], -1.880639)

class RFTMHarmonicAxisymmetric(Agros2DTestCase):
    def setUp(self): 
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        problem.frequency = 1e+09
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # rf_tm
        self.rf_tm = agros2d.field("rf_tm")
        self.rf_tm.analysis_type = "harmonic"
        self.rf_tm.number_of_refinements = 4
        self.rf_tm.polynomial_order = 3
        self.rf_tm.adaptivity_type = "disabled"
        self.rf_tm.solver = "linear"        
        
        # boundaries
        self.rf_tm.add_boundary("PEC", "rf_tm_electric_field", {"rf_tm_electric_field_real" : 0, "rf_tm_electric_field_imag" : 0})
        self.rf_tm.add_boundary("Source", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 1, "rf_tm_magnetic_field_imag" : 0})
        self.rf_tm.add_boundary("Impedance", "rf_tm_impedance", {"rf_tm_impedance" : 377})
        self.rf_tm.add_boundary("PMC", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})        
        
        # materials
        self.rf_tm.add_material("Air", {"rf_tm_permittivity" : 1, "rf_tm_permeability" : 1, "rf_tm_conductivity" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 1, 0, -1, boundaries = {"rf_tm" : "PMC"})
        geometry.add_edge(0, -1, 1, -1, boundaries = {"rf_tm" : "Impedance"})
        geometry.add_edge(1, -1, 1, -0.75, boundaries = {"rf_tm" : "Impedance"})
        geometry.add_edge(1, -0.75, 1.7, 0.65, boundaries = {"rf_tm" : "Impedance"})
        geometry.add_edge(1.7, 0.65, 0.9, 0.35, boundaries = {"rf_tm" : "Impedance"})
        geometry.add_edge(0.9, 0.35, 0, 1, boundaries = {"rf_tm" : "Impedance"})
        geometry.add_edge(0.5, -0.25, 0.25, -0.5, angle = 90, boundaries = {"rf_tm" : "Source"})
        geometry.add_edge(0.25, -0.5, 0.5, -0.75, angle = 90, boundaries = {"rf_tm" : "Source"})
        geometry.add_edge(0.5, -0.75, 0.75, -0.5, angle = 90, boundaries = {"rf_tm" : "Source"})
        geometry.add_edge(0.75, -0.5, 0.5, -0.25, angle = 90, boundaries = {"rf_tm" : "Source"})
        geometry.add_edge(0.4, 0.25, 0.3, -0.05, boundaries = {"rf_tm" : "PEC"})
        geometry.add_edge(0.4, 0.25, 0.75, 0.2, boundaries = {"rf_tm" : "PEC"})
        geometry.add_edge(0.75, 0.2, 0.85, -0.1, boundaries = {"rf_tm" : "PEC"})
        geometry.add_edge(0.3, -0.05, 0.85, -0.1, boundaries = {"rf_tm" : "PEC"})
        
        geometry.add_label(0.399371, 0.440347, materials = {"rf_tm" : "Air"})
        geometry.add_label(0.484795, -0.434246, materials = {"rf_tm" : "none"})
        geometry.add_label(0.57193, 0.0710058, materials = {"rf_tm" : "none"})
        agros2d.view.zoom_best_fit()
        
        problem.solve()
        
    def test_values(self): 
        # point value        
        point = self.rf_tm.local_values(0.92463, -0.20118)
        
        self.value_test("Magnetic field", point["H"], 0.53836)
        self.value_test("Magnetic field - real", point["Hr"], 0.538097)
        self.value_test("Magnetic field - imag", point["Hi"], 0.016823)
        
        self.value_test("Magnetic flux density", point["B"], 6.765228e-7)
        self.value_test("Magnetic flux density - real", point["Br"], 6.761924e-7)
        self.value_test("Magnetic flux density - imag", point["Bi"], 2.114032e-8)
        
        self.value_test("Electric field", point["E"], 274.023386)
        self.value_test("Electric field r component - real", point["Err"], 122.830044)
        self.value_test("Electric field r component - imag", point["Eir"], 221.565147)
        self.value_test("Electric field z component - real", point["Erz"], -103.385085)
        self.value_test("Electric field z component - imag", point["Eiz"], -14.899843)
        
        self.value_test("Electric displacement", point["D"], 2.4447e-9)
        self.value_test("Electric displacement r component - real", point["Drr"], 1.08756e-9)
        self.value_test("Electric displacement r component - imag", point["Dir"], 1.961779e-9)
        self.value_test("Electric displacement z component - real", point["Drz"],-9.15391e-10)
        self.value_test("Electric displacement z component - imag", point["Diz"], -1.31926e-10)
        
        self.value_test("Poynting vector r component", point["Nr"], 27.940976)
        self.value_test("Poynting vector z component", point["Nz"], 34.895617)    

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(RFTMHarmonicPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(RFTMHarmonicAxisymmetric))
    suite.run(result)
    