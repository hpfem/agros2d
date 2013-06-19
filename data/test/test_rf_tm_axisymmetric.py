import agros2d as a2d
from time import time
start_time = time()

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

problem.frequency = 1e+09

# disable view
a2d.view.mesh.disable()
a2d.view.post2d.disable()

# rf_tm
rf_tm = a2d.field("rf_tm")
rf_tm.analysis_type = "harmonic"
rf_tm.number_of_refinements = 4
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

testH = a2d.test("Magnetic field", point["H"], 0.53836)
testHr = a2d.test("Magnetic field - real", point["Hr"], 0.538097)
testHi = a2d.test("Magnetic field - imag", point["Hi"], 0.016823)

testB = a2d.test("Magnetic flux density", point["B"], 6.765228e-7)
testBr = a2d.test("Magnetic flux density - real", point["Br"], 6.761924e-7)
testBi = a2d.test("Magnetic flux density - imag", point["Bi"], 2.114032e-8)

testE = a2d.test("Electric field", point["E"], 274.023386)
testEr_real = a2d.test("Electric field r component - real", point["Err"], 122.830044)
testEr_imag = a2d.test("Electric field r component - imag", point["Eir"], 221.565147)
testEz_real = a2d.test("Electric field z component - real", point["Erz"], -103.385085)
testEz_imag = a2d.test("Electric field z component - imag", point["Eiz"], -14.899843)

testD = a2d.test("Electric displacement", point["D"], 2.4447e-9)
testDr_real = a2d.test("Electric displacement r component - real", point["Drr"], 1.08756e-9)
testDr_imag = a2d.test("Electric displacement r component - imag", point["Dir"], 1.961779e-9)
testDz_real = a2d.test("Electric displacement z component - real", point["Drz"],-9.15391e-10)
testDz_imag = a2d.test("Electric displacement z component - imag", point["Diz"], -1.31926e-10)

testNr = a2d.test("Poynting vector r component", point["Nr"], 27.940976)
testNz = a2d.test("Poynting vector z component", point["Nz"], 34.895617)

elapsed_time = time() - start_time
print("Test: RF TM - axisymmetric ({0}): ".format(round(elapsed_time, 3)) + str(testH and testHr and testHi and testB
and testBr and testBi and testE and testEr_real and testEr_imag and testEz_real and testEz_imag
and testD and testDr_real and testDr_imag and testDz_real and testDz_imag and testNr and testNz))