import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1e+09

# fields
# rf_tm
rf_tm = a2d.field("rf_tm")
rf_tm.analysis_type = "harmonic"
rf_tm.number_of_refinements = 3
rf_tm.polynomial_order = 3
rf_tm.adaptivity_type = "disabled"
rf_tm.linearity_type = "linear"


# boundaries
rf_tm.add_boundary("PEC", "rf_tm_electric_field", {"rf_tm_electric_field_real" : 0, "rf_tm_electric_field_imag" : 0})
rf_tm.add_boundary("Source", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 1, "rf_tm_magnetic_field_imag" : 0})
rf_tm.add_boundary("Impedance", "rf_tm_impedance", {"rf_tm_impedance" : 377})
rf_tm.add_boundary("PMC", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})


# materials
rf_tm.add_material("Air", {"rf_tm_permittivity" : 1, "rf_tm_permeability" : 1, "rf_tm_conductivity" : 0})

# geometry
geometry = a2d.geometry
geometry.add_edge(0, 1, 0, -1, boundaries = {"rf_tm" : "PMC"})
geometry.add_edge(0, -1, 1, -1, boundaries = {"rf_tm" : "Impedance"})
geometry.add_edge(1, -1, 1, -0.75, boundaries = {"rf_tm" : "Impedance"})
geometry.add_edge(1, -0.75, 1.7, 0.65, boundaries = {"rf_tm" : "Impedance"})
geometry.add_edge(1.7, 0.65, 0.9, 0.35, boundaries = {"rf_tm" : "Impedance"})
geometry.add_edge(0.9, 0.35, 0, 1, boundaries = {"rf_tm" : "Impedance"})
geometry.add_edge(0.5, -0.25, 0.25, -0.5, angle = 90, boundaries = {"rf_tm" : "Source"})
geometry.add_edge(0.25, -0.5, 0.5, -0.75, angle = 90, boundaries = {"rf_tm" : "Source"})
geometry.add_edge(0.5, -0.75, 0.75, -0.5, angle = 90, boundaries = {"rf_tm" : "Source"})
geometry.add_edge(0.75, -0.5, 0.5, -0.25, angle = 90, boundaries = {"rf_tm" : "Source"})
geometry.add_edge(0.4, 0.25, 0.3, -0.05, boundaries = {"rf_tm" : "PEC"})
geometry.add_edge(0.4, 0.25, 0.75, 0.2, boundaries = {"rf_tm" : "PEC"})
geometry.add_edge(0.75, 0.2, 0.85, -0.1, boundaries = {"rf_tm" : "PEC"})
geometry.add_edge(0.3, -0.05, 0.85, -0.1, boundaries = {"rf_tm" : "PEC"})

geometry.add_label(0.399371, 0.440347, materials = {"rf_tm" : "Air"})
geometry.add_label(0.484795, -0.434246, materials = {"rf_tm" : "none"})
geometry.add_label(0.57193, 0.0710058, materials = {"rf_tm" : "none"})
a2d.view.zoom_best_fit()

problem.solve()

#point = rf_tm.local_values(0.92463, -0.20118)
#
#testE = agros2d.test("Electric field", point["E"], 0.5385)
#testEr = agros2d.test("Electric field - real", point["Er"], 0.53821)
#testEi = agros2d.test("Electric field - imag", point["Ei"], 0.01767)
#
#testD = agros2d.test("Displacement", point["D"], 4.768e-12)
#testDr = agros2d.test("Displacement - real", point["Dr"], 4.7654e-12)
#testDi = agros2d.test("Displacement - imag", point["Di"], 1.56463e-13)
#
#testH = agros2d.test("Magnetic field", point["H"], 0.00195)
#testHr_real = agros2d.test("Magnetic field r component - real", point["Hrr"], -8.69388e-4)
#testHr_imag = agros2d.test("Magnetic field r component - imag", point["Hir"], -0.00157)
#testHz_real = agros2d.test("Magnetic field z component - real", point["Hrz"], 7.3442e-4)
#testHz_imag = agros2d.test("Magnetic field z component - imag", point["Hiz"], 1.06797e-4)
#
#testB = agros2d.test("Magnetic flux density", point["B"], 2.4447e-9)
#testBr_real = agros2d.test("Magnetic flux density r component - real", point["Brr"], -1.09251e-9)
#testBr_imag = agros2d.test("Magnetic flux density r component - imag", point["Bir"], -1.9781e-9)
#testBz_real = agros2d.test("Magnetic flux density z component - real", point["Brz"], 9.2291e-10)
#testBz_imag = agros2d.test("Magnetic flux density z component - imag", point["Biz"], 1.34204e-10)
#
#testNr = agros2d.test("Poynting vector r component", point["Nr"], 1.98583e-4)
#testNz = agros2d.test("Poynting vector z component", point["Nz"], 2.47866e-4)
#	
#print("Test: RF TM - axisymmetric: " + str(testE and testEr and testEi and testD and 
#  testDr and testDi and testH and testHr_real and testHr_imag and testHz_real and testHz_imag
#  and testB and testBr_real and testBr_imag and testBz_real and testBz_imag
#  and testNr and testNz))
