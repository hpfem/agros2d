# model
#newdocument("Harmonic", "planar", "magnetic", 1, 3, "disabled", 1, 1, 50, "harmonic", 1, 1, 0)
newdocument(name="Magnetostatic", type="planar",
                        physicfield="magnetic", analysistype="harmonic",
                        numberofrefinements=1, polynomialorder=3,
                        nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
#addboundary("A = 0", "magnetic_vector_potential", 0, 0)
addboundary("A = 0", "magnetic_potential", {"Ar" : 0})

# materials
#addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
#addmaterial("Cond 1", 2e7, 0, 1, 5.7e7, 0, 0, 0, 0, 0)
#addmaterial("Cond 2", 3e7, 0, 1, 5.7e7, 0, 0, 0, 0, 0)
#addmaterial("Magnet", 0, 0, 1.1, 0, 0.1, 20, 0, 0, 0)
addmaterial("Air", {"mur" : 1})
addmaterial("Cond 1", {"mur" : 1, "Jer" : 2e7, "gamma" : 5.7e7})
addmaterial("Cond 2", {"mur" : 1, "Jer" : 3e7, "gamma" : 5.7e7})
addmaterial("Magnet", {"mur" : 1.1, "Brm" : 0.1, "Bra" : 20})

# edges
addedge(-0.075, 0.06, 0.075, 0.06, boundary="A = 0")
addedge(0.075, 0.06, 0.075, -0.06, boundary="A = 0")
addedge(0.075, -0.06, -0.075, -0.06, boundary="A = 0")
addedge(-0.075, -0.06, -0.075, 0.06, boundary="A = 0")
addedge(-0.015, -0.01, -0.015, 0.01)
addedge(-0.015, 0.01, -0.005, 0.01)
addedge(-0.015, -0.01, -0.005, -0.01)
addedge(-0.005, -0.01, -0.005, 0.01)
addedge(0.005, 0.02, 0.005, 0)
addedge(0.005, 0, 0.015, 0)
addedge(0.015, 0, 0.015, 0.02)
addedge(0.015, 0.02, 0.005, 0.02)
addedge(0.01, -0.01, 0.03, -0.01)
addedge(0.03, -0.03, 0.01, -0.03)
addedge(0.01, -0.01, 0.01, -0.03)
addedge(0.03, -0.01, 0.03, -0.03)

# labels
#addlabel(0.035349, 0.036683, 0, 0, "Air")
#addlabel(0.00778124, 0.00444642, 1e-05, 0, "Cond 1")
#addlabel(-0.0111161, -0.00311249, 1e-05, 0, "Cond 2")
#addlabel(0.016868, -0.0186625, 0, 0, "Magnet")
addlabel(0.035349, 0.036683, area=0, material="Air")
addlabel(0.00778124, 0.00444642, area=1e-05, material="Cond 1")
addlabel(-0.0111161, -0.00311249, area=1e-05, material="Cond 2")
addlabel(0.016868, -0.0186625, area=0, material="Magnet")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.012448, 0.016473)
testA = test("Magnetic potential", point["A"], 0.001087)
testA_real = test("Magnetic potential - real", point["A_real"], 3.391642e-4)
testA_imag = test("Magnetic potential - imag", point["A_imag"], -0.001033)
testB = test("Flux density", point["B"], 0.038197)
testBx_real = test("Flux density - x - real", point["Bx_real"], -0.004274)
testBx_imag = test("Flux density - x - imag", point["Bx_imag"], 0.02868)
testBy_real = test("Flux density - y - real", point["By_real"], 0.003269)
testBy_imag = test("Flux density - y - imag", point["By_imag"], -0.024707)
testH = test("Magnetic field", point["H"], 30351.803874)
testHx_real = test("Magnetic field - x - real", point["Hx_real"], -3400.886351)
testHx_imag = test("Magnetic field - x - imag", point["Hx_imag"], 22823.176772)
testHy_real = test("Magnetic field - y - real", point["Hy_real"], 2613.37651)
testHy_imag = test("Magnetic field - y - imag", point["Hy_imag"], -19543.255504)
testwm = test("Energy density", point["wm"], 289.413568)
testpj = test("Losses density ", point["pj"], 3.435114e5)
testJit_real = test("Current density - induced transform - real", point["Jit_real"], -1.849337e7)
testJit_imag = test("Current density - induced transform - imag", point["Jit_imag"], -6.073744e6)
testJ_real = test("Current density - total - real", point["J_real"], 1.50663e6)
testJ_imag = test("Current density - total - imag", point["J_imag"], -6.073744e6)
testFx_real = test("Lorentz force - y - real", point["Fx_real"], 1.442159e5)
testFx_imag = test("Lorentz force - x - imag", point["Fx_imag"], 56947.557678)
testFy_real = test("Lorentz force - y - real", point["Fy_real"], 1.677588e5)
testFy_imag = test("Lorentz force - y - imag", point["Fy_imag"], 69168.050723)

# volume integral
volume = volumeintegral(1)
testIit_real = test("Current - induced transform - real", volume["Iit_real"], -4104.701323)
testIit_imag = test("Current - induced transform - imag", volume["Iit_imag"], -1381.947299)
testIe_real = test("Current - external - real", volume["Ie_real"], 4000.0)
testIe_imag = test("Current - external - imag", volume["Ie_imag"], 0.0)
testI_real = test("Current - real", volume["I_real"], -104.701323)
testI_imag = test("Current - imag", volume["I_imag"], -1381.947299)
testWm = test("Energy", volume["Wm"], 0.042927)
testPj = test("Losses", volume["Pj"], 90.542962)
testFLx = test("Lorentz force - x", volume["Fx"], -11.228229)
testFLy = test("Lorentz force - y", volume["Fy"], -4.995809)

print("Test: Magnetic harmonic - planar: " + str(
testA and testA_real and testA_imag and 
testB and testBx_real and testBx_imag and testBy_real and testBy_imag and
testH and testHx_real and testHx_imag and testHy_real and testHy_imag and
testwm and testpj and
testJit_real and testJit_imag and testJ_real and testJ_imag and 
testFx_real and testFx_imag and testFy_real and testFy_imag and
testIit_real and testIit_imag and testIe_real and testIe_imag and testI_real and testI_imag and
testWm and testPj and testFLx and testFLy))
