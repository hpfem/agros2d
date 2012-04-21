import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "elasticity"
problem.name = "unnamed"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
elasticity = agros2d.field("elasticity")
elasticity.analysis_type = "steadystate"
elasticity.number_of_refinements = 2
elasticity.polynomial_order = 3
elasticity.linearity_type = "linear"
elasticity.weak_forms = "compiled"
elasticity.nonlineartolerance = 0.001
elasticity.nonlinearsteps = 10

elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"dx" : 0, "dy" : 0})
elasticity.add_boundary("Free", "elasticity_free_free", {"fx" : 0, "fy" : 0})
elasticity.add_boundary("Load", "elasticity_free_free", {"fx" : 0, "fy" : -10000})

elasticity.add_material("Material 1", {"E" : 2e+11, "nu" : 0.33, "fx" : 0, "fy" : 0, "alpha" : 1e-7, "T" :  30, "Tr" : 30})
elasticity.add_material("Material 2", {"E" : 1e+11, "nu" : 0.33, "fx" : 0, "fy" : 30000, "alpha" : 1.2e-5, "T" :  20.5, "Tr" : 20})
elasticity.add_material("Material 3", {"E" : 1e+11, "nu" : 0.33, "fx" : 0, "fy" : 0, "alpha" : 2e-7, "T" : 30, "Tr" : 30})

# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(1.4, 0.2, 1.6, 0.2, boundaries = {"Load"})
geometry.add_edge(1.6, 0.2, 1.6, 0, boundaries = {"Free"})
geometry.add_edge(1.6, 0, 1.4, 0, boundaries = {"Free"})
geometry.add_edge(0, 0.2, 0, 0, boundaries = {"Fixed"})
geometry.add_edge(1.4, 0.2, 1.4, 0,)
geometry.add_edge(0, 0.2, 0.6, 0.2, boundaries = {"Free"})
geometry.add_edge(0.6, 0.2, 0.6, 0,)
geometry.add_edge(0, 0, 0.6, 0, boundaries = {"Fixed"})
geometry.add_edge(0.6, 0, 1.4, 0, boundaries = {"Free"})
geometry.add_edge(1.4, 0.2, 0.6, 0.2, boundaries = {"Free"})

# labels
geometry.add_label(0.0823077, 0.11114, materials = {"Material 1"})
geometry.add_label(1.48989, 0.108829, materials = {"Material 3"})
geometry.add_label(1.20588, 0.108829, materials = {"Material 2"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = elasticity.local_values(1.369034, 0.04259)
#testVonMises = current.test("Von Mises stress", point["mises"], 1.69779e5)
# testTresca = current.test("Tresca stress", point["tresca"], 1.235475e5)
testd = elasticity.test("Displacement", point["d"], 6.154748e-6)
testdr = elasticity.test("Displacement - x", point["dr"], 5.544176e-6)
testdz = elasticity.test("Displacement - y", point["dz"], -2.672647e-6)
#testsrr = elasticity.test("Stress RR", point["sxx"], -1.132081e5)
#testszz = elasticity.test("Stress ZZ", point["syy"], -1.210277e5)
#testsaa = elasticity.test("Stress aa", point["szz"], -2.7248e5)
#testtxy = elasticity.test("Stress RZ", point["sxy"], 39372.872587)
#testerr = elasticity.test("Strain RR", point["exx"], 6.165992e-6)
#testezz = elasticity.test("Strain ZZ", point["eyy"], 6.061992e-6)
#testeaa = elasticity.test("Strain aa", point["ezz"], 4.049699e-6)
#testerz = elasticity.test("Strain RZ", point["exy"], 5.236592e-7)

# surface integral
# surface = elasticity.surface_integrals([0])
# testI = elasticity.test("Current", surface["I"], 3629.425713)

#print("Test: Structural mechanics - axisymmetric: " + str(testVonMises and testu and testv and testD and testsrr and testszz and testsaa and testerr and testezz and testeaa and testerz))
print("Test: Structural mechanics - axisymmetric: " + str(testd and testdr and testdz))