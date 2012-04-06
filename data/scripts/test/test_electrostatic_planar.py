import agros2d

# model
problem = agros2d.Problem(coordinate_type = "planar", name = "Test electrostatic field - planar")

electrostatics = agros2d.Field(problem, field_id = "electrostatic", analysis_type = "steadystate")
electrostatics.number_of_refinements = 2
electrostatics.polynomial_order = 3
electrostatics.nonlinear_tolerance = 0.001
electrostatics.nonlinear_steps = 10
	
# boundaries
electrostatics.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
electrostatics.add_boundary("U = 0 V", "electrostatic_potential", {"electrostatic_potential" : 0})
electrostatics.add_boundary("U = 1000 V", "electrostatic_potential", {"electrostatic_potential" : 1000})

# materials
electrostatics.add_material("Dieletric", {"electrostatic_permittivity" : 3})
electrostatics.add_material("Air", {"electrostatic_permittivity" : 1})
electrostatics.add_material("Source", {"electrostatic_permittivity" : 10, "electrostatic_charge_density" : 4e-10})

# geometry
geometry = agros2d.Geometry(problem)

# edges
geometry.add_edge(1, 2, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(4, 1, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(1, 2, 4, 2, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(4, 2, 4, 1, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(20, 24, 20, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(20, 1, 20, 0, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(4, 1, 20, 1)
geometry.add_edge(0, 24, 0, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 0, 0, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 0, 20, 0, boundaries = {"electrostatic" : "U = 0 V"})
geometry.add_edge(0, 24, 20, 24, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 1, 1, 1)
geometry.add_edge(7, 13, 14, 13)
geometry.add_edge(14, 13, 14, 18)
geometry.add_edge(14, 18, 7, 18)
geometry.add_edge(7, 18, 7, 13)

# labels
geometry.add_label(2.78257, 1.37346)
geometry.add_label(10.3839, 15.7187, materials = {"electrostatic" : "Source"})
geometry.add_label(3.37832, 15.8626, materials = {"electrostatic" : "Air"})
geometry.add_label(12.3992, 0.556005, materials = {"electrostatic" : "Dieletric"})

geometry.zoom_best_fit()

# solve problem
# problem.solve()
"""
# point value
point = pointresult(13.257584, 11.117738)
testV = test("Scalar potential", point["V"], 1111.544825)
testE = test("Electric field", point["E"], 111.954358)
testEx = test("Electric field - x", point["Ex"], 24.659054)
testEy = test("Electric field - y", point["Ey"], -109.204896)
testD = test("Displacement", point["D"], 9.912649e-10)
testDx = test("Displacement - x", point["Dx"], 2.183359e-10)
testDy = test("Displacement - y", point["Dy"], -9.669207e-10)
testwe = test("Energy density", point["we"], 5.548821e-8)

# volume integral
volume = volumeintegral([1])
testEnergy = test("Energy", volume["We"], 1.307484e-7)

# surface integral
surface = surfaceintegral([0, 1, 2, 3])
testQ = test("Electric charge", surface["Q"], 1.048981e-7)

print("Test: Electrostatic - planar: " + str(testV and testE and testEx and testEy and testD and testDx and testDy and testwe and testEnergy and testQ))
"""