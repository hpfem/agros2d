# model
newdocument("Electrostatic", "axisymmetric", "electrostatic", 0, 3)

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
addlabel(0.019, 0.021, 5e-05, "Dielectric n.1")
addlabel(0.0379, 0.051, 5e-05, "Dielectric n.2")
addlabel(0.0284191, 0.123601, 0.0001, "Air")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.026532, 0.070937)
testPotential = abs(abs(point["V"]) - 26.078318) < 0.1
if (not testPotential):
	print("Scalar potential: ", abs(point["V"]), " == ", 26.078318)

# energy
integral = volumeintegral(0, 1, 2)
testEnergy = abs(abs(integral["We"]) - 1.799349e-8) < 1e-10
if (not testEnergy):
	print("Electric energy: ", abs(integral["We"]), " == ", 1.799349e-8)

# charge
integral = surfaceintegral(1, 12)
testQ = abs(abs(integral["Q"]) - 1.291778e-9) < 1e-11
if (not testQ):
	print("Electric charge: ", abs(integral["Q"]), " == ", 1.291778e-9)

print "Test: Electrostatic - axisymmetric: ", (testPotential and testEnergy and testQ)
