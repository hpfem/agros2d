import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.name = "nepojmenovaný"
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1e+09

# fields
# rf
rf = agros2d.field("rf")
rf.analysis_type = "harmonic"
rf.number_of_refinements = 5
rf.polynomial_order = 2
rf.adaptivity_type = "disabled"
rf.linearity_type = "linear"


# boundaries
rf.add_boundary("PEC", "rf_te_electric_field", {"rf_te_electric_field_real" : 0, "rf_te_electric_field_imag" : 0})
rf.add_boundary("electric field", "rf_te_electric_field", {"rf_te_electric_field_real" : 1, "rf_te_electric_field_imag" : 0})
rf.add_boundary("osa", "rf_te_magnetic_field", {"rf_te_magnetic_field_real" : 0, "rf_te_magnetic_field_imag" : 0})


# materials
rf.add_material("air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 0, "rf_te_current_density_external_real" : 0, "rf_te_current_density_external_imag" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, 1, 0, -1, boundaries = {"rf" : "osa"})
geometry.add_edge(0, -1, 1, -1, boundaries = {"rf" : "osa"})
geometry.add_edge(1, -1, 1, -0.75, boundaries = {"rf" : "electric field"})
geometry.add_edge(1, -0.75, 1, 0.75, boundaries = {"rf" : "electric field"})
geometry.add_edge(1, 0.75, 1, 1, boundaries = {"rf" : "electric field"})
geometry.add_edge(1, 1, 0, 1, boundaries = {"rf" : "osa"})

geometry.add_label(0.399371, 0.440347, materials = {"rf" : "air"})
geometry.zoom_best_fit()

problem.solve()

point = rf.local_values(0.15705, 0.68773)

testEr = agros2d.test("Electric field - real", point["Er"], 1.71298)

print("Test: RF - axisymmetric: " + str(testEr))