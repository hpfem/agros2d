# model
newdocument("RF_waveguide_R100", "planar", "rf", 1, 3, "disabled", 1, 1, 1.6e+10, "harmonic", 1.0, 1.0, 0.0)

# boundaries
addboundary("Perfect electric conductor", "rf_electric_field", 0, 0)
addboundary("Matched boundary", "rf_matched_boundary")
addboundary("Surface current", "rf_surface_current", 1, 0.5)

# materials
addmaterial("Air", 1, 1, 3e-2, 0)

# edges
addedge(-0.01, 0.02286, -0.01, 0, 0, "Surface current")
addedge(0.06907, 0.02286, 0.076, 0.01593, 90, "Perfect electric conductor")
addedge(0.076, 0.01593, 0.081, 0.01593, 0, "Perfect electric conductor")
addedge(0.081, 0.01593, 0.081, 0.02286, 0, "Perfect electric conductor")
addedge(0.086, 0.00693, 0.086, 0, 0, "Perfect electric conductor")
addedge(0.086, 0.00693, 0.081, 0.00693, 0, "Perfect electric conductor")
addedge(0.081, 0.00693, 0.05907, 0, 20, "Perfect electric conductor")
addedge(-0.01, 0, 0.05907, 0, 0, "Perfect electric conductor")
addedge(0.17, 0.02286, 0.081, 0.02286, 0, "Perfect electric conductor")
addedge(0.06907, 0.02286, -0.01, 0.02286, 0, "Perfect electric conductor")
addedge(0.19286, 0, 0.17, 0.02286, 90, "Perfect electric conductor")
addedge(0.19286, 0, 0.19286, -0.04, 0, "Perfect electric conductor")
addedge(0.175, -0.04, 0.175, -0.015, 0, "Perfect electric conductor")
addedge(0.175, -0.015, 0.16, 0, 90, "Perfect electric conductor")
addedge(0.175, -0.04, 0.19286, -0.04, 0, "Matched boundary")
addedge(0.086, 0, 0.16, 0, 0, "Perfect electric conductor")

# labels
addlabel(0.0359418, 0.0109393, 5e-06, 0, "Air")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.019107, 0.016725)
testE = test("Electric field", point["E"], 456.810483)
testE_real = test("Electric field - real", point["E_real"], 141.973049)
testE_imag = test("Electric field - imag", point["E_imag"], 434.18829)
testB = test("Flux density", point["B"], 1.115591e-6)
testBx_real = test("Flux density - x - real", point["Bx_real"], 6.483596e-7)
testBx_imag = test("Flux density - x - imag", point["Bx_imag"], -2.301715e-7)
testBy_real = test("Flux density - y - real", point["By_real"], -4.300969e-7)
testBy_imag = test("Flux density - y - imag", point["By_imag"], -7.656443e-7)
testH = test("Magnetic field", point["H"], 0.887759)
testHx_real = test("Magnetic field - x - real", point["Hx_real"], 0.515948)
testHx_imag = test("Magnetic field - x - imag", point["Hx_imag"], -0.183165)
testHy_real = test("Magnetic field - y - real", point["Hy_real"], -0.34226)
testHy_imag = test("Magnetic field - y - imag", point["Hy_imag"], -0.60928)
testPx = test("Poynting vector - x", point["Px"], 156.567066)
testPy = test("Poynting vector - y", point["Py"], -3.138616, 0.2)
testP = test("Poynting vector", point["P"], 156.598521)

# volume integral
# volume = volumeintegral(0, 1, 2)
# testEnergy = test("Energy", volume["We"], 1.799349e-8)

# surface integral
# surface = surfaceintegral(1, 12)
# testQ = test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: RF TE - planar: " + str(
testE and testE_real and testE_imag and 
testB and testBx_real and testBx_imag and testBy_real and testBy_imag and
testH and testHx_real and testHx_imag and testHy_real and testHy_imag and
testP and testPx and testPy))