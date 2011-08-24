# model
newdocument(name="Magnetostatic", type="axisymmetric", 
			physicfield="magnetic", analysistype="steadystate", 
			numberofrefinements=0, polynomialorder=7, 
			nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("A = 0", "magnetic_potential", {"Ar" : 0})

# materials
addmaterial("Cu", {"mur" : 1, "Jer" : 2e6})
addmaterial("Fe", {"mur" : 300})
addmaterial("Air", {"mur" : 1})
addmaterial("Magnet", {"mur" : 1.1, "Brm" : 0.6, "Bra" : 90})

# edges
addedge(0.01, 0.01, 0.01, 0)
addedge(0.01, 0, 0.04, 0)
addedge(0.04, 0.1, 0, 0.1)
addedge(0, 0.1, 0, 0.08, boundary="A = 0")
addedge(0, 0.08, 0.01, 0.08)
addedge(0.01, 0.09, 0.03, 0.09)
addedge(0.01, 0.09, 0.01, 0.08)
addedge(0.01, 0.01, 0.03, 0.01)
addedge(0.012, 0.088, 0.012, 0.012)
addedge(0.012, 0.012, 0.028, 0.012)
addedge(0.028, 0.012, 0.028, 0.088)
addedge(0.028, 0.088, 0.012, 0.088)
addedge(0, 0.05, 0.009, 0.05)
addedge(0.009, 0.05, 0.009, -0.02)
addedge(0, 0.05, 0, -0.02, boundary="A = 0")
addedge(0, -0.02, 0.009, -0.02)
addedge(0, 0.15, 0, 0.1, boundary="A = 0")
addedge(0, 0.08, 0, 0.05, boundary="A = 0")
addedge(0, -0.02, 0, -0.05, boundary="A = 0")
addedge(0, -0.05, 0.1, 0.05, boundary="A = 0", angle=90)
addedge(0.1, 0.05, 0, 0.15, boundary="A = 0", angle=90)
addedge(0.03, 0.09, 0.03, 0.078)
addedge(0.03, 0.078, 0.04, 0.078)
addedge(0.04, 0.078, 0.04, 0.1)
addedge(0.04, 0.078, 0.04, 0.052)
addedge(0.03, 0.078, 0.03, 0.052)
addedge(0.03, 0.052, 0.04, 0.052)
addedge(0.04, 0.052, 0.04, 0)
addedge(0.03, 0.01, 0.03, 0.052)

# labels
addlabel(0.0348743, 0.0347237, material="Fe")
addlabel(0.00512569, -0.0070852, material="Fe")
addlabel(0.021206, 0.0692964, material="Cu")
addlabel(0.0141705, 0.12445, material="Air")
addlabel(0.0346923, 0.0892198, material="Fe")
addlabel(0.036093, 0.0654078, material="Magnet")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.005985, 0.043924)
testA = test("Scalar potential", point["Ar"], 5.438198e-4)
testB = test("Flux density", point["Br"], 0.195525)
testBr = test("Flux density - r", point["Brr"], 0.059085)
testBz = test("Flux density - z", point["Brz"], 0.186384)
testH = test("Magnetic intensity", point["Hr"], 518.646027)
testHr = test("Magnetic intensity - r", point["Hrr"], 156.728443)
testHz = test("Magnetic intensity - z", point["Hrz"], 494.39852)
#testwm = test("Energy density", point["wm"], 50.704118)

# volume integral
volume = volumeintegral([1])
#testWm = test("Energy", volume["Wm"], 0.002273)

# surface integral
surface = surfaceintegral([12, 13, 14, 15])
#testFz = test("Maxwell force - z", surface["Fy"], 0.368232)

print("Test: Magnetic steady state - axisymmetric: " + str(point and testA and testB and testBr and testBz and testH and testHr and testHz and testwm 
and testWm and testFz))