import agros2d
from time import time
start_time = time()

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

problem.frequency = 1e+09

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
# rf
rf = agros2d.field("rf_te")
rf.analysis_type = "harmonic"
rf.number_of_refinements = 3
rf.polynomial_order = 3
rf.adaptivity_type = "disabled"
rf.linearity_type = "linear"


# boundaries
rf.add_boundary("PEC", "rf_te_electric_field", {"rf_te_electric_field_real" : 0, "rf_te_electric_field_imag" : 0})
rf.add_boundary("Source", "rf_te_electric_field", {"rf_te_electric_field_real" : 1, "rf_te_electric_field_imag" : 0})
rf.add_boundary("Impedance", "rf_te_impedance", {"rf_te_impedance" : 377})
rf.add_boundary("PMC", "rf_te_magnetic_field", {"rf_te_magnetic_field_real" : 0, "rf_te_magnetic_field_imag" : 0})


# materials
rf.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 0, "rf_te_current_density_external_real" : 0, "rf_te_current_density_external_imag" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, 1, 0, -1, boundaries = {"rf_te" : "PEC"})
geometry.add_edge(0, -1, 1, -1, boundaries = {"rf_te" : "Impedance"})
geometry.add_edge(1, -1, 1, -0.75, boundaries = {"rf_te" : "Impedance"})
geometry.add_edge(1, -0.75, 1.7, 0.65, boundaries = {"rf_te" : "Impedance"})
geometry.add_edge(1.7, 0.65, 0.9, 0.35, boundaries = {"rf_te" : "Impedance"})
geometry.add_edge(0.9, 0.35, 0, 1, boundaries = {"rf_te" : "Impedance"})
geometry.add_edge(0.5, -0.25, 0.25, -0.5, angle = 90, boundaries = {"rf_te" : "Source"})
geometry.add_edge(0.25, -0.5, 0.5, -0.75, angle = 90, boundaries = {"rf_te" : "Source"})
geometry.add_edge(0.5, -0.75, 0.75, -0.5, angle = 90, boundaries = {"rf_te" : "Source"})
geometry.add_edge(0.75, -0.5, 0.5, -0.25, angle = 90, boundaries = {"rf_te" : "Source"})
geometry.add_edge(0.4, 0.25, 0.3, -0.05, boundaries = {"rf_te" : "PMC"})
geometry.add_edge(0.4, 0.25, 0.75, 0.2, boundaries = {"rf_te" : "PMC"})
geometry.add_edge(0.75, 0.2, 0.85, -0.1, boundaries = {"rf_te" : "PMC"})
geometry.add_edge(0.3, -0.05, 0.85, -0.1, boundaries = {"rf_te" : "PMC"})

geometry.add_label(0.399371, 0.440347, materials = {"rf_te" : "Air"})
geometry.add_label(0.484795, -0.434246, materials = {"rf_te" : "none"})
geometry.add_label(0.57193, 0.0710058, materials = {"rf_te" : "none"})
agros2d.view.zoom_best_fit()

problem.solve()

point = rf.local_values(0.92463, -0.20118)

testE = agros2d.test("Electric field", point["E"], 0.5385)
testEr = agros2d.test("Electric field - real", point["Er"], 0.53821)
testEi = agros2d.test("Electric field - imag", point["Ei"], 0.01767)

testD = agros2d.test("Displacement", point["D"], 4.768e-12)
testDr = agros2d.test("Displacement - real", point["Dr"], 4.7654e-12)
testDi = agros2d.test("Displacement - imag", point["Di"], 1.56463e-13)

testH = agros2d.test("Magnetic field", point["H"], 0.00195)
testHr_real = agros2d.test("Magnetic field r component - real", point["Hrr"], -8.69388e-4)
testHr_imag = agros2d.test("Magnetic field r component - imag", point["Hir"], -0.00157)
testHz_real = agros2d.test("Magnetic field z component - real", point["Hrz"], 7.3442e-4)
testHz_imag = agros2d.test("Magnetic field z component - imag", point["Hiz"], 1.06797e-4)

testB = agros2d.test("Magnetic flux density", point["B"], 2.4447e-9)
testBr_real = agros2d.test("Magnetic flux density r component - real", point["Brr"], -1.09251e-9)
testBr_imag = agros2d.test("Magnetic flux density r component - imag", point["Bir"], -1.9781e-9)
testBz_real = agros2d.test("Magnetic flux density z component - real", point["Brz"], 9.2291e-10)
testBz_imag = agros2d.test("Magnetic flux density z component - imag", point["Biz"], 1.34204e-10)

testNr = agros2d.test("Poynting vector r component", point["Nr"], 1.98583e-4)
testNz = agros2d.test("Poynting vector z component", point["Nz"], 2.47866e-4)

elapsed_time = time() - start_time
print("Test: RF TE - axisymmetric ({0}): ".format(round(elapsed_time, 3)) + str(testE and testEr and testEi and testD and 
  testDr and testDi and testH and testHr_real and testHr_imag and testHz_real and testHz_imag
  and testB and testBr_real and testBr_imag and testBz_real and testBz_imag
  and testNr and testNz))
