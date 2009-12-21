# model
newdocument("Magnetostatic", "planar", "magnetic", 1, 3, "disabled", 1, 1, 0, "steadystate", 1, 1, 0)

# boundaries
addboundary("A = 0", "", 0)

# materials
addmaterial("Cu", 1e6, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Fe", 0, 0, 500, 0, 0, 0, 0, 0, 0)
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Magnet", 0, 0, 1.11, 0, 0.1, -30, 0, 0, 0)

# edges
addedge(-0.5, 1, 0.5, 1, 0, "A = 0")
addedge(0.5, 1, 0.5, -1, 0, "A = 0")
addedge(0.5, -1, -0.5, -1, 0, "A = 0")
addedge(-0.5, -1, -0.5, 1, 0, "A = 0")
addedge(-0.2, 0.6, 0, 0.6, 0, "none")
addedge(0, 0.3, 0, 0.6, 0, "none")
addedge(-0.2, 0.3, 0, 0.3, 0, "none")
addedge(-0.2, 0.6, -0.2, 0.3, 0, "none")
addedge(-0.035, 0.135, 0.035, 0.135, 0, "none")
addedge(0.035, 0.135, 0.035, -0.04, 0, "none")
addedge(0.035, -0.04, -0.035, -0.04, 0, "none")
addedge(-0.035, 0.135, -0.035, -0.04, 0, "none")
addedge(0, -0.5, 0.3, -0.5, 0, "none")
addedge(0.3, -0.5, 0.3, -0.32, 0, "none")
addedge(0.3, -0.32, 0, -0.32, 0, "none")
addedge(0, -0.32, 0, -0.5, 0, "none")

# labels
addlabel(-0.0959509, 0.445344, 0.01, "Fe")
addlabel(0.00301448, 0.0404858, 0.005, "Cu")
addlabel(-0.145434, -0.706253, 0.01, "Air")
addlabel(0.143596, -0.364811, 0, "Magnet")

# solve
zoombestfit()
solve()

# point value
point = pointresult(-0.011911, 0.170158)
testFluxDensity = abs(abs(point["B_real"]) - 0.022126) < 1e-4
if (not testFluxDensity):
	print "Flux density: ", abs(point["B_real"]), " == ", 0.022126

# energy
integral = volumeintegral(0, 1, 2, 3)
testEnergy = abs(abs(integral["Wm"]) - 144.196623) < 1
if (not testEnergy):
	print "Magnetic energy: ", abs(integral["Wm"]), " == ", 144.196623

print "Test: Magnetic steady state - planar: ", (testFluxDensity and testEnergy)