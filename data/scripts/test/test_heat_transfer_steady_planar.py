import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "unnamed"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
heat = agros2d.field("heat")
heat.analysis_type = "steadystate"
heat.number_of_refinements = 2
heat.polynomial_order = 3
heat.linearity_type = "linear"
heat.weak_forms = "compiled"
heat.nonlineartolerance = 0.001
heat.nonlinearsteps = 10

heat.add_boundary("T inner", "heat_temperature", {"T" : -15})
heat.add_boundary("T outer", "heat_heat_flux", {"f" : 0, "h" : 20, "Te" : 20})
heat.add_boundary("Neumann", "heat_heat_flux", {"f" : 0, "h" : 0, "Te" : 0})

heat.add_material("Material 1", {"lambda" : 2, "p" : 0}) 
heat.add_material("Material 2", {"lambda" : 10, "p" : 70000}) 

# geometry
geometry = agros2d.geometry()

# edges

geometry.add_edge(0.1, 0.15, 0, 0.15, boundaries = {"T outer"})
geometry.add_edge(0, 0.15, 0, 0.1, boundaries = {"Neumann"})
geometry.add_edge(0, 0.1, 0.05, 0.1, boundaries = {"T inner"})
geometry.add_edge(0.05, 0.1, 0.05, 0, boundaries = {"T inner"})
geometry.add_edge(0.05, 0, 0.1, 0, boundaries = {"Neumann"})
geometry.add_edge(0.05, 0.1, 0.1, 0.1,)
geometry.add_edge(0.1, 0.15, 0.1, 0.1, boundaries = {"T outer"})
geometry.add_edge(0.1, 0.1, 0.1, 0, boundaries = {"T outer"})


# labels
geometry.add_label(0.0553981, 0.124595, materials = {"Material 1"}, area=0.0003)
geometry.add_label(0.070091, 0.068229, materials = {"Material 2"}, area=0.0003)

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.086266, 0.087725)
testT = heat.test("Temperature", point["T"], -4.363565)
testG = heat.test("Gradient", point["G"], 155.570361)
testGx = heat.test("Gradient - x", point["Gx"], -153.6640796)
testGy = heat.test("Gradient - y", point["Gy"], -24.2793731)
testF = heat.test("Heat flux", point["F"], 1555.703613)
testFx = heat.test("Heat flux - x", point["Fx"], -1536.640796)
testFy = heat.test("Heat flux - y", point["Fy"], -242.793731)

# volume integral
volume = heat.volume_integrals([0])
testTavg = heat.test("Average temperature", volume["T"], -0.023972)

# surface integral
surface = heat.surface_integrals([0, 6, 7])
testFlux = heat.test("Heat flux", surface["f"], -103.220778)

print("Test: Heat transfer steady state - planar: " + str(testT and testG and testGx and testGy and testF and testFx and testFy and testTavg and testFlux))