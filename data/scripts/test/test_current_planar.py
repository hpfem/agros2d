# model
newdocument("Feeder", "planar", "current", 3, 5)

# boundaries
addboundary("Neumann", "current_inward_current_flow", 0)
addboundary("Zero", "current_potential", 0)
addboundary("Voltage", "current_potential", 1)

# materials
addmaterial("mat 1", 1e7)
addmaterial("mat 2", 1e5)
addmaterial("mat 3", 1e3)

# edges
addedge(0, 0, 0.6, 0, 0, "Zero")
addedge(0, 0.8, 0, 0.5, 0, "Neumann")
addedge(0, 0.5, 0, 0, 0, "Neumann")
addedge(0, 0, 0.35, 0.5, 0, "none")
addedge(0.35, 0.5, 0.6, 0.5, 0, "none")
addedge(0.6, 0.8, 0.6, 0.5, 0, "Neumann")
addedge(0.6, 0.5, 0.6, 0, 0, "Neumann")
addedge(0, 0.5, 0.35, 0.5, 0, "none")
addedge(0, 0.8, 0.6, 0.8, 0, "Voltage")

# labels
addlabel(0.3, 0.670924, 0, "mat 1")
addlabel(0.105779, 0.364111, 0, "mat 2")
addlabel(0.394296, 0.203668, 0, "mat 3")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.11879, 0.346203)
testV = test("Scalar potential", point["V"], 0.928377)
testE = test("Electric field", point["E"], 0.486928)
testEx = test("Electric field - x", point["Ex"], -0.123527)
testEy = test("Electric field - y", point["Ey"], -0.470999)
testJ = test("Current density", point["J"], 48692.830437)
testJx = test("Current density - x", point["Jx"], -12352.691339)
testJy = test("Current density - y", point["Jy"], -47099.923064)
testpj = test("Losses", point["pj"], 23709.917359)

# surface integral
surface = surfaceintegral(0)
testI = test("Current", surface["I"], 3629.425713)

print("Test: Current field - planar: " + str(testV and testE and testEx and testEy and testJ and testJx and testJy and testpj and testI))
