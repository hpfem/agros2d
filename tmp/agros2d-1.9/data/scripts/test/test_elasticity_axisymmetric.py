# model
newdocument("Elasticity - axisymmetric", "axisymmetric", "elasticity", 4, 3, "disabled", 1, 1, 0, "steadystate", 1.0, 1.0, 0.0)

# boundaries
addboundary("Fixed", "elasticity_fixed", "elasticity_fixed", 0, 0, 0, 0)
addboundary("Free", "elasticity_free", "elasticity_free", 0, 0, 0, 0)
addboundary("Load", "elasticity_free", "elasticity_free", 0, -10000, 0, 0)

# materials
addmaterial("Material 1", 2e+11, 0.33, 0, 0, 1e-7, 30, 30)
addmaterial("Material 2", 1e+11, 0.33, 0, 30000, 1.2e-5, 20.5, 20)
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
testVonMises = test("Von Mises stress", point["mises"], 1.69779e5)
# testTresca = test("Tresca stress", point["tresca"], 1.235475e5)
testu = test("Displacement - x", point["u"], 5.544176e-6)
testv = test("Displacement - y", point["v"], -2.672647e-6)
testD = test("Displacement", point["disp"], 6.154748e-6)
testsrr = test("Stress RR", point["sxx"], -1.132081e5)
testszz = test("Stress ZZ", point["syy"], -1.210277e5)
testsaa = test("Stress aa", point["szz"], -2.7248e5)
testtxy = test("Stress RZ", point["sxy"], 39372.872587)
testerr = test("Strain RR", point["exx"], 6.165992e-6)
testezz = test("Strain ZZ", point["eyy"], 6.061992e-6)
testeaa = test("Strain aa", point["ezz"], 4.049699e-6)
testerz = test("Strain RZ", point["exy"], 5.236592e-7)

# surface integral
# surface = surfaceintegral(0)
# testI = test("Current", surface["I"], 3629.425713)

print("Test: Structural mechanics - axisymmetric: " + str(testVonMises and testu and testv and testD and testsrr and testszz and testsaa and testerr and testezz and testeaa and testerz))
