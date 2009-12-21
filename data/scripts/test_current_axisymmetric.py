# model
newdocument("Tube", "axisymmetric", "current", 1, 2)

# boundaries
addboundary("Neumann", "current_inward_current_flow", 0)
addboundary("Ground", "current_potential", 0)
addboundary("Voltage", "current_potential", 10)

# materials
addmaterial("Copper", 57e6)

# edges
addedge(0, 0.45, 0, 0, 0, "Neumann")
addedge(0, 0, 0.2, 0, 0, "Ground")
addedge(0.2, 0, 0.2, 0.15, 0, "Neumann")
addedge(0.2, 0.15, 0.35, 0.45, 0, "Neumann")
addedge(0.35, 0.45, 0, 0.45, 0, "Voltage")

# labels
addlabel(0.125753, 0.324715, 0.001, "Copper")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.213175, 0.25045)
testPotential = abs(abs(point["V"]) - 7.285427) < 1e-2
if (not testPotential): 
    print "Scalar potential: ", abs(point["V"]), " == ", 7.285427

# current
integral = surfaceintegral(1)
testI = abs(abs(integral["I"]) - 2.124955e8) < 1e6
if (not testI):
    print "Current: ", abs(integral.I), " == ", 2.124955e8

print "Test: Current field - axisymmetric: ", (testPotential and testI)