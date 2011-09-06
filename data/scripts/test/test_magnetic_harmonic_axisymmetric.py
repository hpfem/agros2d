# model
#newdocument("Harmonic", "axisymmetric", "magnetic", 1, 3, "disabled", 1, 1, 100, "harmonic", 1, 1, 0)
newdocument(name="Magnetostatic", type="axisymmetric",
                        physicfield="magnetic", analysistype="harmonic",
                        frequency=100,
                        numberofrefinements=1, polynomialorder=3,
                        nonlineartolerance=0.001, nonlinearsteps=10)


# boundaries
addboundary("A = 0", "magnetic_potential", {"Ar" : 0})

# materials
#addmaterial("Coil", 1e6, 0, 1, 0, 0, 0, 0, 0, 0)
#addmaterial("Iron", 0, 0, 50, 5e3, 0, 0, 0, 0, 0)
#addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Coil", {"mur" : 1, "Jer" : 1e6})
addmaterial("Iron", {"mur" : 50, "gamma" : 5e3})
addmaterial("Air", {"mur" : 1})

# edges
addedge(0, -0.17, 0.15, -0.17, boundary="A = 0")
addedge(0.15, -0.17, 0.15, 0.17, boundary="A = 0")
addedge(0.15, 0.17, 0, 0.17, boundary="A = 0")
addedge(0.035, -0.03, 0.055, -0.03)
addedge(0.055, -0.03, 0.055, 0.11)
addedge(0.055, 0.11, 0.035, 0.11)
addedge(0.035, 0.11, 0.035, -0.03)
addedge(0, -0.05, 0.03, -0.05)
addedge(0.03, -0.05, 0.03, 0.05)
addedge(0.03, 0.05, 0, 0.05)
addedge(0, 0.05, 0, -0.05, boundary="A = 0")
addedge(0, 0.17, 0, 0.05, boundary="A = 0")
addedge(0, -0.05, 0, -0.17, boundary="A = 0")

# labels
addlabel(0.109339, 0.112786, area=0, material="Air")
addlabel(0.0442134, 0.0328588, area=0, material="Coil")
addlabel(0.0116506, -0.00740064, area=0, material="Iron")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.027159, 0.039398)
testA = test("Magnetic potential", point["A"], 0.001087)
testA_real = test("Magnetic potential - real", point["A_real"], 0.001107)
testA_imag = test("Magnetic potential - imag", point["A_imag"], -5.24264e-6)
testB = test("Flux density", point["B"], 0.099325)
testBr_real = test("Flux density - r - real", point["Bx_real"], 0.027587)
testBr_imag = test("Flux density - r - imag", point["Bx_imag"], -2.430976e-4)
testBz_real = test("Flux density - z - real", point["By_real"],  0.095414)
testBz_imag = test("Flux density - z - imag", point["By_imag"], 7.424088e-4)
testH = test("Magnetic field", point["H"], 1580.808517)
testHr_real = test("Magnetic field - r - real", point["Hx_real"], 439.052884)
testHr_imag = test("Magnetic field - r - imag", point["Hx_imag"], -3.869019)
testHz_real = test("Magnetic field - z - real", point["Hy_real"], 1518.562988)
testHz_imag = test("Magnetic field - z - imag", point["Hy_imag"], 11.815803)
testwm = test("Energy density", point["wm"], 39.253502)
testpj = test("Losses density ", point["pj"], 1210.138583)
testJit_real = test("Current density - induced transform - real", point["Jit_real"], -16.47024)
testJit_imag = test("Current density - induced transform - imag", point["Jit_imag"], -3478.665629)
testJ_real = test("Current density - total - real", point["J_real"], -16.47024)
testJ_imag = test("Current density - total - imag", point["J_imag"], -3478.665629)
testFr_real = test("Lorentz force - r - real", point["Fx_real"], 1.011098)
testFr_imag = test("Lorentz force - r - imag", point["Fx_imag"], -331.92607)
testFz_real = test("Lorentz force - z - real", point["Fy_real"], 1.300012)
testFz_imag = test("Lorentz force - z - imag", point["Fy_imag"], 95.960227)

# volume integral
volume = volumeintegral(2)
testIit_real = test("Current - induced transform - real", volume["Iit_real"], -0.067164)
testIit_imag = test("Current - induced transform - imag", volume["Iit_imag"], -5.723787)
testIe_real = test("Current - external - real", volume["Ie_real"], 0.0)
testIe_imag = test("Current - external - imag", volume["Ie_imag"], 0.0)
testI_real = test("Current - real", volume["I_real"], -0.067164)
testI_imag = test("Current - imag", volume["I_imag"], -5.723787)
testWm = test("Energy", volume["Wm"], 0.009187)
testPj = test("Losses", volume["Pj"], 0.228758)
testFLr = test("Lorentz force - r", volume["Fx"], -4.018686e-4)
testFLz = test("Lorentz force - z", volume["Fy"], -1.233904e-5)

print("Test: Magnetic harmonic - axisymmetric: " + str(
testA and testA_real and testA_imag and 
testB and testBr_real and testBr_imag and testBz_real and testBz_imag and
testH and testHr_real and testHr_imag and testHz_real and testHz_imag and
testwm and testpj and
testJit_real and testJit_imag and testJ_real and testJ_imag and 
testFr_real and testFr_imag and testFz_real and testFz_imag and
testIit_real and testIit_imag and testIe_real and testIe_imag and testI_real and testI_imag and
testWm and testPj and testFLr and testFLz))
