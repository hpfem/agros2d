import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "unnamed"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
elasticity = agros2d.field("elasticity")
elasticity.analysis_type = "steadystate"
elasticity.number_of_refinements = 3
elasticity.polynomial_order = 3
elasticity.linearity_type = "linear"
elasticity.weak_forms = "compiled"
elasticity.nonlineartolerance = 0.001
elasticity.nonlinearsteps = 10

elasticity.add_boundary("Free", "elasticity_free_free", {"fx" : 0, "fy" : 0})
elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"dx" : 0, "dy" : 0})
elasticity.add_boundary("Load", "elasticity_free_free", {"fx" : 0, "fy" : -1.2e4})

elasticity.add_material("Material 1", {"E" : 2e11, "nu" : 0.33, "fx" : -1e6, "fy" : 0, "alpha" : 1e-6, "T" : 30, "Tr" : 20})
elasticity.add_material("Material 2", {"E" : 1e11, "nu" : 0.33, "fx" : 0, "fy" : 3e4, "alpha" : 1e-6, "T" : 30, "Tr" : 20})
elasticity.add_material("Material 3", {"E" : 1e11, "nu" : 0.33, "fx" : 0, "fy" : 0, "alpha" : 1e-6, "T" : 30, "Tr" : 20})

# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(1.4, 0.2, 1.6, 0.2, boundaries = {"Load"})
geometry.add_edge(1.6, 0.2, 1.6, 0, boundaries = {"Free"})
geometry.add_edge(1.6, 0, 1.4, 0, boundaries = {"Free"})
geometry.add_edge(1.4, 0.2, 1.4, 0, boundaries = {"Fixed"})
geometry.add_edge(0, 0.2, 0.6, 0.2,)
geometry.add_edge(0.6, 0.2, 0.6, 0, boundaries = {"Free"})
geometry.add_edge(0, 0, 0.6, 0,)
geometry.add_edge(0.6, 0, 1.4, 0, boundaries = {"Free"})
geometry.add_edge(1.4, 0.2, 0.6, 0.2, boundaries = {"Free"})
geometry.add_edge(0, 0, 0, 0, boundaries = {"Free"})

# labels
geometry.add_label(0.0823077, 0.11114, materials = {"Material 1"})
geometry.add_label(1.48989, 0.108829, materials = {"Material 3"})
geometry.add_label(1.20588, 0.108829, materials = {"Material 2"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = elasticity.local_values(1.327264, 0.041087)
# testVonMises = elasticity.test("Von Mises stress", point["mises"], 1.175226e5)
# testTresca = elasticity.test("Tresca stress", point["tresca"], 1.344939e5)
testd = elasticity.test("Displacement", point["d"], 1.682194e-5)
testdx = elasticity.test("Displacement - x", point["dx"], 1.681777e-5)
testdy = elasticity.test("Displacement - y", point["dy"], -3.751169e-7, 0.1)
# testsxx = elasticity.test("Stress XX", point["sxx"], 50551.83149)
# testsyy = elasticity.test("Stress YY", point["syy"], 613.931457)
# testszz = elasticity.test("Stress ZZ", point["szz"], -83115.298227)
# testsxy = elasticity.test("Stress XY", point["sxy"], -6478.61142)
# testexx = elasticity.test("Strain XX", point["exx"], 1.777773e-6)
# testeyy = elasticity.test("Strain YY", point["eyy"], 1.113599e-6)
# testexy = elasticity.test("Strain XY", point["exy"], -8.616553e-8)

# surface integral
# surface = elasticity.surface_integrals([0])
# testI = elasticity.test("Current", surface["I"], 3629.425713)

# print("Test: Structural mechanics - planar: " + str(testVonMises and testTresca and testu and testv and testD and testsxx and testsyy and testszz and testsxy and testexx and testeyy and testexy))
print("Test: Structural mechanics - planar: " + str(testd and testdx and testdy))