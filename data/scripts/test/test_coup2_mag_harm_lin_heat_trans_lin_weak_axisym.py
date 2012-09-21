# based on test_coupling_2_axisymmetric

import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 50
problem.time_step_method = "fixed"
problem.time_method_order = 1
problem.time_steps = 1
problem.time_total = 1

# palette quality
agros2d.view.post2d.scalar_palette_quality = "extremely_coarse"

# fields
heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.number_of_refinements = 1
heat.polynomial_order = 2
heat.linearity_type = "linear"

heat.add_boundary("Symmetry", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 10, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("Radiation", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 5, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})

heat.add_material("Steel", {"heat_conductivity" : 70})
heat.add_material("Insulation", {"heat_conductivity" : 6})

magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 2
magnetic.linearity_type = "linear"

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_imag" : 0, "magnetic_potential_real" : 0})

magnetic.add_material("Air", {"magnetic_conductivity" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
magnetic.add_material("Copper", {"magnetic_conductivity" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 6e5, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
magnetic.add_material("Insulation", {"magnetic_conductivity" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
magnetic.add_material("Steel", {"magnetic_conductivity" : 3e5, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 100, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})

# coupling
problem.set_coupling_type("magnetic", "heat", "weak")

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

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
local_values_magnetic = magnetic.local_values(0.2956, 0.2190)
testA = agros2d.test("Magnetic - Vector potential", local_values_magnetic["A"], 0.009137)
local_values_heat = heat.local_values(0.2956, 0.2190)
testT = agros2d.test("Heat transfer - Temperature", local_values_heat["T"], 975.749917)

print("Test: Coupling 2 - axisymmetric, mag harm + heat transient, both lin, weak: " + str(testA and testT))
