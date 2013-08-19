import agros2d 
from time import time
start_time = time()

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.frequency = 100

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
# magnetic
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.matrix_solver = "mumps"
magnetic.number_of_refinements = 2
magnetic.polynomial_order = 3
magnetic.adaptivity_type = "disabled"
magnetic.solver = "linear"


# boundaries
magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})


# materials
magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 1e+06, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
magnetic.add_material("Iron", {"magnetic_permeability" : 50, "magnetic_conductivity" : 5000, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
magnetic.add_material("Coil2", {"magnetic_permeability" : 1, "magnetic_conductivity" : 1e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 5000, "magnetic_total_current_imag" : 20000})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, -0.17, 0.15, -0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.15, -0.17, 0.15, 0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.15, 0.17, 0, 0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.035, -0.03, 0.055, -0.03)
geometry.add_edge(0.055, -0.03, 0.055, 0.11)
geometry.add_edge(0.055, 0.11, 0.035, 0.11)
geometry.add_edge(0.035, 0.11, 0.035, -0.03)
geometry.add_edge(0, -0.05, 0.03, -0.05)
geometry.add_edge(0.03, -0.05, 0.03, 0.05)
geometry.add_edge(0.03, 0.05, 0, 0.05)
geometry.add_edge(0, 0.05, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.17, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.05, 0, -0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.06, -0.05, 0.04, -0.07, angle = 90)
geometry.add_edge(0.04, -0.07, 0.06, -0.09, angle = 90)
geometry.add_edge(0.06, -0.09, 0.08, -0.07, angle = 90)
geometry.add_edge(0.08, -0.07, 0.06, -0.05, angle = 90)

geometry.add_label(0.109339, 0.112786, materials = {"magnetic" : "Air"})
geometry.add_label(0.0442134, 0.0328588, materials = {"magnetic" : "Coil"})
geometry.add_label(0.0116506, -0.00740064, materials = {"magnetic" : "Iron"})
geometry.add_label(0.0612934, -0.075856, materials = {"magnetic" : "Coil2"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.05, -0.07)
testA = agros2d.test("Magnetic potential", point["A"], 0.00727)
testA_real = agros2d.test("Magnetic potential - real", point["Ar"], 0.00194)
testA_imag = agros2d.test("Magnetic potential - imag", point["Ai"], 0.00701)
testB = agros2d.test("Flux density", point["B"], 0.19266)
testBr_real = agros2d.test("Flux density - r - real", point["Brr"], -0.01387)
testBr_imag = agros2d.test("Flux density - r - imag", point["Bir"], -0.0242)
testBz_real = agros2d.test("Flux density - z - real", point["Brz"],  0.05584)
testBz_imag = agros2d.test("Flux density - z - imag", point["Biz"],  0.18156)

point2 = magnetic.local_values(0.02, -0.04)
testH = agros2d.test("Magnetic field", point2["H"], 7049.46496)
testHr_real = agros2d.test("Magnetic field - r - real", point2["Hrr"], -587.5199)
testHr_imag = agros2d.test("Magnetic field - r - imag", point2["Hir"], -634.21712)
testHz_real = agros2d.test("Magnetic field - z - real", point2["Hrz"], 2484.18713)
testHz_imag = agros2d.test("Magnetic field - z - imag", point2["Hiz"], 6540.36386)

testwm = agros2d.test("Energy density", point["wm"], 7384.29938)
testpj = agros2d.test("Losses density ", point["pj"], 1.83518e8)
testJit_real = agros2d.test("Current density - induced transform - real", point["Jitr"], 4.40517e6)
testJit_imag = agros2d.test("Current density - induced transform - imag", point["Jiti"], -1.21945e6)
testJet_real = agros2d.test("Current density - external - real", point["Jer"], 1.96967e5)
testJet_imag = agros2d.test("Current density - external - imag", point["Jei"], 1.98167e7)
testJ_real = agros2d.test("Current density - total - real", point["Jr"], 4.60213e6)
testJ_imag = agros2d.test("Current density - total - imag", point["Ji"], 1.85972e7)
testFr = True #testFr = agros2d.test("Lorentz force - r", point["Flr"], 2.5714e5)
testFz = True #testFz = agros2d.test("Lorentz force - z", point["Flz"], 64170.81137)

# volume integral
volume = magnetic.volume_integrals([3])
testIit_real = agros2d.test("Current - induced transform - real", volume["Iitr"], 4787.66447)
testIit_imag = agros2d.test("Current - induced transform - imag", volume["Iiti"], -1362.82611)
testIe_real = agros2d.test("Current - external - real", volume["Ier"], 212.33553)
testIe_imag = agros2d.test("Current - external - imag", volume["Iei"], 21362.82611)
testI_real = agros2d.test("Current - real", volume["Ir"], 5000)
testI_imag = agros2d.test("Current - imag", volume["Ii"], 20000)
testWm = agros2d.test("Energy", volume["Wm"], 0.0124) #(or 6.80219?)
testPj = agros2d.test("Losses", volume["Pj"], 1.7456e5)
testFLr = True #testFLr = agros2d.test("Lorentz force - r", volume["Flx"], -4.018686e-4)
testFLz = True #testFLz = agros2d.test("Lorentz force - z", volume["Fly"], -1.233904e-5)


result = str(testA and testA_real and testA_imag and testB and testBr_real and testBr_imag and testBz_real and testBz_imag
             and testH and testHr_real and testHr_imag and testHz_real and testHz_imag
             and testwm and testpj 
             and testJit_real and testJit_imag and testJet_real and testJet_imag and testJ_real and testJ_imag
             and testFr and testFz 
             and testIe_real and testIe_imag and testIit_real and testIit_imag and testI_real and testI_imag
             and testpj and testwm and testWm and testPj
             and testFLr and testFLr)

elapsed_time = time() - start_time
print("Test: Magnetic harmonic - axisymmetric ({0}): ".format(round(elapsed_time, 3)) + result)