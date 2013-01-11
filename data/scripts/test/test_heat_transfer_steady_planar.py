import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
heat = agros2d.field("heat")
heat.analysis_type = "steadystate"
heat.number_of_refinements = 2
heat.polynomial_order = 3
heat.linearity_type = "linear"

heat.add_boundary("T inner", "heat_temperature", {"heat_temperature" : -15})
heat.add_boundary("T outer", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 20, "heat_convection_external_temperature" : 20})
heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})

heat.add_material("Material 1", {"heat_volume_heat" : 0, "heat_conductivity" : 2, "heat_density" : 7800, "heat_velocity_x" : 0.00002, "heat_velocity_y" : -0.00003, "heat_specific_heat" : 300, "heat_velocity_angular" : 0})
heat.add_material("Material 2", {"heat_volume_heat" : 70000, "heat_conductivity" : 10, "heat_density" : 0, "heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_specific_heat" : 0, "heat_velocity_angular" : 0})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0.1, 0.15, 0, 0.15, boundaries = {"heat" : "T outer"})
geometry.add_edge(0, 0.15, 0, 0.1, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0, 0.1, 0.05, 0.1, boundaries = {"heat" : "T inner"})
geometry.add_edge(0.05, 0.1, 0.05, 0, boundaries = {"heat" : "T inner"})
geometry.add_edge(0.05, 0, 0.1, 0, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.05, 0.1, 0.1, 0.1)
geometry.add_edge(0.1, 0.15, 0.1, 0.1, boundaries = {"heat" : "T outer"})
geometry.add_edge(0.1, 0.1, 0.1, 0, boundaries = {"heat" : "T outer"})


# labels
geometry.add_label(0.0553981, 0.124595, materials = {"heat" : "Material 1"}, area=0.0003)
geometry.add_label(0.070091, 0.068229, materials = {"heat" : "Material 2"}, area=0.0003)

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.079734, 0.120078)
testT = agros2d.test("Temperature", point["T"], 2.76619)
testG = agros2d.test("Gradient", point["G"], 299.50258)
testGx = agros2d.test("Gradient - x", point["Gx"], -132.7564285)
testGy = agros2d.test("Gradient - y", point["Gy"], -268.47258)
testF = agros2d.test("Heat flux", point["F"], 599.00516)
testFx = agros2d.test("Heat flux - x", point["Fx"], -265.512857)
testFy = agros2d.test("Heat flux - y", point["Fy"], -536.94516)

# volume integral
volume = heat.volume_integrals([0])
testTavg = agros2d.test("Temperature", volume["T"], 0.00335)

# surface integral
surface = heat.surface_integrals([0, 6, 7])
testFlux = agros2d.test("Heat flux", surface["f"], -85.821798)

print("Test: Heat transfer steady state - planar: " + str(testT and testG and testGx and testGy and testF and testFx and testFy and testTavg and testFlux))