# model
newdocument(name="Magnetostatic", type="planar", 
			physicfield="magnetic", analysistype="steadystate", 
			numberofrefinements=1, polynomialorder=7, 
			nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("A = 0", "magnetic_potential", {"Ar" : 0})

# materials
addmaterial("Cu", {"mur" : 1, "Jer" : 1e6})
addmaterial("Fe", {"mur" : 500})
addmaterial("Air", {"mur" : 1})
addmaterial("Magnet", {"mur" : 1.11, "Jer" : 1e6, "Brm" : 0.1, "Bra" : -30})
addmaterial("Velocity", {"mur" : 1, "gamma" : 1e7, "vx" : 2})

# edges
addedge(-0.5, 1, 0.5, 1, boundary="A = 0")
addedge(0.5, -1, -0.5, -1, boundary="A = 0")
addedge(-0.2, 0.6, 0, 0.6)
addedge(0, 0.3, 0, 0.6)
addedge(-0.2, 0.3, 0, 0.3)
addedge(-0.2, 0.6, -0.2, 0.3)
addedge(-0.035, 0.135, 0.035, 0.135)
addedge(0.035, 0.135, 0.035, -0.04)
addedge(0.035, -0.04, -0.035, -0.04)
addedge(-0.035, 0.135, -0.035, -0.04)
addedge(0, -0.5, 0.3, -0.5)
addedge(0.3, -0.5, 0.3, -0.32)
addedge(0.3, -0.32, 0, -0.32)
addedge(0, -0.32, 0, -0.5)
addedge(-0.5, 1, -0.5, -0.15, boundary="A = 0")
addedge(-0.5, -0.15, -0.5, -0.2, boundary="A = 0")
addedge(-0.5, -0.2, -0.5, -1, boundary="A = 0")
addedge(0.5, 1, 0.5, -0.15, boundary="A = 0")
addedge(0.5, -0.15, 0.5, -0.2, boundary="A = 0")
addedge(0.5, -0.2, 0.5, -1, boundary="A = 0")
addedge(0.5, -0.2, -0.5, -0.2)
addedge(0.5, -0.15, -0.5, -0.15)

# labels
addlabel(-0.0959509, 0.445344, 0.001, material="Fe")
addlabel(0.00301448, 0.0404858, 0.005, material="Cu")
addlabel(-0.145434, -0.706253, 0.01, material="Air")
addlabel(0.143596, -0.364811, 0, material="Magnet")
addlabel(0.310203, 0.631164, 0, material="Air")
addlabel(-0.295858, -0.182894, 0, material="Velocity")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.018895, -0.173495)
testA = test("Magnetic potential", point["A_real"], 0.002978)
testB = test("Flux density", point["B_real"], 0.013047)
testBx = test("Flux density - x", point["Bx_real"], 0.008413)
testBy = test("Flux density - y", point["By_real"], -0.009972)
testH = test("Magnetic intensity", point["H_real"], 10382.294319)
testHx = test("Magnetic intensity - x", point["Hx_real"], 6695.236126)
testHy = test("Magnetic intensity - y", point["Hy_real"], -7935.102302)
testwm = test("Energy density", point["wm"], 67.727733)
testpj = test("Losses density ", point["pj"], 3977.255622)
testJiv = test("Current density - induced velocity", point["Jiv_real"], -1.987738e5)
testJ = test("Current density - total", point["J_real"], -1.987738e5)

# volume integral
volume = volumeintegral([5])
testWm = test("Energy", volume["Wm"], 3.088946)
testPj = test("Losses", volume["Pj"], 220.022114)
testFxv = test("Lorentz force - x", volume["Fx"], -110.011057)
testFyv = test("Lorentz force - y", volume["Fy"], -36.62167)
testT = test("Torque", volume["T"], 20.463818)

# surface integral
surface = surfaceintegral([2, 3, 4, 5])
testFx = test("Maxwell force - x", surface["Fx"], 2.531945, 0.05)
testFy = test("Maxwell force - y", surface["Fy"], -10.176192, 0.05)

print("Test: Magnetic steady state - planar: " + str(point and testA and testB and testBx and testBy and testH and testHx and testHy and testwm and testpj 
and testWm and testPj and testFxv and testFxv and testFyv and testT and testFx and testFy))