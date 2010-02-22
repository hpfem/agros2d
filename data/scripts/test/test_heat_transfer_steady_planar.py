# model
newdocument("unnamed", "planar", "heat", 2, 3);

# boundaries
addboundary("T inner", "heat_temperature", -15);
addboundary("T outer", "heat_heat_flux", 0, 20, 20)
addboundary("Neumann", "heat_heat_flux", 0, 0, 0)

# materials
addmaterial("Material 1", 0, 2, 0, 0);
addmaterial("Material 2", 7e4, 10, 0, 0);

# edges
addedge(0.1, 0.15, 0, 0.15, 0, "T outer");
addedge(0, 0.15, 0, 0.1, 0, "Neumann");
addedge(0, 0.1, 0.05, 0.1, 0, "T inner");
addedge(0.05, 0.1, 0.05, 0, 0, "T inner");
addedge(0.05, 0, 0.1, 0, 0, "Neumann");
addedge(0.05, 0.1, 0.1, 0.1, 0, "none");
addedge(0.1, 0.15, 0.1, 0.1, 0, "T outer");
addedge(0.1, 0.1, 0.1, 0, 0, "T outer");

#labels
addlabel(0.0553981, 0.124595, 0.003, "Material 1");
addlabel(0.070091, 0.068229, 0.003, "Material 2");

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
volume = volumeintegral(0)
testTavg = test("Average temperature", volume["T_avg"], -0.023972/0.005)

# surface integral
surface = surfaceintegral(0, 6, 7);
testFlux = test("Heat flux", surface["F"], -103.220778)

print("Test: Heat transfer steady state - planar: " + str(testT and testG and testGx and testGy and testF and testFx and testFy and testTavg and testFlux))
