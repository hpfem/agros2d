# model
newdocument("Heat transfer axisymmetric", "axisymmetric", "heat", 2, 3)

# boundaries
addboundary("Neumann", "heat_heat_flux", 0, 0, 0)
addboundary("Inlet", "heat_heat_flux", 500000, 0, 0)
addboundary("Temperature", "heat_temperature", 0)
addboundary("Convection", "heat_heat_flux", 0, 20, 20)

# materials
addmaterial("Material", 6000000, 52, 0, 0)

# edges
addedge(0.02, 0, 0.1, 0, 0, "Temperature")
addedge(0.1, 0, 0.1, 0.14, 0, "Convection")
addedge(0.1, 0.14, 0.02, 0.14, 0, "Temperature")
addedge(0.02, 0.14, 0.02, 0.1, 0, "Neumann")
addedge(0.02, 0.04, 0.02, 0, 0, "Neumann")
addedge(0.02, 0.04, 0.02, 0.1, 0, "Inlet")

# labels
addlabel(0.0460134, 0.0867717, 0.0003, 0, "Material")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.062926, 0.038129)
testT = test("Temperature", point["T"], 263.811616)
testG = test("Gradient", point["G"], 4615.703047)
testGr = test("Gradient - r", point["Gx"], 808.030874788)
testGz = test("Gradient - z", point["Gy"], -4544.425)
testF = test("Heat flux", point["F"], 2.400166e5)
testFr = test("Heat flux - r", point["Fx"], 42017.605489)
testFz = test("Heat flux - z", point["Fy"], -2.363101e5)

# volume integral
volume = volumeintegral(0)
testTavg = test("Average temperature", volume["T_avg"], 0.949673/0.004222)

# surface integral
surface = surfaceintegral(1);
testFlux = test("Heat flux", surface["F"], 333.504915)

print("Test: Heat transfer steady state - axisymmetric: " + str(testT and testG and testGr and testGz and testF and testFr and testFz and testTavg and testFlux))