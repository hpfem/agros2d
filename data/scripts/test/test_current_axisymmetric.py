# model
newdocument(name="Tube", type="axisymmetric",
            physicfield="current", analysistype="steadystate",
            numberofrefinements=1, polynomialorder=4,
            nonlineartolerance=0.001, nonlinearsteps=10)

# boundaries
addboundary("Neumann", "current_inward_current_flow", {"Jn" : 0})
addboundary("Ground", "current_potential", {"V" : 0})
addboundary("Voltage", "current_potential", {"V" : 10})
addboundary("Inlet", "current_inward_current_flow", {"Jn" : -3e9})

# materials
addmaterial("Copper", {"gamma" : 5.7e7})

# edges
addedge(0, 0.45, 0, 0, boundary="Neumann")
addedge(0, 0, 0.2, 0, boundary="Ground")
addedge(0.2, 0, 0.2, 0.15, boundary="Inlet")
addedge(0.2, 0.15, 0.35, 0.45, boundary="Neumann")
addedge(0.35, 0.45, 0, 0.45, boundary="Voltage")

# labels
addlabel(0.0933957, 0.350253, material="Copper")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.213175, 0.25045)
testV = test("Scalar potential", point["V"], 5.566438)
testE = test("Electric field", point["E"], 32.059116)
testEr = test("Electric field - r", point["Er"], -11.088553)
testEz = test("Electric field - z", point["Ez"], -30.080408)
testJ = test("Current density", point["J"], 1.82737e9)
testJr = test("Current density - r", point["Jr"], -6.320475e8)
testJz = test("Current density - z", point["Jz"], -1.714583e9)
testpj = test("Losses", point["pj"], 5.858385e10)	

# volume integral
volume = volumeintegral(0)
testPj = test("Losses", volume["Pj"], 4.542019e9)

# surface integral
surface = surfaceintegral(1)
testI = test("Current", surface["I"], -2.166256e8)

print("Test: Current field - axisymmetric: " + str(testV and testE and testEr and testEz and testJ and testJr and testJz and testpj and testI))