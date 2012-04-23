import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "RF_waveguide_R100"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
electrostatic = agros2d.field("rf")
electrostatic.analysis_type = "harmonic"
electrostatic.number_of_refinements = 2
electrostatic.polynomial_order = 3
electrostatic.linearity_type = "linear"
electrostatic.weak_forms = "compiled"

#newdocument("", "", "", 1, 3, "disabled", 1, 1, 1.6e+10, "", 1.0, 1.0, 0.0)

# boundaries
rf.add_boundary("Perfect electric conductor", "rf_electric_field", {"" : 0} , 0, 0)
rf.add_boundary("Matched boundary", "rf_matched_boundary", {"" : 0})
rf.add_boundary("Surface current", "rf_surface_current", {"" : 0}, 1, 0.5)
electrostatic.add_material("Air", {"" : 0, "" : 0}, 1, 1, 3e-2, 0)

# geometry
geometry = agros2d.geometry()

# edges

# edges
geometry.add_edge(-0.01, 0.02286, -0.01, 0, 0, boundaries = {"rf" : "Surface current"})
geometry.add_edge(0.06907, 0.02286, 0.076, 0.01593, 90, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.076, 0.01593, 0.081, 0.01593, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.01593, 0.081, 0.02286, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.086, 0, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.081, 0.00693, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.00693, 0.05907, 0, 20, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(-0.01, 0, 0.05907, 0, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.17, 0.02286, 0.081, 0.02286, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.06907, 0.02286, -0.01, 0.02286, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.17, 0.02286, 90, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.19286, -0.04, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.175, -0.015, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.015, 0.16, 0, 90, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.19286, -0.04, 0, boundaries = {"rf" : "Matched boundary"})
geometry.add_edge(0.086, 0, 0.16, 0, 0, boundaries = {"rf" : "Perfect electric conductor"})

# labels
geometry.add_label(0.0359418, 0.0109393, 5e-06, 0, materials = {"" : "Air"})

geometry.zoom_best_fit()

# solve problem
problem.solve()


# point value
local_values = rf.local_values(0.019107, 0.016725)
testE = rf.test("Electric field", point["E"], 456.810483)
testE_real = rf.test("Electric field - real", point["E_real"], 141.973049)
testE_imag = rf.test("Electric field - imag", point["E_imag"], 434.18829)
testB = test("Flux density", point["B"], 1.115591e-6)
testBx_real = rf.test("Flux density - x - real", point["Bx_real"], 6.483596e-7)
testBx_imag = rf.test("Flux density - x - imag", point["Bx_imag"], -2.301715e-7)
testBy_real = rf.test("Flux density - y - real", point["By_real"], -4.300969e-7)
testBy_imag = rf.test("Flux density - y - imag", point["By_imag"], -7.656443e-7)
testH = rf.test("Magnetic field", point["H"], 0.887759)
testHx_real = rf.test("Magnetic field - x - real", point["Hx_real"], 0.515948)
testHx_imag = rf.test("Magnetic field - x - imag", point["Hx_imag"], -0.183165)
testHy_real = rf.test("Magnetic field - y - real", point["Hy_real"], -0.34226)
testHy_imag = rf.test("Magnetic field - y - imag", point["Hy_imag"], -0.60928)
testPx = rf.test("Poynting vector - x", point["Px"], 156.567066)
testPy = rf.test("Poynting vector - y", point["Py"], -3.138616, 0.2)
testP = rf.test("Poynting vector", point["P"], 156.598521)

# volume integral
# volume_integrals = rf.volume_integrals([0, 1, 2])
# testEnergy = rf.test("Energy", volume["We"], 1.799349e-8)

# surface integral
# surface_integrals = rf.surface_integrals([1, 12])
# testQ = rf.test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: RF TE - planar: " + str(
testE and testE_real and testE_imag and 
testB and testBx_real and testBx_imag and testBy_real and testBy_imag and
testH and testHx_real and testHx_imag and testHy_real and testHy_imag and
testP and testPx and testPy))