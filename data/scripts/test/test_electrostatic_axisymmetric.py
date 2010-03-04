# model
newdocument("Electrostatic", "axisymmetric", "electrostatic", 1, 3)

# boundaries
addboundary("Source electrode", "electrostatic_potential", 10)
addboundary("Ground electrode", "electrostatic_potential", 0)
addboundary("Neumann BC", "electrostatic_surface_charge_density", 0)

# materials
addmaterial("Air", 0, 1)
addmaterial("Dielectric n.1", 1e-5, 10)
addmaterial("Dielectric n.2", 0, 3)

# edges
addedge(0, 0.2, 0, 0.08, 0, "Neumann BC")
addedge(0.01, 0.08, 0.01, 0, 0, "Source electrode")
addedge(0.01, 0, 0.03, 0, 0, "Neumann BC")
addedge(0.03, 0, 0.03, 0.08, 0, "none")
addedge(0.03, 0.08, 0.05, 0.08, 0, "none")
addedge(0.05, 0, 0.03, 0, 0, "Neumann BC")
addedge(0.05, 0.08, 0.05, 0, 0, "Ground electrode")
addedge(0.06, 0, 0.06, 0.08, 0, "Ground electrode")
addedge(0.05, 0.08, 0.06, 0.08, 0, "Ground electrode")
addedge(0.06, 0, 0.2, 0, 0, "Neumann BC")
addedge(0.2, 0, 0, 0.2, 90, "Neumann BC")
addedge(0.01, 0.08, 0.03, 0.08, 0, "none")
addedge(0.01, 0.08, 0, 0.08, 0, "Source electrode")

# labels
addlabel(0.019, 0.021, 5e-05, 0, "Dielectric n.1")
addlabel(0.0379, 0.051, 5e-05, 0, "Dielectric n.2")
addlabel(0.0284191, 0.123601, 0.0001, 0, "Air")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.0255872, 0.0738211)
testV = test("Scalar potential", point["V"], 25.89593)
testE = test("Electric field", point["E"], 151.108324)
testEr = test("Electric field - r", point["Ex"], 94.939342)
testEz = test("Electric field - z", point["Ey"], 117.559546)
testD = test("Displacement", point["D"], 1.337941e-8)
testDr = test("Displacement - r", point["Dx"], 8.406108e-9)
testDz = test("Displacement - z", point["Dy"], 1.040894e-8)
testwe = test("Energy density", point["we"], 1.01087e-6)

# volume integral
volume = volumeintegral(0, 1, 2)
testEnergy = test("Energy", volume["We"], 1.799349e-8)

# surface integral
surface = surfaceintegral(1, 12)
testQ = test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: Electrostatic - axisymmetric: " + str(testV and testE and testEr and testEz and testD and testDr and testDz and testwe and testEnergy and testQ))