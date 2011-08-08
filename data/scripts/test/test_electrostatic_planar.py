# model
newdocument(name="Elec2 (QuickField)", type="planar",
            physicfield="electrostatic", analysistype="steadystate",
            numberofrefinements=2, polynomialorder=3,
            nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("Neumann", "electrostatic_surface_charge_density", {"sigma" : 0})
addboundary("U = 0 V", "electrostatic_potential", {"V" : 0})
addboundary("U = 1000 V", "electrostatic_potential", {"V" : 1000})

# materials
addmaterial("Diel", {"epsr" : 3, "rho" : 0})
addmaterial("Air", {"epsr" : 1, "rho" : 0})
addmaterial("Source", {"epsr" : 10, "rho" : 4e-10})

# edges
addedge(1, 2, 1, 1, boundary="U = 1000 V")
addedge(4, 1, 1, 1, boundary="U = 1000 V")
addedge(1, 2, 4, 2, boundary="U = 1000 V")
addedge(4, 2, 4, 1, boundary="U = 1000 V")
addedge(20, 24, 20, 1, boundary="Neumann")
addedge(20, 1, 20, 0, boundary="Neumann")
addedge(4, 1, 20, 1)
addedge(0, 24, 0, 1, boundary="Neumann")
addedge(0, 0, 0, 1, boundary="Neumann")
addedge(0, 0, 20, 0, boundary="U = 0 V")
addedge(0, 24, 20, 24, boundary="Neumann")
addedge(0, 1, 1, 1)
addedge(7, 13, 14, 13)
addedge(14, 13, 14, 18)
addedge(14, 18, 7, 18)
addedge(7, 18, 7, 13)

# labels
addlabel(2.78257, 1.37346, material="none")
addlabel(10.3839, 15.7187, material="Source")
addlabel(3.37832, 15.8626, material="Air")
addlabel(12.3992, 0.556005, material="Diel")

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
volume = volumeintegral([1])
testEnergy = test("Energy", volume["We"], 1.307484e-7)

# surface integral
surface = surfaceintegral([0, 1, 2, 3])
testQ = test("Electric charge", surface["Q"], 1.048981e-7)

print("Test: Electrostatic - planar: " + str(testV and testE and testEx and testEy and testD and testDx and testDy and testwe and testEnergy and testQ))
