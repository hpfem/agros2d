# model
newdocument(name="Heat transfer axisymmetric", type="axisymmetric",
            physicfield="heat", analysistype="steadystate",
            numberofrefinements=2, polynomialorder=3,
            nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("Neumann", "heat_heat_flux", {"f" : 0, "h" : 0, "Te" : 0})
addboundary("Inlet", "heat_heat_flux", {"f" : 500000, "h" : 0, "Te" : 0})
addboundary("Temperature", "heat_temperature", {"T" : 0})
addboundary("Convection", "heat_heat_flux", {"f" : 0, "h" : 20, "Te" : 20})

# materials
addmaterial("Material", {"lambda" : 52, "p" : 6e6})

# edges
addedge(0.02, 0, 0.1, 0, boundary="Temperature")
addedge(0.1, 0, 0.1, 0.14, boundary="Convection")
addedge(0.1, 0.14, 0.02, 0.14, boundary="Temperature")
addedge(0.02, 0.14, 0.02, 0.1, boundary="Neumann")
addedge(0.02, 0.04, 0.02, 0, boundary="Neumann")
addedge(0.02, 0.04, 0.02, 0.1, boundary="Inlet")

# labels
addlabel(0.0460134, 0.0867717, material="Material", area=0.0003)

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.062926, 0.038129)
testT = test("Temperature", point["T"], 263.811616)
testG = test("Gradient", point["G"], 4615.703047)
testGr = test("Gradient - r", point["Gr"], 808.030874788)
testGz = test("Gradient - z", point["Gz"], -4544.425)
testF = test("Heat flux", point["F"], 2.400166e5)
testFr = test("Heat flux - r", point["Fr"], 42017.605489)
testFz = test("Heat flux - z", point["Fz"], -2.363101e5)

# volume integral
volume = volumeintegral([0])
testTavg = test("Average temperature", volume["T"], 0.949673)

# surface integral
surface = surfaceintegral([1]);
testFlux = test("Heat flux", surface["f"], 333.504915)

print("Test: Heat transfer steady state - axisymmetric: " + str(testT and testG and testGr and testGz and testF and testFr and testFz and testTavg and testFlux))