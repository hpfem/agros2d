# based on test_coupling_4_transient_planar.py

import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_steps = 6
problem.time_total = 60

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.polynomial_order = 2
current.linearity_type = "newton"

current.add_boundary("U = 0.1", "current_potential", {"current_potential" : 0.1})
current.add_boundary("U = 0", "current_potential", {"current_potential" : 0})
current.add_boundary("neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})

current.add_material("Cu", {"current_conductivity" : 33e6})

heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.polynomial_order = 2
heat.linearity_type = "newton"
heat.initial_condition = 20

heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("Zero heat flux", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("T = 50", "heat_temperature", {"heat_temperature" : 50})

heat.add_material("Cu", {"heat_conductivity" : 230, "heat_density" : 2700, "heat_specific_heat" : 900, "heat_volume_heat" : 0})
heat.add_material("Fe", {"heat_conductivity" : 80, "heat_density" : 7870, "heat_specific_heat" : 450, "heat_volume_heat" : 0})
heat.add_material("Fe (source)", {"heat_conductivity" : 80, "heat_density" : 7870, "heat_specific_heat" : 450, "heat_volume_heat" : 1e7})

problem.set_coupling_type("current", "heat", "weak")

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

# point value
local_values_current = current.local_values(0.1, 0.15)
testV = agros2d.test("Current - Scalar potential", local_values_current["V"], 0.049999)
testJ = agros2d.test("Current - Current density", local_values_current["Jrc"], 9.901206e6)
local_values_heat = heat.local_values(0.05, 0.05)
testT1 = agros2d.test("Heat - Temperature 1", local_values_heat["T"], 71.88167)
local_values_heat = heat.local_values(-0.05, 0.15)
testT2 = agros2d.test("Heat - Temperature 2", local_values_heat["T"], 25.002605)
volume_integral_heat = heat.volume_integrals([0, 1, 2, 3, 4])
testTvol = agros2d.test("Heat - Temperature volume", volume_integral_heat["T"], 8.498177)

elapsed_time = time() - start_time
print("Test: Coupling 4 - transient - planar, newton, weak ({0}): ".format(round(elapsed_time, 3)) + str(testV and testJ and testT1 and testT2 and testTvol)) #testf
