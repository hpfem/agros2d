# model
newdocument(name="unnamed", type="axisymmetric",
            physicfield="elasticity", analysistype="steadystate",
            numberofrefinements=2, polynomialorder=3,
            nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("Fixed", "elasticity_fixed_fixed", {"dx" : 0, "dy" : 0})
addboundary("Free", "elasticity_free_free", {"fx" : 0, "fy" : 0})
addboundary("Load", "elasticity_free_free", {"fx" : 0, "fy" : -10000})

# materials
addmaterial("Material 1", {"E" : 2e+11, "nu" : 0.33, "fx" : 0, "fy" : 0, "alpha" : 1e-7, "T" :  30, "Tr" : 30})
addmaterial("Material 2", {"E" : 1e+11, "nu" : 0.33, "fx" : 0, "fy" : 30000, "alpha" : 1.2e-5, "T" :  20.5, "Tr" : 20})
addmaterial("Material 3", {"E" : 1e+11, "nu" : 0.33, "fx" : 0, "fy" : 0, "alpha" : 2e-7, "T" : 30, "Tr" : 30})

# edges
addedge(1.4, 0.2, 1.6, 0.2, boundary="Load")
addedge(1.6, 0.2, 1.6, 0, boundary="Free")
addedge(1.6, 0, 1.4, 0, boundary="Free")
addedge(0, 0.2, 0, 0, boundary="Fixed")
addedge(1.4, 0.2, 1.4, 0)
addedge(0, 0.2, 0.6, 0.2, boundary="Free")
addedge(0.6, 0.2, 0.6, 0)
addedge(0, 0, 0.6, 0, boundary="Fixed")
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
point = pointresult(1.369034, 0.04259)
print(point)
#testVonMises = test("Von Mises stress", point["mises"], 1.69779e5)
# testTresca = test("Tresca stress", point["tresca"], 1.235475e5)
testd = test("Displacement", point["d"], 6.154748e-6)
testdr = test("Displacement - x", point["dr"], 5.544176e-6)
testdz = test("Displacement - y", point["dz"], -2.672647e-6)
#testsrr = test("Stress RR", point["sxx"], -1.132081e5)
#testszz = test("Stress ZZ", point["syy"], -1.210277e5)
#testsaa = test("Stress aa", point["szz"], -2.7248e5)
#testtxy = test("Stress RZ", point["sxy"], 39372.872587)
#testerr = test("Strain RR", point["exx"], 6.165992e-6)
#testezz = test("Strain ZZ", point["eyy"], 6.061992e-6)
#testeaa = test("Strain aa", point["ezz"], 4.049699e-6)
#testerz = test("Strain RZ", point["exy"], 5.236592e-7)

# surface integral
# surface = surfaceintegral(0)
# testI = test("Current", surface["I"], 3629.425713)

#print("Test: Structural mechanics - axisymmetric: " + str(testVonMises and testu and testv and testD and testsrr and testszz and testsaa and testerr and testezz and testeaa and testerz))
print("Test: Structural mechanics - axisymmetric: " + str(testd and testdr and testdz))