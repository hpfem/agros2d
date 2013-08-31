#based on test_coupling_1_planar

import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

problem.time_step_method = "fixed"
problem.time_method_order = 1
problem.time_steps = 1
problem.time_total = 1

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.number_of_refinements = 1
current.polynomial_order = 3
current.solver = "newton"

current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
current.add_boundary("U", "current_potential", {"current_potential" : 0.01})
current.add_boundary("Ground", "current_potential", {"current_potential" : 0})

current.add_material("Mild steel", {"current_conductivity" : 6e6})
current.add_material("Aluminium", {"current_conductivity" : 33e6})

elasticity = agros2d.field("elasticity")
elasticity.analysis_type = "steadystate"
elasticity.number_of_refinements = 1
elasticity.polynomial_order = 3
elasticity.solver = "newton"

elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})

elasticity.add_material("Mild steel", {"elasticity_alpha" : 15e-6, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_poisson_ratio" : 0.33, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 20, "elasticity_young_modulus" : 208e9})
elasticity.add_material("Aluminium", {"elasticity_alpha" : 23e-6, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_poisson_ratio" : 0.33, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 20, "elasticity_young_modulus" : 70e9})

heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.number_of_refinements = 1
heat.polynomial_order = 3
heat.solver = "newton"

heat.add_boundary("Zero flux", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})

heat.add_material("Mild steel", {"heat_conductivity" : 400, "heat_volume_heat" : 0, "heat_density" : 0, "heat_specific_heat" : 0})
heat.add_material("Aluminium", {"heat_conductivity" : 250, "heat_volume_heat" : 0, "heat_density" : 0, "heat_specific_heat" : 0})

# coupling
problem.set_coupling_type("current", "heat", "hard")
problem.set_coupling_type("heat", "elasticity", "hard")

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

# point value
local_values_current = current.local_values(0.155787, 0.00713725)
testV = agros2d.test("Current - Scalar potential", local_values_current["V"], 0.001555)
local_values_heat = heat.local_values(0.155787, 0.00713725)
testT = agros2d.test("Heat transfer - Temperature", local_values_heat["T"], 40.649361)
local_values_elasticity = elasticity.local_values(0.155787, 0.00713725)
testD = agros2d.test("Thermoelasticity - Displacement", local_values_elasticity["d"], 1.592721e-4)

print("Test: Coupling 1 - planar, heat transient, newton and hard coupling (all 3): " + str(testV and testT and testD))