# model
newdocument("Magnetostatic", "axisymmetric", "magnetic", 1, 2, "disabled", 1, 1, 0, "steadystate", 1, 1, 0)

# boundaries
addboundary("A = 0", "magnetostatic_vector_potential", 0)

# materials
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Fe", 0, 0, 300, 0, 0, 0, 0, 0, 0)
addmaterial("Cu", 1000000, 0, 1, 0, 0, 0, 0, 0, 0)

# edges
addedge(0.01, 0.01, 0.01, 0, 0, "none")
addedge(0.01, 0, 0.04, 0, 0, "none")
addedge(0.04, 0, 0.04, 0.1, 0, "none")
addedge(0.04, 0.1, 0, 0.1, 0, "none")
addedge(0, 0.1, 0, 0.08, 0, "A = 0")
addedge(0, 0.08, 0.01, 0.08, 0, "none")
addedge(0.01, 0.09, 0.03, 0.09, 0, "none")
addedge(0.01, 0.09, 0.01, 0.08, 0, "none")
addedge(0.03, 0.09, 0.03, 0.01, 0, "none")
addedge(0.01, 0.01, 0.03, 0.01, 0, "none")
addedge(0.012, 0.088, 0.012, 0.012, 0, "none")
addedge(0.012, 0.012, 0.028, 0.012, 0, "none")
addedge(0.028, 0.012, 0.028, 0.088, 0, "none")
addedge(0.028, 0.088, 0.012, 0.088, 0, "none")
addedge(0, 0.05, 0.009, 0.05, 0, "none")
addedge(0.009, 0.05, 0.009, -0.02, 0, "none")
addedge(0, 0.05, 0, -0.02, 0, "A = 0")
addedge(0, -0.02, 0.009, -0.02, 0, "none")
addedge(0, 0.15, 0, 0.1, 0, "A = 0")
addedge(0, 0.08, 0, 0.05, 0, "A = 0")
addedge(0, -0.02, 0, -0.05, 0, "A = 0")
addedge(0, -0.05, 0.1, 0.05, 90, "A = 0")
addedge(0.1, 0.05, 0, 0.15, 90, "A = 0")

# labels
addlabel(0.0348743, 0.0347237, 0, "Fe")
addlabel(0.00512569, -0.0070852, 0, "Fe")
addlabel(0.021206, 0.0692964, 0, "Cu")
addlabel(0.0141705, 0.12445, 0, "Air")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.035324, 0.043477)
testFluxDensity = abs(abs(point["B_real"]) - 0.032477) < 1e-4
if (not testFluxDensity):
	print("Flux density: " + str(abs(point["B_real"])) + " == " + str(0.032477))

# energy
integral = volumeintegral(0, 1, 2, 3)
testEnergy = (abs(integral["Wm"]) - 0.036347) < 1e-5
if (not testEnergy):
	print("Magnetic energy: " + str(abs(integral["Wm"])) + " == " + str(0.036347))

print("Test: Magnetic steady state - axisymmetric: " + str(testFluxDensity and testEnergy))