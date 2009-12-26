# model
newdocument("Feeder", "planar", "current", 2, 5)

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
testPotential = abs(abs(point["V"]) - 0.928377) < 1e-2
if (not testPotential):
	print("Scalar potential: " + str(abs(point["V"])) + " == " + str(0.928377))

# current
integral = surfaceintegral(0)
testI = abs(abs(integral["I"]) - 3629.425713) < 1e2
if (not testI):
	print("Current: " + str(abs(integral["I"])) + " == " + str(3629.425713))

print("Test: Current field - planar: " + str(testPotential and testI))
