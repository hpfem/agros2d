# model
newdocument("Elec2 (QuickField)", "planar", "electrostatic", 2, 3)

# boundaries
addboundary("Neumann", "electrostatic_surface_charge_density", 0)
addboundary("U =  0 V", "electrostatic_potential", 0)
addboundary("U =  1000 V", "electrostatic_potential", 1000)

# materials
addmaterial("Diel", 0, 3)
addmaterial("Air", 0, 1)
addmaterial("Source", 4e-10, 10)

# edges
addedge(1, 2, 1, 1, 0, "U =  1000 V")
addedge(4, 1, 1, 1, 0, "U =  1000 V")
addedge(1, 2, 4, 2, 0, "U =  1000 V")
addedge(4, 2, 4, 1, 0, "U =  1000 V")
addedge(20, 24, 20, 1, 0, "Neumann")
addedge(20, 1, 20, 0, 0, "Neumann")
addedge(4, 1, 20, 1, 0, "none")
addedge(0, 24, 0, 1, 0, "Neumann")
addedge(0, 0, 0, 1, 0, "Neumann")
addedge(0, 0, 20, 0, 0, "U =  0 V")
addedge(0, 24, 20, 24, 0, "Neumann")
addedge(0, 1, 1, 1, 0, "none")
addedge(7, 13, 14, 13, 0, "none")
addedge(14, 13, 14, 18, 0, "none")
addedge(14, 18, 7, 18, 0, "none")
addedge(7, 18, 7, 13, 0, "none")

# labels
addlabel(2.78257, 1.37346, 0, 0, "none")
addlabel(10.3839, 15.7187, 0, 0, "Source")
addlabel(3.37832, 15.8626, 0, 0, "Air")
addlabel(12.3992, 0.556005, 0, 0, "Diel")

# solve
zoombestfit()
solve()

# point value
point = pointresult(13.257584, 11.117738)
testV = test("Scalar potential", point["V"], 1111.544825)
testE = test("Electric field", point["E"], 111.954358)
testEx = test("Electric field - x", point["Ex"], 24.659054)
testEy = test("Electric field - y", point["Ey"], -109.204896)
testD = test("Displacement", point["D"], 9.912649e-10)
testDx = test("Displacement - x", point["Dx"], 2.183359e-10)
testDy = test("Displacement - y", point["Dy"], -9.669207e-10)
testwe = test("Energy density", point["we"], 5.548821e-8)

# volume integral
volume = volumeintegral(1)
testEnergy = test("Energy", volume["We"], 1.307484e-7)

# surface integral
surface = surfaceintegral(0, 1, 2, 3)
testQ = test("Electric charge", surface["Q"], 1.048981e-7)

print("Test: Electrostatic - planar: " + str(testV and testE and testEx and testEy and testD and testDx and testDy and testwe and testEnergy and testQ))
