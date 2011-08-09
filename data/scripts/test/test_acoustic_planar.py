# model
newdocument(name="Acoustic", type="planar",
            physicfield="acoustic", analysistype="harmonic",
            numberofrefinements=3, polynomialorder=2,
            nonlineartolerance=0.001, nonlinearsteps=10,
            frequency=2000)

# boundaries
addboundary("Source", "acoustic_pressure", {"pr" : 0.01, "pi" : 0})
addboundary("Wall", "acoustic_normal_acceleration", {"anr" : 0, "ani" : 0})
addboundary("Matched boundary", "acoustic_impedance", {"Z0" : 1.25*343})

# materials
addmaterial("Vzduch", {"rho" : 1.25, "v" : 343})

# edges
addedge(-0.4, 0.05, 0.1, 0.2, boundary="Matched boundary")
addedge(0.1, -0.2, -0.4, -0.05, boundary="Matched boundary")
addedge(-0.4, 0.05, -0.4, -0.05, boundary="Matched boundary")
addedge(-0.18, -0.06, -0.17, -0.05, boundary="Source", angle=90)
addedge(-0.17, -0.05, -0.18, -0.04, boundary="Source", angle=90)
addedge(-0.18, -0.04, -0.19, -0.05, boundary="Source", angle=90)
addedge(-0.19, -0.05, -0.18, -0.06, boundary="Source", angle=90)
addedge(0.1, -0.2, 0.1, 0.2, boundary="Matched boundary", angle=90)
addedge(0.03, 0.1, -0.04, -0.05, boundary="Wall", angle=90)
addedge(-0.04, -0.05, 0.08, -0.04, boundary="Wall")
addedge(0.08, -0.04, 0.03, 0.1, boundary="Wall")

# labels
addlabel(-0.0814934, 0.0707097, material="Vzduch")
addlabel(-0.181474, -0.0504768, material="none")
addlabel(0.0314514, 0.0411749, material="none")

# solve
zoombestfit()
solve()

# point valueqtcreator 2.2 ppa
point = pointresult(-0.084614, 0.053416)
testp = test("Acoustic pressure", point["p"], 0.003064)
testp_real = test("Acoustic pressure - real", point["p_real"], 0.002322)
testp_imag = test("Acoustic pressure - imag", point["p_imag"], 0.001999)
testI = test("Acoustic pressure", point["Lp"], 40.695085)
# volume integral
volume = volumeintegral(0)
testPv_real = test("Pressure - real", volume["p_real"], -1.915211e-5)
testPv_imag = test("Pressure - imag", volume["p_imag"], -1.918928e-5)

# surface integral 
surface = surfaceintegral(7)
testPs_real = test("Pressure - real", surface["p_real"], 3.079084e-4)
testPs_imag = test("Pressure - imag", surface["p_imag"], 4.437581e-5)

print("Test: Acoustic - planar: " + str(testp and testp_real and testp_imag and testI and testPv_real and testPv_imag and testPs_real and testPs_imag))