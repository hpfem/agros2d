import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
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
elasticity.number_of_refinements = 3
elasticity.polynomial_order = 3
elasticity.linearity_type = "linear"

elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})
elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
elasticity.add_boundary("Load", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : -1.2e4})

elasticity.add_material("Material 1", {"elasticity_young_modulus" : 2e11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : -1e6, "elasticity_volume_force_y" : 0, "elasticity_alpha" : 1e-6, "elasticity_temperature_difference" : 10, "elasticity_temperature_reference" : 20})
elasticity.add_material("Material 2", {"elasticity_young_modulus" : 1e11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 3e4, "elasticity_alpha" : 1e-6, "elasticity_temperature_difference" : 10, "elasticity_temperature_reference" : 20})
elasticity.add_material("Material 3", {"elasticity_young_modulus" : 1e11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : 0, "elasticity_alpha" : 1e-6, "elasticity_temperature_difference" : 10, "elasticity_temperature_reference" : 20})

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
geometry.add_edge(0, 0, 0.6, 0, boundaries = {"elasticity" : "Free"})
geometry.add_edge(0.6, 0, 1.4, 0, boundaries = {"elasticity" : "Free"})
geometry.add_edge(1.4, 0.2, 0.6, 0.2, boundaries = {"elasticity" : "Free"})


# labels
geometry.add_label(0.0823077, 0.11114, materials = {"elasticity" : "Material 1"})
geometry.add_label(1.48989, 0.108829, materials = {"elasticity" : "Material 3"})
geometry.add_label(1.20588, 0.108829, materials = {"elasticity" : "Material 2"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = elasticity.local_values(1.327264, 0.041087)
# testVonMises = agros2d.test("Von Mises stress", point["mises"], 1.175226e5)
# testTresca = agros2d.test("Tresca stress", point["tresca"], 1.344939e5)
testd = agros2d.test("Displacement", point["d"], 1.682194e-5)
testdx = agros2d.test("Displacement - x", point["dx"], 1.681777e-5)
testdy = agros2d.test("Displacement - y", point["dy"], -3.751169e-7, 0.1)
# testsxx = agros2d.test("Stress XX", point["sxx"], 50551.83149)
# testsyy = agros2d.test("Stress YY", point["syy"], 613.931457)
# testszz = agros2d.test("Stress ZZ", point["szz"], -83115.298227)
# testsxy = agros2d.test("Stress XY", point["sxy"], -6478.61142)
# testexx = agros2d.test("Strain XX", point["exx"], 1.777773e-6)
# testeyy = eagros2d.test("Strain YY", point["eyy"], 1.113599e-6)
# testexy = agros2d.test("Strain XY", point["exy"], -8.616553e-8)

# surface integral
# surface = elasticity.surface_integrals([0])
# testI = agros2d.test("Current", surface["I"], 3629.425713)

# print("Test: Structural mechanics - planar: " + str(testVonMises and testTresca and testu and testv and testD and testsxx and testsyy and testszz and testsxy and testexx and testeyy and testexy))
print("Test: Structural mechanics - planar: " + str(testd and testdx and testdy))