import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1e10


# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# rf_tm
rf_tm = a2d.field("rf_tm")
rf_tm.analysis_type = "harmonic"
rf_tm.number_of_refinements = 4
rf_tm.polynomial_order = 3
rf_tm.adaptivity_type = "disabled"
rf_tm.linearity_type = "linear"


# boundaries
rf_tm.add_boundary("Perfect electric conductor", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})
rf_tm.add_boundary("Matched boundary", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})
rf_tm.add_boundary("Surface current", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 1, "rf_tm_magnetic_field_imag" : 1})


# materials
rf_tm.add_material("Air", {"rf_tm_permittivity" : 1, "rf_tm_permeability" : 1, "rf_tm_conductivity" : 0})

# geometry
geometry = a2d.geometry
geometry.add_edge(-0.01, 0.02286, -0.01, 0, boundaries = {"rf_tm" : "Surface current"})
geometry.add_edge(0.06907, 0.02286, 0.076, 0.01593, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.076, 0.01593, 0.081, 0.01593, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.01593, 0.081, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.086, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.081, 0.00693, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.00693, 0.05907, 0, angle = 20, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(-0.01, 0, 0.05907, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.17, 0.02286, 0.081, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.06907, 0.02286, -0.01, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.17, 0.02286, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.19286, -0.04, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.175, -0.015, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.015, 0.16, 0, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.19286, -0.04, boundaries = {"rf_tm" : "Matched boundary"})
geometry.add_edge(0.086, 0, 0.16, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})

geometry.add_label(0.0359418, 0.0109393, materials = {"rf_tm" : "Air"})
a2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = rf_tm.local_values(0.019107, 0.016725)
testH = agros2d.test("Magnetic field", point["H"], 3.23)
testH_real = agros2d.test("Magnetic field - real", point["Hr"], 2.28)
testH_imag = agros2d.test("Magnetic field - imag", point["Hi"], 2.29)
testB = agros2d.test("Magnetic flux density", point["B"], 4.061379e-6)
testB_real = agros2d.test("Magnetic flux density - real", point["Br"], 2.868038e-6)
testB_imag = agros2d.test("Magnetic flux density - imag", point["Bi"], 2.875614e-6)
testE = agros2d.test("Electric field", point["E"], 853.416645103)
testEx_real = agros2d.test("Electric field - x - real", point["Erx"], -503.757373)
testEx_imag = agros2d.test("Electric field - x - imag", point["Eix"],  502.430138)
testEy_real = agros2d.test("Electric field - y - real", point["Ery"], -341.713906)
testEy_imag = agros2d.test("Electric field - y - imag", point["Eiy"], 341.713906)
testD = agros2d.test("Electric displacement", point["D"], 7.615992e-9)
testDx_real = agros2d.test("Electric displacement - x - real", point["Drx"],-4.460362e-9)
testDx_imag = agros2d.test("Electric displacement - x - imag", point["Dix"],4.448611e-9)
testDy_real = agros2d.test("Electric displacement - y - real", point["Dry"], 3.025599e-9)
testDy_imag = agros2d.test("Electric displacement - y - imag", point["Diy"], -3.027227e-9)
#testNx = agros2d.test("Poynting vector - x", point["Nx"], 156.567066)
#testNy = agros2d.test("Poynting vector - y", point["Ny"], -3.138616, 1)


print("Test: RF TM - planar: " + str(testH and testH_real and testH_imag
and testE and testEx_real and testEx_imag and testEy_real and testEy_imag and
testD and testDx_real and testDx_imag and testDy_real and testDy_imag ))
# and testNx and testNy))