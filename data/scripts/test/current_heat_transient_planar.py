import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "nepojmenovaný"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step = 10
problem.time_total = 60

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.weak_forms = "compiled"
current.number_of_refinements = 2
current.polynomial_order = 2
current.linearity_type = "linear"

current.add_boundary("0.1", "current_potential", {"current_potential" : 0.1})
current.add_boundary("0", "current_potential", {"current_potential" : 0})
current.add_boundary("neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})

current.add_material("Cu", {"current_conductivity" : 3.3e+07})

heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.weak_forms = "compiled"
heat.number_of_refinements = 2
heat.polynomial_order = 2
heat.linearity_type = "linear"
heat.initial_condition = 20

heat.add_boundary("convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 20, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("heat flux", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("50", "heat_temperature", {"heat_temperature" : 50})

heat.add_material("Cu", {"heat_conductivity" : 200, "heat_density" : 8700, "heat_specific_heat" : 385, "heat_volume_heat" : 0})


# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0.25, 0.25, 0, 0, boundaries = {"current" : "neumann", "heat" : "convection"}, angle = 90)
geometry.add_edge(0.25, 0.15, 0.1, 0, boundaries = {"current" : "neumann", "heat" : "convection"}, angle = 90)
geometry.add_edge(0, 0, 0.1, 0, boundaries = {"current" : "0.1", "heat" : "50"})
geometry.add_edge(0.25, 0.15, 0.25, 0.25, boundaries = {"current" : "0", "heat" : "heat flux"})

# labels
geometry.add_label(0.100585, 0.148538, materials = {"current" : "Cu", "heat" : "Cu"})

geometry.zoom_best_fit()

problem.solve()

# point value
local_values_current = current.local_values(0.1, 0.15)
testV = agros2d.test("Current - Scalar potential", local_values_current["V"], 0.05)
testJ = agros2d.test("Current - Current density", local_values_current["J"], 9.902297e6)
local_values_heat = heat.local_values(0.05, 0.05)
testT = agros2d.test("Heat - Temperature", local_values_heat["T"], 74.937517)
#surface_integral_heat = heat.surface_integrals([0, 1])
#testf = agros2d.test("Heat - Heat flux", surface_integral_heat["f"], 584.000204*surface_integral_heat["l"])
volume_integral_heat = heat.volume_integrals([0])
testTvol = agros2d.test("Heat - Temperature", volume_integral_heat["T"], 2.466761)

print("Test: current - heat (transient) - planar: " + str(testV and testJ and testT and testTvol)) #testf