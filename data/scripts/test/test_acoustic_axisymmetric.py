# model
newdocument("Acoustic", "axisymmetric", "acoustic", 3, 2, "disabled", 3, 1, 700, "harmonic", 1.0, 1.0, 0.0)

# boundaries
addboundary("Wall", "acoustic_normal_acceleration", 0)
addboundary("Source acceleration", "acoustic_normal_acceleration", 10)
addboundary("Mached boundary", "acoustic_matched_boundary")
addboundary("Source pressure", "acoustic_pressure", 0.2)

# materials
addmaterial("Air", 1.25, 343)

# edges
addedge(0, 1.5, 1.05, 1.25, 0, "Wall")
addedge(1.05, 1.25, 0.25, 0, 0, "Mached boundary")
addedge(0.25, 0, 0, 0, 0, "Source acceleration")
addedge(0, 0, 0, 0.7, 0, "Wall")
addedge(0, 1, 0, 1.5, 0, "Wall")
addedge(0, 0.7, 0.15, 0.85, 90, "Wall")
addedge(0.15, 0.85, 0, 1, 90, "Wall")
addedge(0.35, 1.15, 0.65, 1, 0, "Mached boundary")
addedge(0.65, 1, 0.35, 0.9, 0, "Mached boundary")
addedge(0.35, 1.15, 0.35, 0.9, 0, "Mached boundary")
addedge(0.6, 1.2, 0.6, 1.25, 0, "Source pressure")
addedge(0.6, 1.2, 0.65, 1.2, 0, "Source pressure")
addedge(0.6, 1.25, 0.65, 1.2, 0, "Source pressure")

# labels
addlabel(0.163662, 0.383133, 0, 0, "Air")
addlabel(0.426096, 1.03031, 0, 0, "none")
addlabel(0.616273, 1.21617, 0, 0, "none")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.259371, 0.876998)
testP = test("Acoustic pressure", point["p"], 0.49271)
testP_real = test("Acoustic pressure - real", point["p_real"], 0.395866)
testP_imag = test("Acoustic pressure - imag", point["p_imag"], -0.293348)
testI = test("Acoustic pressure", point["I"], 87.83122)
# volume integral
# volume = volumeintegral(0, 1, 2)
# testEnergy = test("Energy", volume["We"], 1.799349e-8)

# surface integral
# surface = surfaceintegral(1, 12)
# testQ = test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: Acoustic - axisymmetric: " + str(testP and testP_real and testP_imag and testI))