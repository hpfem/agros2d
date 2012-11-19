import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# disable view
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
heat = agros2d.field("heat")
heat.analysis_type = "steadystate"
heat.number_of_refinements = 2
heat.polynomial_order = 2
heat.linearity_type = "newton"
#heat.linearity_type = "picard"
heat.nonlinear_tolerance = 0.001
heat.nonlinear_steps = 10
heat.damping_coeff = 1.0
heat.automatic_damping = True
heat.damping_number_to_increase = 1

heat.add_boundary("Left", "heat_temperature", {"heat_temperature" : 10})
heat.add_boundary("Radiace", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 20, "heat_radiation_emissivity" : 0.9})
heat.add_boundary("Neumann", "heat_heat_flux", {"heat_convection_external_temperature" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature" : 20, "heat_convection_heat_transfer_coefficient" : 50, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0, "heat_radiation_emissivity" : 0})

heat.add_material("Material - nonlin", {"heat_conductivity" : { "x" : [0,100,200,290,500,1000], "y" : [210,280,380,430,310,190] }, "heat_volume_heat" : 2e6})
heat.add_material("Material", {"heat_conductivity" : 230, "heat_volume_heat" : 0})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(-0.25, 0.25, -0.1, 0.1, boundaries = {"heat" : "Convection"})
geometry.add_edge(-0.1, 0.1, 0.05, 0.2, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.05, 0.2, 0.25, 0.25, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.25, 0.25, 0.1, 0.1, boundaries = {"heat" : "Radiace"})
geometry.add_edge(0.1, 0.1, 0.25, -0.25, boundaries = {"heat" : "Radiace"})
geometry.add_edge(0.25, -0.25, 0.05, -0.05, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.05, -0.05, -0.25, -0.25, boundaries = {"heat" : "Neumann"}, angle = 90)
geometry.add_edge(-0.25, -0.05, 0.1, 0.1, boundaries = {})
geometry.add_edge(-0.25, 0.25, -0.25, -0.05, boundaries = {"heat" : "Left"})
geometry.add_edge(-0.25, -0.05, -0.25, -0.25, boundaries = {"heat" : "Left"})

# labels
geometry.add_label(-0.0150215, 0.018161, materials = {"heat" : "Material"})
geometry.add_label(-0.183934, 0.0732177, materials = {"heat" : "Material - nonlin"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(8.620e-02, 1.620e-01)
testT = agros2d.test("Temperature", point["T"], 357.17654)
testG = agros2d.test("Gradient", point["G"], 444.435957)
testF = agros2d.test("Heat flux", point["F"], 1.805517e5)

# volume integral
volume = heat.volume_integrals([1])
testTavg = agros2d.test("Temperature", volume["T"], 12.221687)

# surface integral
surface = heat.surface_integrals([8])
testFlux = agros2d.test("Heat flux", surface["f"], 96464.56418)

print("Test: Heat transfer steady state - nonlin - planar: " + str(testT and testG and testF and testTavg and testFlux))