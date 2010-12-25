# model
newdocument("Elasticity - axisymmetric", "axisymmetric", "elasticity", 3, 3, "disabled", 1, 1, 0, "steadystate", 1.0, 1.0, 0.0)

# boundaries
addboundary("Fixed", "elasticity_fixed", "elasticity_fixed", 0, 0, 0, 0)
addboundary("Free", "elasticity_free", "elasticity_free", 0, 0, 0, 0)
addboundary("Load", "elasticity_free", "elasticity_free", 0, -10000, 0, 0)

# materials
addmaterial("Material 1", 2e+11, 0.33, 0, 0, 1e-7, 30, 30)
addmaterial("Material 2", 1e+11, 0.33, 0, 30000, 1e-7, 30, 30)
addmaterial("Material 3", 1e+11, 0.33, 0, 0, 2e-7, 30, 30)

# edges
addedge(1.4, 0.2, 1.6, 0.2, 0, "Load")
addedge(1.6, 0.2, 1.6, 0, 0, "Free")
addedge(1.6, 0, 1.4, 0, 0, "Free")
addedge(0, 0.2, 0, 0, 0, "Fixed")
addedge(1.4, 0.2, 1.4, 0, 0, "none")
addedge(0, 0.2, 0.6, 0.2, 0, "Free")
addedge(0.6, 0.2, 0.6, 0, 0, "none")
addedge(0, 0, 0.6, 0, 0, "Fixed")
addedge(0.6, 0, 1.4, 0, 0, "Free")
addedge(1.4, 0.2, 0.6, 0.2, 0, "Free")

# labels
addlabel(0.0823077, 0.11114, 0, 0, "Material 1")
addlabel(1.48989, 0.108829, 0, 0, "Material 3")
addlabel(1.20588, 0.108829, 0, 0, "Material 2")

# solve
zoombestfit()
solve()

# point value
point = pointresult(1.369034, 0.04259)
testVonMises = test("Von Mises stress", point["mises"], 30287.972891)
# testTresca = test("Tresca stress", point["tresca"], 1.235475e5)
testu = test("Displacement - x", point["u"], -2.943525e-7)
testv = test("Displacement - y", point["v"], -2.185688e-6)
testD = test("Displacement", point["disp"], 2.205419e-6)
testsrr = test("Stress RR", point["sxx"], -20854.325496)
testszz = test("Stress ZZ", point["syy"], -776.910669)
testsaa = test("Stress aa", point["szz"], -28623.833756)
testtxy = test("Stress RZ", point["sxy"], -9976.028911)
testerr = test("Strain RR", point["exx"], -1.109809e-7)
testezz = test("Strain ZZ", point["eyy"], 1.552395e-7)
testeaa = test("Strain aa", point["ezz"], -2.150074e-7)
testerz = test("Strain RZ", point["exy"], -1.32874e-7)

# surface integral
# surface = surfaceintegral(0)
# testI = test("Current", surface["I"], 3629.425713)

print("Test: Structural mechanics - axisymmetric: " + str(testVonMises and testu and testv and testD and testsrr and testszz and testsaa and testerr and testezz and testeaa and testerz))
