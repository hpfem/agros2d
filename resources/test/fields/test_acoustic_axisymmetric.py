import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.frequency = 700

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
acoustic = agros2d.field("acoustic")
acoustic.analysis_type = "harmonic"
acoustic.number_of_refinements = 3
acoustic.polynomial_order = 2
acoustic.solver = "linear"

acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0, "acoustic_normal_acceleration_imag" : 0})
acoustic.add_boundary("Source acceleration", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 10, "acoustic_normal_acceleration_imag" : 0})
acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : 1.25*343})
acoustic.add_boundary("Source pressure", "acoustic_pressure", {"acoustic_pressure_real" : 0.2, "acoustic_pressure_imag" : 0})
acoustic.add_material("Air", {"acoustic_density" : 1.25, "acoustic_speed" : 343})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 1.5, 1.05, 1.25, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(1.05, 1.25, 0.25, 0, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.25, 0, 0, 0, boundaries = {"acoustic" : "Source acceleration"})
geometry.add_edge(0, 0, 0, 0.7, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0, 1, 0, 1.5, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0, 0.7, 0.15, 0.85, boundaries = {"acoustic" : "Wall"}, angle=90)
geometry.add_edge(0.15, 0.85, 0, 1, boundaries = {"acoustic" : "Wall"}, angle=90)
geometry.add_edge(0.35, 1.15, 0.65, 1, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.65, 1, 0.35, 0.9, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.35, 1.15, 0.35, 0.9, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.6, 1.2, 0.6, 1.25, boundaries = {"acoustic" : "Source pressure"})
geometry.add_edge(0.6, 1.2, 0.65, 1.2, boundaries = {"acoustic" : "Source pressure"})
geometry.add_edge(0.6, 1.25, 0.65, 1.2, boundaries = {"acoustic" : "Source pressure"})

# labels
geometry.add_label(0.163662, 0.383133, materials = {"acoustic" : "Air"})
geometry.add_label(0.426096, 1.03031, materials = {"acoustic" : "none"})
geometry.add_label(0.616273, 1.21617, materials = {"acoustic" : "none"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = acoustic.local_values(0.259371, 0.876998)
testp = agros2d.test("Acoustic pressure", point["p"], 0.49271)
testp_real = agros2d.test("Acoustic pressure - real", point["pr"], 0.395866)
testp_imag = agros2d.test("Acoustic pressure - imag", point["pi"], 0.293348)
# testSPL = acoustic.test("Acoustic sound level", point["SPL"], 84.820922)

# volume integral
volume = acoustic.volume_integrals([0])
#testPv_real = agros2d.test("Pressure - real", volume["p_real"], -0.030632)
#testPv_imag = aagros2d.test("Pressure - imag", volume["p_imag"], -0.010975)

# surface integral
surface = acoustic.surface_integrals([0])
#testPs_real = agros2d.test("Pressure - real", surface["p_real"], 0.196756)
#testPs_imag = agros2d.test("Pressure - imag", surface["p_imag"], -0.324708)

# print("Test: Acoustic - planar: " + str(testp and testp_real and testp_imag and testSPL and testPv_real and testPv_imag and testPs_real and testPs_imag))

print("Test: Acoustic - axisymmetric: " + str(testp and testp_real and testp_imag))