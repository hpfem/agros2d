# model
newdocument("Acoustic", "planar", "acoustic", 3, 2, "disabled", 5, 1, 2000, "harmonic", 1.0, 1.0, 0.0)

# boundaries
addboundary("Source", "acoustic_pressure", 0.01)
addboundary("Wall", "acoustic_normal_acceleration", 0)
addboundary("Matched boundary", "acoustic_matched_boundary", 0)

# materials
addmaterial("Vzduch", 1.25, 343)

# edges
addedge(-0.4, 0.05, 0.1, 0.2, 0, "Matched boundary")
addedge(0.1, -0.2, -0.4, -0.05, 0, "Matched boundary")
addedge(-0.4, 0.05, -0.4, -0.05, 0, "Matched boundary")
addedge(-0.18, -0.06, -0.17, -0.05, 90, "Source")
addedge(-0.17, -0.05, -0.18, -0.04, 90, "Source")
addedge(-0.18, -0.04, -0.19, -0.05, 90, "Source")
addedge(-0.19, -0.05, -0.18, -0.06, 90, "Source")
addedge(0.1, -0.2, 0.1, 0.2, 90, "Matched boundary")
addedge(0.03, 0.1, -0.04, -0.05, 90, "Wall")
addedge(-0.04, -0.05, 0.08, -0.04, 0, "Wall")
addedge(0.08, -0.04, 0.03, 0.1, 0, "Wall")

# labels
addlabel(-0.0814934, 0.0707097, 0, 0, "Vzduch")
addlabel(-0.181474, -0.0504768, 0, 0, "none")
addlabel(0.0314514, 0.0411749, 0, 0, "none")

# solve
zoombestfit()
solve()

# point value
point = pointresult(-0.084614, 0.053416)
testP = test("Acoustic pressure", point["p"], 0.003064)
testP_real = test("Acoustic pressure - real", point["p_real"], 0.002322)
testP_imag = test("Acoustic pressure - imag", point["p_imag"], 0.001999)
testI = test("Acoustic pressure", point["Lp"], 40.695085)
# volume integral
# volume = volumeintegral(0, 1, 2)
# testEnergy = test("Energy", volume["We"], 1.799349e-8)

# surface integral
# surface = surfaceintegral(1, 12)
# testQ = test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: Acoustic - planar: " + str(testP and testP_real and testP_imag and testI))