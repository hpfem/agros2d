# model
newdocument(name="unnamed", type="planar",
            physicfield="elasticity", analysistype="steadystate",
            numberofrefinements=3, polynomialorder=3,
            nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("Free", "elasticity_free_free", {"fx" : 0, "fy" : 0})
addboundary("Fixed", "elasticity_fixed_fixed", {"dx" : 0, "dy" : 0})
addboundary("Load", "elasticity_free_free", {"fx" : 0, "fy" : -1.2e4})

# materials
addmaterial("Material 1", {"E" : 2e11, "nu" : 0.33, "fx" : -1e6, "fy" : 0, "alpha" : 1e-6, "T" : 30, "Tr" : 20})
addmaterial("Material 2", {"E" : 1e11, "nu" : 0.33, "fx" : 0, "fy" : 3e4, "alpha" : 1e-6, "T" : 30, "Tr" : 20})
addmaterial("Material 3", {"E" : 1e11, "nu" : 0.33, "fx" : 0, "fy" : 0, "alpha" : 1e-6, "T" : 30, "Tr" : 20})

# edges
addedge(1.4, 0.2, 1.6, 0.2, boundary="Load")
addedge(1.6, 0.2, 1.6, 0, boundary="Free")
addedge(1.6, 0, 1.4, 0, boundary="Free")
addedge(0, 0.2, 0, 0, boundary="Fixed")
addedge(1.4, 0.2, 1.4, 0)
addedge(0, 0.2, 0.6, 0.2, boundary="Free")
addedge(0.6, 0.2, 0.6, 0)
addedge(0, 0, 0.6, 0, boundary="Free")
addedge(0.6, 0, 1.4, 0, boundary="Free")
addedge(1.4, 0.2, 0.6, 0.2, boundary="Free")

# labels
addlabel(0.0823077, 0.11114, material="Material 1")
addlabel(1.48989, 0.108829, material="Material 3")
addlabel(1.20588, 0.108829, material="Material 2")

# solve
zoombestfit()
solve()

# point value
point = pointresult(1.266507, 0.166771)
# testVonMises = test("Von Mises stress", point["mises"], 1.175226e5)
# testTresca = test("Tresca stress", point["tresca"], 1.344939e5)
testu = test("Displacement - x", point["dx"], 1.14097e-6)
testv = test("Displacement - y", point["dy"], 7.291872e-7)
testD = test("Displacement", point["d"], 1.354077e-6)
# testsxx = test("Stress XX", point["sxx"], 50551.83149)
# testsyy = test("Stress YY", point["syy"], 613.931457)
# testszz = test("Stress ZZ", point["szz"], -83115.298227)
# testsxy = test("Stress XY", point["sxy"], -6478.61142)
# testexx = test("Strain XX", point["exx"], 1.777773e-6)
# testeyy = test("Strain YY", point["eyy"], 1.113599e-6)
# testexy = test("Strain XY", point["exy"], -8.616553e-8)

# surface integral
# surface = surfaceintegral(0)
# testI = test("Current", surface["I"], 3629.425713)

# print("Test: Structural mechanics - planar: " + str(testVonMises and testTresca and testu and testv and testD and testsxx and testsyy and testszz and testsxy and testexx and testeyy and testexy))
print("Test: Structural mechanics - planar: " + str(testu and testv and testD))
