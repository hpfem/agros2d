import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.frequency = 2000

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
acoustic = agros2d.field("acoustic")
acoustic.analysis_type = "harmonic"
acoustic.number_of_refinements = 3
acoustic.polynomial_order = 2
acoustic.linearity_type = "linear"

acoustic.add_boundary("Source", "acoustic_pressure", {"acoustic_pressure_real" : 0.01, "acoustic_pressure_imag" : 0})
acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0, "acoustic_normal_acceleration_imag" : 0})
acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : 1.25*343})

acoustic.add_material("Air", {"acoustic_speed" : 343, "acoustic_density" : 1.25})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(-0.4, 0.05, 0.1, 0.2, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.1, -0.2, -0.4, -0.05, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(-0.4, 0.05, -0.4, -0.05, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(-0.18, -0.06, -0.17, -0.05, boundaries = {"acoustic" : "Source"}, angle=90)
geometry.add_edge(-0.17, -0.05, -0.18, -0.04, boundaries = {"acoustic" : "Source"}, angle=90)
geometry.add_edge(-0.18, -0.04, -0.19, -0.05, boundaries = {"acoustic" : "Source"}, angle=90)
geometry.add_edge(-0.19, -0.05, -0.18, -0.06, boundaries = {"acoustic" : "Source"}, angle=90)
geometry.add_edge(0.1, -0.2, 0.1, 0.2, boundaries = {"acoustic" : "Matched boundary"}, angle=90)
geometry.add_edge(0.03, 0.1, -0.04, -0.05, boundaries = {"acoustic" : "Wall"}, angle=90)
geometry.add_edge(-0.04, -0.05, 0.08, -0.04, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.08, -0.04, 0.03, 0.1, boundaries = {"acoustic" : "Wall"})

# labels
geometry.add_label(-0.0814934, 0.0707097, materials = {"acoustic" : "Air"})
geometry.add_label(-0.181474, -0.0504768)
geometry.add_label(0.0314514, 0.0411749)

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point valueqtcreator 2.2 ppa
point = acoustic.local_values(-0.084614, 0.053416)
testp = agros2d.test("Acoustic pressure", point["p"], 0.003064)
testp_real = agros2d.test("Acoustic pressure - real", point["pr"], 0.002322)
testp_imag = agros2d.test("Acoustic pressure - imag", point["pi"], 0.001999)
testSPL = agros2d.test("Acoustic sound level", point["SPL"], 40.695085, 8)

# volume integral
volume = acoustic.volume_integrals([0])
# testPv_real = agros2d.test("Pressure - real", volume["p_real"], -1.915211e-5)
# testPv_imag = agros2d.test("Pressure - imag", volume["p_imag"], -1.918928e-5)

# surface integral 
surface = acoustic.surface_integrals([0])
# testPs_real = agros2d.test("Pressure - real", surface["p_real"], 3.079084e-4)
# testPs_imag = agros2d.test("Pressure - imag", surface["p_imag"], 4.437581e-5)

# print("Test: Acoustic - planar: " + str(testp and testp_real and testp_imag and testSPL and testPv_real and testPv_imag and testPs_real and testPs_imag))

elapsed_time = time() - start_time
print("Test: Acoustic - planar ({0}): ".format(round(elapsed_time, 3)) + str(testp and testp_real and testp_imag))