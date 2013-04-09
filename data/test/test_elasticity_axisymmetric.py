import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
elasticity = agros2d.field("elasticity")
elasticity.analysis_type = "steadystate"
elasticity.number_of_refinements = 2
elasticity.polynomial_order = 3
elasticity.linearity_type = "linear"

elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})
elasticity.add_boundary("Load", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : -10000})

elasticity.add_material("Material 1", {"elasticity_young_modulus" : 2e+11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_alpha" : 1e-7, "elasticity_temperature_difference" :  0})
elasticity.add_material("Material 2", {"elasticity_young_modulus" : 1e+11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 30000, "elasticity_alpha" : 1.2e-5, "elasticity_temperature_difference" :  0.5})
elasticity.add_material("Material 3", {"elasticity_young_modulus" : 1e+11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_alpha" : 2e-7, "elasticity_temperature_difference" : 0})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(1.4, 0.2, 1.6, 0.2, boundaries = {"elasticity" : "Load"})
geometry.add_edge(1.6, 0.2, 1.6, 0, boundaries = {"elasticity" : "Free"})
geometry.add_edge(1.6, 0, 1.4, 0, boundaries = {"elasticity" : "Free"})
geometry.add_edge(0, 0.2, 0, 0, boundaries = {"elasticity" : "Fixed"})
geometry.add_edge(1.4, 0.2, 1.4, 0)
geometry.add_edge(0, 0.2, 0.6, 0.2, boundaries = {"elasticity" : "Free"})
geometry.add_edge(0.6, 0.2, 0.6, 0)
geometry.add_edge(0, 0, 0.6, 0, boundaries = {"elasticity" : "Fixed"})
geometry.add_edge(0.6, 0, 1.4, 0, boundaries = {"elasticity" : "Free"})
geometry.add_edge(1.4, 0.2, 0.6, 0.2, boundaries = {"elasticity" : "Free"})

# labels
geometry.add_label(0.0823077, 0.11114, materials = {"elasticity" : "Material 1"})
geometry.add_label(1.48989, 0.108829, materials = {"elasticity" : "Material 3"})
geometry.add_label(1.20588, 0.108829, materials = {"elasticity" : "Material 2"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = elasticity.local_values(1.369034, 0.04259)
#testVonMises = agros2d.test("Von Mises stress", point["mises"], 1.69779e5)
# testTresca = agros2d.test("Tresca stress", point["tresca"], 1.235475e5)
testd = agros2d.test("Displacement", point["d"], 6.154748e-6)
testdr = agros2d.test("Displacement - x", point["dr"], 5.544176e-6)
testdz = agros2d.test("Displacement - y", point["dz"], -2.672647e-6)
#testsrr = agros2d.test("Stress RR", point["sxx"], -1.132081e5)
#testszz = agros2d.test("Stress ZZ", point["syy"], -1.210277e5)
#testsaa = agros2d.test("Stress aa", point["szz"], -2.7248e5)
#testtxy = agros2d.test("Stress RZ", point["sxy"], 39372.872587)
#testerr = agros2d.test("Strain RR", point["exx"], 6.165992e-6)
#testezz = agros2d.test("Strain ZZ", point["eyy"], 6.061992e-6)
#testeaa = agros2d.test("Strain aa", point["ezz"], 4.049699e-6)
#testerz = agros2d.test("Strain RZ", point["exy"], 5.236592e-7)

# surface integral
# surface = elasticity.surface_integrals([0])
# testI = agros2d.test("Current", surface["I"], 3629.425713)

#print("Test: Structural mechanics - axisymmetric: " + str(testVonMises and testu and testv and testD and testsrr and testszz and testsaa and testerr and testezz and testeaa and testerz))
print("Test: Structural mechanics - axisymmetric: " + str(testd and testdr and testdz))