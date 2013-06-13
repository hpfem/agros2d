# based on test_coupling_3_axisymmetric_nonlin.py

import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
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
current.polynomial_order = 2
current.linearity_type = "newton"
current.nonlinear_tolerance = 0.001

current.add_boundary("10 V", "current_potential", {"current_potential" : 10})
current.add_boundary("0 V", "current_potential", {"current_potential" : 0})
current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})

current.add_material("Material", {"current_conductivity" : 100000})

heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.number_of_refinements = 2
heat.polynomial_order = 3
heat.linearity_type = "newton"
heat.nonlinear_tolerance = 0.001

heat.add_boundary("300 K", "heat_temperature", {"heat_temperature" : 300})
heat.add_boundary("Flux", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 20, "heat_radiation_emissivity" : 0.95})

heat.add_material("Material", {"heat_conductivity" : { "value" : 385, "x" : [0,100,400,900,1200,1800], "y" : [300,350,480,300,280,320] }, "heat_volume_heat" : 0})

elasticity = agros2d.field("elasticity")
elasticity.analysis_type = "steadystate"
elasticity.number_of_refinements = 1
elasticity.polynomial_order = 3
elasticity.linearity_type = "newton"
elasticity.nonlinear_tolerance = 0.001

elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
elasticity.add_boundary("Fixed free", "elasticity_fixed_free", {"elasticity_displacement_x" : 0, "elasticity_force_y" : 0})
elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})

elasticity.add_material("Material", {"elasticity_alpha" : 2.3e-05, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_poisson_ratio" : 0.33, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 300, "elasticity_young_modulus" : 7e+10})

problem.set_coupling_type("current", "heat", "hard")
problem.set_coupling_type("heat", "elasticity", "hard")

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

# point value
local_values_current = current.local_values(0.140872, -0.015538)
testV = agros2d.test("Current - Scalar potential", local_values_current["V"], 5.712807)
local_values_heat = heat.local_values(0.277308, -0.216051)
testT = agros2d.test("Heat transfer - Temperature", local_values_heat["T"], 395.728987)
testL1 = agros2d.test("Heat transfer - Heat conductivity", local_values_heat["lambda"], 479.949868)
local_values_heat = heat.local_values(0.063718, -0.022513)
testL2 = agros2d.test("Heat transfer - Heat conductivity", local_values_heat["lambda"], 299.195502)
local_values_elasticity = elasticity.local_values(0.277308,-0.216051)
testD = agros2d.test("Thermoelasticity - Displacement", local_values_elasticity["d"], 0.001958)

elapsed_time = time() - start_time
print("Test: Coupling 3 - axisymetric, current nonlin, heat realy nonlin transient, elasticity nonlin,  hard ({0}): ".format(round(elapsed_time, 3)) + str(testV and testT and testD and testL1 and testL2))
