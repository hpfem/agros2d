import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 50

# disable view
a2d.view.mesh.initial_mesh = False
a2d.view.mesh.solution_mesh = False
a2d.view.mesh.order = False
a2d.view.post2d.scalar = False
a2d.view.post2d.contours = False
a2d.view.post2d.vectors = False

# fields
# magnetic
magnetic = a2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 3
magnetic.adaptivity_type = "disabled"
magnetic.linearity_type = "linear"


# boundaries
magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})


# materials
magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 2e7, "magnetic_current_density_external_imag" : -1e7, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : { "expression" : "1e4/2e-4" }, "magnetic_total_current_imag" : { "expression" : "1e4/2e-4" }})
magnetic.add_material("Cond 2", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 3e+07, "magnetic_current_density_external_imag" : 1e7, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : 20, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
magnetic.add_material("Cond3", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})

# geometry
geometry = a2d.geometry
geometry.add_edge(-0.075, 0.06, 0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.075, 0.06, 0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.075, -0.06, -0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.075, -0.06, -0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.015, -0.01, -0.015, 0.01)
geometry.add_edge(-0.015, 0.01, -0.005, 0.01)
geometry.add_edge(-0.015, -0.01, -0.005, -0.01)
geometry.add_edge(-0.005, -0.01, -0.005, 0.01)
geometry.add_edge(0.005, 0.02, 0.005, 0)
geometry.add_edge(0.005, 0, 0.015, 0)
geometry.add_edge(0.015, 0, 0.015, 0.02)
geometry.add_edge(0.015, 0.02, 0.005, 0.02)
geometry.add_edge(0.01, -0.01, 0.03, -0.01)
geometry.add_edge(0.03, -0.03, 0.01, -0.03)
geometry.add_edge(0.01, -0.01, 0.01, -0.03)
geometry.add_edge(0.03, -0.01, 0.03, -0.03)
geometry.add_edge(-0.01, 0.02, 0, 0.03, angle = 90)
geometry.add_edge(-0.02, 0.03, -0.01, 0.02, angle = 90)
geometry.add_edge(-0.01, 0.04, -0.02, 0.03, angle = 90)
geometry.add_edge(0, 0.03, -0.01, 0.04, angle = 90)

geometry.add_label(0.035349, 0.036683, materials = {"magnetic" : "Air"})
geometry.add_label(0.00778124, 0.00444642, area = 1e-05, materials = {"magnetic" : "Cond 1"})
geometry.add_label(-0.0111161, -0.00311249, area = 1e-05, materials = {"magnetic" : "Cond 2"})
geometry.add_label(0.016868, -0.0186625, materials = {"magnetic" : "Magnet"})
geometry.add_label(-0.00707714, 0.0289278, materials = {"magnetic" : "Cond3"})
a2d.view.zoom_best_fit()

# solve problem
problem.solve()

# in this test, total current is prescribed on several domains
# mainly current densities are tested

# induced current density does not match. The reason is, that in areas, where total current is given, 
# comsol somehow alters external current density. Thus the total current density (=external+induced) matches, 
# but neither external nor induced match

# point value
point1 = magnetic.local_values(0.0116, 0.0132)
point2 = magnetic.local_values(-0.0123, 0.00536)
point3 = magnetic.local_values(-0.0042, 0.03236)

# volume integral
volume1 = magnetic.volume_integrals([1])
volume2 = magnetic.volume_integrals([2])
volume3 = magnetic.volume_integrals([4])

#Area 1
testBx_1_real = a2d.test("Flux density - 1 - x - real", point1["Brx"], -0.07714)
testBx_1_imag = a2d.test("Flux density - 1 - x - imag", point1["Bix"], -0.01711)
testBy_1_real = a2d.test("Flux density - 1 - y - real", point1["Bry"], 0.07273)
testBy_1_imag = a2d.test("Flux density - 1 - y - imag", point1["Biy"], 0.0091)

testJit_1_real = a2d.test("Current density - 1 - induced transform - real", point1["Jitr"], 5.62458e7)
testJit_1_imag = a2d.test("Current density - 1 - induced transform - imag", point1["Jiti"], -9.72075e7)
testJe_1_real = a2d.test("Current density - 1 - external - real", point1["Jer"], 1.67159e6)
testJe_1_imag = a2d.test("Current density - 1 - external - imag", point1["Jei"], 1.40589e8)
testJ_1_real = a2d.test("Current density - 1 - total - real", point1["Jr"], 5.79174e7)
testJ_1_imag = a2d.test("Current density - 1 - total - imag", point1["Ji"], 4.3382e7)

testIe_1_real = a2d.test("Current - 1 - external - real", volume1["Ier"], 334.22742)
testIe_1_imag = a2d.test("Current - 1 - external - imag", volume1["Iei"], 28117.91)
testIit_1_real = a2d.test("Current - 1 - induced transform - real", volume1["Iitr"], 9665.77)
testIit_1_imag = a2d.test("Current - 1 - induced transform - imag", volume1["Iiti"], -18117.91)
testI_1_real = a2d.test("Current - 1 - real", volume1["Ir"], 1e4)
testI_1_imag = a2d.test("Current - 1 - imag", volume1["Ii"], 1e4)

#Area 2
testBx_2_real = a2d.test("Flux density - 2 - x - real", point2["Brx"], 0.01789)
testBx_2_imag = a2d.test("Flux density - 2 - x - imag", point2["Bix"], 0.08029)
testBy_2_real = a2d.test("Flux density - 2 - y - real", point2["Bry"], -0.09474)
testBy_2_imag = a2d.test("Flux density - 2 - y - imag", point2["Biy"], 0.01871)

testJit_2_real = a2d.test("Current density - 2 - induced transform - real", point2["Jitr"], -2.86474e7)
testJit_2_imag = a2d.test("Current density - 2 - induced transform - imag", point2["Jiti"], -4.83838e7)
testJ_2_real = a2d.test("Current density - 2 - total - real", point2["Jr"], 1.35258e6)
testJ_2_imag = a2d.test("Current density - 2 - total - imag", point2["Ji"], -3.83838e7)

testIe_2_real = a2d.test("Current - 2 - external - real", volume2["Ier"], 6000)
testIe_2_imag = a2d.test("Current - 2 - external - imag", volume2["Iei"], 2000)
testIit_2_real = a2d.test("Current - 2 - induced transform - real", volume2["Iitr"], -5176.3)
testIit_2_imag = a2d.test("Current - 2 - induced transform - imag", volume2["Iiti"], -9797.11)
testI_2_real = a2d.test("Current - 2 - real", volume2["Ir"], 823.68)
testI_2_imag = a2d.test("Current - 2 - imag", volume2["Ii"], -7797.11)

#Area 3
testBx_3_real = a2d.test("Flux density - 3 - x - real", point3["Brx"], -0.07824)
testBx_3_imag = a2d.test("Flux density - 3 - x - imag", point3["Bix"], 0.01079)
testBy_3_real = a2d.test("Flux density - 3 - y - real", point3["Bry"], -0.04435)
testBy_3_imag = a2d.test("Flux density - 3 - y - imag", point3["Biy"], -0.02718)

testJit_3_real = a2d.test("Current density - 3 - induced transform - real", point3["Jitr"], 9.12211e6)
testJit_3_imag = a2d.test("Current density - 3 - induced transform - imag", point3["Jiti"], -3.67499e7)
testJe_3_real = a2d.test("Current density - 3 - external - real", point3["Jer"], -6.19451e6)
testJe_3_imag = a2d.test("Current density - 3 - external - imag", point3["Jei"], 3.39449e7)
testJ_3_real = a2d.test("Current density - 3 - total - real", point3["Jr"], 2.9276e6)
testJ_3_imag = a2d.test("Current density - 3 - total - imag", point3["Ji"], -2.80499e6)

testIe_3_real = a2d.test("Current - 3 - external - real", volume3["Ier"], -1946.07)
testIe_3_imag = a2d.test("Current - 3 - external - imag", volume3["Iei"], 10664.068)
testIit_3_real = a2d.test("Current - 3 - induced transform - real", volume3["Iitr"], 1946.07)
testIit_3_imag = a2d.test("Current - 3 - induced transform - imag", volume3["Iiti"], -10664.068)
testI_3_real = a2d.test("Current - 3 - real", volume3["Ir"], 0)
testI_3_imag = a2d.test("Current - 3 - imag", volume3["Ii"], 0)

print("Test: Magnetic harmonic - planar - current: " + str(
    testBx_1_real and testBx_1_imag and testBy_1_real and testBy_1_imag and
    testJit_1_real and testJit_1_imag and testJe_1_real and testJe_1_imag and testJ_1_real and testJ_1_imag and
    testBx_2_real and testBx_2_imag and testBy_2_real and testBy_2_imag and
    testJit_2_real and testJit_2_imag and testJ_2_real and testJ_2_imag and
    testBx_3_real and testBx_3_imag and testBy_3_real and testBy_3_imag and
    testJit_3_real and testJit_3_imag and testJ_3_real and testJ_3_imag))