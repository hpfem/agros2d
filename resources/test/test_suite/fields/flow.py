import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class FlowPlanar(Agros2DTestCase):
    def setUp(self):         
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "gmsh_quad_delaunay"        
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()

        # flow
        self.flow = agros2d.field("flow")
        self.flow.analysis_type = "steadystate"
        self.flow.number_of_refinements = 2
        self.flow.polynomial_order = 2
        
        self.flow.solver = "newton"
        self.flow.solver_parameters['tolerance'] = 0.0001
        self.flow.solver_parameters['damping'] = 'automatic'
        self.flow.solver_parameters['damping_factor'] = 1.0
        self.flow.solver_parameters['jacobian_reuse'] = False
        
        # boundaries        
        self.flow.add_boundary("Inlet", "flow_velocity", {"flow_velocity_x" : { "expression" : "cos((y-0.135)/0.035*pi/2)" }, "flow_velocity_y" : 0})
        self.flow.add_boundary("Wall", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : 0})
        self.flow.add_boundary("Outlet", "flow_outlet", {})
        
        # materials        
        self.flow.add_material("Water", {"flow_density" : 1, "flow_force_x" : 0, "flow_force_y" : 0, "flow_viscosity" : 0.001})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(-0.35, 0.05, 0.4, 0.05, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.4, 0.05, 0.4, 0.25, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.25, 0.1, -0.2, 0.1, boundaries = {"flow" : "Wall"}, angle = 90)
        geometry.add_edge(-0.2, 0.1, -0.2, 0.2, boundaries = {"flow" : "Wall"}, angle = 90)
        geometry.add_edge(-0.2, 0.2, -0.25, 0.2, boundaries = {"flow" : "Wall"}, angle = 90)
        geometry.add_edge(-0.25, 0.2, -0.25, 0.1, boundaries = {"flow" : "Wall"}, angle = 90)
        geometry.add_edge(-0.35, 0.2, -0.05, 0.3, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.05, 0.3, 0, 0.15, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0, 0.15, 0.05, 0.1, boundaries = {"flow" : "Wall"}, angle = 90)
        geometry.add_edge(0.05, 0.1, 0.1, 0.15, boundaries = {"flow" : "Wall"}, angle = 90)
        geometry.add_edge(0.2, 0.3, 0.1, 0.15, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.3, 0.3, 0.3, 0.25, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.3, 0.25, 0.4, 0.25, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.2, 0.3, 0.3, 0.3, boundaries = {"flow" : "Outlet"})
        geometry.add_edge(-0.4, 0.17, -0.4, 0.1, boundaries = {"flow" : "Inlet"})
        geometry.add_edge(-0.35, 0.2, -0.35, 0.17, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.4, 0.17, -0.35, 0.17, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.4, 0.1, -0.35, 0.1, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.35, 0.1, -0.35, 0.05, boundaries = {"flow" : "Wall"})
        
        geometry.add_label(-0.086153, 0.205999, materials = {"flow" : "Water"}, area = 0.00048)
        geometry.add_label(-0.224921, 0.126655, materials = {"flow" : "none"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):        
        # point value
        point = self.flow.local_values(-0.023486, 0.117842)
        self.value_test("Pressure", point["p"], 0.65205)
        self.value_test("Velocity", point["v"], 0.406233)
        self.value_test("Velocity - x", point["vx"], 0.337962)
        self.value_test("Velocity - y", point["vy"], -0.225404)
        
        # volume integral
        # volume = flow.volume_integrals([0])
        # testPj = agros2d.test("Losses", volume["Pj"], 10070.23937)
        
        # surface integral
        # surface = flow.surface_integrals([0])
        # testI = agros2d.test("Current", surface["Ir"], 3629.425713)

class FlowAxisymmetric(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "gmsh_quad"
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # flow
        self.flow = agros2d.field("flow")
        self.flow.analysis_type = "steadystate"
        self.flow.number_of_refinements = 1
        self.flow.polynomial_order = 2
        self.flow.adaptivity_type = "disabled"
        
        self.flow.solver = "newton"
        self.flow.solver_parameters['tolerance'] = 0.0001
        self.flow.solver_parameters['damping'] = 'automatic'
        self.flow.solver_parameters['damping_factor'] = 1.0
        self.flow.solver_parameters['jacobian_reuse'] = False
        
        # boundaries
        self.flow.add_boundary("inlet", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : { "expression" : "-cos(r/0.25*pi/2)" }})
        self.flow.add_boundary("outlet", "flow_outlet", {})
        self.flow.add_boundary("wall", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : 0})
        self.flow.add_boundary("symmetry", "flow_symmetry", {})
        
        # materials
        self.flow.add_material("fluid", {"flow_viscosity" : 0.001, "flow_density" : 1, "flow_force_x" : 0, "flow_force_y" : 0})
        
        # geometry
        geometry = agros2d.geometry
        geometry.add_edge(0, 0.5, 0.25, 0.5, boundaries = {"flow" : "inlet"})
        geometry.add_edge(0.25, 0.5, 0.25, -0.05, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.25, -0.05, 0.15, -0.15, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.15, -0.15, 0.15, -0.5, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.05, -0.5, 0.05, -0.05, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.05, -0.05, 0.15, -0.05, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.15, -0.05, 0, 0.1, angle = 90, boundaries = {"flow" : "wall"})
        geometry.add_edge(0, 0.1, 0, 0.5, boundaries = {"flow" : "symmetry"})
        geometry.add_edge(0.05, -0.5, 0, -0.5, boundaries = {"flow" : "wall"})
        geometry.add_edge(0, -0.5, 0, -1.3, boundaries = {"flow" : "symmetry"})
        geometry.add_edge(0, -1.3, 0.15, -1.3, boundaries = {"flow" : "outlet"})
        geometry.add_edge(0.15, -1.3, 0.15, -0.55, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.15, -0.55, 0.15, -0.5, boundaries = {"flow" : "wall"})
        
        geometry.add_label(0.127882, 0.157169, materials = {"flow" : "fluid"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):        
        # point value
        point = self.flow.local_values(2.137e-01, 8.173e-03)
        self.value_test("Pressure", point["p"], 2.880471)
        self.value_test("Velocity", point["v"], 0.652625)
        self.value_test("Velocity - r", point["vr"], 0.02563, 4)
        self.value_test("Velocity - z", point["vz"], -0.652122)
        
        # volume integral
        # volume = self.flow.volume_integrals([0])
        # testPj = agros2d.test("Losses", volume["Pj"], 10070.23937)
        
        # surface integral
        # surface = self.flow.surface_integrals([0])
        # testI = agros2d.test("Current", surface["Ir"], 3629.425713)

if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(FlowPlanar))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(FlowAxisymmetric))
    suite.run(result)
    