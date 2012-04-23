import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.name = "Actuator"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.number_of_refinements = 1
heat.polynomial_order = 5
heat.linearity_type = "linear"
heat.weak_forms = "compiled"
heat.nonlineartolerance = 0.001
heat.nonlinearsteps = 10

#newdocument("", "", "", , , "disabled", 0, 0, 0, , 500, 10000, 20)

heat.add_boundary("Flux", "heat_heat_flux", 0, 0, 0)
heat.add_boundary("Convection", "heat_heat_flux", 0, 10, 20)

heat.add_material("Air", 0, 0.02, 1.2, 1000) 
heat.add_material("Cu", 26000, 200, 8700, 385) 
heat.add_material("Fe", 0, 60, 7800, 460) 
heat.add_material("Brass", 0, 100, 8400, 378) 

# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(0, 0.18, 0.035, 0.18, 0, boundaries = {"Convection"})
geometry.add_edge(0.035, 0.18, 0.035, 0.03, 0, boundaries = {"Convection"})
geometry.add_edge(0.035, 0.03, 0.0135, 0.03, 0, boundaries = {"Convection"})
geometry.add_edge(0.0135, 0.03, 0.0135, 0.038, 0,)
geometry.add_edge(0.0135, 0.038, 0.019, 0.038, 0,)
geometry.add_edge(0.027, 0.038, 0.027, 0.172, 0,)
geometry.add_edge(0.027, 0.172, 0.008, 0.172, 0,)
geometry.add_edge(0.008, 0.172, 0.008, 0.16, 0,)
geometry.add_edge(0.008, 0.16, 0, 0.16, 0,)
geometry.add_edge(0, 0.16, 0, 0.18, 0, boundaries = {"Flux"})
geometry.add_edge(0, 0.13, 0.008, 0.13, 0,)
geometry.add_edge(0.008, 0.13, 0.008, 0.0395, 0,)
geometry.add_edge(0.008, 0, 0, 0, 0, boundaries = {"Convection"})
geometry.add_edge(0, 0, 0, 0.13, 0, boundaries = {"Flux"})
geometry.add_edge(0.019, 0.038, 0.027, 0.038, 0,)
geometry.add_edge(0.019, 0.038, 0.019, 0.0395, 0,)
geometry.add_edge(0, 0.13, 0, 0.16, 0, boundaries = {"Flux"})
geometry.add_edge(0.01, 0.041, 0.01, 0.1705, 0,)
geometry.add_edge(0.025, 0.041, 0.01, 0.041, 0,)
geometry.add_edge(0.01, 0.1705, 0.025, 0.1705, 0,)
geometry.add_edge(0.025, 0.1705, 0.025, 0.041, 0,)
geometry.add_edge(0.008, 0.0395, 0.008, 0.03, 0,)
geometry.add_edge(0.008, 0.03, 0.008, 0, 0, boundaries = {"Convection"})
geometry.add_edge(0.019, 0.0395, 0.009, 0.0395, 0,)
geometry.add_edge(0.008, 0.03, 0.009, 0.03, 0, boundaries = {"Convection"})
geometry.add_edge(0.009, 0.0395, 0.009, 0.03, 0,)
geometry.add_edge(0.009, 0.03, 0.0135, 0.03, 0, boundaries = {"Convection"})

# labels
geometry.add_label(0.0308709, 0.171031, 0, 0, materials = {"Fe"})
geometry.add_label(0.00316251, 0.011224, 0, 0, materials = {"Fe"})
geometry.add_label(0.0163723, 0.144289, 0, 0, materials = {"Cu"})
geometry.add_label(0.00380689, 0.151055, 0, 0, materials = {"Air"})
geometry.add_label(0.0112064, 0.0336487, 0, 0, materials = {"Brass"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.00503, 0.134283)
testT = heat.test("Temperature", point["T"], 44.012004)
testF = heat.test("Heat flux", point["F"], 16.739787)
testFr = heat.test("Heat flux - r", point["Fx"], -7.697043)
testFz = heat.test("Heat flux - z", point["Fy"], -14.865261)

# volume integral
volume = heat.volume_integrals([3])
testTavg = heat.test("Average temperature", volume["T_avg"], 0.034505/8.365e-4)

# surface integral
surface = heat.surface_integrals([26])
testFlux = heat.test("Heat flux", surface["F"], 0.032866)

# print("Test: Heat transfer transient - axisymmetric: " + str(testT and testF and testFr and testFz and testTavg and testFlux))
print("Test: Heat transfer transient - axisymmetric: " + str(testT and testF and testFr and testFz))
