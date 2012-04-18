import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "Acoustic"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
acoustic = agros2d.field("acoustic")
acoustic.analysis_type = "harmonic"
acoustic.number_of_refinements = 3
acoustic.polynomial_order = 2
acoustic.linearity_type = "linear"
acoustic.weak_forms = "compiled"
acoustic.nonlineartolerance = 0.001
acoustic.nonlinearsteps = 10
acoustic.frequency = 2000


acoustic.add_boundary("Source", "acoustic_pressure", {"pr" : 0.01, "pi" : 0})
acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"anr" : 0, "ani" : 0})
acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"Z0" : 1.25*343})

acoustic.add_material("Vzduch", {"vel" : 343, "acoustic_permittivity" : 1.25})


# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(-0.4, 0.05, 0.1, 0.2, boundaries = {"Matched boundary"})

geometry.add_edge(0.1, -0.2, -0.4, -0.05, boundaries = {"Matched boundary"})

geometry.add_edge(0.1, -0.2, -0.4, -0.05, boundaries = {"Matched boundary"})

geometry.add_edge(-0.4, 0.05, -0.4, -0.05, boundaries = {"Matched boundary"})

geometry.add_edge(-0.18, -0.06, -0.17, -0.05, boundaries = {"Source"}, angle=90)

geometry.add_edge(-0.17, -0.05, -0.18, -0.04, boundaries = {"Source"}, angle=90)

geometry.add_edge(-0.18, -0.04, -0.19, -0.05, boundaries = {"Source"}, angle=90)

geometry.add_edge(-0.19, -0.05, -0.18, -0.06, boundaries = {"Source"}, angle=90)

geometry.add_edge(0.1, -0.2, 0.1, 0.2, boundaries = {"Matched boundary"}, angle=90)

geometry.add_edge(0.03, 0.1, -0.04, -0.05, boundaries = {"Wall"}, angle=90)

geometry.add_edge(-0.04, -0.05, 0.08, -0.04, boundaries = {"Wall"})

geometry.add_edge(0.08, -0.04, 0.03, 0.1, boundaries = {"Wall"})

# labels
geometry.add_label(-0.0814934, 0.0707097, materials = {"Vzduch"})
geometry.add_label(-0.181474, -0.0504768, materials = {"none"})
geometry.add_label(0.0314514, 0.0411749, materials = {"none"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point valueqtcreator 2.2 ppa
point = acoustic.local_values(-0.084614, 0.053416)
testp = acoustic.test("Acoustic pressure", point["p"], 0.003064)
testp_real = acoustic.test("Acoustic pressure - real", point["pr"], 0.002322)
testp_imag = acoustic.test("Acoustic pressure - imag", point["pi"], 0.001999)
# testSPL = acoustic.test("Acoustic sound level", point["SPL"], 40.695085)

# volume integral
volume = acoustic.volume_integrals([0])
# testPv_real = acoustic.test("Pressure - real", volume["p_real"], -1.915211e-5)
# testPv_imag = acoustic.test("Pressure - imag", volume["p_imag"], -1.918928e-5)

# surface integral 
surface = acoustic.surface_integrals([0])
# testPs_real = acoustic.test("Pressure - real", surface["p_real"], 3.079084e-4)
# testPs_imag = acoustic.test("Pressure - imag", surface["p_imag"], 4.437581e-5)

# print("Test: Acoustic - planar: " + str(testp and testp_real and testp_imag and testSPL and testPv_real and testPv_imag and testPs_real and testPs_imag))
print("Test: Acoustic - planar: " + str(testp and testp_real and testp_imag))