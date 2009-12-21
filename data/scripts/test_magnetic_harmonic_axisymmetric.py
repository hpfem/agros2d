# model
newdocument("Harmonic", "axisymmetric", "magnetic", 1, 3, "disabled", 1, 1, 100, "harmonic", 1, 1, 0)

# boundaries
addboundary("A = 0", "magnetic_vector_potential", 0)

# materials
addmaterial("Coil", 1e6, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Iron", 0, 0, 50, 5e6, 0, 0, 0, 0, 0)
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)

# edges
addedge(0, -0.17, 0.15, -0.17, 0, "A = 0")
addedge(0.15, -0.17, 0.15, 0.17, 0, "A = 0")
addedge(0.15, 0.17, 0, 0.17, 0, "A = 0")
addedge(0.035, -0.03, 0.055, -0.03, 0, "none")
addedge(0.055, -0.03, 0.055, 0.11, 0, "none")
addedge(0.055, 0.11, 0.035, 0.11, 0, "none")
addedge(0.035, 0.11, 0.035, -0.03, 0, "none")
addedge(0, -0.05, 0.03, -0.05, 0, "none")
addedge(0.03, -0.05, 0.03, 0.05, 0, "none")
addedge(0.03, 0.05, 0, 0.05, 0, "none")
addedge(0, 0.05, 0, -0.05, 0, "A = 0")
addedge(0, 0.17, 0, 0.05, 0, "A = 0")
addedge(0, -0.05, 0, -0.17, 0, "A = 0")

# labels
addlabel(0.109339, 0.112786, 0, "Air")
addlabel(0.0442134, 0.0328588, 0, "Coil")
addlabel(0.0116506, -0.00740064, 0, "Iron")

# solve
zoombestfit()
solve()

# point value
pointPotential = pointresult(0.031809, 0.045406)
testPotentialReal = abs(abs(pointPotential["A_real"]) - 7.719762e-4) < 1e-6
testPotentialImag = abs(abs(pointPotential["A_imag"]) - 1.729612e-4) < 1e-6
if (not testPotentialReal):
	print "Magnetic potential - real: ", abs(pointPotential["A_real"]), " == ", 7.719762e-4
if (not testPotentialImag):
	print "Magnetic potential - imag: ", abs(pointPotential["A_imag"]), " == ", 1.729612e-4

pointTotalCurrentDensity = pointresult(0.02814, 0.047561)
testTotalCurentDensityReal = abs(abs(pointTotalCurrentDensity["J_real"]) - 1.024812e6) < 1e4
testTotalCurentDensityImag = abs(abs(pointTotalCurrentDensity["J_imag"]) - 1.215725e6) < 1e4
if (not testTotalCurentDensityReal):
	print "Total current density - real: ", abs(pointTotalCurrentDensity["J_real"]), " == ", 1.024812e6
if (not testTotalCurentDensityImag):
	print "Total current density - imag: ", abs(pointTotalCurrentDensity["J_imag"]), " == ", 1.215725e6

pointFluxDensity = pointresult(0.028979, 0.022765)
testFluxDensity = abs(abs(pointFluxDensity["B"]) - 0.378015) < 1e-2
if (not testFluxDensity):
	print "Flux density: ", abs(point["B"]), " == ", 0.378015

# energy
integral = volumeintegral(0, 1, 2)
testEnergy = abs(abs(integral["Wm"]) - 0.119061) < 1e-4
if (not testEnergy):
	print "Magnetic energy: ", abs(integral["Wm"]), " == ", 0.119061

# induced current
integral = volumeintegral(2)
testInducedCurrentReal = abs(abs(integral["Ii_real"]) - 637.318482) < 1
testInducedCurrentImag = abs(abs(integral["Ii_imag"]) - 334.407653) < 1
if (not testInducedCurrentReal):
	print "Induced current - real: ", abs(integral["Ii_real"]), " == ", 637.318482
if (not testInducedCurrentImag):
	print "Induced current - imag: ", abs(integral["Ii_imag"]), " == ", 334.407653

# Lorentz force
integral = volumeintegral(2)
testLorentzForceY = abs(abs(integral["Fy"]) - 0.33924) < 1e-3
if (not testLorentzForceY):
	print "Induced current - real: ", abs(integral["Fy"]), " == ", 0.33924

# Joule losses
integral = volumeintegral(2)
testJouleLosses = abs(abs(integral["Pj"]) - 26.78469) < 1e-1
if (not testJouleLosses):
	print "Joule losses: ", abs(integral["Pj"]), " == ", 26.78469

print "Test: Magnetic harmonic - axisymmetric: ", (testPotentialReal and testPotentialImag and testTotalCurentDensityReal and testTotalCurentDensityImag and 
testFluxDensity and testEnergy and testInducedCurrentReal and testInducedCurrentImag and
testLorentzForceY and testJouleLosses)