import agros2d

class CurrentPlanar(agros2d.Agros2DTestCase):
    @classmethod
    def setUpClass(self):  
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.current = agros2d.field("current")
        self.current.analysis_type = "steadystate"
        self.current.number_of_refinements = 3
        self.current.polynomial_order = 5
        self.current.solver = "linear"
        
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        self.current.add_boundary("Zero", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("Voltage", "current_potential", {"current_potential" : 1})
        
        self.current.add_material("mat 1", {"current_conductivity" : 1e7})
        self.current.add_material("mat 2", {"current_conductivity" : 1e5})
        self.current.add_material("mat 3", {"current_conductivity" : 1e3})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, 0, 0.6, 0, boundaries = {"current" : "Zero"})
        geometry.add_edge(0, 0.8, 0, 0.5, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0.5, 0, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0, 0.35, 0.5,)
        geometry.add_edge(0.35, 0.5, 0.6, 0.5,)
        geometry.add_edge(0.6, 0.8, 0.6, 0.5, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.6, 0.5, 0.6, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0.5, 0.35, 0.5,)
        geometry.add_edge(0, 0.8, 0.6, 0.8, boundaries = {"current" : "Voltage"})
        
        # labels
        geometry.add_label(0.3, 0.670924, materials = {"current" : "mat 1"})
        geometry.add_label(0.105779, 0.364111, materials = {"current" : "mat 2"})
        geometry.add_label(0.394296, 0.203668, materials = {"current" : "mat 3"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):
        # point value
        point = self.current.local_values(0.11879, 0.346203)
        self.value_test("Scalar potential", point["V"], 0.928377)
        self.value_test("Electric field", point["Er"], 0.486928)
        self.value_test("Electric field - x", point["Erx"], -0.123527)
        self.value_test("Electric field - y", point["Ery"], -0.470999)
        self.value_test("Current density", point["Jrc"], 48692.830437)
        self.value_test("Current density - x", point["Jrcx"], -12352.691339)
        self.value_test("Current density - y", point["Jrcy"], -47099.923064)
        self.value_test("Losses", point["pj"], 23709.917359)
        
        # volume integral
        volume = self.current.volume_integrals([0, 1, 2])
        self.value_test("Losses", volume["Pj"], 10070.23937)
        
        # surface integral
        surface = self.current.surface_integrals([0])
        self.value_test("Current", surface["Ir"], 3629.425713)
        
class CurrentAxisymmetric(agros2d.Agros2DTestCase):
    @classmethod
    def setUpClass(self):  
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
        self.value_test("Current density", point["Jrc"], 1.82737e9)
        self.value_test("Current density - r", point["Jrcr"], -6.320475e8)
        self.value_test("Current density - z", point["Jrcz"], -1.714583e9)
        self.value_test("Losses", point["pj"], 5.858385e10)	
        
        # volume integral
        volume = self.current.volume_integrals([0])
        self.value_test("Losses", volume["Pj"], 4.542019e9)
        
        # surface integral
        surface = self.current.surface_integrals([1])
        self.value_test("Current", surface["Ir"], -2.166256e8)        
        
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = agros2d.Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(CurrentPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(CurrentAxisymmetric))
    suite.run(result)
