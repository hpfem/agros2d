import agros2d as a2d
from time import time
start_time = time()


# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

problem.frequency = 50

# disable view
a2d.view.mesh.disable()
a2d.view.post2d.disable()

# fields
magnetic = a2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 3
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

# point value
point1 = magnetic.local_values(0.0116, 0.0132)
point2 = magnetic.local_values(-0.0123, 0.00536)
point3 = magnetic.local_values(-0.0042, 0.03236)

# volume integral
volume1 = magnetic.volume_integrals([1])
volume2 = magnetic.volume_integrals([2])
volume3 = magnetic.volume_integrals([4])

#Area 1
testA_1 = a2d.test("Magnetic potential - 1", point1["A"], 0.0062725)
testA_1_real = a2d.test("Magnetic potential - 1 - real", point1["Ar"], 0.00543)
testA_1_imag = a2d.test("Magnetic potential - 1 - imag", point1["Ai"], 0.00314)
testB_1 = a2d.test("Flux density - 1", point1["B"], 0.10789)
testH_1 = a2d.test("Magnetic field - 1", point1["H"], 85856.68)
testHx_1_real = a2d.test("Magnetic field - 1 - x - real", point1["Hrx"], -61409.68265)
testHx_1_imag = a2d.test("Magnetic field - 1 - x - imag", point1["Hix"], -13675.94624)
testHy_1_real = a2d.test("Magnetic field - 1 - y - real", point1["Hry"], 57956.23371)
testHy_1_imag = a2d.test("Magnetic field - 1 - y - imag", point1["Hiy"], 7364.8575)
testwm_1 = a2d.test("Energy density - 1", point1["wm"], 2315.787)
testpj_1 = a2d.test("Losses density - 1 ", point1["pj"], 3.9578e7)

#todo: ujasnit si, co ma byt u harmonickeho pole Lorentzova sila
#to, co my pocitame se v comsolu shoduje s realnou slozkou veliciny "Lorentz force contribution, time average"
#proc je v comsolu time average komplexni cislo???

#ztraty v comsolu jsou i zaporne, co to znamena?

testFx_1 = a2d.test("Lorentz force - 1 - x", point1["Flx"], -2.30979e6)
testFy_1 = a2d.test("Lorentz force - 1 - y", point1["Fly"], -2.60749e6)

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

testWm = a2d.test("Energy - 1", volume1["Wm"], 2.24807)
testPj = a2d.test("Losses - 1", volume1["Pj"], 8029.097)
testFLx = a2d.test("Lorentz force - 1 - x", volume1["Flx"], 265.14584)
testFLy = a2d.test("Lorentz force - 1 - y", volume1["Fly"], 8.32792)


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

result = str(testA_1 and testA_1_real and testA_1_imag and testB_1 and 
    testH_1 and testHx_1_real and testHx_1_imag and testHy_1_real and testHy_1_imag and

	testwm_1 and testpj_1 and testFx_1 and testFy_1 and

    testBx_1_real and testBx_1_imag and testBy_1_real and testBy_1_imag and
    testJit_1_real and testJit_1_imag and testJe_1_real and testJe_1_imag and testJ_1_real and testJ_1_imag and
	testIe_1_real and testIe_1_imag and testIit_1_real and testIit_1_imag and testI_1_real and testI_1_imag and

    testBx_2_real and testBx_2_imag and testBy_2_real and testBy_2_imag and
    testJit_2_real and testJit_2_imag and testJ_2_real and testJ_2_imag and
	testIe_2_real and testIe_2_imag and testIit_2_real and testIit_2_imag and testI_2_real and testI_2_imag and

    testBx_3_real and testBx_3_imag and testBy_3_real and testBy_3_imag and
    testJit_3_real and testJit_3_imag and testJ_3_real and testJ_3_imag and testJ_3_real and testJ_3_imag and
	testIe_3_real and testIe_3_imag and testIit_3_real and testIit_3_imag and testI_3_real and testI_3_imag)

elapsed_time = time() - start_time
print("Test: Magnetic harmonic - planar - current ({0}): ".format(round(elapsed_time, 3)) + result)