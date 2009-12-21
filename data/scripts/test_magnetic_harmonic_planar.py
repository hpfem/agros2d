# model
newdocument("Harmonic", "planar", "magnetic", 1, 3, "disabled", 1, 1, 50, "harmonic", 1, 1, 0)

# boundaries
addboundary("A = 0", "magnetic_vector_potential", 0)

# materials
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Cond 1", 1e6, 0, 1, 5.7e7, 0, 0, 0, 0, 0)
addmaterial("Cond 2", 1e6, 0, 1, 5.7e7, 0, 0, 0, 0, 0)

# edges
addedge(-0.075, 0.06, 0.075, 0.06, 0, "A = 0")
addedge(0.075, 0.06, 0.075, -0.06, 0, "A = 0")
addedge(0.075, -0.06, -0.075, -0.06, 0, "A = 0")
addedge(-0.075, -0.06, -0.075, 0.06, 0, "A = 0")
addedge(-0.015, -0.01, -0.015, 0.01, 0, "none")
addedge(-0.015, 0.01, -0.005, 0.01, 0, "none")
addedge(-0.015, -0.01, -0.005, -0.01, 0, "none")
addedge(-0.005, -0.01, -0.005, 0.01, 0, "none")
addedge(0.005, 0.02, 0.005, 0, 0, "none")
addedge(0.005, 0, 0.015, 0, 0, "none")
addedge(0.015, 0, 0.015, 0.02, 0, "none")
addedge(0.015, 0.02, 0.005, 0.02, 0, "none")

# labels
addlabel(0.035349, 0.036683, 0, "Air")
addlabel(0.00778124, 0.00444642, 1e-05, "Cond 1")
addlabel(-0.0111161, -0.00311249, 1e-05, "Cond 2")

# solve
zoombestfit()
solve()

# point value
pointPotential = pointresult(0.00589,-0.005301)
testPotentialReal = abs(abs(pointPotential["A_real"]) - 1.632192e-5) < 1e-7
testPotentialImag = abs(abs(pointPotential["A_imag"]) - 4.083868e-5) < 1e-7
if (not testPotentialReal):
	print "Magnetic potential - real: ", abs(pointPotential["A_real"]), " == ", 1.632192e-5
if (not testPotentialImag):
	print "Magnetic potential - imag: ", abs(pointPotential["A_imag"]), " == ", 4.083868e-5

pointTotalCurrentDensity = pointresult(0.013567, 0.017081)
testTotalCurentDensityReal = abs(abs(pointTotalCurrentDensity["J_real"]) - 2.408328e5) < 1e2
testTotalCurentDensityImag = abs(abs(pointTotalCurrentDensity["J_imag"]) - 3.299946e5) < 1e2
if (not testTotalCurentDensityReal):
	print "Total current density - real: ", abs(pointTotalCurrentDensity["J_real"]), " == ", 2.408328e5
if (not testTotalCurentDensityImag):
	print "Total current density - imag: ", abs(pointTotalCurrentDensity["J_imag"]), " == ", 3.299946e5

pointFluxDensity = pointresult(0.009454, -0.004136)
testFluxDensity = abs(abs(pointFluxDensity["B"]) - 0.00142) < 1e-4
if (not testFluxDensity):
	print "Flux density: ", abs(point["B"]), " == ", 0.00142

# energy
integral = volumeintegral(0, 1, 2)
testEnergy = abs(abs(integral["Wm"]) - 0.001923) < 1e-5
if (not testEnergy):
	print "Magnetic energy: ", abs(integral["Wm"]), " == ", 0.001923

# Lorentz force
integral = volumeintegral(1);
testLorentzForceX = abs(abs(integral["Fx"]) - 0.020612) < 1e-4
testLorentzForceY = abs(abs(integral["Fy"]) - 0.011107) < 1e-4
if (not testLorentzForceX):
	print "Lorentz force - X: ", abs(integral["Fx"]), " == ", 0.020612
if (not testLorentzForceY):
	print "Lorentz force - Y: ", abs(integral["Fy"]), " == ", 0.011107

# Joule losses
integral = volumeintegral(1)
testJouleLosses = abs(abs(integral["Pj"]) - 0.25327) < 1e-4
if (not testJouleLosses):
	print "Joule losses: ", abs(integral["Pj"]), " == ", 0.25327

print "Test: Magnetic harmonic - planar: ", (testPotentialReal and testPotentialImag and testTotalCurentDensityReal and testTotalCurentDensityImag and 
testFluxDensity and testEnergy and testLorentzForceX and testLorentzForceY and testJouleLosses)
