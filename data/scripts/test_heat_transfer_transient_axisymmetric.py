# model
newdocument("Actuator", "axisymmetric", "heat", 1, 5, "disabled", 0, 0, 0, "transient", 500, 10000, 20)

# boundaries
addboundary("Flux", "heat_heat_flux", 0, 0, 0)
addboundary("Convection", "heat_heat_flux", 0, 10, 20)

# materials
addmaterial("Air", 0, 0.02, 1.2, 1000)
addmaterial("Cu", 26000, 200, 8700, 385)
addmaterial("Fe", 0, 60, 7800, 460)
addmaterial("Brass", 0, 100, 8400, 378)

# edges
addedge(0, 0.18, 0.035, 0.18, 0, "Convection")
addedge(0.035, 0.18, 0.035, 0.03, 0, "Convection")
addedge(0.035, 0.03, 0.0135, 0.03, 0, "Convection")
addedge(0.0135, 0.03, 0.0135, 0.038, 0, "none")
addedge(0.0135, 0.038, 0.019, 0.038, 0, "none")
addedge(0.027, 0.038, 0.027, 0.172, 0, "none")
addedge(0.027, 0.172, 0.008, 0.172, 0, "none")
addedge(0.008, 0.172, 0.008, 0.16, 0, "none")
addedge(0.008, 0.16, 0, 0.16, 0, "none")
addedge(0, 0.16, 0, 0.18, 0, "Flux")
addedge(0, 0.13, 0.008, 0.13, 0, "none")
addedge(0.008, 0.13, 0.008, 0.0395, 0, "none")
addedge(0.008, 0, 0, 0, 0, "Convection")
addedge(0, 0, 0, 0.13, 0, "Flux")
addedge(0.019, 0.038, 0.027, 0.038, 0, "none")
addedge(0.019, 0.038, 0.019, 0.0395, 0, "none")
addedge(0, 0.13, 0, 0.16, 0, "Flux")
addedge(0.01, 0.041, 0.01, 0.1705, 0, "none")
addedge(0.025, 0.041, 0.01, 0.041, 0, "none")
addedge(0.01, 0.1705, 0.025, 0.1705, 0, "none")
addedge(0.025, 0.1705, 0.025, 0.041, 0, "none")
addedge(0.008, 0.0395, 0.008, 0.03, 0, "none")
addedge(0.008, 0.03, 0.008, 0, 0, "Convection")
addedge(0.019, 0.0395, 0.009, 0.0395, 0, "none")
addedge(0.008, 0.03, 0.009, 0.03, 0, "Convection")
addedge(0.009, 0.0395, 0.009, 0.03, 0, "none")
addedge(0.009, 0.03, 0.0135, 0.03, 0, "Convection")

# labels
addlabel(0.0308709, 0.171031, 0, "Fe")
addlabel(0.00316251, 0.011224, 0, "Fe")
addlabel(0.0163723, 0.144289, 0, "Cu")
addlabel(0.00380689, 0.151055, 0, "Air")
addlabel(0.0112064, 0.0336487, 0, "Brass")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.00503, 0.134283)
testT = test("Temperature", point["T"], 44.012004)
testF = test("Heat flux", point["F"], 16.739787)
testFr = test("Heat flux - r", point["Fx"], -7.697043)
testFz = test("Heat flux - z", point["Fy"], -14.865261)

# volume integral
volume = volumeintegral(3)
testTavg = test("Average temperature", volume["T_avg"], 0.034505/8.365e-4)

# surface integral
surface = surfaceintegral(26);
testFlux = test("Heat flux", surface["F"], 0.032866)

print("Heat transfer - planar: " + str(testT and testF and testFr and testFz and testTavg and testFlux))