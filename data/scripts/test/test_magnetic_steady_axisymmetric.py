# model
newdocument("Magnetostatic", "axisymmetric", "magnetic", 0, 3, "disabled", 1, 1, 0, "steadystate", 1, 1, 0)

# boundaries
addboundary("A = 0", "magnetic_vector_potential", 0)

# materials
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Fe", 0, 0, 300, 0, 0, 0, 0, 0, 0)
addmaterial("Cu", 2e6, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Magnet", 0, 0, 1.1, 0, 0.6, 90, 0, 0, 0)

# edges
addedge(0.01, 0.01, 0.01, 0, 0, "none")
addedge(0.01, 0, 0.04, 0, 0, "none")
addedge(0.04, 0.1, 0, 0.1, 0, "none")
addedge(0, 0.1, 0, 0.08, 0, "A = 0")
addedge(0, 0.08, 0.01, 0.08, 0, "none")
addedge(0.01, 0.09, 0.03, 0.09, 0, "none")
addedge(0.01, 0.09, 0.01, 0.08, 0, "none")
addedge(0.01, 0.01, 0.03, 0.01, 0, "none")
addedge(0.012, 0.088, 0.012, 0.012, 0, "none")
addedge(0.012, 0.012, 0.028, 0.012, 0, "none")
addedge(0.028, 0.012, 0.028, 0.088, 0, "none")
addedge(0.028, 0.088, 0.012, 0.088, 0, "none")
addedge(0, 0.05, 0.009, 0.05, 0, "none")
addedge(0.009, 0.05, 0.009, -0.02, 0, "none")
addedge(0, 0.05, 0, -0.02, 0, "A = 0")
addedge(0, -0.02, 0.009, -0.02, 0, "none")
addedge(0, 0.15, 0, 0.1, 0, "A = 0")
addedge(0, 0.08, 0, 0.05, 0, "A = 0")
addedge(0, -0.02, 0, -0.05, 0, "A = 0")
addedge(0, -0.05, 0.1, 0.05, 90, "A = 0")
addedge(0.1, 0.05, 0, 0.15, 90, "A = 0")
addedge(0.03, 0.09, 0.03, 0.078, 0, "none")
addedge(0.03, 0.078, 0.04, 0.078, 0, "none")
addedge(0.04, 0.078, 0.04, 0.1, 0, "none")
addedge(0.04, 0.078, 0.04, 0.052, 0, "none")
addedge(0.03, 0.078, 0.03, 0.052, 0, "none")
addedge(0.03, 0.052, 0.04, 0.052, 0, "none")
addedge(0.04, 0.052, 0.04, 0, 0, "none")
addedge(0.03, 0.01, 0.03, 0.052, 0, "none")

# labels
addlabel(0.0348743, 0.0347237, 0, "Fe")
addlabel(0.00512569, -0.0070852, 0, "Fe")
addlabel(0.021206, 0.0692964, 0, "Cu")
addlabel(0.0141705, 0.12445, 0, "Air")
addlabel(0.0346923, 0.0892198, 0, "Fe")
addlabel(0.036093, 0.0654078, 0, "Magnet")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.005985, 0.043924)
testA = test("Scalar potential", point["A_real"], 5.438198e-4)
testB = test("Flux density", point["B_real"], 0.195525)
testBr = test("Flux density - r", point["Bx_real"], 0.059085)
testBz = test("Flux density - z", point["By_real"], 0.186384)
testH = test("Magnetic intensity", point["H_real"], 518.646027)
testHr = test("Magnetic intensity - r", point["Hx_real"], 156.728443)
testHz = test("Magnetic intensity - z", point["Hy_real"], 494.39852)
testwm = test("Energy density", point["wm"], 50.704118)

# volume integral
volume = volumeintegral(1)
testWm = test("Energy", volume["Wm"], 0.002273)

# surface integral
surface = surfaceintegral(12, 13, 14, 15)
testFz = test("Maxwell force - z", surface["Fy"], 0.368232, 0.2)

print("Test: Magnetic steady state - axisymmetric: " + str(point and testA and testB and testBr and testBz and testH and testHr and testHz and testwm 
and testWm and testFz))