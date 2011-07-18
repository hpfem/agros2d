# model
newdocument("Elasticity - planar", "planar", "elasticity", 3, 3, "disabled", 1, 1, 0, "steadystate", 1.0, 1.0, 0.0)

# boundaries
addboundary("Fixed", "elasticity_fixed", "elasticity_fixed", 0, 0, 0, 0)
addboundary("Free", "elasticity_free", "elasticity_free", 0, 0, 0, 0)
addboundary("Load", "elasticity_free", "elasticity_free", 0, -1.2e4, 0, 0)

# materials
addmaterial("Material 1", 2e11, 0.33, -1e6, 0, 1e-7, 30, 20)
addmaterial("Material 2", 1e11, 0.33, 0, 3e4, 1e-7, 30, 20)
addmaterial("Material 3", 1e11, 0.33, 0, 0, 1e-7, 30, 20)

# edges
addedge(1.4, 0.2, 1.6, 0.2, 0, "Load")
addedge(1.6, 0.2, 1.6, 0, 0, "Free")
addedge(1.6, 0, 1.4, 0, 0, "Free")
addedge(0, 0.2, 0, 0, 0, "Fixed")
addedge(1.4, 0.2, 1.4, 0, 0, "none")
addedge(0, 0.2, 0.6, 0.2, 0, "Free")
addedge(0.6, 0.2, 0.6, 0, 0, "none")
addedge(0, 0, 0.6, 0, 0, "Free")
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
point = pointresult(1.266507, 0.166771)
testVonMises = test("Von Mises stress", point["mises"], 1.175226e5)
testTresca = test("Tresca stress", point["tresca"], 1.344939e5)
testu = test("Displacement - x", point["u"], 1.14097e-6)
testv = test("Displacement - y", point["v"], 7.291872e-7)
testD = test("Displacement", point["disp"], 1.354077e-6)
testsxx = test("Stress XX", point["sxx"], 50551.83149)
testsyy = test("Stress YY", point["syy"], 613.931457)
testszz = test("Stress ZZ", point["szz"], -83115.298227)
testsxy = test("Stress XY", point["sxy"], -6478.61142)
testexx = test("Strain XX", point["exx"], 1.777773e-6)
testeyy = test("Strain YY", point["eyy"], 1.113599e-6)
testexy = test("Strain XY", point["exy"], -8.616553e-8)

# surface integral
# surface = surfaceintegral(0)
# testI = test("Current", surface["I"], 3629.425713)

print("Test: Structural mechanics - planar: " + str(testVonMises and testTresca and testu and testv and testD and testsxx and testsyy and testszz and testsxy and testexx and testeyy and testexy))
