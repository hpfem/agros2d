# model
newdocument(name="Magnetic harmonic - planar", type="planar",
                        physicfield="magnetic", analysistype="harmonic",
                        frequency=50,
                        numberofrefinements=1, polynomialorder=3,
                        nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("A = 0", "magnetic_potential", {"Ar" : 0})

# materials
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
addlabel(0.035349, 0.036683, area=0, material="Air")
addlabel(0.00778124, 0.00444642, area=1e-05, material="Cond 1")
addlabel(-0.0111161, -0.00311249, area=1e-05, material="Cond 2")
addlabel(0.016868, -0.0186625, area=0, material="Magnet")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.012448, 0.016473)
#print(point)
testA = test("Magnetic potential", point["A"], 0.001087)
testA_real = test("Magnetic potential - real", point["Ar"], 3.391642e-4)
testA_imag = test("Magnetic potential - imag", point["Ai"], -0.001033)
testB = test("Flux density", point["B"], 0.038197)
testBx_real = test("Flux density - x - real", point["Brx"], -0.004274)
testBx_imag = test("Flux density - x - imag", point["Bix"], 0.02868)
testBy_real = test("Flux density - y - real", point["Bry"], 0.003269)
testBy_imag = test("Flux density - y - imag", point["Biy"], -0.024707)
testH = test("Magnetic field", point["H"], 30351.803874)
testHx_real = test("Magnetic field - x - real", point["Hrx"], -3400.886351)
testHx_imag = test("Magnetic field - x - imag", point["Hix"], 22823.176772)
testHy_real = test("Magnetic field - y - real", point["Hry"], 2613.37651)
testHy_imag = test("Magnetic field - y - imag", point["Hiy"], -19543.255504)
#testwm = test("Energy density", point["wm"], 289.413568)
#testpj = test("Losses density ", point["pj"], 3.435114e5)
#testJit_real = test("Current density - induced transform - real", point["Jit_real"], -1.849337e7)
#testJit_imag = test("Current density - induced transform - imag", point["Jit_imag"], -6.073744e6)
#testJ_real = test("Current density - total - real", point["Jer"], 1.50663e6)
#testJ_imag = test("Current density - total - imag", point["Jei"], -6.073744e6)
#testFx_real = test("Lorentz force - y - real", point["Fx_real"], 1.442159e5)
#testFx_imag = test("Lorentz force - x - imag", point["Fx_imag"], 56947.557678)
#testFy_real = test("Lorentz force - y - real", point["Fy_real"], 1.677588e5)
#testFy_imag = test("Lorentz force - y - imag", point["Fy_imag"], 69168.050723)

# volume integral
volume = volumeintegral([1])
#print(volume)
#testIit_real = test("Current - induced transform - real", volume["Iit_real"], -4104.701323)
#testIit_imag = test("Current - induced transform - imag", volume["Iit_imag"], -1381.947299)
#testIe_real = test("Current - external - real", volume["Ie_real"], 4000.0)
#testIe_imag = test("Current - external - imag", volume["Ie_imag"], 0.0)
#testI_real = test("Current - real", volume["I_real"], -104.701323)
#testI_imag = test("Current - imag", volume["I_imag"], -1381.947299)
#testWm = test("Energy", volume["Wm"], 0.042927)
#testPj = test("Losses", volume["Pj"], 90.542962)
#testFLx = test("Lorentz force - x", volume["Fx"], -11.228229)
#testFLy = test("Lorentz force - y", volume["Fy"], -4.995809)

print("Test: Magnetic harmonic - planar: " + str(testA and testA_real and testA_imag and testB and testBx_real and testBx_imag and testBy_real and testBy_imag and testH and testHx_real and testHy_real and testHx_imag and testHy_imag))

#testwm and testpj and
#testJit_real and testJit_imag and testJ_real and testJ_imag and
#testFx_real and testFx_imag and testFy_real and testFy_imag and
#testIit_real and testIit_imag and testIe_real and testIe_imag and testI_real and testI_imag and
#testWm and testPj and testFLx and testFLy
#))
