import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.name = "Heat transfer axisymmetric"
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

heat.add_boundary("Neumann", "heat_heat_flux", {"f" : 0, "h" : 0, "Te" : 0})
heat.add_boundary("Inlet", "heat_heat_flux", {"f" : 500000, "h" : 0, "Te" : 0})
heat.add_boundary("Temperature", "heat_temperature", {"T" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"f" : 0, "h" : 20, "Te" : 20})

heat.add_material("Material", {"lambda" : 52, "p" : 6e6})   
             
# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(0.02, 0, 0.1, 0, boundaries = {"Temperature"})
geometry.add_edge(0.1, 0, 0.1, 0.14, boundaries = {"Convection"})
geometry.add_edge(0.1, 0.14, 0.02, 0.14, boundaries = {"Temperature"})
geometry.add_edge(0.02, 0.14, 0.02, 0.1, boundaries = {"Neumann"})
geometry.add_edge(0.02, 0.04, 0.02, 0, boundaries = {"Neumann"})
geometry.add_edge(0.02, 0.04, 0.02, 0.1, boundaries = {"Inlet"})


# labels
geometry.add_label(0.0460134, 0.0867717, materials = {"Material"}, area=0.0003)

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.062926, 0.038129)
testT = heat.test("Temperature", point["T"], 263.811616)
testG = heat.test("Gradient", point["G"], 4615.703047)
testGr = heat.test("Gradient - r", point["Gr"], 808.030874788)
testGz = heat.test("Gradient - z", point["Gz"], -4544.425)
testF = heat.test("Heat flux", point["F"], 2.400166e5)
testFr = heat.test("Heat flux - r", point["Fr"], 42017.605489)
testFz = heat.test("Heat flux - z", point["Fz"], -2.363101e5)

# volume integral
surface = heat.volume_integrals([0])
testTavg = heat.test("Average temperature", volume["T"], 0.949673)

# surface integral
surface = heat.surface_integrals([1])
testFlux = heat.test("Heat flux", surface["f"], 333.504915)

print("Test: Heat transfer steady state - axisymmetric: " + str(testT and testG and testGr and testGz and testF and testFr and testFz and testTavg and testFlux))