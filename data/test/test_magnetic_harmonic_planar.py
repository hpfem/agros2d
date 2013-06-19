import agros2d
from time import time
start_time = time()


# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

problem.frequency = 50

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 3
magnetic.linearity_type = "linear"

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e7, "magnetic_conductivity" : 5.7e7}) 
magnetic.add_material("Cond 2", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 3e7, "magnetic_conductivity" : 5.7e7}) 
magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : 20})    

# geometry
geometry = agros2d.geometry

# edges
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

# labels
geometry.add_label(0.035349, 0.036683, materials = {"magnetic" : "Air"}, area=0)
geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
geometry.add_label(-0.0111161, -0.00311249, materials = {"magnetic" : "Cond 2"}, area=1e-05)
geometry.add_label(0.016868, -0.0186625, materials = {"magnetic" : "Magnet"}, area=0)

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.012448, 0.016473)
testA = agros2d.test("Magnetic potential", point["A"], 0.001087)
testA_real = agros2d.test("Magnetic potential - real", point["Ar"], 3.391642e-4)
testA_imag = agros2d.test("Magnetic potential - imag", point["Ai"], -0.001033)
testB = agros2d.test("Flux density", point["B"], 0.038197)
testBx_real = agros2d.test("Flux density - x - real", point["Brx"], -0.004274)
testBx_imag = agros2d.test("Flux density - x - imag", point["Bix"], 0.02868)
testBy_real = agros2d.test("Flux density - y - real", point["Bry"], 0.003269)
testBy_imag = agros2d.test("Flux density - y - imag", point["Biy"], -0.024707)
testH = agros2d.test("Magnetic field", point["H"], 30351.803874)
testHx_real = agros2d.test("Magnetic field - x - real", point["Hrx"], -3400.886351)
testHx_imag = agros2d.test("Magnetic field - x - imag", point["Hix"], 22823.176772)
testHy_real = agros2d.test("Magnetic field - y - real", point["Hry"], 2613.37651)
testHy_imag = agros2d.test("Magnetic field - y - imag", point["Hiy"], -19543.255504)
testwm = agros2d.test("Energy density", point["wm"], 289.413568)
testpj = agros2d.test("Losses density ", point["pj"], 3.435114e5)
testJit_real = agros2d.test("Current density - induced transform - real", point["Jitr"], -1.849337e7)
testJit_imag = agros2d.test("Current density - induced transform - imag", point["Jiti"], -6.073744e6)
testJ_real = agros2d.test("Current density - total - real", point["Jr"], 1.50663e6)
testJ_imag = agros2d.test("Current density - total - imag", point["Ji"], -6.073744e6)
testFx = agros2d.test("Lorentz force - x", point["Flx"], -77163)
testFy = agros2d.test("Lorentz force - y", point["Fly"], -89097)

# volume integral
volume = magnetic.volume_integrals([1])
testIe_real = agros2d.test("Current - external - real", volume["Ier"], 4000.0)
testIe_imag = agros2d.test("Current - external - imag", volume["Iei"], 0.0)
testIit_real = agros2d.test("Current - induced transform - real", volume["Iitr"], -4104.701323)
testIit_imag = agros2d.test("Current - induced transform - imag", volume["Iiti"], -1381.947299)
testIr = agros2d.test("Current - real", volume["Ir"], -104.701323)
testIi = agros2d.test("Current - imag", volume["Ii"], -1381.947299)
testWm = agros2d.test("Energy", volume["Wm"], 0.042927)
testPj = agros2d.test("Losses", volume["Pj"], 90.542962)
testFLx = agros2d.test("Lorentz force - x", volume["Flx"], -11.228229)
testFLy = agros2d.test("Lorentz force - y", volume["Fly"], -4.995809)

result = str(testA and testA_real and testA_imag and testB and testBx_real and testBx_imag and testBy_real and testBy_imag
             and testH and testHx_real and testHy_real and testHx_imag and testHy_imag
             and testJit_real and testJit_imag and testJ_real and testJ_imag
             and testFx and testFy
             and testIe_real and testIe_imag and testIit_real and testIit_imag and testIr and testIi
             and testwm and testpj and testWm and testPj
             and testFLx and testFLy)

elapsed_time = time() - start_time
print("Test: Magnetic harmonic - planar ({0}): ".format(round(elapsed_time, 3)) + result)