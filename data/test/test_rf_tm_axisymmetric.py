import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1e+09

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

point = rf_tm.local_values(0.92463, -0.20118)

testH = agros2d.test("Magnetic field", point["H"], 0.5385)
testHr = agros2d.test("Magnetic field - real", point["Hr"], 0.53821)
testHi = agros2d.test("Magnetic field - imag", point["Hi"], 0.01767)
#
testB = agros2d.test("Magnetic flux density", point["B"], 6.77389282698e-07)
testBr = agros2d.test("Magnetic flux density - real", point["Br"], 6.77026796583e-07)
testBi = agros2d.test("Magnetic flux density - imag", point["Bi"], 2.21575499861e-08)
#
testE = agros2d.test("Electric field", point["E"], 274.025052067)
testEr_real = agros2d.test("Electric field r component - real", point["Err"], 122.522812609)
testEr_imag = agros2d.test("Electric field r component - imag", point["Eir"], -221.632582129)
testEz_real = agros2d.test("Electric field z component - real", point["Erz"], -103.578888534)
testEz_imag = agros2d.test("Electric field z component - imag", point["Eiz"], 15.1096637854)
#
testD = agros2d.test("Electric displacement", point["D"], 2.4447e-9)
testDr_real = agros2d.test("Electric displacement r component - real", point["Drr"], 1.08481698284e-09)
testDr_imag = agros2d.test("Electric displacement r component - imag", point["Dir"], -1.96233488217e-09)
testDz_real = agros2d.test("Electric displacement z component - real", point["Drz"],-9.17087479076e-10)
testDz_imag = agros2d.test("Electric displacement z component - imag", point["Diz"], 1.33780963155e-10)
#
#testNr = agros2d.test("Poynting vector r component", point["Nr"], 1.98583e-4)
#testNz = agros2d.test("Poynting vector z component", point["Nz"], 2.47866e-4)
#	
print("Test: RF TM - axisymmetric: " + str(testH and testHr and testHi and testB and 
  testBr and testBi and testE and testEr_real and testEr_imag and testEz_real and testEz_imag
  and testD and testDr_real and testDr_imag and testDz_real and testDz_imag))
#  and testNr and testNz))
