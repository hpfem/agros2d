import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# palette quality
agros2d.view.post2d.scalar_palette_quality = "extremely_coarse"

# fields
heat = agros2d.field("heat")
heat.analysis_type = "steadystate"
heat.number_of_refinements = 2
heat.polynomial_order = 3
heat.linearity_type = "linear"

heat.add_boundary("T inner", "heat_temperature", {"heat_temperature" : -15})
heat.add_boundary("T outer", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 20, "heat_convection_external_temperature" : 20})
heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})

heat.add_material("Material 1", {"heat_conductivity" : 2, "heat_volume_heat" : 0}) 
heat.add_material("Material 2", {"heat_conductivity" : 10, "heat_volume_heat" : 70000}) 

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
point = heat.local_values(0.086266, 0.087725)
testT = agros2d.test("Temperature", point["T"], -4.363565)
testG = agros2d.test("Gradient", point["G"], 155.570361)
testGx = agros2d.test("Gradient - x", point["Gx"], -153.6640796)
testGy = agros2d.test("Gradient - y", point["Gy"], -24.2793731)
testF = agros2d.test("Heat flux", point["F"], 1555.703613)
testFx = agros2d.test("Heat flux - x", point["Fx"], -1536.640796)
testFy = agros2d.test("Heat flux - y", point["Fy"], -242.793731)

# volume integral
volume = heat.volume_integrals([0])
testTavg = agros2d.test("Temperature", volume["T"], -0.023972)

# surface integral
surface = heat.surface_integrals([0, 6, 7])
testFlux = agros2d.test("Heat flux", surface["f"], -103.220778)

print("Test: Heat transfer steady state - planar: " + str(testT and testG and testGx and testGy and testF and testFx and testFy and testTavg and testFlux))