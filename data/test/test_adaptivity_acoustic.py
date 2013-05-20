import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 5000

# fields
# acoustic
acoustic = agros2d.field("acoustic")
acoustic.analysis_type = "harmonic"
acoustic.polynomial_order = 1
acoustic.adaptivity_type = "hp-adaptivity"
acoustic.adaptivity_steps = 15
acoustic.adaptivity_tolerance = 10
acoustic.linearity_type = "linear"


# boundaries
acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0, "acoustic_normal_acceleration_imag" : 0})
acoustic.add_boundary("Source", "acoustic_pressure", {"acoustic_pressure_real" : 1, "acoustic_pressure_imag" : 0})
acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : { "expression" : "1.25*343" }})


# materials
acoustic.add_material("Air", {"acoustic_density" : 1.25, "acoustic_speed" : 343})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0.3, 0, 0, 0.3, angle = 90, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.1, 0, 0.3, 0, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.1, 0, 0.025, -0.175, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.025, -0.175, 0.025, -0.2, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.025, -0.2, 0, -0.2, boundaries = {"acoustic" : "Source"})
geometry.add_edge(0, 0.3, 0, -0.2, boundaries = {"acoustic" : "Wall"})

geometry.add_label(0.109723, 0.176647, materials = {"acoustic" : "Air"})
agros2d.view.zoom_best_fit()

problem.solve()

# exact values in this test are taken from Agros -> not a proper test
# only to see if adaptivity works, should be replaced with comsol values
point1 = acoustic.local_values(7.544e-3, -0.145)
test1 = agros2d.test("Acoustic pressure", point1["p"], 7.481e-1)
point2 = acoustic.local_values(6.994e-2, 1.894e-2)
test2 = agros2d.test("Acoustic pressure", point2["p"], 2.889e-1)
print("Test: Adaptivity - acoustic: " + str(test1 and test2))