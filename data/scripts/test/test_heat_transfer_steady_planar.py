# model
newdocument(name="unnamed", type="planar",
            physicfield="heat", analysistype="steadystate",
            numberofrefinements=2, polynomialorder=3,
            nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("T inner", "heat_temperature", {"T" : -15})
addboundary("T outer", "heat_heat_flux", {"f" : 0, "h" : 20, "Te" : 20})
addboundary("Neumann", "heat_heat_flux", {"f" : 0, "h" : 0, "Te" : 0})

# materials
addmaterial("Material 1", {"lambda" : 2, "p" : 0})
addmaterial("Material 2", {"lambda" : 10, "p" : 70000})

# edges
addedge(0.1, 0.15, 0, 0.15, boundary="T outer")
addedge(0, 0.15, 0, 0.1, boundary="Neumann")
addedge(0, 0.1, 0.05, 0.1, boundary="T inner")
addedge(0.05, 0.1, 0.05, 0, boundary="T inner")
addedge(0.05, 0, 0.1, 0, boundary="Neumann")
addedge(0.05, 0.1, 0.1, 0.1)
addedge(0.1, 0.15, 0.1, 0.1, boundary="T outer")
addedge(0.1, 0.1, 0.1, 0, boundary="T outer")

# labels
addlabel(0.0553981, 0.124595, material="Material 1", area=0.003)
addlabel(0.070091, 0.068229, material="Material 2", area=0.003)

# solve
zoombestfit();
solve();

# point value
point = pointresult(0.086266, 0.087725)
testT = test("Temperature", point["T"], -4.363565)
testG = test("Gradient", point["G"], 155.570361)
testGx = test("Gradient - x", point["Gx"], -153.6640796)
testGy = test("Gradient - y", point["Gy"], -24.2793731)
testF = test("Heat flux", point["F"], 1555.703613)
testFx = test("Heat flux - x", point["Fx"], -1536.640796)
testFy = test("Heat flux - y", point["Fy"], -242.793731)

# volume integral
volume = volumeintegral([0])
testTavg = test("Average temperature", volume["T"], -0.023972)

# surface integral
surface = surfaceintegral([0, 6, 7]);
testFlux = test("Heat flux", surface["f"], -103.220778)

print("Test: Heat transfer steady state - planar: " + str(testT and testG and testGx and testGy and testF and testFx and testFy and testTavg and testFlux))
