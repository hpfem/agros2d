import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
heat = agros2d.field("heat")
heat.analysis_type = "steadystate"
heat.number_of_refinements = 2
heat.polynomial_order = 3
heat.linearity_type = "linear"
heat.weak_forms = "compiled"

heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})
heat.add_boundary("Inlet", "heat_heat_flux", {"heat_heat_flux" : 500000, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0})
heat.add_boundary("Temperature", "heat_temperature", {"heat_temperature" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 20, "heat_convection_external_temperature" : 20})

heat.add_material("Material", {"heat_conductivity" : 52, "heat_volume_heat" : 6e6})   
             
# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0.02, 0, 0.1, 0, boundaries = {"heat" : "Temperature"})
geometry.add_edge(0.1, 0, 0.1, 0.14, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.1, 0.14, 0.02, 0.14, boundaries = {"heat" : "Temperature"})
geometry.add_edge(0.02, 0.14, 0.02, 0.1, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.02, 0.04, 0.02, 0, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.02, 0.04, 0.02, 0.1, boundaries = {"heat" : "Inlet"})


# labels
geometry.add_label(0.0460134, 0.0867717, materials = {"heat" : "Material"}, area=0.0003)

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.062926, 0.038129)
testT = agros2d.test("Temperature", point["T"], 263.811616)
testG = agros2d.test("Gradient", point["G"], 4615.703047)
testGr = agros2d.test("Gradient - r", point["Gr"], 808.030874788)
testGz = agros2d.test("Gradient - z", point["Gz"], -4544.425)
testF = agros2d.test("Heat flux", point["F"], 2.400166e5)
testFr = agros2d.test("Heat flux - r", point["Fr"], 42017.605489)
testFz = agros2d.test("Heat flux - z", point["Fz"], -2.363101e5)

# volume integral
volume = heat.volume_integrals([0])
testTavg = agros2d.test("Temperature", volume["T"], 0.949673)

# surface integral
surface = heat.surface_integrals([1])
testFlux = agros2d.test("Heat flux", surface["f"], 333.504915)

print("Test: Heat transfer steady state - axisymmetric: " + str(testT and testG and testGr and testGz and testF and testFr and testFz and testTavg and testFlux))