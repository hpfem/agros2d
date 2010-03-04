# model
newdocument("Tube", "axisymmetric", "current", 1, 4, "disabled", 1, 0, 0, "steadystate", 1, 1, 0)

# boundaries
addboundary("Neumann", "current_inward_current_flow", 0)
addboundary("Ground", "current_potential", 0)
addboundary("Voltage", "current_potential", 10)
addboundary("Inlet", "current_inward_current_flow", -3e9)

# materials
addmaterial("Copper", 5.7e+07)

# edges
addedge(0, 0.45, 0, 0, 0, "Neumann")
addedge(0, 0, 0.2, 0, 0, "Ground")
addedge(0.2, 0, 0.2, 0.15, 0, "Inlet")
addedge(0.2, 0.15, 0.35, 0.45, 0, "Neumann")
addedge(0.35, 0.45, 0, 0.45, 0, "Voltage")

# labels
addlabel(0.0933957, 0.350253, 0, 0, "Copper")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.213175, 0.25045)
testV = test("Scalar potential", point["V"], 5.566438)
testE = test("Electric field", point["E"], 32.059116)
testEr = test("Electric field - r", point["Ex"], -11.088553)
testEz = test("Electric field - z", point["Ey"], -30.080408)
testJ = test("Current density", point["J"], 1.82737e9)
testJr = test("Current density - r", point["Jx"], -6.320475e8)
testJz = test("Current density - z", point["Jy"], -1.714583e9)
testpj = test("Losses", point["pj"], 5.858385e10)	

# surface integral
surface = surfaceintegral(1)
testI = test("Current", surface["I"], -2.166256e8)

print("Test: Current field - planar: " + str(testV and testE and testEr and testEz and testJ and testJr and testJz and testpj and testI))