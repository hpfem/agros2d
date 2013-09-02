import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class CoupledProblemsBasic1(Agros2DTestCase):
    def setUp(self): 
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
        self.current.number_of_refinements = 1
        self.current.polynomial_order = 3
        self.current.solver = "linear"
        
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        self.current.add_boundary("U", "current_potential", {"current_potential" : 0.01})
        self.current.add_boundary("Ground", "current_potential", {"current_potential" : 0})
        
        self.current.add_material("Mild steel", {"current_conductivity" : 6e6})
        self.current.add_material("Aluminium", {"current_conductivity" : 33e6})
        
        self.elasticity = agros2d.field("elasticity")
        self.elasticity.analysis_type = "steadystate"
        self.elasticity.number_of_refinements = 1
        self.elasticity.polynomial_order = 3
        self.elasticity.solver = "linear"
        
        self.elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
        self.elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})
        
        self.elasticity.add_material("Mild steel", {"elasticity_alpha" : 15e-6, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_poisson_ratio" : 0.33, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 20, "elasticity_young_modulus" : 208e9})
        self.elasticity.add_material("Aluminium", {"elasticity_alpha" : 23e-6, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_poisson_ratio" : 0.33, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 20, "elasticity_young_modulus" : 70e9})
        
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 1
        self.heat.polynomial_order = 3
        self.heat.solver = "linear"
        
        self.heat.add_boundary("Zero flux", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        
        self.heat.add_material("Mild steel", {"heat_conductivity" : 400, "heat_volume_heat" : 0})
        self.heat.add_material("Aluminium", {"heat_conductivity" : 250, "heat_volume_heat" : 0})
        
        # coupling
        # problem.set_coupling_type("current", "heat", "hard")
        # problem.set_coupling_type("heat", "elasticity", "hard")
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0, 0.01, 0.191, 0.02, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.205, 0.01, 0.191, 0.02, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Zero flux"})
        geometry.add_edge(0.205, 0.01, 0.204, 0.002, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0, 0.01, 0, 0, boundaries = {"current" : "U", "elasticity" : "Fixed", "heat" : "Zero flux"})
        geometry.add_edge(0, 0, 0, -0.01, boundaries = {"current" : "U", "elasticity" : "Fixed", "heat" : "Zero flux"})
        geometry.add_edge(0.204, 0.002, 0.194, -0.002, boundaries = {"current" : "Ground", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.194, -0.002, 0.194, -0.008, boundaries = {"current" : "Ground", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.194, -0.002, 0, 0, boundaries = {})
        geometry.add_edge(0.13, 0.014, 0.086, 0.002, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Zero flux"})
        geometry.add_edge(0.13, 0.014, 0.12, 0.004, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Zero flux"})
        geometry.add_edge(0.086, 0.002, 0.12, 0.004, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Zero flux"})
        geometry.add_edge(0.084, -0.014, 0.104, -0.004, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.104, -0.004, 0.128, -0.004, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.128, -0.004, 0.13, -0.026, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.13, -0.026, 0.194, -0.008, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        geometry.add_edge(0.084, -0.014, 0, -0.01, boundaries = {"current" : "Neumann", "elasticity" : "Free", "heat" : "Convection"})
        
        # labels
        geometry.add_label(0.173639, 0.0106815, materials = {"current" : "Mild steel", "elasticity" : "Mild steel", "heat" : "Mild steel"})
        geometry.add_label(0.160202, -0.00535067, materials = {"current" : "Aluminium", "elasticity" : "Aluminium", "heat" : "Aluminium"})
        geometry.add_label(0.116793, 0.00774503, materials = {"current" : "none", "elasticity" : "none", "heat" : "none"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):         
        # point value
        local_values_current = self.current.local_values(0.155787, 0.00713725)
        self.value_test("Current - Scalar potential", local_values_current["V"], 0.001555)
        local_values_heat = self.heat.local_values(0.155787, 0.00713725)
        self.value_test("Heat transfer - Temperature", local_values_heat["T"], 40.649361)
        local_values_elasticity = self.elasticity.local_values(0.155787, 0.00713725)
        self.value_test("Thermoelasticity - Displacement", local_values_elasticity["d"], 1.592721e-4)

class CoupledProblemsBasic2(Agros2DTestCase):
    def setUp(self): 
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 50
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 1
        self.heat.polynomial_order = 2
        self.heat.solver = "linear"
        self.heat.solver_parameters['tolerance'] = 0.001
        
        self.heat.add_boundary("Symmetry", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 10, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        self.heat.add_boundary("Radiation", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 5, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        
        self.heat.add_material("Steel", {"heat_conductivity" : 70})
        self.heat.add_material("Insulation", {"heat_conductivity" : 6})
        
        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 2
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_imag" : 0, "magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Air", {"magnetic_conductivity" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        self.magnetic.add_material("Copper", {"magnetic_conductivity" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 6e5, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        self.magnetic.add_material("Insulation", {"magnetic_conductivity" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        self.magnetic.add_material("Steel", {"magnetic_conductivity" : 3e5, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 100, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})        
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0.3, 0.6, 0, 0.6, boundaries = {"heat" : "Radiation"})
        geometry.add_edge(0, 1.2, 0.9, 1.2, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.9, 1.2, 0.9, -0.5, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.9, -0.5, 0, -0.5, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.5, 0, 0, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0, 0.32, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.3, 0.6, 0.3, 0.1, boundaries = {})
        geometry.add_edge(0, 0.1, 0.3, 0.1, boundaries = {})
        geometry.add_edge(0, 0.6, 0, 0.1, boundaries = {"heat" : "Symmetry", "magnetic" : "A = 0"})
        geometry.add_edge(0, 0.1, 0, 0, boundaries = {"heat" : "Symmetry", "magnetic" : "A = 0"})
        geometry.add_edge(0.33, 0.7, 0.4, 0.7, boundaries = {})
        geometry.add_edge(0.4, 0.7, 0.4, 0.046, boundaries = {})
        geometry.add_edge(0.4, 0.046, 0.33, 0.046, boundaries = {})
        geometry.add_edge(0.33, 0.046, 0.33, 0.7, boundaries = {})
        geometry.add_edge(0.3, 0.6, 0.32, 0.6, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.32, 0, 0.32, 0.6, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0, 1.2, 0, 0.6, boundaries = {"magnetic" : "A = 0"})
        
        # labels
        geometry.add_label(0.627519, 0.954318, materials = {"heat" : "none", "magnetic" : "Air"}, area = 0.005)
        geometry.add_label(0.087409, 0.293345, materials = {"heat" : "Steel", "magnetic" : "Steel"}, area = 0.0005)
        geometry.add_label(0.132733, 0.0478408, materials = {"heat" : "Insulation", "magnetic" : "Insulation"})
        geometry.add_label(0.378237, 0.221582, materials = {"heat" : "none", "magnetic" : "Copper"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()

    def test_values(self):           
        # point value
        local_values_magnetic = self.magnetic.local_values(0.2956, 0.2190)
        self.value_test("Magnetic - Vector potential", local_values_magnetic["A"], 0.009137)
        local_values_heat = self.heat.local_values(0.2956, 0.2190)
        self.value_test("Heat transfer - Temperature", local_values_heat["T"], 975.749917)

class CoupledProblemsBasic3(Agros2DTestCase):
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
        self.current.polynomial_order = 2
        self.current.solver = "linear"
        self.current.solver_parameters['tolerance'] = 0.001
        self.current.solver_parameters['steps'] = 10
        
        self.current.add_boundary("10 V", "current_potential", {"current_potential" : 10})
        self.current.add_boundary("0 V", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        
        self.current.add_material("Material", {"current_conductivity" : 100000})
        
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 2
        self.heat.polynomial_order = 3
        
        self.heat.solver = "newton"
        self.heat.solver_parameters['tolerance'] = 0.001
        self.heat.solver_parameters['steps'] = 20
        self.heat.solver_parameters['damping'] = 'fixed'
        self.heat.solver_parameters['damping_factor_increase_steps'] = 1
        self.heat.solver_parameters['jacobian_reuse'] = True
        self.heat.solver_parameters['jacobian_reuse_steps'] = 10
        self.heat.solver_parameters['jacobian_reuse_ratio'] = 0.9
        
        self.heat.add_boundary("300 K", "heat_temperature", {"heat_temperature" : 300})
        self.heat.add_boundary("Flux", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 20, "heat_radiation_emissivity" : 0.95})
        
        self.heat.add_material("Material", {"heat_conductivity" : { "value" : 385, "x" : [0,100,400,900,1200,1800], "y" : [300,350,480,300,280,320] }, "heat_volume_heat" : 0})
        
        self.elasticity = agros2d.field("elasticity")
        self.elasticity.analysis_type = "steadystate"
        self.elasticity.number_of_refinements = 1
        self.elasticity.polynomial_order = 3
        self.elasticity.solver = "linear"
        
        self.elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
        self.elasticity.add_boundary("Fixed free", "elasticity_fixed_free", {"elasticity_displacement_x" : 0, "elasticity_force_y" : 0})
        self.elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})
        
        self.elasticity.add_material("Material", {"elasticity_alpha" : 2.3e-05, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_poisson_ratio" : 0.33, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 300, "elasticity_young_modulus" : 7e+10})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0.25, 0.2, 0.25, -0.25, boundaries = {"elasticity" : "Fixed free"})
        geometry.add_edge(0, -0.25, 0, 0.25, boundaries = {"heat" : "300 K"})
        geometry.add_edge(0, 0.25, 0.1, 0.28, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0.1, 0.28, 0.19, 0.25, boundaries = {"current" : "10 V", "heat" : "Flux"})
        geometry.add_edge(0.19, 0.25, 0.25, 0.2, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0, -0.25, 0.1, -0.4, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0.1, -0.4, 0.15, -0.25, boundaries = {"current" : "0 V", "heat" : "Flux"})
        geometry.add_edge(0.15, -0.25, 0.25, -0.25, boundaries = {"heat" : "Flux"})
        geometry.add_edge(0.1, -0.4, 0.1, 0.28, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.19, 0.25, 0.15, -0.25, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.25, 0.2, 0.45, 0.25, boundaries = {"elasticity" : "Fixed", "heat" : "Flux"})
        geometry.add_edge(0.45, 0.25, 0.3, -0.25, boundaries = {"elasticity" : "Fixed free", "heat" : "300 K"})
        geometry.add_edge(0.3, -0.25, 0.25, -0.25, boundaries = {"elasticity" : "Free", "heat" : "Flux"})
        
        # labels
        geometry.add_label(0.103556, -0.00186029, materials = {"current" : "Material", "elasticity" : "none", "heat" : "Material"})
        geometry.add_label(0.0497159, 0.00460215, materials = {"current" : "none", "elasticity" : "none", "heat" : "Material"})
        geometry.add_label(0.204343, 0.00622128, materials = {"current" : "none", "elasticity" : "none", "heat" : "Material"})
        geometry.add_label(0.276729, -0.0804181, materials = {"current" : "none", "elasticity" : "Material", "heat" : "Material"})
        
        agros2d.view.zoom_best_fit()
        
        # solve problem
        problem.solve()
        
    def test_values(self):    
        # point value
        local_values_current = self.current.local_values(0.140872, -0.015538)
        self.value_test("Current - Scalar potential", local_values_current["V"], 5.712807)
        local_values_heat = self.heat.local_values(0.277308, -0.216051)
        self.value_test("Heat transfer - Temperature", local_values_heat["T"], 395.728987)
        self.value_test("Heat transfer - Heat conductivity", local_values_heat["lambda"], 479.949868)
        local_values_heat = self.heat.local_values(0.063718, -0.022513)
        self.value_test("Heat transfer - Heat conductivity", local_values_heat["lambda"], 299.195502)
        local_values_elasticity = self.elasticity.local_values(0.277308,-0.216051)
        self.value_test("Thermoelasticity - Displacement", local_values_elasticity["d"], 0.001958)        

class CoupledProblemsBasic4(Agros2DTestCase):
    def setUp(self): 
        # model
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.time_step_method = "fixed"
        problem.time_method_order = 2
        problem.time_steps = 6
        problem.time_total = 60
        
        # disable view
        agros2d.view.mesh.disable()
        agros2d.view.post2d.disable()
        
        # fields
        self.current = agros2d.field("current")
        self.current.analysis_type = "steadystate"
        self.current.polynomial_order = 2
        self.current.solver = "linear"
        
        self.current.add_boundary("U = 0.1", "current_potential", {"current_potential" : 0.1})
        self.current.add_boundary("U = 0", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        
        self.current.add_material("Cu", {"current_conductivity" : 33e6})
        
        self.heat = agros2d.field("heat")
        self.heat.analysis_type = "transient"
        self.heat.polynomial_order = 2
        self.heat.solver = "linear"
        self.heat.transient_initial_condition = 20
        
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        self.heat.add_boundary("Zero heat flux", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
        self.heat.add_boundary("T = 50", "heat_temperature", {"heat_temperature" : 50})
        
        self.heat.add_material("Cu", {"heat_conductivity" : 230, "heat_density" : 2700, "heat_specific_heat" : 900, "heat_volume_heat" : 0})
        self.heat.add_material("Fe", {"heat_conductivity" : 80, "heat_density" : 7870, "heat_specific_heat" : 450, "heat_volume_heat" : 0})
        self.heat.add_material("Fe (source)", {"heat_conductivity" : 80, "heat_density" : 7870, "heat_specific_heat" : 450, "heat_volume_heat" : 1e7})
        
        # geometry
        geometry = agros2d.geometry
        
        # edges
        geometry.add_edge(0.25, 0.25, 0, 0, boundaries = {"current" : "neumann"}, angle = 90)
        geometry.add_edge(0.25, 0.15, 0.1, 0, boundaries = {"current" : "neumann"}, angle = 90)
        geometry.add_edge(0, 0, 0.1, 0, boundaries = {"current" : "U = 0.1", "heat" : "Zero heat flux"})
        geometry.add_edge(0.25, 0.15, 0.25, 0.25, boundaries = {"current" : "U = 0", "heat" : "Zero heat flux"})
        geometry.add_edge(0, 0, -0.25, 0, boundaries = {"heat" : "Zero heat flux"})
        geometry.add_edge(0.25, 0.25, 0.25, 0.5, boundaries = {"heat" : "Zero heat flux"})
        geometry.add_edge(0.25, 0, 0.25, 0.15, boundaries = {"heat" : "Zero heat flux"})
        geometry.add_edge(0.25, 0, 0.1, 0, boundaries = {"heat" : "Zero heat flux"})
        geometry.add_edge(-0.25, 0, -0.25, 0.4, boundaries = {"heat" : "Convection"})
        geometry.add_edge(-0.25, 0.4, -0.15, 0.5, boundaries = {"heat" : "T = 50"})
        geometry.add_edge(-0.15, 0.5, 0.25, 0.5, boundaries = {"heat" : "Convection"})
        geometry.add_edge(-0.15, 0.15, -0.1, 0.2, boundaries = {}, angle = 90)
        geometry.add_edge(-0.1, 0.2, -0.15, 0.25, boundaries = {}, angle = 90)
        geometry.add_edge(-0.15, 0.25, -0.15, 0.15, boundaries = {})
        geometry.add_edge(0, 0.35, 0.1, 0.35, boundaries = {})
        geometry.add_edge(0.1, 0.35, 0.05, 0.4, boundaries = {})
        geometry.add_edge(0.05, 0.4, 0, 0.35, boundaries = {})
        
        # labels
        geometry.add_label(0.1, 0.15, materials = {"current" : "Cu", "heat" : "Cu"})
        geometry.add_label(0.2, 0.05, materials = {"current" : "none", "heat" : "Fe"})
        geometry.add_label(-0.15, 0.4, materials = {"current" : "none", "heat" : "Fe"})
        geometry.add_label(-0.123246, 0.198947, materials = {"current" : "none", "heat" : "Fe (source)"})
        geometry.add_label(0.05, 0.378655, materials = {"current" : "none", "heat" : "Fe (source)"})
        
        agros2d.view.zoom_best_fit()
        
        problem.solve()
        
    def test_values(self):        
        # point value
        local_values_current = self.current.local_values(0.1, 0.15)
        self.value_test("Current - Scalar potential", local_values_current["V"], 0.049999)
        self.value_test("Current - Current density", local_values_current["Jrc"], 9.901206e6)
        local_values_heat = self.heat.local_values(0.05, 0.05)
        self.value_test("Heat - Temperature 1", local_values_heat["T"], 71.88167)
        local_values_heat = self.heat.local_values(-0.05, 0.15)
        self.value_test("Heat - Temperature 2", local_values_heat["T"], 25.002605)
        volume_integral_heat = self.heat.volume_integrals([0, 1, 2, 3, 4])
        self.value_test("Heat - Temperature volume", volume_integral_heat["T"], 8.498177)

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(CoupledProblemsBasic1))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(CoupledProblemsBasic2))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(CoupledProblemsBasic3))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(CoupledProblemsBasic4))
    suite.run(result)